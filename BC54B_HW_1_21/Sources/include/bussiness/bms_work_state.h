/**
 *
 * Copyright (c) 2012 Ligoo Inc.
 *
 * @file bms_work_state.h
 * @brief
 * @note
 * @author
 * @date 2014-10-21
 *
 */

#ifndef BMS_WORK_STATE_H_
#define BMS_WORK_STATE_H_

#include "includes.h"
#include "bms_system.h"
#include "bms_business_defs.h"
#include "bms_bcu.h"
#include "bms_data_save.h"
#include "bms_system_voltage.h"
#include "ect_intermediate.h"
#include "power_down_data_save.h"

#define BMS_WORK_STATE_JOB_PERIODIC     500
#define BMS_WORK_STATE_STOP_DELAY       2000 //BMS工作状态进入停止状态的延时 ms
#define BMS_WORK_STATE_STOP_REL_DELAY   1000 //BMS工作状态恢复正常状态的延时 ms
#define BMS_POWER_OFF_ALLOWED_TIMEOUT   3000 //允许下电超时延时时间 ms
#define BMS_POWER_OFF_ALLOWED_TIMEOUT_MAX   5000 //最大允许下电超时延时时间 ms

void bms_work_state_init(void);
void bms_system_protect(void);
void bms_system_unprotect(void);
INT8U bms_system_power_off_is_allowed(void);
INT8U bms_power_trigger_is_not_rtc(void);
INT8U bms_get_power_trigger_signal(void);
void low_power_mode(void); 

#endif /* BMS_WORK_STATE_H_ */
