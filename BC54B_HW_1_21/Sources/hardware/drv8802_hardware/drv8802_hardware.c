/*******************************************************************************
**                       ������������Դ���޹�˾ Copyright (c)
**                            http://www.ligoo.cn/ 
** @file	 �b drv8802_hardware.c                                       

** @brief       1.���Drv8802ģ����������͵Ķ���
                2.��Ҫ��IO�ڵĲ��������Drv8802ģ��ĸ��ֹ��ܺ���
** @copyright	    Ligoo Inc.         
** @date		    2017-03-29.
** @author      �� ��      
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
  * @brief           ���� Drv8802��λģʽ 
  * @param[in]       Drv8802 ���Խṹ��  
                     Type  1��OUT1 high, OUT2 low
                           0��OUT2 high, OUT1 low
  * @return          ��
***********************************************************************/
void Mlock_Drv8802_Phase_Set(const Mlock8802_Platform *__far platform,uint8_t type)
{
    if(platform == NULL)
        return;
    (void)gpio_set_output(platform->gpio_ops, platform->ctrl_mphase, type); 
}

/***********************************************************************
  * @brief           ���� Drv8802˥��ģʽ
  * @param[in]       Drv8802 ���Խṹ��  
                     Type  0������  1������
  * @return          ��
***********************************************************************/
void Mlock_Drv8802_Decay_Set(const Mlock8802_Platform *__far platform,uint8_t type)
{ 
    if(platform == NULL)
        return;
    (void)gpio_set_output(platform->gpio_ops, platform->ctrl_mdecay, type); 
}

/***********************************************************************
  * @brief           ʹ��/ʧ�� Drv8802���븴λģʽ
  * @param[in]       Drv8802 ���Խṹ��  
                     Type  0��ʹ��  1��ʧ��
  * @return          ��
***********************************************************************/
void Mlock_Drv8802_Reset_Set(const Mlock8802_Platform *__far platform,uint8_t type)
{
    if(platform == NULL)
        return;
    (void)gpio_set_output(platform->gpio_ops, platform->ctrl_mrest, type); 
}

/***********************************************************************
  * @brief           ʹ��/ʧ�� Drv8802
  * @param[in]       Drv8802 ���Խṹ��  
                     Type  0��ʹ��  1��ʧ��
  * @return          ��
***********************************************************************/
void Mlock_Drv8802_Enable_Set(const Mlock8802_Platform *__far platform,uint8_t type)
{
    if(platform == NULL)
        return;
    (void)gpio_set_output(platform->gpio_ops, platform->ctrl_menbl, type); 
}

/***********************************************************************
  * @brief           ʹ��/ʧ�� Drv8802��������ģʽ
  * @param[in]       Drv8802 ���Խṹ��  
                     Type  0��ʹ��  1��ʧ��
  * @return          ��
***********************************************************************/
void Mlock_Drv8802_Sleep_Set(const Mlock8802_Platform *__far platform,uint8_t type)
{
    if(platform == NULL)
        return;
    (void)gpio_set_output(platform->gpio_ops, platform->ctrl_msleep, type);
}

/***********************************************************************
  * @brief           ��ȡ Drv8802����״̬
  * @param[in]       Drv8802 ���Խṹ��  
  * @return          0:erro   1:nomal
***********************************************************************/
uint8_t Mlock_Drv8802_Fault_Get(const Mlock8802_Platform *__far platform)
{
    return gpio_input_is_high(platform->gpio_ops, platform->in_mfault); 
}




