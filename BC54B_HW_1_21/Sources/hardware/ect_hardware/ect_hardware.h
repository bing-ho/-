/*******************************************************************************
**                       ������������Դ���޹�˾ Copyright (c)
**                            http://www.ligoo.cn/ 
** @file	 �b   ect_hardware.h                                       

** @brief       1.���ECTģ��ײ���������͵Ķ��� 
                2.���ECTģ��ײ�ĳ�ʼ��
                3.���ECTģ��ײ㲶׽�ȵĹ��ܺ���
** @copyright		Ligoo Inc.         
** @date		    2017-03-30.
** @author            
*******************************************************************************/ 

#ifndef __ECT_HARDWARE_H__
#define __ECT_HARDWARE_H__


#include "os_cpu.h"
#include "types.h"

#include "stdint.h"  

#ifndef __FAR
#define __FAR
#endif

/************************************************
  * @enum   EctChannel
  * @brief  ����������ʱ��ͨ����0-7 �����ͨ����8
 ***********************************************/
typedef enum
{
    kEctChannel0=0,
    kEctChannel1,
    kEctChannel2,
    kEctChannel3,
    kEctChannel4,
    kEctChannel5,
    kEctChannel6,
    kEctChannel7,
    kEctChannelCount
}EctChannel;

/************************************************
  * @enum   EctDiv
  * @brief  ����������Ƶֵ��8�ַ�Ƶֵ
 ***********************************************/
typedef enum
{
    kEctDiv1,
    kEctDiv2,
    kEctDiv4,
    kEctDiv8,
    kEctDiv16,
    kEctDiv32,
    kEctDiv64,
    kEctDiv128
}EctDiv;

/************************************************
  * @enum   EctCapMode
  * @brief  ����������׽�źŷ�ʽ
 ***********************************************/
typedef enum
{
    kCapDisable, //��ֹ��׽
    kCapUp,      //����׽������
    kCapDown,    //����׽�½���
    kCapBoth     //�����ء��½��ؾ���׽
}EctCapMode;

/************************************************
  * @enum   EctFunction
  * @brief  ��������ECTģ�鹤����ʽ
 ***********************************************/
typedef enum
{
    kEctCapture,       //���벶׽����
    kEctOutCompare,     //����ȽϹ���
    kEctOutCompareTimeFlip,   //����Ƚ�ռ�ձȿɵ� ��ʱ�������תģʽ
    kEctOutCompareOc7Contrul, //����Ƚ�ռ�ձȿɵ� ͨ��7��ǿ����
    kEctOutPwm         // ռ�ձȿɵ�pwm���ģʽ
}EctFunction; 

/************************************************
  * @enum   EctOutCompareMode
  * @brief  ��������ECTģ����Ϊ�Ƚ����ʱ���������
 ***********************************************/
typedef enum
{
    kOutCaptureDisable,         //�����
    kOutCaptureChange,         //��ת���
    kOutCaptureLow,           //����͵�ƽ
    kOutCaptureHigh          //����ߵ�ƽ
} EctOutCompareMode;

/************************************************
  * @struct   EctContext
  * @brief    ��������ECTģ���ʼ���ṹ��
 ***********************************************/
typedef struct  _ECTContext
{
    EctChannel  channel ;  //��ʱ��ͨ����   
    EctFunction ect_fun;  //ECT ģ�� ����ѡ�� 
    EctDiv      div;      //ECTģ���Ƶֵ 
    EctCapMode  edge;    //ECTģ�鲶׽�ź�ģʽ
    EctOutCompareMode  out_mode;    // ECT����Ƚ�ʱ�������
    unsigned int      timer;    //ECT����Ƚ�ʱ �Ƚϳ�ֵ ��pwm��� ռ�ձ�
    unsigned int      cycle;   //��pwm���ʱ ����  oc7ֵ
} EctContext;

/************************************************
  * @struct   CaptureSignal
  * @brief    �����������벶׽�Ľṹ��
 ***********************************************/
