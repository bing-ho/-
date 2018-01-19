/*******************************************************************************
**                       安徽力高新能源有限公司 Copyright (c)
**                            http://www.ligoo.cn/ 
** @file	    hcf4052_intermediate.h

** @brief       1.完成hcf4052模块放大倍数选通接口函数处理
             
                
** @copyright		Ligoo Inc.         
** @date		    2017-04-11.
** @author            
*******************************************************************************/ 



#ifndef __HCF4052_INTERMEDIATE_H__
#define __HCF4052_INTERMEDIATE_H__

#include "hcf4052_hardware.h"

#ifndef __FAR
#define __FAR
#endif



/************************************************
  * @enum     hcf4052_channel
  * @brief    hcf4052通道枚举类型
  ***********************************************/
enum hcf4052_channel {
    HCF4052_CHANNEL_0,
    HCF4052_CHANNEL_1,
    HCF4052_CHANNEL_2,
    HCF4052_CHANNEL_3,
    HCF4052_CHANNEL_NONE,
};




/***********************************************************************
  * @brief           hcf4052初始化
  * @param[in]       无 
  * @return          无
***********************************************************************/
void hcf4052_impl_init(void);

/***********************************************************************
  * @brief           hcf4052通道切换
  * @param[in]       platform    hcf4052_platform型结构体指针变量  
  * @param[in]       channel     hcf4052_channel枚举类型 
  * @return          无
***********************************************************************/
void hcf4052_select_channel(const struct hcf4052_platform *__FAR platform, enum hcf4052_channel channel) ;

/***********************************************************************
  * @brief           获取当前hcf4052选通通道
  * @param[in]       platform    hcf4052_platform型结构体指针变量
  * @return          hcf4052_channel枚举类型 
***********************************************************************/
enum hcf4052_channel hcf4052_get_current_channel(const struct hcf4052_platform *__FAR platform);

#endif   /*__HCF4052_INTERMEDIATE_H__*/
