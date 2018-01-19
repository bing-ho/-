/**
*
* Copyright (c) 2014 Ligoo Inc.
*
* @file bms_diagnosis_high_current.c
* @brief
* @note
* @author Liwei Dong
* @date 2014-2-19
*
*/
#include "bms_diagnosis_high_current.h"

#pragma MESSAGE DISABLE C12056 // SP debug info incorrect because of optimization or inline assembler

#define BMS_DIAGNOSIS_OC_LIMIT  50

void bms_chg_high_current_diagnose(void);
void bms_dchg_high_current_diagnose(void);
void bms_feedback_high_current_diagnose(void);

void bms_high_current_diagnose(void)
{
    bms_dchg_high_current_diagnose();
    bms_chg_high_current_diagnose();
    bms_feedback_high_current_diagnose(); //回馈过流报警
}


void bms_chg_high_current_diagnose(void)
{
#if BMS_DIAGNOSIS_OC_INDEPENDENT_WITH_CUR_EN
    INT16U current=0, start_index=BMS_DIAGNOSIS_CHG_HIGH_CUR_RAM_POS;
    
    current = bcu_get_charging_current();
    BMS_SET_ALARM_PARA(FST_ALARM_COND, current >= BMS_DIAGNOSIS_OC_LIMIT && charger_is_connected() && current >= config_get(kChgOCFstAlarmIndex));
    BMS_SET_ALARM_PARA(FST_ALARM_DLY, config_get(kChgOCFstAlarmDlyIndex));
    BMS_SET_ALARM_PARA(FST_ALARM_REL_COND, current < BMS_DIAGNOSIS_OC_LIMIT || charger_is_connected()== 0 || current < CHG_OC_FST_ALARM_REL);//config_get(kChgOCFstAlarmRelIndex)
    BMS_SET_ALARM_PARA(FST_ALARM_REL_DLY, CHG_OC_FST_ALARM_REL_DLY);//config_get(kChgOCFstAlarmRelDlyIndex)
    
    BMS_SET_ALARM_PARA(SND_ALARM_COND, current >= BMS_DIAGNOSIS_OC_LIMIT && charger_is_connected() && current >= config_get(kChgOCSndAlarmIndex));
    BMS_SET_ALARM_PARA(SND_ALARM_DLY, config_get(kChgOCSndAlarmDlyIndex));
    BMS_SET_ALARM_PARA(SND_ALARM_REL_COND, current < BMS_DIAGNOSIS_OC_LIMIT || charger_is_connected()== 0 || current < CHG_OC_SND_ALARM_REL);//config_get(kChgOCSndAlarmRelIndex)
    BMS_SET_ALARM_PARA(SND_ALARM_REL_DLY, CHG_OC_SND_ALARM_REL_DLY);//config_get(kChgOCSndAlarmRelDlyIndex)
    
    BMS_SET_ALARM_PARA(TRD_ALARM_COND, current >= BMS_DIAGNOSIS_OC_LIMIT && charger_is_connected() && current >= config_get(kChgOCTrdAlarmIndex));
    BMS_SET_ALARM_PARA(TRD_ALARM_DLY, config_get(kChgOCTrdAlarmDlyIndex));
    BMS_SET_ALARM_PARA(TRD_ALARM_REL_COND, current < BMS_DIAGNOSIS_OC_LIMIT || charger_is_connected()== 0 || current < CHG_OC_TRD_ALARM_REL);//config_get(kChgOCTrdAlarmRelIndex)
    BMS_SET_ALARM_PARA(TRD_ALARM_REL_DLY, CHG_OC_TRD_ALARM_REL_DLY);//config_get(kChgOCTrdAlarmRelDlyIndex)
    
    rule_stock_higher_level_priority_alarm_run(start_index);
    
    //Update battery over charge current state
    bcu_set_chgr_oc_state(rule_stock_get_var(start_index + BMS_DIAG_CUR_STATE_OFFSET));
#else
    bcu_set_chgr_oc_state(bcu_get_dchg_oc_state());
#endif
}

