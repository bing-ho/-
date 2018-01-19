/*******************************************************************************
**                       安徽力高新能源有限公司 Copyright (c)
**                            http://www.ligoo.cn/ 
** @file	    ads1015_intermediate.c                                       

** @brief       1.完成ads1015模块操作接口函数处理
             
                
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



//ADS1015的满量程为:-2048 - +2048mv
#define ADS1015_RESULT_FULL 2048

/// name ads1015 ads1015驱动相关
/// ADS 1015是一个12位模数转换芯片，使用IIC通信(SDA--串行数据传输和接收数据 和 SCL--串行时钟输入:时钟数据SDA)

#if ADS1015_CHECK != 0
#define ADS1015_INTERFACE_IS_VALID(iface) ((0 != (iface)) && ((iface)->magic == ADS1015_CHECK_MAGIC))
#define ADS1015_IS_VALID(ads1015) ((0 != (ads1015)) && ((ads1015)->magic == ADS1015_CHECK_MAGIC))
#else
#define ADS1015_INTERFACE_IS_VALID(iface) (0 != (iface))
#define ADS1015_IS_VALID(ads1015) (0 != (ads1015))
#endif

#define TestAds 0   //只作为连续采集测试
/*
  测试前应将  g_ADS1015_BspInterface 的  ADS1015_SINGLE_MODE  改为 ADS1015_CONTINUOUS_MODE
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
   
   ADS1015_ContinuousConvert(&g_ADS1015_BspInterface, ADS1015_CHANNEL_2, ADS1015_FULL_SCALE_1024_MV);  //连续模式只对固定通道 有用
   
   job_schedule(MAIN_JOB_GROUP, 500, ads_test, NULL);
     
 }

#endif 




 /************************************************************************
  * @brief           ads1015初始化
  * @param[in]       iface ADS1015_BspInterface型指针变量 
  * @return          无  
***********************************************************************/
void ads1015_init(const ADS1015_BspInterface *iface) {
    if (!ADS1015_INTERFACE_IS_VALID(iface)) {
        return;
    }
    #if (BMS_SUPPORT_CURRENT_DIVIDER !=1)   //硬件不支持分流器，ADS10151单独使用IIC0，需单独初始化
    iface->init(iface);
    #endif
}


 /************************************************************************
  * @brief           ads1015单次采样
  * @param[in]       iface        ADS1015_BspInterface型指针变量 
  * @param[in]       channel      ADS1015_Channel型变量 
  * @param[in]       fullScaleVol ADS1015_FullScaleVoltage型变量 
  * @return          采样值 
***********************************************************************/
signed short ADS1015_SingleConvert(const ADS1015_BspInterface *iface, ADS1015_Channel channel, ADS1015_FullScaleVoltage fullScaleVol) {
    unsigned short i;
    signed short result;
    unsigned short tmp;
    if (!ADS1015_INTERFACE_IS_VALID(iface)) {
        return ADS1015_RESULT_ERROR;
    }
    // ADS1015芯片手册
    // bit[15]  Operational status / single-shot convertion start.---操作状态/单次转换开始
    // bit[14:12] Input multiplexer configuration.--输入多路复用器配置
    // bit[11:9] Programmable gain.--可编程增益
    // bit[8] Device operating mode.--设备操作模式 0连续转换 1单次转换
    // bit[7:5] Data rate.--传输速率
    // bit[1:0] Comparator queue and disable.--比较器队列和使能
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

    // 等待转换完成.
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
  * @brief           ads1015连续采样设置
  * @param[in]       iface        ADS1015_BspInterface型指针变量 
  * @param[in]       channel      ADS1015_Channel型变量 
  * @param[in]       fullScaleVol ADS1015_FullScaleVoltage型变量 
  * @return          1：设置成功  
***********************************************************************/
signed short  ADS1015_ContinuousConvert(const ADS1015_BspInterface *iface, ADS1015_Channel channel, ADS1015_FullScaleVoltage fullScaleVol)
{ 
     unsigned short tmp;
    if (!ADS1015_INTERFACE_IS_VALID(iface)) //判断iface 接口是否有效
    {
        return ADS1015_RESULT_ERROR;  // 无效返回错误状态
    }

    tmp = (0 << 15) |         //填写选通通道和满量程信息及其他配置
          (((unsigned short)channel) << 12) |
          (((unsigned short)fullScaleVol) << 9) |
          (iface->ad_mode << 8) |
          (iface->ads1015rate << 5) |
          (3 << 0);

    if (!iface->i2c_write(iface->bus, 0x01, tmp)) {   
        return ADS1015_RESULT_ERROR;
    }
   // if (!write_reg(iface->bus, 0x01, tmp | (1 << 15))) {  // 启动连续转换
  ///      return ADS1015_RESULT_ERROR;
  //  } 
  return 1;       
 }

 /************************************************************************
  * @brief           获取ads1015连续采样值
  * @param[in]       iface        ADS1015_BspInterface型指针变量
  * @return          采样值  
***********************************************************************/ 
 signed short ADS1015_Read_ContinuousResult (const ADS1015_BspInterface *iface)
{ 
  signed short result;
   if (!ADS1015_INTERFACE_IS_VALID(iface)) //判断iface 接口是否有效
    {
        return ADS1015_RESULT_ERROR;  // 无效返回错误状态
    }

   if (!iface->i2c_read(iface->bus, 0x00, (unsigned short *)&result)) 
    {
         return ADS1015_RESULT_ERROR;
    }
    return result >> 4;
          
 }



/************************************************************************
  * @brief           ads1015采样结果值转换为mv
  * @param[in]       iface               ADS1015_BspInterface型指针变量
  * @param[in]       fullScaleVol        ADS1015_FullScaleVoltage型变量
  * @return          转换值  
***********************************************************************/ 
signed short ADS1015_ResultToVoltageMv(signed short result, ADS1015_FullScaleVoltage fullScaleVol) {


    signed long res;
    static const unsigned short vt[] = {6144, 4096, 2048, 1024, 512, 256};
    
    if (fullScaleVol >= sizeof(vt) / sizeof(vt[0])) return 0;  //判断传入的参数是否超过数组范围

    if (result >= ADS1015_RESULT_FULL) return 0;  //对采集的结果进行判断,是否超范围
    if (result < -ADS1015_RESULT_FULL) return 0;

    res = vt[fullScaleVol];

    res = result * res;
    if (res >= 0) {
        res += ADS1015_RESULT_FULL / 2;  //四舍五入
    } else {
        res -= ADS1015_RESULT_FULL / 2; //四舍五入
    }

    return (signed short)((signed long)(res / ADS1015_RESULT_FULL)); //求实际的电压
}

 /************************************************************************
  * @brief           停止ads1015连续采样设置
  * @param[in]       iface        ADS1015_BspInterface型指针变量 
  * @param[in]       channel      ADS1015_Channel型变量 
  * @param[in]       fullScaleVol ADS1015_FullScaleVoltage型变量 
  * @return          1：设置成功  
***********************************************************************/
signed short ADS1015_Convert_Stop (const ADS1015_BspInterface *iface, ADS1015_Channel channel, ADS1015_FullScaleVoltage fullScaleVol) 
{ 
      unsigned short tmp;
    if (!ADS1015_INTERFACE_IS_VALID(iface)) //判断iface 接口是否有效
    {
        return ADS1015_RESULT_ERROR;  // 无效返回错误状态
    }
     tmp = (0 << 15) |         //填写选通通道和满量程信息及其他配置
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




