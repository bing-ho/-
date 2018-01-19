/**
*
* Copyright (c) 2014 Ligoo Inc.
*
* @file bms_diagnosis_relay.c
* @brief
* @note
* @author Liwei Dong
* @date 2014-12-3
*
*/
#include "bms_diagnosis_relay.h"
#include "bms_diagnosis_adhesion.h"

#pragma MESSAGE DISABLE C4003 // Shift count converted to unsigned char
#pragma MESSAGE DISABLE C12056 // SP debug info incorrect because of optimization or inline assembler

#define RELAY_DIAGNOSIS_DCHG_HV_COND    (bcu_get_dchg_hv_state() >= kAlarmThirdLevel)
#define RELAY_DIAGNOSIS_DCHG_LV_COND    (bcu_get_dchg_lv_state() >= kAlarmThirdLevel)
#define RELAY_DIAGNOSIS_DCHG_HTV_COND   (bcu_get_dchg_htv_state() >= kAlarmThirdLevel)
#define RELAY_DIAGNOSIS_DCHG_LTV_COND   (bcu_get_dchg_ltv_state() >= kAlarmThirdLevel)
#define RELAY_DIAGNOSIS_DCHG_HT_COND    (bcu_get_dchg_ht_state() >= kAlarmThirdLevel)
#define RELAY_DIAGNOSIS_DCHG_LT_COND    (bcu_get_dchg_lt_state() >= kAlarmThirdLevel)
#define RELAY_DIAGNOSIS_DCHG_DV_COND    (bcu_get_dchg_delta_volt_state() >= kAlarmThirdLevel)
#define RELAY_DIAGNOSIS_DCHG_DT_COND    (bcu_get_dchg_delta_temp_state() >= kAlarmThirdLevel)
#define RELAY_DIAGNOSIS_DCHG_OC_COND    (bcu_get_dchg_oc_state() >= kAlarmThirdLevel)

#define RELAY_DIAGNOSIS_CHG_HV_COND     (bcu_get_chg_hv_state() >= kAlarmThirdLevel)
#define RELAY_DIAGNOSIS_CHG_LV_COND     (bcu_get_chg_lv_state() >= kAlarmThirdLevel)
#define RELAY_DIAGNOSIS_CHG_HTV_COND    (bcu_get_chg_htv_state() >= kAlarmThirdLevel)
#define RELAY_DIAGNOSIS_CHG_LTV_COND    (bcu_get_chg_ltv_state() >= kAlarmThirdLevel)
#define RELAY_DIAGNOSIS_CHG_HT_COND     (bcu_get_chg_ht_state() >= kAlarmThirdLevel)||(bcu_get_chgr_outlet_high_temperature_state()>=kAlarmThirdLevel)
#define RELAY_DIAGNOSIS_CHG_LT_COND     (bcu_get_chg_lt_state() >= kAlarmThirdLevel)
#define RELAY_DIAGNOSIS_CHG_DV_COND     (bcu_get_chg_delta_volt_state() >= kAlarmThirdLevel)
#define RELAY_DIAGNOSIS_CHG_DT_COND     (bcu_get_chg_delta_temp_state() >= kAlarmThirdLevel)
#define RELAY_DIAGNOSIS_CHG_OC_COND     (bcu_get_chg_oc_state() >= kAlarmThirdLevel)

#define RELAY_DIAGNOSIS_HSOC_COND       (bcu_get_high_soc_state() >= kAlarmThirdLevel)
#define RELAY_DIAGNOSIS_LSOC_COND       (bcu_get_low_soc_state() >= kAlarmThirdLevel)
#define RELAY_DIAGNOSIS_VOLT_LINE_COND  (bcu_get_voltage_exception_state() >= kAlarmFirstLevel)
#define RELAY_DIAGNOSIS_TEMP_LINE_COND  ((bcu_get_temp_exception_state() >= kAlarmFirstLevel)||(bcu_get_chgr_outlet_temp_line_state()>kAlarmFirstLevel))
#define RELAY_DIAGNOSIS_INSU_LEAK_COND  (bcu_get_battery_insulation_state() >= kAlarmThirdLevel)
#define RELAY_DIAGNOSIS_COMM_ABORT_COND (bcu_get_slave_communication_state() >= kAlarmFirstLevel)

#define RELAY_BASE_CONDITION_MASK_SET(RELAY_ID, BIT) \
    g_relay_diagnose_context[RELAY_ID].base_cond_mask |= (1UL << (BIT-kRelayBaseCondStart))
#define RELAY_SELFCHECK_CONDITION_MASK_SET(RELAY_ID, BIT) \
    g_relay_diagnose_context[RELAY_ID].selfcheck_cond_mask |= (1UL << (BIT-kRelayPwrOnCondStart))
#define RELAY_RUNTIME_CONDITION_MASK_SET(RELAY_ID, BIT) \
    g_relay_diagnose_context[RELAY_ID].runtime_cond_mask |= (1UL << (BIT-kRelayRunCondStart))

#define RELAY_BASE_COND_NOT_REL_MASK_SET(RELAY_ID, BIT) \
    g_relay_diagnose_context[RELAY_ID].base_cond_not_rel_mask |= (1UL << (BIT-kRelayBaseCondStart))
#define RELAY_SELFCHECK_COND_NOT_REL_MASK_SET(RELAY_ID, BIT) \
    g_relay_diagnose_context[RELAY_ID].selfcheck_cond_not_rel_mask |= (1UL << (BIT-kRelayPwrOnCondStart))
#define RELAY_RUNTIME_COND_NOT_REL_MASK_SET(RELAY_ID, BIT) \
    g_relay_diagnose_context[RELAY_ID].runtime_cond_not_rel_mask |= (1UL << (BIT-kRelayRunCondStart))

