/**
*
* Copyright (c) 2014 Ligoo Inc.
*
* @file bms_diagnosis_delta_temperature.c
* @brief
* @note
* @author Liwei Dong
* @date 2014-2-19
*
*/
#include "bms_diagnosis_delta_temperature.h"

#pragma MESSAGE DISABLE C12056 // SP debug info incorrect because of optimization or inline assembler

void bms_chg_delta_temperature_diagnose(void);
void bms_dchg_delta_temperature_diagnose(void);

void bms_delta_temperature_diagnose(void)
{
    bms_dchg_delta_temperature_diagnose();
    bms_chg_delta_temperature_diagnose();
}

void bms_chg_delta_temperature_diagnose(void)
{
#if BMS_DIAGNOSIS_HDT_INDEPENDENT_WITH_CUR_EN
    INT16U start_index = BMS_DIAGNOSIS_CHG_DELTA_TEMP_RAM_POS;
    
    BMS_SET_ALARM_PARA(FST_ALARM_COND, BMS_DIAGNOSIS_CHARGE_CONDITION() && bcu_get_difference_temperature() >= config_get(kChgDTFstAlarmIndex));
    BMS_SET_ALARM_PARA(FST_ALARM_DLY, config_get(kChgDTFstAlarmDlyIndex));
    BMS_SET_ALARM_PARA(FST_ALARM_REL_COND, BMS_DIAGNOSIS_CHARGE_CONDITION()== 0 || bcu_get_difference_temperature() < CHG_DT_FST_ALARM_REL);//config_get(kChgDTFstAlarmRelIndex)
    BMS_SET_ALARM_PARA(FST_ALARM_REL_DLY, CHG_DT_FST_ALARM_REL_DLY);//config_get(kChgDTFstAlarmRelDlyIndex)
    
    BMS_SET_ALARM_PARA(SND_ALARM_COND, BMS_DIAGNOSIS_CHARGE_CONDITION() && bcu_get_difference_temperature() >= config_get(kChgDTSndAlarmIndex));
    BMS_SET_ALARM_PARA(SND_ALARM_DLY, config_get(kChgDTSndAlarmDlyIndex));
    BMS_SET_ALARM_PARA(SND_ALARM_REL_COND, BMS_DIAGNOSIS_CHARGE_CONDITION()== 0 || bcu_get_difference_temperature() < CHG_DT_SND_ALARM_REL);//config_get(kChgDTSndAlarmRelIndex)
    BMS_SET_ALARM_PARA(SND_ALARM_REL_DLY, CHG_DT_SND_ALARM_REL_DLY);//config_get(kChgDTSndAlarmRelDlyIndex)
    
    BMS_SET_ALARM_PARA(TRD_ALARM_COND, BMS_DIAGNOSIS_CHARGE_CONDITION() && bcu_get_difference_temperature() >= config_get(kChgDTTrdAlarmIndex));
    BMS_SET_ALARM_PARA(TRD_ALARM_DLY, config_get(kChgDTTrdAlarmDlyIndex));
    BMS_SET_ALARM_PARA(TRD_ALARM_REL_COND, BMS_DIAGNOSIS_CHARGE_CONDITION()== 0 || bcu_get_difference_temperature() < CHG_DT_TRD_ALARM_REL);//config_get(kChgDTTrdAlarmRelIndex)
    BMS_SET_ALARM_PARA(TRD_ALARM_REL_DLY, CHG_DT_TRD_ALARM_REL_DLY);//config_get(kChgDTTrdAlarmRelDlyIndex)
    
    rule_stock_higher_level_priority_alarm_run(start_index);
    
    //Update battery delta temperature state
    bcu_set_chg_delta_temp_state(rule_stock_get_var(start_index + BMS_DIAG_CUR_STATE_OFFSET));
#else
    bcu_set_chg_delta_temp_state(bcu_get_dchg_delta_temp_state());
#endif
}

