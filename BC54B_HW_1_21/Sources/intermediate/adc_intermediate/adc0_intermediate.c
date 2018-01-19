/*******************************************************************************
**                       安徽力高新能源有限公司 Copyright (c)
**                            http://www.ligoo.cn/ 
** @file	    adc0_intermediate.c                                       

** @brief       1.完成adc0通道接口函数处理
                1.adc0采集通道定义
                
** @copyright		Ligoo Inc.         
** @date		    2017-04-11.
** @author            
*******************************************************************************/ 
#include <string.h>
#include <stdio.h>
#include "adc0_intermediate.h"
#include "bms_bcu.h"

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a)  (sizeof(a)/sizeof(a[0]))
#endif

#define ADC_REF_VOLTAGE_BEFORE_V120_5000mV 5000
#define ADC_REF_VOLTAGE_V120_4096mV        4096

#pragma DATA_SEG __RPAGE_SEG PAGED_RAM                    //liqing 20160418 加
uint16_t adc_ref_voltage = ADC_REF_VOLTAGE_V120_4096mV;
const struct adc_onchip_channel * const *adc_channels = NULL;
#pragma DATA_SEG DEFAULT

/*采集量的名称  采样间隔ms  采样次数  丢弃?个最大值   丢弃?个最小值  采集通道      mul     div */
const struct adc_onchip_channel adc_onchip_temp_u48 = {
"TEMP_U48",   {     2,          10,         3,              3,          0},         1,      1,
};
//VREF_5V（U3）
const struct adc_onchip_channel adc_onchip_temp_u3_v114 = {
"TEMP_U3",    {     2,          10,         3,              3,          12},        1,      1,
};

const struct adc_onchip_channel adc_onchip_temp_u3_v116 = {
"TEMP_U3",    {     2,          10,         3,              3,           5},        1,      1,
};

const struct adc_onchip_channel adc_onchip_cc_adc = {
"CC_ADC",     {     1,          10,         3,              3,           6},        1,      1,
};

const struct adc_onchip_channel adc_onchip_cc2_adc_v114 = {
"CC2_ADC",    {     1,          10,         3,              3,           5},        1,      1,
};

const struct adc_onchip_channel adc_onchip_cc2_adc_v116 = {
"CC2_ADC",    {     1,          10,         3,              3,          13},        1,      1,
};

const struct adc_onchip_channel adc_onchip_imd_hvp = {
"IMD_HV+",    {     1,          20,         5,              5,           7},        1,      1,
};

const struct adc_onchip_channel adc_onchip_imd_hvn = {
"IMD_HV-",    {     1,          20,         5,              5,          14},        1,      1,
};
//24V_OUT_DET ,系统供电电压
const struct adc_onchip_channel adc_onchip_battery_v = {
"BATTERY_V",  {     2,          10,         3,              3,          15},       11,      1,
};

//24V_IN_DET 直流充电机供电电压
const struct adc_onchip_channel adc_onchip_v24_det_v114 = {
"24V_DET",    {     2,          10,         3,              3,          13},       11,      1,
};

const struct adc_onchip_channel adc_onchip_v24_det_v116 = {
"24V_DET",    {     2,          10,         3,              3,          12},       21,      1,
};

const struct adc_onchip_channel adc_onchip_5vhall_adc_v116 = {
"5V_HALL",    {     2,          10,         3,              3,           8},        2,      1,
};
// V120 replaced 5V_HALL with MOTOR_DET
const struct adc_onchip_channel adc_onchip_motor_det_adc_v120 = {
"MOTOR_DET",  {     2,          10,         3,              3,           8},        1,      1,
};

const struct adc_onchip_channel adc_onchip_8vhall_adc_v116 = {
"8V_HALL",    {     2,          10,         3,              3,           1},        2,      1,
};

const struct adc_onchip_channel adc_onchip_5vdet_adc_v116 = {
"5V_DET",     {     2,          10,         3,              3,           9},        1,      1,
};
//BATTERY_DET 铅酸电池电压
const struct adc_onchip_channel adc_onchip_vbat_det_adc_v116 = {
"VBAT_DET",   {     2,          10,         3,              3,           2},       21,      1,
};

const struct adc_onchip_channel adc_onchip_temp_t4_det_adc_v120 = {
"TEMP_T4",    {     2,          10,         3,              3,          10},        1,      1,
};

