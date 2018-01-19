/**
*
* Copyright (c) 2014 Ligoo Inc.
*
* @file bms_diagnosis_communication.c
* @brief
* @note
* @author Liwei Dong
* @date 2014-2-17
*
*/
#include "bms_diagnosis_communication.h"

#pragma MESSAGE DISABLE C12056 // SP debug info incorrect because of optimization or inline assembler

void bms_communication_diagnose(void)
{
    INT16U start_index = BMS_DIAGNOSIS_COMMUNICATION_RAM_POS;
    INT16U diff;
    
    BMS_SET_ALARM_PARA(FST_ALARM_COND, bcu_get_com_abort_num() > 0);
    BMS_SET_ALARM_PARA(FST_ALARM_DLY, 0);
    BMS_SET_ALARM_PARA(FST_ALARM_REL_COND, bcu_get_com_abort_num() == 0);
    BMS_SET_ALARM_PARA(FST_ALARM_REL_DLY,  COMM_FST_ALARM_REL_DLY* 1000);//config_get(kCommFstAlarmRelDlyIndex)
    
    BMS_SET_ALARM_PARA(SND_ALARM_COND, bcu_get_com_abort_num() > 0 && BMS_GET_ALARM_STATE() == kAlarmFirstLevel);
    diff =COMM_SND_ALARM_DLY_DEF- config_get(kCommFstAlarmDlyIndex);//config_get(kCommSndAlarmDlyIndex)
    if(!INT16U_IS_NEGATIVE(diff))
        diff = diff * 1000;
    else
        diff = COMM_ABORT_ALARM_REL_DLY_MIN * 1000;
    BMS_SET_ALARM_PARA(SND_ALARM_DLY, diff);
    BMS_SET_ALARM_PARA(SND_ALARM_REL_COND, bcu_get_com_abort_num() == 0);
    BMS_SET_ALARM_PARA(SND_ALARM_REL_DLY,  COMM_SND_ALARM_REL_DLY_DEF* 1000);//config_get(kCommSndAlarmRelDlyIndex)
    
    BMS_SET_ALARM_PARA(TRD_ALARM_COND, bcu_get_com_abort_num() > 0 && BMS_GET_ALARM_STATE() == kAlarmSecondLevel);
    diff =  COMM_TRD_ALARM_DLY_DEF- COMM_SND_ALARM_DLY_DEF;//config_get(kCommTrdAlarmDlyIndex)-config_get(kCommSndAlarmDlyIndex);
    if(!INT16U_IS_NEGATIVE(diff))
        diff = diff * 1000;
    else
        diff = COMM_ABORT_ALARM_REL_DLY_MIN * 1000;
    BMS_SET_ALARM_PARA(TRD_ALARM_DLY, diff);
    BMS_SET_ALARM_PARA(TRD_ALARM_REL_COND, bcu_get_com_abort_num() == 0);
    BMS_SET_ALARM_PARA(TRD_ALARM_REL_DLY,  COMM_TRD_ALARM_REL_DLY_DEF* 1000);//config_get(kCommTrdAlarmRelDlyIndex)
    
    rule_stock_higher_level_priority_alarm_run(start_index);
    
    //Update slave communication state
    bcu_set_slave_communication_state(rule_stock_get_var(start_index + BMS_DIAG_CUR_STATE_OFFSET));
}

