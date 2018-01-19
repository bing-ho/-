/*******************************************************************************
**                       ������������Դ���޹�˾ Copyright (c)
**                            http://www.ligoo.cn/ 
** @file	 �b   ad_hardware.c                                       

** @brief       1.���ADCģ����������͵Ķ��� 
                2.���ADCģ��ĳ�ʼ��
                3.���ADCģ��ĸ��ֲɼ�ģʽ�Ĺ��ܺ���
** @copyright		Ligoo Inc.         
** @date		    2017-03-22.
** @author      ������      
*******************************************************************************/ 

#include "ad_hardware.h"
#include "adc_xgate.h"
#include "MC9S12XEP100.h"
#include "stddef.h"




// ��ʼ��AdcParam �ṹ�� 


//const struct hcs12_adc_platform adc0_platform = 
//{
//    (struct hcs12_adc_regs *)0x02C0,
   // 0x67u,
//};

//const struct hcs12_adc_platform adc1_platform = 
//{
//    (struct hcs12_adc_regs *)0x0080,
   // 0x67u,
//};

#if 1
/************************************************************************
  * @brief           Adcģ��Ӳ����ʼ������ 
  * @param[in]       AdcParam Adcģ����������
  * @return          1��AD_ERROR   0��AD_OK
***********************************************************************/

char Adc_HardwareInit(const AdcParam *__FAR param)
{
  UINT8* BASE_ADDR = NULL;
  if(param == NULL)  return  AD_ERROR;
  if(param->dev >= kAdcGroupMaxCount)  return  AD_ERROR;
  
  if(param->dev == 0)
  {
    BASE_ADDR = AD0_ADDR;
  }
  else
  {
    BASE_ADDR = AD1_ADDR;
  }
  ATD_CTL1 &= 0x0F;
  ATD_CTL1 |= (param->resolution)<<5;//���ò�������
  ATD_CTL1 |= 0x10;//�����ڲ�����������ǰ�ŵ�
  if(param->trigger_source != kAdcNoTriggerSource)
   {
     ATD_CTL1 |= 0x80;  //ʹ���ⲿ����Դ
     ATD_CTL1 &= 0xF0;
     ATD_CTL1 |= param->trigger_source;//�����ⲿ����Դ
   }
   else
   {
      ATD_CTL1 &= 0xF0;
      ATD_CTL1 |= param->channel;//ʹ��adת���ź�
   }
  ATD_CTL2 = 0x00;
  ATD_CTL2 |= 0x40;         //ʹ���ڲ�ʱ��
  
  if(param->trigger_signal_type != kAdcNoTriggerSignal)
  ATD_CTL2 |= ((param->trigger_signal_type)<<3)|(1<<2);//�����ⲿ����Դ�ź�����
 // if(param->trigger_signal_type != kAdcNoTriggerSignal)
 // ATD_CTL2 |= ((param->trigger_signal_type)<<3);
  
  ATD_CTL3 = 0x00;
  ATD_CTL3 |= (param->format)<<7;//���ò������ݶ��뷽ʽ
  ATD_CTL3 |= 0x02;//�������ϵ���ɵ�ǰת����ֹͣ
  ATD_CTL4 = 0x00;
  ATD_CTL4 |= (param->sample_time)<<5;//���ò���ʱ�����ڸ���
  ATD_CTL4 |= param->prescaler;//���ò���Ƶ��Ԥ��Ƶ
 // ATD_CTL5 = 0x00;
  //ATD_CTL5 |= (param->conversion_mode)<<5;//���ò�����ʽ
 // ATD_CTL5 |= (param->channel_mode)<<4;//��ͨ���������ͨ������
 

  //
 // (void)Adc_SetTrigger(param->dev,1);
  (void)Adc_SetInterrupt(param->dev,1);//ʹ��AD�ж�
 
 //ATD_CTL2  |= 1<<1; //ʹ��AD�ж�
 
  return  AD_OK;
}

