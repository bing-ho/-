/**
 *
 * Copyright (c) 2012 Ligoo Inc.
 *
 * @file bms_precharge_with_tv.h
 * @brief
 * @note
 * @author Liwei Dong
 * @date 2015-2-2
 *
 */

#ifndef BMS_PRECHARGE_WITH_TV_H_
#define BMS_PRECHARGE_WITH_TV_H_

#include "includes.h"
#include "bms_bcu.h"

void bms_precharge_with_tv_finish_check(void);
INT8U bms_precharge_with_retry_is_fail(void);
void bms_precharge_state_reset(void);

#endif  /* BMS_PRECHARGE_WITH_TV_H_ */