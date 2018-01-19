/**
*
* Copyright (c) 2014 Ligoo Inc.
*
* @file bms_diagnosis_insulation.c
* @brief
* @note
* @author Liwei Dong
* @date 2014-2-19
*
*/
#include "bms_diagnosis_insulation.h"

#pragma MESSAGE DISABLE C12056 // SP debug info incorrect because of optimization or inline assembler

void bms_insulation_diagnose(void)
{
    INT16U start_index = BMS_DIAGNOSIS_INSULATION_RAM_POS;
    
    BMS_SET_ALARM_PARA(FST_ALARM_COND, bcu_get_system_insulation_resistance() <= (MV_TO_V(bcu_get_insulation_total_voltage()) * config_get(kInsuFstAlarmIndex) / INSULATION_MODULE_UNIT));
    BMS_SET_ALARM_PARA(FST_ALARM_DLY, config_get(kInsuFstAlarmDlyIndex));
    BMS_SET_ALARM_PARA(FST_ALARM_REL_COND, bcu_get_system_insulation_resistance() > (MV_TO_V(bcu_get_insulation_total_voltage()) * INSU_FST_ALARM_REL/ INSULATION_MODULE_UNIT));//config_get(kInsuFstAlarmRelIndex)
    BMS_SET_ALARM_PARA(FST_ALARM_REL_DLY, INSU_FST_ALARM_REL_DLY);//config_get(kInsuFstAlarmRelDlyIndex)
    
    BMS_SET_ALARM_PARA(SND_ALARM_COND, bcu_get_system_insulation_resistance() <= (MV_TO_V(bcu_get_insulation_total_voltage()) * config_get(kInsuSndAlarmIndex) / INSULATION_MODULE_UNIT));
    BMS_SET_ALARM_PARA(SND_ALARM_DLY, config_get(kInsuSndAlarmDlyIndex));
    BMS_SET_ALARM_PARA(SND_ALARM_REL_COND, bcu_get_system_insulation_resistance() > (MV_TO_V(bcu_get_insulation_total_voltage()) * INSU_SND_ALARM_REL / INSULATION_MODULE_UNIT));//config_get(kInsuSndAlarmRelIndex)
    BMS_SET_ALARM_PARA(SND_ALARM_REL_DLY, INSU_SND_ALARM_REL_DLY);//config_get(kInsuSndAlarmRelDlyIndex)
    
    BMS_SET_ALARM_PARA(TRD_ALARM_COND, bcu_get_system_insulation_resistance() <= (MV_TO_V(bcu_get_insulation_total_voltage()) * config_get(kInsuTrdAlarmIndex) / INSULATION_MODULE_UNIT));
    BMS_SET_ALARM_PARA(TRD_ALARM_DLY, config_get(kInsuTrdAlarmDlyIndex));
    BMS_SET_ALARM_PARA(TRD_ALARM_REL_COND, bcu_get_system_insulation_resistance() > (MV_TO_V(bcu_get_insulation_total_voltage()) * INSU_TRD_ALARM_REL/ INSULATION_MODULE_UNIT));//config_get(kInsuTrdAlarmRelIndex)
    BMS_SET_ALARM_PARA(TRD_ALARM_REL_DLY, INSU_TRD_ALARM_REL_DLY);//config_get(kInsuTrdAlarmRelDlyIndex)
    
    rule_stock_higher_level_priority_alarm_run(start_index);
    
    //Update battery insulation state
    bcu_set_battery_insulation_state(rule_stock_get_var(start_index + BMS_DIAG_CUR_STATE_OFFSET));
}
