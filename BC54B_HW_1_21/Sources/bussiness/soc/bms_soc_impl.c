/**
 *
 * Copyright (c) 2012 Ligoo Inc.
 *
 * @file soc_daemon.c
 * @brief
 * @note
 * @author
 * @date 2012-5-9
 *
 */
#include "bms_soc_impl.h"
#include "adc_xgate.h"
#include "bms_charger_stock.h"
#include "soc0_dischg_factor_impl.h"
#include "mcp3421_hardware.h"
#include "mcp3421_intermediate.h" 
#include "bms_ocv_soc_bm.h"
#if BMS_SUPPORT_SOC == BMS_SUPPORT_SOC

#pragma MESSAGE DISABLE C4001 //Condition always FALSE
#pragma MESSAGE DISABLE C12056 // SP debug info incorrect because of optimization or inline assembler
#pragma MESSAGE DISABLE C5703 // Parameter is not used
#pragma MESSAGE DISABLE  C5919 // Conversion of floating to unsigned integral

#pragma DATA_SEG __RPAGE_SEG PAGED_RAM
INT16S g_soc_current_low_sample_buffer[SOC_CURRENT_SAMPLE_COUNT];
INT16S g_soc_current_high_sample_buffer[SOC_CURRENT_SAMPLE_COUNT];
INT16S g_soc_current_single_sample_buffer[SOC_CURRENT_SAMPLE_COUNT];
static INT32U g_soc_last_tick = 0;
static INT32U g_last_failed_tick = 0;
static INT32U g_soc_total_delta_time = 0;
static INT8U  g_soc_checked_empty = 0;
static INT8U  g_soc_checked_full = 0;
static INT8U  g_soc_checked_high_left_cap = 0;
//static INT8U  g_soc_checked_deep_discharging = 0;
static INT8U g_soc_upward_check_flag = 0;                  //剩余容量大于总容量时触发向上校正，校正后置位此标志，用于校正释放
static INT32U g_soc_empty_calib_check_last_tick = 0;
static INT8U soc_empty_calib_delay_start_flag = 0; //放电单体低压校正延时启动标志
static INT8U g_soc_reach_100_flag = 0;
static const struct soc_adc_param g_current_ad_param = {
    1, // 1ms
    30, // 40次滤波
    0, // 丢弃10个最大值
    0, // 丢弃10个最小值
};
static INT16U ad1_high_value = 0;
static INT16U ad1_low_value = 0;
static INT16U ad1_single_value = 0;
static INT32U g_charge_cap_vary=0;
static INT32U g_discharge_cap_vary=0;
#pragma DATA_SEG __RPAGE_SEG DEFAULT
union 
    {INT16U totalcapword[2];
     INT32U totalcapdoubleword;
    }totalcap={0};


#define CURRENT_TO_MAS(CURRENT, TIME) ((INT32U)(CURRENT) * (TIME) / 10)
#define CURRENT_TO_MAS_WITH_FACTOR(MAS, FACTOR) ((INT32U)MAS * FACTOR / 1000)
#define CHECK_AND_UPDATE_CAP_INTERM(CAP)    cap_interm_max = CURRENT_TO_MAS(config_get(kCurSenTypeIndex) * 10, delta_time);\
                                            if(CAP > cap_interm_max) CAP = cap_interm_max;
