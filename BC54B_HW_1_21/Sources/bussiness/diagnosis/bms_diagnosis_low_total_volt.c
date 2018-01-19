/**
*
* Copyright (c) 2014 Ligoo Inc.
*
* @file bms_diagnosis_low_total_volt.c
* @brief
* @note
* @author Liwei Dong
* @date 2014-2-19
*
*/
#include "bms_diagnosis_low_total_volt.h"

#pragma MESSAGE DISABLE C12056 // SP debug info incorrect because of optimization or inline assembler

void bms_chg_low_total_volt_diagnose(void);
void bms_dchg_low_total_volt_diagnose(void);

void bms_low_total_volt_diagnose(void)
{
    bms_dchg_low_total_volt_diagnose();
    bms_chg_low_total_volt_diagnose();
}

void bms_chg_low_total_volt_diagnose(void)
{
#if BMS_DIAGNOSIS_LTV_INDEPENDENT_WITH_CUR_EN
    INT16U start_index = BMS_DIAGNOSIS_CHG_LTV_RAM_POS;
    
    BMS_SET_ALARM_PARA(FST_ALARM_COND, BMS_DIAGNOSIS_CHARGE_CONDITION() && bcu_get_average_voltage() > 0 && bcu_get_average_voltage() <= config_get(kChgLTVFstAlarmIndex));
    BMS_SET_ALARM_PARA(FST_ALARM_DLY, config_get(kChgLTVFstAlarmDlyIndex));
    BMS_SET_ALARM_PARA(FST_ALARM_REL_COND, BMS_DIAGNOSIS_CHARGE_CONDITION()== 0 || bcu_get_average_voltage() >CHG_LTV_FST_ALARM_REL);//config_get(kChgLTVFstAlarmRelIndex)
    BMS_SET_ALARM_PARA(FST_ALARM_REL_DLY, CHG_LTV_FST_ALARM_REL_DLY);//config_get(kChgLTVFstAlarmRelDlyIndex)
    
    BMS_SET_ALARM_PARA(SND_ALARM_COND, BMS_DIAGNOSIS_CHARGE_CONDITION() && bcu_get_average_voltage() > 0 && bcu_get_average_voltage() <= config_get(kChgLTVSndAlarmIndex));
    BMS_SET_ALARM_PARA(SND_ALARM_DLY, config_get(kChgLTVSndAlarmDlyIndex));
    BMS_SET_ALARM_PARA(SND_ALARM_REL_COND, BMS_DIAGNOSIS_CHARGE_CONDITION()== 0 || bcu_get_average_voltage() >CHG_LTV_SND_ALARM_REL);//config_get(kChgLTVSndAlarmRelIndex)
    BMS_SET_ALARM_PARA(SND_ALARM_REL_DLY, CHG_LTV_SND_ALARM_REL_DLY);//config_get(kChgLTVSndAlarmRelDlyIndex)
    
    BMS_SET_ALARM_PARA(TRD_ALARM_COND, BMS_DIAGNOSIS_CHARGE_CONDITION() && bcu_get_average_voltage() > 0 && bcu_get_average_voltage() <= config_get(kChgLTVTrdAlarmIndex));
    BMS_SET_ALARM_PARA(TRD_ALARM_DLY, config_get(kChgLTVTrdAlarmDlyIndex));
    BMS_SET_ALARM_PARA(TRD_ALARM_REL_COND, BMS_DIAGNOSIS_CHARGE_CONDITION()== 0 || bcu_get_average_voltage() > CHG_LTV_TRD_ALARM_REL);//config_get(kChgLTVTrdAlarmRelIndex)
    BMS_SET_ALARM_PARA(TRD_ALARM_REL_DLY, CHG_LTV_TRD_ALARM_REL_DLY);//config_get(kChgLTVTrdAlarmRelDlyIndex)
    
    rule_stock_higher_level_priority_alarm_run(start_index);
    
    //Update charge low total volt state
    bcu_set_chg_ltv_state(rule_stock_get_var(start_index + BMS_DIAG_CUR_STATE_OFFSET));
#else
    bcu_set_chg_ltv_state(bcu_get_dchg_ltv_state());
#endif
}

