/*******************************************************************************
**                       安徽力高新能源有限公司 Copyright (c)
**                            http://www.ligoo.cn/ 
** @file	    gpio_hardware.c                                       

** @brief       1.完成gpio模块的数据类型的定义 
                2.完成gpio模块的初始化
                3.完成gpio模块的io输入输出控制和io状态采集
** @copyright		Ligoo Inc.         
** @date		    2017-03-24.
** @author           
*******************************************************************************/ 
#include "gpio_hardware.h"
#include "ucos_ii.h"


 
/************************************************************************
  * @brief           gpio模块硬件初始化函数 
  * @param[in]       hcs12_gpio gpio模块配置属性寄存器  gpio_mode gpio 口工作模式
  * @return          1：init ok   0：init err
***********************************************************************/
uint8_t hcs12_gpio_init(const struct hcs12_gpio *__FAR io, enum gpio_mode mode) 
{
    OS_CPU_SR cpu_sr = 0;
    OS_ENTER_CRITICAL();
    if (GPIO_MODE_OUTPUT_PUSHPULL == mode || GPIO_MODE_OUTPUT_OPENDRAIN == mode) //gpio模式 == 推挽输出 或 开漏输出
    {
        *io->dir_reg |= io->bit;
        OS_EXIT_CRITICAL();
        return 1;
    }
    if (GPIO_MODE_INPUT == mode)   //gpio模式 == 输入模式
    {
        *io->dir_reg &= ~io->bit;
        OS_EXIT_CRITICAL();
        return 1;
    }
    OS_EXIT_CRITICAL();
    return 0;
}

 
/************************************************************************
  * @brief           获取gpio模块工作模式 
  * @param[in]       hcs12_gpio_t gpio模块配置
  * @return          io工作模式
***********************************************************************/
enum gpio_mode hcs12_gpio_get_mode(hcs12_gpio_t io) 
{
    if (*io->dir_reg & io->bit) 
    {
        return GPIO_MODE_OUTPUT_PUSHPULL; //推挽输出模式
    } 
    else 
    {
        return GPIO_MODE_INPUT; //输入模式
    }
}


/************************************************************************
  * @brief           判断GPIO输入是否为高
  * @param[in]       hcs12_gpio gpio模块寄存器
  * @return          1:输入为高  0:输入为低
***********************************************************************/
uint8_t hcs12_gpio_input_is_high(const struct hcs12_gpio *__FAR io) 
{
    if (*io->dir_reg & io->bit)  //input mode
    { 
        return 0;
    }
    return 0 != (*io->in_reg & io->bit);
}


/************************************************************************
  * @brief           判断GPIO输出是否为高
  * @param[in]       hcs12_gpio gpio模块寄存器
  * @return          1:输出为高   0:输出为低
***********************************************************************/
uint8_t hcs12_gpio_output_is_high(const struct hcs12_gpio *__FAR io) 
{
    if (*io->dir_reg & io->bit)  //output mode
    { 
        return 0 != (*io->out_reg & io->bit);
    }
    return 0;
}


/************************************************************************
  * @brief           设置gpio口输出
  * @param[in]       hcs12_gpio gpio模块寄存器      is_high--输出电平 1为高，0为低
  * @return          1:set_ok，0:set_err
***********************************************************************/
uint8_t hcs12_gpio_set_output(const struct hcs12_gpio *__FAR io, uint8_t is_high) 
{
    if (*io->dir_reg & io->bit)  //output mode
    { 
        OS_CPU_SR cpu_sr = 0;
        OS_ENTER_CRITICAL();
        if (is_high)    //输出设置为高 
        {
            *io->out_reg |= io->bit;
        } 
        else 
        {
            *io->out_reg &= ~io->bit;
        }
        OS_EXIT_CRITICAL();
        return 1;
    }
    return 0;
}


/************************************************************************
  * @brief          初始化 gpio操作接口函数结构体
  * @param[in]       
  * @return         
***********************************************************************/
const struct gpio_operations hcs12_gpio_ops = 
{
    (uint8_t (*)(void *, enum gpio_mode))hcs12_gpio_init,  //gpio口初始化()
    (enum gpio_mode (*)(void *))hcs12_gpio_get_mode,       //获取gpio口模式()
    (uint8_t (*)(void *))hcs12_gpio_input_is_high,         //pio口输入是否为高()
    (uint8_t (*)(void *))hcs12_gpio_output_is_high,        //gpio口输出是否为高()
    (uint8_t (*)(void *, uint8_t))hcs12_gpio_set_output,   //gpio口设置输出()
};

