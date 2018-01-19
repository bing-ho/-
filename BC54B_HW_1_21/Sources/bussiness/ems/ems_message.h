/*******************************************************************************
 **                       安徽力高新能源技术有限公司 Copyright (c)
 **                            http://www.ligoo.cn
 **文 件 名:ems_message.h
 **作    者:
 **创建日期:2017.7.12
 **文件说明:客户协议消息接口
 **修改记录:
 **版    本:V1.0
 **备    注:
 *******************************************************************************/
#ifndef EMS_MESSAGE_H_
#define EMS_MESSAGE_H_

#include "bms_ems.h"
#include "bms_ems_impl.h"
#include "ems_signal.h"


#define EMS_ECU_VOLT_BASE_CAN_ID    0x1801D2F4  //电池单体电压消息起始ID
#define EMS_VOLT_MSG_SEND_PERIOD    1000 //电池单体电压消息发送周期

#define EMS_ECU_TEMP_BASE_CAN_ID    0x1850D2F4  //电池温度消息起始ID
#define EMS_TEMP_MSG_SEND_PERIOD    1000 //电池温度消息发送周期


//******消息内信号结构体类型声明（固定不变）*****//
typedef struct _SignalInMsg{
  const Signal_t *__far signal;  //指向信号的指针
  INT8U start_bit;  //起始位
}SignalInMsg_t;


//******整车协议消息结构体类型声明（固定不变）*****//
typedef struct _EMS_Message{
  INT8U msg_type;  //消息类型：0：标准帧，1：扩展帧。
  INT8U msg_direction;  //消息传输方向：0：发送，1：接收。
  INT16U msg_period;  //消息发送周期（单位：ms）。
  INT32U msg_id;  //消息ID。
  SignalInMsg_t msg_signal[19];  //消息中包含的信号。
}EMS_Message_t;  

#define EMS_MSG_NUM 4  //消息数

//******消息发送任务函数*****//
void ems_task_ecu_tx_run(void* pdata);

//******消息填充发送函数*****//
void ems_task_ecu_tx_BMS_message_4(void);
void ems_task_ecu_tx_BMS_message_3(void);
void ems_task_ecu_tx_BMS_message_2(void);
void ems_task_ecu_tx_BMS_message_1(void);



void ems_task_ecu_tx_BMS_message_4_fill(CanMessage * _PAGED send_msg); //  
void ems_task_ecu_tx_BMS_message_3_fill(CanMessage * _PAGED send_msg); //  
void ems_task_ecu_tx_BMS_message_2_fill(CanMessage * _PAGED send_msg); //  
void ems_task_ecu_tx_BMS_message_1_fill(CanMessage * _PAGED send_msg); //  

//****消息接收任务函数****//
void ems_task_ecu_rx_run(void* pdata); 

//****消息解析函数****//



//****单体信息发送任务函数****//
void ems_task_dbd_tx_run(void* pdata);
 
//******电池单体电压消息发送函数*****//
void ems_task_dbd_tx_voltage(void);
//******电池温度消息发送任务函数*****//
void ems_task_dbd_tx_temperature(void);
extern INT32U can_rx_timeout_check(void);
extern  void clr_can_rx_timeout(void);

#endif