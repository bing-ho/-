/**
 *
 * Copyright (c) 2012 Ligoo Inc.
 *
 * @file bms_modbus_impl.c
 * @brief
 * @note
 * @author
 * @date 2012-4-26
 *
 */
#include "bms_modbus_impl.h"

#if BMS_SUPPORT_MODBUS
#pragma DATA_SEG __RPAGE_SEG PAGED_RAM

#pragma MESSAGE DISABLE C1420 // Result of function-call is ignored
#pragma MESSAGE DISABLE C5703 // Parameter is not used

modbus_t modbus_create(Rs485Dev dev, INT32U band, INT16U timeout,
        buffers_t buffers, void* pdata)
{
    Rs485Param rs485_param = {0};
    ModbusContext* context = (ModbusContext*) MALLOC(sizeof(ModbusContext));
    if (context == NULL) return NULL;

    /* init the context */
    memset(context, 0, sizeof(ModbusContext));

    /* bind RS485 device*/
   // context->dev = rs485_init(dev, band, modbus_on_receive_chr, context);
   
    rs485_param.band = band;
    context->dev = Rs485_Hardware_Init(dev, &rs485_param, modbus_on_receive_chr, context);   
    
    if (context->dev == NULL) goto failed;

    /* set the timer */
    context->timeout_timer = timer_start(modbus_on_timeout, context, timeout);

    /* set framing */
    context->framer_ctx = framer_create(buffers->buffers, buffers->buffer_size, buffers->buffer_count);

    context->user_data = pdata;
    
    return context;

failed:
    FREE(context);
    return NULL;
}

void modbus_destroy(modbus_t ctx)
{
    if (!ctx) return;

    rs485_uninit(ctx->dev);
    framer_destroy(ctx->framer_ctx);
    timer_stop(ctx->timeout_timer);

    FREE(ctx);
}

int modbus_receive(modbus_t ctx, PPINT8U frame, PINT16U size)
{
    int err;
    INT16U crc;
    if (ctx == NULL) return -1;

    if (ctx->frame) framer_free_frame(ctx->framer_ctx, ctx->frame);
    err = framer_read_frame(ctx->framer_ctx, &ctx->frame, &ctx->frame_size, 0);
    if (err != 0) return err;

    // TODO: disable CRC for test and enable CRC later
    crc = crc_check_bt(ctx->frame, ctx->frame_size);
    if (crc != 0) return RES_FRAME_WRONG_CRC;

    if (frame) *frame = ctx->frame;
    if (size) *size = ctx->frame_size;

    return 0;
}

int modbus_send(modbus_t ctx, PINT8U frame, INT16U size)
{
    int index;
    if (ctx == NULL) return -1;

    for (index = 0; index < size; ++index)
    {
        //OSTimeDly(1);
        rs485_send(ctx->dev, frame[index]);
    }

    return 0;
}

int modbus_send_byte(modbus_t ctx, INT8U chr)
{
    if (ctx == NULL) return -1;
    return rs485_send(ctx->dev, chr);
}

int modbus_reply_read_coils(modbus_t ctx, PINT8U frame, INT8U unit, PINT8U data, INT8U size)
{
    INT16U crc;
    INT16U index = 0;
    if (frame == NULL || data == NULL) return RES_ERR;

    /* set the header */
    WRITE_INT8U(frame, index, unit);
    WRITE_INT8U(frame, index, MODBUS_FUNCTION_READ_COILS);

    /* write the size */
    WRITE_INT8U(frame, index, size);

    /** append the data */
    WRITE_BUFFER(frame, index, data, size);

    /** append CRC */
    crc = crc_check_bt(frame, index);
    WRITE_INT16U(frame, index, crc);

    return modbus_send(ctx, frame, index);
}

int modbus_reply_read_holding_registers(modbus_t ctx, PINT8U frame, INT8U unit, PINT8U data, INT8U size)
{
    INT16U crc;
    INT16U index = 0;
    if (frame == NULL || data == NULL) return RES_ERR;

    /* set the header */
    WRITE_INT8U(frame, index, unit);
    WRITE_INT8U(frame, index, MODBUS_FUNCTION_READ_HOLDING_REGISTERS);

    /** write registers number */
    WRITE_INT8U(frame, index, size);

    /** append the data */
    WRITE_BUFFER(frame, index, data, size);

    /** append CRC */
    crc = crc_check_bt(frame, index);
    WRITE_INT16U(frame, index, crc);

    return modbus_send(ctx, frame, index);
}

int modbus_reply_write_input_register(modbus_t ctx, PINT8U frame, INT8U unit, INT16U address, INT16U value)
{
    INT16U crc;
    INT16U index = 0;
    if (frame == NULL) return RES_ERR;

    /** write the header */
    WRITE_INT8U(frame, index, unit);
    WRITE_INT8U(frame, index, MODBUS_FUNCTION_WRITE_SINGLE_REGISTER);

    /** write register address */
    WRITE_INT16U(frame, index, address);

    /** write the value */
    WRITE_INT16U(frame, index, value);

    /** append CRC */
    crc = crc_check_bt(frame, index);
    WRITE_INT16U(frame, index, crc);

    return modbus_send(ctx, frame, index);
}

int modbus_reply_write_single_colis(modbus_t ctx, PINT8U frame, INT8U unit, INT16U address, INT16U value)
{
    INT16U crc;
    INT16U index = 0;
    if (frame == NULL) return RES_ERR;

    /** write the header */
    WRITE_INT8U(frame, index, unit);
    WRITE_INT8U(frame, index, MODBUS_FUNCTION_WRITE_SINGLE_COILS);

    /** write register address */
    WRITE_INT16U(frame, index, address);

    /** write the value */
    WRITE_INT16U(frame, index, value);

    /** append CRC */
    crc = crc_check_bt(frame, index);
    WRITE_INT16U(frame, index, crc);

    return modbus_send(ctx, frame, index);
}

