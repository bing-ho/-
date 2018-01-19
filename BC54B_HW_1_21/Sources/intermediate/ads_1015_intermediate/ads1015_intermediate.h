/*******************************************************************************
**                       安徽力高新能源有限公司 Copyright (c)
**                            http://www.ligoo.cn/ 
** @file	    ads1015_intermediate.h                                       

** @brief       1.完成ads1015模块操作接口函数处理
             
                
** @copyright		Ligoo Inc.         
** @date		    2017-04-26.
** @author            
*******************************************************************************/ 
#ifndef __ADS1015_INTERMEDIATE_H__
#define __ADS1015_INTERMEDIATE_H__

#include "ads1015_hardware.h"

/// 转换错误时返回值.
#define ADS1015_RESULT_ERROR 32767

/// \name ads1015 ads1015驱动相关
/// @{



/************************************************
  * @enum     ADS1015_Channel
  * @brief    ads1015通道号
 ***********************************************/
typedef enum {
    ADS1015_CHANNEL_0 = 4,
    ADS1015_CHANNEL_1 = 5,
    ADS1015_CHANNEL_2 = 6,
    ADS1015_CHANNEL_3 = 7,
} ADS1015_Channel;

/************************************************
  * @enum     ADS1015_FullScaleVoltage
  * @brief    ads1015满量程电压
 ***********************************************/
typedef enum {
    ADS1015_FULL_SCALE_6144_MV = 0,
    ADS1015_FULL_SCALE_4096_MV = 1,
    ADS1015_FULL_SCALE_2048_MV = 2,
    ADS1015_FULL_SCALE_1024_MV = 3,
    ADS1015_FULL_SCALE_512_MV = 4,
    ADS1015_FULL_SCALE_256_MV = 5,
} ADS1015_FullScaleVoltage;

extern const ADS1015_BspInterface g_ADS1015_BspInterface;


 /************************************************************************
  * @brief           ads1015初始化
  * @param[in]       iface ADS1015_BspInterface型指针变量 
  * @return          无  
***********************************************************************/
void ADS1015_Init(const ADS1015_BspInterface *iface);   


 /************************************************************************
  * @brief           ads1015单次采样
  * @param[in]       iface        ADS1015_BspInterface型指针变量 
  * @param[in]       channel      ADS1015_Channel型变量 
  * @param[in]       fullScaleVol ADS1015_FullScaleVoltage型变量 
  * @return          采样值 
***********************************************************************/
signed short ADS1015_SingleConvert(const ADS1015_BspInterface *iface, ADS1015_Channel channel, ADS1015_FullScaleVoltage fullScaleVol);


/************************************************************************
  * @brief           ads1015采样结果值转换为mv
  * @param[in]       iface               ADS1015_BspInterface型指针变量
  * @param[in]       fullScaleVol        ADS1015_FullScaleVoltage型变量
  * @return          转换值  
***********************************************************************/ 
signed short ADS1015_ResultToVoltageMv(signed short result, ADS1015_FullScaleVoltage fullScaleVol);

 /************************************************************************
  * @brief           ads1015连续采样设置
  * @param[in]       iface        ADS1015_BspInterface型指针变量 
  * @param[in]       channel      ADS1015_Channel型变量 
  * @param[in]       fullScaleVol ADS1015_FullScaleVoltage型变量 
  * @return          1：设置成功  
***********************************************************************/
signed short  ADS1015_ContinuousConvert(const ADS1015_BspInterface *iface, ADS1015_Channel channel, ADS1015_FullScaleVoltage fullScaleVol);

 /************************************************************************
  * @brief           获取ads1015连续采样值
  * @param[in]       iface        ADS1015_BspInterface型指针变量
  * @return          采样值  
***********************************************************************/ 
signed short ADS1015_Read_ContinuousResult (const ADS1015_BspInterface *iface);

/************************************************************************
  * @brief           ads1015采样结果值转换为mv
  * @param[in]       iface               ADS1015_BspInterface型指针变量
  * @param[in]       fullScaleVol        ADS1015_FullScaleVoltage型变量
  * @return          转换值  
***********************************************************************/ 
signed short ADS1015_ResultToVoltageMv(signed short result, ADS1015_FullScaleVoltage fullScaleVol);

#endif  /*__ADS1015_INTERMEDIATE_H__*/
