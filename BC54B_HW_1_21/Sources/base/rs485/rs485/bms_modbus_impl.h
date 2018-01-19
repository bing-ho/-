/**
 *
 * Copyright (c) 2012 Ligoo Inc.
 *
 * @file bms_modbus_impl.h
 * @brief
 * @note
 * @author
 * @date 2012-4-26
 *
 */

#ifndef BMS_MODBUS_IMPL_H_
#define BMS_MODBUS_IMPL_H_
#include "bms_modbus.h"
#include "bms_timer.h"
#include "bms_framer.h"
#include "bms_util.h"

#define MODBUS_UNIT_POS          0
#define MODBUS_FUNCTION_CODE_POS    1
#define MODBUS_START_ADDRESS_POS    2  // 2, 3
#define MODBUS_SINGLE_REGISTER_VALUE_POS 4 // 4, 5
#define MODBUS_DATA_LEN_POS         4  // 4, 5
#define MODBUS_DATA_DATA_START_POS  7

#if 0
#define MODBUS_REPLY_LEN_POS         2
#define MODBUS_REPLY_DATA_POS        3
#define MODBUS_REPLY_ERROR_CODE_POS  2

#define MODBUS_REPLY_WRITE_REGISTERS_ADDRESS_POS 2 // 2, 3
#define MODBUS_REPLY_WRITE_REGISTERS_NUMBER_POS  4 // 4, 5
#endif

typedef struct _modbus {
	rs485_t dev;

	TimerId timeout_timer;
	framer_t framer_ctx;

	PINT8U frame;
	INT16U frame_size;
	void* user_data;
} ModbusContext, *modbus_t;

typedef struct _modbus_handler {
	modbus_t modbus;

	ModbusHandleFunc handlers[MODBUS_FUNCTION_MAX_FUNCTIONS];
} ModbusHandleContext, *modbus_handler_t;

/**
 * handle receiving a character from RS485 or RS232
 * @param handle the device handle
 * @param chr the received character
 * @param user_data the user data
 * @return
 */
int modbus_on_receive_chr(rs485_t handle, char chr, void* user_data);

/**
 * handle the timeout from the timer
 * @param id the timer id
 * @param user_data the user data
 */
void modbus_on_timeout(TimerId id, void* user_data);

#endif /* BMS_MODBUS_IMPL_H_ */