/************************************************************************
  * @brief           Adcģ������ת��
  * @param[in]       dev ͨ����� 
  * @param[in]       channel ͨ���� 
  * @return          1��AD_ERROR   0��AD_OK
***********************************************************************/
char Adc_StartConversion(AdcGroup dev, AdcChannel channel)
{
  UINT8* BASE_ADDR = NULL;
  if(dev >= kAdcGroupMaxCount)  return  AD_ERROR;
  
  if(dev == 0)
  {
    if(channel >= kAdcChannelMaxCount)  return  AD_ERROR;
    BASE_ADDR = AD0_ADDR;
  }
  else
  {
    if(channel >= kAdcChannel8)  return  AD_ERROR;
    BASE_ADDR = AD1_ADDR;
  }  
  ATD_CTL5 &= 0xF0;
  ATD_CTL5 |= channel;//��ʼ�ɼ�ͨ����devͨ��channel

  return  AD_OK;
}

/************************************************************************
  * @brief           Adcģ��ֹͣת��
  * @param[in]       dev ͨ����� 
  * @return          1��AD_ERROR   0��AD_OK
***********************************************************************/
char Adc_StopConversion (AdcGroup dev)
{
  UINT8* BASE_ADDR = NULL;
  if(dev >= kAdcGroupMaxCount)  return  AD_ERROR;
  
  if(dev == 0)
  {
    BASE_ADDR = AD0_ADDR;
  }
  else
  {
    BASE_ADDR = AD1_ADDR;
  }
  ATD_CTL2 &= ~(1<<5); //ֹͣAD����   

  return  AD_OK;
}


/************************************************************************
  * @brief           ����Adcģ���ж�ʹ��/ʧ��
  * @param[in]       dev ͨ����� 
  * @param[in]       set 1��ʹ�� 0��ʧ��  
  * @return          1��AD_ERROR   0��AD_OK
***********************************************************************/
char Adc_SetInterrupt(AdcGroup dev, UINT8 set)
{	
  UINT8* BASE_ADDR = NULL;
  if(dev >= kAdcGroupMaxCount)  return  AD_ERROR;
  if(set >= 2)  return  AD_ERROR;
  
  if(dev == 0)
  {
    BASE_ADDR = AD0_ADDR;
  }
  else
  {
    BASE_ADDR = AD1_ADDR;
  }
  if(set) 
  {
    ATD_CTL2 |= set<<1;
  }
  else  
  {
    ATD_CTL2 &= ~(set<<1);
  }
    
  return  AD_OK;
}


/************************************************************************
  * @brief           ��ȡAdcģ�鵥ͨ��ģʽ�������
  * @param[in]       dev ͨ�����   
  * @return          ת�����
***********************************************************************/
AdcResolutionType Adc_ReadSingleChannel(AdcGroup dev)
{	
  AdcResolutionType tmp;
  if(dev >= kAdcGroupMaxCount)  return  AD_ERROR;
  if (dev == 0)
  {
    tmp = ATD0DR0;  
  }
  else
  {
    tmp = ATD1DR0; 
  }
  return  tmp;
}

/************************************************************************
  * @brief           ����Adcģ���ⲿ����ʹ��/ʧ��
  * @param[in]       dev ͨ����� 
  * @param[in]       set 1��ʹ�� 0��ʧ��  
  * @return          1��AD_ERROR   0��AD_OK
***********************************************************************/
char Adc_SetTrigger(AdcGroup dev, UINT8 set)
{	
  UINT8* BASE_ADDR = NULL;
  if(dev >= kAdcGroupMaxCount)  return  AD_ERROR;
  if(set >= 2)  return  AD_ERROR;
  
  if(dev == 0)
  {
    BASE_ADDR = AD0_ADDR;
  }
  else
  {
    BASE_ADDR = AD1_ADDR;
  }
  if(set)
  {
    ATD_CTL2 |= set<<2;
  }
  else 
  {
    ATD_CTL2 &= ~(set<<2);
  }

  return  AD_OK;
}

#endif 


/************************************************************************
  * @brief           ������ȡAdcģ��ĳ��ͨ���ŵĲ������
  * @param[in]       dev ͨ����� 
  * @param[in]       channel ͨ���� 
  * @return          ת�����
***********************************************************************/
AdcResolutionType Adc_ReadChannelOnce(AdcGroup dev, AdcChannel channel)
{	
  AdcResolutionType tmp;
  if(dev >= kAdcGroupMaxCount)  return  AD_ERROR;
  if(dev == 0)
  {
    if(channel >= kAdcChannelMaxCount)  return  AD_ERROR;
    if((channel == (ATD0CTL5 & 0x0F))&&(kAdcCompleted == Adc_GetStatus(kAdcGroup0))) 
    {  
      tmp = ATD0DR0;
    } 
    else  return AD_ERROR;
  }
  else
  {
    if(channel >= kAdcChannel8)  return  AD_ERROR;
    if((channel == (ATD1CTL5 & 0x0F))&&(kAdcCompleted == Adc_GetStatus(kAdcGroup1))) 
    {  
      tmp = ATD1DR0;
    }
    else  return AD_ERROR; 
  }
  return  tmp;
}