void bms_dchg_delta_temperature_diagnose(void)
{
    INT16U start_index = BMS_DIAGNOSIS_DCHG_DELTA_TEMP_RAM_POS;

#if BMS_DIAGNOSIS_HDT_INDEPENDENT_WITH_CUR_EN    
    BMS_SET_ALARM_PARA(FST_ALARM_COND, BMS_DIAGNOSIS_CHARGE_CONDITION() == 0 && bcu_get_difference_temperature() >= config_get(kDChgDTFstAlarmIndex));
    BMS_SET_ALARM_PARA(FST_ALARM_DLY, config_get(kDChgDTFstAlarmDlyIndex));
    BMS_SET_ALARM_PARA(FST_ALARM_REL_COND, BMS_DIAGNOSIS_CHARGE_CONDITION() || (bcu_get_difference_temperature() < DCHG_DT_FST_ALARM_REL));//config_get(kDChgDTFstAlarmRelIndex)
    BMS_SET_ALARM_PARA(FST_ALARM_REL_DLY, DCHG_DT_FST_ALARM_REL_DLY);//config_get(kDChgDTFstAlarmRelDlyIndex)
    
    BMS_SET_ALARM_PARA(SND_ALARM_COND, BMS_DIAGNOSIS_CHARGE_CONDITION() == 0 && bcu_get_difference_temperature() >= config_get(kDChgDTSndAlarmIndex));
    BMS_SET_ALARM_PARA(SND_ALARM_DLY, config_get(kDChgDTSndAlarmDlyIndex));
    BMS_SET_ALARM_PARA(SND_ALARM_REL_COND, BMS_DIAGNOSIS_CHARGE_CONDITION() || (bcu_get_difference_temperature() < DCHG_DT_SND_ALARM_REL));//config_get(kDChgDTSndAlarmRelIndex)
    BMS_SET_ALARM_PARA(SND_ALARM_REL_DLY, DCHG_DT_SND_ALARM_REL_DLY);//config_get(kDChgDTSndAlarmRelDlyIndex)
    
    BMS_SET_ALARM_PARA(TRD_ALARM_COND, BMS_DIAGNOSIS_CHARGE_CONDITION() == 0 && bcu_get_difference_temperature() >= config_get(kDChgDTTrdAlarmIndex));
    BMS_SET_ALARM_PARA(TRD_ALARM_DLY, config_get(kDChgDTTrdAlarmDlyIndex));
    BMS_SET_ALARM_PARA(TRD_ALARM_REL_COND, BMS_DIAGNOSIS_CHARGE_CONDITION() || (bcu_get_difference_temperature() < DCHG_DT_TRD_ALARM_REL));//config_get(kDChgDTTrdAlarmRelIndex)
    BMS_SET_ALARM_PARA(TRD_ALARM_REL_DLY, DCHG_DT_TRD_ALARM_REL_DLY);//config_get(kDChgDTTrdAlarmRelDlyIndex)
#else
    BMS_SET_ALARM_PARA(FST_ALARM_COND, bcu_get_difference_temperature() >= config_get(kDChgDTFstAlarmIndex));
    BMS_SET_ALARM_PARA(FST_ALARM_DLY, config_get(kDChgDTFstAlarmDlyIndex));
    BMS_SET_ALARM_PARA(FST_ALARM_REL_COND, bcu_get_difference_temperature() < DCHG_DT_FST_ALARM_REL);//config_get(kDChgDTFstAlarmRelIndex)
    BMS_SET_ALARM_PARA(FST_ALARM_REL_DLY, DCHG_DT_FST_ALARM_REL_DLY);//config_get(kDChgDTFstAlarmRelDlyIndex)
    
    BMS_SET_ALARM_PARA(SND_ALARM_COND, bcu_get_difference_temperature() >= config_get(kDChgDTSndAlarmIndex));
    BMS_SET_ALARM_PARA(SND_ALARM_DLY, config_get(kDChgDTSndAlarmDlyIndex));
    BMS_SET_ALARM_PARA(SND_ALARM_REL_COND, bcu_get_difference_temperature() < DCHG_DT_SND_ALARM_REL);//config_get(kDChgDTSndAlarmRelIndex)
    BMS_SET_ALARM_PARA(SND_ALARM_REL_DLY, DCHG_DT_SND_ALARM_REL_DLY);//config_get(kDChgDTSndAlarmRelDlyIndex)
    
    BMS_SET_ALARM_PARA(TRD_ALARM_COND, bcu_get_difference_temperature() >= config_get(kDChgDTTrdAlarmIndex));
    BMS_SET_ALARM_PARA(TRD_ALARM_DLY, config_get(kDChgDTTrdAlarmDlyIndex));
    BMS_SET_ALARM_PARA(TRD_ALARM_REL_COND, bcu_get_difference_temperature() < DCHG_DT_TRD_ALARM_REL);//config_get(kDChgDTTrdAlarmRelIndex)
    BMS_SET_ALARM_PARA(TRD_ALARM_REL_DLY, DCHG_DT_TRD_ALARM_REL_DLY);//config_get(kDChgDTTrdAlarmRelDlyIndex)
#endif
    rule_stock_higher_level_priority_alarm_run(start_index);
    
    //Update battery delta temperature state
    bcu_set_dchg_delta_temp_state(rule_stock_get_var(start_index + BMS_DIAG_CUR_STATE_OFFSET));
}

AlarmLevel bcu_get_delta_temperature_state(void)
{
    AlarmLevel chg_state, dchg_state;
    
    chg_state = bcu_get_chg_delta_temp_state();
    dchg_state = bcu_get_dchg_delta_temp_state();
    
    return chg_state > dchg_state ? chg_state : dchg_state;
}
