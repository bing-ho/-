/**  
*  
* Copyright (c) 2012 Ligoo Inc.  
*  
* @file bms_ems_impl.h
* @brief 
* @note  
* @author
* @date 2012-5-24  
*  
*/

#ifndef BMS_EMS_IMPL_H_
#define BMS_EMS_IMPL_H_
#include "bms_ems.h"

#define EMS_CAN_RE_INIT_TIME      3600000//1hour 整车CAN通信中断后重启延时时间 单位：ms

#if( SWITCH_ON == UDS_SWITCH_CONFIG)
#define  UDS_BUFFER_COUNTER    5
#endif
#if BMS_FORTH_ALARM_SUPPORT 
#define CURRENT_THIRD_ALARM kAlarmForthLevel
#define CURRENT_SECOND_ALARM kAlarmThirdLevel
#define CURRENT_FIRST_ALARM kAlarmSecondLevel
#else
#define CURRENT_THIRD_ALARM kAlarmThirdLevel
#define CURRENT_SECOND_ALARM kAlarmSecondLevel
#define CURRENT_FIRST_ALARM kAlarmFirstLevel
#endif
/**********************************************
 * Structures
 ***********************************************/
typedef struct _EmsContext
{
  can_t can_context;
}EmsContext;

extern OS_STK g_ems_ecu_tx_task_stack[ECU_TX_STK_SIZE];    //整车通信发送任务堆栈
extern OS_STK g_ems_ecu_rx_task_stack[ECU_RX_STK_SIZE];    //整车通信接收任务堆栈   
extern OS_STK g_ems_dbd_tx_task_stack[DBD_TX_STK_SIZE];    //仪表通信发送任务堆栈

#pragma DATA_SEG __RPAGE_SEG PAGED_RAM_EMS

extern EmsContext g_ems_context;   //缓冲区上下文

extern CanInfo g_ems_can_info;     //把ems_buffer的信息保存到g_ems_can_info
extern CanMessage g_ems_can_buffers[EMS_CAN_BUFFER_COUNT];   //在ems中报文的10个缓冲区

extern CanMessage g_ems_ecu_message;   //存储整车发送信息。
extern CanMessage g_ems_ecu_rx_message;   //存储整车接收信息。 
extern CanMessage g_ems_dbd_message;   //存储仪表发送信息。

#pragma DATA_SEG DEFAULT

/**********************************************
 * Varibles
 ***********************************************/
#pragma DATA_SEG __RPAGE_SEG PAGED_RAM_EMS
   
extern EmsContext g_ems_context;   //缓冲区上下文
extern CanMessage g_ems_ecu_message;   //存储整车发送信息
extern CanMessage g_ems_ecu_rx_message;  //存储整车接收信息。 
extern CanMessage g_ems_dbd_message;   //存储仪表发送信息。
#if( SWITCH_ON == UDS_SWITCH_CONFIG)
extern udsbuffrerinfo    uds_buffrer_info;
extern CanMessage g_uds_can_buffers[UDS_BUFFER_COUNTER]; 
 #endif
#pragma DATA_SEG DEFAULT

/**********************************************
 * Functions
 ***********************************************/
void ems_task_create(void);

INT16U ems_charger_continue_current_update(void);  //获取最大可持续充电电流
INT16U ems_discharge_continue_current_update(void); //获取最大可持续放电电流
INT16U ems_charge_current_update(void);  //获取最大短时充电电流
INT16U ems_discharge_current_update(void); //获取最大短时放电电流
AlarmLevel ems_dchg_fault_level_update(void); //获取放电故障等级
AlarmLevel ems_chg_fault_level_update(void); //获取充电故障等级
AlarmLevel ems_bms_alarm_level_update(void); //获取BMS故障等级
INT8U ems_get_Lv4_from_alarm(AlarmLevel alarm_level); //根据故障等级获取错误码(0:kAlarmNone; 1:kAlarmFirstLevel; 2:kAlarmSecondLevel; 3:kAlarmThirdLevel; 4:kAlarmForthLevel)
INT8U ems_get_Lv3_from_alarm(AlarmLevel alarm_level); //根据故障等级获取错误码(0:kAlarmNone,kAlarmFirstLevel; 1:kAlarmSecondLevel; 2:kAlarmThirdLevel; 3:kAlarmForthLevel)
INT8U ems_total_fault_num_update(void); //获取系统总故障数
AlarmLevel ems_max_fault_level_update(void); //获取系统最高故障等级

//整车上正电命令控制函数
INT8U ems_power_control_command_is_enable(void);
void ems_power_on(void);
void ems_power_off(void);

//self-check
void ems_task_self_check_run(void *pdata);
#if( SWITCH_ON == UDS_SWITCH_CONFIG)
INT16U ems_can_callback(can_t context, CanMessage* msg, void* userdata);
#endif
#endif /* BMS_EMS_IMPL_H_ */
