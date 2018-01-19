/*******************************************************************************
**                       ������������Դ���޹�˾ Copyright (c)
**                            http://www.ligoo.cn/ 
** @file	 �b   ads1015_intermediate.c                                       

** @brief       1.���ads1015ģ������ӿں�������
             
                
** @copyright		Ligoo Inc.         
** @date		    2017-04-26.
** @author            
*******************************************************************************/ 


#include "ads1015_intermediate.h" 
#include "ucos_ii.h"
#include "derivative.h"

#pragma MESSAGE DISABLE C4000
#pragma MESSAGE DISABLE C1420 // Result of function-call is ignored

#include "iic_interface.h"



//ADS1015��������Ϊ:-2048 - +2048mv
#define ADS1015_RESULT_FULL 2048

/// name ads1015 ads1015�������
/// ADS 1015��һ��12λģ��ת��оƬ��ʹ��IICͨ��(SDA--�������ݴ���ͽ������� �� SCL--����ʱ������:ʱ������SDA)

#if ADS1015_CHECK != 0
#define ADS1015_INTERFACE_IS_VALID(iface) ((0 != (iface)) && ((iface)->magic == ADS1015_CHECK_MAGIC))
#define ADS1015_IS_VALID(ads1015) ((0 != (ads1015)) && ((ads1015)->magic == ADS1015_CHECK_MAGIC))
#else
#define ADS1015_INTERFACE_IS_VALID(iface) (0 != (iface))
#define ADS1015_IS_VALID(ads1015) (0 != (ads1015))
#endif

#define TestAds 0   //ֻ��Ϊ�����ɼ�����
/*
  ����ǰӦ��  g_ADS1015_BspInterface ��  ADS1015_SINGLE_MODE  ��Ϊ ADS1015_CONTINUOUS_MODE
   const ADS1015_BspInterface g_ADS1015_BspInterface = 
{
#if ADS1015_CHECK != 0
    ADS1015_CHECK_MAGIC,
#endif
    &hcs12_i2c_bus_0,
    ADS1015_1600SPS,
    ADS1015_SINGLE_MODE,
    ADS1015_Init,
    ads_write_reg,
    ads_read_reg,
    ads_delay_ms,
};

*/

#if TestAds
 #include "bms_job_impl.h"
 #include "bms_system.h"
 
 #pragma DATA_SEG __RPAGE_SEG PAGED_RAM
 uint16_t gpio_station_1015=0;
 #pragma DATA_SEG DEFAULT

void ads_test(void* pdata)
{
  
    OS_CPU_SR cpu_sr = 0;
    
    pdata = pdata;
    gpio_station_1015= ADS1015_Read_ContinuousResult(&g_ADS1015_BspInterface);
 }

void ADS1015_TestInit(void)
 {
   
   ADS1015_Init(&g_ADS1015_BspInterface);
   
   ADS1015_ContinuousConvert(&g_ADS1015_BspInterface, ADS1015_CHANNEL_2, ADS1015_FULL_SCALE_1024_MV);  //����ģʽֻ�Թ̶�ͨ�� ����
   
   job_schedule(MAIN_JOB_GROUP, 500, ads_test, NULL);
     
 }

