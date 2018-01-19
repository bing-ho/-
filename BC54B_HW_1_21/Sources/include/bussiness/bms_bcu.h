/**
 *
 * Copyright (c) 2012 Ligoo Inc.
 *
 * @file bms_bcu.h
 * @brief
 * @note
 * @author
 * @date 2012-5-9
 *
 */

#ifndef BMS_BCU_H_
#define BMS_BCU_H_
#include "bms_defs.h"
#include "os_cpu.h"
#include "bms_event.h"
#include "bms_business_defs.h"
#include "bms_alert.h"
#include "bms_bmu.h"
#include "bms_soc.h"
#include "bms_job.h"
#include "bms_log.h"
#include "bms_unit.h"
#include "bms_stat_chg_time.h"
#include "bms_current.h"
#include "bms_clock.h"
#include "bms_time.h"
#include "bms_charger.h"
#include "bms_rule_stock.h"
#include "bms_byu.h"
#include "bms_current.h"
#include "adc.h"
#include "bms_system_voltage.h"
#include "bms_diagnosis_relay.h"
#include "bms_bcu_reset.h"
#include "bms_precharge_with_tv.h"
#include "bms_mlock_impl.h"

#pragma MESSAGE DISABLE C1420 // Result of function-call is ignored
#pragma MESSAGE DISABLE C4001 // Condition always FALSE

#define BMS_POWER_UP_FAULT_RELEASE_DELAY    100 //上电自检故障恢复延时
#define BMS_CURRENT_AUTO_CHECK_DELAY        400 //上电检测HALL是否有零飘
#define CURRENT_START_CHECK_TO_ZERO_CURRENT 60
#define CURRENT_START_TO_READ_MODE          60  //进入读取模式的最高放电电流

typedef enum _AlarmLevel AlarmLevel;

#define BCU_ADD_FLAG(NAME, FLAG) SET(NAME,FLAG)
#define BCU_REMOVE_FLAG(NAME, FLAG) CLR(NAME,FLAG)

#ifndef BCU_BMS_VAR_IMPL
#define BCU_BMS_VAR_DEF(TYPE, VAR) \
        TYPE bcu_get_##VAR(void); \
        void bcu_set_##VAR(TYPE value);
#else
#define BCU_BMS_VAR_DEF(TYPE, VAR) \
    TYPE bcu_get_##VAR(void) \
    { \
        OS_CPU_SR cpu_sr = 0; \
        TYPE value; \
        OS_ENTER_CRITICAL(); \
        value = g_bms_info.VAR; \
        OS_EXIT_CRITICAL(); \
        return value; \
    } \
    void bcu_set_##VAR(TYPE value) \
    { \
        OS_CPU_SR cpu_sr = 0; \
        OS_ENTER_CRITICAL(); \
        g_bms_info.VAR = value; \
        OS_EXIT_CRITICAL(); \
    }
#endif


#ifndef BCU_BMS_VAR_IMPL
#define BCU_BMS_VAR_WITH_EVENT_DEF(TYPE, VAR, EVENT) \
        TYPE bcu_get_##VAR(void); \
        void bcu_set_##VAR(TYPE value);
#else
#define BCU_BMS_VAR_WITH_EVENT_DEF(TYPE, VAR, EVENT) \
    TYPE bcu_get_##VAR(void) \
    { \
        OS_CPU_SR cpu_sr = 0; \
        TYPE value; \
        OS_ENTER_CRITICAL(); \
        value = g_bms_info.VAR; \
        OS_EXIT_CRITICAL(); \
        return value; \
    } \
    void bcu_set_##VAR(TYPE value) \
    { \
        OS_CPU_SR cpu_sr = 0; \
        OS_ENTER_CRITICAL(); \
        if (g_bms_info.VAR != value) \
        { \
            g_bms_info.VAR = value; \
            event_dispatch(EVENT, &value); \
        } \
        OS_EXIT_CRITICAL(); \
    }
#endif

#ifndef BCU_BMS_VAR_IMPL
#define BCU_BMS_FLAG_VAR_DEF(TYPE, VAR) \
        BCU_BMS_VAR_DEF(TYPE, VAR) \
        void bcu_flag_##VAR(TYPE flag); \
        void bcu_unflag_##VAR(TYPE flag);
