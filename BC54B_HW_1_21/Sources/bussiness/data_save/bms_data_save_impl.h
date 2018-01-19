/**
*
* Copyright (c) 2012 Ligoo Inc.
*
* @file bms_data_save_impl.h
* @brief
* @note
* @author
* @date 2012-5-29
*
*/

#ifndef BMS_DATA_SAVE_IMPL_H_
#define BMS_DATA_SAVE_IMPL_H_

#include "bms_data_save.h"




#define RPAGE_RESET() RPAGE = g_data_save_rpage;
#define RPAGE_CALL(VALUE, FUNC) {value = FUNC; RPAGE = g_data_save_rpage;}
#define RPAGE_CALL_WITHOUT_RETURN(FUNC) {FUNC; RPAGE = g_data_save_rpage;}
#define RPAGE_FUNC(RES, FUNC) (RES=FUNC,RPAGE = g_data_save_rpage,RES)
#define DATA_SAVE_SAFE_GET(VAR, FUN) do{RPAGE_SAVE();VAR=FUN;RPAGE_RESTORE();}while(0)
#define CONFIG_CHANGED_DELAY    3   // 延时配置周期，等待配置完全完�?
#define FAULT_CHANGED_DATA_SAVE_COUNT 15

extern INT8U g_data_save_rpage;

#pragma push
#pragma DATA_SEG __RPAGE_SEG PAGED_RAM_SAVE//PAGED_RAM_JOB
extern INT8U g_data_save_buffer[DATA_SAVE_MAX_LINE_SIZE];
#pragma pop

struct DataSaveHandler {
    void (*task)(void *pdata);
    void (*init)(void);
    void (*uninit)(void);
    void (*start)(void);
    void (*stop)(void);
    INT8U (*is_stop)(void);
    DataSaveStatus (*get_status)(void);
    INT32U (*get_counter)(void);
    INT8U (*is_fault)(void);
};
extern void bms_set_nandflash_scan_require(INT32U start_date,INT32U end_date);
extern INT8U bms_get_nandflash_scan_require(void); 

#endif /* BMS_DATA_SAVE_IMPL_H_ */
