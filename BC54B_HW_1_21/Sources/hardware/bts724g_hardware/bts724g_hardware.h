/*******************************************************************************
**                       安徽力高新能源有限公司 Copyright (c)
**                            http://www.ligoo.cn/ 
** @file	    bts724g_hardware.h                                       

** @brief       1.完成bts724g模块的数据类型的定义 
                2.bts724g模块硬件初始化函数 
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
  * @brief    声明 bts724g_ops
  ***********************************************/
//extern const struct gpio_operations *bts724g_ops;   

/************************************************
  * @extern   pt_relay_input_pins
  * @brief    声明 pt_relay_input_pins
  ***********************************************/
extern const struct hcs12_gpio_detail  *pt_relay_input_pins;
//#pragma DATA_SEG DEFAULT

/************************************************
  * @extern   relay_contrl_v114
  * @brief    声明 relay_contrl_v114
  ***********************************************/
extern const struct hcs12_gpio_detail relay_contrl_v114[];

/************************************************
  * @extern   relay_contrl_v116
  * @brief    声明 relay_contrl_v116
  ***********************************************/
extern const struct hcs12_gpio_detail relay_contrl_v116[];

/************************************************
  * @extern   shdn_24v_v116
  * @brief    声明 shdn_24v_v116
  ***********************************************/
extern const struct hcs12_gpio_detail shdn_24v_v116[];


/************************************************************************
  * @brief           bts724g模块硬件初始化函数 
  * @param[in]       无
  * @return          无
***********************************************************************/
void bts724g_init(void);

/************************************************************************
  * @brief           bts724g模块输出控制函数函数 
  * @param[in]       无
  * @return          无
***********************************************************************/
 void relay_input_pin_set(unsigned char id, unsigned char value);


#endif