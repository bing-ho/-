/**
 *
 * Copyright (c) 2012 Ligoo Inc.
 *
 * @file bms_relay_control.h
 * @brief
 * @note
 * @author
 * @date 2012-5-22
 *
 */

#ifndef BMS_RELAY_CONTROL_H_
#define BMS_RELAY_CONTROL_H_
#include "bms_defs.h"
#include "app_cfg.h"
#include "bms_system.h"
#include "bms_config.h"
#include "bms_relay.h"

#define RELAY_ACTION_TIME          100

typedef enum
{
    kRelayControlMutexFlagNone = 0,
    kRelayControlMutexFlagDischarge = 1,
    kRelayControlMutexFlagCharge = 2,
}RelayControlMutexFlag;

void relay_control_init(void);

//void relay_control_check(void);
//void relay_control_check_charging(void);
//void relay_control_check_heating(void);
/*
void relay_control_main_relay_on(void);
void relay_control_main_relay_off(void);
void relay_control_charge_relay_off(void);
void relay_control_charge_relay_on(void);

BOOLEAN relay_control_main_relay_is_on(void);
BOOLEAN relay_control_charger_relay_is_on(void);

void relay_control_set_mutex_flag(RelayControlMutexFlag flag);
*/

#define RELAY_DIS_CHG_BIT   1
#define RELAY_CHG_BIT       2
#define RELAY_PRE_CHG_BIT   4

#define RELAY_DEFAULT_ENABLE_FLAG 1

void relay_control_load_config(void);
void relay_control_off_all();

void relay_control_on(RelayControlType type);
void relay_control_off(RelayControlType type);
INT8U relay_control_is_on(RelayControlType type);
void relay_force_control_on(RelayControlType type);
void relay_force_control_off(RelayControlType type);
void relay_force_control_cancle(RelayControlType type);
INT8U relay_is_force_control_on(RelayControlType type);
INT8U relay_is_force_control_off(RelayControlType type);
INT8U relay_control_get_id(RelayControlType type);
RelayControlType relay_control_get_type(INT8U type);
void relay_control_set_type(INT8U index, RelayControlType type);

void  relay_control_set_flag(RelayControlType type, INT8U is_on);
INT8U relay_control_get_flag(void);

void relay_control_set_enable_flag(RelayControlType type, INT8U is_on);
INT8U relay_control_get_enable_flag(void);
INT8U relay_control_enable_is_on(RelayControlType type);
RelayTroubleStatus relay_control_get_instant_trouble_status(RelayControlType type, InputControlType input_signal_type);
RelayTroubleStatus relay_control_get_trouble_status(RelayControlType type, InputControlType input_signal_type);
void relay_control_update_adhesion_status(RelayControlType type, INT16U tv);
RelayTroubleStatus relay_control_get_adhesion_status(RelayControlType type);
Result relay_control_delay_on(RelayControlType type, INT16U delay);
Result relay_control_delay_off(RelayControlType type, INT16U delay);
RelayCtlStatus relay_control_get_force_command(RelayControlType type);

#endif /* BMS_RELAY_H_ */
