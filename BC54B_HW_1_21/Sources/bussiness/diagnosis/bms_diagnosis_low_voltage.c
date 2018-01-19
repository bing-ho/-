/**
*
* Copyright (c) 2014 Ligoo Inc.
*
* @file bms_diagnosis_low_voltage.c
* @brief
* @note
* @author Liwei Dong
* @date 2014-2-19
*
*/
#include "bms_diagnosis_low_voltage.h"

#pragma MESSAGE DISABLE C12056 // SP debug info incorrect because of optimization or inline assembler

void bms_chg_low_voltage_diagnose(void);
void bms_dchg_low_voltage_diagnose(void);

void bms_low_voltage_diagnose(void)
{
    bms_dchg_low_voltage_diagnose();
    bms_chg_low_voltage_diagnose();
}

void bms_chg_low_voltage_diagnose(void)
{
#if BMS_DIAGNOSIS_LV_INDEPENDENT_WITH_CUR_EN    
    INT16U start_index = BMS_DIAGNOSIS_CHG_LOW_VOLT_RAM_POS;

    BMS_SET_ALARM_PARA(FST_ALARM_COND, BMS_DIAGNOSIS_CHARGE_CONDITION() && bcu_get_low_voltage() <= config_get(kChgLVFstAlarmIndex));
    BMS_SET_ALARM_PARA(FST_ALARM_DLY, config_get(kChgLVFstAlarmDlyIndex));
    BMS_SET_ALARM_PARA(FST_ALARM_REL_COND, BMS_DIAGNOSIS_CHARGE_CONDITION()== 0 || bcu_get_low_voltage() > CHG_LV_FST_ALARM_REL);//config_get(kChgLVFstAlarmRelIndex)
    BMS_SET_ALARM_PARA(FST_ALARM_REL_DLY, CHG_LV_FST_ALARM_REL_DLY);//config_get(kChgLVFstAlarmRelDlyIndex)
    
    BMS_SET_ALARM_PARA(SND_ALARM_COND, BMS_DIAGNOSIS_CHARGE_CONDITION() && bcu_get_low_voltage() <= config_get(kChgLVSndAlarmIndex));
    BMS_SET_ALARM_PARA(SND_ALARM_DLY, config_get(kChgLVSndAlarmDlyIndex));
    BMS_SET_ALARM_PARA(SND_ALARM_REL_COND, BMS_DIAGNOSIS_CHARGE_CONDITION()== 0 || bcu_get_low_voltage() > CHG_LV_SND_ALARM_REL);//config_get(kChgLVSndAlarmRelIndex)
    BMS_SET_ALARM_PARA(SND_ALARM_REL_DLY, CHG_LV_SND_ALARM_REL_DLY);//config_get(kChgLVSndAlarmRelDlyIndex)
    
    BMS_SET_ALARM_PARA(TRD_ALARM_COND, BMS_DIAGNOSIS_CHARGE_CONDITION() && bcu_get_low_voltage() <= config_get(kChgLVTrdAlarmIndex));
    BMS_SET_ALARM_PARA(TRD_ALARM_DLY, config_get(kChgLVTrdAlarmDlyIndex));
    BMS_SET_ALARM_PARA(TRD_ALARM_REL_COND, BMS_DIAGNOSIS_CHARGE_CONDITION()== 0 || bcu_get_low_voltage() > CHG_LV_TRD_ALARM_REL);//config_get(kChgLVTrdAlarmRelIndex)
    BMS_SET_ALARM_PARA(TRD_ALARM_REL_DLY, CHG_LV_TRD_ALARM_REL_DLY);//config_get(kChgLVTrdAlarmRelDlyIndex)
    
    rule_stock_higher_level_priority_alarm_run(start_index);
    
    //Update charge low volt state
    bcu_set_chg_lv_state(rule_stock_get_var(start_index + BMS_DIAG_CUR_STATE_OFFSET));
#else
    bcu_set_chg_lv_state(bcu_get_dchg_lv_state());
#endif
}

