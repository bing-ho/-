/*******************************************************************************
**                       ������������Դ���޹�˾ Copyright (c)
**                            http://www.ligoo.cn/ 
** @file	 �b   gpio_interface.c                                       

** @brief       1.���gpioģ������ݽӿڷ�װ
                
** @copyright		Ligoo Inc.         
** @date		    2017-04-26.
** @author           
*******************************************************************************/ 
#include <stdio.h>
#include "gpio_interface.h"


#define TestGpio  0

/*
  ��������
*/
#if TestGpio == 1
#include "bms_job_impl.h"
#include "bms_system.h"
#include "ucos_ii.h"

#define GPIO_TEST_CYCLE  500    //ѭ������ʱ��

#define MYTEST_GPIO_1    "RLY_1"
#define MYTEST_GPIO_2    "RLY_2"



const struct hcs12_gpio_detail gpio_test_contrl[] = {
    {MYTEST_GPIO_1, { &PORTD, &PORTD, &DDRD, 1<<5 }, 0, 0 },
    {MYTEST_GPIO_2, { &PORTB, &PORTB, &DDRB, 1<<0 }, 0, 0 },

    {(const char *)0, { (volatile uint8_t *)0, (volatile uint8_t *)0, (volatile uint8_t *)0, 0}, 0, 0},
};

#pragma DATA_SEG __RPAGE_SEG PAGED_RAM
uint8_t gpio_station=0;
#pragma DATA_SEG DEFAULT


void gpio_test(void* pdata)
{
  
   // OS_CPU_SR cpu_sr = 0;
    
    pdata = pdata;
        

   // OS_ENTER_CRITICAL();
    //�ȳ�ʼ��Ϊ�����
     gpio_station=hcs12_gpio_init(&gpio_test_contrl[0].io,GPIO_MODE_INPUT);
    
    //��ȡ����״̬
    gpio_station = hcs12_gpio_input_is_high(&gpio_test_contrl[0].io);
    
    //�ڳ�ʼ��Ϊ�����
    gpio_station = hcs12_gpio_init(&gpio_test_contrl[1].io,GPIO_MODE_OUTPUT_PUSHPULL);
    
    //�������״̬��
    
    gpio_station=hcs12_gpio_set_output(&gpio_test_contrl[1].io,1);
    
    
    //�ڶ�ȡ���״̬
   gpio_station = hcs12_gpio_output_is_high(&gpio_test_contrl[1].io) ;
    
    //�������״̬��
    gpio_station=hcs12_gpio_set_output(&gpio_test_contrl[1].io,0);
    
    //�ڶ�ȡ���״̬
    gpio_station = hcs12_gpio_output_is_high(&gpio_test_contrl[1].io) ;
    
    
        
   // OS_EXIT_CRITICAL();
    
    
}


#endif 
/************************************************************************
  * @brief           gpioģ��Ӳ����ʼ������ 
  * @param[in]       ops         gpio_operations �Ͳ�������
  * @param[in]       io          hcs12_gpioģ���������ԼĴ���  
  * @param[in]       mode        gpio_mode�ڹ���ģʽ
  * @return          1��init ok   0��init err
***********************************************************************/
inline uint8_t gpio_init(struct gpio_operations const *__FAR ops, void *__FAR io, enum gpio_mode mode) 
{
    #if TestGpio == 1
    static uint8_t temp=0;
    if(temp==0) {
      
    job_schedule(MAIN_JOB_GROUP, GPIO_TEST_CYCLE, gpio_test, NULL);
    temp=1;
    }
    
    else temp=1;
    #endif 
    return ops->init(io, mode);
}

/************************************************************************
  * @brief           ��ȡgpioģ�鹤��ģʽ  
  * @param[in]       ops         gpio_operations �Ͳ�������
  * @param[in]       io          hcs12_gpioģ���������ԼĴ���  
  * @return          io����ģʽ
***********************************************************************/
inline enum gpio_mode gpio_get_mode(struct gpio_operations const *__FAR ops, void *__FAR io) {
    return ops->get_mode(io);
}

/************************************************************************
  * @brief           �ж�GPIO�����Ƿ�Ϊ�� 
  * @param[in]       ops         gpio_operations �Ͳ�������
  * @param[in]       io          hcs12_gpioģ���������ԼĴ���  
  * @return          1:����Ϊ��  0:����Ϊ��
***********************************************************************/
inline uint8_t gpio_input_is_high(struct gpio_operations const *__FAR ops, void *__FAR io) {
    return ops->input_is_high(io);
}

/************************************************************************
  * @brief           �ж�GPIO����Ƿ�Ϊ��
  * @param[in]       ops         gpio_operations �Ͳ�������
  * @param[in]       io          hcs12_gpioģ���������ԼĴ���  
  * @return          1:���Ϊ��   0:���Ϊ��
***********************************************************************/
inline uint8_t gpio_output_is_high(struct gpio_operations const *__FAR ops, void *__FAR io) {
    return ops->output_is_high(io);
}

/************************************************************************
  * @brief           ����gpio�����
  * @param[in]       ops         gpio_operations �Ͳ�������
  * @param[in]       io          hcs12_gpioģ���������ԼĴ���  
  * @param[in]       is_high     �����ƽ 1Ϊ�ߣ�0Ϊ��
  * @return          1:�ɹ�   0:ʧ��
***********************************************************************/
inline uint8_t gpio_set_output(struct gpio_operations const *__FAR ops, void *__FAR io, uint8_t is_high) {
    return ops->set_output(io, is_high);
}

/************************************************************************
  * @brief           ��ȡgpio���������������
  * @param[in]       mode         gpio_mode io�������������
  * @return          �����ַ���
***********************************************************************/
inline const char *gpio_mode_to_string(enum gpio_mode mode) {
    switch (mode) {
    case GPIO_MODE_INPUT:
        return "INPUT";
    case GPIO_MODE_OUTPUT_PUSHPULL:
        return "OUTPUT_PP";
    case GPIO_MODE_OUTPUT_OPENDRAIN:
        return "OUTPUT_OD";
    default:
        return "UNKNOWN";
    }
}

/************************************************************************
  * @brief           ����gpio�����
  * @param[in]       mode         gpio_mode io�������������
  * @return          �����ַ���
***********************************************************************/
void gpio_dump(struct gpio_operations const *__FAR ops, void *__FAR io, const char *prefix) {
    if (ops && io) {
        uint8_t v;
        enum gpio_mode mode = gpio_get_mode(ops, io);
        if (mode == GPIO_MODE_INPUT) {
            v = gpio_input_is_high(ops, io);
        } else {
            v = gpio_output_is_high(ops, io);
        }

        if (prefix) {
            (void)printf("%s:%s:%d\n", prefix, gpio_mode_to_string(mode), v);
        } else {
            (void)printf("%s:%d\n", gpio_mode_to_string(mode), v);
        }
    }
}

