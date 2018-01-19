#ifndef _BMS_CAN_H__
#define _BMS_CAN_H__
#include "bms_defs.h"
#include "bms_log.h"
#include "bms_memory.h"
#include "bms_job.h"
#include "bms_util.h"
#include "bms_base_cfg.h"
#include "can_intermediate.h"

#ifndef BMS_SUPPORT_CAN
#define BMS_SUPPORT_CAN     1
#endif

#if  0
typedef struct _CanContext* can_t;

#define CAN_DATA_MAX_LEN 8 // the maximum size of CAN frame
typedef union
{
    INT32U value; // 32-bit Data Type
    INT16U words[2]; // 16-bit Data Type
    INT8U bytes[4]; // Four 8-bit Data Type
} Can32Bit;

typedef struct
{
    Can32Bit id;
    Can32Bit mask;
    INT8U len; //报文长度
    INT8U data[CAN_DATA_MAX_LEN]; //CAN Message data
} CanMessage;

typedef enum
{
    kCanDev0,           //MASTER_SLAVE_CAN
    kCanDev1,
    kCanDev2,           //EMS_CAN
    kCanDev3,           //DTU_CAN_DEV
    kCanDev4,
    kCanDevMaxCount
} CanDev;

typedef enum
{
    kCanStandardMode = 0,  //
    kCanExtendMode = 1,   //
} CanMode;

typedef enum
{
    kCan50kBps = 50,
    kCan100kBps = 100,
    kCan125kBps = 125,
    kCan250kBps = 250,
    kCan500kBps = 500,
    kCan800kBps = 800,
    kCan1000kBps = 1000
} CanBps;

typedef INT16U (*CanOnReceiveFunc)(can_t handle, CanMessage* msg, void* userdata);

typedef struct
{
    CanOnReceiveFunc func;
    void* user_data;
} CanHandler, *CanHandlerPtr;

/**
 *
 */
typedef struct
{
    CanDev dev; //< CAN Device
    INT32U receive_id; //< CAN Receive ID
    INT32U mask_id; // < CAN Mask ID 
    INT32U receive_1_id; //< CAN Receive ID
    INT32U mask_1_id; // < CAN Mask ID
    INT32U bps; //< bit rate, the value is not supported in the version
    CanMode mode; //< the mode        
    CanMode mode_1; //< the mode

    CanOnReceiveFunc receive_callback; //< the callback handling function
    void* receive_userdata; //< the userdata of the callback function

    CanMessage* _PAGED buffers; //< the mode
    INT8U buffer_count; //< the mode
} CanInfo;



/**********************************************
 *
 * Can Functions
 *
 ***********************************************/
/**
 * initialize the CAN device
 * @param can_info the information of the CAN device
 * @return the context of the CAN device. Return NULL if failed.
 */
can_t can_init(CanInfo* _PAGED can_info);
void can_re_init_init(void);
Result can_send(can_t context, CanMessage* _PAGED msg);
Result dev_can_send(CanDev dev, CanMessage* _PAGED msg);
Result can_receive(can_t context, CanMessage* _PAGED msg, INT16U timeout);
Result can_uninit(can_t context);
Result can_add_receive_id(CanInfo* _PAGED can_info, INT32U id);
CanBps can_get_bps(INT8U bps_index);
can_t can_get_context(CanDev dev);

//CanHandlerPtr can_reader_create(CanMessage* buffers, INT8U size);
//void can_reader_destory(CanHandlerPtr handler);
//Result can_reader_receive(CanHandlerPtr handler, CanMessage* message, INT16U timeout);

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

#endif 

#endif

