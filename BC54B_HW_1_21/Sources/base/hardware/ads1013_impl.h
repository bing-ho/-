/**
*
* Copyright (c) 2014 Ligoo Inc.
*
* @file ads1013_impl.h
* @brief
* @note
* @author Liwei Dong
* @date 2014-3-3
*
*/

#ifndef ADS1013_IMPL_H_
#define ADS1013_IMPL_H_

#include "ads1013.h"

#define ADS1013_SUPPORT_DEV_MAX     5

#define ADS1013_ADDR                0x90
#define ADS1013_CONFIG_ADDR         0x01
#define ADS1013_CONVERSION_ADDR     0x00

ads1013_cxt_t ads1013_get_free_cxt(void);

#endif