#define RELAY_BASE_CONDITION_FAULT_UPDATE(COND) \
    if(fault_num == 0 || fault_num == i)\
    {\
        if(g_relay_diagnose_context[relay_id].base_cond_mask & (1UL << (i-kRelayBaseCondStart)))\
        {\
            if(COND)\
            {fault_num = i;fault_cnt = 0;}\
            else\
            {\
                if(!(g_relay_diagnose_context[relay_id].base_cond_not_rel_mask & (1UL << (i-kRelayBaseCondStart)))) \
                    fault_num = 0;\
                else\
                    fault_cnt = 0;\
            }\
        }\
        else\
        {\
            fault_num = 0;\
        }\
    }

#define RELAY_SELFCHECK_CONDITION_FAULT_UPDATE(COND) \
    if(fault_num == 0 || fault_num == i)\
    {\
        if(g_relay_diagnose_context[relay_id].selfcheck_cond_mask & (1UL << (i-kRelayPwrOnCondStart)))\
        {\
            if(COND)\
            {fault_num = i; fault_cnt = 0;}\
            else\
            {\
                if(fault_num == i) \
                {\
                    if(!(g_relay_diagnose_context[relay_id].selfcheck_cond_not_rel_mask & (1UL << (i-kRelayPwrOnCondStart)))) \
                        fault_num = 0;\
                    else\
                        fault_cnt = 0;\
                }\
            }\
        }\
        else\
        {\
            fault_num = 0;\
        }\
    }
#define RELAY_RUNTIME_CONDITION_FAULT_UPDATE(COND) \
    if(fault_num == 0 || fault_num == i)\
    {\
        if(g_relay_diagnose_context[relay_id].runtime_cond_mask & (1UL << (i-kRelayRunCondStart)))\
        {\
            if(COND)\
            {fault_num = i;fault_cnt = 0;}\
            else\
            {\
                if(fault_num == i) \
                {\
                    if(!(g_relay_diagnose_context[relay_id].runtime_cond_not_rel_mask & (1UL << (i-kRelayRunCondStart)))) \
                        fault_num = 0;\
                    else\
                        fault_cnt = 0;\
                }\
            }\
        }\
        else\
        {\
            fault_num = 0;\
        }\
    }

#pragma DATA_SEG __RPAGE_SEG PAGED_RAM
RelayDiagnoseContext g_relay_diagnose_context[BMS_RELAY_MAX_COUNT];
#pragma DATA_SEG DEFAULT

void bms_relay_charge_cond_mask_init(void);
void bms_relay_discharge_cond_mask_init(void);
void bms_relay_cond_check(RelayControlType type);



void bms_relay_diagnose_init(void)
{
    bms_relay_charge_cond_mask_init(); //配置充电控制条件掩码
    bms_relay_discharge_cond_mask_init();//配置放电控制条件掩码
}

void bms_relay_diagnose(void)
{
    bms_relay_cond_check(kRelayTypeCharging);
    bms_relay_cond_check(kRelayTypeDischarging);
}

INT8U bms_relay_diagnose_set_fault_num(RelayControlType type, INT8U fault)
{
    INT8U relay_id = 0;
    OS_CPU_SR cpu_sr = 0;
    
    relay_id = relay_control_get_id(type);
    if(relay_id > 0 && relay_id <= relay_count())
    {
        relay_id--;
        OS_ENTER_CRITICAL();
        g_relay_diagnose_context[relay_id].fault_num = fault;
        OS_EXIT_CRITICAL();
        return TRUE;
    }
    return FALSE;
}

INT8U bms_relay_diagnose_get_fault_num(RelayControlType type)
{
    INT8U relay_id = 0, fault = 0;
    OS_CPU_SR cpu_sr = 0;
    
    relay_id = relay_control_get_id(type);
    if(relay_id > 0 && relay_id <= relay_count())
    {
        relay_id--;
        OS_ENTER_CRITICAL();
        fault = g_relay_diagnose_context[relay_id].fault_num;
        OS_EXIT_CRITICAL();
    }
    return fault;
}

