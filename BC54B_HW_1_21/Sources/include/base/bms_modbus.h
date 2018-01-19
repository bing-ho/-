/**
 *
 * Copyright (c) 2012 Ligoo Inc.
 *
 * @file bms_modbus.h
 * @brief
 * @note
 * @author
 * @date 2012-4-26
 *
 */

#ifndef BMS_MODBUS_H_
#define BMS_MODBUS_H_
#include "rs485_hardware.h"
#include "bms_buffer.h"
#include "bms_memory.h"

#ifndef BMS_SUPPORT_MODBUS
#define BMS_SUPPORT_MODBUS      1
#endif

typedef struct _modbus* modbus_t;
typedef struct _modbus_handler* modbus_handler_t;

typedef struct _ModbusFrameHeader
{
    INT8U address;
    INT8U function;
    INT16U start_address;
    INT16U number;
} ModbusFrameHeader;

typedef enum _ModbusFunctionCode
{
    MODBUS_FUNCTION_READ_COILS = 0x01,
    MODBUS_FUNCTION_READ_DISCRETE_INPUTS = 0x02,
    MODBUS_FUNCTION_READ_HOLDING_REGISTERS = 0x03,
    MODBUS_FUNCTION_READ_INPUT_REGISTERS = 0x04,
    MODBUS_FUNCTION_WRITE_SINGLE_COILS = 0x05,
    MODBUS_FUNCTION_WRITE_SINGLE_REGISTER = 0x06,
    MODBUS_FUNCTION_READ_EXCPETION_STATUS = 0x07,
    MODBUS_FUNCTION_WRITE_MULTIPLE_COILS = 0x0F,
    MODBUS_FUNCTION_WRITE_MULTIPLE_REGISTERS = 0x10,
    MODBUS_FUNCTION_MAX_FUNCTIONS,
}ModbusFunctionCode;

/****************************************
 * Error code for MODBUS
 */
typedef enum _ModbusExceptionCode
{
    MODBUS_EXECPTION_NOT_SUPPORT_FUNCTION_CODE = 0x01,
    MODBUS_EXECPTION_INVALID_ADDRESS_LENGTH = 0x02,
    MODBUS_EXECPTION_OUT_OF_RANGE_NUMBER = 0x03,
    MODBUS_EXECPTION_CANNOT_READ = 0x04,
}ModbusExceptionCode;

#define MODBUS_FUNCTION_BASE_ERROR_CODE 0x80

/********************************************************
 *
 * Modbus
 *
********************************************************/
typedef int (*ModbusHandleFunc)(modbus_t modbus, PINT8U frame, INT16U size);

/**
 * @brief
 *
 * @param[in] dev the RS485 device
 * @param[in] band the band rate
 * @param[in] frame_interval timeout of the frame
 * @param[in] buffers allocate buffers for receiving frames.
 *            The purpose is that modbus avoid to malloc buffers
 * @return - the context of modbus
 */
modbus_t modbus_create(Rs485Dev dev, INT32U band, INT16U frame_interval,
        buffers_t buffers, void* pdata);

/**
 * destroy modbus
 * @param ctx the modbus context
 */
void modbus_destroy(modbus_t ctx);

/** control function */
/**
 * receive a frame
 * @param ctx the context of modbus
 * @param frame the data frame address
 * @param size
 * @return
 */
int modbus_receive(modbus_t ctx, PPINT8U frame, PINT16U size);

/**
 * Send a frame
 * @param ctx the context of modbus
 * @param frame the address of the data frame
 * @param size the size of the data frame
 * @return
 */
int modbus_send(modbus_t ctx, PINT8U frame, INT16U size);

/**
 *  send a byte
 * @param ctx the modbus context
 * @param chr the sending char
 * @return
 */
int modbus_send_byte(modbus_t ctx, INT8U chr);

/**
 * Reply a read coils response
 * @param ctx the modbus context
 * @param frame the data frame address
 * @param unit unit id
 * @param data the address of the coils
 * @param size the size of the coils
 * @return return 0 if successful
 */
