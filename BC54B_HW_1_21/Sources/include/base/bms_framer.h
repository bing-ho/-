/**
 *
 * Copyright (c) 2012 Ligoo Inc.
 *
 * @file bms_util.h
 * @brief
 * @note
 * @author
 * @date 2012-4-26
 *
 */

#ifndef _BMS_UTIL_H__
#define _BMS_UTIL_H__

#include "bms_defs.h"
#include "includes.h"
#include "bms_util.h"
#include "bms_memory.h"

#ifndef BMS_SUPPORT_FRAMER
#define BMS_SUPPORT_FRAMER      1
#endif

typedef struct _FramingContext
{
    PPINT8U buffers;
    INT16U max_buffer_count;
    INT16U max_buffer_size;

    OS_EVENT* filled_buffer_event;

    INT16U write_buffer_index;
    INT16U write_buffer_pos;

    INT16U read_buffer_index;

    INT16U avaliabled_buffer_cout;
} FramerContext, *framer_t;


/**
 * Framing
 */
framer_t framer_create(PPINT8U buffers, INT16U size, INT16U count);
void framer_destroy(framer_t context);
int framer_write(framer_t context, PINT8U data, INT16U size);
int framer_write_frame_eof(framer_t context);
int framer_read_frame(framer_t context, PPINT8U frame, PINT16U size, INT16U timeout);
int framer_free_frame(framer_t context, PINT8U frame);


#endif
