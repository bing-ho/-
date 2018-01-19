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

#define EMS_CAN_RE_INIT_TIME      3600000//1hour ����CANͨ���жϺ�������ʱʱ�� ��λ��ms

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

extern OS_STK g_ems_ecu_tx_task_stack[ECU_TX_STK_SIZE];    //����ͨ�ŷ��������ջ
extern OS_STK g_ems_ecu_rx_task_stack[ECU_RX_STK_SIZE];    //����ͨ�Ž��������ջ   
extern OS_STK g_ems_dbd_tx_task_stack[DBD_TX_STK_SIZE];    //�Ǳ�ͨ�ŷ��������ջ

#pragma DATA_SEG __RPAGE_SEG PAGED_RAM_EMS

extern EmsContext g_ems_context;   //������������

extern CanInfo g_ems_can_info;     //��ems_buffer����Ϣ���浽g_ems_can_info
extern CanMessage g_ems_can_buffers[EMS_CAN_BUFFER_COUNT];   //��ems�б��ĵ�10��������

extern CanMessage g_ems_ecu_message;   //�洢����������Ϣ��
extern CanMessage g_ems_ecu_rx_message;   //�洢����������Ϣ�� 
extern CanMessage g_ems_dbd_message;   //�洢�Ǳ�����Ϣ��

#pragma DATA_SEG DEFAULT

/**********************************************
 * Varibles
 ***********************************************/
#pragma DATA_SEG __RPAGE_SEG PAGED_RAM_EMS
   
extern EmsContext g_ems_context;   //������������
extern CanMessage g_ems_ecu_message;   //�洢����������Ϣ
extern CanMessage g_ems_ecu_rx_message;  //�洢����������Ϣ�� 
extern CanMessage g_ems_dbd_message;   //�洢�Ǳ�����Ϣ��
#if( SWITCH_ON == UDS_SWITCH_CONFIG)
extern udsbuffrerinfo    uds_buffrer_info;
extern CanMessage g_uds_can_buffers[UDS_BUFFER_COUNTER]; 
 #endif
#pragma DATA_SEG DEFAULT

/**********************************************
 * Functions
 ***********************************************/
void ems_task_create(void);

INT16U ems_charger_continue_current_update(void);  //��ȡ���ɳ���������
INT16U ems_discharge_continue_current_update(void); //��ȡ���ɳ����ŵ����
INT16U ems_charge_current_update(void);  //��ȡ����ʱ������
INT16U ems_discharge_current_update(void); //��ȡ����ʱ�ŵ����
AlarmLevel ems_dchg_fault_level_update(void); //��ȡ�ŵ���ϵȼ�
AlarmLevel ems_chg_fault_level_update(void); //��ȡ�����ϵȼ�
AlarmLevel ems_bms_alarm_level_update(void); //��ȡBMS���ϵȼ�
INT8U ems_get_Lv4_from_alarm(AlarmLevel alarm_level); //���ݹ��ϵȼ���ȡ������(0:kAlarmNone; 1:kAlarmFirstLevel; 2:kAlarmSecondLevel; 3:kAlarmThirdLevel; 4:kAlarmForthLevel)
INT8U ems_get_Lv3_from_alarm(AlarmLevel alarm_level); //���ݹ��ϵȼ���ȡ������(0:kAlarmNone,kAlarmFirstLevel; 1:kAlarmSecondLevel; 2:kAlarmThirdLevel; 3:kAlarmForthLevel)
INT8U ems_total_fault_num_update(void); //��ȡϵͳ�ܹ�����
AlarmLevel ems_max_fault_level_update(void); //��ȡϵͳ��߹��ϵȼ�

//����������������ƺ���
INT8U ems_power_control_command_is_enable(void);
void ems_power_on(void);
void ems_power_off(void);

//self-check
void ems_task_self_check_run(void *pdata);
#if( SWITCH_ON == UDS_SWITCH_CONFIG)
INT16U ems_can_callback(can_t context, CanMessage* msg, void* userdata);
#endif
#endif /* BMS_EMS_IMPL_H_ */
