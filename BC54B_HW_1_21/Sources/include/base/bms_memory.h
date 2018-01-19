/**
*
* Copyright (c) 2012 Ligoo Inc.
*
* @file bms_memory.h
* @brief
* @note
* @author Xiangyong Zhang
* @date 2012-9-22
*
*/

#ifndef BMS_MEMORY_H_
#define BMS_MEMORY_H_
#include <stdlib.h>
#include "bms_defs.h"

#ifndef BMS_SUPPORT_FIX_MEMORY
#define BMS_SUPPORT_FIX_MEMORY      1
#endif

#define BMS_MALLOC_FIX_MEMORY_SIZE   508

#define MALLOC bms_malloc
#define FREE   bms_free

void* bms_malloc(int size);
void bms_free(void* ptr);
INT16U bms_malloc_size(void);


#endif /* BMS_MEMORY_H_ */
