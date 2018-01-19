/**
*
* Copyright (c) 2012 Ligoo Inc.
*
* @file bms_byu.h
* @brief
* @note
* @author
* @date 2014-3-23
*
*/

#ifndef BMS_BYU_H_
#define BMS_BYU_H_

#include "includes.h"
#include "bms_defs.h"

#define BYU_USE_SLAVE_INDEX   0   //0, 1, 2...

typedef struct _BMS_BYU_CXT
{
    INT16U volt[MAX_LTC6803_NUM][LTC_CELLV_NUM];
    INT8U temp[MAX_LTC6803_NUM][LTC_CELLV_NUM];
    INT16U AverageVolt;
    INT8U LtcVHeartBeat[MAX_LTC6803_NUM];
    INT8U LtcTHeartBeat[MAX_LTC6803_NUM];
    INT8U batCnt[MAX_LTC6803_NUM];
    INT8U tempCnt[MAX_LTC6803_NUM];
    INT16U batteryBlanceState[MAX_LTC6803_NUM];
}BMS_BYU_CXT;

#define BMS_INFO    BMS_BYU_CXT
#define g_bmsInfo   g_byuCxt

void byu_init(void);
INT8U byu_all_battery_sample_board_is_online(void);

#endif