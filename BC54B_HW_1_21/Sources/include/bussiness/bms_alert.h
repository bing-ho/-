/**
*
* Copyright (c) 2012 Ligoo Inc.
*
* @file bms_alert.h
* @brief
* @note
* @author
* @date 2012-5-31
*
*/

#ifndef BMS_ALERT_H_
#define BMS_ALERT_H_
#include "bms_defs.h"
#include "includes.h"
#include "bms_system.h"
#include "bms_job.h"
#include "bms_config.h"

void alert_init(void);
void alert_unit(void);

void alert_enable(void);
void alert_disable(void);
INT8U alert_is_enabled(void);

void alert_long_beep(INT16U time);

void alert_beep_on(INT32U type);
void alert_beep_off(INT32U type);



#endif /* BMS_ALERT_H_ */
