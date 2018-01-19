/**
 *
 * Copyright (c) 2012 Ligoo Inc.
 *
 * @file bms_util.c
 * @brief
 * @note
 * @author
 * @date 2012-4-26
 *
 */

#include "bms_framer.h"

#if BMS_SUPPORT_FRAMER

#pragma MESSAGE DISABLE C1420 // Result of function-call is ignored

#define FRAMING_WRITE_LEN(BUF, LEN)  *((PINT16U)(BUF)) = LEN
#define FRAMING_READ_LEN(BUF) (*((PINT16U)(BUF)))

framer_t framer_create(PPINT8U buffers, INT16U size, INT16U count)
{
    FramerContext* context;
    if (buffers == NULL || size <= sizeof(INT16U) || count == 0) return NULL;

    context = (FramerContext*) MALLOC(sizeof(FramerContext));
    if (context == NULL || buffers == NULL) return NULL;

    memset(context, 0, sizeof(FramerContext));
    context->max_buffer_count = count;
    context->max_buffer_size = size;
    context->buffers = buffers;
    context->avaliabled_buffer_cout = count;

    context->write_buffer_pos = sizeof(INT16U);

    context->filled_buffer_event = OSSemCreate(0);

    return context;
}

void framer_destroy(framer_t context)
{
    if (context == NULL) return;

    FREE(context);
}

int framer_write(framer_t context, PINT8U data, INT16U size)
{
    if (context == NULL) return -1;
    if (context->avaliabled_buffer_cout == 0) return -1;

    if (context->write_buffer_pos + size > context->max_buffer_size)
    {
        /** drop the data because the buffer overflows */
        return -1;
    }

    if (size == 1)
        *(context->buffers[context->write_buffer_index]
                + context->write_buffer_pos) = *data;
    else
        safe_memcpy(
                context->buffers[context->write_buffer_index]
                        + context->write_buffer_pos, data, size);

    context->write_buffer_pos += size;

    return 0;
}

int framer_write_frame_eof(framer_t context)
{
    if (context == NULL || context->avaliabled_buffer_cout == 0) return -1;

    /** drop the empty frame */
    if (context->write_buffer_pos <= sizeof(INT16U)) return 0;

    /** write the buffer length into the header */
    FRAMING_WRITE_LEN(context->buffers[context->write_buffer_index],
            context->write_buffer_pos - sizeof(INT16U));
    context->write_buffer_index = (context->write_buffer_index + 1)
            % context->max_buffer_count;
    context->write_buffer_pos = sizeof(INT16U);

    context->avaliabled_buffer_cout--;

    OSSemPost(context->filled_buffer_event);

    return 0;
}

int framer_read_frame(framer_t context, PPINT8U frame, PINT16U size, INT16U timeout)
{
    INT8U err;
    if (context == NULL) return -1;

    OSSemPend(context->filled_buffer_event, timeout, &err);
    if (err == OS_ERR_TIMEOUT) return ERR_TIMEOUT;
    if (err != 0) return RES_ERROR;

    *frame = context->buffers[context->read_buffer_index] + sizeof(INT16U);
    *size = FRAMING_READ_LEN(context->buffers[context->read_buffer_index]);

    return 0;
}

int framer_free_frame(framer_t context, PINT8U frame)
{
    if (!context) return -1;
    if (frame != context->buffers[context->read_buffer_index] + sizeof(INT16U))
        return -1;

    context->read_buffer_index = (context->read_buffer_index + 1)
            % context->max_buffer_count;
    ++context->avaliabled_buffer_cout;

    return 0;
}

#endif

