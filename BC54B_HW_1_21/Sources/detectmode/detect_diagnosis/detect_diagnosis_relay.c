/**
*
* Copyright (c) 2017 Ligoo Inc.
*
* @file detect_diagnosis_relay.c
* @brief
* @note
* @author Han Zhaoxia
* @date 2017-11-1
*
*/
#include "detect_diagnosis_relay.h"

#pragma MESSAGE DISABLE C4003 // Shift count converted to unsigned char

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
extern RelayDiagnoseContext g_relay_diagnose_context[BMS_RELAY_MAX_COUNT];
#pragma DATA_SEG DEFAULT

void detect_relay_charge_cond_mask_init(void);
void detect_relay_discharge_cond_mask_init(void);

void detect_relay_diagnose_init(void)
{
    detect_relay_charge_cond_mask_init(); //配置充电控制条件掩码
    detect_relay_discharge_cond_mask_init();//配置放电控制条件掩码
}

void detect_relay_charge_cond_mask_init(void)
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
        
        /*上电自检条件*/
        g_relay_diagnose_context[relay_id].selfcheck_cond_mask = 0;

        RELAY_SELFCHECK_CONDITION_MASK_SET(relay_id, kRelayPwrOnCondDchgHT); //使能上电放电温度过高检查
        RELAY_SELFCHECK_CONDITION_MASK_SET(relay_id, kRelayPwrOnCondDchgHV); //使能上电放电最高单体过高检查
        RELAY_SELFCHECK_CONDITION_MASK_SET(relay_id, kRelayPwrOnCondDchgHTV); //使能上电放电总压过高检查
        RELAY_SELFCHECK_CONDITION_MASK_SET(relay_id, kRelayPwrOnCondDchgLV); //使能上电放电最高单体过高检查
        RELAY_SELFCHECK_CONDITION_MASK_SET(relay_id, kRelayPwrOnCondDchgLTV); //使能上电放电总压过高检查

        g_relay_diagnose_context[relay_id].selfcheck_cond_not_rel_mask = 0;
        
        /*运行时条件*/
        g_relay_diagnose_context[relay_id].runtime_cond_mask = 0;

        //RELAY_RUNTIME_CONDITION_MASK_SET(relay_id, kRelayRunCondFullChg); //使能满充状态检查
        //RELAY_RUNTIME_CONDITION_MASK_SET(relay_id, kRelayRunCondMutex); //使能充放电互斥状态检查
        RELAY_RUNTIME_CONDITION_MASK_SET(relay_id, kRelayRunCondDchgHT); //使能上电放电温度过高检查
        RELAY_RUNTIME_CONDITION_MASK_SET(relay_id, kRelayRunCondDchgHV); //使能上电放电最高单体过高检查
        RELAY_RUNTIME_CONDITION_MASK_SET(relay_id, kRelayRunCondDchgHTV); //使能上电放电总压过高检查
        RELAY_RUNTIME_CONDITION_MASK_SET(relay_id, kRelayRunCondDchgLV); //使能上电放电最高单体过高检查
        RELAY_RUNTIME_CONDITION_MASK_SET(relay_id, kRelayRunCondDchgLTV); //使能上电放电总压过高检查

        g_relay_diagnose_context[relay_id].runtime_cond_not_rel_mask = 0;
        
        /*在此添加运行时异常不释放初始化*/
        //RELAY_RUNTIME_COND_NOT_REL_MASK_SET(relay_id, kRelayRunCondDchgHV); //使能放电单体高压异常不释放
    }
}

void detect_relay_discharge_cond_mask_init(void)
{
    INT8U relay_id = 0;
    
    relay_id = relay_control_get_id(kRelayTypeDischarging);
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
        
        /*上电自检条件*/
        g_relay_diagnose_context[relay_id].selfcheck_cond_mask = 0;

        RELAY_SELFCHECK_CONDITION_MASK_SET(relay_id, kRelayPwrOnCondDchgHT); //使能上电放电温度过高检查
        RELAY_SELFCHECK_CONDITION_MASK_SET(relay_id, kRelayPwrOnCondDchgHV); //使能上电放电最高单体过高检查
        RELAY_SELFCHECK_CONDITION_MASK_SET(relay_id, kRelayPwrOnCondDchgHTV); //使能上电放电总压过高检查
        RELAY_SELFCHECK_CONDITION_MASK_SET(relay_id, kRelayPwrOnCondDchgLV); //使能上电放电最高单体过高检查
        RELAY_SELFCHECK_CONDITION_MASK_SET(relay_id, kRelayPwrOnCondDchgLTV); //使能上电放电总压过高检查
        RELAY_SELFCHECK_CONDITION_MASK_SET(relay_id, kRelayPwrOnCondChgHV); //使能上电充电最高单体过高检查
        RELAY_SELFCHECK_CONDITION_MASK_SET(relay_id, kRelayPwrOnCondChgHTV); //使能上电充电总压过高检查

        g_relay_diagnose_context[relay_id].selfcheck_cond_not_rel_mask = 0;
        
        /*运行时条件*/
        g_relay_diagnose_context[relay_id].runtime_cond_mask = 0;

        //RELAY_RUNTIME_CONDITION_MASK_SET(relay_id, kRelayRunCondFullChg); //使能满充状态检查
        //RELAY_RUNTIME_CONDITION_MASK_SET(relay_id, kRelayRunCondMutex); //使能充放电互斥状态检查
        RELAY_RUNTIME_CONDITION_MASK_SET(relay_id, kRelayRunCondDchgHT); //使能上电放电温度过高检查
        RELAY_RUNTIME_CONDITION_MASK_SET(relay_id, kRelayRunCondDchgHV); //使能上电放电最高单体过高检查
        RELAY_RUNTIME_CONDITION_MASK_SET(relay_id, kRelayRunCondDchgHTV); //使能上电放电总压过高检查
        RELAY_RUNTIME_CONDITION_MASK_SET(relay_id, kRelayRunCondDchgLV); //使能上电放电最高单体过高检查
        RELAY_RUNTIME_CONDITION_MASK_SET(relay_id, kRelayRunCondDchgLTV); //使能上电放电总压过高检查
        RELAY_RUNTIME_CONDITION_MASK_SET(relay_id, kRelayRunCondChgHV); //使能上电充电最高单体过高检查
        RELAY_RUNTIME_CONDITION_MASK_SET(relay_id, kRelayRunCondChgHTV); //使能上电充总压过高检查
        
        g_relay_diagnose_context[relay_id].runtime_cond_not_rel_mask = 0;
        
        /*在此添加运行时异常不释放初始化*/
        //RELAY_RUNTIME_COND_NOT_REL_MASK_SET(relay_id, kRelayRunCondDchgHV); //使能放电单体高压异常不释放
    }
}