/************************************************************************
  * @brief           ��ȡAdcģ���ͨ��ģʽ�µĲ������
  * @param[in]       dev ͨ����� 
  * @param[in]       data ��ȡ���� 
  * @return          1��AD_ERROR   0��AD_OK
***********************************************************************/
char Adc_ReadMultiChannel(AdcGroup dev, AdcResolutionType* data)
{
  char i;
  char num;
  char channel;
  word* p;
  if(dev >= kAdcGroupMaxCount)  return  AD_ERROR;
  if(data == NULL)  return  AD_ERROR;
  if(dev == 0)
  { 
    channel = (ATD0CTL5 & 0x0F);//��ȡ������ʼͨ��
    p = &ATD0DR0;     
    i = channel;
    for(num = 0;num < 16;num++)
    {
      data[i++] = *p; //����AD��������
      p++; 
      if(i > 15)//����ͨ��
      {
        i = 0;
      }
    }   
  }
  else
  {
    channel = (ATD1CTL5 & 0x0F);//��ȡ������ʼͨ��
    p = &ATD1DR0;//��ʽ��ѡ
    i = channel;
    for(num = 0;num < 8;num++)
    {
      data[i++] = *p; //����AD��������
      p++;
      if(i > 7)//����ͨ��
      {
        i = 0;
      }
    } 
  }
  return  AD_OK;
}


/************************************************************************
  * @brief           ��ȡAdcģ���ͨ��ģʽ�µ�ĳ��ͨ���Ĳ������
  * @param[in]       dev ͨ����� 
  * @param[in]       channel ͨ���� 
  * @return          ת�����
***********************************************************************/
AdcResolutionType Adc_ReadOneOfMultiChannel(AdcGroup dev, AdcChannel channel)
{
//  char i;
//  char num;
  char sta_channel;
  word* p;
  if(dev >= kAdcGroupMaxCount)  return  AD_ERROR;
  if(dev == 0)
  { 
    if(channel >= kAdcChannelMaxCount)  return  AD_ERROR;
    p = &ATD0DR0;
    sta_channel = (ATD0CTL5 & 0x0F);//��ȡ������ʼͨ��
    if(channel >= sta_channel) 
    {
    
      return  *(p+(channel-sta_channel));
    } 
    else
    {
    
      return  *(p+(kAdcChannelMaxCount-sta_channel+channel));
    }   
  }
  else
  {
    if(channel >= kAdcChannel8)  return  AD_ERROR;
    p = &ATD1DR0;
    sta_channel = (ATD1CTL5 & 0x0F);//��ȡ������ʼͨ��
    if(channel >= sta_channel) 
    {
    
      return  *(p+(channel-sta_channel));
    } 
    else
    {
    
      return  *(p+(kAdcChannel8-sta_channel+channel));
    } 
  }
}





/************************************************************************
  * @brief           ��ȡAdcģ��״̬
  * @param[in]       dev ͨ�����  
  * @return          ״̬���
***********************************************************************/
AdcStau Adc_GetStatus(AdcGroup dev)
{	
  if(dev >= kAdcGroupMaxCount)  return  kAdcError;
  if(dev == 0)
  {
    if(ATD0STAT0_SCF == 0)	return kAdcNotCompleted; 
    else return kAdcCompleted; 
  }
  else
  {
    if(ATD1STAT0_SCF == 0)	return kAdcNotCompleted; 
    else return kAdcCompleted; 
  }
}

 
 //�жϴ�����xgateЭ��������
/*

void Adc_ISR(AdcGroup dev)
{	
  AdcResolutionType tmp;
  AdcResolutionType data[16];
  void (*p_single)(unsigned short*,unsigned char,unsigned short);
  p_single = adc_buffer_put_data;
  if (dev == 0)
  {
    if(ATD0CTL5 & 0x10)
    {
          
    }
    else
    {
      
    } 
  }
  else
  {
    if(ATD1CTL5 & 0x10)
    {
      
    }
    else
    {
      
    }
  }
}

*/

















