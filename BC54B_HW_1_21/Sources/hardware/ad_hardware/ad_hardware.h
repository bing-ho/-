/*******************************************************************************
**                       安徽力高新能源有限公司 Copyright (c)
**                            http://www.ligoo.cn/ 
** @file	    ad_hardware.h                                       

** @brief       1.完成ADC模块的数据类型的定义 
                2.完成ADC模块的初始化
                3.完成ADC模块的各种采集模式的功能函数
** @copyright		Ligoo Inc.         
** @date		    2017-03-22.
** @author      王文贤      
*******************************************************************************/ 

#ifndef __AD_HARDWARE_H__
#define __AD_HARDWARE_H__

#include "Types.h"

#ifndef __FAR
#define __FAR
#endif

#define 	AD_ERROR       0x01
#define 	AD_OK          0x00

#define AD0_ADDR  ((byte*)0x02C0)          //ad寄存器起始地址
#define AD1_ADDR  ((byte*)0x0080) 
#define ATD_CTL0  (*(BASE_ADDR + 0x00))
/*     bit7      bit6      bit5    bit4    bit3          bit2        bit1     bit0
                                           WRAP3         WRAP2       WRAP1    WRAP0
    0  保留     保留       保留    保留      
    1  保留     保留       保留    保留    后四位用于多通道检测   
 */
 
#define ATD_CTL1  (*(BASE_ADDR + 0x01))
/*     bit7        bit6      bit5    bit4          bit3          bit2        bit1     bit0
                   SRES1     SRES2                 ETRIG3        ETRIG2      ETRIG1   ETRIG0
    0  自身触发    bit[6:5]ad转换    不放电      
    1  其他触发    精度              采样前放电        后四位用于选择外部触发方式
      
 */
 
#define ATD_CTL2  (*(BASE_ADDR + 0x02))
/*     bit7     bit6             bit5          bit4        bit3       bit2        bit1               bit0
                                               ETRIGLE   ETRIGP     
    0  保留     写1清除标志位    禁止          bit[4:3]外部电平       禁止        禁止               禁止
    1  保留     快速清除标志位   使能内部时钟   触发方式选择          使能外部    转换结束中断势能   比较中断势能
      
 */
 
#define ATD_CTL3  (*(BASE_ADDR + 0x03))
/*     bit7     bit6        bit5        bit4      bit3       bit2        bit1         bit0
                S8C         S4C         S2C       S1C     
    0  左对齐     bit[6:3]4位组成转换序列保存长度           非先进先出    bit[1:0]
    1  右对齐                                               先进先出      后台调试控制位
      
 */
 
#define ATD_CTL4  (*(BASE_ADDR + 0x04))
/*     bit7     bit6        bit5        bit4      bit3       bit2        bit1         bit0
       SMP2     SMP1        SMP0           
    0  bit[7:5]3位组成采样时钟          bit[4:0]组成ad时钟分频控制位
    1  选择为                                              
      
 */
 
#define ATD_CTL5  (*(BASE_ADDR + 0x05))
/*     bit7      bit6             bit5      bit4      bit3   bit2     bit1     bit0
                                                      CD     CC       CB       CA                  
    0  保留     关闭特殊通道转换  单次    单通道    bit[3:0]组成ad通道选择位
    1  保留     特出通道转换      连续    多通道                                     
      
 */
//struct hcs12_adc_regs {               //adc硬件寄存器
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
  * @brief  Adc模块通道组号
  ***********************************************/
typedef enum
{
    kAdcGroup0,
    kAdcGroup1,
    kAdcGroupMaxCount
}AdcGroup;

/************************************************
  * @enum   AdcChannel
  * @brief  Adc模块通道号
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
  * @brief  Adc模块采样精度  8\10\12\--bit
  ***********************************************/
typedef enum
{
    kAdc8bit,  //8位采样精度
    kAdc10bit, //10位采样精度
    kAdc12bit  //12位采样精度
}AdcResolution;


/************************************************
  * @enum   AdcSampleTime
  * @brief  Adc采样时钟周期(number of clockcycles)
  ***********************************************/
typedef enum
{
    kAdc4Cycles,//4个A/D时钟周期
    kAdc6Cycles,
    kAdc8Cycles,
    kAdc10Cycles,
    kAdc12Cycles,
    kAdc16Cycles,
    kAdc20Cycles
}AdcSampleTime;


/************************************************
  * @type   Prescaler
  * @brief  采样频率预分频,设置范围0-31实现2-64分频
  ***********************************************/
typedef UINT8 Prescaler;


/************************************************
  * @enum   AdcStau
  * @brief  Adc模块状态 not completed \completed
  ***********************************************/
typedef enum
{
    kAdcError,
    kAdcNotCompleted,    //AD采样未完成
    kAdcCompleted        //AD采样完成
}AdcStau;


/************************************************
  * @enum   AdcConversionMode
  * @brief  Adc模块采样方式  单次/连续
  ***********************************************/
typedef enum
{
    kAdcSingle,//单次采样方式
    kAdcContinuous//连续采样方式
}AdcConversionMode;


/************************************************
  * @enum   AdcTriggerSource
  * @brief  Adc模块外部触发源选择
  ***********************************************/
