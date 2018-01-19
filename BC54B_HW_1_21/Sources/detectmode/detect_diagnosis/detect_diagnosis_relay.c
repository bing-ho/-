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
    detect_relay_charge_cond_mask_init(); //���ó�������������
    detect_relay_discharge_cond_mask_init();//���÷ŵ������������
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
        /*��������*/
        g_relay_diagnose_context[relay_id].base_cond_mask = 0;
        RELAY_BASE_CONDITION_MASK_SET(relay_id, kRelayBaseCondCtrlEnable); //ʹ�̵ܼ�������ʹ�ܼ��
        RELAY_BASE_CONDITION_MASK_SET(relay_id, kRelayBaseCondForceCtrl); //ʹ�̵ܼ���ǿ�ƶϿ����
        //RELAY_BASE_CONDITION_MASK_SET(relay_id, kRelayBaseCondNoRule); //ʹ�̵ܼ������ƹ�����
        
        g_relay_diagnose_context[relay_id].base_cond_not_rel_mask = 0;
        
        /*�ϵ��Լ�����*/
        g_relay_diagnose_context[relay_id].selfcheck_cond_mask = 0;

        RELAY_SELFCHECK_CONDITION_MASK_SET(relay_id, kRelayPwrOnCondDchgHT); //ʹ���ϵ�ŵ��¶ȹ��߼��
        RELAY_SELFCHECK_CONDITION_MASK_SET(relay_id, kRelayPwrOnCondDchgHV); //ʹ���ϵ�ŵ���ߵ�����߼��
        RELAY_SELFCHECK_CONDITION_MASK_SET(relay_id, kRelayPwrOnCondDchgHTV); //ʹ���ϵ�ŵ���ѹ���߼��
        RELAY_SELFCHECK_CONDITION_MASK_SET(relay_id, kRelayPwrOnCondDchgLV); //ʹ���ϵ�ŵ���ߵ�����߼��
        RELAY_SELFCHECK_CONDITION_MASK_SET(relay_id, kRelayPwrOnCondDchgLTV); //ʹ���ϵ�ŵ���ѹ���߼��

        g_relay_diagnose_context[relay_id].selfcheck_cond_not_rel_mask = 0;
        
        /*����ʱ����*/
        g_relay_diagnose_context[relay_id].runtime_cond_mask = 0;

        //RELAY_RUNTIME_CONDITION_MASK_SET(relay_id, kRelayRunCondFullChg); //ʹ������״̬���
        //RELAY_RUNTIME_CONDITION_MASK_SET(relay_id, kRelayRunCondMutex); //ʹ�ܳ�ŵ绥��״̬���
        RELAY_RUNTIME_CONDITION_MASK_SET(relay_id, kRelayRunCondDchgHT); //ʹ���ϵ�ŵ��¶ȹ��߼��
        RELAY_RUNTIME_CONDITION_MASK_SET(relay_id, kRelayRunCondDchgHV); //ʹ���ϵ�ŵ���ߵ�����߼��
        RELAY_RUNTIME_CONDITION_MASK_SET(relay_id, kRelayRunCondDchgHTV); //ʹ���ϵ�ŵ���ѹ���߼��
        RELAY_RUNTIME_CONDITION_MASK_SET(relay_id, kRelayRunCondDchgLV); //ʹ���ϵ�ŵ���ߵ�����߼��
        RELAY_RUNTIME_CONDITION_MASK_SET(relay_id, kRelayRunCondDchgLTV); //ʹ���ϵ�ŵ���ѹ���߼��

        g_relay_diagnose_context[relay_id].runtime_cond_not_rel_mask = 0;
        
        /*�ڴ��������ʱ�쳣���ͷų�ʼ��*/
        //RELAY_RUNTIME_COND_NOT_REL_MASK_SET(relay_id, kRelayRunCondDchgHV); //ʹ�ܷŵ絥���ѹ�쳣���ͷ�
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
        /*��������*/
        g_relay_diagnose_context[relay_id].base_cond_mask = 0;
        RELAY_BASE_CONDITION_MASK_SET(relay_id, kRelayBaseCondCtrlEnable); //ʹ�̵ܼ�������ʹ�ܼ��
        RELAY_BASE_CONDITION_MASK_SET(relay_id, kRelayBaseCondForceCtrl); //ʹ�̵ܼ���ǿ�ƶϿ����
        //RELAY_BASE_CONDITION_MASK_SET(relay_id, kRelayBaseCondNoRule); //ʹ�̵ܼ������ƹ�����
        
        g_relay_diagnose_context[relay_id].base_cond_not_rel_mask = 0;
        
        /*�ϵ��Լ�����*/
        g_relay_diagnose_context[relay_id].selfcheck_cond_mask = 0;

        RELAY_SELFCHECK_CONDITION_MASK_SET(relay_id, kRelayPwrOnCondDchgHT); //ʹ���ϵ�ŵ��¶ȹ��߼��
        RELAY_SELFCHECK_CONDITION_MASK_SET(relay_id, kRelayPwrOnCondDchgHV); //ʹ���ϵ�ŵ���ߵ�����߼��
        RELAY_SELFCHECK_CONDITION_MASK_SET(relay_id, kRelayPwrOnCondDchgHTV); //ʹ���ϵ�ŵ���ѹ���߼��
        RELAY_SELFCHECK_CONDITION_MASK_SET(relay_id, kRelayPwrOnCondDchgLV); //ʹ���ϵ�ŵ���ߵ�����߼��
        RELAY_SELFCHECK_CONDITION_MASK_SET(relay_id, kRelayPwrOnCondDchgLTV); //ʹ���ϵ�ŵ���ѹ���߼��
        RELAY_SELFCHECK_CONDITION_MASK_SET(relay_id, kRelayPwrOnCondChgHV); //ʹ���ϵ�����ߵ�����߼��
        RELAY_SELFCHECK_CONDITION_MASK_SET(relay_id, kRelayPwrOnCondChgHTV); //ʹ���ϵ�����ѹ���߼��

        g_relay_diagnose_context[relay_id].selfcheck_cond_not_rel_mask = 0;
        
        /*����ʱ����*/
        g_relay_diagnose_context[relay_id].runtime_cond_mask = 0;

        //RELAY_RUNTIME_CONDITION_MASK_SET(relay_id, kRelayRunCondFullChg); //ʹ������״̬���
        //RELAY_RUNTIME_CONDITION_MASK_SET(relay_id, kRelayRunCondMutex); //ʹ�ܳ�ŵ绥��״̬���
        RELAY_RUNTIME_CONDITION_MASK_SET(relay_id, kRelayRunCondDchgHT); //ʹ���ϵ�ŵ��¶ȹ��߼��
        RELAY_RUNTIME_CONDITION_MASK_SET(relay_id, kRelayRunCondDchgHV); //ʹ���ϵ�ŵ���ߵ�����߼��
        RELAY_RUNTIME_CONDITION_MASK_SET(relay_id, kRelayRunCondDchgHTV); //ʹ���ϵ�ŵ���ѹ���߼��
        RELAY_RUNTIME_CONDITION_MASK_SET(relay_id, kRelayRunCondDchgLV); //ʹ���ϵ�ŵ���ߵ�����߼��
        RELAY_RUNTIME_CONDITION_MASK_SET(relay_id, kRelayRunCondDchgLTV); //ʹ���ϵ�ŵ���ѹ���߼��
        RELAY_RUNTIME_CONDITION_MASK_SET(relay_id, kRelayRunCondChgHV); //ʹ���ϵ�����ߵ�����߼��
        RELAY_RUNTIME_CONDITION_MASK_SET(relay_id, kRelayRunCondChgHTV); //ʹ���ϵ����ѹ���߼��
        
        g_relay_diagnose_context[relay_id].runtime_cond_not_rel_mask = 0;
        
        /*�ڴ��������ʱ�쳣���ͷų�ʼ��*/
        //RELAY_RUNTIME_COND_NOT_REL_MASK_SET(relay_id, kRelayRunCondDchgHV); //ʹ�ܷŵ絥���ѹ�쳣���ͷ�
    }
}
