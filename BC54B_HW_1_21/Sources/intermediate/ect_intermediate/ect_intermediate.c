/*******************************************************************************
**                       ������������Դ���޹�˾ Copyright (c)
**                            http://www.ligoo.cn/ 
** @file	 �b   etc_intermediate.c                                       

** @brief       1.���etcģ��ӿں�������
                
                
** @copyright		Ligoo Inc.         
** @date		    2017-04-11.
** @author            
*******************************************************************************/ 
 
#include "derivative.h"
#include "ect_intermediate.h"
#include "ect_xgate.h"


#pragma push
#pragma DATA_SEG __RPAGE_SEG SHARED_DATA
 CaptureSignal cp_pwm_xgate_data = {
    0, 0, 0, 0,  (1 << 2), &ECT_TC2
};

 CaptureSignal st1_pwm_xgate_data = {
    0, 0, 0, 0,  (1 << 4), &ECT_TC4
};

 CaptureSignal st3_pwm_xgate_data = {
    0, 0, 0, 0,  (1 << 5), &ECT_TC5
};

 CaptureSignal st5_pwm_xgate_data = {
    0, 0, 0, 0,  (1 << 6), &ECT_TC6
};

 CaptureSignal st7_pwm_xgate_data = {
    0, 0, 0, 0,  (1 << 7), &ECT_TC7
};
#pragma DATA_SEG DEFAULT
#pragma pop

const  EctContext st1_ect_context = {
   kEctChannel4,          //ͨ��
   kEctCapture,           //����ģʽ
   kEctDiv4,              //���ɶ�ʱ����Ƶ
   kCapBoth,              //��׽�ķ�ʽ
   kOutCaptureDisable,  
   0,
   0,
                  
};

const  EctContext st3_ect_context = {
   kEctChannel5,          //ͨ��
   kEctCapture,           //����ģʽ
   kEctDiv4,              //���ɶ�ʱ����Ƶ
   kCapBoth,              //��׽�ķ�ʽ
   kOutCaptureDisable,  
   0,
   0,
                  
};

const  EctContext st5_ect_context = {
   kEctChannel6,          //ͨ��
   kEctCapture,           //����ģʽ
   kEctDiv4,              //���ɶ�ʱ����Ƶ
   kCapBoth,              //��׽�ķ�ʽ
   kOutCaptureDisable,  
   0,
   0,
                  
};

const  EctContext st7_ect_context = {
   kEctChannel7,          //ͨ��
   kEctCapture,           //����ģʽ
   kEctDiv4,              //���ɶ�ʱ����Ƶ
   kCapBoth,              //��׽�ķ�ʽ
   kOutCaptureDisable,  
   0,
   0,
                  
};

const  EctContext cp_ect_context = {
   kEctChannel2,          //ͨ��
   kEctCapture,           //����ģʽ
   kEctDiv4,              //���ɶ�ʱ����Ƶ
   kCapBoth,              //��׽�ķ�ʽ
   kOutCaptureDisable,  
   0,
   0,
                  
};

const  EctContext count_ect_context = {
   kEctChannelCount,          //ͨ��
   0,                         //����ģʽ
   kEctDiv4,                  //���ɶ�ʱ����Ƶ
   0,                         //��׽�ķ�ʽ
   0,  
   0,
   0,
                  
};





/***********************************************************************
  * @brief           ��ʼ���̵���ճ�����벶׽ͨ��
  * @param[in]       ��
  * @return          1: ok   0:err
***********************************************************************/
char st_pwm_start(void) {
 
  
  if( Ect_Capture_OutCompare_Init(&st1_ect_context,1,1) != 1)return 0;
  if( Ect_Capture_OutCompare_Init(&st3_ect_context,1,1) != 1)return 0;
  if( Ect_Capture_OutCompare_Init(&st5_ect_context,1,1) != 1)return 0;
  if( Ect_Capture_OutCompare_Init(&st7_ect_context,1,1) != 1)return 0;
  
  return 1;


}

/***********************************************************************
  * @brief           ��ʼ���������׮���Ӽ�����벶׽
  * @param[in]       ��
  * @return          1: ok   0:err
***********************************************************************/
char cp_pwm_start(void) {

   if( Ect_Capture_OutCompare_Init(&cp_ect_context,1,1) != 1)return 0;
  
  return 1;
}

/***********************************************************************
  * @brief           ��ȡ�̵���ճ������ռ�ձ�
  * @param[in]       pin  ͨ����
  * @return          ռ�ձ�
***********************************************************************/
unsigned short st_pwm_get_duty(unsigned char pin) {
    if (pin > 12 || pin < 9) return 0;

    switch (pin) {
    case 9:
        return pwm_get_duty(&st1_pwm_xgate_data);
    case 10:
        return pwm_get_duty(&st3_pwm_xgate_data);
    case 11:
        return pwm_get_duty(&st5_pwm_xgate_data);
    case 12:
        return pwm_get_duty(&st7_pwm_xgate_data);
    default:
        return pwm_get_duty(&st1_pwm_xgate_data);
    }
}

/***********************************************************************
  * @brief           ��ȡ��������źŵ�ռ�ձ�
  * @param[in]       ��
  * @return          ռ�ձȴ�С
***********************************************************************/
unsigned short cp_pwm_get_duty(void) {
    // ���ص͵�ƽ��ռ�ձ�, Ӳ�����з���
    return 1000 - pwm_get_duty(&cp_pwm_xgate_data);
}

/***********************************************************************
  * @brief           ��ȡ��������ź�Ƶ��
  * @param[in]       ��
  * @return          Ƶ�ʴ�С
***********************************************************************/
unsigned short cp_pwm_get_frequence(void) {
    unsigned long f = pwm_get_frequence(&cp_pwm_xgate_data);
    if (f > 65535) return 65535;
    return (unsigned short)f;
}

/***********************************************************************
  * @brief           �жϳ�������ź��Ƿ�Ϊ��
  * @param[in]       ��
  * @return          1 ��
***********************************************************************/
unsigned char cp_input_is_high(void) {
    return PTT_PTT2 == 0;
}

/***********************************************************************
  * @brief          ect�жϴ���
  * @param[in]       ��
  * @return          ��
***********************************************************************/
void PwmEctISR_Handler(void)
{
    //ECT_TFLG1_C2F = 1; // Clear interrupt
    
   // pwm_ect_isr_hook();
}