INT8U SOC0_flag = 0;
void soc_init(void)
{
    Result res = 0;
    INT8U sample_num = (INT8U)SOC_CURRENT_SAMPLE_COUNT;//config_get(kCurSampleCntIndex);
    INT8U min_num;
    
    watch_dog_register(WDT_CURRENT_ID, WDT_CURRENT_TIME);

    /* 从配置记录中恢复上次记录的SOC值*/
    //sd2405_init(sd2405_platform_impl);
    if (power_down_restore_left_cap() == 0)
        bcu_set_left_cap_interm(CAP_TENFOLD_AH_TO_MAS(config_get(kLeftCapIndex)));//bcu_set_left_cap_interm(CAP_AH_TO_MAS(config_get(kLeftCapIndex))); //剩余容量：1Ah==>1Ams
    if((config_get(kLeftCapIndex) == 0) && (config_get(kSocFixAsEmptyIndex) == 0))  SOC0_flag = LEFTSOC0FLAGE;     
    bcu_set_SOC(soc_from_cap(config_get(kLeftCapIndex), config_get(kTotalCapIndex)));
    ocv_soc_calib_params_init(); 
    
    /** 计算最小采样数 */
    min_num = (INT8U)((sample_num + 1) / 2);

    /** 初始化电流采样模块*/  
 #if BMS_SUPPORT_CURRENT_DIVIDER 
  if((config_get(kCurSenRangeTypeIndex) == CURRENT_DIVIDER_TYPE))     //硬件支持且配置选择分流器
  {
      MCP3421_Hardware_Init(&g_Mcp3421_BspInterface); 
  }
  else 
 #endif
  {
    
    res = current_sample_init(SOC_CURRENT_SAMPLE_PERIOD,
            sample_num, g_soc_current_low_sample_buffer,g_soc_current_high_sample_buffer,g_soc_current_single_sample_buffer,
            min_num, SOC_CURRENT_SAMPLE_FILTER_NUM,
            config_get(kCurSenTypeIndex),config_get(kCurSenType1Index),(INT8U)config_get(kCurSenRangeTypeIndex));//config_get(kCurSampleintervalIndex)
    BMS_ASSERT(res == RES_OK);
  }



    event_observe(kInfoEventBatteryChargeState, soc_on_voltage_charging_normal, NULL);
    event_observe(kInfoEventBatteryChargeState, soc_on_voltage_charging_2nd_normal, NULL);
    event_observe(kInfoEventBatteryDischargeState, soc_on_voltage_discharging_normal, NULL);
    event_observe(kInfoEventBatteryDischargeState, soc_on_voltage_discharging_3rd_alarm, NULL);

    bcu_set_soc_full_uncalib_reason(kSOCFullCalibNoTrigger);
    bcu_set_soc_empty_uncalib_reason(kSOCEmptyCalibNoTrigger);
    /** 开启SOC监控任务 */
    
    soc_adc_init();
    soc_adc_start(&g_current_ad_param);
}

void soc_unit(void)
{
  #if (BMS_SUPPORT_CURRENT_DIVIDER != 1)
   if((config_get(kCurSenRangeTypeIndex) != CURRENT_DIVIDER_TYPE))     //电流采集方式为HALL
     {
        current_sample_uninit(); 
     }
  #endif
}

INT8U soc_is_support(void)
{
    return BMS_SUPPORT_SOC;
}

INT32U soc_get(void)
{
    return bcu_get_SOC();
}

