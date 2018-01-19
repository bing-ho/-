/**
 *
 * Copyright (c) 2012 Ligoo Inc.
 *
 * @file bms_charger.c
 * @brief
 * @note
 * @author
 * @date 2012-5-8
 *
 */
#ifndef BMS_CHARGER_TEMPERATURE_C_
#define BMS_CHARGER_TEMPERATURE_C_

#include "bms_charger_temperature.h"
#include "adc0_intermediate.h"
#pragma DATA_SEG __RPAGE_SEG PAGED_RAM_CHARGER
static charger_outlet_type g_charger_ac_outlet_info = {0};
static charger_outlet_type g_charger_dc_outlet_info = {0};
#pragma DATA_SEG DEFAULT

extern const struct adc_onchip_channel adc_onchip_temp_t1_det_adc_v120;
extern const struct adc_onchip_channel adc_onchip_temp_t2_det_adc_v120;
extern const struct adc_onchip_channel adc_onchip_temp_t3_det_adc_v120;
extern const struct adc_onchip_channel adc_onchip_temp_t4_det_adc_v120;

void charger_outlet_temperature_update(void* data)
{
    INT8U high_temperature = 0, temp = 0;
    INT8U num = 0, i = 0;  
    INT8U temp_exception = 0;
    INT8U res = 1;
    INT16U value = 0;
    
    OS_CPU_SR cpu_sr = 0; 
    UNUSED(data);
    
    if (!adc_onchip_read(&adc_onchip_temp_t3_det_adc_v120, &value)) 
    {
        res = 0;
    }
    if(res != 0)
    {
        temp = TempQuery(value, NTCTAB_CWF4B103F3950);       //根据采集到的电压查表
        if(temp == INVALID_TEMP_VALUE)
        {
            temp = 0;
        }
        else
        {
            temp += 10;
        }
        
        OS_ENTER_CRITICAL();
        g_charger_ac_outlet_info.temperature[0] = temp;
        OS_EXIT_CRITICAL();
    }
    
    res = 1;
    if (!adc_onchip_read(&adc_onchip_temp_t4_det_adc_v120, &value)) 
    {
        res = 0;
    }
    if(res != 0)
    {
        temp = TempQuery(value, NTCTAB_CWF4B103F3950);       //根据采集到的电压查表
        if(temp == INVALID_TEMP_VALUE)
        {
            temp = 0;
        }
        else
        {
            temp += 10;
        }
        
        OS_ENTER_CRITICAL();
        g_charger_ac_outlet_info.temperature[1] = temp;
        OS_EXIT_CRITICAL();
    }
    
    res = 1;
    if (!adc_onchip_read(&adc_onchip_temp_t1_det_adc_v120, &value)) 
    {
        res = 0;
    }
    if(res != 0)
    {
        temp = TempQuery(value, NTCTAB_CWF4B103F3950);       //根据采集到的电压查表
        if(temp == INVALID_TEMP_VALUE)
        {
            temp = 0;
        }
        else
        {
            temp += 10;
        }
    
        OS_ENTER_CRITICAL();
        g_charger_dc_outlet_info.temperature[0] = temp;
        OS_EXIT_CRITICAL();
    }
    
    res = 1;
    if (!adc_onchip_read(&adc_onchip_temp_t2_det_adc_v120, &value)) 
    {
        res = 0;
    }
    if(res != 0)
    {
        temp = TempQuery(value, NTCTAB_CWF4B103F3950);       //根据采集到的电压查表
        if(temp == INVALID_TEMP_VALUE)
        {
            temp = 0;
        }
        else
        {
            temp += 10;
        }
        
        OS_ENTER_CRITICAL();
        g_charger_dc_outlet_info.temperature[1] = temp;
        OS_EXIT_CRITICAL();
    }
    
    // 统计慢充
    num = bms_get_chgr_ac_outlet_temperature_num();
    for(i=0; i<num; i++)
    {
        temp = bms_get_chgr_ac_outlet_temperature(i);
        if(!charger_is_valid_temperature(temp))//if(!bmu_is_valid_temperature(temp))
        {
            temp_exception = 1;
            continue;
        }
        if(temp > high_temperature)
        {
            high_temperature = temp;
        }
    }
    bms_set_chgr_ac_outlet_high_temperature(high_temperature);
    bms_set_chgr_ac_outlet_temperature_exception(temp_exception);
    
    
    // 统计快充
    temp_exception = 0;
    high_temperature = 0;
    num = bms_get_chgr_dc_outlet_temperature_num();
    for(i=0; i<num; i++)
    {
        temp = bms_get_chgr_dc_outlet_temperature(i);
        if(!charger_is_valid_temperature(temp))//if(!bmu_is_valid_temperature(temp))
        {
            temp_exception = 1;
            continue;
        }
        if(temp > high_temperature)
        {
            high_temperature = temp;
        }
    }
    bms_set_chgr_dc_outlet_high_temperature(high_temperature);
    bms_set_chgr_dc_outlet_temperature_exception(temp_exception);
}