void bms_relay_charge_cond_mask_init(void)
{
    INT8U relay_id = 0;
    
    relay_id = relay_control_get_id(kRelayTypeCharging);
    if(relay_id > 0 && relay_id <= relay_count())
    {
        relay_id--;
        g_relay_diagnose_context[relay_id].fault_num = 0;
        g_relay_diagnose_context[relay_id].selfcheck_cond_mask = 0;
        /*基础条件*/
        g_relay_diagnose_context[relay_id].base_cond_mask = 0;
        RELAY_BASE_CONDITION_MASK_SET(relay_id, kRelayBaseCondCtrlEnable); //使能继电器控制使能检查
        RELAY_BASE_CONDITION_MASK_SET(relay_id, kRelayBaseCondForceCtrl); //使能继电器强制断开检查
        //RELAY_BASE_CONDITION_MASK_SET(relay_id, kRelayBaseCondNoRule); //使能继电器控制规则检查
        
        g_relay_diagnose_context[relay_id].base_cond_not_rel_mask = 0;
        /*在此添加基础异常不释放初始化*/
        //RELAY_BASE_COND_NOT_REL_MASK_SET(relay_id, kRelayBaseCondCtrlEnable); //使能继电器控制使能异常不释放
        
        /*上电自检条件*/
        g_relay_diagnose_context[relay_id].selfcheck_cond_mask = 0;
        RELAY_SELFCHECK_CONDITION_MASK_SET(relay_id, kRelayPwrOnCondSysVolt); //使能上电系统电压有效性检查
        RELAY_SELFCHECK_CONDITION_MASK_SET(relay_id, kRelayPwrOnCondVoltLine); //使能上电电压排线异常检查
        RELAY_SELFCHECK_CONDITION_MASK_SET(relay_id, kRelayPwrOnCondTempLine); //使能上电温感排线异常检查
        RELAY_SELFCHECK_CONDITION_MASK_SET(relay_id, kRelayPwrOnCondSlaveComm); //使能上电从机通信异常检查
        RELAY_SELFCHECK_CONDITION_MASK_SET(relay_id, kRelayPwrOnCondHardware); //使能上电硬件异常检查      
        RELAY_SELFCHECK_CONDITION_MASK_SET(relay_id, kRelayPwrOnCondInsuLeak); //使能上电漏电检查
       
#if BMS_DIAGNOSIS_HT_INDEPENDENT_WITH_CUR_EN
        RELAY_SELFCHECK_CONDITION_MASK_SET(relay_id, kRelayPwrOnCondChgHT); //使能上电充电温度过高检查
#else
        RELAY_SELFCHECK_CONDITION_MASK_SET(relay_id, kRelayPwrOnCondDchgHT); //使能上电放电温度过高检查
#endif      
       
#if BMS_DIAGNOSIS_HV_INDEPENDENT_WITH_CUR_EN
        RELAY_SELFCHECK_CONDITION_MASK_SET(relay_id, kRelayPwrOnCondChgHV); //使能上电充电最高单体过高检查
#else
        RELAY_SELFCHECK_CONDITION_MASK_SET(relay_id, kRelayPwrOnCondDchgHV); //使能上电放电最高单体过高检查
#endif

#if BMS_DIAGNOSIS_HTV_INDEPENDENT_WITH_CUR_EN
        RELAY_SELFCHECK_CONDITION_MASK_SET(relay_id, kRelayPwrOnCondChgHTV); //使能上电充电总压过高检查
#else
        RELAY_SELFCHECK_CONDITION_MASK_SET(relay_id, kRelayPwrOnCondDchgHTV); //使能上电放电总压过高检查
#endif

#if BMS_DIAGNOSIS_LT_INDEPENDENT_WITH_CUR_EN
        RELAY_SELFCHECK_CONDITION_MASK_SET(relay_id, kRelayPwrOnCondChgLT); //使能上电充电温度过低检查
#else
        RELAY_SELFCHECK_CONDITION_MASK_SET(relay_id, kRelayPwrOnCondDchgLT); //使能上电放电温度过低检查
#endif

#if BMS_DIAGNOSIS_HDV_INDEPENDENT_WITH_CUR_EN
        RELAY_SELFCHECK_CONDITION_MASK_SET(relay_id, kRelayPwrOnCondChgDV); //使能上电充电压差过高检查
#else
        RELAY_SELFCHECK_CONDITION_MASK_SET(relay_id, kRelayPwrOnCondDchgDV); //使能上电放电压差过高检查
#endif

#if BMS_DIAGNOSIS_HDT_INDEPENDENT_WITH_CUR_EN
        RELAY_SELFCHECK_CONDITION_MASK_SET(relay_id, kRelayPwrOnCondChgDT); //使能上电充电温差过高检查
#else
        RELAY_SELFCHECK_CONDITION_MASK_SET(relay_id, kRelayPwrOnCondDchgDT); //使能上电放电温差过高检查
#endif
        g_relay_diagnose_context[relay_id].selfcheck_cond_not_rel_mask = 0;
        /*在此添加自检异常不释放初始化*/
        //RELAY_SELFCHECK_COND_NOT_REL_MASK_SET(relay_id, kRelayPwrOnCondSysVolt); //使能上电系统电压异常不释放
        
        /*运行时条件*/
        g_relay_diagnose_context[relay_id].runtime_cond_mask = 0;
        RELAY_RUNTIME_CONDITION_MASK_SET(relay_id, kRelayRunCondVoltLine); //使能电压检测排线脱落检查
        RELAY_RUNTIME_CONDITION_MASK_SET(relay_id, kRelayRunCondTempLine); //使能温感检测排线脱落检查
        RELAY_RUNTIME_CONDITION_MASK_SET(relay_id, kRelayRunCondSlaveComm); //使能主从通信异常状态检查
        RELAY_RUNTIME_CONDITION_MASK_SET(relay_id, kRelayRunCondInsuLeak); //使能绝缘漏电状态检查
        RELAY_RUNTIME_CONDITION_MASK_SET(relay_id, kRelayRunCondFullChg); //使能满充状态检查
        RELAY_RUNTIME_CONDITION_MASK_SET(relay_id, kRelayRunCondMutex); //使能充放电互斥状态检查
        
        RELAY_RUNTIME_CONDITION_MASK_SET(relay_id, kRelayAdhesion); //使能粘连检测

        
#if BMS_DIAGNOSIS_HV_INDEPENDENT_WITH_CUR_EN
        RELAY_RUNTIME_CONDITION_MASK_SET(relay_id, kRelayRunCondChgHV); //使能充电单体高压状态检查
#else
        RELAY_RUNTIME_CONDITION_MASK_SET(relay_id, kRelayRunCondDchgHV); //使能放电单体高压状态检查
#endif

#if BMS_DIAGNOSIS_HT_INDEPENDENT_WITH_CUR_EN
        RELAY_RUNTIME_CONDITION_MASK_SET(relay_id, kRelayRunCondChgHT); //使能充电高温状态检查
#else
        RELAY_RUNTIME_CONDITION_MASK_SET(relay_id, kRelayRunCondDchgHT); //使能放电高温状态检查
#endif
            
#if BMS_DIAGNOSIS_HTV_INDEPENDENT_WITH_CUR_EN
        RELAY_RUNTIME_CONDITION_MASK_SET(relay_id, kRelayRunCondChgHTV); //使能充电总压高状态检查
#else
        RELAY_RUNTIME_CONDITION_MASK_SET(relay_id, kRelayRunCondDchgHTV); //使能放电总压高状态检查
#endif

#if BMS_DIAGNOSIS_LT_INDEPENDENT_WITH_CUR_EN
        RELAY_RUNTIME_CONDITION_MASK_SET(relay_id, kRelayRunCondChgLT); //使能充电低温状态检查
#else
        RELAY_RUNTIME_CONDITION_MASK_SET(relay_id, kRelayRunCondDchgLT); //使能放电低温状态检查
#endif

#if BMS_DIAGNOSIS_HDV_INDEPENDENT_WITH_CUR_EN
        RELAY_RUNTIME_CONDITION_MASK_SET(relay_id, kRelayRunCondChgDV); //使能充电压差状态检查
#else
        RELAY_RUNTIME_CONDITION_MASK_SET(relay_id, kRelayRunCondDchgDV); //使能放电压差状态检查
#endif

#if BMS_DIAGNOSIS_HDT_INDEPENDENT_WITH_CUR_EN
        RELAY_RUNTIME_CONDITION_MASK_SET(relay_id, kRelayRunCondChgDT); //使能充电温差状态检查
#else
        RELAY_RUNTIME_CONDITION_MASK_SET(relay_id, kRelayRunCondDchgDT); //使能放电温差状态检查
#endif

        g_relay_diagnose_context[relay_id].runtime_cond_not_rel_mask = 0;
        /*在此添加运行时异常不释放初始化*/
#if BMS_DIAGNOSIS_HV_INDEPENDENT_WITH_CUR_EN
        RELAY_RUNTIME_COND_NOT_REL_MASK_SET(relay_id, kRelayRunCondChgHV); //使能充电单体高压异常不释放
#else
        RELAY_RUNTIME_COND_NOT_REL_MASK_SET(relay_id, kRelayRunCondDchgHV); //使能放电单体高压异常不释放
#endif    

#if BMS_DIAGNOSIS_HTV_INDEPENDENT_WITH_CUR_EN
        RELAY_RUNTIME_COND_NOT_REL_MASK_SET(relay_id, kRelayRunCondChgHTV); //使能充电总压高异常不释放
#else
        RELAY_RUNTIME_COND_NOT_REL_MASK_SET(relay_id, kRelayRunCondDchgHTV); //使能放电总压高异常不释放
#endif

    }
}

