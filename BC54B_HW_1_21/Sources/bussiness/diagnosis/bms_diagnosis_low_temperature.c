/**
*
* Copyright (c) 2014 Ligoo Inc.
*
* @file bms_diagnosis_low_temperature.c
* @brief
* @note
* @author Liwei Dong
* @date 2014-2-19
*
*/
#include "bms_diagnosis_low_temperature.h"

#pragma MESSAGE DISABLE C12056 // SP debug info incorrect because of optimization or inline assembler

void bms_chg_low_temperature_diagnose(void);
void bms_dchg_low_temperature_diagnose(void);

void bms_low_temperature_diagnose(void)
{
    bms_dchg_low_temperature_diagnose();
    bms_chg_low_temperature_diagnose();
}

void bms_chg_low_temperature_diagnose(void)
{
#if BMS_DIAGNOSIS_LT_INDEPENDENT_WITH_CUR_EN
    INT16U start_index = BMS_DIAGNOSIS_CHG_LOW_TEMP_RAM_POS;

    BMS_SET_ALARM_PARA(FST_ALARM_COND, BMS_DIAGNOSIS_CHARGE_CONDITION() && bcu_get_low_temperature() <= config_get(kChgLTFstAlarmIndex));
    BMS_SET_ALARM_PARA(FST_ALARM_DLY, config_get(kChgLTFstAlarmDlyIndex));
    BMS_SET_ALARM_PARA(FST_ALARM_REL_COND, BMS_DIAGNOSIS_CHARGE_CONDITION()== 0 || bcu_get_low_temperature() >CHG_LT_FST_ALARM_REL );//config_get(kChgLTFstAlarmRelIndex)
    BMS_SET_ALARM_PARA(FST_ALARM_REL_DLY, CHG_LT_FST_ALARM_REL_DLY);//config_get(kChgLTFstAlarmRelDlyIndex)
    
    BMS_SET_ALARM_PARA(SND_ALARM_COND, BMS_DIAGNOSIS_CHARGE_CONDITION() && bcu_get_low_temperature() <= config_get(kChgLTSndAlarmIndex));
    BMS_SET_ALARM_PARA(SND_ALARM_DLY, config_get(kChgLTSndAlarmDlyIndex));
    BMS_SET_ALARM_PARA(SND_ALARM_REL_COND, BMS_DIAGNOSIS_CHARGE_CONDITION()== 0 || bcu_get_low_temperature() >CHG_LT_SND_ALARM_REL);//config_get(kChgLTSndAlarmRelIndex)
    BMS_SET_ALARM_PARA(SND_ALARM_REL_DLY, CHG_LT_SND_ALARM_REL_DLY);//config_get(kChgLTSndAlarmRelDlyIndex)
    
    BMS_SET_ALARM_PARA(TRD_ALARM_COND, BMS_DIAGNOSIS_CHARGE_CONDITION() && bcu_get_low_temperature() <= config_get(kChgLTTrdAlarmIndex));
    BMS_SET_ALARM_PARA(TRD_ALARM_DLY, config_get(kChgLTTrdAlarmDlyIndex));
    BMS_SET_ALARM_PARA(TRD_ALARM_REL_COND, BMS_DIAGNOSIS_CHARGE_CONDITION()== 0 || bcu_get_low_temperature() > CHG_LT_TRD_ALARM_REL);//config_get(kChgLTTrdAlarmRelIndex)
    BMS_SET_ALARM_PARA(TRD_ALARM_REL_DLY, CHG_LT_TRD_ALARM_REL_DLY);//config_get(kChgLTTrdAlarmRelDlyIndex)
    
    rule_stock_higher_level_priority_alarm_run(start_index);
    
    //Update battery low temperature state
    bcu_set_chg_lt_state(rule_stock_get_var(start_index + BMS_DIAG_CUR_STATE_OFFSET));
#else
    bcu_set_chg_lt_state(bcu_get_dchg_lt_state());
#endif
}

