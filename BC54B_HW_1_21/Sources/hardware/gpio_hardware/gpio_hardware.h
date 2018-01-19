/*******************************************************************************
**                       ������������Դ���޹�˾ Copyright (c)
**                            http://www.ligoo.cn/ 
** @file	 �b   gpio_hardware.h                                       

** @brief       1.���gpioģ����������͵Ķ��� 
                2.���gpioģ��ĳ�ʼ��
                3.���gpioģ���io����������ƺ�io״̬�ɼ�
** @copyright		Ligoo Inc.         
** @date		    2017-03-24.
** @author           
*******************************************************************************/ 

#ifndef __GPIO_HARDWARE_H__
#define __GPIO_HARDWARE_H__

#include "stdint.h" 
#include "derivative.h" 

#ifndef __FAR
#define __FAR
#endif

/************************************************
  * @struct   hcs12_gpio
  * @brief    оƬIO���������ԼĴ���
 ***********************************************/

struct hcs12_gpio {
    volatile uint8_t *in_reg;            //����Ĵ���
    volatile uint8_t *out_reg;           //����Ĵ���
    volatile uint8_t *dir_reg;           //����Ĵ���
    volatile uint8_t bit;                //λ
};
/************************************************
  * @struct   hcs12_gpio_detail
  * @brief   оƬGPIO�ڳ�ʼ������
 ***********************************************/
struct hcs12_gpio_detail {
    const char *name;                  //����
    struct hcs12_gpio io;              //io������
    //uint8_t def_val;                 //Ĭ��ֵ
    //uint8_t revert;                  //��λֵ
    
    uint8_t def_val:1;                 //Ĭ��ֵ
    uint8_t revert:1;                  //��λֵ
};
  
/************************************************
  * @struct   gpio_operations
  * @brief    gpioģ�������������
 ***********************************************/
struct gpio_operations {
    uint8_t (*init)(void *__FAR io, enum gpio_mode mode);      //gpio�ڳ�ʼ��()
    enum gpio_mode (*get_mode)(void *__FAR io);                //gpioģʽ()
    uint8_t (*input_is_high)(void *__FAR io);                  //gpio�����Ƿ�Ϊ��()
    uint8_t (*output_is_high)(void *__FAR io);                 //gpio����Ƿ�Ϊ��()
    uint8_t (*set_output)(void *__FAR io, uint8_t is_high);    //�������()
};
/************************************************
  * @enum   gpio_mode
  * @brief  gpio�ڹ���ģʽ
  ***********************************************/
enum gpio_mode {
    GPIO_MODE_INPUT,                 //����ģʽ
    GPIO_MODE_OUTPUT_PUSHPULL,       //�������
    GPIO_MODE_OUTPUT_OPENDRAIN,      //��©���
};

/************************************************
  * @extern   hcs12_gpio_ops
  * @brief    ���� hcs12_gpio_ops
  ***********************************************/
extern const struct gpio_operations hcs12_gpio_ops;




/************************************************
  * @typedef   hcs12_gpio_t
  * @brief  ����hcs12_gpio����hcs12_gpio_t
  ***********************************************/
typedef const struct hcs12_gpio *__FAR hcs12_gpio_t;

/************************************************************************
  * @brief           gpioģ��Ӳ����ʼ������ 
  * @param[in]       hcs12_gpio gpioģ���������ԼĴ���  gpio_mode gpio �ڹ���ģʽ
  * @return          1��init ok   0��init err
***********************************************************************/
uint8_t hcs12_gpio_init(hcs12_gpio_t io, enum gpio_mode mode);

/************************************************************************
  * @brief           ��ȡgpioģ�鹤��ģʽ 
  * @param[in]       hcs12_gpio_t gpioģ������
  * @return          io����ģʽ
***********************************************************************/
enum gpio_mode hcs12_gpio_get_mode(hcs12_gpio_t io);

/************************************************************************
  * @brief           �ж�GPIO�����Ƿ�Ϊ��
  * @param[in]       hcs12_gpio gpioģ��Ĵ���
  * @return          1:����Ϊ��  0:����Ϊ��
***********************************************************************/
uint8_t hcs12_gpio_input_is_high(hcs12_gpio_t io);

/************************************************************************
  * @brief           �ж�GPIO����Ƿ�Ϊ��
  * @param[in]       hcs12_gpio gpioģ��Ĵ���
  * @return          1:���Ϊ��   0:���Ϊ��
***********************************************************************/
uint8_t hcs12_gpio_output_is_high(hcs12_gpio_t io);

/************************************************************************
  * @brief           ����gpio�����
  * @param[in]       hcs12_gpio gpioģ��Ĵ���      is_high--�����ƽ 1Ϊ�ߣ�0Ϊ��
  * @return          1:set_ok��0:set_err
***********************************************************************/
uint8_t hcs12_gpio_set_output(hcs12_gpio_t io, uint8_t is_high);






#endif 