/**************************************************
* 名    称：bms_get_chgr_dc_outlet_temperature
* 输入参数：id    通道编号 0~1
* 返 回 值：无
* 功能说明：读取快充充电插座温度
**************************************************/
INT8U bms_get_chgr_dc_outlet_temperature(INT8U id)
{
    INT8U temp = 0;            
    OS_CPU_SR cpu_sr = 0;
    
    if(id >= CHARGER_OUTLET_TEMP_NUM_MAX) return 0;
    OS_ENTER_CRITICAL();
    temp = g_charger_dc_outlet_info.temperature[id];
    OS_EXIT_CRITICAL();
    
    return temp;
} 
/**************************************************
* 名    称：bms_get_chgr_ac_outlet_temperature
* 输入参数：id    通道编号 0~1
* 返 回 值：无
* 功能说明：读取慢充充电插座温度
**************************************************/
INT8U bms_get_chgr_ac_outlet_temperature(INT8U id)
{
    INT8U temp = 0;      
    OS_CPU_SR cpu_sr = 0;
    
    if(id >= CHARGER_OUTLET_TEMP_NUM_MAX) return 0;
    OS_ENTER_CRITICAL();
    temp = g_charger_ac_outlet_info.temperature[id];
    OS_EXIT_CRITICAL();
    
    return temp;
}

/**************************************************
* 名    称：bms_get_chgr_ac_outlet_temperature_num
* 输入参数：无
* 返 回 值：慢充充电插座温感个数
* 功能说明：读取慢充充电插座温度个数
**************************************************/
INT8U bms_get_chgr_ac_outlet_temperature_num(void)
{
    INT8U temp = (INT8U)config_get(kChgrAcTempNumIndex);
    if(temp > BMS_CHGR_AC_OUTLET_NUM_MAX)
        temp = BMS_CHGR_AC_OUTLET_NUM_MAX;
    else if(temp < BMS_CHGR_AC_OUTLET_NUM_MIN)
        temp = BMS_CHGR_AC_OUTLET_NUM_MIN;
    
    return temp;
}

/**************************************************
* 名    称：bms_get_chgr_dc_outlet_temperature_num
* 输入参数：无
* 返 回 值：快充充电插座温感个数
* 功能说明：读取快充充电插座温度个数
**************************************************/
INT8U bms_get_chgr_dc_outlet_temperature_num(void)
{
    INT8U temp = (INT8U)config_get(kChgrDcTempNumIndex);
    if(temp > BMS_CHGR_DC_OUTLET_NUM_MAX)
        temp = BMS_CHGR_DC_OUTLET_NUM_MAX;
    else if(temp < BMS_CHGR_DC_OUTLET_NUM_MIN)
        temp = BMS_CHGR_DC_OUTLET_NUM_MIN;
    
    return temp;
}
/**************************************************
* 名    称：bms_set_chgr_ac_outlet_high_temperature
* 输入参数：temperature 温度
* 返 回 值：无
* 功能说明：设置慢充充电插座最高温度
**************************************************/
void bms_set_chgr_ac_outlet_high_temperature(INT8U temperature)
{
    OS_INIT_CRITICAL();
    OS_ENTER_CRITICAL();
    g_charger_ac_outlet_info.high_temperature = temperature;
    OS_EXIT_CRITICAL();
};
/**************************************************
* 名    称：bms_set_chgr_dc_outlet_high_temperature
* 输入参数：temperature 温度
* 返 回 值：无
* 功能说明：设置快充充电插座最高温度
**************************************************/
void bms_set_chgr_dc_outlet_high_temperature(INT8U temperature)
{
    OS_INIT_CRITICAL();
    OS_ENTER_CRITICAL();
    g_charger_dc_outlet_info.high_temperature = temperature;
    OS_EXIT_CRITICAL();
};
/**************************************************
* 名    称：bms_get_chgr_ac_outlet_high_temperature
* 输入参数：无
* 返 回 值：慢充充电插座温度最高温度
* 功能说明：读取慢充充电插座最高温度
**************************************************/
INT8U bms_get_chgr_ac_outlet_high_temperature(void)
{
    INT8U high_temperature = 0;
    OS_INIT_CRITICAL();
    
    OS_ENTER_CRITICAL();
    high_temperature = g_charger_ac_outlet_info.high_temperature;
    OS_EXIT_CRITICAL();
    return high_temperature;
}

