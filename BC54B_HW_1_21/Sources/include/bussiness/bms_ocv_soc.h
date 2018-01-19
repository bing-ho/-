/**
 *
 * Copyright (c) 2012 Ligoo Inc.
 *
 * @file bms_ocv_soc.h
 * @brief
 * @note
 * @author Liwei Dong
 * @date 2015-1-6
 *
 */

#ifndef BMS_OCV_SOC_H_
#define BMS_OCV_SOC_H_

#include "includes.h"
#include "bms_bcu.h"

#define OCV_SOC_LINEAR_DIFFERENTIAL_EN  0 //线性插值使能

#define OCV_SOC_TEMPERATURE_BUFF_MAX    6
#define OCV_SOC_SOC_BUFF_MAX            9

#define OCV_SOC_POWER_UP_CHECK_DLY      2000 //ms
#define OCV_SOC_CHECK_INTERVAL          24 //分辨率：hour
#define OCV_SOC_CHECK_TIME_INTERVAL     1800000 //分辨率：ms
#define OCV_SOC_CALIB_DIFF_SOC_MIN      0 //电池启动开路电压校正最小SOC差值
#define OCV_SOC_SOC_CHANGE_MAX          5


#define SOH_CALIB_START_SOC_MAX         25 // 1%/bit起始SOC
#define SOH_CALIB_START_LV_MAX          3660 //mv/bit 起始最低电压最大值
#define SOH_CALIB_STOP_SOC_MIN          75
#define SOH_CALIB_STOP_LV_MIN           4000
#define SOH_CALIB_LIMIT_CUR             40 //0.1A/bit 默认小电流值
#define SOH_CALIB_LIMIT_CUR_MAX         50 //小电流充电最大电流值
#define SOH_CALIB_LIMIT_CUR_TIME        60000 //ms/bit 小电流充电时长
#define SOH_CALIB_CHG_TIME_MIN          7200000 //ms/bit最小有效充电时间
#define TOTAL_CAP_DECREASE_MAX          10//0.1AH 1 //AH 单次向下修正总容量最大值
#define TOTAL_CAP_INCREASE_MAX          10//0.1AH 1 //AH 单次向上修正总容量最大值
#define SOH_CALIB_VALID_CHG_AH_MIN      config_get(kTotalCapIndex)/2 //最小有效充电AH数
#define SOH_CALIB_CHG_FACTOR            0.99 //电池充电效率

#define CHARGER_IS_CONNECTED()          (charger_is_connected()) //只适用于慢充充电

INT8U bms_ocv_soc_check(void); /** 要开启OCV-SOC，只需实时调用此函数即可 */
void bms_soh_check(void);

#endif  /* BMS_OCV_SOC_H_ */