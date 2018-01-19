/*******************************************************************************
**                       ������������Դ���޹�˾ Copyright (c)
**                            http://www.ligoo.cn/ 
** @file	 �b   hcf4052_intermediate.c                                       

** @brief       1.���hcf4052ģ��Ŵ���ѡͨ�ӿں�������
             
                
** @copyright		Ligoo Inc.         
** @date		    2017-04-11.
** @author            
*******************************************************************************/ 

#include "hcf4052_intermediate.h"


/***********************************************************************
  * @brief           hcf4052��ʼ��
  * @param[in]       �� 
  * @return          ��
***********************************************************************/
void hcf4052_impl_init(void)
{
    hcf4052_init();
}

/***********************************************************************
  * @brief           hcf4052ͨ���л�
  * @param[in]       platform    hcf4052_platform�ͽṹ��ָ�����  
  * @param[in]       channel     hcf4052_channelö������ 
  * @return          ��
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
  * @brief           ��ȡ��ǰhcf4052ѡͨͨ��
  * @param[in]       platform    hcf4052_platform�ͽṹ��ָ�����
  * @return          hcf4052_channelö������ 
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

