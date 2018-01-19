/*******************************************************************************
**                       ������������Դ���޹�˾ Copyright (c)
**                            http://www.ligoo.cn/ 
** @file	 �b   etc_intermediate.h                                       

** @brief       1.���etcģ��ӿں�������
                
                
** @copyright		Ligoo Inc.         
** @date		    2017-04-11.
** @author            
*******************************************************************************/ 


#ifndef __ECT_INTERMEDIATE_H_
#define __ECT_INTERMEDIATE_H_

#include "ect_hardware.h"

extern const  EctContext count_ect_context;



#pragma push
#pragma DATA_SEG __RPAGE_SEG SHARED_DATA
extern CaptureSignal cp_pwm_xgate_data;
extern CaptureSignal st1_pwm_xgate_data;
extern CaptureSignal st3_pwm_xgate_data;
extern CaptureSignal st5_pwm_xgate_data;
extern CaptureSignal st7_pwm_xgate_data;
#pragma DATA_SEG DEFAULT
#pragma pop

/***********************************************************************
  * @brief           ��ʼ���̵���ճ�����벶׽ͨ��
  * @param[in]       ��
  * @return          1: ok   0:err
***********************************************************************/
char st_pwm_start(void);

/***********************************************************************
  * @brief           ��ʼ���������׮���Ӽ�����벶׽
  * @param[in]       ��
  * @return          1: ok   0:err
***********************************************************************/
char cp_pwm_start(void);

/***********************************************************************
  * @brief           ��ȡ��������źŵ�ռ�ձ�
  * @param[in]       ��
  * @return          ռ�ձȴ�С
***********************************************************************/
unsigned short cp_pwm_get_duty(void);

/***********************************************************************
  * @brief           ��ȡ��������ź�Ƶ��
  * @param[in]       ��
  * @return          Ƶ�ʴ�С
***********************************************************************/
unsigned short cp_pwm_get_frequence(void);

/***********************************************************************
  * @brief           �жϳ�������ź��Ƿ�Ϊ��
  * @param[in]       ��
  * @return          1 ��
***********************************************************************/
unsigned char cp_input_is_high(void);

/***********************************************************************
  * @brief           ��ȡ�̵���ճ������ռ�ձ�
  * @param[in]       pin  ͨ����
  * @return          ռ�ձ�
***********************************************************************/
unsigned short st_pwm_get_duty(unsigned char pin) ;



#endif