int modbus_reply_read_coils(modbus_t ctx, PINT8U frame, INT8U unit, PINT8U data, INT8U size);

/**
 * reply read-holding-registers response
 * @param ctx the modbus context
 * @param frame the data frame address
 * @param unit
 * @param data
 * @param size
 * @return
 */
int modbus_reply_read_holding_registers(modbus_t ctx, PINT8U frame, INT8U unit, PINT8U data, INT8U size);

/**
 * reply read-input-registers response
 * @param ctx the modbus context
 * @param frame the data frame address
 * @param unit
 * @param data
 * @param size
 * @return
 */
int modbus_reply_read_input_registers(modbus_t ctx, PINT8U frame, INT8U unit, PINT8U data, INT16U size);

int modbus_reply_write_input_register(modbus_t ctx, PINT8U frame, INT8U unit, INT16U address, INT16U value);

int modbus_reply_write_single_colis(modbus_t ctx, PINT8U frame, INT8U unit, INT16U address, INT16U value);

/**
 * Reply write-input-registers response
 * @param ctx the modbus context
 * @param frame the data frame address
 * @param unit
 * @param address
 * @param size
 * @return
 */
int modbus_reply_write_input_registers(modbus_t ctx, PINT8U frame, INT8U unit, INT16U address, INT16U number);

/**
 * Reply a error pdu
 * @param ctx the modbus context
 * @param frame the data frame address
 * @param unit
 * @param function_code
 * @param error_code
 * @return
 */
int modbus_reply_error(modbus_t ctx, PINT8U frame, INT8U unit, INT8U function_code, INT8U error_code);


/** parsing received frame function */
/**
 * Parse the address from the frame
 * @param ctx the modbus context
 * @param frame the data frame address
 * @return
 */
INT16U modbus_frame_parse_start_address(modbus_t ctx, PINT8U frame);

INT16U modbus_frame_parse_input_single_value(modbus_t ctx, PINT8U frame);

/**
 * Parse the function code from the frame
 * @param ctx the modbus context
 * @param frame the data frame address
 * @return
 */
INT8U modbus_frame_parse_function_code(modbus_t ctx, PINT8U frame);

/**
 * Parse the length from the frame
 * @param ctx the modbus context
 * @param frame the data frame address
 * @return
 */
INT16U modbus_frame_parse_length(modbus_t ctx, PINT8U frame);

/**
 * Parse the unit id from the frame
 * @param ctx the modbus context
 * @param frame the data frame address
 * @return
 */
INT8U modbus_frame_parse_unit_id(modbus_t ctx, PINT8U frame);

/**
 * Parse the data from the frame
 * @param ctx the modbus context
 * @param frame the data frame address
 * @return
 */
PINT8U modbus_frame_parse_data(modbus_t ctx, PINT8U frame);

/**
 * get user data ptr from the modbus context
 * @param ctx the modbus context 
 * @return ptr of the user data
 */
void* modbus_get_user_data(modbus_t ctx);

/********************************************************
 *
 * Handler
 *
 * a group of handler manager
 *
********************************************************/
/**
 * Create a handler manager
 * @param modbus
 * @return
 */
modbus_handler_t modbus_handler_create(modbus_t modbus);

/**
 * Destroy the handler manager
 * @param context
 */
void modbus_handler_destroy(modbus_handler_t context);

/**
 * Register a handler
 * @param context
 * @param code
 * @param func
 * @return
 */
int modbus_handler_register(modbus_handler_t context, ModbusFunctionCode code, ModbusHandleFunc func);

/**
 * Dispatch the modbus frame to the handlers
 * @param context
 * @param frame the data frame address
 * @param size the frame size
 * @return
 */
int modbus_handler_dispatch(modbus_handler_t context, PINT8U frame, INT16U size);


#endif /* BMS_MODBUS_H_ */