void bms_dchg_low_total_volt_diagnose(void)
{
    INT16U start_index = BMS_DIAGNOSIS_DCHG_LTV_RAM_POS;

#if BMS_DIAGNOSIS_LTV_INDEPENDENT_WITH_CUR_EN
    BMS_SET_ALARM_PARA(FST_ALARM_COND, BMS_DIAGNOSIS_CHARGE_CONDITION() == 0 && bcu_get_average_voltage() > 0 && bcu_get_average_voltage() <= config_get(kDChgLTVFstAlarmIndex));
    BMS_SET_ALARM_PARA(FST_ALARM_DLY, config_get(kDChgLTVFstAlarmDlyIndex));
    BMS_SET_ALARM_PARA(FST_ALARM_REL_COND, BMS_DIAGNOSIS_CHARGE_CONDITION() || bcu_get_average_voltage() > DCHG_LTV_FST_ALARM_REL);//config_get(kDChgLTVFstAlarmRelIndex)
    BMS_SET_ALARM_PARA(FST_ALARM_REL_DLY, DCHG_LTV_FST_ALARN_REL_DLY);//config_get(kDChgLTVFstAlarmRelDlyIndex)
    
    BMS_SET_ALARM_PARA(SND_ALARM_COND, BMS_DIAGNOSIS_CHARGE_CONDITION() == 0 && bcu_get_average_voltage() > 0 && bcu_get_average_voltage() <= config_get(kDChgLTVSndAlarmIndex));
    BMS_SET_ALARM_PARA(SND_ALARM_DLY, config_get(kDChgLTVSndAlarmDlyIndex));
    BMS_SET_ALARM_PARA(SND_ALARM_REL_COND, BMS_DIAGNOSIS_CHARGE_CONDITION() || bcu_get_average_voltage() > DCHG_LTV_SND_ALARM_REL);//config_get(kDChgLTVSndAlarmRelIndex)
    BMS_SET_ALARM_PARA(SND_ALARM_REL_DLY, DCHG_LTV_SND_ALARM_REL_DLY);//config_get(kDChgLTVSndAlarmRelDlyIndex)
    
    BMS_SET_ALARM_PARA(TRD_ALARM_COND, BMS_DIAGNOSIS_CHARGE_CONDITION() == 0 && bcu_get_average_voltage() > 0 && bcu_get_average_voltage() <= config_get(kDChgLTVTrdAlarmIndex));
    BMS_SET_ALARM_PARA(TRD_ALARM_DLY, config_get(kDChgLTVTrdAlarmDlyIndex));
    BMS_SET_ALARM_PARA(TRD_ALARM_REL_COND, BMS_DIAGNOSIS_CHARGE_CONDITION() || bcu_get_average_voltage() > DCHG_LTV_TRD_ALARM_REL);//config_get(kDChgLTVTrdAlarmRelIndex)
    BMS_SET_ALARM_PARA(TRD_ALARM_REL_DLY, DCHG_LTV_TRD_ALARM_REL_DLY);//config_get(kDChgLTVTrdAlarmRelDlyIndex)
#else
    BMS_SET_ALARM_PARA(FST_ALARM_COND, bcu_get_average_voltage() > 0 && bcu_get_average_voltage() <= config_get(kDChgLTVFstAlarmIndex));
    BMS_SET_ALARM_PARA(FST_ALARM_DLY, config_get(kDChgLTVFstAlarmDlyIndex));
    BMS_SET_ALARM_PARA(FST_ALARM_REL_COND, bcu_get_average_voltage() > DCHG_LTV_FST_ALARM_REL);//config_get(kDChgLTVFstAlarmRelIndex)
    BMS_SET_ALARM_PARA(FST_ALARM_REL_DLY, DCHG_LTV_FST_ALARN_REL_DLY);//config_get(kDChgLTVFstAlarmRelDlyIndex)
    
    BMS_SET_ALARM_PARA(SND_ALARM_COND, bcu_get_average_voltage() > 0 && bcu_get_average_voltage() <= config_get(kDChgLTVSndAlarmIndex));
    BMS_SET_ALARM_PARA(SND_ALARM_DLY, config_get(kDChgLTVSndAlarmDlyIndex));
    BMS_SET_ALARM_PARA(SND_ALARM_REL_COND, bcu_get_average_voltage() > DCHG_LTV_SND_ALARM_REL);//config_get(kDChgLTVSndAlarmRelIndex)
    BMS_SET_ALARM_PARA(SND_ALARM_REL_DLY, DCHG_LTV_SND_ALARM_REL_DLY);//config_get(kDChgLTVSndAlarmRelDlyIndex)
    
    BMS_SET_ALARM_PARA(TRD_ALARM_COND, bcu_get_average_voltage() > 0 && bcu_get_average_voltage() <= config_get(kDChgLTVTrdAlarmIndex));
    BMS_SET_ALARM_PARA(TRD_ALARM_DLY, config_get(kDChgLTVTrdAlarmDlyIndex));
    BMS_SET_ALARM_PARA(TRD_ALARM_REL_COND, bcu_get_average_voltage() > DCHG_LTV_TRD_ALARM_REL);//config_get(kDChgLTVTrdAlarmRelIndex)
    BMS_SET_ALARM_PARA(TRD_ALARM_REL_DLY, DCHG_LTV_TRD_ALARM_REL_DLY);//config_get(kDChgLTVTrdAlarmRelDlyIndex)
#endif
    rule_stock_higher_level_priority_alarm_run(start_index);
    
    //Update discharge low total volt state
    bcu_set_dchg_ltv_state(rule_stock_get_var(start_index + BMS_DIAG_CUR_STATE_OFFSET));
}

AlarmLevel bcu_get_low_total_volt_state(void)
{
    AlarmLevel chg_state, dchg_state;
    
    chg_state = bcu_get_chg_ltv_state();
    dchg_state = bcu_get_dchg_ltv_state();
    
    return chg_state > dchg_state ? chg_state : dchg_state;
}
