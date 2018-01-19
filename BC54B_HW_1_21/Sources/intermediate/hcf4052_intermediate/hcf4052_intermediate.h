/*******************************************************************************
**                       ������������Դ���޹�˾ Copyright (c)
**                            http://www.ligoo.cn/ 
** @file	 �b   hcf4052_intermediate.h

** @brief       1.���hcf4052ģ��Ŵ���ѡͨ�ӿں�������
             
                
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
  * @brief    hcf4052ͨ��ö������
  ***********************************************/
enum hcf4052_channel {
    HCF4052_CHANNEL_0,
    HCF4052_CHANNEL_1,
    HCF4052_CHANNEL_2,
    HCF4052_CHANNEL_3,
    HCF4052_CHANNEL_NONE,
};




/***********************************************************************
  * @brief           hcf4052��ʼ��
  * @param[in]       �� 
  * @return          ��
***********************************************************************/
void hcf4052_impl_init(void);

/***********************************************************************
  * @brief           hcf4052ͨ���л�
  * @param[in]       platform    hcf4052_platform�ͽṹ��ָ�����  
  * @param[in]       channel     hcf4052_channelö������ 
  * @return          ��
***********************************************************************/
void hcf4052_select_channel(const struct hcf4052_platform *__FAR platform, enum hcf4052_channel channel) ;

/***********************************************************************
  * @brief           ��ȡ��ǰhcf4052ѡͨͨ��
  * @param[in]       platform    hcf4052_platform�ͽṹ��ָ�����
  * @return          hcf4052_channelö������ 
***********************************************************************/
enum hcf4052_channel hcf4052_get_current_channel(const struct hcf4052_platform *__FAR platform);

#endif   /*__HCF4052_INTERMEDIATE_H__*/
