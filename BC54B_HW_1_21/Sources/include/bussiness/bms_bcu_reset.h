/**
 *
 * Copyright (c) 2012 Ligoo Inc.
 *
 * @file bms_bcu_reset.h
 * @brief
 * @note
 * @author Liwei Dong
 * @date 2016-1-7
 *
 */

#ifndef BMS_BCU_RESET_H_
#define BMS_BCU_RESET_H_

#include "bms_work_state.h"

void bcu_reset_init(void);
void bcu_reset_uninit(void);
void bcu_start_reset(void);
void bcu_stop_reset(void);
INT8U bcu_reset_is_start(void);
void bcu_reset_check(void);

#endif //BMS_BCU_RESET_H_