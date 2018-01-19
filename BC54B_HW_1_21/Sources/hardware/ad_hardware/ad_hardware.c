/*******************************************************************************
**                       安徽力高新能源有限公司 Copyright (c)
**                            http://www.ligoo.cn/ 
** @file	    ad_hardware.c                                       

** @brief       1.完成ADC模块的数据类型的定义 
                2.完成ADC模块的初始化
                3.完成ADC模块的各种采集模式的功能函数
** @copyright		Ligoo Inc.         
** @date		    2017-03-22.
** @author      王文贤      
*******************************************************************************/ 

#include "ad_hardware.h"
#include "adc_xgate.h"
#include "MC9S12XEP100.h"
#include "stddef.h"




// 初始化AdcParam 结构体 


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
  * @brief           Adc模块硬件初始化函数 
  * @param[in]       AdcParam Adc模块配置属性
  * @return          1：AD_ERROR   0：AD_OK
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
  ATD_CTL1 |= (param->resolution)<<5;//设置采样精度
  ATD_CTL1 |= 0x10;//设置内部电容器采样前放电
  if(param->trigger_source != kAdcNoTriggerSource)
   {
     ATD_CTL1 |= 0x80;  //使用外部触发源
     ATD_CTL1 &= 0xF0;
     ATD_CTL1 |= param->trigger_source;//设置外部触发源
   }
   else
   {
      ATD_CTL1 &= 0xF0;
      ATD_CTL1 |= param->channel;//使用ad转换信号
   }
  ATD_CTL2 = 0x00;
  ATD_CTL2 |= 0x40;         //使能内部时钟
  
  if(param->trigger_signal_type != kAdcNoTriggerSignal)
  ATD_CTL2 |= ((param->trigger_signal_type)<<3)|(1<<2);//设置外部触发源信号类型
 // if(param->trigger_signal_type != kAdcNoTriggerSignal)
 // ATD_CTL2 |= ((param->trigger_signal_type)<<3);
  
  ATD_CTL3 = 0x00;
  ATD_CTL3 |= (param->format)<<7;//设置采样数据对齐方式
  ATD_CTL3 |= 0x02;//设置遇断点完成当前转换后停止
  ATD_CTL4 = 0x00;
  ATD_CTL4 |= (param->sample_time)<<5;//设置采样时钟周期个数
  ATD_CTL4 |= param->prescaler;//设置采样频率预分频
 // ATD_CTL5 = 0x00;
  //ATD_CTL5 |= (param->conversion_mode)<<5;//设置采样方式
 // ATD_CTL5 |= (param->channel_mode)<<4;//单通道采样或多通道采样
 

  //
 // (void)Adc_SetTrigger(param->dev,1);
  (void)Adc_SetInterrupt(param->dev,1);//使能AD中断
 
 //ATD_CTL2  |= 1<<1; //使能AD中断
 
  return  AD_OK;
}

/************************************************************************
  * @brief           Adc模块启动转换
  * @param[in]       dev 通道组号 
  * @param[in]       channel 通道号 
  * @return          1：AD_ERROR   0：AD_OK
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
  ATD_CTL5 |= channel;//开始采集通道组dev通道channel

  return  AD_OK;
}

/************************************************************************
  * @brief           Adc模块停止转换
  * @param[in]       dev 通道组号 
  * @return          1：AD_ERROR   0：AD_OK
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
  ATD_CTL2 &= ~(1<<5); //停止AD采样   

  return  AD_OK;
}


/************************************************************************
  * @brief           设置Adc模块中断使能/失能
  * @param[in]       dev 通道组号 
  * @param[in]       set 1：使能 0：失能  
  * @return          1：AD_ERROR   0：AD_OK
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
  * @brief           读取Adc模块单通道模式采样结果
  * @param[in]       dev 通道组号   
  * @return          转换结果
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
  * @brief           设置Adc模块外部触发使能/失能
  * @param[in]       dev 通道组号 
  * @param[in]       set 1：使能 0：失能  
  * @return          1：AD_ERROR   0：AD_OK
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
  * @brief           立即读取Adc模块某个通道号的采样结果
  * @param[in]       dev 通道组号 
  * @param[in]       channel 通道号 
  * @return          转换结果
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
  * @brief           读取Adc模块多通道模式下的采样结果
  * @param[in]       dev 通道组号 
  * @param[in]       data 存取缓存 
  * @return          1：AD_ERROR   0：AD_OK
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
    channel = (ATD0CTL5 & 0x0F);//获取采样起始通道
    p = &ATD0DR0;     
    i = channel;
    for(num = 0;num < 16;num++)
    {
      data[i++] = *p; //保存AD采样数据
      p++; 
      if(i > 15)//环绕通道
      {
        i = 0;
      }
    }   
  }
  else
  {
    channel = (ATD1CTL5 & 0x0F);//获取采样起始通道
    p = &ATD1DR0;//方式可选
    i = channel;
    for(num = 0;num < 8;num++)
    {
      data[i++] = *p; //保存AD采样数据
      p++;
      if(i > 7)//环绕通道
      {
        i = 0;
      }
    } 
  }
  return  AD_OK;
}


/************************************************************************
  * @brief           读取Adc模块多通道模式下的某个通道的采样结果
  * @param[in]       dev 通道组号 
  * @param[in]       channel 通道号 
  * @return          转换结果
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
    sta_channel = (ATD0CTL5 & 0x0F);//获取采样起始通道
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
    sta_channel = (ATD1CTL5 & 0x0F);//获取采样起始通道
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
  * @brief           读取Adc模块状态
  * @param[in]       dev 通道组号  
  * @return          状态结果
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

 
 //中断处理在xgate协处理器中
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

















