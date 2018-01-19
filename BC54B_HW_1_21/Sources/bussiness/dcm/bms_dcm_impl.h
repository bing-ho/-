/**
*
* Copyright (c) 2012 Ligoo Inc.
*
* @file bms_slave_rs485_impl.h
* @brief
* @note
* @author
* @date 2012-5-25
*
*/

#ifndef BMS_SLAVE_RS485_IMPL_H_
#define BMS_SLAVE_RS485_IMPL_H_
#include "bms_dcm.h"


#define DCM_FID         0x01

/* TODO: 目前的分帧策略有问题，如果数据中含有开始帧，数据可能会异常 */
#define DCM_START_FRAME_BYTE    0x68

// TODO: 整合所有的最大电压信息
#define DCM_MAX_VOLTAGE 60000

/**********************************************
 *
 * Structures
 *
 ***********************************************/
typedef struct
{
    rs485_t dev;

    framer_t framer;
    Buffers buffers;

    INT8U   current_index;

    INT8U   send_buffer[BMS_DCM_RS485_FRAME_SIZE];
} DcmRs485Context;

void dcm_task_create(void);
void dcm_task_run(void* data);

void dcm_task_process_slave(void);
void dcm_task_process_insulation(void);

void dcm_task_request_insulation_frame(void);
void dcm_task_request_slave_frame(void);

void dcm_task_update_slave_id(void);
void dcm_task_receive_response_frame(void);

Result dcm_task_process_frame(PINT8U frame, INT8U size);
Result dcm_task_process_response_frame(PINT8U frame, INT8U size);
Result dcm_task_verify_frame(PINT8U frame, INT8U size);

int dcm_task_rs485_receive(rs485_t context, char chr, void* user_data);

/**********************************************
 * Range Functions
 * 用来循环获取从机与绝缘模块编号的辅助函数
 ***********************************************/
void dcm_ranges_reset(void);
void dcm_ranges_add(INT8U left, INT8U right);
INT8U dcm_ranges_get_next(INT8U value);

#endif /* BMS_SLAVE_RS485_IMPL_H_ */
