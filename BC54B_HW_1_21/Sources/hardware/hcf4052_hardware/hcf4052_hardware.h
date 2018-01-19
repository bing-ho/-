/*******************************************************************************
**                       安徽力高新能源有限公司 Copyright (c)
**                            http://www.ligoo.cn/ 
** @file	    hcf4052_hardware.h                                       

** @brief       1.完成hcf4052模块的数据类型的定义 
                2.完成hcf4052模块的初始化
               
** @copyright		Ligoo Inc.         
** @date		    2017-03-22.
** @author           
*******************************************************************************/ 

#ifndef __HCF4052_HARDWARE_H__
#define __HCF4052_HARDWARE_H__

#include "gpio_interface.h"


/************************************************
  * @struct   hcf4052_platform
  * @brief    hcf4052模块硬件操作配置属性
 ***********************************************/
struct hcf4052_platform {
    void *ctrl_a;
    void *ctrl_b;
    void *ctrl_inh;
    const struct gpio_operations *gpio_ops;
};




/************************************************
  * @extern   insu_gain_ctrl_1_14
  * @brief    声明 insu_gain_ctrl_1_14
  ***********************************************/
extern const struct hcs12_gpio_detail insu_gain_ctrl_1_14[];

/************************************************
  * @extern   insu_gain_ctrl_1_16
  * @brief    声明 insu_gain_ctrl_1_16
  ***********************************************/
extern const struct hcs12_gpio_detail insu_gain_ctrl_1_16[];


/************************************************
  * @extern   hcf4052_impl
  * @brief    声明 hcf4052_impl
  ***********************************************/
extern const struct hcf4052_platform *hcf4052_impl;



/************************************************************************
  * @brief           hcf4052模块初始化
  * @param[in]       hcf4052_platform型hcf4052操作属性  
  * @return          无
***********************************************************************/
void hcf4052_init(void) ;




#endif