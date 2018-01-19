/**
 *
 * Copyright (c) 2012 Ligoo Inc.
 *
 * @file bms_input_signal_impl.c
 * @brief
 * @note
 * @author
 * @date 2013-6-25
 *
 */

#include "bms_input_signal_impl.h"
#include "gpio_interface.h"      
#include "bms_bcu.h"


#define STRING_KEY_ON       "KEY_ON"
#define STRING_CHR_RDY      "CHR_RDY"
#define STRING_SIG_IN1      "SIG_IN1"
#define STRING_SIG_IN2      "SIG_IN2"
#define STRING_SIG_IN3      "SIG_IN3"
#define STRING_WKUP_INH     "WKUP_INH"
#define STRING_ST12         "ST12"
#define STRING_ST34         "ST34"
#define STRING_SW_IN1       "SW_IN1"
#define STRING_SW_IN2       "SW_IN2"
#define STRING_SW_IN3       "SW_IN3"
#define STRING_SW_IN4       "SW_IN4"
#define STRING_MCU_ZD       "MCU_ZD"
#define STRING_RTC_INT      "RTC_INT"
#define STRING_GSM_ID       "GSM_ID"
#define STRING_MFAULT       "MFAULT"//////////2017年2月15日sunyan添加

const struct input_signal_reg_info signal_input_pins_v114[] = {
    { STRING_KEY_ON,     { &PT1AD0,  &PT1AD0,    &DDR1AD0,   1 << 4 }, 0, 0, &ATD0DIENL},
    { STRING_CHR_RDY,    { &PT0AD0,  &PT0AD0,    &DDR0AD0,   1 << 3 }, 0, 0, &ATD0DIENH},
    { STRING_SIG_IN1,    { &PT1AD0,  &PT1AD0,    &DDR1AD0,   1 << 3 }, 0, 0, &ATD0DIENL},
    { STRING_SIG_IN2,    { &PT0AD0,  &PT0AD0,    &DDR0AD0,   1 << 2 }, 0, 0, &ATD0DIENH},
    { STRING_SW_IN1,     { &PT0AD0,  &PT0AD0,    &DDR0AD0,   1 << 0 }, 0, 0, &ATD0DIENH},
    { STRING_SW_IN2,     { &PT1AD0,  &PT1AD0,    &DDR1AD0,   1 << 1 }, 0, 0, &ATD0DIENL},
    { STRING_SW_IN3,     { &PT1AD0,  &PT1AD0,    &DDR1AD0,   1 << 2 }, 0, 0, &ATD0DIENL},
    { STRING_SW_IN4,     { &PT0AD0,  &PT0AD0,    &DDR0AD0,   1 << 1 }, 0, 0, &ATD0DIENH},
    { STRING_MCU_ZD,     { &PTIP,    &PTP,       &DDRP,      1 << 6 }, 0, 0, NULL},
    { STRING_RTC_INT,    { &PTIP,    &PTP,       &DDRP,      1 << 7 }, 0, 0, NULL},
    { STRING_GSM_ID,     { &PORTA,   &PORTA,     &DDRA,      1 << 2 }, 0, 0, NULL}
};

const struct input_signal_reg_info signal_input_pins_v116[] = {
    { STRING_KEY_ON,     { &PT1AD1,  &PT1AD1,    &DDR1AD1,   1 << 4}, 0 , 0, &ATD1DIENL}, 
    { STRING_CHR_RDY,    { &PT1AD1,  &PT1AD1,    &DDR1AD1,   1 << 1}, 0 , 0, &ATD1DIENL},
    { STRING_SIG_IN1,    { &PTIP,    &PTP,       &DDRP,      1 << 2}, 0 , 0, NULL},
    { STRING_SIG_IN2,    { &PTIT,    &PTT,       &DDRT,      1 << 3}, 0 , 0, NULL},
    { STRING_SW_IN1,     { &PORTD,   &PORTD,     &DDRD,      1 << 6}, 0 , 0, NULL},
    { STRING_SW_IN2,     { &PT0AD0,  &PT0AD0,    &DDR0AD0,   1 << 2}, 0 , 0, &ATD0DIENH},
    { STRING_SW_IN3,     { &PT0AD0,  &PT0AD0,    &DDR0AD0,   1 << 3}, 0 , 0, &ATD0DIENH},
    { STRING_SW_IN4,     { &PT1AD0,  &PT1AD0,    &DDR1AD0,   1 << 3}, 0 , 0, &ATD0DIENL},
    { STRING_MCU_ZD,     { &PTIP,    &PTP,       &DDRP,      1 << 6}, 0,  0, NULL},
    { STRING_RTC_INT,    { &PTIP,    &PTP,       &DDRP,      1 << 7}, 0,  0, NULL},
    { STRING_GSM_ID,     { &PORTA,   &PORTA,     &DDRA,      1 << 1}, 0,  0, NULL}
};

