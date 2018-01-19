/**
*
* Copyright (c) 2012 Ligoo Inc.
*
* @file bms_charger_imp.h
* @brief
* @note
* @author
* @date 2012-5-12
*
*/
#ifndef BMS_CHARGER_IMP_H_
#define BMS_CHARGER_IMP_H_

#include "bms_charger.h"
#include "bms_can.h"
#include "bms_config.h"
#include "j1939_cfg.h"

#define TIECHENG_CHARGER_ID                 0xE5

#define TIECHENG_CHARGER_BMS_RECEIVE_ID    0x18FF50E5
#define TIECHENG_CHARGER_CHARGER_RECEIVE_ID 0x1806E5F4
#define TIECHENG_CHARGER_CAN_MODE kCanExtendMode

#define TIECHENG_CHARGER_ENABLE_CONTROL_BYTE 0
#define TIECHENG_CHARGER_DISABLE_CONTROL_BYTE 1

#define TIECHENG_CHARGER_FLAG_HARDWARE_EXCEPTION      0x01
#define TIECHENG_CHARGER_FLAG_TEMPERATURE_EXCEPTION   0x02
#define TIECHENG_CHARGER_FLAG_INPUT_VOLTAGE_EXCEPTION 0x04
#define TIECHENG_CHARGER_FLAG_BATTERY_VOLTAGE_CLOSE   0x08
#define TIECHENG_CHARGER_FLAG_COMM_TIMEOUT            0x10

#define TIECHENG_CHARGER_RECEVIE_FRAME_TIMEOUT        5000
#define TIECHENG_CHARGER_SEND_FRAME_INTERVAL          977 // about 1second


typedef struct
{
    can_t can_context;
    INT16U max_voltage;
    INT16U current;
    INT8U  control_byte;

    INT16U output_voltage;
    INT16S output_current;
    INT16U status;
}TieChengCharger;


void tiecheng_charger_task_create(void);
void tiecheng_charger_task_tx_run(void*);
void tiecheng_charger_task_rx_run(void*);

void tiecheng_charger_fill_control_frame(J1939SendMessageBuff* _PAGED message);

Result tiecheng_charger_on_config_changing(ConfigIndex index, INT16U new_value);

extern void tiecheng_charger_init(void);


#endif /* BMS_CHARGER_IMP_H_ */

