/**
*
* Copyright (c) 2012 Ligoo Inc.
*
* @file bms_pflash_mmap.h
* @brief
* @note
* @author Xiangyong Zhang
* @date 2012-8-16
*
*/

#ifndef BMS_PFLASH_MMAP_H_
#define BMS_PFLASH_MMAP_H_

#include "bms_defs.h"
#include "bms_business_defs.h"
#include "bms_bcu.h"


typedef struct MMapContext* mmap_t;

mmap_t mmap_open(INT32U address);
Result mmap_write(mmap_t mmap, PINT8U buffer, INT16U size);
INT16U mmap_size(mmap_t mmap);
void mmap_close(mmap_t mmap);

INT8U* _PAGED mmap_flash_to_logical_address(INT32U flash_address);



#endif /* BMS_PFLASH_MMAP_H_ */
