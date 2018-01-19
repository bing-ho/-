/**
 *
 * Copyright (c) 2011 Ligoo Inc.
 *
 * @file  pwm_ect.h
 * @brief pwm信号检测驱动头文件
 * @note 
 * @version 1.00
 * @author
 * @date 2014/3/17
 *
 */

#include "includes.h"

#ifndef PWM_ECT_H_
#define PWM_ECT_H_

void pwm_ect_init(void);
void pwm_ect_isr_hook(void);
INT16U pwm_ect_get_timer_cnt(void);


#endif