void bms_dchg_low_temperature_diagnose(void)
{
    INT16U start_index = BMS_DIAGNOSIS_DCHG_LOW_TEMP_RAM_POS;

#if BMS_DIAGNOSIS_LT_INDEPENDENT_WITH_CUR_EN
    BMS_SET_ALARM_PARA(FST_ALARM_COND, BMS_DIAGNOSIS_CHARGE_CONDITION() == 0 && bcu_get_low_temperature() <= config_get(kDChgLTFstAlarmIndex));
    BMS_SET_ALARM_PARA(FST_ALARM_DLY, config_get(kDChgLTFstAlarmDlyIndex));
    BMS_SET_ALARM_PARA(FST_ALARM_REL_COND, BMS_DIAGNOSIS_CHARGE_CONDITION() || (bcu_get_low_temperature() > DCHG_LT_FST_ALARM_REL));//config_get(kDChgLTFstAlarmRelIndex)
    BMS_SET_ALARM_PARA(FST_ALARM_REL_DLY, DCHG_LT_FST_ALARM_REL_DLY);//config_get(kDChgLTFstAlarmRelDlyIndex)
    
    BMS_SET_ALARM_PARA(SND_ALARM_COND, BMS_DIAGNOSIS_CHARGE_CONDITION() == 0 && bcu_get_low_temperature() <= config_get(kDChgLTSndAlarmIndex));
    BMS_SET_ALARM_PARA(SND_ALARM_DLY, config_get(kDChgLTSndAlarmDlyIndex));
    BMS_SET_ALARM_PARA(SND_ALARM_REL_COND, BMS_DIAGNOSIS_CHARGE_CONDITION() || (bcu_get_low_temperature() > DCHG_LT_SND_ALARM_REL));//config_get(kDChgLTSndAlarmRelIndex)
    BMS_SET_ALARM_PARA(SND_ALARM_REL_DLY, DCHG_LT_SND_ALARM_REL_DLY);//config_get(kDChgLTSndAlarmRelDlyIndex)
    
    BMS_SET_ALARM_PARA(TRD_ALARM_COND, BMS_DIAGNOSIS_CHARGE_CONDITION() == 0 && bcu_get_low_temperature() <= config_get(kDChgLTTrdAlarmIndex));
    BMS_SET_ALARM_PARA(TRD_ALARM_DLY, config_get(kDChgLTTrdAlarmDlyIndex));
    BMS_SET_ALARM_PARA(TRD_ALARM_REL_COND, BMS_DIAGNOSIS_CHARGE_CONDITION() || (bcu_get_low_temperature() > DCHG_LT_TRD_ALARM_REL));//config_get(kDChgLTTrdAlarmRelIndex)
    BMS_SET_ALARM_PARA(TRD_ALARM_REL_DLY, DCHG_LT_TRD_ALARM_REL_DLY);//config_get(kDChgLTTrdAlarmRelDlyIndex)
#else
    BMS_SET_ALARM_PARA(FST_ALARM_COND, bcu_get_low_temperature() <= config_get(kDChgLTFstAlarmIndex));
    BMS_SET_ALARM_PARA(FST_ALARM_DLY, config_get(kDChgLTFstAlarmDlyIndex));
    BMS_SET_ALARM_PARA(FST_ALARM_REL_COND, bcu_get_low_temperature() > DCHG_LT_FST_ALARM_REL);//config_get(kDChgLTFstAlarmRelIndex)
    BMS_SET_ALARM_PARA(FST_ALARM_REL_DLY, DCHG_LT_FST_ALARM_REL_DLY);//config_get(kDChgLTFstAlarmRelDlyIndex)
    
    BMS_SET_ALARM_PARA(SND_ALARM_COND, bcu_get_low_temperature() <= config_get(kDChgLTSndAlarmIndex));
    BMS_SET_ALARM_PARA(SND_ALARM_DLY, config_get(kDChgLTSndAlarmDlyIndex));
    BMS_SET_ALARM_PARA(SND_ALARM_REL_COND, bcu_get_low_temperature() > DCHG_LT_SND_ALARM_REL);//config_get(kDChgLTSndAlarmRelIndex)
    BMS_SET_ALARM_PARA(SND_ALARM_REL_DLY, DCHG_LT_SND_ALARM_REL_DLY);//config_get(kDChgLTSndAlarmRelDlyIndex)
    
    BMS_SET_ALARM_PARA(TRD_ALARM_COND, bcu_get_low_temperature() <= config_get(kDChgLTTrdAlarmIndex));
    BMS_SET_ALARM_PARA(TRD_ALARM_DLY, config_get(kDChgLTTrdAlarmDlyIndex));
    BMS_SET_ALARM_PARA(TRD_ALARM_REL_COND, bcu_get_low_temperature() > DCHG_LT_TRD_ALARM_REL);//config_get(kDChgLTTrdAlarmRelIndex)
    BMS_SET_ALARM_PARA(TRD_ALARM_REL_DLY, DCHG_LT_TRD_ALARM_REL_DLY);//config_get(kDChgLTTrdAlarmRelDlyIndex)
#endif    
    rule_stock_higher_level_priority_alarm_run(start_index);
    
    //Update battery low temperature state
    bcu_set_dchg_lt_state(rule_stock_get_var(start_index + BMS_DIAG_CUR_STATE_OFFSET));
}

AlarmLevel bcu_get_low_temperature_state(void)
{
    AlarmLevel chg_state, dchg_state;
    
    chg_state = bcu_get_chg_lt_state();
    dchg_state = bcu_get_dchg_lt_state();
    
    return chg_state > dchg_state ? chg_state : dchg_state;
}
