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
#ifndef __DRV8802_HARDWARE_H__
#define __DRV8802_HARDWARE_H__

#include "stdint.h"
#include "gpio_interface.h"



/************************************************
  * @struct   Mlock8802_Platform
  * @brief    ֱ�����������Drv8802�����еĹ�������
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
  * @brief           ���� Drv8802��λģʽ 
  * @param[in]       Drv8802 ���Խṹ��  
                     Type  1��OUT1 high, OUT2 low
                           0��OUT2 high, OUT1 low
  * @return          ��
***********************************************************************/
void Mlock_Drv8802_Phase_Set(const Mlock8802_Platform *__far platform,uint8_t type);

/***********************************************************************
  * @brief           ���� Drv8802˥��ģʽ
  * @param[in]       Drv8802 ���Խṹ��  
                     Type  0������  1������
  * @return          ��
***********************************************************************/
void Mlock_Drv8802_Decay_Set(const Mlock8802_Platform *__far platform,uint8_t type);

/***********************************************************************
  * @brief           ʹ��/ʧ�� Drv8802���븴λģʽ
  * @param[in]       Drv8802 ���Խṹ��  
                     Type  0��ʹ��  1��ʧ��
  * @return          ��
***********************************************************************/
void Mlock_Drv8802_Reset_Set(const Mlock8802_Platform *__far platform,uint8_t type);

/***********************************************************************
  * @brief           ʹ��/ʧ�� Drv8802
  * @param[in]       Drv8802 ���Խṹ��  
                     Type  0��ʹ��  1��ʧ��
  * @return          ��
***********************************************************************/
void Mlock_Drv8802_Enable_Set(const Mlock8802_Platform *__far platform,uint8_t type);

/***********************************************************************
  * @brief           ʹ��/ʧ�� Drv8802��������ģʽ
  * @param[in]       Drv8802 ���Խṹ��  
                     Type  0��ʹ��  1��ʧ��
  * @return          ��
***********************************************************************/
void Mlock_Drv8802_Sleep_Set(const Mlock8802_Platform *__far platform,uint8_t type);

/***********************************************************************
  * @brief           ��ȡ Drv8802����״̬
  * @param[in]       Drv8802 ���Խṹ��  
  * @return          0:erro   1:nomal
***********************************************************************/
uint8_t Mlock_Drv8802_Fault_Get(const Mlock8802_Platform *__far platform);

extern const Mlock8802_Platform Mlock8802_v121;

#endif