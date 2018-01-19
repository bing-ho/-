/**
 *
 * Copyright (c) 2012 Ligoo Inc.
 *
 * @file bms_work_state_impl.c
 * @brief
 * @note
 * @author
 * @date 2014-10-21
 *
 */
#include "bms_work_state_impl.h"
#include "dtu_m35.h"
#include "run_mode.h"
#include "bms_data_save.h"
#include "gpio_hardware.h"
#include "bms_insulation_impl.h"
#include "can_nm.h"
extern const struct hcs12_gpio_detail shdn_24v_v116[];

// 设置唤醒的时间间隔, 这个间隔比唤醒间隔稍短.
static INT32U wakup_set_interval_ms;
// 记录唤醒模式需要正常关机的时刻,用于延迟关机
static INT32U wakup_mode_normal_power_off_tick;
// 唤醒之后最长的工作时间
static INT32U work_max_time_after_wakeup_ms;
static INT32U rtc_interrupt_time;
// 设置RTC唤醒的时间点 
static INT32U set_rtc_interrupt_time;
// 翻转IO测试使用
static INT32U io_tick;
// 软件电源锁定标志
static INT8U power_latch_flag = 0;
// 系统下电保护启动标志
static INT8U power_down_protection_is_start = 0;

#if BMS_SUPPORT_CP_DOWN_POWER
 #define BMS_IS_POWER_ON()   \
    (dc_24V_voltage_is_usful() == TRUE || \
    Get_can_nm_state()  || \
    guobiao_charger_pwm_is_connected() || \
    input_signal_control_is_high(kInputTypePowerOn) == 0 ||\
    input_signal_control_is_high(kInputTypeChargerConnect) == 0 )
#else
  #define BMS_IS_POWER_ON()   \
    (cp_input_is_high() || \
	  Get_can_nm_state()  || \
    dc_24V_voltage_is_usful() == TRUE || \
    guobiao_charger_pwm_is_connected() || \
    input_signal_control_is_high(kInputTypePowerOn) == 0 ||\
    input_signal_control_is_high(kInputTypeChargerConnect) == 0 )
#endif    

#define BMS_POWER_LATCH_ON()    DDR1AD1_DDR1AD16 = 1; PT1AD1_PT1AD16 = 1; power_latch_flag = 1;
#define BMS_POWER_LATCH_OFF()   DDR1AD1_DDR1AD16 = 1; PT1AD1_PT1AD16 = 0; power_latch_flag = 0;

static INT32U g_bms_work_stop_tick = 0, g_bms_work_stop_rel_tick = 0, g_bms_work_stop_timeout_tick = 0;

static void diagnose_work_state_wakeup(void* data);

#if( SWITCH_ON == UDS_SWITCH_CONFIG)
INT8U bms_get_power_state(void)
{
    return BMS_IS_POWER_ON();
}
#endif

void bms_work_state_init(void)
{
    enum runmode mode;
    INT32U now_tick = get_tick_count();
    g_bms_work_stop_tick = get_tick_count();
    g_bms_work_stop_rel_tick = g_bms_work_stop_tick;

    BMS_POWER_LATCH_ON();
    
    mode = runmode_get();
    
    if (mode == RUN_MODE_WAKEUP) {
        uint32_t alarm, now;
        rtc_interrupt_time = now_tick;
        io_tick = now_tick;
        clock_init();//sd2405_init(sd2405_impl);
        clock_clear_interrupt(); 
        bcu_set_mcu_work_state(kMCUWorkStateWork);
        
        if ((!clock_get_seconds(&now)) || (!clock_get_alarm_time(&alarm))) 
        {
            // 读取时间失败
            bcu_set_mcu_work_state(kMCUWorkStateStop);
        } 
        else 
        {
            if (now < alarm) {
            // 没有达到唤醒时间
                bcu_set_mcu_work_state(kMCUWorkStateStop);
            } else if ((now - alarm) > 60UL * 2) {
            // 距离唤醒事件过去太长时间
                bcu_set_mcu_work_state(kMCUWorkStateStop);
            }
        }
        job_schedule(MAIN_JOB_GROUP, BMS_WORK_STATE_JOB_PERIODIC, diagnose_work_state_wakeup, NULL);
    } else {
        clock_init();
        bcu_set_mcu_work_state(kMCUWorkStateWork);
        job_schedule(MAIN_JOB_GROUP, BMS_WORK_STATE_JOB_PERIODIC, diagnose_work_state, NULL);
    }
}

