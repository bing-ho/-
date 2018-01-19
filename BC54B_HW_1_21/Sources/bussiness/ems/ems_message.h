/*******************************************************************************
 **                       ������������Դ�������޹�˾ Copyright (c)
 **                            http://www.ligoo.cn
 **�� �� ��:ems_message.h
 **��    ��:
 **��������:2017.7.12
 **�ļ�˵��:�ͻ�Э����Ϣ�ӿ�
 **�޸ļ�¼:
 **��    ��:V1.0
 **��    ע:
 *******************************************************************************/
#ifndef EMS_MESSAGE_H_
#define EMS_MESSAGE_H_

#include "bms_ems.h"
#include "bms_ems_impl.h"
#include "ems_signal.h"


#define EMS_ECU_VOLT_BASE_CAN_ID    0x1801D2F4  //��ص����ѹ��Ϣ��ʼID
#define EMS_VOLT_MSG_SEND_PERIOD    1000 //��ص����ѹ��Ϣ��������

#define EMS_ECU_TEMP_BASE_CAN_ID    0x1850D2F4  //����¶���Ϣ��ʼID
#define EMS_TEMP_MSG_SEND_PERIOD    1000 //����¶���Ϣ��������


//******��Ϣ���źŽṹ�������������̶����䣩*****//
typedef struct _SignalInMsg{
  const Signal_t *__far signal;  //ָ���źŵ�ָ��
  INT8U start_bit;  //��ʼλ
}SignalInMsg_t;


//******����Э����Ϣ�ṹ�������������̶����䣩*****//
typedef struct _EMS_Message{
  INT8U msg_type;  //��Ϣ���ͣ�0����׼֡��1����չ֡��
  INT8U msg_direction;  //��Ϣ���䷽��0�����ͣ�1�����ա�
  INT16U msg_period;  //��Ϣ�������ڣ���λ��ms����
  INT32U msg_id;  //��ϢID��
  SignalInMsg_t msg_signal[19];  //��Ϣ�а������źš�
}EMS_Message_t;  

#define EMS_MSG_NUM 4  //��Ϣ��

//******��Ϣ����������*****//
void ems_task_ecu_tx_run(void* pdata);

//******��Ϣ��䷢�ͺ���*****//
void ems_task_ecu_tx_BMS_message_4(void);
void ems_task_ecu_tx_BMS_message_3(void);
void ems_task_ecu_tx_BMS_message_2(void);
void ems_task_ecu_tx_BMS_message_1(void);



void ems_task_ecu_tx_BMS_message_4_fill(CanMessage * _PAGED send_msg); //  
void ems_task_ecu_tx_BMS_message_3_fill(CanMessage * _PAGED send_msg); //  
void ems_task_ecu_tx_BMS_message_2_fill(CanMessage * _PAGED send_msg); //  
void ems_task_ecu_tx_BMS_message_1_fill(CanMessage * _PAGED send_msg); //  

//****��Ϣ����������****//
void ems_task_ecu_rx_run(void* pdata); 

//****��Ϣ��������****//



//****������Ϣ����������****//
void ems_task_dbd_tx_run(void* pdata);
 
//******��ص����ѹ��Ϣ���ͺ���*****//
void ems_task_dbd_tx_voltage(void);
//******����¶���Ϣ����������*****//
void ems_task_dbd_tx_temperature(void);
extern INT32U can_rx_timeout_check(void);
extern  void clr_can_rx_timeout(void);

#endif