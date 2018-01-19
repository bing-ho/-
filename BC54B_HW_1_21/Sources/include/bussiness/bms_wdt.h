/**
*
* Copyright (c) 2012 Ligoo Inc.
*
* @file bms_wdt.h
* @brief
* @note
* @author
* @date 2012-5-31
*
*/

#ifndef BMS_WDT_H_
#define BMS_WDT_H_

#include "bms_main_job.h"

void watch_dog_init(void);
void watch_dog_unit(void);
void watch_dog_run(void);

void watch_dog_register(INT8U id, INT8U timeout);
void watch_dog_feed(INT8U id);

void watch_dog_disable(void);
void watch_dog_enable(void);


#endif /* BMS_WDT_H_ */
