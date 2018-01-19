/**
*
* Copyright (c) 2014 Ligoo Inc.
*
* @file bms_diagnosis_low_soc.c
* @brief
* @note
* @author Liwei Dong
* @date 2014-2-19
*
*/
#include "bms_diagnosis_low_soc.h"

#pragma MESSAGE DISABLE C12056 // SP debug info incorrect because of optimization or inline assembler

void bms_low_soc_diagnose(void)
{
    INT16U start_index = BMS_DIAGNOSIS_LOW_SOC_RAM_POS;
    
    BMS_SET_ALARM_PARA(FST_ALARM_COND, bcu_get_SOC() <= config_get(kLSOCFstAlarmIndex));
    BMS_SET_ALARM_PARA(FST_ALARM_DLY, config_get(kLSOCFstAlarmDlyIndex));
    BMS_SET_ALARM_PARA(FST_ALARM_REL_COND, bcu_get_SOC() > LSOC_FST_ALARM_REL);//config_get(kLSOCFstAlarmRelIndex)
    BMS_SET_ALARM_PARA(FST_ALARM_REL_DLY, LSOC_FST_ALARM_REL_DLY);//config_get(kLSOCFstAlarmRelDlyIndex)
    
    BMS_SET_ALARM_PARA(SND_ALARM_COND, bcu_get_SOC() <= config_get(kLSOCSndAlarmIndex));
    BMS_SET_ALARM_PARA(SND_ALARM_DLY, config_get(kLSOCSndAlarmDlyIndex));
    BMS_SET_ALARM_PARA(SND_ALARM_REL_COND, bcu_get_SOC() > LSOC_SND_ALARM_REL);//config_get(kLSOCSndAlarmRelIndex)
    BMS_SET_ALARM_PARA(SND_ALARM_REL_DLY, LSOC_SND_ALARM_REL_DLY);//config_get(kLSOCSndAlarmRelDlyIndex)
    
    BMS_SET_ALARM_PARA(TRD_ALARM_COND, bcu_get_SOC() <= config_get(kSOCLowTrdAlarmIndex));
    BMS_SET_ALARM_PARA(TRD_ALARM_DLY, config_get(kSOCLowTrdAlarmDlyIndex));
    BMS_SET_ALARM_PARA(TRD_ALARM_REL_COND, bcu_get_SOC() > SOC_LOW_TRD_ALARM_REL);//config_get(kSOCLowTrdAlarmRelIndex)
    BMS_SET_ALARM_PARA(TRD_ALARM_REL_DLY, SOC_LOW_TRD_ALARM_REL_DLY);//config_get(kSOCLowTrdAlarmRelDlyIndex)
    
    rule_stock_higher_level_priority_alarm_run(start_index);
    
    //Update low soc state
    bcu_set_low_soc_state(rule_stock_get_var(start_index + BMS_DIAG_CUR_STATE_OFFSET));
}