const struct input_signal_reg_info signal_input_pins_v120[] = {
    { STRING_KEY_ON,     { &PT1AD1,  &PT1AD1,    &DDR1AD1,   1 << 4}, 0 , 0, &ATD1DIENL},
    { STRING_CHR_RDY,    { &PT1AD1,  &PT1AD1,    &DDR1AD1,   1 << 1}, 0 , 0, &ATD1DIENL},
    { STRING_SIG_IN1,    { &PTIP,    &PTP,       &DDRP,      1 << 2}, 0 , 0, NULL},
    //{ STRING_SIG_IN2,    { &PTIT,    &PTT,       &DDRT,      1 << 3}, 0 , 0, NULL},        //removed from v120
    { STRING_SW_IN1,     { &PORTD,   &PORTD,     &DDRD,      1 << 5}, 0 , 0, NULL},          //v116: PD6   --> v120: PD5, 
    { STRING_SW_IN2,     { &PORTD,   &PORTD,     &DDRD,      1 << 6}, 0 , 0, NULL},          //v116: PAD10 --> v120: PD6
    { STRING_MCU_ZD,     { &PTIP,    &PTP,       &DDRP,      1 << 6}, 0 , 0, NULL},
    { STRING_RTC_INT,    { &PTIP,    &PTP,       &DDRP,      1 << 7}, 0 , 0, NULL},
    { STRING_GSM_ID,     { &PORTA,   &PORTA,     &DDRA,      1 << 1}, 0 , 0, NULL},
    { STRING_MFAULT,     { &PORTA,   &PORTA,     &DDRA,      1 << 6}, 0,  0, NULL}   ///////////2017年12月5日sunyan将此引脚改为输入模式
};


const struct input_signal_reg_info signal_input_pins_byu_v110[] = {
  	{ STRING_KEY_ON,   { &PT1AD1,  &PT1AD1,    &DDR1AD1,   1 << 4}, 0 , 0, &ATD1DIENL},
    { STRING_CHR_RDY,  { &PT1AD1,  &PT1AD1,    &DDR1AD1,   1 << 1}, 0 , 0, &ATD1DIENL}, 
    { STRING_SIG_IN1,  { &PORTD,   &PORTD,     &DDRD,      1 << 5}, 0 , 0, NULL},     
    { STRING_SIG_IN2,  { &PORTD,   &PORTD,     &DDRD,      1 << 6}, 0 , 0, NULL},     
    { STRING_SIG_IN3,  { &PORTD,   &PORTD,     &DDRD,      1 << 7}, 0 , 0, NULL},     
    { STRING_WKUP_INH, { &PORTK,   &PORTK,     &DDRK,      1 << 0}, 0 , 0, NULL},
    { STRING_ST12,     { &PTIT,    &PTT,       &DDRT,      1 << 4}, 0 , 0, NULL},
    { STRING_ST34,     { &PTIT,    &PTT,       &DDRT,      1 << 5}, 0 , 0, NULL}            
};


INT8U g_input_signal_cnt = 0;
const struct input_signal_reg_info* pt_signal_input_pins;

const struct input_signal_reg_info*  all_ver_input_signal_list[] = 
{
#if BMS_SUPPORT_BY5248D == 0   //主机

#if BMS_SUPPORT_HARDWARE_BEFORE == 1   // yang 20161213
    signal_input_pins_v114,
    signal_input_pins_v114,
    signal_input_pins_v116,
#endif
    signal_input_pins_v120
    
#else         //一体机模式不支持114
    signal_input_pins_byu_v110    
#endif
};


#define IS_VALID_ID(id) ((id) <= g_input_signal_cnt && (id) > 0)


