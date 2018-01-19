/*******************************************************************************
**                       ������������Դ���޹�˾ Copyright (c)
**                            http://www.ligoo.cn/ 
** @file	 �b   gpio_hardware.c                                       

** @brief       1.���gpioģ����������͵Ķ��� 
                2.���gpioģ��ĳ�ʼ��
                3.���gpioģ���io����������ƺ�io״̬�ɼ�
** @copyright		Ligoo Inc.         
** @date		    2017-03-24.
** @author           
*******************************************************************************/ 
#include "gpio_hardware.h"
#include "ucos_ii.h"


 
/************************************************************************
  * @brief           gpioģ��Ӳ����ʼ������ 
  * @param[in]       hcs12_gpio gpioģ���������ԼĴ���  gpio_mode gpio �ڹ���ģʽ
  * @return          1��init ok   0��init err
***********************************************************************/
uint8_t hcs12_gpio_init(const struct hcs12_gpio *__FAR io, enum gpio_mode mode) 
{
    OS_CPU_SR cpu_sr = 0;
    OS_ENTER_CRITICAL();
    if (GPIO_MODE_OUTPUT_PUSHPULL == mode || GPIO_MODE_OUTPUT_OPENDRAIN == mode) //gpioģʽ == ������� �� ��©���
    {
        *io->dir_reg |= io->bit;
        OS_EXIT_CRITICAL();
        return 1;
    }
    if (GPIO_MODE_INPUT == mode)   //gpioģʽ == ����ģʽ
    {
        *io->dir_reg &= ~io->bit;
        OS_EXIT_CRITICAL();
        return 1;
    }
    OS_EXIT_CRITICAL();
    return 0;
}

 
/************************************************************************
  * @brief           ��ȡgpioģ�鹤��ģʽ 
  * @param[in]       hcs12_gpio_t gpioģ������
  * @return          io����ģʽ
***********************************************************************/
enum gpio_mode hcs12_gpio_get_mode(hcs12_gpio_t io) 
{
    if (*io->dir_reg & io->bit) 
    {
        return GPIO_MODE_OUTPUT_PUSHPULL; //�������ģʽ
    } 
    else 
    {
        return GPIO_MODE_INPUT; //����ģʽ
    }
}


/************************************************************************
  * @brief           �ж�GPIO�����Ƿ�Ϊ��
  * @param[in]       hcs12_gpio gpioģ��Ĵ���
  * @return          1:����Ϊ��  0:����Ϊ��
***********************************************************************/
uint8_t hcs12_gpio_input_is_high(const struct hcs12_gpio *__FAR io) 
{
    if (*io->dir_reg & io->bit)  //input mode
    { 
        return 0;
    }
    return 0 != (*io->in_reg & io->bit);
}


/************************************************************************
  * @brief           �ж�GPIO����Ƿ�Ϊ��
  * @param[in]       hcs12_gpio gpioģ��Ĵ���
  * @return          1:���Ϊ��   0:���Ϊ��
***********************************************************************/
uint8_t hcs12_gpio_output_is_high(const struct hcs12_gpio *__FAR io) 
{
    if (*io->dir_reg & io->bit)  //output mode
    { 
        return 0 != (*io->out_reg & io->bit);
    }
    return 0;
}


/************************************************************************
  * @brief           ����gpio�����
  * @param[in]       hcs12_gpio gpioģ��Ĵ���      is_high--�����ƽ 1Ϊ�ߣ�0Ϊ��
  * @return          1:set_ok��0:set_err
***********************************************************************/
uint8_t hcs12_gpio_set_output(const struct hcs12_gpio *__FAR io, uint8_t is_high) 
{
    if (*io->dir_reg & io->bit)  //output mode
    { 
        OS_CPU_SR cpu_sr = 0;
        OS_ENTER_CRITICAL();
        if (is_high)    //�������Ϊ�� 
        {
            *io->out_reg |= io->bit;
        } 
        else 
        {
            *io->out_reg &= ~io->bit;
        }
        OS_EXIT_CRITICAL();
        return 1;
    }
    return 0;
}


/************************************************************************
  * @brief          ��ʼ�� gpio�����ӿں����ṹ��
  * @param[in]       
  * @return         
***********************************************************************/
const struct gpio_operations hcs12_gpio_ops = 
{
    (uint8_t (*)(void *, enum gpio_mode))hcs12_gpio_init,  //gpio�ڳ�ʼ��()
    (enum gpio_mode (*)(void *))hcs12_gpio_get_mode,       //��ȡgpio��ģʽ()
    (uint8_t (*)(void *))hcs12_gpio_input_is_high,         //pio�������Ƿ�Ϊ��()
    (uint8_t (*)(void *))hcs12_gpio_output_is_high,        //gpio������Ƿ�Ϊ��()
    (uint8_t (*)(void *, uint8_t))hcs12_gpio_set_output,   //gpio���������()
};

