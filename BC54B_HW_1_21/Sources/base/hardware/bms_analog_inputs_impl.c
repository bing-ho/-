/**
 *
 * Copyright (c) 2012 Ligoo Inc.
 *
 * @file bms_system_voltage_impl.c
 * @brief
 * @note
 * @author 
 * @date 2015-10-12
 *
 */
#include "bms_analog_inputs_impl.h"

#pragma MESSAGE DISABLE C1420 // Result of function-call is ignored
#pragma MESSAGE DISABLE C5703 // Parameter is not used
#pragma MESSAGE DISABLE C5919 // Conversion of floating to unsigned integral

static INT16U g_analog_inputs_value = 0;
extern uint16_t adc_ref_voltage;
#pragma DATA_SEG __RPAGE_SEG PAGED_RAM

static ADC_ChannelParameter g_analog_inputs_ad;
INT16U analog_inputs_ad_buff[BCU_ANALOG_INPUTS_BUFF_MAX] = {0};

#pragma DATA_SEG DEFAULT

static void analog_inputs_notify_finished(const ADC_ChannelParameter * _PAGED p)  {
  *((INT8U *far)p->pdata) = 1;
  
}

void bcu_analog_inputs_init(void)
{
    g_analog_inputs_ad.interval = 2000;
    g_analog_inputs_ad.times = BCU_ANALOG_INPUTS_BUFF_MAX;
    g_analog_inputs_ad.buf = (INT16U* _PAGED)analog_inputs_ad_buff;
    g_analog_inputs_ad.notify_finished = analog_inputs_notify_finished;
    
    job_schedule(MAIN_JOB_GROUP, BCU_ANALOG_INPUTS_JOB_PERIODIC, bcu_analog_inputs_detect, NULL);
    bcu_calc_analog_inputs(&g_analog_inputs_value);
}

INT16U bcu_analog_inputs_get(void)
{
    return g_analog_inputs_value;
}

/** 模拟输入信号检测 */
INT8U bcu_calc_analog_inputs(INT16U *volt)
{
    INT8U i;
    float sum = 0;
    INT8U flag = 0;
    
    g_analog_inputs_ad.pdata = &flag;
        
    if(volt == NULL) return 0;
        
    adc0_channel_start(BCU_ANALOG_INPUTS_AD_PORT,&g_analog_inputs_ad);
    
    while (flag == 0) sleep(10);

    
    for(i=0; i<BCU_ANALOG_INPUTS_BUFF_MAX; i++)
    {
        sum += analog_inputs_ad_buff[i];
    }
    sum /= BCU_ANALOG_INPUTS_BUFF_MAX;
  
    if(INT16U_IS_NEGATIVE(sum)) 
        sum = 0;
    else 
        sum = (sum * adc_ref_voltage / 4096) * BCU_ANALOG_INPUTS_MAGNIFICATION;
    
    *volt = (INT16U)sum;
    
    return 1;
}

void bcu_analog_inputs_detect(void* data)//系统供电电压检测
{
    INT16U volt;
    
    UNUSED(data); 
    if(bcu_calc_analog_inputs(&volt))    
    {
        g_analog_inputs_value = volt;
    }
}


