/*******************************************************************************
**                       安徽力高新能源有限公司 Copyright (c)
**                            http://www.ligoo.cn/ 
** @file	    gpio_hardware.h                                       

** @brief       1.完成gpio模块的数据类型的定义 
                2.完成gpio模块的初始化
                3.完成gpio模块的io输入输出控制和io状态采集
** @copyright		Ligoo Inc.         
** @date		    2017-03-24.
** @author           
*******************************************************************************/ 

#ifndef __GPIO_HARDWARE_H__
#define __GPIO_HARDWARE_H__

#include "stdint.h" 
#include "derivative.h" 

#ifndef __FAR
#define __FAR
#endif

/************************************************
  * @struct   hcs12_gpio
  * @brief    芯片IO口配置属性寄存器
 ***********************************************/

struct hcs12_gpio {
    volatile uint8_t *in_reg;            //输入寄存器
    volatile uint8_t *out_reg;           //输出寄存器
    volatile uint8_t *dir_reg;           //方向寄存器
    volatile uint8_t bit;                //位
};
/************************************************
  * @struct   hcs12_gpio_detail
  * @brief   芯片GPIO口初始化属性
 ***********************************************/
struct hcs12_gpio_detail {
    const char *name;                  //名字
    struct hcs12_gpio io;              //io口配置
    //uint8_t def_val;                 //默认值
    //uint8_t revert;                  //复位值
    
    uint8_t def_val:1;                 //默认值
    uint8_t revert:1;                  //复位值
};
  
/************************************************
  * @struct   gpio_operations
  * @brief    gpio模块操作函数集合
 ***********************************************/
struct gpio_operations {
    uint8_t (*init)(void *__FAR io, enum gpio_mode mode);      //gpio口初始化()
    enum gpio_mode (*get_mode)(void *__FAR io);                //gpio模式()
    uint8_t (*input_is_high)(void *__FAR io);                  //gpio输入是否为高()
    uint8_t (*output_is_high)(void *__FAR io);                 //gpio输出是否为高()
    uint8_t (*set_output)(void *__FAR io, uint8_t is_high);    //设置输出()
};
/************************************************
  * @enum   gpio_mode
  * @brief  gpio口工作模式
  ***********************************************/
enum gpio_mode {
    GPIO_MODE_INPUT,                 //输入模式
    GPIO_MODE_OUTPUT_PUSHPULL,       //推挽输出
    GPIO_MODE_OUTPUT_OPENDRAIN,      //开漏输出
};

/************************************************
  * @extern   hcs12_gpio_ops
  * @brief    声明 hcs12_gpio_ops
  ***********************************************/
extern const struct gpio_operations hcs12_gpio_ops;




/************************************************
  * @typedef   hcs12_gpio_t
  * @brief  定义hcs12_gpio类型hcs12_gpio_t
  ***********************************************/
typedef const struct hcs12_gpio *__FAR hcs12_gpio_t;

/************************************************************************
  * @brief           gpio模块硬件初始化函数 
  * @param[in]       hcs12_gpio gpio模块配置属性寄存器  gpio_mode gpio 口工作模式
  * @return          1：init ok   0：init err
***********************************************************************/
uint8_t hcs12_gpio_init(hcs12_gpio_t io, enum gpio_mode mode);

/************************************************************************
  * @brief           获取gpio模块工作模式 
  * @param[in]       hcs12_gpio_t gpio模块配置
  * @return          io工作模式
***********************************************************************/
enum gpio_mode hcs12_gpio_get_mode(hcs12_gpio_t io);

/************************************************************************
  * @brief           判断GPIO输入是否为高
  * @param[in]       hcs12_gpio gpio模块寄存器
  * @return          1:输入为高  0:输入为低
***********************************************************************/
uint8_t hcs12_gpio_input_is_high(hcs12_gpio_t io);

/************************************************************************
  * @brief           判断GPIO输出是否为高
  * @param[in]       hcs12_gpio gpio模块寄存器
  * @return          1:输出为高   0:输出为低
***********************************************************************/
uint8_t hcs12_gpio_output_is_high(hcs12_gpio_t io);

/************************************************************************
  * @brief           设置gpio口输出
  * @param[in]       hcs12_gpio gpio模块寄存器      is_high--输出电平 1为高，0为低
  * @return          1:set_ok，0:set_err
***********************************************************************/
uint8_t hcs12_gpio_set_output(hcs12_gpio_t io, uint8_t is_high);






#endif 