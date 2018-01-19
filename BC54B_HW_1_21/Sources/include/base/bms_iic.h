/**
 *
 * Copyright (c) 2011 Ligoo Inc.
 *
 * @file  bms_iic.h
 * @brief IIC的驱动头文件
 * @note 
 * @version 1.00
 * @author
 * @date 2014/9/23
 *
 */
#ifndef _BMS_IIC_H
#define _BMS_IIC_H

#include "includes.h"
#include "bms_memory.h"
#include "bms_util.h"


#define I2C_SUCCESS		   			0x00				/*I2C访问成功*/
#define I2C_ERR	    	 			0x01				/*I2C访问失败*/



void iic_init(void);
void iic_start(void);
void iic_stop(void);
uchar iic_send_data(uchar ch);
uchar iic_wait_ack(void);
uchar iic_write(uchar addr, uchar sub, uchar* data, uchar num);
uchar iic_write_byte(uchar addr,uchar sub,uchar data);
uchar iic_read(uchar addr,uchar sub,uchar* ptr,uchar num);

//IIC0接口
void iic0_init(void);
void iic0_start(void);
void iic0_stop(void);
uchar iic0_send_data(uchar ch);
uchar iic0_wait_ack(void);
uchar iic0_write(uchar addr, uchar sub, uchar* data, uchar num);
uchar iic0_write_byte(uchar addr,uchar sub,uchar data);
uchar iic0_read(uchar addr,uchar sub,uchar* ptr,uchar num);

#endif