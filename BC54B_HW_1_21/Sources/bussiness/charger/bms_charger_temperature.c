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
        temp = TempQuery(value, NTCTAB_CWF4B103F3950);       //���ݲɼ����ĵ�ѹ���
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
        temp = TempQuery(value, NTCTAB_CWF4B103F3950);       //���ݲɼ����ĵ�ѹ���
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
        temp = TempQuery(value, NTCTAB_CWF4B103F3950);       //���ݲɼ����ĵ�ѹ���
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
        temp = TempQuery(value, NTCTAB_CWF4B103F3950);       //���ݲɼ����ĵ�ѹ���
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
    
    // ͳ������
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
    
    
    // ͳ�ƿ��
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
* ��    �ƣ�bms_get_chgr_dc_outlet_temperature
* ���������id    ͨ����� 0~1
* �� �� ֵ����
* ����˵������ȡ���������¶�
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
* ��    �ƣ�bms_get_chgr_ac_outlet_temperature
* ���������id    ͨ����� 0~1
* �� �� ֵ����
* ����˵������ȡ����������¶�
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
* ��    �ƣ�bms_get_chgr_ac_outlet_temperature_num
* �����������
* �� �� ֵ������������¸и���
* ����˵������ȡ����������¶ȸ���
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
* ��    �ƣ�bms_get_chgr_dc_outlet_temperature_num
* �����������
* �� �� ֵ�����������¸и���
* ����˵������ȡ���������¶ȸ���
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
* ��    �ƣ�bms_set_chgr_ac_outlet_high_temperature
* ���������temperature �¶�
* �� �� ֵ����
* ����˵���������������������¶�
**************************************************/
void bms_set_chgr_ac_outlet_high_temperature(INT8U temperature)
{
    OS_INIT_CRITICAL();
    OS_ENTER_CRITICAL();
    g_charger_ac_outlet_info.high_temperature = temperature;
    OS_EXIT_CRITICAL();
};
/**************************************************
* ��    �ƣ�bms_set_chgr_dc_outlet_high_temperature
* ���������temperature �¶�
* �� �� ֵ����
* ����˵�������ÿ�����������¶�
**************************************************/
void bms_set_chgr_dc_outlet_high_temperature(INT8U temperature)
{
    OS_INIT_CRITICAL();
    OS_ENTER_CRITICAL();
    g_charger_dc_outlet_info.high_temperature = temperature;
    OS_EXIT_CRITICAL();
};
/**************************************************
* ��    �ƣ�bms_get_chgr_ac_outlet_high_temperature
* �����������
* �� �� ֵ������������¶�����¶�
* ����˵������ȡ�������������¶�
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
* ��    �ƣ�bms_get_chgr_dc_outlet_high_temperature
* ���������id    ͨ����� 0~1
* �� �� ֵ�����������¶�����¶�
* ����˵������ȡ������������¶�
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
* ��    �ƣ�bms_set_chgr_ac_outlet_temperature_exception
* ���������state       �쳣״̬
                0   ����
                1   �쳣
* �� �� ֵ����
* ����˵������������������¸���������״̬
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
* ��    �ƣ�bms_set_chgr_dc_outlet_temperature_exception
* ���������state       �쳣״̬
                0   ����
                1   �쳣
* �� �� ֵ����
* ����˵�������ÿ��������¸���������״̬
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
* ��    �ƣ�bms_get_chgr_ac_outlet_temperature_exception
* �����������
* �� �� ֵ��0   ����
            1   �쳣
* ����˵������ȡ����������¸���������״̬
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
* ��    �ƣ�bms_get_chgr_dc_outlet_temperature_exception
* �����������
* �� �� ֵ��0   ����
            1   �쳣
* ����˵������ȡ���������¸���������״̬
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

