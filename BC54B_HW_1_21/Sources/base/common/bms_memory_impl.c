/**
*
* Copyright (c) 2012 Ligoo Inc.
*
* @file bms_memory_impl.c
* @brief
* @note
* @author
* @date 2012-9-22
*
*/
#include "bms_memory.h"
#include "bms_log.h"

// if the optimization is enabled, free memory is invalid.

#pragma MESSAGE DISABLE C4001 // Condition always FALSE

INT16U g_bms_malloc_memory_size = 0;

#if BMS_SUPPORT_FIX_MEMORY
static INT8U g_bms_memory_block[BMS_MALLOC_FIX_MEMORY_SIZE];
#endif


void* bms_malloc(int size)
{
#if BMS_SUPPORT_FIX_MEMORY
    INT8U* ptr = NULL;
    OS_INIT_CRITICAL();

    OS_ENTER_CRITICAL();
    if (g_bms_malloc_memory_size + size <= BMS_MALLOC_FIX_MEMORY_SIZE)
    {
        ptr = g_bms_memory_block + g_bms_malloc_memory_size;
        g_bms_malloc_memory_size += size;
    }
    OS_EXIT_CRITICAL();
    BMS_ASSERT(ptr != NULL);

    return ptr;
#else
    void* ptr = malloc(size);
    g_bms_malloc_memory_size += size;
    return ptr;
#endif
}

void bms_free(void* ptr)
{
#if BMS_SUPPORT_FIX_MEMORY
    BMS_ASSERT(ptr == NULL);
    UNUSED(ptr);    
#else
    free(ptr);
#endif
}


INT16U bms_malloc_size(void)
{
    return g_bms_malloc_memory_size;
}


