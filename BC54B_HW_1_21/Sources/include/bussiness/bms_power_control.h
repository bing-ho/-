/**
 *
 * Copyright (c) 2012 Ligoo Inc.
 *
 * @file bms_power_control.h
 * @brief
 * @note
 * @author Liwei Dong
 * @date 2015-1-6
 *
 */

#ifndef BMS_POWER_CONTROL_H_
#define BMS_POWER_CONTROL_H_

#include "includes.h"
#include "bms_bcu.h"


INT16U bms_get_discharge_current_max(void); /** 获取短时放电电流 时间较短 如(30s)*/
INT16U bms_get_discharge_continue_current_max(void); /** 获取持续放电电流 时间较长 如(30min)*/
INT16U bms_get_charge_current_max(void); /** 获取短时充电电流 时间较短 如(30s)*/
INT16U bms_get_charge_continue_current_max(void); /** 获取持续充电电流 时间较长 如(30min)*/

#endif  /* BMS_POWER_CONTROL_H_ */