void bms_system_protect(void)
{
    power_down_protection_is_start = TRUE;
#if BMS_SUPPORT_DATA_SAVE
    set_data_save_request_stopped(0x01);//data_save_stop();   //停止SD卡存储
#endif
    if(g_bms_work_stop_timeout_tick == 0) g_bms_work_stop_timeout_tick = get_tick_count(); //系统未保护时保持为0，仅启动保护时才更新此值
}

void bms_system_unprotect(void)
{
    power_down_protection_is_start = FALSE;
#if BMS_SUPPORT_DATA_SAVE
    data_save_start();   //恢复SD卡存储
#endif
    g_bms_work_stop_timeout_tick = 0;
}

INT8U bms_system_is_protecting(void)
{
    INT8U res = FALSE;
    
#if BMS_SUPPORT_DATA_SAVE
    res = data_save_is_stop();
#endif
    if(res == TRUE)
    {
        if(get_elapsed_tick_count(g_bms_work_stop_timeout_tick) < 100) //保证复位命令正常响应
        {
            res = FALSE;
        }
    }
    return res;
}

INT32U bms_calc_power_off_timeout(void)
{
    INT16U battery_num = 0;
    INT32U timeout = BMS_POWER_OFF_ALLOWED_TIMEOUT;
    
    battery_num = bcu_get_total_voltage_num();
    if(battery_num > 200) //电池串数大于200串
    {//每增加50串增加100ms超时
        timeout += ((battery_num - 200) / 50) * 100;
    }
    if(timeout > BMS_POWER_OFF_ALLOWED_TIMEOUT_MAX) 
        timeout = BMS_POWER_OFF_ALLOWED_TIMEOUT_MAX;
    
    return timeout;
}

INT8U bms_system_power_off_is_allowed(void)
{
    if(bms_system_is_protecting() == TRUE) return TRUE;
    
    if(get_elapsed_tick_count(g_bms_work_stop_timeout_tick) >= bms_calc_power_off_timeout()) 
    {
        g_bms_work_stop_timeout_tick = 0;
        return TRUE;
    }
    return FALSE;
}


void toggle_do(void);


static int set_alarm() {
    unsigned long r;
    unsigned short minutes;
    
    if (!input_signal_is_high_by_name("MCU_ZD")) 
    {
        clock_clear_interrupt();
    }

    if (!clock_get_seconds(&r)) {
        return 0;
    }
    
    minutes = config_get(kWakeupIntervalMinutes);    
    if (minutes == 0) { // 关闭唤醒功能.
        return 1;
    }
    
    if (!system_voltage_is_usful()) {// 电压太低, 不设置唤醒
        return 1;        
    }
    
    if (minutes < 2) {
        // 最小2分钟
        minutes = 2;
    }
    
    
    if (!clock_config_interrupt(RTC_INTERRUPT_ALARM, RTC_INTERRUPT_SINGLE_LOWLEVEL)) 
    {
        return 0;
    }

    
    // 由于RTC定时只精确到分钟, 增加5秒以保证分钟.
    r += 60UL * (unsigned long)config_get(kWakeupIntervalMinutes) + 5;         
    if (!clock_set_alarm(r)) 
    {
        return 0;
    }
    
    return 1;    
}