void bms_dchg_low_voltage_diagnose(void)
{
    INT16U start_index = BMS_DIAGNOSIS_DCHG_LOW_VOLT_RAM_POS;

#if BMS_DIAGNOSIS_LV_INDEPENDENT_WITH_CUR_EN
    BMS_SET_ALARM_PARA(FST_ALARM_COND, BMS_DIAGNOSIS_CHARGE_CONDITION() == 0 && bcu_get_low_voltage() <= config_get(kDChgLVFstAlarmIndex));
    BMS_SET_ALARM_PARA(FST_ALARM_DLY, config_get(kDChgLVFstAlarmDlyIndex));
    BMS_SET_ALARM_PARA(FST_ALARM_REL_COND, BMS_DIAGNOSIS_CHARGE_CONDITION() || bcu_get_low_voltage() > DCHG_LV_FST_ALARM_REL);//config_get(kDChgLVFstAlarmRelIndex));
    BMS_SET_ALARM_PARA(FST_ALARM_REL_DLY, DCHG_LV_FST_AlARM_REL_DLY);//config_get(kDChgLVFstAlarmRelDlyIndex));
    
    BMS_SET_ALARM_PARA(SND_ALARM_COND, BMS_DIAGNOSIS_CHARGE_CONDITION() == 0 && bcu_get_low_voltage() <= config_get(kDChgLVSndAlarmIndex));
    BMS_SET_ALARM_PARA(SND_ALARM_DLY, config_get(kDChgLVSndAlarmDlyIndex));
    BMS_SET_ALARM_PARA(SND_ALARM_REL_COND, BMS_DIAGNOSIS_CHARGE_CONDITION() || bcu_get_low_voltage() > DCHG_LV_SND_ALARM_REL);//config_get(kDChgLVSndAlarmRelIndex));
    BMS_SET_ALARM_PARA(SND_ALARM_REL_DLY, DCHG_LV_SND_AlARM_REL_DLY);//config_get(kDChgLVSndAlarmRelDlyIndex));
    
    BMS_SET_ALARM_PARA(TRD_ALARM_COND, BMS_DIAGNOSIS_CHARGE_CONDITION() == 0 && bcu_get_low_voltage() <= config_get(kDChgLVTrdAlarmIndex));
    BMS_SET_ALARM_PARA(TRD_ALARM_DLY, config_get(kDChgLVTrdAlarmDlyIndex));
    BMS_SET_ALARM_PARA(TRD_ALARM_REL_COND, BMS_DIAGNOSIS_CHARGE_CONDITION() || bcu_get_low_voltage() > DCHG_LV_TRD_ALARM_REL);//config_get(kDChgLVTrdAlarmRelIndex)
    BMS_SET_ALARM_PARA(TRD_ALARM_REL_DLY, DCHG_LV_TRD_ALARM_REL_DLY);//config_get(kDChgLVTrdAlarmRelDlyIndex)
#else
    BMS_SET_ALARM_PARA(FST_ALARM_COND, bcu_get_low_voltage() <= config_get(kDChgLVFstAlarmIndex));
    BMS_SET_ALARM_PARA(FST_ALARM_DLY, config_get(kDChgLVFstAlarmDlyIndex));
    BMS_SET_ALARM_PARA(FST_ALARM_REL_COND, bcu_get_low_voltage() > DCHG_LV_FST_ALARM_REL);//config_get(kDChgLVFstAlarmRelIndex));
    BMS_SET_ALARM_PARA(FST_ALARM_REL_DLY, DCHG_LV_FST_AlARM_REL_DLY);//config_get(kDChgLVFstAlarmRelDlyIndex));
    
    BMS_SET_ALARM_PARA(SND_ALARM_COND, bcu_get_low_voltage() <= config_get(kDChgLVSndAlarmIndex));
    BMS_SET_ALARM_PARA(SND_ALARM_DLY, config_get(kDChgLVSndAlarmDlyIndex));
    BMS_SET_ALARM_PARA(SND_ALARM_REL_COND, bcu_get_low_voltage() > DCHG_LV_SND_ALARM_REL);//config_get(kDChgLVSndAlarmRelIndex));
    BMS_SET_ALARM_PARA(SND_ALARM_REL_DLY, DCHG_LV_SND_AlARM_REL_DLY);//config_get(kDChgLVSndAlarmRelDlyIndex));
    
    BMS_SET_ALARM_PARA(TRD_ALARM_COND, bcu_get_low_voltage() <= config_get(kDChgLVTrdAlarmIndex));
    BMS_SET_ALARM_PARA(TRD_ALARM_DLY, config_get(kDChgLVTrdAlarmDlyIndex));
    BMS_SET_ALARM_PARA(TRD_ALARM_REL_COND, bcu_get_low_voltage() > DCHG_LV_TRD_ALARM_REL);//config_get(kDChgLVTrdAlarmRelIndex)
    BMS_SET_ALARM_PARA(TRD_ALARM_REL_DLY, DCHG_LV_TRD_ALARM_REL_DLY);//config_get(kDChgLVTrdAlarmRelDlyIndex)
#endif    
    rule_stock_higher_level_priority_alarm_run(start_index);
    
    //Update discharge low volt state
    bcu_set_dchg_lv_state(rule_stock_get_var(start_index + BMS_DIAG_CUR_STATE_OFFSET));
}

AlarmLevel bcu_get_discharge_state(void)
{
    AlarmLevel chg_state, dchg_state;
    
    chg_state = bcu_get_chg_lv_state();
    dchg_state = bcu_get_dchg_lv_state();
    
    return chg_state > dchg_state ? chg_state : dchg_state;
}


