/**
*
* Copyright (c) 2014 Ligoo Inc.
*
* @file bms_diagnosis_delta_voltage.c
* @brief
* @note
* @author Liwei Dong
* @date 2014-2-19
*
*/
#include "bms_diagnosis_delta_voltage.h"

#pragma MESSAGE DISABLE C12056 // SP debug info incorrect because of optimization or inline assembler

void bms_chg_delta_voltage_diagnose(void);
void bms_dchg_delta_voltage_diagnose(void);

void bms_delta_voltage_diagnose(void)
{
    bms_dchg_delta_voltage_diagnose();
    bms_chg_delta_voltage_diagnose();
}

void bms_chg_delta_voltage_diagnose(void)
{
#if BMS_DIAGNOSIS_HDV_INDEPENDENT_WITH_CUR_EN
    INT16U start_index = BMS_DIAGNOSIS_CHG_DELTA_VOLT_RAM_POS;
    
    BMS_SET_ALARM_PARA(FST_ALARM_COND, BMS_DIAGNOSIS_CHARGE_CONDITION() && bcu_get_difference_voltage() >= config_get(kChgDVFstAlarmIndex));
    BMS_SET_ALARM_PARA(FST_ALARM_DLY, config_get(kChgDVFstAlarmDlyIndex));
    BMS_SET_ALARM_PARA(FST_ALARM_REL_COND, BMS_DIAGNOSIS_CHARGE_CONDITION()== 0 || bcu_get_difference_voltage() < CHG_DV_FST_ALARM_REL);//config_get(kChgDVFstAlarmRelIndex)
    BMS_SET_ALARM_PARA(FST_ALARM_REL_DLY, CHG_DV_FST_ALARM_REL_DLY);//config_get(kChgDVFstAlarmRelDlyIndex)
    
    BMS_SET_ALARM_PARA(SND_ALARM_COND, BMS_DIAGNOSIS_CHARGE_CONDITION() && bcu_get_difference_voltage() >= config_get(kChgDVSndAlarmIndex));
    BMS_SET_ALARM_PARA(SND_ALARM_DLY, config_get(kChgDVSndAlarmDlyIndex));
    BMS_SET_ALARM_PARA(SND_ALARM_REL_COND, BMS_DIAGNOSIS_CHARGE_CONDITION()== 0 || bcu_get_difference_voltage() < CHG_DV_SND_ALARM_REL);//config_get(kChgDVSndAlarmRelIndex)
    BMS_SET_ALARM_PARA(SND_ALARM_REL_DLY, CHG_DV_SND_ALARM_REL_DLY);//config_get(kChgDVSndAlarmRelDlyIndex)
    
    BMS_SET_ALARM_PARA(TRD_ALARM_COND, BMS_DIAGNOSIS_CHARGE_CONDITION() && bcu_get_difference_voltage() >= config_get(kChgDVTrdAlarmIndex));
    BMS_SET_ALARM_PARA(TRD_ALARM_DLY, config_get(kChgDVTrdAlarmDlyIndex));
    BMS_SET_ALARM_PARA(TRD_ALARM_REL_COND, BMS_DIAGNOSIS_CHARGE_CONDITION()== 0 || bcu_get_difference_voltage() < CHG_DV_TRD_ALARM_REL);//config_get(kChgDVTrdAlarmRelIndex)
    BMS_SET_ALARM_PARA(TRD_ALARM_REL_DLY, CHG_DV_TRD_ALARM_REL_DLY);//config_get(kChgDVTrdAlarmRelDlyIndex)
    
    rule_stock_higher_level_priority_alarm_run(start_index);
    
    //Update battery delta voltage state
    bcu_set_chg_delta_volt_state(rule_stock_get_var(start_index + BMS_DIAG_CUR_STATE_OFFSET));
#else
    bcu_set_chg_delta_volt_state(bcu_get_dchg_delta_volt_state());
#endif
}

