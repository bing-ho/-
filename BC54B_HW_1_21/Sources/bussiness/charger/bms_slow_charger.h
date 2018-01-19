/**
*
* Copyright (c) 2012 Ligoo Inc.
*
* @file bms_slow_charger.h
* @brief
* @note
* @author
* @date 2015-2-2
*
*/
#ifndef BMS_SLOW_CHARGER_H_
#define BMS_SLOW_CHARGER_H_

#include "includes.h"
#include "bms_charger.h"
#include "bms_can.h"
#include "bms_config.h"
#include "j1939_cfg.h"
#include "bms_ocv_soc.h"

#define BMS_SLOW_CHARGER_RECEIVE_ID     0x18FF50E5
#define BMS_SLOW_CHARGER_SEND_ID        0x1806E5F4

void bms_slow_charger_init(J1939CanInfo* _PAGED can_info);
void bms_slow_charger_uninit(void);
void bms_slow_charger_communication_check(void);
INT8U bms_slow_charger_is_communication(void);
void bms_slow_charger_set_communication_flag(INT8U comm_flag);
INT8U bms_slow_charger_is_connected(void);
void bms_slow_charger_send_message(CanDev dev);
INT8U bms_is_slow_charger_can_id(INT32U id);
Result bms_slow_charger_receive(CanMessage* _PAGED rec_msg);
Result bms_slow_charger_receive_message(J1939CanContext* _PAGED context, J1939RecMessage* _PAGED rec_msg);
void bms_slow_charger_set_charging_flag(INT8U flag);
INT8U bms_slow_charger_is_charging(void);

#endif