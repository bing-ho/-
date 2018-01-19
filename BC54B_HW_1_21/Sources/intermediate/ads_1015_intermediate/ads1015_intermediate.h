/*******************************************************************************
**                       ������������Դ���޹�˾ Copyright (c)
**                            http://www.ligoo.cn/ 
** @file	 �b   ads1015_intermediate.h                                       

** @brief       1.���ads1015ģ������ӿں�������
             
                
** @copyright		Ligoo Inc.         
** @date		    2017-04-26.
** @author            
*******************************************************************************/ 
#ifndef __ADS1015_INTERMEDIATE_H__
#define __ADS1015_INTERMEDIATE_H__

#include "ads1015_hardware.h"

/// ת������ʱ����ֵ.
#define ADS1015_RESULT_ERROR 32767

/// \name ads1015 ads1015�������
/// @{



/************************************************
  * @enum     ADS1015_Channel
  * @brief    ads1015ͨ����
 ***********************************************/
typedef enum {
    ADS1015_CHANNEL_0 = 4,
    ADS1015_CHANNEL_1 = 5,
    ADS1015_CHANNEL_2 = 6,
    ADS1015_CHANNEL_3 = 7,
} ADS1015_Channel;

/************************************************
  * @enum     ADS1015_FullScaleVoltage
  * @brief    ads1015�����̵�ѹ
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
  * @brief           ads1015��ʼ��
  * @param[in]       iface ADS1015_BspInterface��ָ����� 
  * @return          ��  
***********************************************************************/
void ADS1015_Init(const ADS1015_BspInterface *iface);   


 /************************************************************************
  * @brief           ads1015���β���
  * @param[in]       iface        ADS1015_BspInterface��ָ����� 
  * @param[in]       channel      ADS1015_Channel�ͱ��� 
  * @param[in]       fullScaleVol ADS1015_FullScaleVoltage�ͱ��� 
  * @return          ����ֵ 
***********************************************************************/
signed short ADS1015_SingleConvert(const ADS1015_BspInterface *iface, ADS1015_Channel channel, ADS1015_FullScaleVoltage fullScaleVol);


/************************************************************************
  * @brief           ads1015�������ֵת��Ϊmv
  * @param[in]       iface               ADS1015_BspInterface��ָ�����
  * @param[in]       fullScaleVol        ADS1015_FullScaleVoltage�ͱ���
  * @return          ת��ֵ  
***********************************************************************/ 
signed short ADS1015_ResultToVoltageMv(signed short result, ADS1015_FullScaleVoltage fullScaleVol);

 /************************************************************************
  * @brief           ads1015������������
  * @param[in]       iface        ADS1015_BspInterface��ָ����� 
  * @param[in]       channel      ADS1015_Channel�ͱ��� 
  * @param[in]       fullScaleVol ADS1015_FullScaleVoltage�ͱ��� 
  * @return          1�����óɹ�  
***********************************************************************/
signed short  ADS1015_ContinuousConvert(const ADS1015_BspInterface *iface, ADS1015_Channel channel, ADS1015_FullScaleVoltage fullScaleVol);

 /************************************************************************
  * @brief           ��ȡads1015��������ֵ
  * @param[in]       iface        ADS1015_BspInterface��ָ�����
  * @return          ����ֵ  
***********************************************************************/ 
signed short ADS1015_Read_ContinuousResult (const ADS1015_BspInterface *iface);

/************************************************************************
  * @brief           ads1015�������ֵת��Ϊmv
  * @param[in]       iface               ADS1015_BspInterface��ָ�����
  * @param[in]       fullScaleVol        ADS1015_FullScaleVoltage�ͱ���
  * @return          ת��ֵ  
***********************************************************************/ 
signed short ADS1015_ResultToVoltageMv(signed short result, ADS1015_FullScaleVoltage fullScaleVol);

#endif  /*__ADS1015_INTERMEDIATE_H__*/
