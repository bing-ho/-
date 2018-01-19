/**
 *
 * Copyright (c) 2012 Ligoo Inc.
 *
 * @file bms_work_state_impl.h
 * @brief
 * @note
 * @author
 * @date 2014-10-21
 *
 */

#ifndef BMS_WORK_STATE_IMPL_H_
#define BMS_WORK_STATE_IMPL_H_

#include "bms_work_state.h"


void diagnose_work_state(void* data);
void bms_power_down_protect_rel_check(void);
#if( SWITCH_ON == UDS_SWITCH_CONFIG)
INT8U bms_get_power_state(void);
#endif
#endif /* BMS_WORK_STATE_IMPL_H_ */