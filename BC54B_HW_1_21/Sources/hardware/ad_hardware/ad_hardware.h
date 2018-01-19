/*******************************************************************************
**                       ������������Դ���޹�˾ Copyright (c)
**                            http://www.ligoo.cn/ 
** @file	 �b   ad_hardware.h                                       

** @brief       1.���ADCģ����������͵Ķ��� 
                2.���ADCģ��ĳ�ʼ��
                3.���ADCģ��ĸ��ֲɼ�ģʽ�Ĺ��ܺ���
** @copyright		Ligoo Inc.         
** @date		    2017-03-22.
** @author      ������      
*******************************************************************************/ 

#ifndef __AD_HARDWARE_H__
#define __AD_HARDWARE_H__

#include "Types.h"

#ifndef __FAR
#define __FAR
#endif

#define 	AD_ERROR       0x01
#define 	AD_OK          0x00

#define AD0_ADDR  ((byte*)0x02C0)          //ad�Ĵ�����ʼ��ַ
#define AD1_ADDR  ((byte*)0x0080) 
#define ATD_CTL0  (*(BASE_ADDR + 0x00))
/*     bit7      bit6      bit5    bit4    bit3          bit2        bit1     bit0
                                           WRAP3         WRAP2       WRAP1    WRAP0
    0  ����     ����       ����    ����      
    1  ����     ����       ����    ����    ����λ���ڶ�ͨ�����   
 */
 
#define ATD_CTL1  (*(BASE_ADDR + 0x01))
/*     bit7        bit6      bit5    bit4          bit3          bit2        bit1     bit0
                   SRES1     SRES2                 ETRIG3        ETRIG2      ETRIG1   ETRIG0
    0  ������    bit[6:5]adת��    ���ŵ�      
    1  ��������    ����              ����ǰ�ŵ�        ����λ����ѡ���ⲿ������ʽ
      
 */
 
#define ATD_CTL2  (*(BASE_ADDR + 0x02))
/*     bit7     bit6             bit5          bit4        bit3       bit2        bit1               bit0
                                               ETRIGLE   ETRIGP     
    0  ����     д1�����־λ    ��ֹ          bit[4:3]�ⲿ��ƽ       ��ֹ        ��ֹ               ��ֹ
    1  ����     ���������־λ   ʹ���ڲ�ʱ��   ������ʽѡ��          ʹ���ⲿ    ת�������ж�����   �Ƚ��ж�����
      
 */
 
#define ATD_CTL3  (*(BASE_ADDR + 0x03))
/*     bit7     bit6        bit5        bit4      bit3       bit2        bit1         bit0
                S8C         S4C         S2C       S1C     
    0  �����     bit[6:3]4λ���ת�����б��泤��           ���Ƚ��ȳ�    bit[1:0]
    1  �Ҷ���                                               �Ƚ��ȳ�      ��̨���Կ���λ
      
 */
 
#define ATD_CTL4  (*(BASE_ADDR + 0x04))
/*     bit7     bit6        bit5        bit4      bit3       bit2        bit1         bit0
       SMP2     SMP1        SMP0           
    0  bit[7:5]3λ��ɲ���ʱ��          bit[4:0]���adʱ�ӷ�Ƶ����λ
    1  ѡ��Ϊ                                              
      
 */
 
#define ATD_CTL5  (*(BASE_ADDR + 0x05))
/*     bit7      bit6             bit5      bit4      bit3   bit2     bit1     bit0
                                                      CD     CC       CB       CA                  
    0  ����     �ر�����ͨ��ת��  ����    ��ͨ��    bit[3:0]���adͨ��ѡ��λ
    1  ����     �س�ͨ��ת��      ����    ��ͨ��                                     
      
 */
//struct hcs12_adc_regs {               //adcӲ���Ĵ���
//    volatile ATD1CTL0 ATD_CTL0;
//    volatile ATD1CTL1 ATD_CTL1;
 //   volatile ATD1CTL2 ATD_CTL2;
 //   volatile ATD1CTL3 ATD_CTL3;
 //   volatile ATD1CTL4 ATD_CTL4;
//    volatile ATD1CTL5 ATD_CTL5;
//    
//};     

//struct hcs12_adc_platform
// {
//    struct hcs12_adc_regs *regs;
    //uint8_t freq_div;
