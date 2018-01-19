/*******************************************************************************
 **                       ������������Դ�������޹�˾ Copyright (c)
 **                            http://www.ligoo.cn
 **�� �� ��:detect_ems_message.h
 **��    ��:
 **��������:2017.11.1
 **�ļ�˵��:�ͻ�Э����Ϣ�ӿ�
 **�޸ļ�¼:
 **��    ��:V1.0
 **��    ע:
 *******************************************************************************/
#ifndef DETECT_EMS_MESSAGE_H_
#define DETECT_EMS_MESSAGE_H_

#include "bms_ems.h"
#include "detect_ems_impl.h"
#include "detect_ems_signal.h"
#include "ems_message.h"

#pragma push
#pragma DATA_SEG __RPAGE_SEG PAGED_RAM_EMS_VAR

extern INT32U __far msg_last_tick[EMS_MSG_NUM];  //��Ϣ���ͼ�ʱ��

extern INT32U __far volt_msg_last_tick; //��ص����ѹ��Ϣ���ͼ�ʱ��
extern INT32U __far temp_msg_last_tick; //����¶���Ϣ���ͼ�ʱ��

#pragma pop
#pragma DATA_SEG __RPAGE_SEG DEFAULT


#pragma CONST_SEG PAGED_EMS_CONFIG

//******��Ϣ����������*****//
void detect_ems_task_ecu_tx_run(void* pdata);

//******��Ϣ��䷢�ͺ���*****//
void detect_ems_task_ecu_tx_BMS_message_4(void);
void detect_ems_task_ecu_tx_BMS_message_3(void);
void detect_ems_task_ecu_tx_BMS_message_2(void);
void detect_ems_task_ecu_tx_BMS_message_1(void);



void detect_ems_task_ecu_tx_BMS_message_4_fill(CanMessage * _PAGED send_msg); //  
void detect_ems_task_ecu_tx_BMS_message_3_fill(CanMessage * _PAGED send_msg); //  
void detect_ems_task_ecu_tx_BMS_message_2_fill(CanMessage * _PAGED send_msg); //  
void detect_ems_task_ecu_tx_BMS_message_1_fill(CanMessage * _PAGED send_msg); //  

//****��Ϣ����������****//
void detect_ems_task_ecu_rx_run(void* pdata); 

//****��Ϣ��������****//



//****������Ϣ����������****//
void detect_ems_task_dbd_tx_run(void* pdata);
 
//******��ص����ѹ��Ϣ���ͺ���*****//
void detect_ems_task_dbd_tx_voltage(void);
//******����¶���Ϣ����������*****//
void detect_ems_task_dbd_tx_temperature(void);


#endif