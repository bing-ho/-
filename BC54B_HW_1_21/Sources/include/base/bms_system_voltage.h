/**
*
* Copyright (c) 2012 Ligoo Inc.
*
* @file bms_system_voltage.h
* @brief
* @note
* @author Liwei Dong
* @date 2014-9-23
*
*/

#ifndef BMS_SYSTEM_VOLTAGE_H_
#define BMS_SYSTEM_VOLTAGE_H_

#include "bms_defs.h"
#include "app_cfg.h"
#include "ads1013.h"
#include "bms_bcu.h"
#include "bms_job.h"
#include "adc.h"

#define BMS_SYSTEM_VOLTAGE_MIN              9000
#define BMS_SYSTEM_VOLTAGE_MAX              36000
#define BMS_SYSTEM_VOLTAGE_LOW_THRESHOLD    5000

#if( SWITCH_ON == UDS_SWITCH_CONFIG)
#define BMS_CAN_VOLTAGE_MIN              5000  //CAN最小通讯电压 设置
#define BMS_CAN_VOLTAGE_MAX              20000 //CAN最大通讯电压 设置
#endif
#define BMS_24V_DC_VOLTAGE_MIN              9000
#define BMS_24V_DC_VOLTAGE_MAX              36000
#define BMS_24V_DC_VOLTAGE_LOW_THRESHOLD    5000

#define BMS_BAT_DET_VOLTAGE_MIN             9000
#define BMS_BAT_DET_VOLTAGE_MAX             36000
#define BMS_BAT_DET_VOLTAGE_LOW_THRESHOLD   5000

void system_voltage_init(void);
INT16U system_voltage_get(void);
INT8U system_voltage_is_usful(void); 
INT8U system_voltage_is_valid(void);
#if( SWITCH_ON == UDS_SWITCH_CONFIG)
INT8U can_voltage_is_valid(void);  //zmx
#endif

void bcu_system_voltage_detect(void* data);//系统供电电压检测

INT16U dc_24V_voltage_get(void);
INT8U dc_24V_voltage_is_usful(void);
INT8U dc_24V_voltage_is_valid(void);
INT8U bcu_calc_24V_dc_voltage(INT16U *volt);
INT16U bcu_get_lead_acid_volt(void);
INT16U bat_det_voltage_get(void);
INT8U bat_det_voltage_is_usful(void);
#endif /* BMS_SYSTEM_VOLTAGE_H_ */
