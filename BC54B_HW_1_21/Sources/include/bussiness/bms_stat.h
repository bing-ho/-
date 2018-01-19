/**
*
* Copyright (c) 2012 Ligoo Inc.
*
* @file bms_stat.h
* @brief
* @note
* @author Xiangyong Zhang
* @date 2012-10-8
*
*/

#ifndef BMS_STAT_H_
#define BMS_STAT_H_
#include "bms_defs.h"

extern INT8U g_bcu_system_time_upate_flag; //从时钟芯片更新系统时间标志

void stat_init(void);

#endif /* BMS_STAT_H_ */
