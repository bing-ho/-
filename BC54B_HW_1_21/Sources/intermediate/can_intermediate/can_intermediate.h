/*******************************************************************************
**                       安徽力高新能源有限公司 Copyright (c)
**                            http://www.ligoo.cn/ 
** @file	    can_intermediate.h                                       

** @brief       1.完成CAN模块中间层的数据收发 
                2.完成CAN模块中间层的数据缓冲区的处理
                3.完成CAN模块中间层的其他相关功能函数        
                
** @copyright   	Ligoo Inc.         
** @date		    2017-04-25.
** @author            
*******************************************************************************/ 

//#ifndef __BMS_CAN_IMPL_H__
//#define __BMS_CAN_IMPL_H__
#ifndef __CAN_INTERMEDIATE_H_
#define __CAN_INTERMEDIATE_H_
#include "bms_can.h"
#include "bms_defs.h"
#include "includes.h"
#include "can_hardware.h"

/**********************************************
 *
 * Context
 *
 **********************************************
typedef struct _CanContext
{
    CanDev dev;
    CanInfo info;

    INT32U receive_id;
    INT32U mask_id;
    INT32U receive_1_id;
    INT32U mask_1_id;
    INT8U read_buffer_count;
    INT8U read_buffer_write_pos;
    INT8U read_buffer_read_pos;
    OS_EVENT* read_buffer_event;

    BOOLEAN     buffers_is_allocated_by_self; // 内部分配接受buffer
} CanContext, *can_t;*/

#define CAN_DEV1_RE_INIT_DELAY              600000    //CAN1重新初始化延时 单位：ms
#define CAN_DEV2_RE_INIT_DELAY              600000    //CAN2重新初始化延时 单位：ms
#define CAN_DEV3_RE_INIT_DELAY              600000    //CAN3重新初始化延时 单位：ms
#define CAN_DEV4_RE_INIT_DELAY              600000    //CAN4重新初始化延时 单位：ms
#define CAN_DEV5_RE_INIT_DELAY              600000    //CAN5重新初始化延时 单位：ms

#define CAN_DEFAULT_BUFFER_MESSAGE_COUNT    5




/**********************************************
 *
 * Helper Functions
 *
 ***********************************************/
int is_extend_mode(can_t context);
int is_extend_mode_1(can_t context);
void can_rx_receive(can_t context, CanMessage* _PAGED message);
void can_rx_isr_handler(can_t context);
Result can_update_re_init_heart_beat_tick(CanDev dev);
Result can_check_buffers(can_t context);
Result can_configurate_device(can_t context);
Result  Poll_Rx_Can  (CanDev dev);


can_t can_init(CanInfo* _PAGED can_info);
void can_re_init_init(void);
Result can_send(can_t context, CanMessage* _PAGED msg);
Result dev_can_send(CanDev dev, CanMessage* _PAGED msg);
Result can_receive(can_t context, CanMessage* _PAGED msg, INT16U timeout);
Result can_uninit(can_t context);
Result can_add_receive_id(CanInfo* _PAGED can_info, INT32U id);
CanBps can_get_bps(INT8U bps_index);

/**********************************************
 *
 * Conversion Functions
 *
 ***********************************************/
INT32U can_id_from_extend_id(INT32U id);
INT32U can_id_to_extend_id(INT32U id);
INT32U can_id_from_std_id(INT32U id);
INT32U can_id_to_std_id(INT32U id);
INT8U can_channel_is_valid(CanDev channel);

extern CanContext* g_can_contexts[kCanDevMaxCount];
/************************************************************************
  * @brief           获取CAN context  数组
  * @param[in]       dev CAN设备   
  * @return          context can报文结构体 
***********************************************************************/ 
can_t can_get_context(CanDev dev);
/************************************************************************
  * @brief           通过设备号重新初始化CAN设备
  * @param[in]       dev CAN设备   
  * @return          0：成功  1：失败
***********************************************************************/ 
Result can_re_init_with_dev(CanDev dev);

/************************************************************************
  * @brief           通过 context重新初始化CAN设备
  * @param[in]       context can报文结构体   
  * @return          0：成功  1：失败
***********************************************************************/ 
Result can_re_init_with_context(can_t context);

#endif /* BMS_CAN_IMPL_H_ */
