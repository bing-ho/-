/**
*
* Copyright (c) 2014 Ligoo Inc.
*
* @file bms_diagnosis_temp_line.c
* @brief
* @note
* @author Liwei Dong
* @date 2014-2-19
*
*/
#include "bms_diagnosis_temp_line.h"
#include "bms_charger_temperature.h"

#pragma MESSAGE DISABLE C12056 // SP debug info incorrect because of optimization or inline assembler

void bms_temp_line_diagnose(void)
{
    INT16U start_index = BMS_DIAGNOSIS_TEMP_LINE_RAM_POS;
    INT16U diff;
    
    BMS_SET_ALARM_PARA(FST_ALARM_COND, bcu_get_temp_exception());
    BMS_SET_ALARM_PARA(FST_ALARM_DLY, config_get(kTLineFstAlarmDlyIndex));
    BMS_SET_ALARM_PARA(FST_ALARM_REL_COND, bcu_get_temp_exception() == 0);
    BMS_SET_ALARM_PARA(FST_ALARM_REL_DLY, TLINE_FST_ALARM_REL_DLY);//config_get(kTLineFstAlarmRelDlyIndex)
    
    BMS_SET_ALARM_PARA(SND_ALARM_COND, bcu_get_temp_exception() && BMS_GET_ALARM_STATE() == kAlarmFirstLevel);
    diff = TEMP_LINE_SND_ALARM_DLY_DEF- config_get(kTLineFstAlarmDlyIndex);//config_get(kTempLineSndAlarmDlyIndex) 
    if(!INT16U_IS_NEGATIVE(diff))
        diff = diff;
    else
        diff = SYS_DEFAULT_ALARM_DLY_MIN_DEF;
    BMS_SET_ALARM_PARA(SND_ALARM_DLY, diff);
    BMS_SET_ALARM_PARA(SND_ALARM_REL_COND, bcu_get_temp_exception() == 0);
    BMS_SET_ALARM_PARA(SND_ALARM_REL_DLY, TEMP_LINE_SND_ALARM_REL_DLY_DEF);//config_get(kTempLineSndAlarmRelDlyIndex)
    
    BMS_SET_ALARM_PARA(TRD_ALARM_COND, bcu_get_temp_exception() > 0 && BMS_GET_ALARM_STATE() == kAlarmSecondLevel);
    diff = TEMP_LINE_TRD_ALARM_DLY_DEF- TEMP_LINE_SND_ALARM_DLY_DEF;//config_get(kTempLineTrdAlarmDlyIndex)-config_get(kTempLineSndAlarmDlyIndex);
    if(!INT16U_IS_NEGATIVE(diff))
        diff = diff;
    else
        diff = SYS_DEFAULT_ALARM_DLY_MIN_DEF;
    BMS_SET_ALARM_PARA(TRD_ALARM_DLY, diff);
    BMS_SET_ALARM_PARA(TRD_ALARM_REL_COND, bcu_get_temp_exception() == 0);
    BMS_SET_ALARM_PARA(TRD_ALARM_REL_DLY, TEMP_LINE_TRD_ALARM_REL_DLY_DEF);//config_get(kTempLineTrdAlarmRelDlyIndex)
    
    rule_stock_higher_level_priority_alarm_run(start_index);
    
    //Update temperature exception state
    bcu_set_temp_exception_state(rule_stock_get_var(start_index + BMS_DIAG_CUR_STATE_OFFSET));
}

void bms_chgr_ac_outlet_temp_line_diagnose(void)
{
    INT16U start_index = BMS_DIAGNOSIS_CHGR_AC_OUTLET_TEMP_LINE_RAM_POS;
    INT16U diff;
    INT8U condition;
    
    condition = bms_get_chgr_ac_outlet_temperature_exception();
    
    BMS_SET_ALARM_PARA(FST_ALARM_COND, BMS_DIAGNOSIS_AC_CHARGE_CONDITION() && condition != 0);
    BMS_SET_ALARM_PARA(FST_ALARM_DLY, config_get(kChgrOutletTempLineFstAlarmDlyIndex));
    BMS_SET_ALARM_PARA(FST_ALARM_REL_COND, BMS_DIAGNOSIS_AC_CHARGE_CONDITION()==0 || condition == 0);
    BMS_SET_ALARM_PARA(FST_ALARM_REL_DLY, CHGR_OUTLET_TEMP_LINE_FST_ALARM_REL_DLY);//config_get(kChgrOutletTempLineFstAlarmRelDlyIndex)
    
    BMS_SET_ALARM_PARA(SND_ALARM_COND, BMS_DIAGNOSIS_AC_CHARGE_CONDITION() && condition != 0 && BMS_GET_ALARM_STATE() == kAlarmFirstLevel);
    diff = config_get(kChgrOutletTempLineSndAlarmDlyIndex) - config_get(kChgrOutletTempLineFstAlarmDlyIndex);
    if(!INT16U_IS_NEGATIVE(diff))
        diff = diff;
    else
        diff = SYS_DEFAULT_ALARM_DLY_MIN_DEF;
    BMS_SET_ALARM_PARA(SND_ALARM_DLY, diff);
    BMS_SET_ALARM_PARA(SND_ALARM_REL_COND, BMS_DIAGNOSIS_AC_CHARGE_CONDITION()==0 || condition == 0);
    BMS_SET_ALARM_PARA(SND_ALARM_REL_DLY, CHGR_OUTLET_TEMP_LINE_SND_ALARM_REL_DLY);//config_get(kChgrOutletTempLineSndAlarmRelDlyIndex)
    
    BMS_SET_ALARM_PARA(TRD_ALARM_COND, BMS_DIAGNOSIS_AC_CHARGE_CONDITION() && condition != 0 && BMS_GET_ALARM_STATE() == kAlarmSecondLevel);
    diff = TEMP_LINE_TRD_ALARM_DLY_DEF - config_get(kChgrOutletTempLineSndAlarmDlyIndex);//config_get(kChgrOutletTempLineTrdAlarmDlyIndex)
    if(!INT16U_IS_NEGATIVE(diff))
        diff = diff;
    else
        diff = SYS_DEFAULT_ALARM_DLY_MIN_DEF;
    BMS_SET_ALARM_PARA(TRD_ALARM_DLY, diff);
    BMS_SET_ALARM_PARA(TRD_ALARM_REL_COND, BMS_DIAGNOSIS_AC_CHARGE_CONDITION()==0 || condition == 0);
    BMS_SET_ALARM_PARA(TRD_ALARM_REL_DLY, TEMP_LINE_TRD_ALARM_REL_DLY_DEF);//config_get(kChgrOutletTempLineTrdAlarmRelDlyIndex)
    
    rule_stock_higher_level_priority_alarm_run(start_index);
    
    //Update temperature exception state
    bcu_set_chgr_ac_outlet_temp_exception_state(rule_stock_get_var(start_index + BMS_DIAG_CUR_STATE_OFFSET));
}