static void diagnose_work_state_wakeup(void* data) {
    MCUWorkState work_state;
    INT32U now_tick = get_tick_count();
       
    UNUSED(data);
    if(BMS_IS_POWER_ON() != 0)  // 其他上电请求需要直接复位,退出唤醒模式
    {
        power_down_store_data();
        mcu_reset();
    }

    work_state = bcu_get_mcu_work_state();    
    if (hardware_io_revision_get() >= HW_VER_115 &&
        wakup_set_interval_ms == 0) 
     {
        if (set_alarm()) {
            set_rtc_interrupt_time = now_tick;
        }
        wakup_set_interval_ms = config_get(kWakeupIntervalMinutes);
        
        if (wakup_set_interval_ms <= 10) {
            wakup_set_interval_ms -= 1; 
        }
        wakup_set_interval_ms *= 54000;
        
        
        work_max_time_after_wakeup_ms = config_get(kWorkMaxTimeAfterWakeupSeconds);
        work_max_time_after_wakeup_ms *= 1000;  // 1000ms/seconds
    }
    switch(work_state)
    {
        case kMCUWorkStateWork:
            if(wakup_mode_normal_power_off_tick != 0) {
                if(get_interval_by_tick(wakup_mode_normal_power_off_tick, now_tick) >= 5000UL) {
                    bcu_set_mcu_work_state(kMCUWorkStateStop);
                }
            }else if(get_interval_by_tick(rtc_interrupt_time, now_tick) >= work_max_time_after_wakeup_ms) {
                // 超过了唤醒允许工作的最长时间.
                power_down_store_data();
                bcu_set_mcu_work_state(kMCUWorkStateStop);
            }
#if BMS_SUPPORT_DATA_SAVE
            else{
                int upload_done = (DTUM35_GetPacketCounter() >= 2);    // 最少上传记录数暂定为 2
                int data_save_done = 1;//sdcard_is_available() ? (data_save_get_counter() >= 2) : 1; // 最少存储记录数暂定为 2

                if(upload_done && data_save_done) {
                    power_down_store_data();
                    wakup_mode_normal_power_off_tick = now_tick;             
                }
            }
#endif
            if(get_interval_by_tick(io_tick, now_tick) >= 2000UL) { // do test
                io_tick = now_tick;
                toggle_do();
            }
            break;
        case kMCUWorkStateStop:
            {
                int i;
                for(i=0; i<5; ++i) {
                    if (set_alarm()) {
                        break;
                    }
                }
            }
            BMS_POWER_LATCH_OFF();
            break;
        default:
            bcu_set_mcu_work_state(kMCUWorkStateStop);
            break;
    }
}

void diagnose_work_state(void* data)
{
    MCUWorkState work_state;
    INT32U now_tick = get_tick_count();
       
    UNUSED(data);
    work_state = bcu_get_mcu_work_state();
    
    if (hardware_io_revision_get() >= HW_VER_115 &&
        wakup_set_interval_ms == 0) 
    {
        if (set_alarm()) {
            set_rtc_interrupt_time = now_tick;
        }
        wakup_set_interval_ms = config_get(kWakeupIntervalMinutes);
        
        if (wakup_set_interval_ms <= 10) {
            wakup_set_interval_ms -= 1; 
        }
        wakup_set_interval_ms *= 54000;
    }
    
    power_down_eeeprom_store_valid_check();
    switch(work_state)
    {
        case kMCUWorkStateWork:
            if(BMS_IS_POWER_ON() == 0)
            {
                bms_system_protect();
                g_bms_work_stop_rel_tick = now_tick;
                if(get_interval_by_tick(g_bms_work_stop_tick, now_tick) >= BMS_WORK_STATE_STOP_DELAY)
                {
                    if(bms_system_power_off_is_allowed())
                    {
                        g_bms_work_stop_tick = now_tick;
                        power_down_store_data();
                        can_disconnect_emscan();  //下电前设置CAN控制器进入初始化状态，防止下电时发送错误帧
                        bcu_set_mcu_work_state(kMCUWorkStateStop); //进入系统保护
                    }
                }
            }
            else
            {
                BMS_POWER_LATCH_ON();
                bms_power_down_protect_rel_check();

            }
            
            if (hardware_io_revision_get() >= HW_VER_115 &&
                get_interval_by_tick(set_rtc_interrupt_time, now_tick) >= wakup_set_interval_ms) {
                // 超过了设置唤醒的时间, 重新设置.
                if (set_alarm()) {
                     set_rtc_interrupt_time = now_tick;
                }
            }
            
            break;
        case kMCUWorkStateStop:
            if (hardware_io_revision_get() >= HW_VER_115 ) {
            //bms power off
                int i;
                for(i=0; i<5; ++i) {
                    if (set_alarm()) {
                        break;
                    }
                }
            }            
            BMS_POWER_LATCH_OFF();
            bms_power_down_protect_rel_check();
            break;
        default:
            bcu_set_mcu_work_state(kMCUWorkStateStop);
            break;
    }
}

