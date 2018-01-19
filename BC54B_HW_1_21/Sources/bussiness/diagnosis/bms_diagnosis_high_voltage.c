/**
*
* Copyright (c) 2014 Ligoo Inc.
*
* @file bms_diagnosis_high_voltage.c
* @brief
* @note
* @author Liwei Dong
* @date 2014-2-19
*
*/
#include "bms_diagnosis_high_voltage.h"

#pragma MESSAGE DISABLE C12056 // SP debug info incorrect because of optimization or inline assembler

void bms_chg_high_voltage_diagnose(void);
void bms_dchg_high_voltage_diagnose(void);

void bms_high_voltage_diagnose(void)
{
    bms_dchg_high_voltage_diagnose();
    bms_chg_high_voltage_diagnose();
}

void bms_chg_high_voltage_diagnose(void)
{
#if BMS_DIAGNOSIS_HV_INDEPENDENT_WITH_CUR_EN
    INT16U start_index = BMS_DIAGNOSIS_CHG_HIGH_VOLT_RAM_POS;

    BMS_SET_ALARM_PARA(FST_ALARM_COND, BMS_DIAGNOSIS_CHARGE_CONDITION() && bcu_get_high_voltage() >= config_get(kChgHVFstAlarmIndex));
    BMS_SET_ALARM_PARA(FST_ALARM_DLY, config_get(kChgHVFstAlarmDlyIndex));
    BMS_SET_ALARM_PARA(FST_ALARM_REL_COND, BMS_DIAGNOSIS_CHARGE_CONDITION() == 0 || bcu_get_high_voltage() < CHG_HV_FST_ALARM_REL);//config_get(kChgHVFstAlarmRelIndex)
    BMS_SET_ALARM_PARA(FST_ALARM_REL_DLY, CHG_HV_FST_ALARM_REL_DLY);//config_get(kChgHVFstAlarmRelDlyIndex)
    
    BMS_SET_ALARM_PARA(SND_ALARM_COND, BMS_DIAGNOSIS_CHARGE_CONDITION() && bcu_get_high_voltage() >= config_get(kChgHVSndAlarmIndex));
    BMS_SET_ALARM_PARA(SND_ALARM_DLY, config_get(kChgHVSndAlarmDlyIndex));
    BMS_SET_ALARM_PARA(SND_ALARM_REL_COND, BMS_DIAGNOSIS_CHARGE_CONDITION()== 0 || bcu_get_high_voltage() < CHG_HV_SND_ALARM_REL);//config_get(kChgHVSndAlarmRelIndex)
    BMS_SET_ALARM_PARA(SND_ALARM_REL_DLY, CHG_HV_SND_ALARM_REL_DLY);//config_get(kChgHVSndAlarmRelDlyIndex)
    
    BMS_SET_ALARM_PARA(TRD_ALARM_COND, BMS_DIAGNOSIS_CHARGE_CONDITION() && bcu_get_high_voltage() >= config_get(kChgHVTrdAlarmIndex));
    BMS_SET_ALARM_PARA(TRD_ALARM_DLY, config_get(kChgHVTrdAlarmDlyIndex));
    BMS_SET_ALARM_PARA(TRD_ALARM_REL_COND, BMS_DIAGNOSIS_CHARGE_CONDITION()== 0 || bcu_get_high_voltage() < CHG_HV_TRD_ALARM_REL);//config_get(kChgHVTrdAlarmRelIndex)
    BMS_SET_ALARM_PARA(TRD_ALARM_REL_DLY, CHG_HV_TRD_ALARM_REL_DLY);//config_get(kChgHVTrdAlarmRelDlyIndex)
    
    rule_stock_higher_level_priority_alarm_run(start_index);
    
    //Update charge high volt state
    bcu_set_chg_hv_state(rule_stock_get_var(start_index + BMS_DIAG_CUR_STATE_OFFSET));
#else    
    bcu_set_chg_hv_state(bcu_get_dchg_hv_state());
#endif
}

