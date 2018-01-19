/*******************************************************************************
**                       ������������Դ���޹�˾ Copyright (c)
**                            http://www.ligoo.cn/ 
** @file	 �b   bts724g_hardware.h                                       

** @brief       1.���bts724gģ����������͵Ķ��� 
                2.bts724gģ��Ӳ����ʼ������ 
** @copyright		Ligoo Inc.         
** @date		    2017-03-24.
** @author            
*******************************************************************************/ 

#ifndef __BTS724G_HARDWARE_H__
#define __BTS724G_HARDWARE_H__


  #include "gpio_interface.h" 
  //#include "type.h"
//#include "pwm_ect_impl.h"
//#include "gpio.h"     
//#include "bms_bcu.h"



//#pragma DATA_SEG __RPAGE_SEG PAGED_RAM
/************************************************
  * @extern   bts724g_ops
  * @brief    ���� bts724g_ops
  ***********************************************/
//extern const struct gpio_operations *bts724g_ops;   

/************************************************
  * @extern   pt_relay_input_pins
  * @brief    ���� pt_relay_input_pins
  ***********************************************/
extern const struct hcs12_gpio_detail  *pt_relay_input_pins;
//#pragma DATA_SEG DEFAULT

/************************************************
  * @extern   relay_contrl_v114
  * @brief    ���� relay_contrl_v114
  ***********************************************/
extern const struct hcs12_gpio_detail relay_contrl_v114[];

/************************************************
  * @extern   relay_contrl_v116
  * @brief    ���� relay_contrl_v116
  ***********************************************/
extern const struct hcs12_gpio_detail relay_contrl_v116[];

/************************************************
  * @extern   shdn_24v_v116
  * @brief    ���� shdn_24v_v116
  ***********************************************/
extern const struct hcs12_gpio_detail shdn_24v_v116[];


/************************************************************************
  * @brief           bts724gģ��Ӳ����ʼ������ 
  * @param[in]       ��
  * @return          ��
***********************************************************************/
void bts724g_init(void);

/************************************************************************
  * @brief           bts724gģ��������ƺ������� 
  * @param[in]       ��
  * @return          ��
***********************************************************************/
 void relay_input_pin_set(unsigned char id, unsigned char value);


#endif