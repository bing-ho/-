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
#include "drv8802_hardware.h"
#include "gpio_hardware.h"
#include "stddef.h"
#include "derivative.h"
#include "gpio_interface.h"


const  struct hcs12_gpio ctrl_mphase_io_v121 = {&PORTA,     &PORTA,     &DDRA,      1 << 3};
const  struct hcs12_gpio ctrl_menable_io_v121= {&PORTA,     &PORTA,     &DDRA,      1 << 4};
const  struct hcs12_gpio ctrl_mdecay_io_v121 = {&PORTA,     &PORTA,     &DDRA,      1 << 5};
const  struct hcs12_gpio ctrl_msleep_io_v121 = {&PORTA,     &PORTA,     &DDRA,      1 << 7};
const  struct hcs12_gpio ctrl_mreset_io_v121 = {&PORTD,     &PORTD,     &DDRD,      1 << 4};    
const  struct hcs12_gpio in_mfault_io_v121   = {&PORTA,     &PORTA,     &DDRA,      1 << 6};

const Mlock8802_Platform Mlock8802_v121 =
{
   &ctrl_mphase_io_v121,
   &ctrl_menable_io_v121,
   &ctrl_mdecay_io_v121,
   &ctrl_msleep_io_v121,
   &ctrl_mreset_io_v121,
   &in_mfault_io_v121,
   &hcs12_gpio_ops
};
/***********************************************************************
  * @brief           设置 Drv8802相位模式 
  * @param[in]       Drv8802 属性结构体  
                     Type  1：OUT1 high, OUT2 low
                           0：OUT2 high, OUT1 low
  * @return          无
***********************************************************************/
void Mlock_Drv8802_Phase_Set(const Mlock8802_Platform *__far platform,uint8_t type)
{
    if(platform == NULL)
        return;
    (void)gpio_set_output(platform->gpio_ops, platform->ctrl_mphase, type); 
}

/***********************************************************************
  * @brief           设置 Drv8802衰减模式
  * @param[in]       Drv8802 属性结构体  
                     Type  0：慢速  1：快速
  * @return          无
***********************************************************************/
void Mlock_Drv8802_Decay_Set(const Mlock8802_Platform *__far platform,uint8_t type)
{ 
    if(platform == NULL)
        return;
    (void)gpio_set_output(platform->gpio_ops, platform->ctrl_mdecay, type); 
}

/***********************************************************************
  * @brief           使能/失能 Drv8802进入复位模式
  * @param[in]       Drv8802 属性结构体  
                     Type  0：使能  1：失能
  * @return          无
***********************************************************************/
void Mlock_Drv8802_Reset_Set(const Mlock8802_Platform *__far platform,uint8_t type)
{
    if(platform == NULL)
        return;
    (void)gpio_set_output(platform->gpio_ops, platform->ctrl_mrest, type); 
}

/***********************************************************************
  * @brief           使能/失能 Drv8802
  * @param[in]       Drv8802 属性结构体  
                     Type  0：使能  1：失能
  * @return          无
***********************************************************************/
void Mlock_Drv8802_Enable_Set(const Mlock8802_Platform *__far platform,uint8_t type)
{
    if(platform == NULL)
        return;
    (void)gpio_set_output(platform->gpio_ops, platform->ctrl_menbl, type); 
}

/***********************************************************************
  * @brief           使能/失能 Drv8802进入休眠模式
  * @param[in]       Drv8802 属性结构体  
                     Type  0：使能  1：失能
  * @return          无
***********************************************************************/
void Mlock_Drv8802_Sleep_Set(const Mlock8802_Platform *__far platform,uint8_t type)
{
    if(platform == NULL)
        return;
    (void)gpio_set_output(platform->gpio_ops, platform->ctrl_msleep, type);
}

/***********************************************************************
  * @brief           读取 Drv8802负载状态
  * @param[in]       Drv8802 属性结构体  
  * @return          0:erro   1:nomal
***********************************************************************/
uint8_t Mlock_Drv8802_Fault_Get(const Mlock8802_Platform *__far platform)
{
    return gpio_input_is_high(platform->gpio_ops, platform->in_mfault); 
}