//};



/************************************************
  * @enum   AdcGroup
  * @brief  Adcģ��ͨ�����
  ***********************************************/
typedef enum
{
    kAdcGroup0,
    kAdcGroup1,
    kAdcGroupMaxCount
}AdcGroup;

/************************************************
  * @enum   AdcChannel
  * @brief  Adcģ��ͨ����
  ***********************************************/
typedef enum
{
    kAdcChannel0,
    kAdcChannel1,
    kAdcChannel2,
    kAdcChannel3,
    kAdcChannel4,
    kAdcChannel5,
    kAdcChannel6,
    kAdcChannel7,
    kAdcChannel8,
    kAdcChannel9,
    kAdcChannel10,
    kAdcChannel11,
    kAdcChannel12,
    kAdcChannel13,
    kAdcChannel14,
    kAdcChannel15,
    kAdcChannelMaxCount
}AdcChannel;


/************************************************
  * @enum   AdcResolution
  * @brief  Adcģ���������  8\10\12\--bit
  ***********************************************/
typedef enum
{
    kAdc8bit,  //8λ��������
    kAdc10bit, //10λ��������
    kAdc12bit  //12λ��������
}AdcResolution;


/************************************************
  * @enum   AdcSampleTime
  * @brief  Adc����ʱ������(number of clockcycles)
  ***********************************************/
typedef enum
{
    kAdc4Cycles,//4��A/Dʱ������
    kAdc6Cycles,
    kAdc8Cycles,
    kAdc10Cycles,
    kAdc12Cycles,
    kAdc16Cycles,
    kAdc20Cycles
}AdcSampleTime;


/************************************************
  * @type   Prescaler
  * @brief  ����Ƶ��Ԥ��Ƶ,���÷�Χ0-31ʵ��2-64��Ƶ
  ***********************************************/
typedef UINT8 Prescaler;


/************************************************
  * @enum   AdcStau
  * @brief  Adcģ��״̬ not completed \completed
  ***********************************************/
typedef enum
{
    kAdcError,
    kAdcNotCompleted,    //AD����δ���
    kAdcCompleted        //AD�������
}AdcStau;


/************************************************
  * @enum   AdcConversionMode
  * @brief  Adcģ�������ʽ  ����/����
  ***********************************************/
typedef enum
{
    kAdcSingle,//���β�����ʽ
    kAdcContinuous//����������ʽ
}AdcConversionMode;


/************************************************
  * @enum   AdcTriggerSource
  * @brief  Adcģ���ⲿ����Դѡ��
  ***********************************************/
typedef enum
{
    kAdcETRIG0,       // pulse width modulator channel 1       
    kAdcETRIG1,       // pulse width modulator channel 1      
    kAdcETRIG2,       // pIt 0    
    kAdcETRIG3,       // pit 1
    kAdcNoTriggerSource   //û��ʹ���ⲿ����Դ   
}AdcTriggerSource;


/************************************************
  * @enum   AdcHwTriggerSignalType
  * @brief  Adcģ���ⲿ����Դ�ź����� rising/falling/both
  ***********************************************/
typedef enum
{
    kAdcFalling,         //�½���
    kAdcRising,          //������
    kAdcLow,             //�͵�ƽ
    kAdcHigh,             //�ߵ�ƽ
    kAdcNoTriggerSignal   //û�е�ƽ�����ź�
}AdcHwTriggerSignalType;


/************************************************
  * @enum   AdcFormat
  * @brief  Adcģ����ɼ����ݶ��뷽ʽ
  ***********************************************/
typedef enum
{
    kAdcLeft,//����뷽ʽ
    kAdcRight//�Ҷ��뷽ʽ
}AdcFormat;


/************************************************
  * @type   AdcResolutionType
  * @brief  Adcģ��ת�������������
  ***********************************************/
typedef unsigned short AdcResolutionType;


/************************************************
  * @enum   AdcChannelMode
  * @brief  Adcģ��ͨ��������ʽ  ��ͨ��/��ͨ��
  ***********************************************/
typedef enum
{
    kAdcSingleChannel,//��ͨ������ģʽ
    kAdcMultiChannel//��ͨ������ģʽ
}AdcChannelMode;


/************************************************
  * @struct   AdcParam
  * @brief    Adcģ��Ӳ����ʼ����������
 ***********************************************/