void bms_dchg_high_voltage_diagnose(void)
{
    INT16U start_index = BMS_DIAGNOSIS_DCHG_HIGH_VOLT_RAM_POS;

#if BMS_DIAGNOSIS_HV_INDEPENDENT_WITH_CUR_EN   
    BMS_SET_ALARM_PARA(FST_ALARM_COND, BMS_DIAGNOSIS_CHARGE_CONDITION() == 0 && bcu_get_high_voltage() >= config_get(kDChgHVFstAlarmIndex));
    BMS_SET_ALARM_PARA(FST_ALARM_DLY, config_get(kDChgHVFstAlarmDlyIndex));
    BMS_SET_ALARM_PARA(FST_ALARM_REL_COND, BMS_DIAGNOSIS_CHARGE_CONDITION() || bcu_get_high_voltage() < DCHG_HV_FST_ALARM_REL);//config_get(kDChgHVFstAlarmRelIndex));
    BMS_SET_ALARM_PARA(FST_ALARM_REL_DLY, DCHG_HV_FST_AlARM_REL_DLY);//config_get(kDChgHVFstAlarmRelDlyIndex));
    
    BMS_SET_ALARM_PARA(SND_ALARM_COND, BMS_DIAGNOSIS_CHARGE_CONDITION() == 0 && bcu_get_high_voltage() >= config_get(kDChgHVSndAlarmIndex));
    BMS_SET_ALARM_PARA(SND_ALARM_DLY, config_get(kDChgHVSndAlarmDlyIndex));
    BMS_SET_ALARM_PARA(SND_ALARM_REL_COND, BMS_DIAGNOSIS_CHARGE_CONDITION() || bcu_get_high_voltage() < DCHG_HV_SND_AlARM_REL);//config_get(kDChgHVSndAlarmRelIndex));
    BMS_SET_ALARM_PARA(SND_ALARM_REL_DLY, DCHG_HV_SND_AlARM_REL_DLY);//config_get(kDChgHVSndAlarmRelDlyIndex));
    
    BMS_SET_ALARM_PARA(TRD_ALARM_COND, BMS_DIAGNOSIS_CHARGE_CONDITION() == 0 && bcu_get_high_voltage() >= config_get(kDChgHVTrdAlarmIndex));
    BMS_SET_ALARM_PARA(TRD_ALARM_DLY, config_get(kDChgHVTrdAlarmDlyIndex));
    BMS_SET_ALARM_PARA(TRD_ALARM_REL_COND, BMS_DIAGNOSIS_CHARGE_CONDITION() || bcu_get_high_voltage() < DCHG_HV_TRD_ALARM_REL);//config_get(kDChgHVTrdAlarmRelIndex)
    BMS_SET_ALARM_PARA(TRD_ALARM_REL_DLY, DCHG_HV_TRD_ALARM_REL_DLY);//config_get(kDChgHVTrdAlarmRelDlyIndex)
#else
    BMS_SET_ALARM_PARA(FST_ALARM_COND, bcu_get_high_voltage() >= config_get(kDChgHVFstAlarmIndex));
    BMS_SET_ALARM_PARA(FST_ALARM_DLY, config_get(kDChgHVFstAlarmDlyIndex));
    BMS_SET_ALARM_PARA(FST_ALARM_REL_COND, bcu_get_high_voltage() < DCHG_HV_FST_ALARM_REL);//config_get(kDChgHVFstAlarmRelIndex));
    BMS_SET_ALARM_PARA(FST_ALARM_REL_DLY, DCHG_HV_FST_AlARM_REL_DLY);//config_get(kDChgHVFstAlarmRelDlyIndex));
    
    BMS_SET_ALARM_PARA(SND_ALARM_COND, bcu_get_high_voltage() >= config_get(kDChgHVSndAlarmIndex));
    BMS_SET_ALARM_PARA(SND_ALARM_DLY, config_get(kDChgHVSndAlarmDlyIndex));
    BMS_SET_ALARM_PARA(SND_ALARM_REL_COND, bcu_get_high_voltage() < DCHG_HV_SND_AlARM_REL);//config_get(kDChgHVSndAlarmRelIndex));
    BMS_SET_ALARM_PARA(SND_ALARM_REL_DLY, DCHG_HV_SND_AlARM_REL_DLY);//config_get(kDChgHVSndAlarmRelDlyIndex));
    
    BMS_SET_ALARM_PARA(TRD_ALARM_COND, bcu_get_high_voltage() >= config_get(kDChgHVTrdAlarmIndex));
    BMS_SET_ALARM_PARA(TRD_ALARM_DLY, config_get(kDChgHVTrdAlarmDlyIndex));
    BMS_SET_ALARM_PARA(TRD_ALARM_REL_COND, bcu_get_high_voltage() < DCHG_HV_TRD_ALARM_REL);//config_get(kDChgHVTrdAlarmRelIndex)
    BMS_SET_ALARM_PARA(TRD_ALARM_REL_DLY, DCHG_HV_TRD_ALARM_REL_DLY);//config_get(kDChgHVTrdAlarmRelDlyIndex)
#endif
    rule_stock_higher_level_priority_alarm_run(start_index);
    
    //Update discharge high volt state
    bcu_set_dchg_hv_state(rule_stock_get_var(start_index + BMS_DIAG_CUR_STATE_OFFSET));
}

AlarmLevel bcu_get_charge_state(void)
{
    AlarmLevel chg_state, dchg_state;
    
    chg_state = bcu_get_chg_hv_state();
    dchg_state = bcu_get_dchg_hv_state();
    
    return chg_state > dchg_state ? chg_state : dchg_state;
}
