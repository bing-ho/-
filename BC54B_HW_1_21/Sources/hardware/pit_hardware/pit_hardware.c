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

#include "pit_hardware.h"
#include "MC9S12XEP100.h"
#include "stddef.h"
 
#define 	PIT_ERROR       0x01
#define 	PIT_OK          0x00



/*
********************************************************************************
**�������ƣ�Pit_MicroTimer_Init
**��    �ܣ�PIT΢��ʱ������ֵ
**��    ����PitContext�ṹ��ָ�����
**�� �� ֵ����
********************************************************************************
*/ 
void Pit_MicroTimer_Init(PitContext*  pit_context) 
{
  if(pit_context->mt_dev== kPitMtDev0)
        {
            PITCFLMT_PFLMT0 =  0 ;              //�ر�΢��ʱ��0
            PITMTLD0 = pit_context->TimerBase;  //��΢��ʱ�����ֵ
            PITCFLMT_PFLMT0 =  1 ;
        } 
  else {
            PITCFLMT_PFLMT1 =  0 ;              //�ر�΢��ʱ��0
            PITMTLD1 = pit_context->TimerBase;  //��΢��ʱ�����ֵ
            PITCFLMT_PFLMT1 =  1 ;
       }
  
}

/*
********************************************************************************
**�������ƣ�Pit_Hardware_Init
**��    �ܣ�PITӲ����ʼ��
**��    ����PitContext�ṹ��ָ�����
**�� �� ֵ��void
********************************************************************************
*/ 
char Pit_Hardware_Init(PitContext*  pit_context)
{
    if(pit_context->mt_dev >= kPitMtDevMaxCount)    return PIT_ERROR;
    if(pit_context->dev >= kPitDevMaxCount)    return PIT_ERROR;
    if(pit_context->mt_dev== kPitMtDev0)
        {
            switch(pit_context->dev)
            {
                case kPitDev0:
                	PITLD0 = pit_context->Timer; //�趨ʱ��Timer 0��ֵ
                    break;
                case kPitDev1:
                	PITLD1 = pit_context->Timer; //�趨ʱ��Timer 1��ֵ
                    break;
                case kPitDev2:
                	PITLD2 = pit_context->Timer; //�趨ʱ��Timer 2��ֵ
                    break;
                case kPitDev3:
                	PITLD3 = pit_context->Timer; //�趨ʱ��Timer 3��ֵ
                    break;
                case kPitDev4:
                	PITLD4 = pit_context->Timer; //�趨ʱ��Timer 4��ֵ
                    break;
                case kPitDev5:
                	PITLD5 = pit_context->Timer; //�趨ʱ��Timer 5��ֵ
                    break;
                case kPitDev6:
                	PITLD6 = pit_context->Timer; //�趨ʱ��Timer 6��ֵ
                    break;
                case kPitDev7:
                	PITLD7 = pit_context->Timer; //�趨ʱ��Timer 7��ֵ
                    break;
            }
            PITCE &=~(1<< pit_context->dev); //�رն�ʱ��ͨ��0
            PITMUX &=~(1<< pit_context->dev);//��ʱ��ʹ��΢ʱ���׼0
            PITINTE &=~(1<< pit_context->dev); //�رն�ʱ��������ж�
    	}
    if(pit_context->mt_dev== kPitMtDev1)
        {
            switch(pit_context->dev)
            {
                case  kPitDev0:
                	PITLD0 = pit_context->Timer; //�趨ʱ��Timer 0��ֵ
                    break;
                case  kPitDev1:
                	PITLD1 = pit_context->Timer; //�趨ʱ��Timer1��ֵ
                    break;
                case  kPitDev2:
                	PITLD2 = pit_context->Timer; //�趨ʱ��Timer 2��ֵ
                    break;
                case  kPitDev3:
                  	PITLD3 = pit_context->Timer; //�趨ʱ��Timer 3��ֵ
                    break;
                case  kPitDev4:
                	PITLD4 = pit_context->Timer; //�趨ʱ��Timer 4��ֵ
                    break;
                case  kPitDev5:
                	PITLD5 = pit_context->Timer; //�趨ʱ��Timer 5��ֵ
                    break;
                case  kPitDev6:
                	PITLD6 = pit_context->Timer; //�趨ʱ��Timer 6��ֵ
                    break;
                case  kPitDev7:
                	PITLD7 = pit_context->Timer; //�趨ʱ��Timer7��ֵ
                    break;
            }
            PITCE &=~(1<< pit_context->dev); //�رն�ʱ��ͨ��0
            PITMUX |= (1<< pit_context->dev); //��ʱ��ʹ�û�׼1
            PITINTE &=~(1<< pit_context->dev); //�رն�ʱ������ж�
    	}
    	return PIT_OK;
}

/*
********************************************************************************
**�������ƣ�Pit_Hardware_Start
**��    �ܣ�����16λ��ʱ��
**��    ����PitDev������ѡ���豸��;
**�� �� ֵ��void
********************************************************************************
*/ 
void  Pit_Hardware_Start(PitDev dev)
{
    PITCFLMT_PITE = 1;   //ʹ��PIT
    PITCE |= (1<<dev);//������ʱ��
}