void bms_chgr_dc_outlet_temp_line_diagnose(void)
{
    INT16U start_index = BMS_DIAGNOSIS_CHGR_DC_OUTLET_TEMP_LINE_RAM_POS;
    INT16U diff;
    INT8U condition;
    
    condition = bms_get_chgr_dc_outlet_temperature_exception();
    
    BMS_SET_ALARM_PARA(FST_ALARM_COND, BMS_DIAGNOSIS_DC_CHARGE_CONDITION() && condition != 0);
    BMS_SET_ALARM_PARA(FST_ALARM_DLY, config_get(kChgrOutletTempLineFstAlarmDlyIndex));
    BMS_SET_ALARM_PARA(FST_ALARM_REL_COND, BMS_DIAGNOSIS_DC_CHARGE_CONDITION() == 0 || condition == 0);
    BMS_SET_ALARM_PARA(FST_ALARM_REL_DLY, CHGR_OUTLET_TEMP_LINE_FST_ALARM_REL_DLY);//config_get(kChgrOutletTempLineFstAlarmRelDlyIndex)
    
    BMS_SET_ALARM_PARA(SND_ALARM_COND, BMS_DIAGNOSIS_DC_CHARGE_CONDITION() && condition != 0 && BMS_GET_ALARM_STATE() == kAlarmFirstLevel);
    diff = config_get(kChgrOutletTempLineSndAlarmDlyIndex) - config_get(kChgrOutletTempLineFstAlarmDlyIndex);
    if(!INT16U_IS_NEGATIVE(diff))
        diff = diff;
    else
        diff = SYS_DEFAULT_ALARM_DLY_MIN_DEF;
    BMS_SET_ALARM_PARA(SND_ALARM_DLY, diff);
    BMS_SET_ALARM_PARA(SND_ALARM_REL_COND, BMS_DIAGNOSIS_DC_CHARGE_CONDITION() == 0 || condition == 0);
    BMS_SET_ALARM_PARA(SND_ALARM_REL_DLY, CHGR_OUTLET_TEMP_LINE_SND_ALARM_REL_DLY);//config_get(kChgrOutletTempLineSndAlarmRelDlyIndex)
    
    BMS_SET_ALARM_PARA(TRD_ALARM_COND, BMS_DIAGNOSIS_DC_CHARGE_CONDITION() && condition != 0 && BMS_GET_ALARM_STATE() == kAlarmSecondLevel);
    diff = TEMP_LINE_TRD_ALARM_DLY_DEF - config_get(kChgrOutletTempLineSndAlarmDlyIndex);//config_get(kChgrOutletTempLineTrdAlarmDlyIndex)
    if(!INT16U_IS_NEGATIVE(diff))
        diff = diff;
    else
        diff = SYS_DEFAULT_ALARM_DLY_MIN_DEF;
    BMS_SET_ALARM_PARA(TRD_ALARM_DLY, diff);
    BMS_SET_ALARM_PARA(TRD_ALARM_REL_COND, BMS_DIAGNOSIS_DC_CHARGE_CONDITION() == 0 || condition == 0);
    BMS_SET_ALARM_PARA(TRD_ALARM_REL_DLY, TEMP_LINE_TRD_ALARM_REL_DLY_DEF);//config_get(kChgrOutletTempLineTrdAlarmRelDlyIndex)
    
    rule_stock_higher_level_priority_alarm_run(start_index);
    
    //Update temperature exception state
    bcu_set_chgr_dc_outlet_temp_exception_state(rule_stock_get_var(start_index + BMS_DIAG_CUR_STATE_OFFSET));
}

AlarmLevel bcu_get_chgr_outlet_temp_line_state(void)
{
    AlarmLevel ac_state, dc_state;
    
    ac_state = bcu_get_chgr_ac_outlet_temp_exception_state();
    dc_state = bcu_get_chgr_dc_outlet_temp_exception_state();
    
    return ac_state > dc_state ? ac_state : dc_state;
}

