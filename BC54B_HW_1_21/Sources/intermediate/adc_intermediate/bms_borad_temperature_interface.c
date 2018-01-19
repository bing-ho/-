/**
 *
 * Copyright (c) 2012 Ligoo Inc.
 *
 * @file bms_board_temperature_impl.c
 * @brief
 * @note
 * @author Liwei Dong
 * @date 2014-9-23
 *
 */
#include "bms_borad_temperature_interface.h"
#include "bms_system.h"
#include "adc0_intermediate.h"
#include "bms_bcu.h"

#pragma MESSAGE DISABLE C1420 // Result of function-call is ignored
#pragma MESSAGE DISABLE C4001 // Condition always FALSE
#pragma MESSAGE DISABLE C5703 // Parameter is not used
#pragma MESSAGE DISABLE C5919 // Conversion of floatint to unsigned integral

//static INT8U g_boardTemperatureValue = 0;      //liqing 20170428
//static INT8U g_boardTemperature2Value = 0;

#if BMS_SUPPORT_BOARD_TEMPERATURE

#pragma DATA_SEG __RPAGE_SEG PAGED_RAM
//INT16U board_temperature_buff[BCU_BOARD_TEMPERATURE_BUFF_MAX] = {0};  liqing 20170428 屏蔽
static const struct adc_onchip_channel *adc_onchip_temp_u3;
static INT8U g_boardTemperatureValue = 0;
static INT8U g_boardTemperature2Value = 0; 
   
#pragma DATA_SEG DEFAULT

/***********************************************************************
  * @brief           板载温度接口初始化
  * @param[in]       无  
  * @return          无
***********************************************************************/  
void hw_board_temp_init(void)
{
#if BMS_SUPPORT_BY5248D == 0 
#if BMS_SUPPORT_HARDWARE_BEFORE == 1   // yang 20161213
    if(hardware_io_revision_get() >= HW_VER_116){//V1.16 & V1.20
        adc_onchip_temp_u3 = &adc_onchip_temp_u3_v116;
    }
    else{
        adc_onchip_temp_u3 = &adc_onchip_temp_u3_v114;
    }
#else
    adc_onchip_temp_u3 = &adc_onchip_temp_u3_v116;
#endif
#else
    adc_onchip_temp_u3 = &adc_onchip_temp_u3_v116; 
#endif
}

/***********************************************************************
  * @brief           板载温度初始化函数
  * @param[in]       无  
  * @return          无
***********************************************************************/ 
void board_temperature_init(void)
{    
    hw_board_temp_init();
    
    job_schedule(MAIN_JOB_GROUP, BCU_BOARD_TEMPERATURE_JOB_PERIODIC, bcu_board_temperature_detect, NULL);
}

/***********************************************************************
  * @brief           获取板载温度1大小
  * @param[in]       无  
  * @return          温度值
***********************************************************************/ 
inline INT8U board_temperature_get(void)
{
    return g_boardTemperatureValue;
}

/***********************************************************************
  * @brief           获取板载温度2大小
  * @param[in]       无  
  * @return          温度值
***********************************************************************/ 
inline INT8U board_temperature2_get(void)
{
    return g_boardTemperature2Value;
}

/***********************************************************************
  * @brief           返回有效温度的平均值
  * @param[in]       无  
  * @return          温度值
***********************************************************************/ 
INT8U board_average_temperature_get(void)
{
    INT8U cnt = 0;
    INT16U temp = 0;
    
    if(bmu_is_valid_temperature(g_boardTemperatureValue))  //判断温度是否有效
    {
        temp += g_boardTemperatureValue;
        cnt++;
    }
    if(bmu_is_valid_temperature(g_boardTemperature2Value))
    {
        temp += g_boardTemperature2Value;
        cnt++;
    }
    if(cnt) temp /= cnt;
    else temp = 0;
    
    return (INT8U)temp;
}



/***********************************************************************
  * @brief           获取板载温度值
  * @param[in]       which   温度序号  
  * @param[in]       *temp   返回温度值 
  * @return          1：ok  0：err
***********************************************************************/ 
INT8U bcu_calc_board_temperature(INT8U which, INT8U *temp)
{
    INT16U vol = 0;

    if(temp == NULL) 
    {
        return 0;
    }
    if (which != 0 && which != 1) 
    {
        return 0;
    }
    
    if (!adc_onchip_read(which == 0 ? &adc_onchip_temp_u48 : adc_onchip_temp_u3, &vol)) 
    {
        return 0;
    }

    //should use NTCTAB_NTCG163JF103FTB, but the param most the same as NTCTAB_NCP18XH103
    *temp = TempQuery(vol, NTCTAB_NCP18XH103) + 10;//offset:-50   
    if(*temp==9)  *temp=0; //在NTC表中没有查询到，返回OXFF，导致溢出*temp=9；需排查掉 2017019sunyan
    return 1;
}

/***********************************************************************
  * @brief           获取板载温度值
  * @param[in]       data   未使用  
  * @return          无
***********************************************************************/ 
void bcu_board_temperature_detect(void* data) //bcu板载温度
{
    INT8U temp = 0;
    
    UNUSED(data);
    if(bcu_calc_board_temperature(0, &temp)) 
    {
        g_boardTemperatureValue = temp;
    }
    if(bcu_calc_board_temperature(1, &temp)) 
    {
        g_boardTemperature2Value = temp;
    }
}

#endif

