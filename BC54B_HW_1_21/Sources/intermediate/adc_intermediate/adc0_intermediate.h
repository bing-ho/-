/*******************************************************************************
**                       安徽力高新能源有限公司 Copyright (c)
**                            http://www.ligoo.cn/ 
** @file	    adc0_intermediate.h                                       

** @brief       1.完成adc0通道接口函数处理
                1.adc0采集通道定义
                
** @copyright		Ligoo Inc.         
** @date		    2017-04-11.
** @author            
*******************************************************************************/ 

#ifndef __ADC0_INTERMEDIATE_H_
#define __ADC0_INTERMEDIATE_H_

#include "stdint.h"
#include "adc_xgate.h"


/************************************************
  * @struct   adc_onchip_channel
  * @brief    adc0采集通道设置属性
 ***********************************************/
struct adc_onchip_channel {
    const char *name;
    struct xadc0_param param;
    uint16_t mul_factor;
    uint16_t div_factor;
};

extern const struct adc_onchip_channel adc_onchip_v24_det_v114;
extern const struct adc_onchip_channel adc_onchip_v24_det_v116;
extern const struct adc_onchip_channel adc_onchip_battery_v;

extern const struct adc_onchip_channel adc_onchip_cc2_adc_v114;
extern const struct adc_onchip_channel adc_onchip_cc2_adc_v116;
extern const struct adc_onchip_channel adc_onchip_cc_adc;
extern const struct adc_onchip_channel adc_onchip_imd_hvp;
extern const struct adc_onchip_channel adc_onchip_imd_hvn;
extern const struct adc_onchip_channel adc_onchip_temp_u3_v114;
extern const struct adc_onchip_channel adc_onchip_temp_u3_v116;
extern const struct adc_onchip_channel adc_onchip_temp_u48;

extern const struct adc_onchip_channel adc_onchip_5vhall_adc_v116;
extern const struct adc_onchip_channel adc_onchip_8vhall_adc_v116;
extern const struct adc_onchip_channel adc_onchip_5vdet_adc_v116;
extern const struct adc_onchip_channel adc_onchip_vbat_det_adc_v116;

extern const struct adc_onchip_channel adc_onchip_temp_t1_det_adc_v120;
extern const struct adc_onchip_channel adc_onchip_temp_t2_det_adc_v120;
extern const struct adc_onchip_channel adc_onchip_temp_t3_det_adc_v120;
extern const struct adc_onchip_channel adc_onchip_temp_t4_det_adc_v120;

extern const struct adc_onchip_channel adc_onchip_motor_det_adc_v120;



typedef struct adc_onchip_channel const *adc_onchip_channel_t;

/***********************************************************************
  * @brief           读取指定通道的ad值
  * @param[in]       channel  adc_onchip_channel型adc通道
  * @param[in]       *V       采集值存放的地址指针
  * @return          1: ok   0:err
***********************************************************************/
uint8_t adc_onchip_read(const struct adc_onchip_channel *__FAR channel, uint16_t *v);
//uint8_t adc_onchip_read(adc_onchip_channel_t channel, uint16_t *v);   //liqing 20160418 加

/***********************************************************************
  * @brief           读取指定名称的ad值
  * @param[in]       *name    adc通道名称
  * @param[in]       *V       采集值存放的地址指针
  * @return          1: ok   0 :err
***********************************************************************/
uint8_t adc_onchip_read_with_name(const char *name, uint16_t *v);

/***********************************************************************
  * @brief           打印指定名称的ad值
  * @param[in]       *name    adc通道名称  
  * @return          1: ok   0:err
***********************************************************************/
uint8_t adc_onchip_dump_with_name(const char *name);

/***********************************************************************
  * @brief           打印所有通道的ad值
  * @param[in]       无  
  * @return          1: ok   0:err
***********************************************************************/
uint8_t adc_onchip_dump_all(void);

/***********************************************************************
  * @brief           打印所有采集通道的名称
  * @param[in]       无  
  * @return          1: ok   0:err
***********************************************************************/
void adc_onchip_list_all(void);



#endif 