/*
********************************************************************************
**�������ƣ�Pit_Hardware_Stop
**��    �ܣ��ر�16λ��ʱ��
**��    ����PitDev������ѡ���豸��;
**�� �� ֵ��void
********************************************************************************
*/
void  Pit_Hardware_Stop(PitDev dev)
{
    PITCE &= ~(1<<dev);//�رն�ʱ��
}

/*
********************************************************************************
**�������ƣ�Pit_Hardware_SetInterrupt
**��    �ܣ�ʹ��/ʧ�� PIT�ж�
**��    ����PitDev������ѡ���豸��;
            set    1��ʹ�ܣ� 0��ʧ��
**�� �� ֵ��void
********************************************************************************
*/
void  Pit_Hardware_SetInterrupt(PitDev dev,INT8U set)
{
    if(set == 1)
        PITINTE |=(1<<dev); //������ʱ���ж�
    if(set == 0)
        PITINTE &=~(1<<dev); //�رն�ʱ���ж�
} 

/*
********************************************************************************
**�������ƣ�Pit_Hardware_Clear_Time_out_Flag
**��    �ܣ���������־λ
**��    ����PitDev������ѡ���豸��;
**�� �� ֵ��void
********************************************************************************
*/
void Pit_Hardware_Clear_Time_out_Flag(PitDev dev)
{
    PITTF = 1<< dev; //��������־λ Ĭ��Ϊ0
}

/*
********************************************************************************
**�������ƣ�Pit_Hardware_down_counter
**��    �ܣ�����Ӧ��ʮ��λ��ʱ�����ص�������ģʽ
**��    ����PitDev������ѡ���豸��;
**�� �� ֵ��void
********************************************************************************
*/
void Pit_Hardware_down_counter(PitDev dev,INT8U set)
{
    if(set == 1)
        PITFLT = 1<< dev;//��1Ϊ��������ģʽ��Ĭ��Ϊ0
    if(set == 0)
        PITFLT = 0<< dev;//Ĭ��Ϊ0
}


char Pit_Hardware_Reload(PitContext*  pit_context)
{
    if(pit_context->mt_dev >= kPitMtDevMaxCount)    return PIT_ERROR;
    if(pit_context->dev >= kPitDevMaxCount)    return PIT_ERROR;
    if(pit_context->mt_dev== kPitMtDev0)
        {
            switch(pit_context->dev)
            {
                case kPitDev0:
                	PITLD0 = pit_context->Timer; //�趨ʱ��Timer 0��ֵ
                    break;
                case kPitDev1:
                	PITLD1 = pit_context->Timer; //�趨ʱ��Timer 1��ֵ
                    break;
                case kPitDev2:
                	PITLD2 = pit_context->Timer; //�趨ʱ��Timer 2��ֵ
                    break;
                case kPitDev3:
                	PITLD3 = pit_context->Timer; //�趨ʱ��Timer 3��ֵ
                    break;
                case kPitDev4:
                	PITLD4 = pit_context->Timer; //�趨ʱ��Timer 4��ֵ
                    break;
                case kPitDev5:
                	PITLD5 = pit_context->Timer; //�趨ʱ��Timer 5��ֵ
                    break;
                case kPitDev6:
                	PITLD6 = pit_context->Timer; //�趨ʱ��Timer 6��ֵ
                    break;
                case kPitDev7:
                	PITLD7 = pit_context->Timer; //�趨ʱ��Timer 7��ֵ
                    break;
            }
    	}
    if(pit_context->mt_dev== kPitMtDev1)
        {
            switch(pit_context->dev)
            {
                case  kPitDev0:
                	PITLD0 = pit_context->Timer; //�趨ʱ��Timer 0��ֵ
                    break;
                case  kPitDev1:
                	PITLD1 = pit_context->Timer; //�趨ʱ��Timer1��ֵ
                    break;
                case  kPitDev2:
                	PITLD2 = pit_context->Timer; //�趨ʱ��Timer 2��ֵ
                    break;
                case  kPitDev3:
                  	PITLD3 = pit_context->Timer; //�趨ʱ��Timer 3��ֵ
                    break;
                case  kPitDev4:
                	PITLD4 = pit_context->Timer; //�趨ʱ��Timer 4��ֵ
                    break;
                case  kPitDev5:
                	PITLD5 = pit_context->Timer; //�趨ʱ��Timer 5��ֵ
                    break;
                case  kPitDev6:
                	PITLD6 = pit_context->Timer; //�趨ʱ��Timer 6��ֵ
                    break;
                case  kPitDev7:
                	PITLD7 = pit_context->Timer; //�趨ʱ��Timer7��ֵ
                    break;
            }
    	}
    	return PIT_OK;
}

void Pit_Hardware_isr(PitDev dev) 
{
    PITCE &= ~(1<<dev);//�رն�ʱ��
    PITTF = 1<< dev; //��������־λ Ĭ��Ϊ0
    // ��ʼ�ɼ�ͨ��channel
   // Pit_Hardware_Reload(&pit_param);
    PITCE |= (1<<dev);//������ʱ��   
}