/**  
*  
* Copyright (c) 2012 Ligoo Inc.  
*  
* @file bms_slave_net.h
* @brief 
* @note  
* @author 
* @date 2012-5-11  
*  
*/

#ifndef BMS_SLAVE_NET_H_
#define BMS_SLAVE_NET_H_
#include "bms_can.h"
#include "bms_memory.h"

#ifndef BMS_SUPPORT_SLAVE_NET
#define BMS_SUPPORT_SLAVE_NET       1
#endif

typedef struct _SlaveNetContext* slave_net_t;


/**********************************************
 *
 * Frame Types
 *
 ***********************************************/
typedef enum
{
    kSlaveNetRequestNoneCode = 0,
    kSlaveNetRequestStatusCode = 1,
    kSlaveNetRequestConfigCode = 2,
    kSlaveNetSetConfigCode = 3,
    kSlaveNetMaxCode
}SlaveNetFunctionCode;

typedef struct
{
    INT32U receive_id;
    INT32U mask_id;
    INT8U  len;

    INT8U  function_code;
    INT8U  slave_id;

    INT8U  data[6];
}SlaveRequestInfoFrame;

typedef struct
{
    INT32U receive_id;
    INT32U mask_id;
    INT8U  len;

    INT8U  function_code;
    INT8U  slave_id;

    INT8U  data[6];
}SlaveNetQueryFrame;

typedef struct
{
    INT32U receive_id;
    INT32U mask_id;
    INT8U  len;

    INT8U  function_code;
    INT8U  frame_id;
    INT8U  data[6];
}SlaveNetResponseFrame;

typedef struct
{
    INT32U receive_id;
    INT32U mask_id;
    INT8U  len;

    INT8U  function_code;
    INT8U  frame_id;
    INT8U  slave_id;

    INT8U  data[5];
}SlaveNetConfigFrame;


/**********************************************
 *
 * Functions
 *
 ***********************************************/
slave_net_t slave_net_create_can(CanInfo* can_info);
Result slave_net_destroy(slave_net_t context);
Result slave_net_query(slave_net_t context, SlaveNetQueryFrame* frame);
Result slave_net_config(slave_net_t context, SlaveNetConfigFrame* frame);

Result slave_net_receive(slave_net_t context, SlaveNetResponseFrame* frame, INT16U timeout);



#if 0
/**********************************************
 *
 * Handlers
 *
 ***********************************************/
typedef void* slave_net_handler_t;
typedef Result (*SlaveNetHandleFunc)(slave_net_t context, SlaveNetResponseFrame* frame);
slave_net_handler_t slave_net_handler_create(slave_net_t slave_net);
void slave_net_destroy(slave_net_handler_t context);
Result modbus_handler_register(slave_net_handler_t context, SlaveNetFunctionCode code, ModbusHandleFunc func);
Result modbus_handler_dispatch(slave_net_handler_t context, INT8U* frame, INT16U size);
#endif


#endif /* BMS_SLAVE_PROTOCOL_H_ */
