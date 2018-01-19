/**
 *
 * Copyright (c) 2012 Ligoo Inc.
 *
 * @file bms_hmi.h
 * @brief
 * @note
 * @author  
 * @date 2012-4-26
 *
 */

#ifndef _BMS_HMI_H__
#define _BMS_HMI_H__

#include "bms_defs.h"
#include "bms_bmu.h"
#include "bms_clock.h"
#include "bms_charger_common.h"
#include "rs485_hardware.h"
#include "bms_timer.h"
#include "bms_util.h"
#include "bms_modbus.h"
#include "bms_hardware.h"
#include "bms_data_save.h"
#include "bms_mlock.h"

#ifndef BMS_SUPPORT_HMI
#define BMS_SUPPORT_HMI     1
#endif


/**********************************************
 *
 * Structures
 *
 ***********************************************/
typedef struct _HmiContext
{
    modbus_t modbus_context;
    modbus_handler_t modbus_handler;

    //INT8U* buffers[HMI_MAX_RECEIVE_BUFFER_COUNT];

    /* save the current frame to reduce the heap usage */
    PINT8U send_buffer;
    PINT8U register_buffer;
    INT8U  register_buffer_pos;
    INT16U current_slave_index;
    PINT16U volt_temp_show_bits;
    PINT8U frame;
    INT16U frame_size;
    INT16U length;
    INT16U address;
    DateTime g_hmi_now;
} HmiContext;

/**********************************************
 *
 * Public Functions
 *
 ***********************************************/
void hmi_init(void);
void hmi_uninit(void);
void hmi_modbus_handler_init(HmiContext* context);
void hmi_check_rx_frame(HmiContext* context);
void hmi_set_current_slave_index(HmiContext* hmi_context, INT16U value);

#endif
