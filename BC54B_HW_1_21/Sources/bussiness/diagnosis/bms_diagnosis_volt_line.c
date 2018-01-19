/**
*
* Copyright (c) 2014 Ligoo Inc.
*
* @file bms_diagnosis_volt_line.c
* @brief
* @note
* @author Liwei Dong
* @date 2014-2-19
*
*/
#include "bms_diagnosis_volt_line.h"

#pragma MESSAGE DISABLE C12056 // SP debug info incorrect because of optimization or inline assembler

void bms_volt_line_diagnose(void)
{
    INT16U start_index = BMS_DIAGNOSIS_VOLT_LINE_RAM_POS;
    INT16U diff;
    
    BMS_SET_ALARM_PARA(FST_ALARM_COND, bcu_get_voltage_exception());
    BMS_SET_ALARM_PARA(FST_ALARM_DLY, config_get(kVLineFstAlarmDlyIndex));
    BMS_SET_ALARM_PARA(FST_ALARM_REL_COND, bcu_get_voltage_exception() == 0);
    BMS_SET_ALARM_PARA(FST_ALARM_REL_DLY, VLINE_FST_ALARM_REL_DLY);//config_get(kVLineFstAlarmRelDlyIndex)
    
    BMS_SET_ALARM_PARA(SND_ALARM_COND, bcu_get_voltage_exception() && BMS_GET_ALARM_STATE() == kAlarmFirstLevel);
    diff = VOLT_LINE_SND_ALARM_DLY_DEF- config_get(kVLineFstAlarmDlyIndex);//config_get(kVoltLineSndAlarmDlyIndex)
    if(!INT16U_IS_NEGATIVE(diff))
        diff = diff;
    else
        diff = SYS_DEFAULT_ALARM_DLY_MIN_DEF;
    BMS_SET_ALARM_PARA(SND_ALARM_DLY, diff);
    BMS_SET_ALARM_PARA(SND_ALARM_REL_COND, bcu_get_voltage_exception() == 0);
    BMS_SET_ALARM_PARA(SND_ALARM_REL_DLY, VOLT_LINE_SND_ALARM_REL_DLY_DEF);//config_get(kVoltLineSndAlarmRelDlyIndex)
    
    BMS_SET_ALARM_PARA(TRD_ALARM_COND, bcu_get_voltage_exception() > 0 && BMS_GET_ALARM_STATE() == kAlarmSecondLevel);
    diff = VOLT_LINE_TRD_ALARM_DLY_DEF-VOLT_LINE_SND_ALARM_DLY_DEF;//config_get(kVoltLineTrdAlarmDlyIndex)-config_get(kVoltLineSndAlarmDlyIndex);
    if(!INT16U_IS_NEGATIVE(diff))
        diff = diff;
    else
        diff = SYS_DEFAULT_ALARM_DLY_MIN_DEF;
    BMS_SET_ALARM_PARA(TRD_ALARM_DLY, diff);
    BMS_SET_ALARM_PARA(TRD_ALARM_REL_COND, bcu_get_voltage_exception() == 0);
    BMS_SET_ALARM_PARA(TRD_ALARM_REL_DLY, VOLT_LINE_TRD_ALARM_REL_DLY_DEF);//config_get(kVoltLineTrdAlarmRelDlyIndex)
    
    rule_stock_higher_level_priority_alarm_run(start_index);
    
    //Update voltage exception state
    bcu_set_voltage_exception_state(rule_stock_get_var(start_index + BMS_DIAG_CUR_STATE_OFFSET));
}