int modbus_reply_read_input_registers(modbus_t ctx, PINT8U frame, INT8U unit, PINT8U data, INT16U size)
{
    INT16U crc;
    INT16U index = 0;
    if (frame == NULL || data == NULL) return RES_ERR;

    /** write the header */
    WRITE_INT8U(frame, index, unit);
    WRITE_INT8U(frame, index, MODBUS_FUNCTION_READ_INPUT_REGISTERS);

    /** write registers number */
    WRITE_INT8U(frame, index, size);

    /** append the data */
    WRITE_BUFFER(frame, index, data, size);

    /** append CRC */
    crc = crc_check_bt(frame, index);
    WRITE_INT16U(frame, index, crc);

    return modbus_send(ctx, frame, index);
}

int modbus_reply_write_input_registers(modbus_t ctx, PINT8U frame, INT8U unit, INT16U address, INT16U number)
{

    INT16U crc;
    INT16U index = 0;
    if (frame == NULL) return RES_ERR;

    /** write the header */
    WRITE_INT8U(frame, index, unit);
    WRITE_INT8U(frame, index, MODBUS_FUNCTION_READ_INPUT_REGISTERS);

    /** write the address */
    WRITE_INT16U(frame, index, address);
    /** write the register number */
    WRITE_INT16U(frame, index, number);

    /** write CRC */
    crc = crc_check_bt(frame, index);
    WRITE_INT16U(frame, index, crc);

    return modbus_send(ctx, frame, index);
}

int modbus_reply_error(modbus_t ctx, PINT8U frame, INT8U unit, INT8U function_code, INT8U error_code)
{
    INT16U crc;
    INT16U index = 0;
    if (frame == NULL) return RES_ERR;

    /** write the header */
    WRITE_INT8U(frame, index, unit);
    WRITE_INT8U(frame, index, MODBUS_FUNCTION_BASE_ERROR_CODE + function_code);

    /* set the header */
    WRITE_INT8U(frame, index, error_code);

    /** write CRC */
    crc = crc_check_bt(frame, index);
    WRITE_INT16U(frame, index, crc);

    return modbus_send(ctx, frame, index);
}

INT16U modbus_frame_parse_start_address(modbus_t ctx, PINT8U frame)
{
    if (frame == NULL) return 0;

    return GET_INT16U(frame, MODBUS_START_ADDRESS_POS);
}

INT16U modbus_frame_parse_input_single_value(modbus_t ctx, PINT8U frame)
{
    if (frame == NULL) return 0;

    return GET_INT16U(frame, MODBUS_SINGLE_REGISTER_VALUE_POS);
}

INT8U modbus_frame_parse_function_code(modbus_t ctx, PINT8U frame)
{
    if (frame == NULL) return 0;

    return GET_INT8U(frame, MODBUS_FUNCTION_CODE_POS);
}

INT16U modbus_frame_parse_length(modbus_t ctx, PINT8U frame)
{
    if (frame == NULL) return 0;

    return GET_INT16U(frame, MODBUS_DATA_LEN_POS);
}

INT8U modbus_frame_parse_unit_id(modbus_t ctx, PINT8U frame)
{
    if (frame == NULL) return 0;

    return GET_INT8U(frame, MODBUS_UNIT_POS);
}

PINT8U modbus_frame_parse_data(modbus_t ctx, PINT8U frame)
{
    if (frame == NULL) return NULL;

    return frame + MODBUS_DATA_DATA_START_POS;
}

int modbus_on_receive_chr(Rs485Dev handle, char chr, void* user_data)
{
    ModbusContext* context = (ModbusContext*) user_data;
    if (context == NULL) return -1;

    timer_reset(context->timeout_timer);
    framer_write(context->framer_ctx, (PINT8U) &chr, sizeof(chr));

    return 0;
}

void modbus_on_timeout(TimerId id, void* user_data)
{
    ModbusContext* context = (ModbusContext*) user_data;
    if (context == NULL) return;

    /** check the frame */
    framer_write_frame_eof(context->framer_ctx);
}

void* modbus_get_user_data(modbus_t ctx)
{
    if (ctx == NULL) return NULL;
    return ctx->user_data;
}

/********************************************************
 *
 * Handler
 *
 ********************************************************/
/** a group of handler manager */
modbus_handler_t modbus_handler_create(modbus_t modbus)
{
    ModbusHandleContext* context = (ModbusHandleContext*) MALLOC(sizeof(ModbusHandleContext));
    if (context == NULL) return NULL;

    memset(context, 0, sizeof(ModbusHandleContext));
    context->modbus = modbus;

    return context;
}

void modbus_handler_destroy(modbus_handler_t context)
{
    FREE(context);
}

int modbus_handler_register(modbus_handler_t context, ModbusFunctionCode code, ModbusHandleFunc func)
{
    if (code < 0 || code >= MODBUS_FUNCTION_MAX_FUNCTIONS) return -1;

    context->handlers[code] = func;

    return 0;
}

int modbus_handler_dispatch(modbus_handler_t context, PINT8U frame, INT16U size)
{
    INT8U function_code;
    if (context == NULL) return -1;
    function_code = modbus_frame_parse_function_code(context->modbus, frame);
    if (function_code < 0 || function_code >= MODBUS_FUNCTION_MAX_FUNCTIONS) return -1;

    if (context->handlers[function_code] == NULL) return -1;

    return context->handlers[function_code](context->modbus, frame, size);
}

#endif