/**************************************************
* 名    称：bms_get_chgr_dc_outlet_high_temperature
* 输入参数：id    通道编号 0~1
* 返 回 值：快充充电插座温度最高温度
* 功能说明：读取快充充电插座最高温度
**************************************************/
INT8U bms_get_chgr_dc_outlet_high_temperature(void)
{
    INT8U high_temperature = 0;
    OS_INIT_CRITICAL();
    
    OS_ENTER_CRITICAL();
    high_temperature = g_charger_dc_outlet_info.high_temperature;
    OS_EXIT_CRITICAL();
    return high_temperature;
}
/**************************************************
* 名    称：bms_set_chgr_ac_outlet_temperature_exception
* 输入参数：state       异常状态
                0   正常
                1   异常
* 返 回 值：无
* 功能说明：设置慢充充电插座温感排线脱落状态
**************************************************/
void bms_set_chgr_ac_outlet_temperature_exception(INT8U state)
{
    OS_INIT_CRITICAL();
    if(state != 0) state = 1;
    OS_ENTER_CRITICAL();
    g_charger_ac_outlet_info.exception = state;
    OS_EXIT_CRITICAL();
};
/**************************************************
* 名    称：bms_set_chgr_dc_outlet_temperature_exception
* 输入参数：state       异常状态
                0   正常
                1   异常
* 返 回 值：无
* 功能说明：设置快充充电插座温感排线脱落状态
**************************************************/
void bms_set_chgr_dc_outlet_temperature_exception(INT8U state)
{
    OS_INIT_CRITICAL();
    if(state != 0) state = 1;
    OS_ENTER_CRITICAL();
    g_charger_dc_outlet_info.exception = state;
    OS_EXIT_CRITICAL();
}
/**************************************************
* 名    称：bms_get_chgr_ac_outlet_temperature_exception
* 输入参数：无
* 返 回 值：0   正常
            1   异常
* 功能说明：读取慢充充电插座温感排线脱落状态
**************************************************/
INT8U bms_get_chgr_ac_outlet_temperature_exception(void)
{
    OS_INIT_CRITICAL();
    INT8U temp = 0;
    OS_ENTER_CRITICAL();
    temp = g_charger_ac_outlet_info.exception;
    OS_EXIT_CRITICAL();
    if(temp != 0) temp = 1;
    return temp;
}
/**************************************************
* 名    称：bms_get_chgr_dc_outlet_temperature_exception
* 输入参数：无
* 返 回 值：0   正常
            1   异常
* 功能说明：读取快充充电插座温感排线脱落状态
**************************************************/
INT8U bms_get_chgr_dc_outlet_temperature_exception(void)
{
    OS_INIT_CRITICAL();
    INT8U temp = 0;
    OS_ENTER_CRITICAL();
    temp = g_charger_dc_outlet_info.exception;
    OS_EXIT_CRITICAL();
    if(temp != 0) temp = 1;
    return temp;
}

#endif   // BMS_CHARGER_TEMPERATURE_C_

