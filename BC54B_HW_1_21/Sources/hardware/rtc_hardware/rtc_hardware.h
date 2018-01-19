/*******************************************************************************
**                       安徽力高新能源有限公司 Copyright (c)
**                            http://www.ligoo.cn/ 
** @file	  rtc_hardware.c                                       

** @brief       1.完成RTC模块的数据类型的定义 
                2.完成RTC模块的初始化
                3.完成RTC模块的各种定时器的功能函数
** @copyright		Ligoo Inc.         
** @date		    2017-03-27.
** @author      韩 慧      
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
  * @brief  RTC的中断类型
  ***********************************************/
typedef enum _rtc_interrupt_type {
    RTC_INTERRUPT_DISABLE,
    RTC_INTERRUPT_ALARM,
    RTC_INTERRUPT_FREQUENCY,
    RTC_INTERRUPT_COUNTDOWN,
}rtc_interrupt_type;

/************************************************
  * @enum   rtc_interrupt_single
  * @brief  RTC的独立中断类型
  ***********************************************/
typedef enum _rtc_interrupt_single {
    RTC_INTERRUPT_SINGLE_LOWLEVEL,
    RTC_INTERRUPT_SINGLE_PAUSE,
}rtc_interrupt_single;

/************************************************
  * @struct   rtc_oops
  * @brief    实时时钟RTC所具有的功能属性
 ***********************************************/
typedef struct _rtc_oops 
{
    char (*rtc_init)         (void* __FAR dev);//RTC硬件初始化
    char (*rtc_write_time)   (void* __FAR dev, uint32_t time);//RTC写入时间
    char (*rtc_read_time)    (void* __FAR dev, uint32_t *second);//RTC读取时间
    uint16_t (*rtc_get_ram_size_bit) (void *__FAR dev);//RTC获取RAM空间大小
    char (*rtc_store_data)   (void* __FAR dev, const unsigned char *__FAR dat, 
                              unsigned char offset,unsigned char len);//RTC存储数据功能
    char (*rtc_restore_data) (void* __FAR dev,unsigned char *__FAR dat,unsigned char offset,unsigned char len); //RTC存储数据功能
    char (*rtc_config_interrupt) (void* __FAR dev, rtc_interrupt_type int_type, rtc_interrupt_single int_type_single); //RTC配置中断功能
    char (*rtc_clear_interrupt)  (void* __FAR dev);//RTC清除中断功能
    char (*rtc_set_alarm)        (void* __FAR dev, uint32_t seconds);//RTC设置报警功能
    char (*rtc_get_alarm_time)   (void* __FAR dev, uint32_t *seconds); //RTC获取报警时间
}rtc_oops;
extern const rtc_oops pcf8563_oops;

/************************************************
  * @struct   rtc_dev
  * @brief    实时时钟RTC对应芯片型号所具有的属性
 ***********************************************/
typedef struct _rtc_dev 
{
    const rtc_oops *__FAR prtc_ops;
    void *__FAR chip;
}rtc_dev;

/***********************************************
  * @struct   pcf8563
  * @brief    实时时钟PCF8563具有属性
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