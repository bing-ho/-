/*******************************************************************************
**                       安徽力高新能源有限公司 Copyright (c)
**                            http://www.ligoo.cn/ 
** @file	    hcf4052_intermediate.c                                       

** @brief       1.完成hcf4052模块放大倍数选通接口函数处理
             
                
** @copyright		Ligoo Inc.         
** @date		    2017-04-11.
** @author            
*******************************************************************************/ 

#include "hcf4052_intermediate.h"


/***********************************************************************
  * @brief           hcf4052初始化
  * @param[in]       无 
  * @return          无
***********************************************************************/
void hcf4052_impl_init(void)
{
    hcf4052_init();
}

/***********************************************************************
  * @brief           hcf4052通道切换
  * @param[in]       platform    hcf4052_platform型结构体指针变量  
  * @param[in]       channel     hcf4052_channel枚举类型 
  * @return          无
***********************************************************************/
void hcf4052_select_channel(const struct hcf4052_platform *__FAR platform, enum hcf4052_channel channel) 
{
    //disable
    (void)gpio_set_output(platform->gpio_ops, platform->ctrl_inh, 1);

    //switch channel
    switch (channel) {
    case HCF4052_CHANNEL_0:
        (void)gpio_set_output(platform->gpio_ops, platform->ctrl_a, 0);
        (void)gpio_set_output(platform->gpio_ops, platform->ctrl_b, 0);
        break;
    case HCF4052_CHANNEL_1:
        (void)gpio_set_output(platform->gpio_ops, platform->ctrl_a, 1);
        (void)gpio_set_output(platform->gpio_ops, platform->ctrl_b, 0);
        break;
    case HCF4052_CHANNEL_2:
        (void)gpio_set_output(platform->gpio_ops, platform->ctrl_a, 0);
        (void)gpio_set_output(platform->gpio_ops, platform->ctrl_b, 1);
        break;
    case HCF4052_CHANNEL_3:
        (void)gpio_set_output(platform->gpio_ops, platform->ctrl_a, 1);
        (void)gpio_set_output(platform->gpio_ops, platform->ctrl_b, 1);
        break;
    default:
        return;
    }

    // enable
    (void)gpio_set_output(platform->gpio_ops, platform->ctrl_inh, 0);
}

/***********************************************************************
  * @brief           获取当前hcf4052选通通道
  * @param[in]       platform    hcf4052_platform型结构体指针变量
  * @return          hcf4052_channel枚举类型 
***********************************************************************/
enum hcf4052_channel hcf4052_get_current_channel(const struct hcf4052_platform *__FAR platform) 
{
    uint8_t bits = 0;
    static const enum hcf4052_channel io_to_channel[4] = {
        HCF4052_CHANNEL_0,
        HCF4052_CHANNEL_2,
        HCF4052_CHANNEL_1,
        HCF4052_CHANNEL_3,
    };
    if (gpio_output_is_high(platform->gpio_ops, platform->ctrl_inh)) {
        return HCF4052_CHANNEL_NONE;
    }

    if (gpio_output_is_high(platform->gpio_ops, platform->ctrl_a)) {
        bits |= 0x01;
    }
    if (gpio_output_is_high(platform->gpio_ops, platform->ctrl_a)) {
        bits |= 0x02;
    }

    return io_to_channel[bits];
}

