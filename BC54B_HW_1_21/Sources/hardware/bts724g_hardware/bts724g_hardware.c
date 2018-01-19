/*******************************************************************************
**                       安徽力高新能源有限公司 Copyright (c)
**                            http://www.ligoo.cn/ 
** @file	    bts724g_hardware.c                                       

** @brief       1.完成bts724g模块的数据类型的定义 
                2.bts724g模块硬件初始化函数 
** @copyright		Ligoo Inc.         
** @date		    2017-03-24.
** @author           
*******************************************************************************/ 

#include "bts724g_hardware.h"

#include "ect_intermediate.h"
#include "app_cfg.h"
#include "stddef.h"

    
//#include "bms_bcu.h"

#define STRING_RLY_1    "RLY_1"
#define STRING_RLY_2    "RLY_2"
#define STRING_RLY_3    "RLY_3"
#define STRING_RLY_4    "RLY_4"
#define STRING_RLY_5    "RLY_5"
#define STRING_RLY_6    "RLY_6"
#define STRING_RLY_7    "RLY_7"
#define STRING_RLY_8    "RLY_8"
#define STRING_DO_1     "DO1"
#define STRING_DO_2     "DO2"

// 定义hcs12_gpio_detail 结构体数组
const struct hcs12_gpio_detail relay_contrl_v114[] = {
    {STRING_RLY_1, { &PORTB, &PORTB, &DDRB, 1<<0 }, 0, 0 },
    {STRING_RLY_2, { &PORTB, &PORTB, &DDRB, 1<<2 }, 0, 0 },
    {STRING_RLY_3, { &PORTB, &PORTB, &DDRB, 1<<3 }, 0, 0 },
    {STRING_RLY_4, { &PORTB, &PORTB, &DDRB, 1<<5 }, 0, 0 },
    {STRING_RLY_5, { &PORTB, &PORTB, &DDRB, 1<<6 }, 0, 0 },
    {STRING_RLY_6, { &PORTC, &PORTC, &DDRC, 1<<4 }, 0, 0 },
    {STRING_RLY_7, { &PORTC, &PORTC, &DDRC, 1<<5 }, 0, 0 },
    {STRING_RLY_8, { &PORTC, &PORTC, &DDRC, 1<<7 }, 0, 0 },
    {STRING_DO_1,   { &PORTK, &PORTK, &DDRK, 1<<6 }, 0, 1 },
    {STRING_DO_2,   { &PORTK, &PORTK, &DDRK, 1<<3 }, 0, 1 },
    {(const char *)0, { (volatile uint8_t *)0, (volatile uint8_t *)0, (volatile uint8_t *)0, 0}, 0, 0},
};

// 定义hcs12_gpio_detail 结构体数组
const struct hcs12_gpio_detail relay_contrl_v116[] = {
    {STRING_RLY_1, { &PORTB, &PORTB, &DDRB, 1<<0 }, 0, 0 }, //CTRL1
    {STRING_RLY_2, { &PORTB, &PORTB, &DDRB, 1<<2 }, 0, 0 }, //CTRL2
    {STRING_RLY_3, { &PORTB, &PORTB, &DDRB, 1<<3 }, 0, 0 }, //CTRL3
    {STRING_RLY_4, { &PORTB, &PORTB, &DDRB, 1<<5 }, 0, 0 }, //CTRL4
    {STRING_RLY_5, { &PORTB, &PORTB, &DDRB, 1<<6 }, 0, 0 }, //CTRL5
    {STRING_RLY_6, { &PORTC, &PORTC, &DDRC, 1<<4 }, 0, 0 }, //CTRL6
    {STRING_RLY_7, { &PORTC, &PORTC, &DDRC, 1<<5 }, 0, 0 }, //CTRL7
    {STRING_RLY_8, { &PORTC, &PORTC, &DDRC, 1<<7 }, 0, 0 }, //CTRL8
    {STRING_DO_1,   { &PORTB, &PORTB, &DDRB, 1<<1 }, 0, 1 }, //DO1
    {STRING_DO_2,   { &PORTB, &PORTB, &DDRB, 1<<4 }, 0, 1 }, //DO2
    {(const char *)0, { (volatile uint8_t *)0, (volatile uint8_t *)0, (volatile uint8_t *)0, 0}, 0, 0},
};

// 定义hcs12_gpio_detail 结构体数组
const struct hcs12_gpio_detail shdn_24v_v116[] = {
    { "24VSHDN", {&PT1AD1, &PT1AD1, &DDR1AD1, 1 << 5}, 1, 0},
    {(const char *)0, { (volatile uint8_t *)0, (volatile uint8_t *)0, (volatile uint8_t *)0, 0}, 0, 0},
};


//#pragma DATA_SEG __RPAGE_SEG PAGED_RAM
// 定义gpio_operations 结构体指针
//const struct gpio_operations *bts724g_ops=&hcs12_gpio_ops;
//新加占用ram空间  需要检查 arm空间是否会超


 const struct hcs12_gpio_detail *pt_relay_input_pins = NULL;

//#pragma DATA_SEG DEFAULT


/************************************************************************
  * @brief           bts724g模块硬件初始化函数 
  * @param[in]       无
  * @return          无
***********************************************************************/
void bts724g_init(void)
{
     const struct hcs12_gpio_detail *relay_contrl;
     
#if BMS_SUPPORT_BY5248D == 0 

   #if BMS_SUPPORT_HARDWARE_BEFORE == 1   // yang 20161213
    if(hardware_io_revision_get() >= HW_VER_116)
    {
        pt_relay_input_pins = relay_contrl_v116;
        gpio_init(&hcs12_gpio_ops,&shdn_24v_v116[0].io, GPIO_MODE_OUTPUT_PUSHPULL);
        gpio_set_output(&hcs12_gpio_ops,&shdn_24v_v116[0].io, shdn_24v_v116[0].def_val);
    }
    else
    {
        pt_relay_input_pins = relay_contrl_v114;
    }
#else
    pt_relay_input_pins = relay_contrl_v116;
    (void)gpio_init(&hcs12_gpio_ops,&shdn_24v_v116[0].io, GPIO_MODE_OUTPUT_PUSHPULL);
    (void)gpio_set_output(&hcs12_gpio_ops,&shdn_24v_v116[0].io, shdn_24v_v116[0].def_val);  
#endif  
   
#else        //高精度一体机沿用主机继电器IO
    pt_relay_input_pins = relay_contrl_v116;
    (void)gpio_init(&hcs12_gpio_ops,&shdn_24v_v116[0].io, GPIO_MODE_OUTPUT_PUSHPULL);
    (void)gpio_set_output(&hcs12_gpio_ops,&shdn_24v_v116[0].io, shdn_24v_v116[0].def_val);  
#endif

    for (relay_contrl = pt_relay_input_pins; relay_contrl->name != 0; ++relay_contrl) 
    {
        (void)gpio_init(&hcs12_gpio_ops,&relay_contrl->io, GPIO_MODE_OUTPUT_PUSHPULL);
        (void)gpio_set_output(&hcs12_gpio_ops,&relay_contrl->io, relay_contrl->def_val);
     }
}

/************************************************************************
  * @brief           bts724g模块输出控制函数函数 
  * @param[in]       无
  * @return          无
***********************************************************************/
 void  relay_input_pin_set(unsigned char id, unsigned char value) 
{
    (void)gpio_set_output(&hcs12_gpio_ops,&pt_relay_input_pins[id - 1].io, value);
   //  g_relay_last_tick[id]  = get_tick_count();
}









