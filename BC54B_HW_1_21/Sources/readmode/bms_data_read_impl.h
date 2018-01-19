/**
* Copyright (c) 2012 Ligoo Inc.
*
* @file bms_data_read_impl.h
* @brief ¶ÁnandflashÊý¾Ý
* @note
* @author Bing He
* @date 2017-10-24
*
*/

#ifndef BMS_DATA_READ_IMPL_H_
#define BMS_DATA_READ_IMPL_H_

#include "bms_data_save.h"
#include "nandflash_hardware.h"
#include "nandflash_intermediate.h"
#include "nandflash_interface.h"

#pragma push
#pragma DATA_SEG __RPAGE_SEG PAGED_RAM_SAVE
extern INT8U g_data_save_buffer[DATA_SAVE_MAX_LINE_SIZE];
#pragma pop

extern INT8U g_data_scan_require;
extern INT32U g_start_date;
extern INT32U g_end_date;

#define INVALID_ID      0x00000000UL
#define IDLE_ID         0xFFFFFFFFUL
#define ntohl(a)        a

INT8U page_addr_extract(const struct StorageObject *__FAR obj, INT32U start_required_date, INT32U stop_required_date);

INT32U bms_get_nand_start_page_addr(void);
INT32U bms_get_nand_stop_page_addr(void);

void bms_set_nandflash_scan_require(INT32U start_date,INT32U end_date);
INT8U bms_get_nandflash_scan_require(void);


#endif