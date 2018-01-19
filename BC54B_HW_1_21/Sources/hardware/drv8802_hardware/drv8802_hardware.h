/*******************************************************************************
**                       安徽力高新能源有限公司 Copyright (c)
**                            http://www.ligoo.cn/ 
** @file	  drv8802_hardware.c                                       

** @brief       1.完成Drv8802模块的数据类型的定义
                2.主要对IO口的操作，完成Drv8802模块的各种功能函数
** @copyright	    Ligoo Inc.         
** @date		    2017-03-29.
** @author      韩 慧      
*******************************************************************************/ 
#ifndef __DRV8802_HARDWARE_H__
#define __DRV8802_HARDWARE_H__

#include "stdint.h"
#include "gpio_interface.h"



/************************************************
  * @struct   Mlock8802_Platform
  * @brief    直流电机驱动器Drv8802所具有的功能属性
 ***********************************************/
typedef struct _Mlock8802_Platform{
    const void *ctrl_mphase;
    const void *ctrl_menbl;
    const void *ctrl_mdecay;
    const void *ctrl_msleep;
    const void *ctrl_mrest;
    const void *in_mfault;
    const struct gpio_operations *gpio_ops;
}Mlock8802_Platform;

#ifndef __FAR
#define __FAR
#endif
/***********************************************************************
  * @brief           设置 Drv8802相位模式 
  * @param[in]       Drv8802 属性结构体  
                     Type  1：OUT1 high, OUT2 low
                           0：OUT2 high, OUT1 low
  * @return          无
***********************************************************************/
void Mlock_Drv8802_Phase_Set(const Mlock8802_Platform *__far platform,uint8_t type);

/***********************************************************************
  * @brief           设置 Drv8802衰减模式
  * @param[in]       Drv8802 属性结构体  
                     Type  0：慢速  1：快速
  * @return          无
***********************************************************************/
void Mlock_Drv8802_Decay_Set(const Mlock8802_Platform *__far platform,uint8_t type);

/***********************************************************************
  * @brief           使能/失能 Drv8802进入复位模式
  * @param[in]       Drv8802 属性结构体  
                     Type  0：使能  1：失能
  * @return          无
***********************************************************************/
void Mlock_Drv8802_Reset_Set(const Mlock8802_Platform *__far platform,uint8_t type);

/***********************************************************************
  * @brief           使能/失能 Drv8802
  * @param[in]       Drv8802 属性结构体  
                     Type  0：使能  1：失能
  * @return          无
***********************************************************************/
void Mlock_Drv8802_Enable_Set(const Mlock8802_Platform *__far platform,uint8_t type);

/***********************************************************************
  * @brief           使能/失能 Drv8802进入休眠模式
  * @param[in]       Drv8802 属性结构体  
                     Type  0：使能  1：失能
  * @return          无
***********************************************************************/
void Mlock_Drv8802_Sleep_Set(const Mlock8802_Platform *__far platform,uint8_t type);

/***********************************************************************
  * @brief           读取 Drv8802负载状态
  * @param[in]       Drv8802 属性结构体  
  * @return          0:erro   1:nomal
***********************************************************************/
uint8_t Mlock_Drv8802_Fault_Get(const Mlock8802_Platform *__far platform);

extern const Mlock8802_Platform Mlock8802_v121;

#endif