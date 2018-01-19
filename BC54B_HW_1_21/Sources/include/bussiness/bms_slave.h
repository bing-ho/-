/**
 *
 * Copyright (c) 2012 Ligoo Inc.
 *
 * @file bms_slave.h
 * @brief
 * @note
 * @author
 * @date 2012-5-11
 *
 */

#ifndef BMS_SLAVE_H_
#define BMS_SLAVE_H_
#include "bms_defs.h"
#include "bms_buffer.h"
#include "includes.h"
#include "bms_system.h"
#include "bms_log.h"
#include "bms_util.h"
#include "bms_wdt.h"
#include "bms_charger_common.h"
#include "bms_slave_net.h"

/**********************************************
 *
 * Slave Public Function
 *
 ***********************************************/
void slave_init(void);
void slave_uninit(void);

#endif /* BMS_SLAVE_H_ */
