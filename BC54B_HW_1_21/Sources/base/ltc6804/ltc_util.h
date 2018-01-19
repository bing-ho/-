/*******************************************************************************
 **                       ��������Դ�������޹�˾ Copyright (c)
 **                            http://www...
 **�� �� ��:LTC_Util.h
 **��    ��:��־��
 **��������:2016.12.15
 **�ļ�˵��:
 **�޸ļ�¼:
 **��    ��:V1.0
 **��    ע:
 *******************************************************************************/
#ifndef  _LTC_UTIL_H_
#define  _LTC_UTIL_H_

#include "spi_ltc6804.h" 
#include "bms_business_defs.h"

#define TEMP_CABLE_SHORT_VAULE     0xFF//�¶����߶�·ֵ
#define TEMP_CABLE_OPEN_VAULE      0xFE//�¶����߿�·ֵ

//PEC15����
INT16U pec15_calc(INT8U len, const INT8U *far data);
//CRC4����
INT8U crc4_calc(INT16U data);
//Ƭѡ�ź�����
void ltc6804_cs_pin_low(void);
//Ƭѡ�ź�����
void ltc6804_cs_pin_high(void);
//��IC����
void wakeup_ltc6804_sleep(void);
//��isoSPI���߻���
void wakeup_iso_spi_idle(void);
//�������
void ltc6804_buffer_set(void*__far dest, INT8U value, INT16U size);
//�¶Ȳ��
INT8U ltc6804_temp_query(INT16U R);

#endif


