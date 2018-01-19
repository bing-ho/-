/**
*
* Copyright (c) 2012 Ligoo Inc.
*
* @file bms_pflash_mmap_impl.h
* @brief
* @note
* @author Xiangyong Zhang
* @date 2012-8-16
*
*/

#ifndef BMS_PFLASH_MMAP_IMPL_H_
#define BMS_PFLASH_MMAP_IMPL_H_

#include "bms_pflash_mmap.h"

#define PFLASH_WRITE_BLOCK_SIZE 8

typedef struct
{
    INT32U block_address;
    INT16U block_current_pos;

    INT16U total_write_size;

    INT8U  buffer[PFLASH_WRITE_BLOCK_SIZE];
    INT8U  buffer_pos;

    INT32U last_sector;
}MMapContext, * mmap_t;

Result mmap_write_block(mmap_t mmap, INT8U* buffer, INT8U size);
Result mmap_write_tail(mmap_t mmap);

#endif /* BMS_PFLASH_MMAP_IMPL_H_ */