const struct adc_onchip_channel adc_onchip_temp_t3_det_adc_v120 = {
"TEMP_T3",    {     2,          10,         3,              3,           3},        1,      1,
};

const struct adc_onchip_channel adc_onchip_temp_t2_det_adc_v120 = {
"TEMP_T2",    {     2,          10,         3,              3,          11},        1,      1,
};

const struct adc_onchip_channel adc_onchip_temp_t1_det_adc_v120 = {
"TEMP_T1",    {     2,          10,         3,              3,           4},        1,      1,
};

//#pragma DATA_SEG __RPAGE_SEG PAGED_RAM
const struct adc_onchip_channel *const adc_channels_v114[] = {
    &adc_onchip_temp_u48,               // 0
    &adc_onchip_cc2_adc_v114,           // 5
    &adc_onchip_cc_adc,                 // 6
    &adc_onchip_imd_hvp,                // 7
    &adc_onchip_temp_u3_v114,           // 12
    &adc_onchip_v24_det_v114,           // 13
    &adc_onchip_imd_hvn,                // 14
    &adc_onchip_battery_v,              // 15
    NULL
};

const struct adc_onchip_channel *const adc_channels_v116[] = {
    &adc_onchip_temp_u48,               // 0
    &adc_onchip_8vhall_adc_v116,        // 1
    &adc_onchip_vbat_det_adc_v116,      // 2
    &adc_onchip_temp_u3_v116,           // 5
    &adc_onchip_cc_adc,                 // 6
    &adc_onchip_imd_hvp,                // 7
    &adc_onchip_5vhall_adc_v116,        // 8
    &adc_onchip_5vdet_adc_v116,         // 9
    &adc_onchip_v24_det_v116,           // 12
    &adc_onchip_cc2_adc_v116,           // 13
    &adc_onchip_imd_hvn,                // 14
    &adc_onchip_battery_v,              // 15
    NULL
};

const struct adc_onchip_channel *const adc_channels_v120[] = {
    &adc_onchip_temp_u48,            // PAD0
    //&adc_onchip_8vhall_adc_v116,      // 1 // removed from V120
    &adc_onchip_vbat_det_adc_v116,      // 2
    &adc_onchip_temp_t3_det_adc_v120,   // 3 // new added from V120
    &adc_onchip_temp_t1_det_adc_v120,   // 4 // new added from V120
    &adc_onchip_temp_u3_v116,           // 5
    &adc_onchip_cc_adc,                 // 6
    &adc_onchip_imd_hvp,                // 7
    &adc_onchip_motor_det_adc_v120,     // 8
    &adc_onchip_5vdet_adc_v116,         // 9
    &adc_onchip_temp_t4_det_adc_v120,   // 10 // new added from V120
    &adc_onchip_temp_t2_det_adc_v120,   // 11 // new added from V120
    &adc_onchip_v24_det_v116,           // 12
    &adc_onchip_cc2_adc_v116,           // 13
    &adc_onchip_imd_hvn,                // 14
    &adc_onchip_battery_v,              // 15
    NULL
};
//#pragma DATA_SEG DEFAULT
#pragma DATA_SEG __RPAGE_SEG PAGED_RAM                            //liqing 20160418 加
const struct adc_onchip_channel *const *all_ver_adc_channel_list[] = 
{
    adc_channels_v114,
    adc_channels_v114,
    adc_channels_v116,
    adc_channels_v120
};
#pragma DATA_SEG DEFAULT

/***********************************************************************
  * @brief           初始化adc配置参数
  * @param[in]       无
  * @return          1: ok   0:err
***********************************************************************/
void adc_onchip_init(void) 
{
    HW_VER_NUM ver = hardware_io_revision_get();
    
 #if BMS_SUPPORT_BY5248D == 0  
    adc_channels = all_ver_adc_channel_list[ver];
 #else
    adc_channels = all_ver_adc_channel_list[HW_VER_120]; 
 #endif   
    
    if(ver >= HW_VER_120)
    {
        adc_ref_voltage = ADC_REF_VOLTAGE_V120_4096mV;
    }
    else
    {
        adc_ref_voltage = ADC_REF_VOLTAGE_BEFORE_V120_5000mV;
    }
}  
INT8U adc_use_flag = 0;
void clear_adc_use_flag(void)
{
    OS_INIT_CRITICAL();
    OS_ENTER_CRITICAL();
    adc_use_flag = 0;
    OS_EXIT_CRITICAL();
}

