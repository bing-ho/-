/*******************************************************************************
 **                       安徽新能源技术有限公司 Copyright (c)
 **                            http://www...
 **文 件 名:LTC_Util.h
 **作    者:曹志勇
 **创建日期:2016.12.15
 **文件说明:
 **修改记录:
 **版    本:V1.0
 **备    注:
 *******************************************************************************/
#ifndef  _LTC_UTIL_H_
#define  _LTC_UTIL_H_

#include "spi_ltc6804.h" 
#include "bms_business_defs.h"

#define TEMP_CABLE_SHORT_VAULE     0xFF//温度排线短路值
#define TEMP_CABLE_OPEN_VAULE      0xFE//温度排线开路值

//PEC15计算
INT16U pec15_calc(INT8U len, const INT8U *far data);
//CRC4计算
INT8U crc4_calc(INT16U data);
//片选信号拉低
void ltc6804_cs_pin_low(void);
//片选信号拉高
void ltc6804_cs_pin_high(void);
//将IC唤醒
void wakeup_ltc6804_sleep(void);
//将isoSPI总线唤醒
void wakeup_iso_spi_idle(void);
//清除数组
void ltc6804_buffer_set(void*__far dest, INT8U value, INT16U size);
//温度查表
INT8U ltc6804_temp_query(INT16U R);

#endif


