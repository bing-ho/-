/**
*
* Copyright (c) 2012 Ligoo Inc.
*
* @file bms_input_signal_impl.h
* @brief
* @note
* @author
* @date 2013-6-25
*
*/

#include "bms_input_signal.h"
#include "gpio_interface.h"

#ifndef BMS_INPUT_SIGNAL_IMPL_H_
#define BMS_INPUT_SIGNAL_IMPL_H_

/*
typedef enum _InputPin
{
    kInputPin_DIGITAL_START ,
    kInputPin_KEY_ON = kInputPin_DIGITAL_START,
    kInputPin_CHARGER_READY ,
    kInputPin_SIG_IN1 ,
    kInputPin_SIG_IN2 ,
    kInputPin_SWITCH_IN1 ,
    kInputPin_SWITCH_IN2 ,
    kInputPin_SWITCH_IN3 ,
    kInputPin_SWITCH_IN4 ,

    BMS_INPUT_SIGNAL_MAX_COUNT,

    kInputPin_SWITCH_A0 = BMS_INPUT_SIGNAL_MAX_COUNT,
    kInputPin_SWITCH_A1 ,
    kInputPin_DTU_STATUS ,
    kInputPin_MAX 
}InputPin;
*/

struct input_signal_reg_info {
    struct hcs12_gpio_detail gpio;
    volatile uint8_t* ctrl_reg_addr;
};

extern INT8U g_input_signal_cnt;
extern const struct input_signal_reg_info* pt_signal_input_pins;

extern void input_signal_init(void);

#endif