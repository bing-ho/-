/**
*
* Copyright (c) 2014 Ligoo Inc.
*
* @file bms_diagnosis_high_temperature.c
* @brief
* @note
* @author Liwei Dong
* @date 2014-2-19
*
*/
#include "bms_diagnosis_high_temperature.h"
#include "bms_charger_temperature.h"

#pragma MESSAGE DISABLE C12056 // SP debug info incorrect because of optimization or inline assembler

void bms_chg_high_temperature_diagnose(void);
void bms_dchg_high_temperature_diagnose(void);
void bms_chgr_ac_outlet_high_temperature_diagnose(void);
void bms_chgr_dc_outlet_high_temperature_diagnose(void);

void bms_high_temperature_diagnose(void)
{
    bms_dchg_high_temperature_diagnose();
    bms_chg_high_temperature_diagnose();
    bms_chgr_ac_outlet_high_temperature_diagnose();
    bms_chgr_dc_outlet_high_temperature_diagnose();
}

void bms_chg_high_temperature_diagnose(void)
{
#if BMS_DIAGNOSIS_HT_INDEPENDENT_WITH_CUR_EN
    INT16U start_index = BMS_DIAGNOSIS_CHG_HIGH_TEMP_RAM_POS;
    
    BMS_SET_ALARM_PARA(FST_ALARM_COND, BMS_DIAGNOSIS_CHARGE_CONDITION() && bcu_get_high_temperature() >= config_get(kChgHTFstAlarmIndex));
    BMS_SET_ALARM_PARA(FST_ALARM_DLY, config_get(kChgHTFstAlarmDlyIndex));
    BMS_SET_ALARM_PARA(FST_ALARM_REL_COND, BMS_DIAGNOSIS_CHARGE_CONDITION()== 0 || bcu_get_high_temperature() < CHG_HT_FST_ALARM_REL);//config_get(kChgHTFstAlarmRelIndex)
    BMS_SET_ALARM_PARA(FST_ALARM_REL_DLY, CHG_HT_FST_ALARM_REL_DLY);//config_get(kChgHTFstAlarmRelDlyIndex)
    
    BMS_SET_ALARM_PARA(SND_ALARM_COND, BMS_DIAGNOSIS_CHARGE_CONDITION() && bcu_get_high_temperature() >= config_get(kChgHTSndAlarmIndex));
    BMS_SET_ALARM_PARA(SND_ALARM_DLY, config_get(kChgHTSndAlarmDlyIndex));
    BMS_SET_ALARM_PARA(SND_ALARM_REL_COND, BMS_DIAGNOSIS_CHARGE_CONDITION()== 0 || bcu_get_high_temperature() < CHG_HT_SND_ALARM_REL);//config_get(kChgHTSndAlarmRelIndex)
    BMS_SET_ALARM_PARA(SND_ALARM_REL_DLY, CHG_HT_SND_ALARM_REL_DLY);//config_get(kChgHTSndAlarmRelDlyIndex)
    
    BMS_SET_ALARM_PARA(TRD_ALARM_COND, BMS_DIAGNOSIS_CHARGE_CONDITION() && bcu_get_high_temperature() >= config_get(kChgHTTrdAlarmIndex));
    BMS_SET_ALARM_PARA(TRD_ALARM_DLY, config_get(kChgHTTrdAlarmDlyIndex));
    BMS_SET_ALARM_PARA(TRD_ALARM_REL_COND, BMS_DIAGNOSIS_CHARGE_CONDITION()== 0 || bcu_get_high_temperature() < CHG_HT_TRD_ALARM_REL);//config_get(kChgHTTrdAlarmRelIndex)
    BMS_SET_ALARM_PARA(TRD_ALARM_REL_DLY, CHG_HT_TRD_ALARM_REL_DLY);//config_get(kChgHTTrdAlarmRelDlyIndex)
    
    rule_stock_higher_level_priority_alarm_run(start_index);
    
    //Update battery high temperature state
    bcu_set_chg_ht_state(rule_stock_get_var(start_index + BMS_DIAG_CUR_STATE_OFFSET));
#else
    bcu_set_chg_ht_state(bcu_get_dchg_ht_state());
#endif
}