typedef  struct
{
    AdcGroup  dev; //�豸���
    AdcChannel  channel; //��ʼͨ�����
    AdcResolution  resolution; //��������
    AdcTriggerSource  trigger_source;//�ⲿ����Դѡ��
    AdcHwTriggerSignalType  trigger_signal_type;//�ⲿ����Դ�ź�����
    AdcFormat  format;//�������ݶ��뷽ʽ
    AdcSampleTime  sample_time; //����ʱ�����ڸ���
    Prescaler prescaler;//����Ƶ��Ԥ��Ƶ�����÷�Χ0-31��ʵ��2-64��Ƶ
    AdcConversionMode conversion_mode;//������ʽ
    AdcChannelMode channel_mode;//��ͨ���������ͨ������
}AdcParam;






/************************************************************************
  * @brief           Adcģ��Ӳ����ʼ������ 
  * @param[in]       AdcParam Adcģ����������
  * @return          1��AD_ERROR   0��AD_OK
***********************************************************************/
char Adc_HardwareInit(const AdcParam *__FAR param);

 
/************************************************************************
  * @brief           Adcģ������ת��
  * @param[in]       dev ͨ����� 
  * @param[in]       channel ͨ���� 
  * @return          1��AD_ERROR   0��AD_OK
***********************************************************************/
char Adc_StartConversion(AdcGroup dev, AdcChannel channel);

/************************************************************************
  * @brief           Adcģ��ֹͣת��
  * @param[in]       dev ͨ����� 
  * @return          1��AD_ERROR   0��AD_OK
***********************************************************************/
char Adc_StopConversion(AdcGroup dev);

/************************************************************************
  * @brief           ����Adcģ���ж�ʹ��/ʧ��
  * @param[in]       dev ͨ����� 
  * @param[in]       set 1��ʹ�� 0��ʧ��  
  * @return          1��AD_ERROR   0��AD_OK
***********************************************************************/
char Adc_SetInterrupt(AdcGroup dev, UINT8 set);

/************************************************************************
  * @brief           ��ȡAdcģ�鵥ͨ��ģʽ�������
  * @param[in]       dev ͨ�����   
  * @return          ת�����
***********************************************************************/
AdcResolutionType Adc_ReadSingleChannel(AdcGroup dev);

/************************************************************************
  * @brief           ������ȡAdcģ��ĳ��ͨ���ŵĲ������
  * @param[in]       dev ͨ����� 
  * @param[in]       channel ͨ���� 
  * @return          ת�����
***********************************************************************/
AdcResolutionType Adc_ReadChannelOnce(AdcGroup dev, AdcChannel channel);

/************************************************************************
  * @brief           ��ȡAdcģ���ͨ��ģʽ�µĲ������
  * @param[in]       dev ͨ����� 
  * @param[in]       data ��ȡ���� 
  * @return          1��AD_ERROR   0��AD_OK
***********************************************************************/
char Adc_ReadMultiChannel(AdcGroup dev, AdcResolutionType* data);

/************************************************************************
  * @brief           ��ȡAdcģ���ͨ��ģʽ�µ�ĳ��ͨ���Ĳ������
  * @param[in]       dev ͨ����� 
  * @param[in]       channel ͨ���� 
  * @return          ת�����
***********************************************************************/
AdcResolutionType Adc_ReadOneOfMultiChannel(AdcGroup dev, AdcChannel channel);

/************************************************************************
  * @brief           ����Adcģ���ⲿ����ʹ��/ʧ��
  * @param[in]       dev ͨ����� 
  * @param[in]       set 1��ʹ�� 0��ʧ��  
  * @return          1��AD_ERROR   0��AD_OK
***********************************************************************/
char Adc_SetTrigger(AdcGroup dev, UINT8 set);

/************************************************************************
  * @brief           ��ȡAdcģ��״̬
  * @param[in]       dev ͨ�����  
  * @return          ״̬���
***********************************************************************/
AdcStau  Adc_GetStatus(AdcGroup dev);

/************************************************************************
  * @brief           ��ȡAdcģ���жϺ���
  * @param[in]       dev ͨ�����  
  * @return          ��
***********************************************************************/
void  Adc_ISR(AdcGroup dev);



#endif
