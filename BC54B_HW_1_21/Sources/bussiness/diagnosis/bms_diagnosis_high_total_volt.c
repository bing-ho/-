/**
*
* Copyright (c) 2014 Ligoo Inc.
*
* @file bms_diagnosis_high_total_volt.c
* @brief
* @note
* @author Liwei Dong
* @date 2014-2-19
*
*/
#include "bms_diagnosis_high_total_volt.h"

#pragma MESSAGE DISABLE C12056 // SP debug info incorrect because of optimization or inline assembler

void bms_chg_high_total_volt_diagnose(void);
void bms_dchg_high_total_volt_diagnose(void);

void bms_high_total_volt_diagnose(void)
{
    bms_dchg_high_total_volt_diagnose();
    bms_chg_high_total_volt_diagnose();
}

void bms_chg_high_total_volt_diagnose(void)
{
#if BMS_DIAGNOSIS_HTV_INDEPENDENT_WITH_CUR_EN
    INT16U start_index = BMS_DIAGNOSIS_CHG_HTV_RAM_POS;
    
    BMS_SET_ALARM_PARA(FST_ALARM_COND, BMS_DIAGNOSIS_CHARGE_CONDITION() && bcu_get_average_voltage() >= config_get(kChgHTVFstAlarmIndex));
    BMS_SET_ALARM_PARA(FST_ALARM_DLY, config_get(kChgHTVFstAlarmDlyIndex));
    BMS_SET_ALARM_PARA(FST_ALARM_REL_COND, BMS_DIAGNOSIS_CHARGE_CONDITION()== 0 || bcu_get_average_voltage() < CHG_HTV_FST_ALARM_REL);//config_get(kChgHTVFstAlarmRelIndex)
    BMS_SET_ALARM_PARA(FST_ALARM_REL_DLY, CHG_HTV_FST_ALARM_REL_DLY);//config_get(kChgHTVFstAlarmRelDlyIndex)
    
    BMS_SET_ALARM_PARA(SND_ALARM_COND, BMS_DIAGNOSIS_CHARGE_CONDITION() && bcu_get_average_voltage() >= config_get(kChgHTVSndAlarmIndex));
    BMS_SET_ALARM_PARA(SND_ALARM_DLY, config_get(kChgHTVSndAlarmDlyIndex));
    BMS_SET_ALARM_PARA(SND_ALARM_REL_COND, BMS_DIAGNOSIS_CHARGE_CONDITION()== 0 || bcu_get_average_voltage() < CHG_HTV_SND_ALARM_REL);//config_get(kChgHTVSndAlarmRelIndex)
    BMS_SET_ALARM_PARA(SND_ALARM_REL_DLY, CHG_HTV_SND_ALARM_REL_DLY);//config_get(kChgHTVSndAlarmRelDlyIndex)
    
    BMS_SET_ALARM_PARA(TRD_ALARM_COND, BMS_DIAGNOSIS_CHARGE_CONDITION() && bcu_get_average_voltage() >= config_get(kChgHTVTrdAlarmIndex));
    BMS_SET_ALARM_PARA(TRD_ALARM_DLY, config_get(kChgHTVTrdAlarmDlyIndex));
    BMS_SET_ALARM_PARA(TRD_ALARM_REL_COND, BMS_DIAGNOSIS_CHARGE_CONDITION()== 0 || bcu_get_average_voltage() < CHG_HTV_TRD_ALARM_REL);//config_get(kChgHTVTrdAlarmRelIndex)
    BMS_SET_ALARM_PARA(TRD_ALARM_REL_DLY, CHG_HTV_TRD_ALARM_REL_DLY);//config_get(kChgHTVTrdAlarmRelDlyIndex)
    
    rule_stock_higher_level_priority_alarm_run(start_index);
    
    //Update charge high total volt state
    bcu_set_chg_htv_state(rule_stock_get_var(start_index + BMS_DIAG_CUR_STATE_OFFSET));
#else
    bcu_set_chg_htv_state(bcu_get_dchg_htv_state());
#endif
}