#else
#define BCU_BMS_FLAG_VAR_DEF(TYPE, VAR) \
        BCU_BMS_VAR_DEF(TYPE, VAR) \
        void bcu_flag_##VAR(TYPE flag) \
        { \
            OS_CPU_SR cpu_sr = 0; \
            OS_ENTER_CRITICAL(); \
            g_bms_info.VAR |= flag; \
            OS_EXIT_CRITICAL(); \
        } \
        void bcu_unflag_##VAR(TYPE flag) \
        { \
            OS_CPU_SR cpu_sr = 0; \
            OS_ENTER_CRITICAL(); \
            g_bms_info.VAR &= ~(flag); \
            OS_EXIT_CRITICAL(); \
        }
#endif

#ifndef BCU_BMS_VAR_IMPL
#define BCU_BMS_FLAG_VAR_WITH_EVENT_DEF(TYPE, VAR, EVENT) \
        BCU_BMS_VAR_DEF(TYPE, VAR) \
        void bcu_flag_##VAR(TYPE flag); \
        void bcu_unflag_##VAR(TYPE flag);
#else
#define BCU_BMS_FLAG_VAR_WITH_EVENT_DEF(TYPE, VAR, EVENT) \
        BCU_BMS_VAR_DEF(TYPE, VAR) \
        void bcu_flag_##VAR(TYPE flag) \
        { \
            OS_CPU_SR cpu_sr = 0; \
            TYPE value; \
            OS_ENTER_CRITICAL(); \
            if((g_bms_info.VAR & flag) != flag)\
            {\
                g_bms_info.VAR |= flag; \
                value = g_bms_info.VAR; \
                event_dispatch(EVENT, &value);\
            }\
            OS_EXIT_CRITICAL(); \
        } \
        void bcu_unflag_##VAR(TYPE flag) \
        { \
            OS_CPU_SR cpu_sr = 0; \
            TYPE value; \
            OS_ENTER_CRITICAL(); \
            if((g_bms_info.VAR & flag) == flag)\
            {\
                g_bms_info.VAR &= ~(flag); \
                value = g_bms_info.VAR; \
                event_dispatch(EVENT, &value);\
            }\
            OS_EXIT_CRITICAL(); \
        }
#endif

#ifndef BCU_BMS_VAR_IMPL
#define BCU_BMS_VAR_TEST_DEF(TYPE, VAR) \
        TYPE bcu_get_##VAR(void); \
        void bcu_set_##VAR(TYPE value);
