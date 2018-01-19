/**
 *
 * Copyright (c) 2014 Ligoo Inc.
 *
 * @file  bms_i2c.h
 * @brief ads1013的IIC驱动头文件
 * @note 
 * @version 1.00
 * @author
 * @date 2014/3/3
 *
 */
#ifndef _ADS1013_H
#define _ADS1013_H

#include "includes.h"
#include "bms_i2c.h"
#include "bms_base_cfg.h"
#include "iic_interface.h"

#define ADS1013_CONFIG              0x42E3
#define ADS1013_REFERENCE           4096 //参考基准
#define ADS1013_AD_MAX              2047
#define ADS1013_AD_MIN              -2048

typedef INT8U ADS1013_HANDLE;

typedef struct _ADS1013_CONTEXT
{
    ADS1013_HANDLE id;
    I2C_HANDLE i2c_handle;
    I2C_REG i2c_reg;
}ADS1013_CXT, * _PAGED ads1013_cxt_t;


ADS1013_HANDLE ads1013_init(I2C_REG* reg, INT16U config);
INT16U ads1013_read(ADS1013_HANDLE handle);
INT8U ads1013_config(ADS1013_HANDLE handle, INT16U config);
INT8U ads1013_require(void);
void ads1013_release(void);
INT8U ads1013_value_is_negative(INT16U value);

#endif