void set_adc_use_flag(void)
{
    OS_INIT_CRITICAL();
    OS_ENTER_CRITICAL();
    adc_use_flag = 1;
    OS_EXIT_CRITICAL();
}
INT8U is_adc_in_use(void)
{  
    INT8U flag = 0;
    OS_INIT_CRITICAL();
    OS_ENTER_CRITICAL();
    flag = adc_use_flag;
    OS_EXIT_CRITICAL();
    return  flag;
}

/***********************************************************************
  * @brief           读取指定通道的ad值
  * @param[in]       channel  adc_onchip_channel型adc通道
  * @param[in]       *V       采集值存放的地址指针
  * @return          1: ok   0:err
***********************************************************************/
//uint8_t adc_onchip_read(const struct adc_onchip_channel * channel, uint16_t *v) {  //liqing 20160418 加
uint8_t adc_onchip_read(const struct adc_onchip_channel *__FAR channel, uint16_t *v) {
    uint16_t v16;
    if(is_adc_in_use()==1)
    {
       return 0;  
    }
    set_adc_use_flag();
    if (!channel) 
    {
        clear_adc_use_flag();
        return 0;  
    }
    if (!v) 
    {
        clear_adc_use_flag();
        return 0;
    }
    if (0 == xadc0_start(&channel->param)) 
    {
        clear_adc_use_flag();
        return 0;
    }
    if (0 == xadc0_wait(&v16, channel->param.interval * channel->param.times * 2)) 
    {
        clear_adc_use_flag();
        return 0;
    }

    v16 = (uint16_t)((uint32_t)((uint32_t)v16 * adc_ref_voltage / 4096));
    *v = (uint16_t)(uint32_t)(((uint32_t)v16 * channel->mul_factor  + channel->div_factor / 2) / channel->div_factor);
    clear_adc_use_flag();
    return 1;
}

/***********************************************************************
  * @brief           读取指定名称的ad值
  * @param[in]       *name    adc通道名称
  * @param[in]       *V       采集值存放的地址指针
  * @return          1: ok   0 :err
***********************************************************************/
uint8_t adc_onchip_read_with_name(const char *name, uint16_t *v) {
    uint8_t i;
    for (i = 0; ; ++i) {
        if(adc_channels[i] == NULL){
            break;
        }
        if (strcmp(adc_channels[i]->name, name) == 0) {
            return adc_onchip_read(adc_channels[i], v);
        }
    }
    return 0;
}

/***********************************************************************
  * @brief           打印指定名称的ad值
  * @param[in]       *name    adc通道名称  
  * @return          1: ok   0:err
***********************************************************************/
uint8_t adc_onchip_dump_with_name(const char *name) {
    uint8_t ret = 1;
    uint8_t i;
    uint16_t v=0;
    for (i = 0; ; ++i) {
        if(adc_channels[i] == NULL){
            break;
        }
        if (strcmp(name, adc_channels[i]->name) == 0) {
            
            if (adc_onchip_read(adc_channels[i], &v)) {
                (void)printf("%s:%u\n", adc_channels[i]->name, v);
                 
                return 1;
            }

            return 0;
        }
    }
    return 0;
}

/***********************************************************************
  * @brief           打印所有通道的ad值
  * @param[in]       无  
  * @return          1: ok   0:err
***********************************************************************/
uint8_t adc_onchip_dump_all(void) {
    uint8_t ret = 1;
    uint8_t i;
    uint16_t v;
    for (i = 0; ; ++i) {
        if(adc_channels[i] == NULL){
            break;
        }
        if (0 != adc_onchip_read(adc_channels[i], &v)) {
            (void)printf("%s:%d\n", adc_channels[i]->name, v);
        } else {
            (void)printf("%s:error\n", adc_channels[i]->name);
            return 0;
        }
    }
    return 1;
}

/***********************************************************************
  * @brief           打印所有采集通道的名称
  * @param[in]       无  
  * @return          1: ok   0:err
***********************************************************************/
void adc_onchip_list_all(void) {
    uint8_t i;
    for (i = 0; ; ++i) {
        if(adc_channels[i] == NULL){
            break;
        }
        (void)printf("%s\n", adc_channels[i]->name);
    }
}