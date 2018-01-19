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

#define BMS_POWER_UP_FAULT_RELEASE_DELAY    100 //�ϵ��Լ���ϻָ���ʱ
#define BMS_CURRENT_AUTO_CHECK_DELAY        400 //�ϵ���HALL�Ƿ�����Ʈ
#define CURRENT_START_CHECK_TO_ZERO_CURRENT 60
#define CURRENT_START_TO_READ_MODE          60  //�����ȡģʽ����߷ŵ����

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
	/***********************BMS�ڲ�ʹ�ñ���******************************/    
    CapAms_t left_cap_interm; //���ʣ������  ��λ��1Ams
    INT32U pack_state; /*��ؼ�BMS״̬*/
    INT32U com_abort_num; //ͨ���жϱ��
    INT32U insulation_tick; // ��Եģ������
    INT32U insulation_total_voltage; //��Եģ���ϴ�����ѹ����ֵ
    INT32U insulation_pre_total_voltage; //��Եģ���ϴ���pre��ѹ����ֵ
    INT32U insulation_pch_total_voltage; //��Եģ���ϴ���pch��ѹ����ֵ
    INT32U insulation_hv3_total_voltage; //��Եģ���ϴ���hv3��ѹ����ֵ
    INT32U ext_insulation_total_voltage; //��Ӿ�Եģ���ϴ�����ѹ����ֵ
    INT32U ext_insulation_pre_total_voltage; //��Ӿ�Եģ���ϴ���pre��ѹ����ֵ
    INT32U ext_insulation_pch_total_voltage; //��Ӿ�Եģ���ϴ���pch��ѹ����ֵ
    INT32U ext_insulation_hv3_total_voltage; //��Ӿ�Եģ���ϴ���hv3��ѹ����ֵ
    INT32U alarm_check_flag; //BMS����״̬��־
    INT32U sys_exception_flags;
    INT32U power_ctl_mode_tick;
    
    INT16U hmi_bms_state; //HMI BMS״̬λ
    INT16U system_voltage; //ϵͳ��ѹ
    INT16U charge_exception_flag; // ����쳣��ע
    INT16U discharge_exception_flag; // �ŵ��쳣��־
    INT16U voltage_exception; // ��ѹ��������
    INT16U temp_exception; // �¶���������
    INT16U discharge_relay_check; // �ŵ�̵��������ʱֵ�����ڸ������Ʋ��ԣ�����̵������״̬ 
    INT16U pcm_total_voltage; //0.1v
    INT16U pcm_tv_rate_of_change; //0.1v
    INT16U custom_information[HMI_CUSTOM_INFORMATION_NUM];
    
    MCUWorkState mcu_work_state; //MCU����״̬��0:ֹͣ��1:����
    INT8U insulation_error; //��Ե���ģ�������;
    INT8U insulation_online; //��Եģ�����������0  -- ������  1---����
    INT8U ext_insulation_error; //��Ӿ�Ե���ģ�������;
    INT8U ext_insulation_online; //��Ӿ�Եģ�����������0  -- ������  1---����
    ConfigState config_state; //���ù���״̬
    INT8U config_changed_flag; //���øı��־
    INT8U full_charge_flag; //�����־ 0:δ���� 1:����
    INT8U current_auto_check_flag;  //�����Զ�У׼��־
    AlarmLevel pcm_communication_state;  //pcmͨ��״̬
    INT8U dchg_start_failure_state;
    INT8U chg_start_failure_state;
    DateTime system_time;
    SOCFullUnCalibReason soc_full_uncalib_reason;
    SOCEmptyUnCalibReason soc_empty_uncalib_reason;
    INT8U soh_calib_cur_limit_flag; //������У����־
    INT8U io_revision_alarm; //0:���� 1:Ӳ��IO�汾���� 2:���IO�汾����
    
 
	/**************************BMSͨ�ò���������*************************/
	  /*��Ҫ����*/
	  INT16U SOC; //�����SOC  ��λ��0.01%
	  INT16U SOH; //�����SOH  ��λ��0.01%
	  INT32U total_voltage; //�������ѹ  ��λ��0.001V
    INT16S current; //������ܵ���(������/�ŵ�(+/-)�ܵ���)  ��λ��0.1A
    INT16S current1;//������ܵ���2(������/�ŵ�(+/-)�ܵ���)  ��λ��0.1A
    INT16U high_voltage; //��ߵ����ѹ  ��λ��0.001V
    INT16U high_voltage_id; //��ߵ����ѹ���          
	  INT16U low_voltage; //��͵����ѹ  ��λ��0.001V    
    INT16U low_voltage_id; //��͵�ѹ���               
    INT16U average_voltage; //ƽ�������ѹ  ��λ��0.001V 
  	INT16U high_temperature; //����¶�   ��λ��1degC ,ƫ����-50    
    INT16U high_temperature_id; //����¶ȱ��         
    INT16U low_temperature; //����¶�   ��λ��1degC ,ƫ����-50         
    INT16U low_temperature_id; //����¶ȱ��          
    INT8U  average_temperature; //ƽ���¶�   ��λ��1degC ,ƫ����-50 
    //INT16U diff_temperature; // �² ƫ����-50
  	INT16U total_voltage_num;     //����ܴ���                         
    INT16U total_temperature_num; //������¶ȵ���                     
	INT16U positive_insulation_resistance; //������Ե���裬��λ 0.1KOhm 
    INT16U negative_insulation_resistance; //������Ե����,��λ��0.1KOhm 
    INT16U system_insulation_resistance;   //ϵͳ��Ե���裬��λ��0.1KOhm

    INT16U ext_positive_insulation_resistance; //���������Ե���裬��λ 0.1KOhm
    INT16U ext_negative_insulation_resistance; //��Ӹ�����Ե����,��λ��0.1Kohm
    INT16U ext_system_insulation_resistance; //���ϵͳ��Ե���裬��λ��0.1Kohm
    

  	/*������ѹ*/
  	AlarmLevel chg_delta_volt_state; //����ѹ״̬
  	AlarmLevel chg_hv_state; //��絥���ѹ
    AlarmLevel chg_lv_state; //��絥��Ƿѹ
    AlarmLevel chg_htv_state; //�����ѹ��
    AlarmLevel chg_ltv_state; //�����ѹ��
  	AlarmLevel dchg_delta_volt_state; //�ŵ��ѹ״̬
  	AlarmLevel dchg_hv_state; //�ŵ絥���ѹ
    AlarmLevel dchg_lv_state; //�ŵ絥��Ƿѹ
    AlarmLevel dchg_ltv_state; //�ŵ���ѹ��
    AlarmLevel dchg_htv_state; //�ŵ���ѹ��
    AlarmLevel chgr_oc_state; //������
    AlarmLevel dchg_oc_state; //�ŵ����
    AlarmLevel feedback_oc_state; //��������
    
  	/*SOC*/
  	AlarmLevel high_soc_state; //SOC��
    AlarmLevel low_soc_state; //SOC��

  	/*��Ե*/
  	AlarmLevel battery_insulation_state ; //��Ե��
    InsulationWorkState insulation_work_state; //��Ե״̬
    InsulationWorkState ext_insulation_work_state;

  	/*ͨѶ*/
  	AlarmLevel slave_communication_state; //���ӻ�(�ڲ�����)ͨ��״̬
  	AlarmLevel voltage_exception_state; //��ѹ�ɼ����߹���
    AlarmLevel temp_exception_state; //�¶Ȳɼ����߹���

  	/*ϵͳ*/
  	SystemState system_state; //���ϵͳ״̬

  	/*�¶�*/
  	AlarmLevel chg_delta_temp_state; //����²����
  	AlarmLevel chg_lt_state; //�����¹���
  	AlarmLevel chg_ht_state; //�����¹���
  	AlarmLevel dchg_delta_temp_state; //�ŵ��²����
  	AlarmLevel dchg_lt_state; //�ŵ���¹���
    AlarmLevel dchg_ht_state; //�ŵ���¹���
  	AlarmLevel temp_rising_state; //����״̬

  	/*�̵���*/
    RelayState charge_relay_state; //���̵���״̬  
    RelayState discharge_relay_state; //�ŵ�̵���״̬  

  	/*�ŵ�*/
  	BMS_PRECHARGE_STATE precharge_with_tv_state;  //Ԥ��״̬

  	/*���*/
    AlarmLevel chgr_ac_outlet_ht_state;// �������������״̬  
    AlarmLevel chgr_dc_outlet_ht_state;// ������������״̬  
    AlarmLevel chgr_ac_outlet_temp_exception_state; // �����¸���������  
    AlarmLevel chgr_dc_outlet_temp_exception_state; // ����¸���������  
    
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

