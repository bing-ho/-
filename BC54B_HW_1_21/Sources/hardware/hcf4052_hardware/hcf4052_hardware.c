/*******************************************************************************
**                       安徽力高新能源有限公司 Copyright (c)
**                            http://www.ligoo.cn/ 
** @file	    ad_hardware.c                                       

** @brief       1.完成hcf4052模块的数据类型的定义 
                2.完成hcf4052模块的初始化
               
** @copyright		Ligoo Inc.         
** @date		    2017-03-24.
** @author           
*******************************************************************************/ 




#include "gpio_interface.h"
#include "derivative.h"  
#include "bms_bcu.h"
#include "hcf4052_hardware.h"
#include "app_cfg.h"

#define STRING_HCF_U37_A         "HCF_U37_A"
#define STRING_HCF_U37_B         "HCF_U37_B"
#define STRING_HCF_U37_INH       "HCF_U37_INH"

// 初始化hcs12_gpio_detail 结构体 
const struct hcs12_gpio_detail insu_gain_ctrl_1_14[] = {
    { STRING_HCF_U37_A,     { &PTIP,  &PTP, &DDRP, 1 << 3}, 1  , 0},
    { STRING_HCF_U37_B,     { &PTIP,  &PTP, &DDRP, 1 << 2}, 1  , 0},
    { STRING_HCF_U37_INH,   { &PTIJ,  &PTJ, &DDRJ, 1 << 2}, 1  , 0},
    {(const char *)0, { (volatile uint8_t *)0, (volatile uint8_t *)0, (volatile uint8_t *)0, 0}, 0, 0},
};

// 初始化hcs12_gpio_detail 结构体 
const struct hcs12_gpio_detail insu_gain_ctrl_1_16[] = {
    { STRING_HCF_U37_A,     { &PTIP,  &PTP, &DDRP, 1 << 3}, 1  , 0},
    { STRING_HCF_U37_B,     { &PTIT,  &PTT, &DDRT, 1 << 0}, 1  , 0},
    { STRING_HCF_U37_INH,   { &PTIJ,  &PTJ, &DDRJ, 1 << 2}, 1  , 0},
    {(const char *)0, { (volatile uint8_t *)0, (volatile uint8_t *)0, (volatile uint8_t *)0, 0}, 0, 0},
};

// 初始化hcf4052_platform 结构体 
static const struct hcf4052_platform hcs4052_impl_1_14 = {
    (void *) &insu_gain_ctrl_1_14[0].io,
    (void *) &insu_gain_ctrl_1_14[1].io,
    (void *) &insu_gain_ctrl_1_14[2].io,
    (void *) &hcs12_gpio_ops,
};

// 初始化hcf4052_platform 结构体
static const struct hcf4052_platform hcs4052_impl_1_16 = { 
    (void *) &insu_gain_ctrl_1_16[0].io,
    (void *) &insu_gain_ctrl_1_16[1].io,
    (void *) &insu_gain_ctrl_1_16[2].io,
    (void *) &hcs12_gpio_ops,
};

// 定义hcf4052_platform 结构体指针
const struct hcf4052_platform *hcf4052_impl=NULL;

/************************************************************************
  * @brief           hcf4052模块初始化
  * @param[in]       hcf4052_platform型hcf4052操作属性  
  * @return          无
***********************************************************************/
void hcf4052_init(void) 
{
  #if BMS_SUPPORT_HARDWARE_BEFORE == 1    // yang 20161213
    if(hardware_io_revision_get() >= HW_VER_116)  //查询版本
    {
        hcf4052_impl = &hcs4052_impl_1_16;      
    }
    else
    {
        hcf4052_impl = &hcs4052_impl_1_14;
    }
 #else
     hcf4052_impl = &hcs4052_impl_1_16;
 #endif
    /* 初始化hcf4052状态*/
    (void)gpio_init(hcf4052_impl->gpio_ops, hcf4052_impl->ctrl_inh, GPIO_MODE_OUTPUT_PUSHPULL);
    (void)gpio_set_output(hcf4052_impl->gpio_ops, hcf4052_impl->ctrl_inh, 1);
    (void)gpio_init(hcf4052_impl->gpio_ops, hcf4052_impl->ctrl_a, GPIO_MODE_OUTPUT_PUSHPULL);
    (void)gpio_init(hcf4052_impl->gpio_ops, hcf4052_impl->ctrl_b, GPIO_MODE_OUTPUT_PUSHPULL);

   
}