void bms_dchg_high_temperature_diagnose(void)
{
    INT16U start_index = BMS_DIAGNOSIS_DCHG_HIGH_TEMP_RAM_POS;

#if BMS_DIAGNOSIS_HT_INDEPENDENT_WITH_CUR_EN    
    BMS_SET_ALARM_PARA(FST_ALARM_COND, BMS_DIAGNOSIS_CHARGE_CONDITION() == 0 && bcu_get_high_temperature() >= config_get(kDChgHTFstAlarmIndex));
    BMS_SET_ALARM_PARA(FST_ALARM_DLY, config_get(kDChgHTFstAlarmDlyIndex));
    BMS_SET_ALARM_PARA(FST_ALARM_REL_COND, BMS_DIAGNOSIS_CHARGE_CONDITION() || (bcu_get_high_temperature() < DCHG_HT_FST_ALARM_REL));//config_get(kDChgHTFstAlarmRelIndex)
    BMS_SET_ALARM_PARA(FST_ALARM_REL_DLY, DCHG_HT_FST_ALARM_REL_DLY);//config_get(kDChgHTFstAlarmRelDlyIndex)
    
    BMS_SET_ALARM_PARA(SND_ALARM_COND, BMS_DIAGNOSIS_CHARGE_CONDITION() == 0 && bcu_get_high_temperature() >= config_get(kDChgHTSndAlarmIndex));
    BMS_SET_ALARM_PARA(SND_ALARM_DLY, config_get(kDChgHTSndAlarmDlyIndex));
    BMS_SET_ALARM_PARA(SND_ALARM_REL_COND, BMS_DIAGNOSIS_CHARGE_CONDITION() || (bcu_get_high_temperature() < DCHG_HT_SND_ALARM_REL));//config_get(kDChgHTSndAlarmRelIndex)
    BMS_SET_ALARM_PARA(SND_ALARM_REL_DLY, DCHG_HT_SND_ALARM_REL_DLY);//config_get(kDChgHTSndAlarmRelDlyIndex)
    
    BMS_SET_ALARM_PARA(TRD_ALARM_COND, BMS_DIAGNOSIS_CHARGE_CONDITION() == 0 && bcu_get_high_temperature() >= config_get(kDChgHTTrdAlarmIndex));
    BMS_SET_ALARM_PARA(TRD_ALARM_DLY, config_get(kDChgHTTrdAlarmDlyIndex));
    BMS_SET_ALARM_PARA(TRD_ALARM_REL_COND, BMS_DIAGNOSIS_CHARGE_CONDITION() || (bcu_get_high_temperature() < DCHG_HT_TRD_ALARM_REL));//config_get(kDChgHTTrdAlarmRelIndex)
    BMS_SET_ALARM_PARA(TRD_ALARM_REL_DLY, DCHG_HT_TRD_ALARM_REL_DLY);//config_get(kDChgHTTrdAlarmRelDlyIndex)
#else
    BMS_SET_ALARM_PARA(FST_ALARM_COND, bcu_get_high_temperature() >= config_get(kDChgHTFstAlarmIndex));
    BMS_SET_ALARM_PARA(FST_ALARM_DLY, config_get(kDChgHTFstAlarmDlyIndex));
    BMS_SET_ALARM_PARA(FST_ALARM_REL_COND, bcu_get_high_temperature() < DCHG_HT_FST_ALARM_REL);//config_get(kDChgHTFstAlarmRelIndex)
    BMS_SET_ALARM_PARA(FST_ALARM_REL_DLY, DCHG_HT_FST_ALARM_REL_DLY);//config_get(kDChgHTFstAlarmRelDlyIndex)
    
    BMS_SET_ALARM_PARA(SND_ALARM_COND, bcu_get_high_temperature() >= config_get(kDChgHTSndAlarmIndex));
    BMS_SET_ALARM_PARA(SND_ALARM_DLY, config_get(kDChgHTSndAlarmDlyIndex));
    BMS_SET_ALARM_PARA(SND_ALARM_REL_COND, bcu_get_high_temperature() < DCHG_HT_SND_ALARM_REL);//config_get(kDChgHTSndAlarmRelIndex)
    BMS_SET_ALARM_PARA(SND_ALARM_REL_DLY, DCHG_HT_SND_ALARM_REL_DLY);//config_get(kDChgHTSndAlarmRelDlyIndex)
    
    BMS_SET_ALARM_PARA(TRD_ALARM_COND, bcu_get_high_temperature() >= config_get(kDChgHTTrdAlarmIndex));
    BMS_SET_ALARM_PARA(TRD_ALARM_DLY, config_get(kDChgHTTrdAlarmDlyIndex));
    BMS_SET_ALARM_PARA(TRD_ALARM_REL_COND, bcu_get_high_temperature() < DCHG_HT_TRD_ALARM_REL);//config_get(kDChgHTTrdAlarmRelIndex)
    BMS_SET_ALARM_PARA(TRD_ALARM_REL_DLY, DCHG_HT_TRD_ALARM_REL_DLY);//config_get(kDChgHTTrdAlarmRelDlyIndex)
#endif
    rule_stock_higher_level_priority_alarm_run(start_index);
    
    //Update battery high temperature state
    bcu_set_dchg_ht_state(rule_stock_get_var(start_index + BMS_DIAG_CUR_STATE_OFFSET));
}