#if BMS_SUPPORT_INPUT_SIGNAL
void input_signal_init(void) 
{
    uint8_t i;

    PUCR_PUPAE = 1;         /* Pull-up Port A Enable */

#if BMS_SUPPORT_BY5248D == 0   //主机

  #if BMS_SUPPORT_HARDWARE_BEFORE == 1   // yang 20161213
    pt_signal_input_pins = all_ver_input_signal_list[hardware_io_revision_get()];
    if(hardware_io_revision_get() == HW_VER_120)
    {
        g_input_signal_cnt = 5;//////////////////////////
    }
    else
    {
        g_input_signal_cnt = 10;//BMS_INPUT_SIGNAL_MAX_COUNT;
    }
  #else
    pt_signal_input_pins = all_ver_input_signal_list[0];
    g_input_signal_cnt = 5;
  #endif
    
#else      //一体机模式不支持114

    pt_signal_input_pins = all_ver_input_signal_list[0];
    g_input_signal_cnt = 5;
#endif
    
    /*if(hardware_io_revision_get() == HW_VER_116)
    {
        pt_signal_input_pins = signal_input_pins_v116;
        //TO use the digital input function on Port AD
        ATD1DIENL_IEN4 = 1; //KEY_ON
        ATD1DIENL_IEN1 = 1; //CHARGER_READY
        ATD0DIENH_IEN10 = 1; //SWITCH_IN2
        ATD0DIENH_IEN11 = 1;//SWITCH_IN3
        ATD0DIENL_IEN3 = 1; //SWITCH_IN4
    }
    else
    {
        pt_signal_input_pins = signal_input_pins_v114;
    //TO use the digital input function on Port AD
    ATD0DIENL_IEN4 = 1; //KEY_ON
    ATD0DIENH_IEN11 = 1; //CHARGER_READY
    ATD0DIENL_IEN3 = 1;//SIG_IN1
    ATD0DIENH_IEN10 = 1; //SIG_IN2
    ATD0DIENH_IEN8 = 1;//SWITCH_IN1
    ATD0DIENL_IEN1 = 1; //SWITCH_IN2
    ATD0DIENL_IEN2 = 1;//SWITCH_IN3
    ATD0DIENH_IEN9 = 1; //SWITCH_IN4
    }*/
    
    for (i=0; i<g_input_signal_cnt; ++i) 
    {
        if(pt_signal_input_pins[i].ctrl_reg_addr != NULL)
        {
            *pt_signal_input_pins[i].ctrl_reg_addr |= pt_signal_input_pins[i].gpio.io.bit;
        }
        (void)hcs12_gpio_init(&pt_signal_input_pins[i].gpio.io, GPIO_MODE_INPUT);
    }
    //////////////////////////////////////////////////////////////////////////////
    
#if BMS_SUPPORT_BY5248D == 0   //主机
    for (i=g_input_signal_cnt; i<9; ++i) 
    {
        if(pt_signal_input_pins[i].ctrl_reg_addr != NULL)
        {
            *pt_signal_input_pins[i].ctrl_reg_addr |= pt_signal_input_pins[i].gpio.io.bit;
        }
        (void)hcs12_gpio_init(&pt_signal_input_pins[i].gpio.io, GPIO_MODE_INPUT);
    }
 #else
    for (i=g_input_signal_cnt; i<8; ++i) 
    {
        if(pt_signal_input_pins[i].ctrl_reg_addr != NULL)
        {
            *pt_signal_input_pins[i].ctrl_reg_addr |= pt_signal_input_pins[i].gpio.io.bit;
        }
        (void)hcs12_gpio_init(&pt_signal_input_pins[i].gpio.io, GPIO_MODE_INPUT);
    }
#endif
    /////////////////////////////////////////////////////////////////////////////////
    
}

INT8U input_signal_is_high_by_name(char *name)     //判断输入信号是否为高电平
{
    uint8_t i;
    for(i=0; i<g_input_signal_cnt; i++)
    {
        if (strcmp(pt_signal_input_pins[i].gpio.name, name) == 0)
        {
            return hcs12_gpio_input_is_high(&pt_signal_input_pins[i].gpio.io);
        }
    
     }
    /////////////////////////////////////////////////////////////////////////////////////
    
#if BMS_SUPPORT_BY5248D == 0   //主机
    for(i=g_input_signal_cnt; i<9; i++)
    {
        if (strcmp(pt_signal_input_pins[i].gpio.name, name) == 0)
        {
            return hcs12_gpio_input_is_high(&pt_signal_input_pins[i].gpio.io);
        }
    
     }
#else
    for (i=g_input_signal_cnt; i<8; ++i) 
    {
        if(pt_signal_input_pins[i].ctrl_reg_addr != NULL)
        {
            *pt_signal_input_pins[i].ctrl_reg_addr |= pt_signal_input_pins[i].gpio.io.bit;
        }
        (void)hcs12_gpio_init(&pt_signal_input_pins[i].gpio.io, GPIO_MODE_INPUT);
    }
#endif
    //////////////////////////////////////////////////////////////////////////////////////
        return 0;
    }

INT8U input_signal_rtc_interrupt_is_assert(void) 
{
    return !input_signal_is_high_by_name("RTC_INT");
}

INT8U input_signal_is_high(INT8U id) {
    if (!IS_VALID_ID(id)) {
        return 0;
    }

    return hcs12_gpio_input_is_high(&pt_signal_input_pins[id - 1].gpio.io);
}

INT8U input_signal_is_low(INT8U id) {
    if (!IS_VALID_ID(id)) {
        return 0;
    }

    return !hcs12_gpio_input_is_high(&pt_signal_input_pins[id - 1].gpio.io);
    }

INT8U input_signal_get_count(void) 
{
    return g_input_signal_cnt;
}

#endif
