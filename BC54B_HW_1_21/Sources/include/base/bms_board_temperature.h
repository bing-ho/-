/**
*
* Copyright (c) 2012 Ligoo Inc.
*
* @file bms_board_temperature.h
* @brief
* @note
* @author Liwei Dong
* @date 2014-9-23
*
*/

#ifndef BMS_BOARD_TEMPERATURE_H_
#define BMS_BOARD_TEMPERATURE_H_

#include "bms_defs.h"
#include "app_cfg.h"
#include "adc.h"
#include "bms_job.h"
#include "NTC.h"
#include "bms_util.h"
#include "bms_bmu.h"

#define NCP18XH103_MAX          34
#define NCP18XH103_TEMP_FACTOR  5 //温度分辨率为5
#define NCP18XH103_TEMP_OFFSET  -40
#define NCP18XH103_TEMP_MIN     -40
#define NCP18XH103_TEMP_MAX     125

void board_temperature_init(void);
INT8U board_temperature_get(void); 
INT8U board_temperature2_get(void); 

INT8U bcu_calc_board_temperature(INT8U which, INT8U *temp);
void bcu_board_temperature_detect(void* data);//系统供电电压检测
INT8U board_average_temperature_get(void);

#endif /* BMS_BOARD_TEMPERATURE_H_ */