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

// ���û��ѵ�ʱ����, �������Ȼ��Ѽ���Զ�.
static INT32U wakup_set_interval_ms;
// ��¼����ģʽ��Ҫ�����ػ���ʱ��,�����ӳٹػ�
static INT32U wakup_mode_normal_power_off_tick;
// ����֮����Ĺ���ʱ��
static INT32U work_max_time_after_wakeup_ms;
static INT32U rtc_interrupt_time;
// ����RTC���ѵ�ʱ��� 
static INT32U set_rtc_interrupt_time;
// ��תIO����ʹ��
static INT32U io_tick;
// �����Դ������־
static INT8U power_latch_flag = 0;
// ϵͳ�µ籣��������־
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
            // ��ȡʱ��ʧ��
            bcu_set_mcu_work_state(kMCUWorkStateStop);
        } 
        else 
        {
            if (now < alarm) {
            // û�дﵽ����ʱ��
                bcu_set_mcu_work_state(kMCUWorkStateStop);
            } else if ((now - alarm) > 60UL * 2) {
            // ���뻽���¼���ȥ̫��ʱ��
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
    set_data_save_request_stopped(0x01);//data_save_stop();   //ֹͣSD���洢
#endif
    if(g_bms_work_stop_timeout_tick == 0) g_bms_work_stop_timeout_tick = get_tick_count(); //ϵͳδ����ʱ����Ϊ0������������ʱ�Ÿ��´�ֵ
}

void bms_system_unprotect(void)
{
    power_down_protection_is_start = FALSE;
#if BMS_SUPPORT_DATA_SAVE
    data_save_start();   //�ָ�SD���洢
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
        if(get_elapsed_tick_count(g_bms_work_stop_timeout_tick) < 100) //��֤��λ����������Ӧ
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
    if(battery_num > 200) //��ش�������200��
    {//ÿ����50������100ms��ʱ
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
    if (minutes == 0) { // �رջ��ѹ���.
        return 1;
    }
    
    if (!system_voltage_is_usful()) {// ��ѹ̫��, �����û���
        return 1;        
    }
    
    if (minutes < 2) {
        // ��С2����
        minutes = 2;
    }
    
    
    if (!clock_config_interrupt(RTC_INTERRUPT_ALARM, RTC_INTERRUPT_SINGLE_LOWLEVEL)) 
    {
        return 0;
    }

    
    // ����RTC��ʱֻ��ȷ������, ����5���Ա�֤����.
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
    if(BMS_IS_POWER_ON() != 0)  // �����ϵ�������Ҫֱ�Ӹ�λ,�˳�����ģʽ
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
                // �����˻������������ʱ��.
                power_down_store_data();
                bcu_set_mcu_work_state(kMCUWorkStateStop);
            }
#if BMS_SUPPORT_DATA_SAVE
            else{
                int upload_done = (DTUM35_GetPacketCounter() >= 2);    // �����ϴ���¼���ݶ�Ϊ 2
                int data_save_done = 1;//sdcard_is_available() ? (data_save_get_counter() >= 2) : 1; // ���ٴ洢��¼���ݶ�Ϊ 2

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
                        can_disconnect_emscan();  //�µ�ǰ����CAN�����������ʼ��״̬����ֹ�µ�ʱ���ʹ���֡
                        bcu_set_mcu_work_state(kMCUWorkStateStop); //����ϵͳ����
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
                // ���������û��ѵ�ʱ��, ��������.
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
        bms_system_unprotect(); //ȡ��ϵͳ����
        can_re_init_emscan(); //���½�������CAN��ʼ������ can_disconnect_emscan() �ɶ�ʹ��
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
    
    hcs12_gpio_set_output(&shdn_24v_v116[0].io, 0); //�Ͽ�SHDN��Դ���е������Դ���̵�����HMI���ӻ�.....
    reset_mos_status();    //�򿪾�Ե��������mos��
    BMS_POWER_LATCH_OFF();
    CLKSEL = 0x00; // disengage PLL to system
    
    for (;;) 
    {
     wdt_feed_dog();
    } 
}