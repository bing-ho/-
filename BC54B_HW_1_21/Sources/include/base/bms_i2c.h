/**
 *
 * Copyright (c) 2011 Ligoo Inc.
 *
 * @file  bms_i2c.h
 * @brief I2C的IO模拟驱动头文件
 * @note 
 * @version 1.00
 * @author
 * @date 2014/2/28
 *
 */
#ifndef _BMS_I2C_H
#define _BMS_I2C_H

#include "includes.h"

typedef INT8U I2C_HANDLE;

typedef union _REG_STR
{
    INT32U* data;
}REG_STR;

typedef struct _I2C_REG
{
    INT8U scl_pos;
    INT8U sda_pos;
    REG_STR scl_dir;
    REG_STR sda_dir;
    REG_STR scl_dat;
    REG_STR sda_dat;
}I2C_REG;

I2C_HANDLE i2c_init(I2C_REG* reg);
void i2c_start(I2C_HANDLE id);
void i2c_stop(I2C_HANDLE id);
INT8U i2c_wait_ack(I2C_HANDLE id);
void i2c_send_ack(I2C_HANDLE id);
void i2c_send_no_ack(I2C_HANDLE id);
INT8U i2c_send_byte(I2C_HANDLE id, INT8U mdata);
int i2c_send_byte_with_ack(I2C_HANDLE id, INT8U mdata);
INT8U i2c_receive_byte(I2C_HANDLE id);

#endif