void bms_dchg_high_current_diagnose(void)
{
    INT16U current=0, start_index=BMS_DIAGNOSIS_DCHG_HIGH_CUR_RAM_POS;

#if BMS_DIAGNOSIS_OC_INDEPENDENT_WITH_CUR_EN
    current = bcu_get_discharging_current();
    
    BMS_SET_ALARM_PARA(FST_ALARM_COND, current >= BMS_DIAGNOSIS_OC_LIMIT && charger_is_connected() == 0 && current >= config_get(kDChgOCFstAlarmIndex));
    BMS_SET_ALARM_PARA(FST_ALARM_DLY, config_get(kDChgOCFstAlarmDlyIndex));
    BMS_SET_ALARM_PARA(FST_ALARM_REL_COND, current < BMS_DIAGNOSIS_OC_LIMIT || charger_is_connected() || (current < DCHG_OC_FST_ALARM_REL));//config_get(kDChgOCFstAlarmRelIndex)
    BMS_SET_ALARM_PARA(FST_ALARM_REL_DLY, DCHG_OC_FST_ALARM_REL_DLY);//config_get(kDChgOCFstAlarmRelDlyIndex)
    
    BMS_SET_ALARM_PARA(SND_ALARM_COND, current >= BMS_DIAGNOSIS_OC_LIMIT && charger_is_connected() == 0 && current >= config_get(kDChgOCSndAlarmIndex));
    BMS_SET_ALARM_PARA(SND_ALARM_DLY, config_get(kDChgOCSndAlarmDlyIndex));
    BMS_SET_ALARM_PARA(SND_ALARM_REL_COND, current < BMS_DIAGNOSIS_OC_LIMIT || charger_is_connected() || (current < DCHG_OC_SND_ALARM_REL));//config_get(kDChgOCSndAlarmRelIndex)
    BMS_SET_ALARM_PARA(SND_ALARM_REL_DLY, DCHG_OC_SND_ALARM_REL_DLY);//config_get(kDChgOCSndAlarmRelDlyIndex)
    
    BMS_SET_ALARM_PARA(TRD_ALARM_COND, current >= BMS_DIAGNOSIS_OC_LIMIT && charger_is_connected() == 0 && current >= config_get(kDChgOCTrdAlarmIndex));
    BMS_SET_ALARM_PARA(TRD_ALARM_DLY, config_get(kDChgOCTrdAlarmDlyIndex));
    BMS_SET_ALARM_PARA(TRD_ALARM_REL_COND, current < BMS_DIAGNOSIS_OC_LIMIT || charger_is_connected() || (current < DCHG_OC_TRD_ALARM_REL));//config_get(kDChgOCTrdAlarmRelIndex)
    BMS_SET_ALARM_PARA(TRD_ALARM_REL_DLY, DCHG_OC_TRD_ALARM_REL_DLY);//config_get(kDChgOCTrdAlarmRelDlyIndex)
#else
    current = abs(bcu_get_current());
    
    BMS_SET_ALARM_PARA(FST_ALARM_COND, current >= BMS_DIAGNOSIS_OC_LIMIT && current >= config_get(kDChgOCFstAlarmIndex));
    BMS_SET_ALARM_PARA(FST_ALARM_DLY, config_get(kDChgOCFstAlarmDlyIndex));
    BMS_SET_ALARM_PARA(FST_ALARM_REL_COND, current < BMS_DIAGNOSIS_OC_LIMIT || current < DCHG_OC_FST_ALARM_REL);//config_get(kDChgOCFstAlarmRelIndex)
    BMS_SET_ALARM_PARA(FST_ALARM_REL_DLY, DCHG_OC_FST_ALARM_REL_DLY);//config_get(kDChgOCFstAlarmRelDlyIndex)
    
    BMS_SET_ALARM_PARA(SND_ALARM_COND, current >= BMS_DIAGNOSIS_OC_LIMIT && current >= config_get(kDChgOCSndAlarmIndex));
    BMS_SET_ALARM_PARA(SND_ALARM_DLY, config_get(kDChgOCSndAlarmDlyIndex));
    BMS_SET_ALARM_PARA(SND_ALARM_REL_COND, current < BMS_DIAGNOSIS_OC_LIMIT || current < DCHG_OC_SND_ALARM_REL);//config_get(kDChgOCSndAlarmRelIndex)
    BMS_SET_ALARM_PARA(SND_ALARM_REL_DLY, DCHG_OC_SND_ALARM_REL_DLY);//config_get(kDChgOCSndAlarmRelDlyIndex)
    
    BMS_SET_ALARM_PARA(TRD_ALARM_COND, current >= BMS_DIAGNOSIS_OC_LIMIT && current >= config_get(kDChgOCTrdAlarmIndex));
    BMS_SET_ALARM_PARA(TRD_ALARM_DLY, config_get(kDChgOCTrdAlarmDlyIndex));
    BMS_SET_ALARM_PARA(TRD_ALARM_REL_COND, current < BMS_DIAGNOSIS_OC_LIMIT || current < DCHG_OC_TRD_ALARM_REL);//config_get(kDChgOCTrdAlarmRelIndex)
    BMS_SET_ALARM_PARA(TRD_ALARM_REL_DLY, DCHG_OC_TRD_ALARM_REL_DLY);//config_get(kDChgOCTrdAlarmRelDlyIndex)
#endif
    rule_stock_higher_level_priority_alarm_run(start_index);
    
    //Update battery over discharge current state
    bcu_set_dchg_oc_state(rule_stock_get_var(start_index + BMS_DIAG_CUR_STATE_OFFSET));
}

