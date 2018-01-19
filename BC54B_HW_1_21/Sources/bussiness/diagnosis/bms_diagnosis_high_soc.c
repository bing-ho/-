/**
*
* Copyright (c) 2014 Ligoo Inc.
*
* @file bms_diagnosis_high_soc.c
* @brief
* @note
* @author Liwei Dong
* @date 2014-2-19
*
*/
#include "bms_diagnosis_high_soc.h"

#pragma MESSAGE DISABLE C12056 // SP debug info incorrect because of optimization or inline assembler

void bms_high_soc_diagnose(void)
{
    INT16U start_index = BMS_DIAGNOSIS_HIGH_SOC_RAM_POS;
    
    BMS_SET_ALARM_PARA(FST_ALARM_COND, bcu_get_SOC() >= config_get(kHSOCFstAlarmIndex));
    BMS_SET_ALARM_PARA(FST_ALARM_DLY, config_get(kHSOCFstAlarmDlyIndex));
    BMS_SET_ALARM_PARA(FST_ALARM_REL_COND, bcu_get_SOC() < HSOC_FST_ALARM_REL);//config_get(kHSOCFstAlarmRelIndex)
    BMS_SET_ALARM_PARA(FST_ALARM_REL_DLY, HSOC_FST_ALARM_REL_DLY);//config_get(kHSOCFstAlarmRelDlyIndex)
    
    BMS_SET_ALARM_PARA(SND_ALARM_COND, bcu_get_SOC() >= SOC_HIGH_SND_ALARM_DEF);//config_get(kHSOCSndAlarmIndex)
    BMS_SET_ALARM_PARA(SND_ALARM_DLY, SOC_HIGH_SND_ALARM_DLY_DEF);//config_get(kHSOCSndAlarmDlyIndex)
    BMS_SET_ALARM_PARA(SND_ALARM_REL_COND, bcu_get_SOC() < SOC_HIGH_SND_ALARM_REL_DEF);//config_get(kHSOCSndAlarmRelIndex)
    BMS_SET_ALARM_PARA(SND_ALARM_REL_DLY, SOC_HIGH_SND_ALARM_REL_DLY_DEF);//config_get(kHSOCSndAlarmRelDlyIndex)
    
    BMS_SET_ALARM_PARA(TRD_ALARM_COND, bcu_get_SOC() >= SOC_HIGH_TRD_ALARM_DEF);//config_get(kSOCHighTrdAlarmIndex)
    BMS_SET_ALARM_PARA(TRD_ALARM_DLY, SOC_HIGH_TRD_ALARM_DLY_DEF);//config_get(kSOCHighTrdAlarmDlyIndex)
    BMS_SET_ALARM_PARA(TRD_ALARM_REL_COND, bcu_get_SOC() < SOC_HIGH_TRD_ALARM_REL_DEF);//config_get(kSOCHighTrdAlarmRelIndex)
    BMS_SET_ALARM_PARA(TRD_ALARM_REL_DLY, SOC_HIGH_TRD_ALARM_REL_DLY_DEF);//config_get(kSOCHighTrdAlarmRelDlyIndex)
    
    rule_stock_higher_level_priority_alarm_run(start_index);
    
    //Update high soc state
    bcu_set_high_soc_state(rule_stock_get_var(start_index + BMS_DIAG_CUR_STATE_OFFSET));
}