void bms_relay_discharge_cond_mask_init(void)
{
    INT8U relay_id = 0;
    
    relay_id = relay_control_get_id(kRelayTypeDischarging);
    if(relay_id > 0 && relay_id <= relay_count())
    {
        relay_id--;
        g_relay_diagnose_context[relay_id].fault_num = 0;
        g_relay_diagnose_context[relay_id].selfcheck_cond_mask = 0;
        //基础条件
        g_relay_diagnose_context[relay_id].base_cond_mask = 0;
        RELAY_BASE_CONDITION_MASK_SET(relay_id, kRelayBaseCondCtrlEnable); //使能继电器控制使能检查
        RELAY_BASE_CONDITION_MASK_SET(relay_id, kRelayBaseCondForceCtrl); //使能继电器强制断开检查
        //RELAY_BASE_CONDITION_MASK_SET(relay_id, kRelayBaseCondNoRule); //使能继电器控制规则检查
        //上电自检条件
        g_relay_diagnose_context[relay_id].selfcheck_cond_mask = 0;
        RELAY_SELFCHECK_CONDITION_MASK_SET(relay_id, kRelayPwrOnCondSysVolt); //使能上电系统电压有效性检查
        RELAY_SELFCHECK_CONDITION_MASK_SET(relay_id, kRelayPwrOnCondVoltLine); //使能上电电压排线异常检查
        RELAY_SELFCHECK_CONDITION_MASK_SET(relay_id, kRelayPwrOnCondTempLine); //使能上电温感排线异常检查
        RELAY_SELFCHECK_CONDITION_MASK_SET(relay_id, kRelayPwrOnCondSlaveComm); //使能上电从机通信异常检查
        RELAY_SELFCHECK_CONDITION_MASK_SET(relay_id, kRelayPwrOnCondDchgHT); //使能上电放电温度过高检查
        RELAY_SELFCHECK_CONDITION_MASK_SET(relay_id, kRelayPwrOnCondDchgLV); //使能上电放电最低单体过低检查
        RELAY_SELFCHECK_CONDITION_MASK_SET(relay_id, kRelayPwrOnCondHardware); //使能上电硬件异常检查
        RELAY_SELFCHECK_CONDITION_MASK_SET(relay_id, kRelayPwrOnCondDchgLTV); //使能上电放电总压过低检查
        RELAY_SELFCHECK_CONDITION_MASK_SET(relay_id, kRelayPwrOnCondDchgLT); //使能上电放电温度过低检查
        //运行时条件
        g_relay_diagnose_context[relay_id].runtime_cond_mask = 0;
        RELAY_RUNTIME_CONDITION_MASK_SET(relay_id, kRelayRunCondDchgLV); //使能放电单体低压状态检查
        RELAY_RUNTIME_CONDITION_MASK_SET(relay_id, kRelayRunCondDchgHT); //使能放电高温状态检查
        RELAY_RUNTIME_CONDITION_MASK_SET(relay_id, kRelayRunCondDchgLTV); //使能放电总压低状态检查
        //RELAY_RUNTIME_CONDITION_MASK_SET(relay_id, kRelayRunCondInsuLeak); //使能绝缘漏电状态检查
        RELAY_RUNTIME_CONDITION_MASK_SET(relay_id, kRelayRunCondMutex); //使能充放电互斥状态检查
        RELAY_RUNTIME_CONDITION_MASK_SET(relay_id, kRelayRunCondPrechargeFailure); //使能预充失败状态检查
        RELAY_RUNTIME_CONDITION_MASK_SET(relay_id, kRelayRunCondPrecharging); //使能预充电中状态检查
        
        RELAY_RUNTIME_CONDITION_MASK_SET(relay_id, kRelayAdhesion); //使能粘连检测
    }
}
void bms_relay_cond_check(RelayControlType type)
{
    INT8U i, relay_id = 0, start_failure=0, condition = 0;
    INT8U fault_num, fault_cnt, flag;
    
    relay_id = relay_control_get_id(type);
    if(relay_id > 0 && relay_id <= relay_count())
    {
        //基础条件检查
        relay_id--;
        fault_num = g_relay_diagnose_context[relay_id].fault_num;
        
        if( fault_num == 0 ||
            (fault_num >= kRelayBaseCondStart &&
            fault_num <= kRelayBaseCondStop))
        {
            fault_cnt = kRelayBaseCondStop - kRelayBaseCondStart + 1;
            i = fault_num;
            while(fault_cnt--)
            {
                if(i<kRelayBaseCondStart || i>kRelayBaseCondStop) i = kRelayBaseCondStart;
                switch(i)
                {
                    case kRelayBaseCondCtrlEnable:
                        RELAY_BASE_CONDITION_FAULT_UPDATE(relay_control_enable_is_on(type) == 0)
                        break;
                    case kRelayBaseCondForceCtrl:
                        RELAY_BASE_CONDITION_FAULT_UPDATE(relay_is_force_control_off(type))
                        break;
                    case kRelayBaseCondNoRule:
                        RELAY_BASE_CONDITION_FAULT_UPDATE(rule_engine_get_rule_num() == 0)
                        break;
                }
                i++;
            }
        }
       if((fault_num >= kRelayBaseCondStart) && (fault_num <= kRelayBaseCondStop))
        {
          if(g_relay_diagnose_context[relay_id].fault_num != fault_num)
           {
                g_relay_diagnose_context[relay_id].fault_num = fault_num; 
           }
           return;
        }
        
        //上电自检条件检查
        if(type == kRelayTypeCharging)
        {   
            start_failure = bcu_get_chg_start_failure_state();
            if(start_failure == kStartFailureOccure)
            {
              fault_num =  bcu_get_chg_selfcheck_fault_num();
            }
            else
            {
                if(fault_num >= kRelayPwrOnCondStart && fault_num <= kRelayPwrOnCondStop)
                {
                    fault_num = 0;
                }
            }
        }
        else if((type == kRelayTypeDischarging))
        {
            start_failure = bcu_get_dchg_start_failure_state(); 
            if(start_failure == kStartFailureOccure)
            {
              fault_num = bcu_get_dchg_selfcheck_fault_num();
            }
            else
            {
                if(fault_num >= kRelayPwrOnCondStart && fault_num <= kRelayPwrOnCondStop)
                {
                    fault_num = 0;
                }
            }
        }
       /* if(start_failure == kStartFailureOccure)
        {
            bms_relay_selfcheck_fault_update(type, &fault_num);
        }
        else
        {
            if(fault_num >= kRelayPwrOnCondStart && fault_num <= kRelayPwrOnCondStop)
            {
                fault_num = 0;
            }
        }
        //运行时条件检查
        if((fault_num == 0 ||
            (fault_num >= kRelayRunCondStart &&
            fault_num <= kRelayRunCondStop)) == 0)
            goto End_SEG;  */
        if(start_failure == kStartFailureOccure) 
        {
           if(g_relay_diagnose_context[relay_id].fault_num != fault_num)
           {
                g_relay_diagnose_context[relay_id].fault_num = fault_num; 
           }
           return; 
        }
        if(type == kRelayTypeCharging)
        {
            condition = config_get(kRelayDischargeSamePort) && bcu_is_discharging();
        }
        else if(type == kRelayTypeDischarging)
        {
            condition = config_get(kRelayDischargeSamePort) && bcu_is_charging();
        }
        
        fault_cnt = kRelayRunCondStop - kRelayRunCondStart + 1;
		if(fault_num>=kRelayRunCondStart && fault_num<=kRelayRunCondStop) i = fault_num;
		else i = kRelayRunCondStart;
		while(fault_cnt--)
		{
			if(i<kRelayRunCondStart || i>kRelayRunCondStop) i = kRelayRunCondStop;
			switch(i)
            {
                case kRelayRunCondDchgHV:
                    RELAY_RUNTIME_CONDITION_FAULT_UPDATE(RELAY_DIAGNOSIS_DCHG_HV_COND && condition == 0)
                    break;
                case kRelayRunCondDchgLV:
                    RELAY_RUNTIME_CONDITION_FAULT_UPDATE(RELAY_DIAGNOSIS_DCHG_LV_COND && condition == 0)
                    break;
                case kRelayRunCondDchgHTV:
                    RELAY_RUNTIME_CONDITION_FAULT_UPDATE(RELAY_DIAGNOSIS_DCHG_HTV_COND && condition == 0)
                    break;
                case kRelayRunCondDchgLTV:
                    RELAY_RUNTIME_CONDITION_FAULT_UPDATE(RELAY_DIAGNOSIS_DCHG_LTV_COND && condition == 0)
                    break;
                case kRelayRunCondDchgHT:
                    RELAY_RUNTIME_CONDITION_FAULT_UPDATE(RELAY_DIAGNOSIS_DCHG_HT_COND && condition == 0)
                    break;
                case kRelayRunCondDchgLT:
                    RELAY_RUNTIME_CONDITION_FAULT_UPDATE(RELAY_DIAGNOSIS_DCHG_LT_COND && condition == 0)
                    break;
                case kRelayRunCondDchgDV:
                    RELAY_RUNTIME_CONDITION_FAULT_UPDATE(RELAY_DIAGNOSIS_DCHG_DV_COND && condition == 0)
                    break;
                case kRelayRunCondDchgDT:
                    RELAY_RUNTIME_CONDITION_FAULT_UPDATE(RELAY_DIAGNOSIS_DCHG_DT_COND && condition == 0)
                    break;
                case kRelayRunCondDchgOC:
                    RELAY_RUNTIME_CONDITION_FAULT_UPDATE(RELAY_DIAGNOSIS_DCHG_OC_COND && condition == 0)
                    break;
                case kRelayRunCondChgHV:
                    RELAY_RUNTIME_CONDITION_FAULT_UPDATE(RELAY_DIAGNOSIS_CHG_HV_COND && condition == 0)
                    break;
                case kRelayRunCondChgLV:
                    RELAY_RUNTIME_CONDITION_FAULT_UPDATE(RELAY_DIAGNOSIS_CHG_LV_COND && condition == 0)
                    break;
                case kRelayRunCondChgHTV:
                    RELAY_RUNTIME_CONDITION_FAULT_UPDATE(RELAY_DIAGNOSIS_CHG_HTV_COND && condition == 0)
                    break;
                case kRelayRunCondChgLTV:
                    RELAY_RUNTIME_CONDITION_FAULT_UPDATE(RELAY_DIAGNOSIS_CHG_LTV_COND && condition == 0)
                    break;
                case kRelayRunCondChgHT:
                    RELAY_RUNTIME_CONDITION_FAULT_UPDATE(RELAY_DIAGNOSIS_CHG_HT_COND && condition == 0)
                    break;
                case kRelayRunCondChgLT:
                    RELAY_RUNTIME_CONDITION_FAULT_UPDATE(RELAY_DIAGNOSIS_CHG_LT_COND && condition == 0) 
                    break;
                case kRelayRunCondChgDV:
                    RELAY_RUNTIME_CONDITION_FAULT_UPDATE(RELAY_DIAGNOSIS_CHG_DV_COND && condition == 0)
                    break;
                case kRelayRunCondChgDT:
                    RELAY_RUNTIME_CONDITION_FAULT_UPDATE(RELAY_DIAGNOSIS_CHG_DT_COND && condition == 0)
                    break;
                case kRelayRunCondChgOC:
                    RELAY_RUNTIME_CONDITION_FAULT_UPDATE(RELAY_DIAGNOSIS_CHG_OC_COND && condition == 0)
                    break;
                case kRelayRunCondHSOC:
                    RELAY_RUNTIME_CONDITION_FAULT_UPDATE(RELAY_DIAGNOSIS_HSOC_COND && condition == 0)
                    break;
                case kRelayRunCondLSOC:
                    RELAY_RUNTIME_CONDITION_FAULT_UPDATE(RELAY_DIAGNOSIS_LSOC_COND && condition == 0)
                    break;
                case kRelayRunCondVoltLine:
                    RELAY_RUNTIME_CONDITION_FAULT_UPDATE(RELAY_DIAGNOSIS_VOLT_LINE_COND && condition == 0)
                    break;
                case kRelayRunCondTempLine:
                    RELAY_RUNTIME_CONDITION_FAULT_UPDATE(RELAY_DIAGNOSIS_TEMP_LINE_COND && condition == 0)
                    break;
                case kRelayRunCondInsuLeak:
                    RELAY_RUNTIME_CONDITION_FAULT_UPDATE(RELAY_DIAGNOSIS_INSU_LEAK_COND && condition == 0)
                    break;
                case kRelayRunCondSlaveComm:
                    RELAY_RUNTIME_CONDITION_FAULT_UPDATE(RELAY_DIAGNOSIS_COMM_ABORT_COND && condition == 0)
                    break;
                case kRelayRunCondFullChg:
                    RELAY_RUNTIME_CONDITION_FAULT_UPDATE(bcu_get_full_charge_flag() && condition == 0)
                    break;
                case kRelayRunCondMutex:
                    if(type == kRelayTypeCharging)
                    {
                        flag = !((config_get(kRelayChargeMutex)== 0) || (charger_is_connected() != 0));
                    }
                    else
                    {
                        flag = !((config_get(kRelayChargeMutex)== 0) || (charger_is_connected() == 0));
                    }
                    RELAY_RUNTIME_CONDITION_FAULT_UPDATE(flag)
                    break;
                case kRelayRunCondPrecharging:
                    RELAY_RUNTIME_CONDITION_FAULT_UPDATE(bcu_get_precharge_with_tv_state() != kPrechargeStateFailure && 
                                                            bcu_get_discharge_relay_state() == kRelayStatusPreCharging &&
                                                            relay_control_is_on(kRelayTypePreCharging) == 1)
                    break;
                case kRelayRunCondPrechargeFailure:
                    RELAY_RUNTIME_CONDITION_FAULT_UPDATE(bcu_get_precharge_with_tv_state() == kPrechargeStateFailure)
                    break;
                case kRelayRunCondUnRelease:
                    
                    break;
                case kRelayRunCondCommAbort:
                    
                    break;
                case kRelayCondUndefine:
                    
                    break;
                case kRelayAdhesion:
                #if BMS_SUPPORT_NEGATIVE_ADHESION
                    RELAY_RUNTIME_CONDITION_FAULT_UPDATE(bms_get_relay_state(kRelayTypeNegative) == kRelayAdhesionTrouble)
                #endif
                    break;
            }
            i++;
        }
//End_SEG:
        if(g_relay_diagnose_context[relay_id].fault_num != fault_num)
            g_relay_diagnose_context[relay_id].fault_num = fault_num;
    }
}

