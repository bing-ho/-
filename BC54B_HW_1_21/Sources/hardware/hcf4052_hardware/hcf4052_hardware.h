/*******************************************************************************
**                       ������������Դ���޹�˾ Copyright (c)
**                            http://www.ligoo.cn/ 
** @file	 �b   hcf4052_hardware.h                                       

** @brief       1.���hcf4052ģ����������͵Ķ��� 
                2.���hcf4052ģ��ĳ�ʼ��
               
** @copyright		Ligoo Inc.         
** @date		    2017-03-22.
** @author           
*******************************************************************************/ 

#ifndef __HCF4052_HARDWARE_H__
#define __HCF4052_HARDWARE_H__

#include "gpio_interface.h"


/************************************************
  * @struct   hcf4052_platform
  * @brief    hcf4052ģ��Ӳ��������������
 ***********************************************/
struct hcf4052_platform {
    void *ctrl_a;
    void *ctrl_b;
    void *ctrl_inh;
    const struct gpio_operations *gpio_ops;
};




/************************************************
  * @extern   insu_gain_ctrl_1_14
  * @brief    ���� insu_gain_ctrl_1_14
  ***********************************************/
extern const struct hcs12_gpio_detail insu_gain_ctrl_1_14[];

/************************************************
  * @extern   insu_gain_ctrl_1_16
  * @brief    ���� insu_gain_ctrl_1_16
  ***********************************************/
extern const struct hcs12_gpio_detail insu_gain_ctrl_1_16[];


/************************************************
  * @extern   hcf4052_impl
  * @brief    ���� hcf4052_impl
  ***********************************************/
extern const struct hcf4052_platform *hcf4052_impl;



/************************************************************************
  * @brief           hcf4052ģ���ʼ��
  * @param[in]       hcf4052_platform��hcf4052��������  
  * @return          ��
***********************************************************************/
void hcf4052_init(void) ;




#endif