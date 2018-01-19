/*******************************************************************************
**                       安徽力高新能源有限公司 Copyright (c)
**                            http://www.ligoo.cn/ 
** @file	    gpio_interface.h                                       

** @brief       1.完成gpio模块的数据接口封装
                
** @copyright		Ligoo Inc.         
** @date		    2017-04-26.
** @author           
*******************************************************************************/ 
#ifndef __GPIO_INTERFACE_H__
#define __GPIO_INTERFACE_H__

#include "stdint.h"
#include "gpio_hardware.h"

#ifndef __FAR
#define __FAR
#endif

#define inline

/************************************************
  * @typedef   gpio_t
  * @brief     定义gpio_t长指针类型
 ***********************************************/
typedef void *__FAR gpio_t;

/************************************************
  * @typedef   gpio_ops_t
  * @brief     定义gpio_operations结构体长指针类型
 ***********************************************/
typedef struct gpio_operations const *__FAR gpio_ops_t;

/************************************************************************
  * @brief           gpio模块硬件初始化函数 
  * @param[in]       ops         gpio_operations 型操作函数
  * @param[in]       io          hcs12_gpio模块配置属性寄存器  
  * @param[in]       mode        gpio_mode口工作模式
  * @return          1：init ok   0：init err
***********************************************************************/
inline uint8_t gpio_init(gpio_ops_t ops, gpio_t io, enum gpio_mode mode);

/************************************************************************
  * @brief           获取gpio模块工作模式  
  * @param[in]       ops         gpio_operations 型操作函数
  * @param[in]       io          hcs12_gpio模块配置属性寄存器  
  * @return          io工作模式
***********************************************************************/
inline enum gpio_mode gpio_get_mode(gpio_ops_t ops, gpio_t io);

/************************************************************************
  * @brief           获取gpio设置输入输出类型
  * @param[in]       mode         gpio_mode io口输入输出类型
  * @return          类型字符串
***********************************************************************/
inline const char *gpio_mode_to_string(enum gpio_mode moe);

/************************************************************************
  * @brief           判断GPIO输出是否为高
  * @param[in]       ops         gpio_operations 型操作函数
  * @param[in]       io          hcs12_gpio模块配置属性寄存器  
  * @return          1:输出为高   0:输出为低
***********************************************************************/
inline uint8_t gpio_input_is_high(gpio_ops_t ops, gpio_t io);

/************************************************************************
  * @brief           设置gpio口输出
  * @param[in]       ops         gpio_operations 型操作函数
  * @param[in]       io          hcs12_gpio模块配置属性寄存器  
  * @param[in]       is_high     输出电平 1为高，0为低
  * @return          1:输出为高   0:输出为低
***********************************************************************/
inline uint8_t gpio_output_is_high(gpio_ops_t ops, gpio_t io);

/************************************************************************
  * @brief           设置gpio口输出
  * @param[in]       ops         gpio_operations 型操作函数
  * @param[in]       io          hcs12_gpio模块配置属性寄存器  
  * @param[in]       is_high     输出电平 1为高，0为低
  * @return          1:成功   0:失败
***********************************************************************/
inline uint8_t gpio_set_output(gpio_ops_t ops, gpio_t io, uint8_t is_high);

/************************************************************************
  * @brief           设置gpio口输出
  * @param[in]       mode         gpio_mode io口输入输出类型
  * @return          类型字符串
***********************************************************************/
void gpio_dump(struct gpio_operations const *__FAR ops, void *__FAR io, const char *prefix);

#endif