#else
#define BCU_BMS_VAR_TEST_DEF(TYPE, VAR) \
    TYPE bcu_get_##VAR(void) \
    { \
        INT32U last, duration, index; \
        INT8U num; \
        last = g_test_##VAR[0]; \
        if (last == 0) \
        {last = get_tick_count();g_test_##VAR[0] = last;} \
        duration = get_elapsed_tick_count(last); \
        index = duration / g_test_##VAR[1] ; \
        num = sizeof(g_test_##VAR) / sizeof(INT32U); \
        return g_test_##VAR[(index % (num - 2)) + 2]; \
    } \
    void bcu_set_##VAR(TYPE value) \
    { \
    }
#endif


typedef enum _HW_VER_NUM
{
    HW_VER_BELOW_115    = 0,
    HW_VER_115          = 1,
    HW_VER_116          = 2,
    HW_VER_120          = 3,
    HW_VER_BYU_110      = 4,
    HW_VER_BYU_111      = 5 
}HW_VER_NUM;

typedef enum
{
    /** Basic */
    kInfoEventNone = kInfoEventStart,
    kInfoEventMcuWorkState,

    kInfoEventTotalVoltage,
    kInfoEventCurrent,
    kInfoEventSOC,
    kInfoEventSOH,
    kInfoEventLeftCapInterm,
    kInfoEventAverageVoltage,
    kInfoEventHighVoltage,
    kInfoEventHighVoltageId,
    kInfoEventLowVoltage,
    kInfoEventLowVoltageId,
    kInfoEventHighTemperature,
    kInfoEventHighTemperatureId,
    kInfoEventLowTemperature,
    kInfoEventLowTemperatureId,
    kInfoEventPackState,
    kInfoEventComAbortNum,
    kInfoEventPositiveInsulationResistance,
    kInfoEventNegativeInsulationResistance,
    kInfoEventSystemInsulationResistance,
    kInfoEventInsulationState,
    kInfoEventInsulationError,
    kInfoEventInsulationOnline,
    kInfoEventInsulationTick,
    kInfoEventHmiBmsState,
    kInfoEventAlarmCheckFlag,
    kInfoEventSystemState,
    kInfoEventConfigState,
    kInfoEventConfigChangedFlag,
    kInfoEventBatteryChargeState,
    kInfoEventBatteryDischargeState,
    kInfoEventBatterySocState,
    kInfoEventBatteryHighTemperatureState,
    kInfoEventBatteryDifferenceTemperatureState,
    kInfoEventBatteryOverChargeCurrentState,
    kInfoEventBatteryOverDischargeCurrentState,
    kInfoEventBatteryInsulationState,
    kInfoEventInsulationTotalVoltage,
    kInfoEventStatusChangedFlag,
    kInfoEventSlaveCommunicationState,
    kInfoEventSystemVoltageState,
    kInfoEventSystemVoltage,

    kInfoEventChargeRelayState,
    kInfoEventDischargeRelayState,

    kInfoEventBatteryLowTemperatureState,
    kInfoEventDifferenceVoltageState,

    kInfoEventTempExceptionState,
    kInfoEventVoltageExceptionState,

    kInfoEventTotalVoltageState,

    kInfoEventSystemStartup,
    kInfoEventChrOutletHTState,
}InfoEventType;

typedef struct
{
  INT16S diff0; 
  INT16S diff1; 
} CurrentCheck;

typedef struct
{
	/***********************BMS内部使用变量******************************/    
    CapAms_t left_cap_interm; //电池剩余容量  单位：1Ams
    INT32U pack_state; /*电池及BMS状态*/
    INT32U com_abort_num; //通信中断编号
    INT32U insulation_tick; // 绝缘模块心跳
    INT32U insulation_total_voltage; //绝缘模块上传的总压数据值
    INT32U insulation_pre_total_voltage; //绝缘模块上传的pre总压数据值
    INT32U insulation_pch_total_voltage; //绝缘模块上传的pch总压数据值
    INT32U insulation_hv3_total_voltage; //绝缘模块上传的hv3总压数据值
    INT32U ext_insulation_total_voltage; //外接绝缘模块上传的总压数据值
    INT32U ext_insulation_pre_total_voltage; //外接绝缘模块上传的pre总压数据值
    INT32U ext_insulation_pch_total_voltage; //外接绝缘模块上传的pch总压数据值
    INT32U ext_insulation_hv3_total_voltage; //外接绝缘模块上传的hv3总压数据值
    INT32U alarm_check_flag; //BMS报警状态标志
    INT32U sys_exception_flags;
    INT32U power_ctl_mode_tick;
    
    INT16U hmi_bms_state; //HMI BMS状态位
    INT16U system_voltage; //系统电压
    INT16U charge_exception_flag; // 充电异常标注
    INT16U discharge_exception_flag; // 放电异常标志
    INT16U voltage_exception; // 电压排线脱落
    INT16U temp_exception; // 温度排线脱落
    INT16U discharge_relay_check; // 放电继电器检查临时值，用于辅助控制策略，缓存继电器检查状态 
    INT16U pcm_total_voltage; //0.1v
    INT16U pcm_tv_rate_of_change; //0.1v
    INT16U custom_information[HMI_CUSTOM_INFORMATION_NUM];
    
    MCUWorkState mcu_work_state; //MCU工作状态，0:停止，1:工作
    INT8U insulation_error; //绝缘检测模块故障码;
    INT8U insulation_online; //绝缘模块在线情况，0  -- 不在线  1---在线
    INT8U ext_insulation_error; //外接绝缘检测模块故障码;
    INT8U ext_insulation_online; //外接绝缘模块在线情况，0  -- 不在线  1---在线
    ConfigState config_state; //配置功能状态
    INT8U config_changed_flag; //配置改变标志
    INT8U full_charge_flag; //满充标志 0:未满充 1:满充
    INT8U current_auto_check_flag;  //电流自动校准标志
    AlarmLevel pcm_communication_state;  //pcm通信状态
    INT8U dchg_start_failure_state;
    INT8U chg_start_failure_state;
    DateTime system_time;
    SOCFullUnCalibReason soc_full_uncalib_reason;
    SOCEmptyUnCalibReason soc_empty_uncalib_reason;
    INT8U soh_calib_cur_limit_flag; //总容量校正标志
    INT8U io_revision_alarm; //0:正常 1:硬件IO版本过高 2:软件IO版本过高
    
 
	/**************************BMS通用参数变量集*************************/
	  /*主要参数*/
	  INT16U SOC; //电池组SOC  单位：0.01%
	  INT16U SOH; //电池组SOH  单位：0.01%
	  INT32U total_voltage; //电池组总压  单位：0.001V
    INT16S current; //电池组总电流(电池组充/放电(+/-)总电流)  单位：0.1A
    INT16S current1;//电池组总电流2(电池组充/放电(+/-)总电流)  单位：0.1A
    INT16U high_voltage; //最高单体电压  单位：0.001V
    INT16U high_voltage_id; //最高单体电压编号          
	  INT16U low_voltage; //最低单体电压  单位：0.001V    
    INT16U low_voltage_id; //最低电压编号               
    INT16U average_voltage; //平均单体电压  单位：0.001V 
  	INT16U high_temperature; //最高温度   单位：1degC ,偏移量-50    
    INT16U high_temperature_id; //最高温度编号         
    INT16U low_temperature; //最低温度   单位：1degC ,偏移量-50         
    INT16U low_temperature_id; //最低温度编号          
    INT8U  average_temperature; //平均温度   单位：1degC ,偏移量-50 
    //INT16U diff_temperature; // 温差， 偏移量-50
  	INT16U total_voltage_num;     //电池总串数                         
    INT16U total_temperature_num; //电池总温度点数                     
	INT16U positive_insulation_resistance; //正极绝缘电阻，单位 0.1KOhm 
    INT16U negative_insulation_resistance; //负极绝缘电阻,单位：0.1KOhm 
    INT16U system_insulation_resistance;   //系统绝缘电阻，单位：0.1KOhm

    INT16U ext_positive_insulation_resistance; //外接正极绝缘电阻，单位 0.1KOhm
    INT16U ext_negative_insulation_resistance; //外接负极绝缘电阻,单位：0.1Kohm
    INT16U ext_system_insulation_resistance; //外接系统绝缘电阻，单位：0.1Kohm
    

  	/*电流电压*/
  	AlarmLevel chg_delta_volt_state; //充电差压状态
  	AlarmLevel chg_hv_state; //充电单体过压
    AlarmLevel chg_lv_state; //充电单体欠压
    AlarmLevel chg_htv_state; //充电总压高
    AlarmLevel chg_ltv_state; //充电总压低
  	AlarmLevel dchg_delta_volt_state; //放电差压状态
  	AlarmLevel dchg_hv_state; //放电单体过压
    AlarmLevel dchg_lv_state; //放电单体欠压
    AlarmLevel dchg_ltv_state; //放电总压低
    AlarmLevel dchg_htv_state; //放电总压高
    AlarmLevel chgr_oc_state; //充电过流
    AlarmLevel dchg_oc_state; //放电过流
    AlarmLevel feedback_oc_state; //回馈过流
    
  	/*SOC*/
  	AlarmLevel high_soc_state; //SOC高
    AlarmLevel low_soc_state; //SOC低

  	/*绝缘*/
  	AlarmLevel battery_insulation_state ; //绝缘低
    InsulationWorkState insulation_work_state; //绝缘状态
    InsulationWorkState ext_insulation_work_state;

  	/*通讯*/
  	AlarmLevel slave_communication_state; //主从机(内部网络)通信状态
  	AlarmLevel voltage_exception_state; //电压采集掉线故障
    AlarmLevel temp_exception_state; //温度采集掉线故障

  	/*系统*/
  	SystemState system_state; //电池系统状态

  	/*温度*/
  	AlarmLevel chg_delta_temp_state; //充电温差故障
  	AlarmLevel chg_lt_state; //充电低温故障
  	AlarmLevel chg_ht_state; //充电高温故障
  	AlarmLevel dchg_delta_temp_state; //放电温差故障
  	AlarmLevel dchg_lt_state; //放电低温故障
    AlarmLevel dchg_ht_state; //放电高温故障
  	AlarmLevel temp_rising_state; //温升状态

  	/*继电器*/
    RelayState charge_relay_state; //充电继电器状态  
    RelayState discharge_relay_state; //放电继电器状态  

  	/*放电*/
  	BMS_PRECHARGE_STATE precharge_with_tv_state;  //预充状态

  	/*充电*/
    AlarmLevel chgr_ac_outlet_ht_state;// 慢充充电插座高温状态  
    AlarmLevel chgr_dc_outlet_ht_state;// 快充充电插座高温状态  
    AlarmLevel chgr_ac_outlet_temp_exception_state; // 慢充温感排线脱落  
    AlarmLevel chgr_dc_outlet_temp_exception_state; // 快充温感排线脱落  
    
} BmsInfo;

typedef enum
{
    CHARGE_NORM = 0,
    CHARGE_FULL
}FULL_CHARGE_STATE;

#pragma push
#pragma DATA_SEG __RPAGE_SEG PAGED_RAM_EMS_VAR

#ifndef BCU_BMS_VAR_IMPL
extern BmsInfo __far g_bms_info;
#else
BmsInfo __far g_bms_info = {0};
#endif

#pragma pop
#pragma DATA_SEG __RPAGE_SEG DEFAULT


/**********************************************
 *
 * Parameters
 *
 ***********************************************/
BmsInfo* __far bcu_get_bms_info(void);

/***********************BMS内部使用变量接口定义*****************************/
BCU_BMS_FLAG_VAR_WITH_EVENT_DEF(INT32U, pack_state, kInfoEventPackState)
BCU_BMS_VAR_DEF(INT32U, com_abort_num)
BCU_BMS_VAR_DEF(INT32U, insulation_tick)
BCU_BMS_VAR_DEF(INT32U, insulation_total_voltage) //绝缘模块上传的总压数据值
BCU_BMS_VAR_DEF(INT32U, insulation_pre_total_voltage) //绝缘模块上传的pre总压数据值
BCU_BMS_VAR_DEF(INT32U, insulation_pch_total_voltage) //绝缘模块上传的pch总压数据值
BCU_BMS_VAR_DEF(INT32U, insulation_hv3_total_voltage)
BCU_BMS_VAR_DEF(INT32U, ext_insulation_total_voltage) //绝缘模块上传的总压数据值
BCU_BMS_VAR_DEF(INT32U, ext_insulation_pre_total_voltage) //绝缘模块上传的pre总压数据值
BCU_BMS_VAR_DEF(INT32U, ext_insulation_pch_total_voltage) //绝缘模块上传的pch总压数据值
BCU_BMS_VAR_DEF(INT32U, ext_insulation_hv3_total_voltage)
BCU_BMS_FLAG_VAR_WITH_EVENT_DEF(INT32U, alarm_check_flag, kInfoEventAlarmCheckFlag)
BCU_BMS_FLAG_VAR_DEF(INT32U, sys_exception_flags)
BCU_BMS_VAR_DEF(INT32U, power_ctl_mode_tick)

BCU_BMS_VAR_DEF(INT16U, hmi_bms_state)
BCU_BMS_VAR_DEF(INT16U, system_voltage)
BCU_BMS_VAR_DEF(INT16U, charge_exception_flag)
BCU_BMS_VAR_DEF(INT16U, discharge_exception_flag)
BCU_BMS_VAR_DEF(INT16U, discharge_relay_check)
BCU_BMS_VAR_DEF(INT16U, voltage_exception)
BCU_BMS_VAR_DEF(INT16U, temp_exception)
BCU_BMS_VAR_DEF(INT16U, pcm_total_voltage)
BCU_BMS_VAR_DEF(INT16U, pcm_tv_rate_of_change)

BCU_BMS_VAR_WITH_EVENT_DEF(MCUWorkState, mcu_work_state, kInfoEventMcuWorkState)
BCU_BMS_VAR_DEF(CapAms_t, left_cap_interm) //电池剩余容量  单位：1Ams
BCU_BMS_VAR_DEF(INT8U, insulation_error)
BCU_BMS_VAR_DEF(INT8U, insulation_online)
BCU_BMS_VAR_DEF(INT8U, ext_insulation_error)
BCU_BMS_VAR_DEF(INT8U, ext_insulation_online)
BCU_BMS_VAR_DEF(ConfigState, config_state)
BCU_BMS_VAR_DEF(INT8U, config_changed_flag)
BCU_BMS_VAR_DEF(INT8U, full_charge_flag)
BCU_BMS_VAR_DEF(INT8U, current_auto_check_flag)
BCU_BMS_VAR_DEF(AlarmLevel, pcm_communication_state)
BCU_BMS_VAR_DEF(INT8U, dchg_start_failure_state)
BCU_BMS_VAR_DEF(INT8U, chg_start_failure_state)
BCU_BMS_VAR_DEF(SOCFullUnCalibReason, soc_full_uncalib_reason)
BCU_BMS_VAR_DEF(SOCEmptyUnCalibReason, soc_empty_uncalib_reason)
BCU_BMS_VAR_DEF(INT8U, soh_calib_cur_limit_flag)
BCU_BMS_VAR_DEF(INT8U, io_revision_alarm)


/***********************BMS通用参数变量接口定义*****************************/
/*主要参数*/
BCU_BMS_VAR_WITH_EVENT_DEF(INT16U, SOC, kInfoEventSOC)
BCU_BMS_VAR_WITH_EVENT_DEF(INT16U, SOH, kInfoEventSOH)
BCU_BMS_VAR_DEF(INT32U, total_voltage)
BCU_BMS_VAR_DEF(INT16S, current) 
BCU_BMS_VAR_DEF(INT16S, current1)
BCU_BMS_VAR_DEF(INT16U, high_voltage)										  
BCU_BMS_VAR_DEF(INT16U, high_voltage_id)									  
BCU_BMS_VAR_DEF(INT16U, low_voltage)										   
BCU_BMS_VAR_DEF(INT16U, low_voltage_id) 									   
BCU_BMS_VAR_DEF(INT16U, average_voltage) 
BCU_BMS_VAR_DEF(INT16U, high_temperature)									   
BCU_BMS_VAR_DEF(INT16U, high_temperature_id)
BCU_BMS_VAR_DEF(INT16U, low_temperature)									   
BCU_BMS_VAR_DEF(INT16U, low_temperature_id) 								   
BCU_BMS_VAR_DEF(INT8U, average_temperature) 
//BCU_BMS_VAR_DEF(INT16U, diff_temperature)
BCU_BMS_VAR_DEF(INT16U, total_voltage_num)                                           
BCU_BMS_VAR_DEF(INT16U, total_temperature_num)                                  
BCU_BMS_VAR_DEF(INT16U, positive_insulation_resistance)                        
BCU_BMS_VAR_DEF(INT16U, negative_insulation_resistance)                        
BCU_BMS_VAR_DEF(INT16U, system_insulation_resistance)                          
BCU_BMS_VAR_DEF(INT16U, ext_positive_insulation_resistance)
BCU_BMS_VAR_DEF(INT16U, ext_negative_insulation_resistance)
BCU_BMS_VAR_DEF(INT16U, ext_system_insulation_resistance)

/*电流电压*/
BCU_BMS_VAR_WITH_EVENT_DEF(AlarmLevel, chg_delta_volt_state, kInfoEventDifferenceVoltageState) 
BCU_BMS_VAR_WITH_EVENT_DEF(AlarmLevel, chg_hv_state, kInfoEventBatteryChargeState)
BCU_BMS_VAR_WITH_EVENT_DEF(AlarmLevel, chg_lv_state, kInfoEventBatteryDischargeState) 
BCU_BMS_VAR_WITH_EVENT_DEF(AlarmLevel, chg_htv_state, kInfoEventTotalVoltageState) 
BCU_BMS_VAR_WITH_EVENT_DEF(AlarmLevel, chg_ltv_state, kInfoEventTotalVoltageState) 
BCU_BMS_VAR_WITH_EVENT_DEF(AlarmLevel, dchg_delta_volt_state, kInfoEventDifferenceVoltageState)
BCU_BMS_VAR_WITH_EVENT_DEF(AlarmLevel, dchg_hv_state, kInfoEventBatteryChargeState) 
BCU_BMS_VAR_WITH_EVENT_DEF(AlarmLevel, dchg_lv_state, kInfoEventBatteryDischargeState)
BCU_BMS_VAR_WITH_EVENT_DEF(AlarmLevel, dchg_htv_state, kInfoEventTotalVoltageState)
BCU_BMS_VAR_WITH_EVENT_DEF(AlarmLevel, dchg_ltv_state, kInfoEventTotalVoltageState) 
BCU_BMS_VAR_WITH_EVENT_DEF(AlarmLevel, chgr_oc_state, kInfoEventBatteryOverChargeCurrentState) //充电过流状态
BCU_BMS_VAR_WITH_EVENT_DEF(AlarmLevel, dchg_oc_state, kInfoEventBatteryOverDischargeCurrentState) //放电过流状态
BCU_BMS_VAR_WITH_EVENT_DEF(AlarmLevel, feedback_oc_state, kInfoEventBatteryOverChargeCurrentState)

/*SOC*/
BCU_BMS_VAR_WITH_EVENT_DEF(AlarmLevel, high_soc_state, kInfoEventBatterySocState) 
BCU_BMS_VAR_WITH_EVENT_DEF(AlarmLevel, low_soc_state, kInfoEventBatterySocState)  

/*绝缘*/
BCU_BMS_VAR_DEF(InsulationWorkState, insulation_work_state) //电池组绝缘状态
BCU_BMS_VAR_DEF(InsulationWorkState, ext_insulation_work_state)
BCU_BMS_VAR_WITH_EVENT_DEF(AlarmLevel, battery_insulation_state, kInfoEventBatteryInsulationState) 

/*通讯*/
BCU_BMS_VAR_WITH_EVENT_DEF(AlarmLevel, slave_communication_state, kInfoEventSlaveCommunicationState) //主从机(内部网络)通信状态
BCU_BMS_VAR_WITH_EVENT_DEF(AlarmLevel, voltage_exception_state, kInfoEventVoltageExceptionState)
BCU_BMS_VAR_WITH_EVENT_DEF(AlarmLevel, temp_exception_state, kInfoEventTempExceptionState)

/*系统*/
BCU_BMS_VAR_WITH_EVENT_DEF(SystemState, system_state, kInfoEventSystemState)

/*温度*/
BCU_BMS_VAR_WITH_EVENT_DEF(AlarmLevel, chg_delta_temp_state, kInfoEventBatteryDifferenceTemperatureState) //电池组温度状态
BCU_BMS_VAR_WITH_EVENT_DEF(AlarmLevel, chg_lt_state, kInfoEventBatteryLowTemperatureState) 
BCU_BMS_VAR_WITH_EVENT_DEF(AlarmLevel, chg_ht_state, kInfoEventBatteryHighTemperatureState) 
BCU_BMS_VAR_WITH_EVENT_DEF(AlarmLevel, dchg_delta_temp_state, kInfoEventBatteryDifferenceTemperatureState) 
BCU_BMS_VAR_WITH_EVENT_DEF(AlarmLevel, dchg_lt_state, kInfoEventBatteryLowTemperatureState) 
BCU_BMS_VAR_WITH_EVENT_DEF(AlarmLevel, dchg_ht_state, kInfoEventBatteryHighTemperatureState) 
BCU_BMS_VAR_DEF(AlarmLevel, temp_rising_state)

/*继电器*/
BCU_BMS_VAR_WITH_EVENT_DEF(RelayState, charge_relay_state, kInfoEventChargeRelayState) 
BCU_BMS_VAR_WITH_EVENT_DEF(RelayState, discharge_relay_state, kInfoEventDischargeRelayState) 

/*放电*/
BCU_BMS_VAR_DEF(BMS_PRECHARGE_STATE, precharge_with_tv_state)

/*充电*/
BCU_BMS_VAR_WITH_EVENT_DEF(AlarmLevel, chgr_ac_outlet_ht_state, kInfoEventChrOutletHTState)// 慢充充电插座高温状态 
BCU_BMS_VAR_WITH_EVENT_DEF(AlarmLevel, chgr_dc_outlet_ht_state, kInfoEventChrOutletHTState)// 快充充电插座高温状态
BCU_BMS_VAR_WITH_EVENT_DEF(AlarmLevel, chgr_ac_outlet_temp_exception_state, kInfoEventChrOutletHTState) // 慢充温感排线脱落
BCU_BMS_VAR_WITH_EVENT_DEF(AlarmLevel, chgr_dc_outlet_temp_exception_state, kInfoEventChrOutletHTState) // 快充温感排线脱落



/**
 * 获取版本信息
 * @return
 */
const Version* bcu_get_version(void);

/**
 * 获取版本信息的文字描述
 * @return
 */
const char* bcu_get_version_string(void);

/**
 * 获取温度差
 * @return
 */
INT16U bcu_get_difference_temperature(void);

/**
 * 获取充电电流
 * @return
 */
INT16U bcu_get_charging_current(void);
INT16U bcu_get_discharging_current(void);

/**
 * 判断BCU是否在充电
 * @return
 */
BOOLEAN bcu_is_charging(void);

/**
 * 判断BCU是否在放电
 * @return
 */
BOOLEAN bcu_is_discharging(void);

BOOLEAN bcu_is_system_failure(void);
BOOLEAN bcu_is_system_start_failure(void);

/**
 * 获取bms状态位信息
 * @return
 */
INT16U bcu_get_status_flag(void);

void bcu_alarm_enable(INT32U flag);
void bcu_alarm_disable(INT32U flag);

void bcu_charger_enable(INT32U flag);
void bcu_charger_disable(INT32U flag);
INT16U bcu_get_bat_num_in_pack(INT16U pos);//pos:高8位：从机号  低8位：从机内电池号
INT16U bcu_get_temperature_num_in_pack(INT16U pos);//pos:高8位：从机号  低8位：从机内电池号
INT16U bcu_get_voltage_with_num_in_pack(INT16U bat_num);//根据电池在pack中的编号获取对应电池电压
INT8U bcu_get_temperature_with_num_in_pack(INT16U bat_num);//根据电池在pack中的编号获取对应电池温度
/**********************************************
 *
 * Main Function
 *
 ***********************************************/
void bcu_init(void);
void bcu_unint(void);
void bcu_update(void* data);
/**********************************************
 *
 * Observers Function
 *
 ***********************************************/
typedef void (*BcuInfoChangedHandler)(INT16U index, void* value, void* user_data);
void bcu_info_observe(INT16U index, BcuInfoChangedHandler handler, void* user_data);
void bcu_info_unobserve(INT16U index, BcuInfoChangedHandler handler);
void bcu_info_notify_changed(INT16U index, void* value);

INT16U bcu_get_difference_voltage(void);

/**********************************************
 *
 * Main Function
 *
 ***********************************************/


/**********************************************
 *
 * Inside Function
 *
 ***********************************************/
void bcu_load_config(void);

void bcu_update_config(void);
void bcu_update_soc(void);
void bcu_check_insulation(void);
void bcu_update_communication(void);
INT32U bcu_get_running_time(void); // 返回秒


void start_bcu_insu_heart_beat_tick(void);
void bcu_insu_heart_beat_tick_update(void);
void start_bcu_pcm_heart_beat_tick(void);
void bcu_pcm_heart_beat_tick_update(void);
BOOLEAN insu_type_is_rs485(void);
BOOLEAN insu_type_is_can(void);
BOOLEAN insu_type_is_on_board(void);
BOOLEAN insu_type_is_both(void);
BOOLEAN insu_type_is_none(void);
INT8U bms_ocv_soc_update_soc(void);

void bcu_check_current_auto_check(void* data);
void bcu_check_current_auto_to_zero(void* data);
void bcu_ChargerLock_StateUpdate(void* data);

INT16U bcu_get_custom_information(INT16U index);
INT8U bcu_set_custom_information(INT16U index, INT16U value);

INT16U bcu_get_voltage_num(void);
INT16U bcu_get_temperature_num(void);
INT16U bcu_get_heat_temperature_num(void);
INT16U bcu_get_slave_voltage_num_max(void);
INT16U bcu_get_slave_temperature_num_max(void);

AlarmLevel bcu_get_charge_state(void);
AlarmLevel bcu_get_discharge_state(void);
AlarmLevel bcu_get_high_current_state(void);
AlarmLevel bcu_get_high_temperature_state(void);
AlarmLevel bcu_get_low_temperature_state(void);
AlarmLevel bcu_get_delta_temperature_state(void);
AlarmLevel bcu_get_delta_voltage_state(void);
AlarmLevel bcu_get_high_total_volt_state(void);
AlarmLevel bcu_get_low_total_volt_state(void);
AlarmLevel bcu_get_chgr_outlet_high_temperature_state(void);
AlarmLevel bcu_get_chgr_outlet_temp_line_state(void);
INT16U bcu_get_current_total_voltage_num(void); //获取当前实际监控电压串数,受通信中断影响
INT16U bcu_get_current_total_temperature_num(void); //获取当前实际监控温感串数,受通信中断影响

INT8U bcu_set_system_time(DateTime* _PAGED time);
INT8U bcu_get_system_time(DateTime* _PAGED time);

INT8U bcu_config_para_cannot_restart_check(void);

INT16U bms_get_discharge_current_max(void);
INT16U bms_get_charge_current_max(void);

INT8U bcu_get_comm_exception_slave_num(void); //无通信中断延时

INT16U bcu_get_delta_volt_max_in_slave(void); // 获取所有从机内的压差最大值
INT8U bcu_get_delta_temp_max_in_slave(void); // 获取所有从机内的温差最大值

INT8U bcu_slave_self_check(void); //所有从机电压温度信息异常检查
PCSTR bcu_get_device_name_4_display(void);
void bcu_reset_all_can_channel(void);

HW_VER_NUM hardware_io_revision_get(void);
void hardware_revision_check(void);

INT8U bcu_get_high_heat_temperature(void); //获取最高加热温度
INT8U bcu_get_low_heat_temperature(void); //获取最低加热温度

AlarmLevel bcu_get_chg_oc_state(void);
void bcu_set_chg_oc_state(AlarmLevel alarm);

Result bms_discharge_relay_off_event(EventTypeId event_id, void* event_data, void* user_data);

#if BMU_CHR_OUTLET_TEMP_SUPPORT
INT16U bcu_get_chr_outlet_temperature_num(void);//获取当前总的充电插座温感数
INT8U bcu_get_high_chr_outlet_temperature(void);
#endif
Result bcu_chr_outlet_ht_alarm_event(EventTypeId event_id, void* event_data, void* user_data);

INT8U bcu_get_dchg_selfcheck_fault_num(void);   //获取放电状态自检故障码
INT8U bcu_get_chg_selfcheck_fault_num(void);    //获取充电状态自检故障码
CurrentCheck bcu_get_current_check_diff(void);

#endif /* BMS_BCU_H_ */