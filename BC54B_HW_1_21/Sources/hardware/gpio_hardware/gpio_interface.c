/*******************************************************************************
**                       安徽力高新能源有限公司 Copyright (c)
**                            http://www.ligoo.cn/ 
** @file	    gpio_interface.c                                       

** @brief       1.完成gpio模块的数据接口封装
                
** @copyright		Ligoo Inc.         
** @date		    2017-04-26.
** @author           
*******************************************************************************/ 
#include <stdio.h>
#include "gpio_interface.h"


#define TestGpio  0

/*
  测试用例
*/
#if TestGpio == 1
#include "bms_job_impl.h"
#include "bms_system.h"
#include "ucos_ii.h"

#define GPIO_TEST_CYCLE  500    //循环操作时间

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
    //先初始化为输入口
     gpio_station=hcs12_gpio_init(&gpio_test_contrl[0].io,GPIO_MODE_INPUT);
    
    //读取输入状态
    gpio_station = hcs12_gpio_input_is_high(&gpio_test_contrl[0].io);
    
    //在初始化为输出口
    gpio_station = hcs12_gpio_init(&gpio_test_contrl[1].io,GPIO_MODE_OUTPUT_PUSHPULL);
    
    //设置输出状态高
    
    gpio_station=hcs12_gpio_set_output(&gpio_test_contrl[1].io,1);
    
    
    //在读取输出状态
   gpio_station = hcs12_gpio_output_is_high(&gpio_test_contrl[1].io) ;
    
    //设置输出状态低
    gpio_station=hcs12_gpio_set_output(&gpio_test_contrl[1].io,0);
    
    //在读取输出状态
    gpio_station = hcs12_gpio_output_is_high(&gpio_test_contrl[1].io) ;
    
    
        
   // OS_EXIT_CRITICAL();
    
    
}


#endif 
/************************************************************************
  * @brief           gpio模块硬件初始化函数 
  * @param[in]       ops         gpio_operations 型操作函数
  * @param[in]       io          hcs12_gpio模块配置属性寄存器  
  * @param[in]       mode        gpio_mode口工作模式
  * @return          1：init ok   0：init err
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
  * @brief           获取gpio模块工作模式  
  * @param[in]       ops         gpio_operations 型操作函数
  * @param[in]       io          hcs12_gpio模块配置属性寄存器  
  * @return          io工作模式
***********************************************************************/
inline enum gpio_mode gpio_get_mode(struct gpio_operations const *__FAR ops, void *__FAR io) {
    return ops->get_mode(io);
}

/************************************************************************
  * @brief           判断GPIO输入是否为高 
  * @param[in]       ops         gpio_operations 型操作函数
  * @param[in]       io          hcs12_gpio模块配置属性寄存器  
  * @return          1:输入为高  0:输入为低
***********************************************************************/
inline uint8_t gpio_input_is_high(struct gpio_operations const *__FAR ops, void *__FAR io) {
    return ops->input_is_high(io);
}

/************************************************************************
  * @brief           判断GPIO输出是否为高
  * @param[in]       ops         gpio_operations 型操作函数
  * @param[in]       io          hcs12_gpio模块配置属性寄存器  
  * @return          1:输出为高   0:输出为低
***********************************************************************/
inline uint8_t gpio_output_is_high(struct gpio_operations const *__FAR ops, void *__FAR io) {
    return ops->output_is_high(io);
}

/************************************************************************
  * @brief           设置gpio口输出
  * @param[in]       ops         gpio_operations 型操作函数
  * @param[in]       io          hcs12_gpio模块配置属性寄存器  
  * @param[in]       is_high     输出电平 1为高，0为低
  * @return          1:成功   0:失败
***********************************************************************/
inline uint8_t gpio_set_output(struct gpio_operations const *__FAR ops, void *__FAR io, uint8_t is_high) {
    return ops->set_output(io, is_high);
}

/************************************************************************
  * @brief           获取gpio设置输入输出类型
  * @param[in]       mode         gpio_mode io口输入输出类型
  * @return          类型字符串
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
  * @brief           设置gpio口输出
  * @param[in]       mode         gpio_mode io口输入输出类型
  * @return          类型字符串
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