typedef enum
{
    kAdcETRIG0,       // pulse width modulator channel 1       
    kAdcETRIG1,       // pulse width modulator channel 1      
    kAdcETRIG2,       // pIt 0    
    kAdcETRIG3,       // pit 1
    kAdcNoTriggerSource   //没有使用外部触发源   
}AdcTriggerSource;


/************************************************
  * @enum   AdcHwTriggerSignalType
  * @brief  Adc模块外部触发源信号类型 rising/falling/both
  ***********************************************/
typedef enum
{
    kAdcFalling,         //下降沿
    kAdcRising,          //上升沿
    kAdcLow,             //低电平
    kAdcHigh,             //高电平
    kAdcNoTriggerSignal   //没有电平触发信号
}AdcHwTriggerSignalType;


/************************************************
  * @enum   AdcFormat
  * @brief  Adc模块外采集数据对齐方式
  ***********************************************/
typedef enum
{
    kAdcLeft,//左对齐方式
    kAdcRight//右对齐方式
}AdcFormat;


/************************************************
  * @type   AdcResolutionType
  * @brief  Adc模块转换结果数据类型
  ***********************************************/
typedef unsigned short AdcResolutionType;


/************************************************
  * @enum   AdcChannelMode
  * @brief  Adc模块通道采样方式  单通道/多通道
  ***********************************************/
typedef enum
{
    kAdcSingleChannel,//单通道采样模式
    kAdcMultiChannel//多通道采样模式
}AdcChannelMode;


/************************************************
  * @struct   AdcParam
  * @brief    Adc模块硬件初始化配置属性
 ***********************************************/
typedef  struct
{
    AdcGroup  dev; //设备编号
    AdcChannel  channel; //起始通道编号
    AdcResolution  resolution; //采样精度
    AdcTriggerSource  trigger_source;//外部触发源选择
    AdcHwTriggerSignalType  trigger_signal_type;//外部触发源信号类型
    AdcFormat  format;//采样数据对齐方式
    AdcSampleTime  sample_time; //采样时钟周期个数
    Prescaler prescaler;//采样频率预分频，设置范围0-31，实现2-64分频
    AdcConversionMode conversion_mode;//采样方式
    AdcChannelMode channel_mode;//单通道采样或多通道采样
}AdcParam;






/************************************************************************
  * @brief           Adc模块硬件初始化函数 
  * @param[in]       AdcParam Adc模块配置属性
  * @return          1：AD_ERROR   0：AD_OK
***********************************************************************/
char Adc_HardwareInit(const AdcParam *__FAR param);

 
/************************************************************************
  * @brief           Adc模块启动转换
  * @param[in]       dev 通道组号 
  * @param[in]       channel 通道号 
  * @return          1：AD_ERROR   0：AD_OK
***********************************************************************/
char Adc_StartConversion(AdcGroup dev, AdcChannel channel);

/************************************************************************
  * @brief           Adc模块停止转换
  * @param[in]       dev 通道组号 
  * @return          1：AD_ERROR   0：AD_OK
***********************************************************************/
char Adc_StopConversion(AdcGroup dev);

/************************************************************************
  * @brief           设置Adc模块中断使能/失能
  * @param[in]       dev 通道组号 
  * @param[in]       set 1：使能 0：失能  
  * @return          1：AD_ERROR   0：AD_OK
***********************************************************************/
char Adc_SetInterrupt(AdcGroup dev, UINT8 set);

/************************************************************************
  * @brief           读取Adc模块单通道模式采样结果
  * @param[in]       dev 通道组号   
  * @return          转换结果
***********************************************************************/
AdcResolutionType Adc_ReadSingleChannel(AdcGroup dev);

/************************************************************************
  * @brief           立即读取Adc模块某个通道号的采样结果
  * @param[in]       dev 通道组号 
  * @param[in]       channel 通道号 
  * @return          转换结果
***********************************************************************/
AdcResolutionType Adc_ReadChannelOnce(AdcGroup dev, AdcChannel channel);

/************************************************************************
  * @brief           读取Adc模块多通道模式下的采样结果
  * @param[in]       dev 通道组号 
  * @param[in]       data 存取缓存 
  * @return          1：AD_ERROR   0：AD_OK
***********************************************************************/
char Adc_ReadMultiChannel(AdcGroup dev, AdcResolutionType* data);

/************************************************************************
  * @brief           读取Adc模块多通道模式下的某个通道的采样结果
  * @param[in]       dev 通道组号 
  * @param[in]       channel 通道号 
  * @return          转换结果
***********************************************************************/
AdcResolutionType Adc_ReadOneOfMultiChannel(AdcGroup dev, AdcChannel channel);

/************************************************************************
  * @brief           设置Adc模块外部触发使能/失能
  * @param[in]       dev 通道组号 
  * @param[in]       set 1：使能 0：失能  
  * @return          1：AD_ERROR   0：AD_OK
***********************************************************************/
char Adc_SetTrigger(AdcGroup dev, UINT8 set);

/************************************************************************
  * @brief           读取Adc模块状态
  * @param[in]       dev 通道组号  
  * @return          状态结果
***********************************************************************/
AdcStau  Adc_GetStatus(AdcGroup dev);

/************************************************************************
  * @brief           读取Adc模块中断函数
  * @param[in]       dev 通道组号  
  * @return          无
***********************************************************************/
void  Adc_ISR(AdcGroup dev);



#endif