void bms_chgr_ac_outlet_high_temperature_diagnose(void)
{
    INT8U high_temp = 0, flag = 0, config = 0;
    INT16U start_index = BMS_DIAGNOSIS_CHGR_AC_OUTLET_HT_RAM_POS;
    
    high_temp = bms_get_chgr_ac_outlet_high_temperature();
    BMS_SET_ALARM_PARA(FST_ALARM_COND, BMS_DIAGNOSIS_AC_CHARGE_CONDITION() && high_temp >= config_get(kChgrAcOutletHTFstAlarmIndex));
    BMS_SET_ALARM_PARA(FST_ALARM_DLY, config_get(kChgrAcOutletHTFstAlarmDlyIndex));
    BMS_SET_ALARM_PARA(FST_ALARM_REL_COND, BMS_DIAGNOSIS_AC_CHARGE_CONDITION()== 0 || high_temp < CHGR_AC_OUTLET_HT_FST_ALARM_REL);//config_get(kChgrAcOutletHTFstAlarmRelIndex)
    BMS_SET_ALARM_PARA(FST_ALARM_REL_DLY, CHGR_AC_OUTLET_HT_FST_ALARM_REL_DLY);//config_get(kChgrAcOutletHTFstAlarmRelDlyIndex)
    
    BMS_SET_ALARM_PARA(SND_ALARM_COND, BMS_DIAGNOSIS_AC_CHARGE_CONDITION() && high_temp >= config_get(kChgrAcOutletHTSndAlarmIndex));
    BMS_SET_ALARM_PARA(SND_ALARM_DLY, config_get(kChgrAcOutletHTSndAlarmDlyIndex));
    BMS_SET_ALARM_PARA(SND_ALARM_REL_COND, BMS_DIAGNOSIS_AC_CHARGE_CONDITION()== 0 || high_temp <CHGR_AC_OUTLET_HT_SND_ALARM_REL );//config_get(kChgrAcOutletHTSndAlarmRelIndex)
    BMS_SET_ALARM_PARA(SND_ALARM_REL_DLY, CHGR_AC_OUTLET_HT_SND_ALARM_REL_DLY);//config_get(kChgrAcOutletHTSndAlarmRelDlyIndex)
    
    BMS_SET_ALARM_PARA(TRD_ALARM_COND, BMS_DIAGNOSIS_AC_CHARGE_CONDITION() && high_temp >= CHG_OUTLET_HT_TRD_ALARM_DEF);//config_get(kChgrAcOutletHTTrdAlarmIndex)
    BMS_SET_ALARM_PARA(TRD_ALARM_DLY, CHG_OUTLET_HT_TRD_ALARM_DLY_DEF);//config_get(kChgrAcOutletHTTrdAlarmDlyIndex)
    //BMS_SET_ALARM_PARA(TRD_ALARM_REL_COND, BMS_DIAGNOSIS_AC_CHARGE_CONDITION()== 0 || high_temp < CHG_OUTLET_HT_TRD_ALARM_REL_DEF);//config_get(kChgrAcOutletHTTrdAlarmRelIndex)
    //BMS_SET_ALARM_PARA(TRD_ALARM_REL_DLY, CHG_OUTLET_HT_TRD_ALARM_REL_DLY_DEF);//config_get(kChgrAcOutletHTTrdAlarmRelDlyIndex)
    
    rule_stock_higher_level_priority_alarm_run(start_index);
    
    //Update battery high temperature state
    bcu_set_chgr_ac_outlet_ht_state(rule_stock_get_var(start_index + BMS_DIAG_CUR_STATE_OFFSET));
}