void bms_relay_selfcheck_fault_update(RelayControlType type, INT8U* _PAGED current_fault_num)
{
    INT8U i = kRelayPwrOnCondStart, relay_id = 0, fault_num, fault_cnt;
    
    fault_num = *current_fault_num;
    if(fault_num>=kRelayPwrOnCondStart && fault_num<=kRelayPwrOnCondStop) i = *current_fault_num;
    
    relay_id = relay_control_get_id(type);
    if(relay_id > 0 && relay_id <= relay_count())
    {
        relay_id--;
        fault_cnt = kRelayPwrOnCondStop - kRelayPwrOnCondStart + 1;
        while(fault_cnt--)
        {
            if(i<kRelayPwrOnCondStart || i>kRelayPwrOnCondStop) i = kRelayPwrOnCondStart;
            switch(i)
            {
                case kRelayPwrOnCondSysVolt:
                    RELAY_SELFCHECK_CONDITION_FAULT_UPDATE(!(system_voltage_is_valid()==FALSE || system_voltage_is_usful()==TRUE))
                    break;
                case kRelayPwrOnCondVoltLine:
                    RELAY_SELFCHECK_CONDITION_FAULT_UPDATE(bcu_get_voltage_exception())
                    break;
                case kRelayPwrOnCondTempLine:
                    RELAY_SELFCHECK_CONDITION_FAULT_UPDATE(bcu_get_temp_exception()||
                                                     (BMS_DIAGNOSIS_AC_CHARGE_CONDITION() && bms_get_chgr_ac_outlet_temperature_exception())||
                                                     (BMS_DIAGNOSIS_DC_CHARGE_CONDITION() && bms_get_chgr_dc_outlet_temperature_exception()))
                    break;
                case kRelayPwrOnCondSlaveComm:
                    RELAY_SELFCHECK_CONDITION_FAULT_UPDATE(bcu_get_comm_exception_slave_num())
                    break;
                case kRelayPwrOnCondHardware:
                    RELAY_SELFCHECK_CONDITION_FAULT_UPDATE((bcu_get_sys_exception_flags() & SYS_EXCEPTION_CONFIG_WRITE_ERROR) ||
                                                    (bcu_get_sys_exception_flags() & SYS_EXCEPTION_FLASH_ERROR))
                    break;
                case kRelayPwrOnCondDchgHV:
                    RELAY_SELFCHECK_CONDITION_FAULT_UPDATE(bcu_get_high_voltage() >= config_get(kDChgHVSndAlarmIndex))
                    break;
                case kRelayPwrOnCondDchgLV:
                    RELAY_SELFCHECK_CONDITION_FAULT_UPDATE(bcu_get_low_voltage() <= config_get(kDChgLVTrdAlarmIndex))
                    break;
                case kRelayPwrOnCondDchgHTV:
                    RELAY_SELFCHECK_CONDITION_FAULT_UPDATE(bcu_get_average_voltage() >= config_get(kDChgHTVTrdAlarmIndex))
                    break;
                case kRelayPwrOnCondDchgLTV:
                    RELAY_SELFCHECK_CONDITION_FAULT_UPDATE(bcu_get_average_voltage() > 0 && bcu_get_average_voltage() <= config_get(kDChgLTVTrdAlarmIndex))
                    break;
                case kRelayPwrOnCondDchgHT:
                    RELAY_SELFCHECK_CONDITION_FAULT_UPDATE(bcu_get_high_temperature() >= config_get(kDChgHTTrdAlarmIndex))
                    break;
                case kRelayPwrOnCondDchgLT:
                    RELAY_SELFCHECK_CONDITION_FAULT_UPDATE(bcu_get_low_temperature() <= config_get(kDChgLTTrdAlarmIndex))
                    break;
                case kRelayPwrOnCondDchgDV:
                    RELAY_SELFCHECK_CONDITION_FAULT_UPDATE(bcu_get_difference_voltage() >= config_get(kDChgDVTrdAlarmIndex))
                    break;
                case kRelayPwrOnCondDchgDT:
                    RELAY_SELFCHECK_CONDITION_FAULT_UPDATE(bcu_get_difference_temperature() >= config_get(kDChgDTTrdAlarmIndex))
                    break;
                case kRelayPwrOnCondDchgOC:
                    RELAY_SELFCHECK_CONDITION_FAULT_UPDATE(bcu_get_discharging_current() >= config_get(kDChgOCTrdAlarmIndex))
                    break;
                case kRelayPwrOnCondChgHV:
                    RELAY_SELFCHECK_CONDITION_FAULT_UPDATE(bcu_get_high_voltage() >= config_get(kChgHVSndAlarmIndex))
                    break;
                case kRelayPwrOnCondChgLV:
                    RELAY_SELFCHECK_CONDITION_FAULT_UPDATE(bcu_get_low_voltage() <= config_get(kChgLVTrdAlarmIndex))
                    break;
                case kRelayPwrOnCondChgHTV:
                    RELAY_SELFCHECK_CONDITION_FAULT_UPDATE(bcu_get_average_voltage() >= config_get(kChgHTVTrdAlarmIndex))
                    break;
                case kRelayPwrOnCondChgLTV:
                    RELAY_SELFCHECK_CONDITION_FAULT_UPDATE(bcu_get_average_voltage() > 0 && bcu_get_average_voltage() <= config_get(kChgLTVTrdAlarmIndex))
                    break;
                case kRelayPwrOnCondChgHT:
                    RELAY_SELFCHECK_CONDITION_FAULT_UPDATE((bcu_get_high_temperature() >= config_get(kChgHTTrdAlarmIndex))||
                                                (BMS_DIAGNOSIS_AC_CHARGE_CONDITION() && bms_get_chgr_ac_outlet_high_temperature()>=CHG_OUTLET_HT_TRD_ALARM_DEF)||
                                                (BMS_DIAGNOSIS_DC_CHARGE_CONDITION() && bms_get_chgr_dc_outlet_high_temperature()>=CHG_OUTLET_HT_TRD_ALARM_DEF))                                        
                    break;
                case kRelayPwrOnCondChgLT:
                    RELAY_SELFCHECK_CONDITION_FAULT_UPDATE(bcu_get_low_temperature() <= config_get(kChgLTTrdAlarmIndex))
                    break;
                case kRelayPwrOnCondChgDV:
                    RELAY_SELFCHECK_CONDITION_FAULT_UPDATE(bcu_get_difference_voltage() >= config_get(kChgDVTrdAlarmIndex))
                    break;
                case kRelayPwrOnCondChgDT:
                    RELAY_SELFCHECK_CONDITION_FAULT_UPDATE(bcu_get_difference_temperature() >= config_get(kChgDTTrdAlarmIndex))
                    break;
                case kRelayPwrOnCondChgOC:
                    RELAY_SELFCHECK_CONDITION_FAULT_UPDATE(bcu_get_charging_current() >= config_get(kChgOCTrdAlarmIndex))
                    break;
                case kRelayPwrOnCondHSOC:
                    RELAY_SELFCHECK_CONDITION_FAULT_UPDATE(bcu_get_SOC() >= SOC_HIGH_TRD_ALARM_DEF)//config_get(kSOCHighTrdAlarmIndex)
                    break;
                case kRelayPwrOnCondLSOC:
                    RELAY_SELFCHECK_CONDITION_FAULT_UPDATE(bcu_get_SOC() <= config_get(kSOCLowTrdAlarmIndex))
                    break;
                case kRelayPwrOnCondInsuLeak:
                    RELAY_SELFCHECK_CONDITION_FAULT_UPDATE(bcu_get_system_insulation_resistance() <= MV_TO_V(bcu_get_insulation_total_voltage())*config_get(kInsuTrdAlarmIndex)/100)
                    break;
                case kRelayPwrOnCondUnRelease:
                    
                    break;
            }
            i++;
        }
        *current_fault_num = fault_num;
    }
}

