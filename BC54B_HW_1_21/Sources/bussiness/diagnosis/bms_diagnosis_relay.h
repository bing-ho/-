/**
*
* Copyright (c) 2014 Ligoo Inc.
*
* @file bms_diagnosis_relay.h
* @brief
* @note
* @author Liwei Dong
* @date 2014-12-3
*
*/
#ifndef BMS_DIAGNOSIS_RELAY_H_
#define BMS_DIAGNOSIS_RELAY_H_

#include "bms_diagnosis_impl.h"
#include "bms_relay.h"
#include "bms_relay_control.h"
#include "bms_charger_temperature.h"

typedef struct
{
    INT8U fault_num;
    INT32U base_cond_mask;
    INT32U base_cond_not_rel_mask; //不释放标志
    INT32U selfcheck_cond_mask;
    INT32U selfcheck_cond_not_rel_mask;
    INT32U runtime_cond_mask;
    INT32U runtime_cond_not_rel_mask;
}RelayDiagnoseContext;

typedef enum
{
    kRelayCondNone=0,
    kRelayBaseCondStart, //1 基础条件
    kRelayBaseCondCtrlEnable=kRelayBaseCondStart,
    kRelayBaseCondForceCtrl,
    kRelayBaseCondNoRule,
    kRelayBaseCondStop=kRelayBaseCondNoRule,
    
    kRelayPwrOnCondStart, //4 上电条件
    kRelayPwrOnCondSysVolt = kRelayPwrOnCondStart,
    kRelayPwrOnCondVoltLine,
    kRelayPwrOnCondTempLine,
    kRelayPwrOnCondSlaveComm,
    kRelayPwrOnCondHardware,
    kRelayPwrOnCondDchgHV,
    kRelayPwrOnCondDchgLV,  //10
    kRelayPwrOnCondDchgHTV,
    kRelayPwrOnCondDchgLTV,
    kRelayPwrOnCondDchgHT,
    kRelayPwrOnCondDchgLT,
    kRelayPwrOnCondDchgDV,
    kRelayPwrOnCondDchgDT,
    kRelayPwrOnCondDchgOC,
    kRelayPwrOnCondChgHV,
    kRelayPwrOnCondChgLV,
    kRelayPwrOnCondChgHTV,   //20
    kRelayPwrOnCondChgLTV,
    kRelayPwrOnCondChgHT,
    kRelayPwrOnCondChgLT,
    kRelayPwrOnCondChgDV,
    kRelayPwrOnCondChgDT,
    kRelayPwrOnCondChgOC,
    kRelayPwrOnCondHSOC,
    kRelayPwrOnCondLSOC,
    kRelayPwrOnCondInsuLeak,
    kRelayPwrOnCondUnRelease, //不恢复故障   //30   
    kRelayPwrOnCondStop = kRelayPwrOnCondUnRelease,
    
    kRelayRunCondStart, //31 运行时条件
    kRelayRunCondDchgHV=kRelayRunCondStart,
    kRelayRunCondDchgLV,
    kRelayRunCondDchgHTV,
    kRelayRunCondDchgLTV,
    kRelayRunCondDchgHT,
    kRelayRunCondDchgLT,
    kRelayRunCondDchgDV,  
    kRelayRunCondDchgDT,    
    kRelayRunCondDchgOC,
    kRelayRunCondChgHV,   //40
    kRelayRunCondChgLV,
    kRelayRunCondChgHTV,
    kRelayRunCondChgLTV,
    kRelayRunCondChgHT,
    kRelayRunCondChgLT,
    kRelayRunCondChgDV,
    kRelayRunCondChgDT, 
    kRelayRunCondChgOC,  
    kRelayRunCondHSOC,
    kRelayRunCondLSOC,      //50
    kRelayRunCondVoltLine,
    kRelayRunCondTempLine,
    kRelayRunCondInsuLeak,
    kRelayRunCondSlaveComm,
    kRelayRunCondFullChg,
    kRelayRunCondMutex,
    kRelayRunCondPrechargeFailure,
    kRelayRunCondPrecharging,      
    kRelayRunCondUnRelease, //不恢复故障
    kRelayRunCondCommAbort,         //60
    kRelayCondUndefine,
    kRelayAdhesion, 
    kRelayRunCondStop=kRelayAdhesion
}RelayDiagnoseCondition;


void bms_relay_diagnose(void);
INT8U bms_relay_diagnose_get_fault_num(RelayControlType type);
INT8U bms_relay_diagnose_set_fault_num(RelayControlType type, INT8U fault);
void bms_relay_selfcheck_fault_update(RelayControlType type, INT8U* _PAGED current_fault_num);
void bms_relay_runtime_fault_update(RelayControlType type, INT8U* _PAGED current_fault_num);

#endif