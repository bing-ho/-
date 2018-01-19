/*******************************************************************************
**                       ������������Դ���޹�˾ Copyright (c)
**                            http://www.ligoo.cn/ 
** @file	 �b   gpio_interface.h                                       

** @brief       1.���gpioģ������ݽӿڷ�װ
                
** @copyright		Ligoo Inc.         
** @date		    2017-04-26.
** @author           
*******************************************************************************/ 
#ifndef __GPIO_INTERFACE_H__
#define __GPIO_INTERFACE_H__

#include "stdint.h"
#include "gpio_hardware.h"

#ifndef __FAR
#define __FAR
#endif

#define inline

/************************************************
  * @typedef   gpio_t
  * @brief     ����gpio_t��ָ������
 ***********************************************/
typedef void *__FAR gpio_t;

/************************************************
  * @typedef   gpio_ops_t
  * @brief     ����gpio_operations�ṹ�峤ָ������
 ***********************************************/
typedef struct gpio_operations const *__FAR gpio_ops_t;

/************************************************************************
  * @brief           gpioģ��Ӳ����ʼ������ 
  * @param[in]       ops         gpio_operations �Ͳ�������
  * @param[in]       io          hcs12_gpioģ���������ԼĴ���  
  * @param[in]       mode        gpio_mode�ڹ���ģʽ
  * @return          1��init ok   0��init err
***********************************************************************/
inline uint8_t gpio_init(gpio_ops_t ops, gpio_t io, enum gpio_mode mode);

/************************************************************************
  * @brief           ��ȡgpioģ�鹤��ģʽ  
  * @param[in]       ops         gpio_operations �Ͳ�������
  * @param[in]       io          hcs12_gpioģ���������ԼĴ���  
  * @return          io����ģʽ
***********************************************************************/
inline enum gpio_mode gpio_get_mode(gpio_ops_t ops, gpio_t io);

/************************************************************************
  * @brief           ��ȡgpio���������������
  * @param[in]       mode         gpio_mode io�������������
  * @return          �����ַ���
***********************************************************************/
inline const char *gpio_mode_to_string(enum gpio_mode moe);

/************************************************************************
  * @brief           �ж�GPIO����Ƿ�Ϊ��
  * @param[in]       ops         gpio_operations �Ͳ�������
  * @param[in]       io          hcs12_gpioģ���������ԼĴ���  
  * @return          1:���Ϊ��   0:���Ϊ��
***********************************************************************/
inline uint8_t gpio_input_is_high(gpio_ops_t ops, gpio_t io);

/************************************************************************
  * @brief           ����gpio�����
  * @param[in]       ops         gpio_operations �Ͳ�������
  * @param[in]       io          hcs12_gpioģ���������ԼĴ���  
  * @param[in]       is_high     �����ƽ 1Ϊ�ߣ�0Ϊ��
  * @return          1:���Ϊ��   0:���Ϊ��
***********************************************************************/
inline uint8_t gpio_output_is_high(gpio_ops_t ops, gpio_t io);

/************************************************************************
  * @brief           ����gpio�����
  * @param[in]       ops         gpio_operations �Ͳ�������
  * @param[in]       io          hcs12_gpioģ���������ԼĴ���  
  * @param[in]       is_high     �����ƽ 1Ϊ�ߣ�0Ϊ��
  * @return          1:�ɹ�   0:ʧ��
***********************************************************************/
inline uint8_t gpio_set_output(gpio_ops_t ops, gpio_t io, uint8_t is_high);

/************************************************************************
  * @brief           ����gpio�����
  * @param[in]       mode         gpio_mode io�������������
  * @return          �����ַ���
***********************************************************************/
void gpio_dump(struct gpio_operations const *__FAR ops, void *__FAR io, const char *prefix);

#endif