void bms_relay_runtime_fault_update(RelayControlType type, INT8U* _PAGED current_fault_num)
{
    INT8U i = kRelayRunCondStart, relay_id = 0, fault_num, fault_cnt;
    
    fault_num = *current_fault_num;
    if(fault_num>=kRelayRunCondStart && fault_num<=kRelayRunCondStop) i = fault_num;
    
    relay_id = relay_control_get_id(type);
    if(relay_id > 0 && relay_id <= relay_count())
    {
        relay_id--;
        fault_cnt = kRelayRunCondStop - kRelayRunCondStart + 1;
        while(fault_cnt--)
        {
            if(i<kRelayRunCondStart || i>kRelayRunCondStop) i = kRelayRunCondStart;
            switch(i)
            {
                case kRelayRunCondDchgHV:
                    RELAY_RUNTIME_CONDITION_FAULT_UPDATE(RELAY_DIAGNOSIS_DCHG_HV_COND)
                    break;
                case kRelayRunCondDchgLV:
                    RELAY_RUNTIME_CONDITION_FAULT_UPDATE(RELAY_DIAGNOSIS_DCHG_LV_COND)
                    break;
                case kRelayRunCondDchgHTV:
                    RELAY_RUNTIME_CONDITION_FAULT_UPDATE(RELAY_DIAGNOSIS_DCHG_HTV_COND)
                    break;
                case kRelayRunCondDchgLTV:
                    RELAY_RUNTIME_CONDITION_FAULT_UPDATE(RELAY_DIAGNOSIS_DCHG_LTV_COND)
                    break;
                case kRelayRunCondDchgHT:
                    RELAY_RUNTIME_CONDITION_FAULT_UPDATE(RELAY_DIAGNOSIS_DCHG_HT_COND)
                    break;
                case kRelayRunCondDchgLT:
                    RELAY_RUNTIME_CONDITION_FAULT_UPDATE(RELAY_DIAGNOSIS_DCHG_LT_COND)
                    break;
                case kRelayRunCondDchgDV:
                    RELAY_RUNTIME_CONDITION_FAULT_UPDATE(RELAY_DIAGNOSIS_DCHG_DV_COND)
                    break;
                case kRelayRunCondDchgDT:
                    RELAY_RUNTIME_CONDITION_FAULT_UPDATE(RELAY_DIAGNOSIS_DCHG_DT_COND)
                    break;
                case kRelayRunCondDchgOC:
                    RELAY_RUNTIME_CONDITION_FAULT_UPDATE(RELAY_DIAGNOSIS_DCHG_OC_COND)
                    break;
                case kRelayRunCondChgHV:
                    RELAY_RUNTIME_CONDITION_FAULT_UPDATE(RELAY_DIAGNOSIS_CHG_HV_COND)
                    break;
                case kRelayRunCondChgLV:
                    RELAY_RUNTIME_CONDITION_FAULT_UPDATE(RELAY_DIAGNOSIS_CHG_LV_COND)
                    break;
                case kRelayRunCondChgHTV:
                    RELAY_RUNTIME_CONDITION_FAULT_UPDATE(RELAY_DIAGNOSIS_CHG_HTV_COND)
                    break;
                case kRelayRunCondChgLTV:
                    RELAY_RUNTIME_CONDITION_FAULT_UPDATE(RELAY_DIAGNOSIS_CHG_LTV_COND)
                    break;
                case kRelayRunCondChgHT:
                    RELAY_RUNTIME_CONDITION_FAULT_UPDATE(RELAY_DIAGNOSIS_CHG_HT_COND)
                    break;
                case kRelayRunCondChgLT:
                    RELAY_RUNTIME_CONDITION_FAULT_UPDATE(RELAY_DIAGNOSIS_CHG_LT_COND) 
                    break;
                case kRelayRunCondChgDV:
                    RELAY_RUNTIME_CONDITION_FAULT_UPDATE(RELAY_DIAGNOSIS_CHG_DV_COND)
                    break;
                case kRelayRunCondChgDT:
                    RELAY_RUNTIME_CONDITION_FAULT_UPDATE(RELAY_DIAGNOSIS_CHG_DT_COND)
                    break;
                case kRelayRunCondChgOC:
                    RELAY_RUNTIME_CONDITION_FAULT_UPDATE(RELAY_DIAGNOSIS_CHG_OC_COND)
                    break;
                case kRelayRunCondHSOC:
                    RELAY_RUNTIME_CONDITION_FAULT_UPDATE(RELAY_DIAGNOSIS_HSOC_COND)
                    break;
                case kRelayRunCondLSOC:
                    RELAY_RUNTIME_CONDITION_FAULT_UPDATE(RELAY_DIAGNOSIS_LSOC_COND)
                    break;
                case kRelayRunCondVoltLine:
                    RELAY_RUNTIME_CONDITION_FAULT_UPDATE(RELAY_DIAGNOSIS_VOLT_LINE_COND)
                    break;
                case kRelayRunCondTempLine:
                    RELAY_RUNTIME_CONDITION_FAULT_UPDATE(RELAY_DIAGNOSIS_TEMP_LINE_COND)
                    break;
                case kRelayRunCondInsuLeak:
                    RELAY_RUNTIME_CONDITION_FAULT_UPDATE(RELAY_DIAGNOSIS_INSU_LEAK_COND)
                    break;
                case kRelayRunCondSlaveComm:
                    RELAY_RUNTIME_CONDITION_FAULT_UPDATE(RELAY_DIAGNOSIS_COMM_ABORT_COND)
                    break;
                case kRelayRunCondPrechargeFailure:
                    
                    break;
                case kRelayRunCondUnRelease:
                    
                    break;
                case kRelayRunCondCommAbort:
                    
                    break;
                case kRelayCondUndefine:
                                                                                          
                    break;
            }
            i++;
        }
        *current_fault_num = fault_num;
    }
}