#endif 




 /************************************************************************
  * @brief           ads1015��ʼ��
  * @param[in]       iface ADS1015_BspInterface��ָ����� 
  * @return          ��  
***********************************************************************/
void ads1015_init(const ADS1015_BspInterface *iface) {
    if (!ADS1015_INTERFACE_IS_VALID(iface)) {
        return;
    }
    #if (BMS_SUPPORT_CURRENT_DIVIDER !=1)   //Ӳ����֧�ַ�������ADS10151����ʹ��IIC0���赥����ʼ��
    iface->init(iface);
    #endif
}


 /************************************************************************
  * @brief           ads1015���β���
  * @param[in]       iface        ADS1015_BspInterface��ָ����� 
  * @param[in]       channel      ADS1015_Channel�ͱ��� 
  * @param[in]       fullScaleVol ADS1015_FullScaleVoltage�ͱ��� 
  * @return          ����ֵ 
***********************************************************************/
signed short ADS1015_SingleConvert(const ADS1015_BspInterface *iface, ADS1015_Channel channel, ADS1015_FullScaleVoltage fullScaleVol) {
    unsigned short i;
    signed short result;
    unsigned short tmp;
    if (!ADS1015_INTERFACE_IS_VALID(iface)) {
        return ADS1015_RESULT_ERROR;
    }
    // ADS1015оƬ�ֲ�
    // bit[15]  Operational status / single-shot convertion start.---����״̬/����ת����ʼ
    // bit[14:12] Input multiplexer configuration.--�����·����������
    // bit[11:9] Programmable gain.--�ɱ������
    // bit[8] Device operating mode.--�豸����ģʽ 0����ת�� 1����ת��
    // bit[7:5] Data rate.--��������
    // bit[1:0] Comparator queue and disable.--�Ƚ������к�ʹ��
    tmp = (0 << 15) |
          (((unsigned short)channel) << 12) |
          (((unsigned short)fullScaleVol) << 9) |
          (iface->ad_mode << 8) |
          (iface->ads1015rate << 5) |
         // (1<< 8) |
         // (4 << 5) |
          (3 << 0);

    if (!iface->i2c_write(iface->bus, 0x01, tmp)) {
        return ADS1015_RESULT_ERROR;
    }
    if (!iface->i2c_write(iface->bus, 0x01, tmp | (1 << 15))) {
        return ADS1015_RESULT_ERROR;
    }

    // �ȴ�ת�����.
    i = 0;
    do {
        iface->delay_ms(iface, 1);
        if (++i > 5) {
            return ADS1015_RESULT_ERROR;
        }
        if (!iface->i2c_read(iface->bus, 0x01, &tmp)) {
            return ADS1015_RESULT_ERROR;
        }
    } while ((tmp & (1 << 15)) == 0);

    if (!iface->i2c_read(iface->bus, 0x00, (unsigned short *)&result)) {
        return ADS1015_RESULT_ERROR;
    }
    return result >> 4;
}

 /************************************************************************
  * @brief           ads1015������������
  * @param[in]       iface        ADS1015_BspInterface��ָ����� 
  * @param[in]       channel      ADS1015_Channel�ͱ��� 
  * @param[in]       fullScaleVol ADS1015_FullScaleVoltage�ͱ��� 
  * @return          1�����óɹ�  
***********************************************************************/
signed short  ADS1015_ContinuousConvert(const ADS1015_BspInterface *iface, ADS1015_Channel channel, ADS1015_FullScaleVoltage fullScaleVol)
{ 
     unsigned short tmp;
    if (!ADS1015_INTERFACE_IS_VALID(iface)) //�ж�iface �ӿ��Ƿ���Ч
    {
        return ADS1015_RESULT_ERROR;  // ��Ч���ش���״̬
    }

    tmp = (0 << 15) |         //��дѡͨͨ������������Ϣ����������
          (((unsigned short)channel) << 12) |
          (((unsigned short)fullScaleVol) << 9) |
          (iface->ad_mode << 8) |
          (iface->ads1015rate << 5) |
          (3 << 0);

    if (!iface->i2c_write(iface->bus, 0x01, tmp)) {   
        return ADS1015_RESULT_ERROR;
    }
   // if (!write_reg(iface->bus, 0x01, tmp | (1 << 15))) {  // ��������ת��
  ///      return ADS1015_RESULT_ERROR;
  //  } 
  return 1;       
 }

 /************************************************************************
  * @brief           ��ȡads1015��������ֵ
  * @param[in]       iface        ADS1015_BspInterface��ָ�����
  * @return          ����ֵ  
***********************************************************************/ 
 signed short ADS1015_Read_ContinuousResult (const ADS1015_BspInterface *iface)
{ 
  signed short result;
   if (!ADS1015_INTERFACE_IS_VALID(iface)) //�ж�iface �ӿ��Ƿ���Ч
    {
        return ADS1015_RESULT_ERROR;  // ��Ч���ش���״̬
    }

   if (!iface->i2c_read(iface->bus, 0x00, (unsigned short *)&result)) 
    {
         return ADS1015_RESULT_ERROR;
    }
    return result >> 4;
          
 }



/************************************************************************
  * @brief           ads1015�������ֵת��Ϊmv
  * @param[in]       iface               ADS1015_BspInterface��ָ�����
  * @param[in]       fullScaleVol        ADS1015_FullScaleVoltage�ͱ���
  * @return          ת��ֵ  
***********************************************************************/ 
signed short ADS1015_ResultToVoltageMv(signed short result, ADS1015_FullScaleVoltage fullScaleVol) {


    signed long res;
    static const unsigned short vt[] = {6144, 4096, 2048, 1024, 512, 256};
    
    if (fullScaleVol >= sizeof(vt) / sizeof(vt[0])) return 0;  //�жϴ���Ĳ����Ƿ񳬹����鷶Χ

    if (result >= ADS1015_RESULT_FULL) return 0;  //�Բɼ��Ľ�������ж�,�Ƿ񳬷�Χ
    if (result < -ADS1015_RESULT_FULL) return 0;

    res = vt[fullScaleVol];

    res = result * res;
    if (res >= 0) {
        res += ADS1015_RESULT_FULL / 2;  //��������
    } else {
        res -= ADS1015_RESULT_FULL / 2; //��������
    }

    return (signed short)((signed long)(res / ADS1015_RESULT_FULL)); //��ʵ�ʵĵ�ѹ
}

 /************************************************************************
  * @brief           ֹͣads1015������������
  * @param[in]       iface        ADS1015_BspInterface��ָ����� 
  * @param[in]       channel      ADS1015_Channel�ͱ��� 
  * @param[in]       fullScaleVol ADS1015_FullScaleVoltage�ͱ��� 
  * @return          1�����óɹ�  
***********************************************************************/
signed short ADS1015_Convert_Stop (const ADS1015_BspInterface *iface, ADS1015_Channel channel, ADS1015_FullScaleVoltage fullScaleVol) 
{ 
      unsigned short tmp;
    if (!ADS1015_INTERFACE_IS_VALID(iface)) //�ж�iface �ӿ��Ƿ���Ч
    {
        return ADS1015_RESULT_ERROR;  // ��Ч���ش���״̬
    }
     tmp = (0 << 15) |         //��дѡͨͨ������������Ϣ����������
          (((unsigned short)channel) << 12) |
          (((unsigned short)fullScaleVol) << 9) |
          (1<< 8) |
          (iface->ads1015rate << 5) |
          (3 << 0);

    if (!iface->i2c_write(iface->bus, 0x01, tmp)) {   
        return ADS1015_RESULT_ERROR;
    }
    if (!iface->i2c_write(iface->bus, 0x01, tmp | (1 << 15))) {  
        return ADS1015_RESULT_ERROR;
    }
    return 1;
     
 }




