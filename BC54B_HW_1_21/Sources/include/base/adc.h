/**
 *
 * Copyright (c) 2014 Ligoo Inc.
 *
 * @file  adc.h
 * @brief adc驱动头文件
 * @note 
 * @version 1.00
 * @author
 * @date 2014/9/19
 *
 */
#ifndef _ADC_H
#define _ADC_H

#include "includes.h"
#include "bms_base_cfg.h"
#include "bms_job.h"      
#include "bms_util.h"

//#define ADC_REFERENCE               5000 //mv

#define ADC0_CHANNEL_IN_USE_MAX     8
#define ADC1_CHANNEL_IN_USE_MAX     2

typedef enum
{
    kAdcDev0,
    kAdcDev1,
    kAdcDevMax
}AdcDev;

typedef struct _ADC_ChannelParameter{
    void *__FAR pdata;
    INT16U interval; /// 时间间隔 ms
    INT8U times; /// 采样次数
    INT16U* _PAGED buf; /// 采样保存的数据.
    void (*notify_finished)(const struct _ADC_ChannelParameter * _PAGED);
}ADC_ChannelParameter;

char adc0_channel_start(unsigned char channel, const ADC_ChannelParameter* _PAGED parameter);

void adc0_init(void);

void adc_para_init(void);
INT8U adc_add_channel(AdcDev dev,INT8U start_channel,INT8U channel_num);
INT8U adc_channel_data_get(AdcDev dev,INT8U channel,INT8U count,INT16U* _PAGED adc_data);




#endif