/***********************BMS�ڲ�ʹ�ñ����ӿڶ���*****************************/
BCU_BMS_FLAG_VAR_WITH_EVENT_DEF(INT32U, pack_state, kInfoEventPackState)
BCU_BMS_VAR_DEF(INT32U, com_abort_num)
BCU_BMS_VAR_DEF(INT32U, insulation_tick)
BCU_BMS_VAR_DEF(INT32U, insulation_total_voltage) //��Եģ���ϴ�����ѹ����ֵ
BCU_BMS_VAR_DEF(INT32U, insulation_pre_total_voltage) //��Եģ���ϴ���pre��ѹ����ֵ
BCU_BMS_VAR_DEF(INT32U, insulation_pch_total_voltage) //��Եģ���ϴ���pch��ѹ����ֵ
BCU_BMS_VAR_DEF(INT32U, insulation_hv3_total_voltage)
BCU_BMS_VAR_DEF(INT32U, ext_insulation_total_voltage) //��Եģ���ϴ�����ѹ����ֵ
BCU_BMS_VAR_DEF(INT32U, ext_insulation_pre_total_voltage) //��Եģ���ϴ���pre��ѹ����ֵ
BCU_BMS_VAR_DEF(INT32U, ext_insulation_pch_total_voltage) //��Եģ���ϴ���pch��ѹ����ֵ
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
BCU_BMS_VAR_DEF(CapAms_t, left_cap_interm) //���ʣ������  ��λ��1Ams
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


