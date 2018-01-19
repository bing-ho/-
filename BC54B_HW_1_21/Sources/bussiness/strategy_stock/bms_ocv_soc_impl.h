/**
 *
 * Copyright (c) 2012 Ligoo Inc.
 *
 * @file bms_ocv_soc_impl.h
 * @brief
 * @note
 * @author Liwei Dong
 * @date 2015-1-6
 *
 */

#ifndef BMS_OCV_SOC_IMPL_H_
#define BMS_OCV_SOC_IMPL_H_

#include "bms_ocv_soc.h"


#define OCV_SOC_GET_TIME_INTERVAL(NOW_TIME, LAST_TIME)    (NOW_TIME > LAST_TIME ? (NOW_TIME - LAST_TIME) / 3600 : 0)
#define OCV_SOC_IS_CURRENT_ACTIVED()  (abs(bcu_get_current()) >= 30)

float bms_ocv_soc_get_soc_float(INT8U temperature, INT16U voltage);

#endif /* BMS_OCV_SOC_IMPL_H_ */