void bms_power_down_protect_rel_check(void)
{
    INT32U now_tick = get_tick_count();
    
    g_bms_work_stop_tick = now_tick;
    if(power_down_protection_is_start == FALSE) return;
    if(bcu_reset_is_start() == TRUE) return;

    if(get_interval_by_tick(g_bms_work_stop_rel_tick, now_tick) >= BMS_WORK_STATE_STOP_REL_DELAY)
    {
        g_bms_work_stop_rel_tick = now_tick;
        bms_system_unprotect(); //取消系统保护
        can_re_init_emscan(); //重新进行整车CAN初始化，与 can_disconnect_emscan() 成对使用
        bcu_set_mcu_work_state(kMCUWorkStateWork);
    }
}

INT8U bms_power_trigger_is_not_rtc(void)
{
    if(input_signal_control_is_high(kInputTypePowerOn) == 0
        || input_signal_control_is_high(kInputTypeChargerConnect) == 0
        || input_signal_control_is_high(kInputTypeSigOne) == 0
        || cp_input_is_high() 
        || guobiao_charger_pwm_is_connected()
        || dc_24V_voltage_is_usful() == TRUE)
        return TRUE;
    return FALSE;
}

INT8U bms_get_power_trigger_signal(void)
{
   /* if(input_signal_control_is_high(kInputTypePowerOn) == 0) return kBmsPowerTriggerON;
    else if(input_signal_control_is_high(kInputTypeChargerConnect) == 0) return kBmsPowerTriggerChargerReady;
    else if(input_signal_control_is_high(kInputTypeSigOne) == 0) return kBmsPowerTriggerSignalIn1;
    else if(cp_input_is_high() || guobiao_charger_pwm_is_connected()) return kBmsPowerTriggerCP;
    else if(dc_24V_voltage_is_usful() == TRUE) return kBmsPowerTrigger24VIN;
    else if(power_latch_flag) return kBmsPowerTriggerSoftwareLatch;  
    return kBmsPowerTriggerNone;*/
    INT8U triggerdata = 0;
    if(input_signal_control_is_high(kInputTypePowerOn) == 0) triggerdata|=0x01;    
    if(input_signal_control_is_high(kInputTypeChargerConnect) == 0) triggerdata|=0x02;
    //if(input_signal_control_is_high(kInputTypeSigOne) == 0) return triggerdata|=0x04;
    if(cp_input_is_high() || guobiao_charger_pwm_is_connected()) triggerdata|=0x08;
    if(dc_24V_voltage_is_usful() == TRUE) triggerdata|=0x10;
    if(power_latch_flag) triggerdata|=0x20; 
    return  triggerdata;
      
}

void low_power_mode(void) 
{
    
    hcs12_gpio_set_output(&shdn_24v_v116[0].io, 0); //断开SHDN电源所有的外设电源，继电器、HMI、从机.....
    reset_mos_status();    //打开绝缘检测的两个mos管
    BMS_POWER_LATCH_OFF();
    CLKSEL = 0x00; // disengage PLL to system
    
    for (;;) 
    {
     wdt_feed_dog();
    } 
}