typedef struct  _CaptureSignal {
    unsigned long high_cnt;     // �ߵ�ƽʱ��
    unsigned long low_cnt;       //�͵�ƽʱ��
    unsigned long count_pre;    //����ʱ��
    unsigned short last_capture;   //�ϴβ����ź�ʱ��
    unsigned int port_t_bit;       //ͨ�����ź�
    unsigned int *ect2_tc;        //��Ӧͨ���ڼ�����
} CaptureSignal;

/************************************************
  * @struct   OutCompareSignal
  * @brief    ���������Ƚ�����Ľṹ��
 ***********************************************/
typedef struct  _OutCompareSignal {
    unsigned char  last_level;    //�ϴ������ƽ�ź�  0:�� 1����
    unsigned char  now_level;     //��ǰ�����ƽ�ź�   0:�� 1����
    unsigned int  last_timer;     //�ϴαȽϳ�ֵ
    unsigned int  next_timer;     //�´αȽϳ�ֵ
    unsigned int  port_t_bit;     //ͨ�����ź�
    unsigned int *ect2_tc;        //��Ӧͨ���ڼ�����
} OutCompareSignal; 



/************************************************
  * @enum   MccntCount
  * @brief  ģ�����������з�ʽ
 ***********************************************/
typedef enum {
  SingleCount=0,
  CycleCount,  
}MccntCount;

/************************************************
  * @enum   MccntDiv
  * @brief  ģ����������Ƶֵ
 ***********************************************/
typedef enum
{
    MccntDiv1,
    MccntDiv2,
    MccntDiv4,
    MccntDiv8,
   
}MccntDiv;


/************************************************
  * @struct   MccntConfig
  * @brief  ģ����������������
 ***********************************************/
typedef struct  _MccntConfig
{
  MccntCount    count;
  MccntDiv      div;
  unsigned int  initial_value;
  
}MccntConfig;


/***********************************************************************
  * @brief           ���ڶ�ECTģ����г�ʼ�� 
  * @param[in]       ECTContext  �ṹ��Ĳ���
  * @return          0���ɹ�  1��ʧ��
***********************************************************************/
unsigned char Ect_Tcnt_Init(const EctContext *__FAR ect_context,char to_xgate, unsigned char prio);

/***********************************************************************
  * @brief           ���ڶ�ECTģ�鲶�������Ƚ��ж����Գ�ʼ�� 
  * @param[in]       ECTContext  �ṹ��Ĳ���
  * @param[in]       to_xgate    �Ƿ�ʹ��Э�������ж�
  * @param[in]       prio        �ж����ȼ�    
  * @return          1���ɹ�  0��ʧ��
***********************************************************************/
unsigned char Ect_Capture_OutCompare_Init(const EctContext *__FAR ect_context,char to_xgate, unsigned char prio);

/***********************************************************************
  * @brief           ���ڶ�ECTģ���ݼ�������ģ����г�ʼ�� 
  * @param[in]       ECTContext  �ṹ��Ĳ���
  * @return          0���ɹ�  1��ʧ��
***********************************************************************/
void Ect_Mccnt_Init(const MccntConfig *__FAR ect_mccnt,char to_xgate, unsigned char prio);



/***********************************************************************
  * @brief           ���ڶ�ECTģ��������Ƚ�ʱ���ֵ
  * @param[in]       OutCompareSignal�ṹ��Ĳ���
                     UINT16 Timer  ��ֵֵ
  * @return          ��
***********************************************************************/
void Compare_SetTc(OutCompareSignal *__restrict pwm,INT16U Timer);


/***********************************************************************
  * @brief           ���ڶ�ȡECTģ�������벶׽ʱ�Ĳ�׽ֵ 
  * @param[in]       OutCompareSignal�ṹ��Ĳ���
  * @return          ��
***********************************************************************/
INT16U Compare_GetTc(OutCompareSignal *__restrict pwm);

#endif
