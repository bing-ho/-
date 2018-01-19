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

#define OCV_SOC_LINEAR_DIFFERENTIAL_EN  0 //���Բ�ֵʹ��

#define OCV_SOC_TEMPERATURE_BUFF_MAX    6
#define OCV_SOC_SOC_BUFF_MAX            9

#define OCV_SOC_POWER_UP_CHECK_DLY      2000 //ms
#define OCV_SOC_CHECK_INTERVAL          24 //�ֱ��ʣ�hour
#define OCV_SOC_CHECK_TIME_INTERVAL     1800000 //�ֱ��ʣ�ms
#define OCV_SOC_CALIB_DIFF_SOC_MIN      0 //���������·��ѹУ����СSOC��ֵ
#define OCV_SOC_SOC_CHANGE_MAX          5


#define SOH_CALIB_START_SOC_MAX         25 // 1%/bit��ʼSOC
#define SOH_CALIB_START_LV_MAX          3660 //mv/bit ��ʼ��͵�ѹ���ֵ
#define SOH_CALIB_STOP_SOC_MIN          75
#define SOH_CALIB_STOP_LV_MIN           4000
#define SOH_CALIB_LIMIT_CUR             40 //0.1A/bit Ĭ��С����ֵ
#define SOH_CALIB_LIMIT_CUR_MAX         50 //С�������������ֵ
#define SOH_CALIB_LIMIT_CUR_TIME        60000 //ms/bit С�������ʱ��
#define SOH_CALIB_CHG_TIME_MIN          7200000 //ms/bit��С��Ч���ʱ��
#define TOTAL_CAP_DECREASE_MAX          10//0.1AH 1 //AH ���������������������ֵ
#define TOTAL_CAP_INCREASE_MAX          10//0.1AH 1 //AH ���������������������ֵ
#define SOH_CALIB_VALID_CHG_AH_MIN      config_get(kTotalCapIndex)/2 //��С��Ч���AH��
#define SOH_CALIB_CHG_FACTOR            0.99 //��س��Ч��

#define CHARGER_IS_CONNECTED()          (charger_is_connected()) //ֻ������������

INT8U bms_ocv_soc_check(void); /** Ҫ����OCV-SOC��ֻ��ʵʱ���ô˺������� */
void bms_soh_check(void);

#endif  /* BMS_OCV_SOC_H_ */