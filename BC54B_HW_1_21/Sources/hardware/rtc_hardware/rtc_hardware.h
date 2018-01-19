/*******************************************************************************
**                       ������������Դ���޹�˾ Copyright (c)
**                            http://www.ligoo.cn/ 
** @file	 �b rtc_hardware.c                                       

** @brief       1.���RTCģ����������͵Ķ��� 
                2.���RTCģ��ĳ�ʼ��
                3.���RTCģ��ĸ��ֶ�ʱ���Ĺ��ܺ���
** @copyright		Ligoo Inc.         
** @date		    2017-03-27.
** @author      �� ��      
*******************************************************************************/ 
#ifndef __RTC_HARDWARE_H__
#define __RTC_HARDWARE_H__


#include "stdint.h"
#include "iic_hardware.h"
#include "bms_defs.h"
#include "bms_system.h"
#include "bms_util.h"
#include "iic_interface.h"


/************************************************
  * @enum   rtc_interrupt_type
  * @brief  RTC���ж�����
  ***********************************************/
typedef enum _rtc_interrupt_type {
    RTC_INTERRUPT_DISABLE,
    RTC_INTERRUPT_ALARM,
    RTC_INTERRUPT_FREQUENCY,
    RTC_INTERRUPT_COUNTDOWN,
}rtc_interrupt_type;

/************************************************
  * @enum   rtc_interrupt_single
  * @brief  RTC�Ķ����ж�����
  ***********************************************/
typedef enum _rtc_interrupt_single {
    RTC_INTERRUPT_SINGLE_LOWLEVEL,
    RTC_INTERRUPT_SINGLE_PAUSE,
}rtc_interrupt_single;

/************************************************
  * @struct   rtc_oops
  * @brief    ʵʱʱ��RTC�����еĹ�������
 ***********************************************/
typedef struct _rtc_oops 
{
    char (*rtc_init)         (void* __FAR dev);//RTCӲ����ʼ��
    char (*rtc_write_time)   (void* __FAR dev, uint32_t time);//RTCд��ʱ��
    char (*rtc_read_time)    (void* __FAR dev, uint32_t *second);//RTC��ȡʱ��
    uint16_t (*rtc_get_ram_size_bit) (void *__FAR dev);//RTC��ȡRAM�ռ��С
    char (*rtc_store_data)   (void* __FAR dev, const unsigned char *__FAR dat, 
                              unsigned char offset,unsigned char len);//RTC�洢���ݹ���
    char (*rtc_restore_data) (void* __FAR dev,unsigned char *__FAR dat,unsigned char offset,unsigned char len); //RTC�洢���ݹ���
    char (*rtc_config_interrupt) (void* __FAR dev, rtc_interrupt_type int_type, rtc_interrupt_single int_type_single); //RTC�����жϹ���
    char (*rtc_clear_interrupt)  (void* __FAR dev);//RTC����жϹ���
    char (*rtc_set_alarm)        (void* __FAR dev, uint32_t seconds);//RTC���ñ�������
    char (*rtc_get_alarm_time)   (void* __FAR dev, uint32_t *seconds); //RTC��ȡ����ʱ��
}rtc_oops;
extern const rtc_oops pcf8563_oops;

/************************************************
  * @struct   rtc_dev
  * @brief    ʵʱʱ��RTC��ӦоƬ�ͺ������е�����
 ***********************************************/
typedef struct _rtc_dev 
{
    const rtc_oops *__FAR prtc_ops;
    void *__FAR chip;
}rtc_dev;

/***********************************************
  * @struct   pcf8563
  * @brief    ʵʱʱ��PCF8563��������
***********************************************/
typedef struct pcf8563 
{
    const struct i2c_bus *__FAR bus;
    uint8_t status;
};

#ifndef __FAR
#define __FAR
#endif

typedef struct pcf8563 *__FAR pcf8563_t;
#endif