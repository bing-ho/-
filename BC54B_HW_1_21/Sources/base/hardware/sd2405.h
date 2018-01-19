/*******************************************************************************
**                     ������������Դ�Ƽ����޹�˾ Copyright (c)
**                            http://www.ligoo.cn
**
**
**�� �� ����PCF8563.h
**��    �ߣ�����ΰ
**�������ڣ�2011.06.16
**�޸ļ�¼��

**�ļ�˵��:ʵʱʱ������ͷ�ļ�
**��    ��:v1.0
**��    ע���������ö�ȡʱ��оƬPCF8563����Ϣ
*******************************************************************************/
#ifndef _SD_2405_H
#define _SD_2405_H

#include "bms_clock.h"
#include "bms_i2c.h"
#include "iic_impl.h"

#define CLOCK_BIT_INVALID   0xFF

/**********************************************
 *
 * Clock Address
 *
 ***********************************************/

#define CLOCK_ADDRESS       0x64

/**********************************************
 *
 * Clock Bits
 *
 ***********************************************/
#define CLOCK_BIT_YEAR      0x06
#define CLOCK_BIT_MONTH     0x05
#define CLOCK_BIT_DAY       0x04
#define CLOCK_BIT_WEEK      0x03
#define CLOCK_BIT_HOUR      0x02
#define CLOCK_BIT_MINUTE    0x01
#define CLOCK_BIT_SECOND    0x00

#define CLOCK_BIT_START     0x00
#define CLOCK_BIT_LEN       0x07

#if BMS_SUPPORT_CLOCK == BMS_CLOCK_I2C

#define SCL_TIME_ON()  PTJ_PTJ7 = 1
#define SCL_TIME_OFF() PTJ_PTJ7 = 0
#define SDA_TIME_ON()  PTJ_PTJ6 = 1
#define SDA_TIME_OFF() PTJ_PTJ6 = 0
#define SDA_TIME_IN()  DDRJ_DDRJ6 = 0
#define SDA_TIME_OUT() DDRJ_DDRJ6 = 1
#define CHECK_SDA_TIME() (PTJ_PTJ6 == 1)


void i2c_time_init(void);

INT8U i2c_time_write(INT8U addr, INT8U data);
INT8U i2c_time_read(INT8U *time, INT8U size);

INT8U i2c_time_write_enable(void);
INT8U i2c_time_write_disable(void);

INT8U i2c_time_write_item_start(void);
INT8U i2c_time_write_item_stop(void);
INT8U i2c_time_write_item(INT8U data);

#else

void iic_time_init(void);

INT8U iic_time_write(INT8U addr, INT8U data);
INT8U iic_time_read(INT8U *time, INT8U size);

INT8U iic_time_write_enable(void);
INT8U iic_time_write_disable(void);

INT8U iic_time_write_item_start(void);
INT8U iic_time_write_item_stop(void);
INT8U iic_time_write_item(INT8U data);

#endif //BMS_SUPPORT_CLOCK

#endif //_SD_2405_H
