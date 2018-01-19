/**  
*  
* Copyright (c) 2012 Ligoo Inc.  
*  
* @file bms_dtu_impl.h
* @brief 
* @note  
* @author
* @date 2012-5-25  
*  
*/

#ifndef BMS_DTU_IMPL_H_
#define BMS_DTU_IMPL_H_

#include "bms_dtu.h"

#if BMS_SUPPORT_DTU == BMS_DTU_RS485


/**********************************************
 *
 * Structures
 *
 ***********************************************/
typedef struct _DtuContext
{
    rs485_t dev;
    INT8U total_frame;
    INT8U current_frame;
    INT8U   send_buffer[DTU_DEV_FRAME_SIZE];
} DtuContext;

void dtu_task_create(void);
void dtu_task_tx_run(void* pdata);

void dtu_task_tx_bmu_message(void);
void dtu_task_tx_bcu_message(void);

#elif  BMS_SUPPORT_DTU == BMS_DTU_CAN

/**********************************************
 *
 * Macro define
 *
 ***********************************************/

#define DTU_CAN_MASTER_NUM          1

#define DTU_CAN_RECEIVE_TIMEOUT     30000//5000 // ms
#define DTU_CAN_SEND_INTERVAL       1000 // ms
#define DTU_CAN_BCU_FRAME_SIZE_POS  2
#define DTU_CAN_BMU_FRAME_SIZE_POS  1
#define DTU_CAN_BMU_BAL_CUR_NUM     2

#define DTU_CAN_BCU_FRAME_PGN       0x9000
#define DTU_CAN_BMU_FRAME_PGN       0xB000
#define DTU_CAN_ACK_PGN             0xE800

#define DTU_CAN_REC_PDU_BUFF                    5           //接收PDU单帧数据缓存大小
#define DTU_CAN_SENT_FRAME_BUFF                 5           //发送帧缓存个数
#define DTU_CAN_REC_FRAME_BUFF                  5           //接收帧缓存个数
#define DTU_CAN_SENT_MESSAGE_LENGTH_MAX         200         //待发送消息的最大长度
#define DTU_CAN_REC_MESSAGE_LENGTH_MAX          200         //接收消息的最大长度
#define DTU_CAN_REC_MESSAGE_BUFF_MAX            1           //接收消息的最大缓存个数
#define DTU_CAN_TP_REC_MAX                      1           //同时可以接收的TP的最大个数
#define DTU_CAN_FRAME_TIMEOUT_DEFINE	        100	        //CAN一帧数据的超时时间定义 分辨率：1ms/bit


void dtu_can_task_create(void);
void dtu_can_task_tx_run(void* pdata);
void dtu_can_task_rx_run(void* pdata);
void dtu_can_send(J1939SendMessageBuff* _PAGED buff);
void dtu_can_receive_ack(J1939RecMessage* _PAGED msg);

#endif /* BMS_SUPPORT_DTU */

INT8U dtu_insu_state_to_data(void);

#endif /* BMS_DTU_IMPL_H_ */
