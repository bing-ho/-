/**
*
* Copyright (c) 2012 Ligoo Inc.
*
* @file bms_data_save.h
* @brief
* @note
* @author
* @date 2012-5-29
*
*/

#ifndef BMS_DATA_SAVE_H_
#define BMS_DATA_SAVE_H_
#include "bms_defs.h"
#include "bms_bcu.h"
#include "bms_system.h"
#include "includes.h"
#include "bms_clock.h"
#include "diskio.h"
#include "bms_util.h"
#include "bms_log.h"
#include "ff.h"
#include "bms_event.h"
#include "bms_config.h"

void data_save_init(void);
void data_save_uninit(void);

typedef enum
{
    kSDWriteEnable=0,
    kSDWriteWriting,
    kSDWriteDisable
}SDWriteState;

typedef enum
{
    kDataSaveStatusNotAvaliable,
    kDataSaveStatusNormal,
    kDataSaveStatusStopped,
}DataSaveStatus;

void data_save_start(void);
void data_save_stop(void);
INT8U data_save_is_stop(void);
DataSaveStatus data_save_get_status(void);
INT8U data_save_is_sd_fault(void);
INT32U data_save_get_counter(void);
void data_save_sd_write_state_init(void);
SDWriteState data_save_get_sd_write_state(void);
void data_save_set_sd_write_state(SDWriteState state);
void set_data_save_request_stopped(INT8U request_stop);

#endif /* BMS_DATA_SAVE_H_ */