/***********************BMSͨ�ò��������ӿڶ���*****************************/
/*��Ҫ����*/
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

/*������ѹ*/
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
BCU_BMS_VAR_WITH_EVENT_DEF(AlarmLevel, chgr_oc_state, kInfoEventBatteryOverChargeCurrentState) //������״̬
BCU_BMS_VAR_WITH_EVENT_DEF(AlarmLevel, dchg_oc_state, kInfoEventBatteryOverDischargeCurrentState) //�ŵ����״̬
BCU_BMS_VAR_WITH_EVENT_DEF(AlarmLevel, feedback_oc_state, kInfoEventBatteryOverChargeCurrentState)

/*SOC*/
BCU_BMS_VAR_WITH_EVENT_DEF(AlarmLevel, high_soc_state, kInfoEventBatterySocState) 
BCU_BMS_VAR_WITH_EVENT_DEF(AlarmLevel, low_soc_state, kInfoEventBatterySocState)  

/*��Ե*/
BCU_BMS_VAR_DEF(InsulationWorkState, insulation_work_state) //������Ե״̬
BCU_BMS_VAR_DEF(InsulationWorkState, ext_insulation_work_state)
BCU_BMS_VAR_WITH_EVENT_DEF(AlarmLevel, battery_insulation_state, kInfoEventBatteryInsulationState) 

/*ͨѶ*/
BCU_BMS_VAR_WITH_EVENT_DEF(AlarmLevel, slave_communication_state, kInfoEventSlaveCommunicationState) //���ӻ�(�ڲ�����)ͨ��״̬
BCU_BMS_VAR_WITH_EVENT_DEF(AlarmLevel, voltage_exception_state, kInfoEventVoltageExceptionState)
BCU_BMS_VAR_WITH_EVENT_DEF(AlarmLevel, temp_exception_state, kInfoEventTempExceptionState)

/*ϵͳ*/
BCU_BMS_VAR_WITH_EVENT_DEF(SystemState, system_state, kInfoEventSystemState)

/*�¶�*/
BCU_BMS_VAR_WITH_EVENT_DEF(AlarmLevel, chg_delta_temp_state, kInfoEventBatteryDifferenceTemperatureState) //������¶�״̬
BCU_BMS_VAR_WITH_EVENT_DEF(AlarmLevel, chg_lt_state, kInfoEventBatteryLowTemperatureState) 
BCU_BMS_VAR_WITH_EVENT_DEF(AlarmLevel, chg_ht_state, kInfoEventBatteryHighTemperatureState) 
BCU_BMS_VAR_WITH_EVENT_DEF(AlarmLevel, dchg_delta_temp_state, kInfoEventBatteryDifferenceTemperatureState) 
BCU_BMS_VAR_WITH_EVENT_DEF(AlarmLevel, dchg_lt_state, kInfoEventBatteryLowTemperatureState) 
BCU_BMS_VAR_WITH_EVENT_DEF(AlarmLevel, dchg_ht_state, kInfoEventBatteryHighTemperatureState) 
BCU_BMS_VAR_DEF(AlarmLevel, temp_rising_state)

/*�̵���*/
BCU_BMS_VAR_WITH_EVENT_DEF(RelayState, charge_relay_state, kInfoEventChargeRelayState) 
BCU_BMS_VAR_WITH_EVENT_DEF(RelayState, discharge_relay_state, kInfoEventDischargeRelayState) 

/*�ŵ�*/
BCU_BMS_VAR_DEF(BMS_PRECHARGE_STATE, precharge_with_tv_state)

