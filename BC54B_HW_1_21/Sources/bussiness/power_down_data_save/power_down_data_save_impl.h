/**
 *
 * Copyright (c) 2012 Ligoo Inc.
 *
 * @file power_down_data_save_impl.h
 * @brief
 * @note
 * @author kai.lv
 * @date 2015-11-6
 *
 */
#ifndef POWER_DOWN_DATA_SAVE_IMPL_H_
#define POWER_DOWN_DATA_SAVE_IMPL_H_


#include "bms_eeprom_impl.h"
#include "bms_config.h"
#include "power_down_data_save.h"
#include "bms_eeeprom.h"

#define POWER_DOWN_EEEPROM_STORE_CAP_DIFF_MAX   1800000 //Ams 与实际容量最大差值

void power_down_data_save_init(void);

#endif 