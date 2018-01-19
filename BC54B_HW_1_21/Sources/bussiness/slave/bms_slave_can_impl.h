/**  
*  
* Copyright (c) 2012 Ligoo Inc.  
*  
* @file slave_impl.h
* @brief 
* @note  
* @author 
* @date 2012-5-12  
*  
*/
#ifndef BMS_SLAVE_IMPL_H
#define BMS_SLAVE_IMPL_H
#include "bms_slave.h"


#define SLAVE_REQUEST_FLAG_CHARGE           0x01
#define SLAVE_REQUEST_FLAG_NOT_BANLANCE     0x02

#define  SLAVE_START_ID 0x50

typedef enum
{
    kSlaveNoneType = 0, kSlaveISOType, kSlaveHVCMType
} SlaveType;

typedef enum
{
    kSlaveSummaryFrameId = 1,
    kSlaveBalanceCurrentFrameId = 2,
    kSlaveVoltageStartFrameId = 11,
    kSlaveVoltageEndFrameId = 18,
    kSlaveTemperatureFrameId = 51
} SlaveFrameId;


void slave_task_create(void);
void slave_task_run_tx(void* data);
void slave_task_run_rx(void* data);

void slave_query_slaves(void);
void slave_receive_next_frame(void);
void slave_query_next_slave(void);

/**********************************************
 *
 * Inside Helper Function
 *
 ***********************************************/
void slave_query_slave(INT8U slave);
void slave_next_slave(void);
SlaveType slave_get_type(INT8U slave_id);

void slave_handle_status_response(SlaveNetResponseFrame* frame);

#endif