void bms_dchg_delta_voltage_diagnose(void)
{
    INT16U start_index = BMS_DIAGNOSIS_DCHG_DELTA_VOLT_RAM_POS;

#if BMS_DIAGNOSIS_HDV_INDEPENDENT_WITH_CUR_EN    
    BMS_SET_ALARM_PARA(FST_ALARM_COND, BMS_DIAGNOSIS_CHARGE_CONDITION() == 0 && bcu_get_difference_voltage() >= config_get(kDChgDVFstAlarmIndex));
    BMS_SET_ALARM_PARA(FST_ALARM_DLY, config_get(kDChgDVFstAlarmDlyIndex));
    BMS_SET_ALARM_PARA(FST_ALARM_REL_COND, BMS_DIAGNOSIS_CHARGE_CONDITION() || (bcu_get_difference_voltage() < DCHG_DV_FST_ALARM_REL));//config_get(kDChgDVFstAlarmRelIndex)
    BMS_SET_ALARM_PARA(FST_ALARM_REL_DLY, DCHG_DV_FST_AlARM_REL_DLY);//config_get(kDChgDVFstAlarmRelDlyIndex)
    
    BMS_SET_ALARM_PARA(SND_ALARM_COND, BMS_DIAGNOSIS_CHARGE_CONDITION() == 0 && bcu_get_difference_voltage() >= config_get(kDChgDVSndAlarmIndex));
    BMS_SET_ALARM_PARA(SND_ALARM_DLY, config_get(kDChgDVSndAlarmDlyIndex));
    BMS_SET_ALARM_PARA(SND_ALARM_REL_COND, BMS_DIAGNOSIS_CHARGE_CONDITION() || (bcu_get_difference_voltage() < DCHG_DV_SND_AlARM_REL));//config_get(kDChgDVSndAlarmRelIndex)
    BMS_SET_ALARM_PARA(SND_ALARM_REL_DLY, DCHG_DV_SND_AlARM_REL_DLY);//config_get(kDChgDVSndAlarmRelDlyIndex)
    
    BMS_SET_ALARM_PARA(TRD_ALARM_COND, BMS_DIAGNOSIS_CHARGE_CONDITION() == 0 && bcu_get_difference_voltage() >= config_get(kDChgDVTrdAlarmIndex));
    BMS_SET_ALARM_PARA(TRD_ALARM_DLY, config_get(kDChgDVTrdAlarmDlyIndex));
    BMS_SET_ALARM_PARA(TRD_ALARM_REL_COND, BMS_DIAGNOSIS_CHARGE_CONDITION() || (bcu_get_difference_voltage() < DCHG_DV_TRD_ALARM_REL));//config_get(kDChgDVTrdAlarmRelIndex)
    BMS_SET_ALARM_PARA(TRD_ALARM_REL_DLY, DCHG_DV_TRD_ALARM_REL_DLY);//config_get(kDChgDVTrdAlarmRelDlyIndex)
#else
    BMS_SET_ALARM_PARA(FST_ALARM_COND, bcu_get_difference_voltage() >= config_get(kDChgDVFstAlarmIndex));
    BMS_SET_ALARM_PARA(FST_ALARM_DLY, config_get(kDChgDVFstAlarmDlyIndex));
    BMS_SET_ALARM_PARA(FST_ALARM_REL_COND, bcu_get_difference_voltage() < DCHG_DV_FST_ALARM_REL);//config_get(kDChgDVFstAlarmRelIndex)
    BMS_SET_ALARM_PARA(FST_ALARM_REL_DLY, DCHG_DV_FST_AlARM_REL_DLY);//config_get(kDChgDVFstAlarmRelDlyIndex)
    
    BMS_SET_ALARM_PARA(SND_ALARM_COND, bcu_get_difference_voltage() >= config_get(kDChgDVSndAlarmIndex));
    BMS_SET_ALARM_PARA(SND_ALARM_DLY, config_get(kDChgDVSndAlarmDlyIndex));
    BMS_SET_ALARM_PARA(SND_ALARM_REL_COND, bcu_get_difference_voltage() < DCHG_DV_SND_AlARM_REL);//config_get(kDChgDVSndAlarmRelIndex)
    BMS_SET_ALARM_PARA(SND_ALARM_REL_DLY, DCHG_DV_SND_AlARM_REL_DLY);//config_get(kDChgDVSndAlarmRelDlyIndex)
    
    BMS_SET_ALARM_PARA(TRD_ALARM_COND, bcu_get_difference_voltage() >= config_get(kDChgDVTrdAlarmIndex));
    BMS_SET_ALARM_PARA(TRD_ALARM_DLY, config_get(kDChgDVTrdAlarmDlyIndex));
    BMS_SET_ALARM_PARA(TRD_ALARM_REL_COND, bcu_get_difference_voltage() < DCHG_DV_TRD_ALARM_REL);//config_get(kDChgDVTrdAlarmRelIndex)
    BMS_SET_ALARM_PARA(TRD_ALARM_REL_DLY, DCHG_DV_TRD_ALARM_REL_DLY);//config_get(kDChgDVTrdAlarmRelDlyIndex)
#endif
    rule_stock_higher_level_priority_alarm_run(start_index);
    
    //Update battery delta voltage state
    bcu_set_dchg_delta_volt_state(rule_stock_get_var(start_index + BMS_DIAG_CUR_STATE_OFFSET));
}

AlarmLevel bcu_get_delta_voltage_state(void)
{
    AlarmLevel chg_state, dchg_state;
    
    chg_state = bcu_get_chg_delta_volt_state();
    dchg_state = bcu_get_dchg_delta_volt_state();
    
    return chg_state > dchg_state ? chg_state : dchg_state;
}
