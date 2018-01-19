/**
 *
 * Copyright (c) 2012 Ligoo Inc.
 *
 * @file bms_input_signal.h
 * @brief
 * @note
 * @author
 * @date 2013-6-25
 *
 */
#ifndef BMS_INPUT_SIGNAL_H_
#define BMS_INPUT_SIGNAL_H_

#include "includes.h"

#ifndef BMS_SUPPORT_INPUT_SIGNAL
#define BMS_SUPPORT_INPUT_SIGNAL    1
#endif

void input_signal_init(void);
INT8U input_signal_is_high(INT8U id);
INT8U input_signal_is_low(INT8U id);
INT8U input_signal_get_count(void);
INT8U input_signal_rtc_interrupt_is_assert(void);
 INT8U input_signal_is_high_by_name(char *name) ;

#endif