void bms_dchg_high_total_volt_diagnose(void)
{
    INT16U start_index = BMS_DIAGNOSIS_DCHG_HTV_RAM_POS;

#if BMS_DIAGNOSIS_HTV_INDEPENDENT_WITH_CUR_EN    
    BMS_SET_ALARM_PARA(FST_ALARM_COND, BMS_DIAGNOSIS_CHARGE_CONDITION() == 0 && bcu_get_average_voltage() >= config_get(kDChgHTVFstAlarmIndex));
    BMS_SET_ALARM_PARA(FST_ALARM_DLY, config_get(kDChgHTVFstAlarmDlyIndex));
    BMS_SET_ALARM_PARA(FST_ALARM_REL_COND, BMS_DIAGNOSIS_CHARGE_CONDITION() || bcu_get_average_voltage() < DCHG_HTV_FST_ALARM_REL);//config_get(kDChgHTVFstAlarmRelIndex)
    BMS_SET_ALARM_PARA(FST_ALARM_REL_DLY, DCHG_HTV_FST_ALARM_REL_DLY);//config_get(kDChgHTVFstAlarmRelDlyIndex)
    
    BMS_SET_ALARM_PARA(SND_ALARM_COND, BMS_DIAGNOSIS_CHARGE_CONDITION() == 0 && bcu_get_average_voltage() >= config_get(kDChgHTVSndAlarmIndex));
    BMS_SET_ALARM_PARA(SND_ALARM_DLY, config_get(kDChgHTVSndAlarmDlyIndex));
    BMS_SET_ALARM_PARA(SND_ALARM_REL_COND, BMS_DIAGNOSIS_CHARGE_CONDITION() || bcu_get_average_voltage() < DCHG_HTV_SND_ALARM_REL);//config_get(kDChgHTVSndAlarmRelIndex)
    BMS_SET_ALARM_PARA(SND_ALARM_REL_DLY, DCHG_HTV_SND_ALARM_REL_DLY);//config_get(kDChgHTVSndAlarmRelDlyIndex)
    
    BMS_SET_ALARM_PARA(TRD_ALARM_COND, BMS_DIAGNOSIS_CHARGE_CONDITION() == 0 && bcu_get_average_voltage() >= config_get(kDChgHTVTrdAlarmIndex));
    BMS_SET_ALARM_PARA(TRD_ALARM_DLY, config_get(kDChgHTVTrdAlarmDlyIndex));
    BMS_SET_ALARM_PARA(TRD_ALARM_REL_COND, BMS_DIAGNOSIS_CHARGE_CONDITION() || bcu_get_average_voltage() < DCHG_HTV_TRD_ALARM_REL);//config_get(kDChgHTVTrdAlarmRelIndex)
    BMS_SET_ALARM_PARA(TRD_ALARM_REL_DLY, DCHG_HTV_TRD_ALARM_REL_DLY);//config_get(kDChgHTVTrdAlarmRelDlyIndex)
#else
    BMS_SET_ALARM_PARA(FST_ALARM_COND, bcu_get_average_voltage() >= config_get(kDChgHTVFstAlarmIndex));
    BMS_SET_ALARM_PARA(FST_ALARM_DLY, config_get(kDChgHTVFstAlarmDlyIndex));
    BMS_SET_ALARM_PARA(FST_ALARM_REL_COND, bcu_get_average_voltage() < DCHG_HTV_FST_ALARM_REL);//config_get(kDChgHTVFstAlarmRelIndex)
    BMS_SET_ALARM_PARA(FST_ALARM_REL_DLY, DCHG_HTV_FST_ALARM_REL_DLY);//config_get(kDChgHTVFstAlarmRelDlyIndex)
    
    BMS_SET_ALARM_PARA(SND_ALARM_COND, bcu_get_average_voltage() >= config_get(kDChgHTVSndAlarmIndex));
    BMS_SET_ALARM_PARA(SND_ALARM_DLY, config_get(kDChgHTVSndAlarmDlyIndex));
    BMS_SET_ALARM_PARA(SND_ALARM_REL_COND, bcu_get_average_voltage() < DCHG_HTV_SND_ALARM_REL);//config_get(kDChgHTVSndAlarmRelIndex)
    BMS_SET_ALARM_PARA(SND_ALARM_REL_DLY, DCHG_HTV_SND_ALARM_REL_DLY);//config_get(kDChgHTVSndAlarmRelDlyIndex)
    
    BMS_SET_ALARM_PARA(TRD_ALARM_COND, bcu_get_average_voltage() >= config_get(kDChgHTVTrdAlarmIndex));
    BMS_SET_ALARM_PARA(TRD_ALARM_DLY, config_get(kDChgHTVTrdAlarmDlyIndex));
    BMS_SET_ALARM_PARA(TRD_ALARM_REL_COND, bcu_get_average_voltage() < DCHG_HTV_TRD_ALARM_REL);//config_get(kDChgHTVTrdAlarmRelIndex)
    BMS_SET_ALARM_PARA(TRD_ALARM_REL_DLY, DCHG_HTV_TRD_ALARM_REL_DLY);//config_get(kDChgHTVTrdAlarmRelDlyIndex)
#endif
    rule_stock_higher_level_priority_alarm_run(start_index);
    
    //Update discharge high total volt state
    bcu_set_dchg_htv_state(rule_stock_get_var(start_index + BMS_DIAG_CUR_STATE_OFFSET));
}

AlarmLevel bcu_get_high_total_volt_state(void)
{
    AlarmLevel chg_state, dchg_state;
    
    chg_state = bcu_get_chg_htv_state();
    dchg_state = bcu_get_dchg_htv_state();
    
    return chg_state > dchg_state ? chg_state : dchg_state;
}
