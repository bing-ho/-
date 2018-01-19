/*******************************************************************************
**                       ������������Դ���޹�˾ Copyright (c)
**                            http://www.ligoo.cn/ 
** @file	 �b   adc0_intermediate.h                                       

** @brief       1.���adc0ͨ���ӿں�������
                1.adc0�ɼ�ͨ������
                
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
  * @brief    adc0�ɼ�ͨ����������
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
  * @brief           ��ȡָ��ͨ����adֵ
  * @param[in]       channel  adc_onchip_channel��adcͨ��
  * @param[in]       *V       �ɼ�ֵ��ŵĵ�ַָ��
  * @return          1: ok   0:err
***********************************************************************/
uint8_t adc_onchip_read(const struct adc_onchip_channel *__FAR channel, uint16_t *v);
//uint8_t adc_onchip_read(adc_onchip_channel_t channel, uint16_t *v);   //liqing 20160418 ��

/***********************************************************************
  * @brief           ��ȡָ�����Ƶ�adֵ
  * @param[in]       *name    adcͨ������
  * @param[in]       *V       �ɼ�ֵ��ŵĵ�ַָ��
  * @return          1: ok   0 :err
***********************************************************************/
uint8_t adc_onchip_read_with_name(const char *name, uint16_t *v);

/***********************************************************************
  * @brief           ��ӡָ�����Ƶ�adֵ
  * @param[in]       *name    adcͨ������  
  * @return          1: ok   0:err
***********************************************************************/
uint8_t adc_onchip_dump_with_name(const char *name);

/***********************************************************************
  * @brief           ��ӡ����ͨ����adֵ
  * @param[in]       ��  
  * @return          1: ok   0:err
***********************************************************************/
uint8_t adc_onchip_dump_all(void);

/***********************************************************************
  * @brief           ��ӡ���вɼ�ͨ��������
  * @param[in]       ��  
  * @return          1: ok   0:err
***********************************************************************/
void adc_onchip_list_all(void);



#endif 