/*���*/
BCU_BMS_VAR_WITH_EVENT_DEF(AlarmLevel, chgr_ac_outlet_ht_state, kInfoEventChrOutletHTState)// �������������״̬ 
BCU_BMS_VAR_WITH_EVENT_DEF(AlarmLevel, chgr_dc_outlet_ht_state, kInfoEventChrOutletHTState)// ������������״̬
BCU_BMS_VAR_WITH_EVENT_DEF(AlarmLevel, chgr_ac_outlet_temp_exception_state, kInfoEventChrOutletHTState) // �����¸���������
BCU_BMS_VAR_WITH_EVENT_DEF(AlarmLevel, chgr_dc_outlet_temp_exception_state, kInfoEventChrOutletHTState) // ����¸���������



/**
 * ��ȡ�汾��Ϣ
 * @return
 */
const Version* bcu_get_version(void);

/**
 * ��ȡ�汾��Ϣ����������
 * @return
 */
const char* bcu_get_version_string(void);

/**
 * ��ȡ�¶Ȳ�
 * @return
 */
INT16U bcu_get_difference_temperature(void);

/**
 * ��ȡ������
 * @return
 */
INT16U bcu_get_charging_current(void);
INT16U bcu_get_discharging_current(void);

/**
 * �ж�BCU�Ƿ��ڳ��
 * @return
 */
BOOLEAN bcu_is_charging(void);

/**
 * �ж�BCU�Ƿ��ڷŵ�
 * @return
 */
BOOLEAN bcu_is_discharging(void);

BOOLEAN bcu_is_system_failure(void);
BOOLEAN bcu_is_system_start_failure(void);

/**
 * ��ȡbms״̬λ��Ϣ
 * @return
 */
INT16U bcu_get_status_flag(void);

void bcu_alarm_enable(INT32U flag);
void bcu_alarm_disable(INT32U flag);

void bcu_charger_enable(INT32U flag);
void bcu_charger_disable(INT32U flag);
INT16U bcu_get_bat_num_in_pack(INT16U pos);//pos:��8λ���ӻ���  ��8λ���ӻ��ڵ�غ�
INT16U bcu_get_temperature_num_in_pack(INT16U pos);//pos:��8λ���ӻ���  ��8λ���ӻ��ڵ�غ�
INT16U bcu_get_voltage_with_num_in_pack(INT16U bat_num);//���ݵ����pack�еı�Ż�ȡ��Ӧ��ص�ѹ
INT8U bcu_get_temperature_with_num_in_pack(INT16U bat_num);//���ݵ����pack�еı�Ż�ȡ��Ӧ����¶�
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
INT32U bcu_get_running_time(void); // ������


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
INT16U bcu_get_current_total_voltage_num(void); //��ȡ��ǰʵ�ʼ�ص�ѹ����,��ͨ���ж�Ӱ��
INT16U bcu_get_current_total_temperature_num(void); //��ȡ��ǰʵ�ʼ���¸д���,��ͨ���ж�Ӱ��

INT8U bcu_set_system_time(DateTime* _PAGED time);
INT8U bcu_get_system_time(DateTime* _PAGED time);

INT8U bcu_config_para_cannot_restart_check(void);

INT16U bms_get_discharge_current_max(void);
INT16U bms_get_charge_current_max(void);

INT8U bcu_get_comm_exception_slave_num(void); //��ͨ���ж���ʱ

INT16U bcu_get_delta_volt_max_in_slave(void); // ��ȡ���дӻ��ڵ�ѹ�����ֵ
INT8U bcu_get_delta_temp_max_in_slave(void); // ��ȡ���дӻ��ڵ��²����ֵ

INT8U bcu_slave_self_check(void); //���дӻ���ѹ�¶���Ϣ�쳣���
PCSTR bcu_get_device_name_4_display(void);
void bcu_reset_all_can_channel(void);

HW_VER_NUM hardware_io_revision_get(void);
void hardware_revision_check(void);

INT8U bcu_get_high_heat_temperature(void); //��ȡ��߼����¶�
INT8U bcu_get_low_heat_temperature(void); //��ȡ��ͼ����¶�

AlarmLevel bcu_get_chg_oc_state(void);
void bcu_set_chg_oc_state(AlarmLevel alarm);

Result bms_discharge_relay_off_event(EventTypeId event_id, void* event_data, void* user_data);

#if BMU_CHR_OUTLET_TEMP_SUPPORT
INT16U bcu_get_chr_outlet_temperature_num(void);//��ȡ��ǰ�ܵĳ������¸���
INT8U bcu_get_high_chr_outlet_temperature(void);
#endif
Result bcu_chr_outlet_ht_alarm_event(EventTypeId event_id, void* event_data, void* user_data);

INT8U bcu_get_dchg_selfcheck_fault_num(void);   //��ȡ�ŵ�״̬�Լ������
INT8U bcu_get_chg_selfcheck_fault_num(void);    //��ȡ���״̬�Լ������
CurrentCheck bcu_get_current_check_diff(void);

#endif /* BMS_BCU_H_ */