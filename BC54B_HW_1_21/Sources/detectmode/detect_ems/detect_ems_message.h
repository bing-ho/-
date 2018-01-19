/*******************************************************************************
 **                       安徽力高新能源技术有限公司 Copyright (c)
 **                            http://www.ligoo.cn
 **文 件 名:detect_ems_message.h
 **作    者:
 **创建日期:2017.11.1
 **文件说明:客户协议消息接口
 **修改记录:
 **版    本:V1.0
 **备    注:
 *******************************************************************************/
#ifndef DETECT_EMS_MESSAGE_H_
#define DETECT_EMS_MESSAGE_H_

#include "bms_ems.h"
#include "detect_ems_impl.h"
#include "detect_ems_signal.h"
#include "ems_message.h"

#pragma push
#pragma DATA_SEG __RPAGE_SEG PAGED_RAM_EMS_VAR

extern INT32U __far msg_last_tick[EMS_MSG_NUM];  //消息发送计时。

extern INT32U __far volt_msg_last_tick; //电池单体电压消息发送计时。
extern INT32U __far temp_msg_last_tick; //电池温度消息发送计时。

#pragma pop
#pragma DATA_SEG __RPAGE_SEG DEFAULT


#pragma CONST_SEG PAGED_EMS_CONFIG

//******消息发送任务函数*****//
void detect_ems_task_ecu_tx_run(void* pdata);

//******消息填充发送函数*****//
void detect_ems_task_ecu_tx_BMS_message_4(void);
void detect_ems_task_ecu_tx_BMS_message_3(void);
void detect_ems_task_ecu_tx_BMS_message_2(void);
void detect_ems_task_ecu_tx_BMS_message_1(void);



void detect_ems_task_ecu_tx_BMS_message_4_fill(CanMessage * _PAGED send_msg); //  
void detect_ems_task_ecu_tx_BMS_message_3_fill(CanMessage * _PAGED send_msg); //  
void detect_ems_task_ecu_tx_BMS_message_2_fill(CanMessage * _PAGED send_msg); //  
void detect_ems_task_ecu_tx_BMS_message_1_fill(CanMessage * _PAGED send_msg); //  

//****消息接收任务函数****//
void detect_ems_task_ecu_rx_run(void* pdata); 

//****消息解析函数****//



//****单体信息发送任务函数****//
void detect_ems_task_dbd_tx_run(void* pdata);
 
//******电池单体电压消息发送函数*****//
void detect_ems_task_dbd_tx_voltage(void);
//******电池温度消息发送任务函数*****//
void detect_ems_task_dbd_tx_temperature(void);


#endif