void bms_feedback_high_current_diagnose(void)
{
#if BMS_DIAGNOSIS_OC_INDEPENDENT_WITH_CUR_EN
    INT16U current=0, start_index=BMS_DIAGNOSIS_FD_OC_RAM_POS;
    
    current = bcu_get_charging_current();
    BMS_SET_ALARM_PARA(FST_ALARM_COND, current >= BMS_DIAGNOSIS_OC_LIMIT && charger_is_connected()== 0 && current >= config_get(kChgOCFstAlarmIndex));
    BMS_SET_ALARM_PARA(FST_ALARM_DLY, config_get(kChgOCFstAlarmDlyIndex));
    BMS_SET_ALARM_PARA(FST_ALARM_REL_COND, current < BMS_DIAGNOSIS_OC_LIMIT || charger_is_connected() || current < CHG_OC_FST_ALARM_REL);//config_get(kChgOCFstAlarmRelIndex)
    BMS_SET_ALARM_PARA(FST_ALARM_REL_DLY, CHG_OC_FST_ALARM_REL_DLY);//config_get(kChgOCFstAlarmRelDlyIndex)
    
    BMS_SET_ALARM_PARA(SND_ALARM_COND, current >= BMS_DIAGNOSIS_OC_LIMIT && charger_is_connected()== 0 && current >= config_get(kChgOCSndAlarmIndex));
    BMS_SET_ALARM_PARA(SND_ALARM_DLY, config_get(kChgOCSndAlarmDlyIndex));
    BMS_SET_ALARM_PARA(SND_ALARM_REL_COND, current < BMS_DIAGNOSIS_OC_LIMIT || charger_is_connected() || current < CHG_OC_SND_ALARM_REL);//config_get(kChgOCSndAlarmRelIndex)
    BMS_SET_ALARM_PARA(SND_ALARM_REL_DLY, CHG_OC_SND_ALARM_REL_DLY);//config_get(kChgOCSndAlarmRelDlyIndex)
    
    BMS_SET_ALARM_PARA(TRD_ALARM_COND, current >= BMS_DIAGNOSIS_OC_LIMIT && charger_is_connected()== 0 && current >= config_get(kChgOCTrdAlarmIndex));
    BMS_SET_ALARM_PARA(TRD_ALARM_DLY, config_get(kChgOCTrdAlarmDlyIndex));
    BMS_SET_ALARM_PARA(TRD_ALARM_REL_COND, current < BMS_DIAGNOSIS_OC_LIMIT || charger_is_connected() || current < CHG_OC_TRD_ALARM_REL);//config_get(kChgOCTrdAlarmRelIndex)
    BMS_SET_ALARM_PARA(TRD_ALARM_REL_DLY, CHG_OC_TRD_ALARM_REL_DLY);//config_get(kChgOCTrdAlarmRelDlyIndex)
    
    rule_stock_higher_level_priority_alarm_run(start_index);
    
    //Update battery over charge current state
    bcu_set_feedback_oc_state(rule_stock_get_var(start_index + BMS_DIAG_CUR_STATE_OFFSET));
#else
    bcu_set_chgr_oc_state(bcu_get_dchg_oc_state());
#endif
}

AlarmLevel bcu_get_high_current_state(void)
{
    AlarmLevel chg_state, dchg_state, fd_state;
    
    chg_state = bcu_get_chgr_oc_state();
    dchg_state = bcu_get_dchg_oc_state();
    fd_state = bcu_get_feedback_oc_state();
    
    chg_state = chg_state > dchg_state ? chg_state : dchg_state;
    chg_state = chg_state > fd_state ? chg_state : fd_state;
    
    return chg_state;
}