void bms_chgr_dc_outlet_high_temperature_diagnose(void)
{
    INT8U high_temp = 0, flag = 0, config = 0;
    INT16U start_index = BMS_DIAGNOSIS_CHGR_DC_OUTLET_HT_RAM_POS;
    
    high_temp = bms_get_chgr_dc_outlet_high_temperature();
    BMS_SET_ALARM_PARA(FST_ALARM_COND, BMS_DIAGNOSIS_DC_CHARGE_CONDITION() && high_temp >= config_get(kChgrDcOutletHTFstAlarmIndex));
    BMS_SET_ALARM_PARA(FST_ALARM_DLY, config_get(kChgrDcOutletHTFstAlarmDlyIndex));
    BMS_SET_ALARM_PARA(FST_ALARM_REL_COND, BMS_DIAGNOSIS_DC_CHARGE_CONDITION()== 0 || high_temp < CHGR_DC_OUTLET_HT_FST_ALARM_REL);//config_get(kChgrDcOutletHTFstAlarmRelIndex)
    BMS_SET_ALARM_PARA(FST_ALARM_REL_DLY, CHGR_DC_OUTLET_HT_FST_ALARM_REL_DLY);//config_get(kChgrDcOutletHTFstAlarmRelDlyIndex)
    
    BMS_SET_ALARM_PARA(SND_ALARM_COND, BMS_DIAGNOSIS_DC_CHARGE_CONDITION() && high_temp >= config_get(kChgrDcOutletHTSndAlarmIndex));
    BMS_SET_ALARM_PARA(SND_ALARM_DLY, config_get(kChgrDcOutletHTSndAlarmDlyIndex));
    BMS_SET_ALARM_PARA(SND_ALARM_REL_COND, BMS_DIAGNOSIS_DC_CHARGE_CONDITION()== 0 || high_temp < CHGR_DC_OUTLET_HT_SND_ALARM_REL);//config_get(kChgrDcOutletHTSndAlarmRelIndex)
    BMS_SET_ALARM_PARA(SND_ALARM_REL_DLY, CHGR_DC_OUTLET_HT_SND_ALARM_REL_DLY);//config_get(kChgrDcOutletHTSndAlarmRelDlyIndex)
    
    BMS_SET_ALARM_PARA(TRD_ALARM_COND, BMS_DIAGNOSIS_DC_CHARGE_CONDITION() && high_temp >= CHG_OUTLET_HT_TRD_ALARM_DEF);//config_get(kChgrDcOutletHTTrdAlarmIndex)
    BMS_SET_ALARM_PARA(TRD_ALARM_DLY, CHG_OUTLET_HT_TRD_ALARM_DLY_DEF);//config_get(kChgrDcOutletHTTrdAlarmDlyIndex)
    //BMS_SET_ALARM_PARA(TRD_ALARM_REL_COND, BMS_DIAGNOSIS_DC_CHARGE_CONDITION()== 0 || high_temp < CHG_OUTLET_HT_TRD_ALARM_REL_DEF);//config_get(kChgrDcOutletHTTrdAlarmRelIndex)
    //BMS_SET_ALARM_PARA(TRD_ALARM_REL_DLY, CHG_OUTLET_HT_TRD_ALARM_REL_DLY_DEF);//config_get(kChgrDcOutletHTTrdAlarmRelDlyIndex)
    
    rule_stock_higher_level_priority_alarm_run(start_index);
    
    //Update battery high temperature state
    bcu_set_chgr_dc_outlet_ht_state(rule_stock_get_var(start_index + BMS_DIAG_CUR_STATE_OFFSET));
}

AlarmLevel bcu_get_high_temperature_state(void)
{
    AlarmLevel chg_state, dchg_state;
    
    chg_state = bcu_get_chg_ht_state();
    dchg_state = bcu_get_dchg_ht_state();
    
    return chg_state > dchg_state ? chg_state : dchg_state;
}

AlarmLevel bcu_get_chgr_outlet_high_temperature_state(void)
{
    AlarmLevel ac_state, dc_state;
    
    ac_state = bcu_get_chgr_ac_outlet_ht_state();
    dc_state = bcu_get_chgr_dc_outlet_ht_state();
    
    return ac_state > dc_state ? ac_state : dc_state;
}