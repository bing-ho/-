/*******************************************************************************
**                       ������������Դ���޹�˾ Copyright (c)
**                            http://www.ligoo.cn/ 
** @file	 �b pit_hardware.c                                       

** @brief       1.���PITģ����������͵Ķ��� 
                2.���PITģ��ĳ�ʼ��
                3.���PITģ��ĸ��ֶ�ʱ���Ĺ��ܺ���
** @copyright		Ligoo Inc.         
** @date		    2017-03-21.
** @author      �� ��      
*******************************************************************************/
#ifndef __PIT_HARDWARE_H__
#define __PIT_HARDWARE_H__

#include "Types.h"
#include "os_cpu.h"

//typedef unsigned long  INT32U;
//typedef unsigned char  INT8U;

//΢��ʱ����ͨ����0-1�������2
typedef enum
{
    kPitMtDev0,
    kPitMtDev1,
    kPitMtDevMaxCount
}PitMtDev;


//��ʱ��ͨ����0-7 �����ͨ����8
typedef enum
{
    kPitDev0,
    kPitDev1,
    kPitDev2,
    kPitDev3,
    kPitDev4,
    kPitDev5,
    kPitDev6,
    kPitDev7,
    kPitDevMaxCount
}PitDev;


//PITģ��Ӳ����ʼ����������
typedef  struct
{
    PitMtDev mt_dev;  //΢��ʱ��ͨ����
    PitDev dev;    //��ʱ��ͨ����   
    INT8U TimerBase;  //PITģ���е�8λ��������ֵ 
    INT16U Timer;    //PITģ���е�16λ��������ֵ
}PitContext;

extern PitContext pit_param;


char Pit_Hardware_Init(PitContext* pit_context);

void Pit_MicroTimer_Init(PitContext*  pit_context) ;

void Pit_Hardware_Start(PitDev dev);
void Pit_Hardware_Stop(PitDev dev);
void Pit_Hardware_SetInterrupt(PitDev dev,INT8U set);
void Pit_Hardware_Clear_Time_out_Flag(PitDev dev);
void Pit_Hardware_down_counter(PitDev dev,INT8U set);
char Pit_Hardware_Reload(PitContext*  pit_context);
void Pit_Hardware_isr(PitDev dev);

#endif
