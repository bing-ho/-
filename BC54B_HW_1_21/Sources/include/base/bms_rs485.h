#ifndef _BMS_RS485_H__
#define _BMS_RS485_H__
#include "bms_defs.h"
#include "bms_memory.h"


#ifndef BMS_SUPPORT_RS485
#define BMS_SUPPORT_RS485       1
#endif

typedef enum
{
  kRs485Dev0 = 0,
  kRs485Dev1,
  kRs485Dev2,
  kRs485Dev3,
  kRs485MaxDev,
}Rs485Dev;


typedef struct _Rs485Context *_PAGED rs485_t;
//typedef void* handle_t;

/** rs485 device */
typedef int (*Rs485ReceiveHandler)(rs485_t context, char chr, void* user_data);

/**
* @brief initialize rs485 device
*
* @param[in] dev - the device name
* @param[in] band - the band
* @param[in] func - the callback function
* @param[in] user_data - user data for the call function
* @return - the handle of the device. Return NULL if failed.
*/
rs485_t rs485_init(Rs485Dev dev, INT32U band, Rs485ReceiveHandler func, void* user_data);

/**
* @brief set the callback function
*
* @param[in] func - the callback function
* @param[in] user_data - user data for the call function
* @return - the handle of the device. Return NULL if failed.
*/
Result rs485_set_callback(rs485_t context, Rs485ReceiveHandler func, void* user_data);

/**
* @brief send a character
*
* @param[in] handle - the opened handle of the RS485 device
* @param[in] chr - the character
* @return - the error code. 0 if success.
*/
Result rs485_send(rs485_t context, char chr);

/**
* @brief sync receive a character
*
* @param[in] handle - the opened handle of the RS485 device
* @param[out] chr - receive a character
* @param[in] timeout - the timeout for
* @return - the error code. 0 if success.
*/
//Result rs485_recv(rs485_t context, char* chr, INT16U timeout);

/**
* @brief un-initialize rs485 device
*
* @param[in] handle - the opened handle of the RS485 device
* @return - the handle of the device. Return NULL if failed.
*/
Result rs485_uninit(rs485_t context);

INT32U rs485_get_bps(INT8U bps_index);
Result rs485_set_recv_handler(rs485_t context, Rs485ReceiveHandler func, void* user_data);

rs485_t rs485_get_context(Rs485Dev dev);

#endif