INT32U soc_get_total_delta_time(void)
{
    return g_soc_total_delta_time;
}
/*
void soc_update_left_cap_by_ah(INT16U cap)
{
    OS_CPU_SR cpu_sr = 0;

    if (cap > soc_get_max_cap()) cap = soc_get_max_cap();

    OS_ENTER_CRITICAL();
    bcu_set_left_cap_interm(CAP_AH_TO_MAS(cap)); //剩余容量：1Ah==>1Ams
    config_save(kLeftCapIndex, cap);
    bcu_set_SOC(soc_from_cap(config_get(kLeftCapIndex), config_get(kTotalCapIndex)));
    OS_EXIT_CRITICAL();
}*/
//////////////////////////////////////////////////////////////////////////////////////////////
void soc_update_left_cap_by_tenfold_ah(INT16U cap)
{
    OS_CPU_SR cpu_sr = 0;

    if (cap > soc_get_max_cap()) cap = soc_get_max_cap();

    OS_ENTER_CRITICAL();
    bcu_set_left_cap_interm(CAP_TENFOLD_AH_TO_MAS(cap));//bcu_set_left_cap_interm(CAP_AH_TO_MAS(cap)); //剩余容量：1Ah==>1Ams
    config_save(kLeftCapIndex, cap);
    bcu_set_SOC(soc_from_cap(config_get(kLeftCapIndex), config_get(kTotalCapIndex)));
    OS_EXIT_CRITICAL();
}
/////////////////////////////////////////////////////////////////////////////////////////////////
/*void soc_update_left_cap_by_ams(CapAms_t cap)
{
    INT16U cap_ah;
    OS_CPU_SR cpu_sr = 0;

    cap_ah = CAP_MAS_TO_AH(cap);
    if (cap_ah > soc_get_max_cap()) 
    {
        cap_ah = soc_get_max_cap();
        cap = CAP_AH_TO_MAS(cap_ah);
    }

    OS_ENTER_CRITICAL();
    bcu_set_left_cap_interm(cap); //剩余容量
    config_save(kLeftCapIndex, cap_ah);
    bcu_set_SOC(soc_from_cap(config_get(kLeftCapIndex), config_get(kTotalCapIndex)));
    OS_EXIT_CRITICAL();
}*/
///////////////////////////////////////////////////////////////////////////////////////////////////
void soc_update_tenfold_left_cap_by_ams(CapAms_t cap)
{
    INT16U cap_ah;
    OS_CPU_SR cpu_sr = 0;

    cap_ah = CAP_MAS_TO_TENFOLD_AH(cap);
    if (cap_ah > soc_get_max_cap()) 
    {
        cap_ah = soc_get_max_cap();
        cap = CAP_TENFOLD_AH_TO_MAS(cap_ah);
    }

    OS_ENTER_CRITICAL();
    bcu_set_left_cap_interm(cap); //剩余容量
    config_save(kLeftCapIndex, cap_ah);
    bcu_set_SOC(soc_from_cap(config_get(kLeftCapIndex), config_get(kTotalCapIndex)));
    OS_EXIT_CRITICAL();
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void soc_upward_check_limit(void)
{
    g_soc_upward_check_flag = 1;
}

void soc_upward_check_limit_release(void)
{
    g_soc_upward_check_flag = 0;
}

INT8U soc_is_upward_check_limit(void)
{
    if(g_soc_upward_check_flag) return 1;
    
    return 0;
}

void soc_set_soc(INT16U soc)
{
   static INT16U  save_soc=0;
#if SOC_NOT_100_PERCENT_FOR_CHARGING_EN
    if(charger_is_connected() && charger_charging_is_enable())
    {
        if(g_soc_reach_100_flag == 0 && soc > SOC_MAX_FOR_CHARGER_CHARGING)
        {
            if(soc > soc_get()) 
            {
                soc = SOC_MAX_FOR_CHARGER_CHARGING; 
                bcu_set_left_cap_interm((INT32U)soc*config_get(kTotalCapIndex)*36);
            }
            else return;
        }
    }
    else
    {
        if(soc > (SOC_MAX_FOR_CHARGER_CHARGING+10)) g_soc_reach_100_flag = 1; //充满电且SOC大于99%,置位标志
    }
    if(soc <= SOC_MAX_FOR_CHARGER_CHARGING) g_soc_reach_100_flag = 0; //SOC下降到99%及以下时清零标志
#endif
     bcu_set_SOC(soc);
     if(abs(save_soc - soc)>10)  //SOC变化0.1%存储一次
     {
        save_soc = soc;
        config_save(kLeftCapIndex,CAP_MAS_TO_TENFOLD_AH(bcu_get_left_cap_interm()));
        save_delta_battery_cap_ams_to_config();
        save_left_cap_ams_display_to_config();
     }
}

void soc_refresh(void)
{
    INT16U temp_cap;
    OS_CPU_SR cpu_sr = 0;
    INT16U soc = bcu_get_SOC();
    INT16U total_cap, left_cap;
    INT16U volt_soc = 0;
    INT16U low_volt = bcu_get_low_voltage();

    OS_ENTER_CRITICAL();
    total_cap = config_get(kTotalCapIndex);
    left_cap = config_get(kLeftCapIndex);
    temp_cap = CAP_MAS_TO_TENFOLD_AH(bcu_get_left_cap_interm());//temp_cap = CAP_MAS_TO_AH(bcu_get_left_cap_interm()); //四舍五入的方式转换当前剩余容量：1Ams==>1Ah
    
    if (temp_cap > total_cap)
    {
        temp_cap = total_cap;
        bcu_set_left_cap_interm(CAP_TENFOLD_AH_TO_MAS(temp_cap));//bcu_set_left_cap_interm(CAP_AH_TO_MAS(temp_cap));
    }

    //if (temp_cap != left_cap)
    //{
    //    /*更新剩余容量*/
    //    config_save(kLeftCapIndex, temp_cap);
    //}
    OS_EXIT_CRITICAL();
    if((SOC0_flag == LEFTSOC0FLAGE) && (bmu_is_valid_voltage(low_volt) == 1) && (low_volt != 0)) 
        {    
          SOC0_flag=0;
          volt_soc=get_soc_from_volt_table(low_volt);
          bcu_set_SOC(volt_soc);
          bcu_set_left_cap_interm((INT32U)volt_soc*config_get(kTotalCapIndex)*36);   
        }
    else /** 计算SOC值并更新全局SOC值 */
        {
         if(get_tick_count() > SOC_INIT_DISPLAY_DELAY)    //soc在上电2S内不进行刷新  防止上电还没检测到充电连接就先刷新SOC值导致SOC保持策略失效
          { 
            soc = soc_from_cap2(bcu_get_left_cap_interm(), total_cap);
            soc_set_soc(soc); 
          }
        }
        
    if(bcu_get_SOC() > config_get(kCapCalibLowSoc))  config_save(kSocFixAsEmptyIndex,0);  //SOC大于过放SOC校正阀值，恢复空校正标志位
           
    /** 判断电池充放电周期 */
    if (config_get(kCycleStsIndex) == 1 && bcu_get_SOC() < SOC_CYCLE_LOW_THRESHOLD) /*40%更新循环次数*/
    {
        config_save(kCycleStsIndex, 0);
    }
    else if ((config_get(kCycleStsIndex) == 0) && (bcu_get_SOC() > SOC_CYCLE_HIGH_THRESHOLD)) /*90%更新循环次数可写标识*/
    {
        temp_cap = total_cap / (config_get(kChargerCurIndex) / 10)/ 3; /*至少充电总充电时间的1/3*/
        if(get_bms_charge_eclipse_time_s() >= S_FROM_HOUR(temp_cap))
        {
            config_save(kCycleStsIndex, 1);
            config_save(kCycleCntIndex, config_get(kCycleCntIndex) + 1);
        }
    }
}

void soc_empty_calib_check(void)
{
    if(soc_empty_calib_delay_start_flag == 0) return;
    
    if(get_interval_by_tick(g_soc_empty_calib_check_last_tick, get_tick_count()) < (INT32U)config_get(kCapCalibLowSocDly) * 1000)
    {
        bcu_set_soc_empty_uncalib_reason(kSOCEmptyCalibDelay);
        return;
    }
    soc_empty_calib_delay_start_flag = 0;
    soc_check_and_fix_as_empty(); //启动校正
    return;   
}

void soc_daemon_run(void)
{
    g_soc_last_tick = get_tick_count();
    for (;;)
    {
        soc_daemon_check();
    }
}

INT16U bms_get_ad1_value_h(void)
{
    return ad1_high_value;
}

INT16U bms_get_ad1_value_l(void) 
{
    return ad1_low_value;
}
INT16U bms_get_ad1_value_single(void) 
{
    return ad1_single_value;
}
void soc_daemon_check(void)
{
    Result res;
    INT16S current=0;
    INT16U charger_current = 0;
    INT32U now_tick;
    INT32U delta_time, cap_interm_max;
    INT32U temp;
    INT32U remain_cap = 0;
    INT16U high_value = 0;
    INT16U low_value = 0;
    INT16U single_value = 0;
    
    INT32U left_cap;
    INT8U  left_cap_sign = get_delta_battery_cap_sign();
    
    INT32U i2c_value = 0;
    CurrentCheck current_check_diff=bcu_get_current_check_diff();
    Current_platform platform = {0};
    watch_dog_feed(WDT_CURRENT_ID);
 #if BMS_SUPPORT_CURRENT_DIVIDER 
   if((config_get(kCurSenRangeTypeIndex) == CURRENT_DIVIDER_TYPE))      //硬件支持且配置选择分流器
    {
         res = Mcp3421_AvrgCurrent_Get(&current);
          bcu_set_current(current);
    }
    else
 #endif 
    {  
    while (0 == soc_wait_timeout(&high_value, &low_value,&single_value,g_current_ad_param.interval * g_current_ad_param.times * 3));
    //TODO
    ad1_high_value = high_value; 
    ad1_low_value = low_value;
    ad1_single_value = single_value; 
    //res = current_sample_get(ad1_high_value,ad1_low_value,single_value,&current,&current1, config_get(kCchkIndex), config_get(kHighCurChkIndex), config_get(kCurZeroFilterIndex));
    //res = current_sample_get(ad1_high_value,ad1_low_value,single_value,&current,&current1, current_check_diff.diff0, current_check_diff.diff1, config_get(kCurZeroFilterIndex));
    
    platform.ad_high_value = high_value;
    platform.ad_low_value = low_value;
    platform.ad_single_value = single_value;
    platform.calibration_value = current_check_diff.diff0;
    platform.high_calib_value = current_check_diff.diff1;
    platform.cur_zero_filter = config_get(kCurZeroFilterIndex);
    res = get_sample_current(platform);
   }
    if (res != RES_OK)
    {
        if (get_elapsed_tick_count_with_init(&g_last_failed_tick) >= SOC_CURRENT_FAILURE_DELAY)
        {
            bcu_set_current(0);
            bcu_flag_alarm_check_flag(SELFCHECK_CURRENT_SAMPLE_FAILURE);
            bcu_flag_sys_exception_flags(SYS_EXCEPTION_CURRENT_ERROR);//TODO:测试用，此标志不清零
        }

        sleep(CURRENT_SAMPLE_MIN_PERIODIC);
        return;
    }
    current = bcu_get_current();
    /* 刷新系统电流信息 */
    //bcu_set_current(current);
    //bcu_set_current1(current1);
	if (g_last_failed_tick != 0)
    {
       g_last_failed_tick = 0;
	   bcu_unflag_alarm_check_flag(SELFCHECK_CURRENT_SAMPLE_FAILURE);
    }

    /** calc the delta time */
    now_tick = get_tick_count();
    delta_time = get_interval_by_tick(g_soc_last_tick, now_tick);
    g_soc_last_tick = now_tick;

    /** check the micro-current */

    /** 更新剩余容量 */
    if (current > 0) //充电时
    {
        INT32U left_cap;
        totalcap.totalcapword[1]=config_get(kChgTotalCapLowIndex);
        totalcap.totalcapword[0]=config_get(kChgTotalCapHighIndex);
        temp = CURRENT_TO_MAS_WITH_FACTOR(CURRENT_TO_MAS(current, delta_time), CHARGE_EFFICIENCY_FACTOR_DEF);//config_get(kChargeEfficiencyFactor)
        CHECK_AND_UPDATE_CAP_INTERM(temp);
        g_charge_cap_vary+=temp;
        
        left_cap = get_delta_battery_cap_ams();//bcu_get_left_cap_interm();
            
        if(left_cap_sign > 0)
        {
                
            if(0xFFFFFFFF - left_cap < temp) 
            {
                left_cap = 0xFFFFFFFF;
            }
            else 
            {
               left_cap = left_cap + temp;
            }
        }
        else
        {
            if(left_cap < temp)
            {
                left_cap = temp - left_cap;
                set_delta_battery_cap_sign(1);
            }
            else
            {
                left_cap = left_cap - temp;
            }
        }
              
        if( (10000*get_soc_display() ) < SOC_MAX_FOR_CHARGER_CHARGING)
        {
            set_delta_battery_cap_ams( left_cap );
            update_left_cap_ams_display_with_factor(temp, 1);
            update_left_cap_by_soc_display();
        }

        if(CAP_MAS_TO_TENFOLD_AH(g_charge_cap_vary)>=5) //if(CAP_MAS_TO_AH(g_charge_cap_vary)>=1) 
        {  
          totalcap.totalcapdoubleword=totalcap.totalcapdoubleword+5;
          config_save(kChgTotalCapLowIndex,totalcap.totalcapword[1]);
          config_save(kChgTotalCapHighIndex,totalcap.totalcapword[0]);
          g_charge_cap_vary=g_charge_cap_vary - CAP_TENFOLD_AH_TO_MAS(5);//CAP_AH_TO_MAS(1);
        }
        // bcu_set_left_cap_interm(temp);
        bcu_flag_pack_state(1 << PACK_CHG_BIT); /*充电标识*/
    }
    else
    {
        totalcap.totalcapword[1]=config_get(kDChgTotalCapLowIndex);
        totalcap.totalcapword[0]=config_get(kDChgTotalCapHighIndex); 
        // #if BMS_SUPPORT_SOC0_DISCHG_FACTOR ==1 //使能
        //     temp = CURRENT_TO_MAS_WITH_FACTOR(CURRENT_TO_MAS(abs((INT16S)current), delta_time), get_dischg_factor()); 
        // #else
        //     temp = CURRENT_TO_MAS_WITH_FACTOR(CURRENT_TO_MAS(abs((INT16S)current), delta_time), DISCHARGE_EFFICIENCY_FACTOR_DEF); //config_get(kDisChargeEfficiencyFactor)
        // #endif
        temp = CURRENT_TO_MAS_WITH_FACTOR(CURRENT_TO_MAS(abs((INT16S)current), delta_time), DISCHARGE_EFFICIENCY_FACTOR_DEF); 
        
        CHECK_AND_UPDATE_CAP_INTERM(temp);
        
        left_cap = get_delta_battery_cap_ams();;
        if(left_cap_sign > 0)
        {
            
            if (left_cap > temp)
            {
              left_cap = left_cap - temp;
            }
            else 
            {  
              left_cap = temp - left_cap;
              set_delta_battery_cap_sign(0);
            }                  
        }
        else
        {
            if(0xFFFFFFFF - left_cap < temp) 
            {
                left_cap = 0xFFFFFFFF;
            }
            else 
            {
                left_cap = left_cap + temp;
            }
        }
        set_delta_battery_cap_ams( left_cap );
        temp = update_left_cap_ams_display_with_factor(temp, -1);
        update_left_cap_by_soc_display();
        
        if (bcu_get_left_cap_interm() > temp) 
        {
            g_discharge_cap_vary+=temp;
            if(CAP_MAS_TO_TENFOLD_AH(g_discharge_cap_vary)>=5) //if(CAP_MAS_TO_AH(g_discharge_cap_vary)>=1) 
            { 
              totalcap.totalcapdoubleword=totalcap.totalcapdoubleword+5;
              config_save(kDChgTotalCapLowIndex,totalcap.totalcapword[1]);
              config_save(kDChgTotalCapHighIndex,totalcap.totalcapword[0]);
              g_discharge_cap_vary = g_discharge_cap_vary - CAP_TENFOLD_AH_TO_MAS(5);// CAP_AH_TO_MAS(1);
            }
            // bcu_set_left_cap_interm(bcu_get_left_cap_interm() - temp);
        }
        // else
            // bcu_set_left_cap_interm(0);
        bcu_unflag_pack_state(1 << PACK_CHG_BIT); /*取消充电标识*/
    }
    #if BMS_SUPPORT_SOC0_DISCHG_FACTOR == 1
    SOC0_IntegralAlgorithm(delta_time,current);
    #endif
    g_soc_total_delta_time += delta_time;

    if ((INT16U)delta_time <= CURRENT_SAMPLE_MIN_PERIODIC)
        sleep(CURRENT_SAMPLE_MIN_PERIODIC - (INT16U)delta_time);
    else
        sleep(1);
}

INT16U soc_from_cap(INT16U left_cap, INT16U total_cap)
{
    if (total_cap == 0 || left_cap == 0) return 0;
    if (left_cap >= total_cap) return SOC_MAX_VALUE;
    return (INT16U)DIVISION(GAIN(left_cap , SOC_MAX_VALUE) ,total_cap);//return (INT16U)((INT32U) left_cap * SOC_MAX_VALUE / total_cap);
}

INT16U soc_from_cap2(CapAms_t left_cap, INT16U total_cap)
{
    CapAms_t total;
    INT16U soc;
    if (total_cap == 0 || left_cap == 0) return 0;
    total = CAP_TENFOLD_AH_TO_MAS(total_cap);//CAP_AH_TO_MAS(total_cap);

    if (left_cap >= total) return SOC_MAX_VALUE;
    soc = (INT16U)((double)left_cap / total * SOC_MAX_VALUE + 0.5);
    if (soc >= SOC_MAX_VALUE) soc = SOC_MAX_VALUE;

    return soc;
}

INT16U soc_get_min_total_cap()
{
    return (INT16U)((INT32U)config_get(kNominalCapIndex) * SOC_CALIB_MIN_SOH / 100);
}

INT16U soc_get_max_cap(void)
{
    return config_get(kNominalCapIndex) + MAX_CAP_OFFSET;
}

void soc_fix_as_empty(void)
 {
    set_delta_battery_cap_ams(0);
    set_left_cap_ams_display(0);
    set_soc_bm_high(0);
    set_soc_bm_low(0);
    config_save(kSOCBMAvailableFlag, 0);
    
    save_delta_battery_cap_ams_to_config();
    save_left_cap_ams_display_to_config();
    
    config_save(kLeftCapIndex, 0);
    bcu_set_left_cap_interm(0);
 }

void soc_fix_as_full(void)
 {
    INT16U total_cap;

    total_cap = config_get(kTotalCapIndex);

    set_delta_battery_cap_ams(0);
    set_left_cap_ams_display((INT32U)total_cap*3600*1000);
    set_soc_bm_high(1);
    set_soc_bm_low(1);
    config_save(kSOCBMAvailableFlag, 0);
    
    save_delta_battery_cap_ams_to_config();
    save_left_cap_ams_display_to_config();
    
    config_save(kLeftCapIndex, total_cap);
    bcu_set_left_cap_interm(CAP_TENFOLD_AH_TO_MAS(total_cap));//bcu_set_left_cap_interm(CAP_AH_TO_MAS(total_cap));
    soc_set_soc(PERCENT_TO_SOC(100));
    
    full_chg_set_dischg_factor();
 }

void soc_check_and_fix_as_full(void)
{
    if (g_soc_checked_full) 
    {
        bcu_set_soc_full_uncalib_reason(kSOCFullCalibNotRel);
        return;
    }

    if (bcu_get_average_voltage() < config_get(kCapCalibHighAverageVoltage))
    {
        bcu_set_soc_full_uncalib_reason(kSOCFullCalibLowAverageVolt);
        return;
    }
    if(config_get(kTotalCapIndex) != config_get(kLeftCapIndex))
    {
        g_soc_checked_full = 1;
        g_soc_reach_100_flag = 1; //允许SOC达到100%
        bcu_set_soc_full_uncalib_reason(kSOCFullCalibNormal);
        soc_fix_as_full();
    }
}

void soc_check_and_fix_as_empty(void)
{
    INT16U average_voltage;
    
    if(g_soc_checked_empty) 
    {
        return;
    }

    average_voltage = bcu_get_average_voltage();
    if (average_voltage == 0 || average_voltage > config_get(kCapCalibLowAverageVoltage))
    {
        return;
    }
    
    bcu_set_soc_empty_uncalib_reason(kSOCEmptyCalibNormal);
    g_soc_checked_empty = 1;
    config_save(kSocFixAsEmptyIndex,g_soc_checked_empty);//将SOC空校正标志位存储在EEPROM中
    soc_fix_as_empty();
}

Result soc_on_voltage_charging_normal(EventTypeId event, void* event_data, void* user_data)
{
    if (bcu_get_charge_state() == kAlarmNone)
    {
        g_soc_checked_full = 0;
        bcu_set_soc_full_uncalib_reason(kSOCFullCalibNoTrigger);
    }
    
    if (g_soc_checked_high_left_cap && bcu_get_charge_state() == kAlarmNone)
        g_soc_checked_high_left_cap = 0;

    return RES_OK;
}

Result soc_on_voltage_charging_2nd_normal(EventTypeId event, void* event_data, void* user_data)
{
    if(bcu_get_charge_state() == kAlarmSecondLevel && charger_is_connected())
    {
        soc_check_and_fix_as_full();
    }
    return RES_OK;
}

Result soc_on_voltage_discharging_normal(EventTypeId event, void* event_data, void* user_data)
{
    if (bcu_get_discharge_state() == kAlarmNone)
    {
        g_soc_checked_empty = 0;
        bcu_set_soc_empty_uncalib_reason(kSOCEmptyCalibNoTrigger);
    }
    
    soc_empty_calib_delay_start_flag = 0;
    
    return RES_OK;
}

Result soc_on_voltage_discharging_3rd_alarm(EventTypeId event, void* event_data, void* user_data)
{
    INT16U average_voltage;
    
    g_soc_empty_calib_check_last_tick = get_tick_count();

    if(bcu_get_dchg_lv_state() < kAlarmThirdLevel)
    {
        return RES_ERR;
    }
    if(bcu_get_SOC() > config_get(kCapCalibLowSoc))
    {
        bcu_set_soc_empty_uncalib_reason(kSOCEmptyCalibHSOC);
        return RES_ERR;
    }
    if(g_soc_checked_empty) 
    {
        bcu_set_soc_empty_uncalib_reason(kSOCEmptyCalib3rdLVNotRel);
        return RES_ERR;
    }
    average_voltage = bcu_get_average_voltage();
    if (average_voltage == 0 || average_voltage > config_get(kCapCalibLowAverageVoltage))
    {
        bcu_set_soc_empty_uncalib_reason(kSOCEmptyCalibHighAverageVolt);
        return RES_ERR;
    }
    if(bcu_get_SOC() == 0)
    {
        bcu_set_soc_empty_uncalib_reason(kSOCEmptyCalibNoTrigger);
        return RES_ERR;
    }
    soc_empty_calib_delay_start_flag = 1;
    return RES_OK;
}

void soc_reset_charging_checked_flags(void)
{
    g_soc_checked_full = 0;
    g_soc_checked_high_left_cap = 0;
    bcu_set_soc_full_uncalib_reason(kSOCFullCalibNoTrigger);
}

void soc_reset_discharging_checked_flags(void)
{
    g_soc_checked_empty = 0;
}

void soc_reset_checked_flags(void)
{
    soc_reset_charging_checked_flags();
    soc_reset_discharging_checked_flags();
}


#endif
