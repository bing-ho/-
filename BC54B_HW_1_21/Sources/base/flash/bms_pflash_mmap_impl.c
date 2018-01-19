/**
*
* Copyright (c) 2012 Ligoo Inc.
*
* @file bms_pflash_mmap_impl.c
* @brief
* @note
* @author Xiangyong Zhang
* @date 2012-8-16
*
*/
#include "bms_pflash_mmap_impl.h"
#include "Types.h"
#include "PFlash.h"
#include "bms_memory.h"
#include "bms_util.h"

#pragma MESSAGE DISABLE C1420 // Result of function-call is ignored

#define MMAP_SINGLE_INSTANCE  1

#define RULE_FLASH_START_ADDRESS 0x7F0000UL
#define RULE_FLASH_BLOCK_SIZE    0x4000

#define LOGICAL_START_BLOCK 0xE0
#define LOGICAL_OFFSET_ADDRESS 0x008000

static MMapContext g_mmap_context = {0, 0};

//static INT8U g_mmap_block_buffer[PFLASH_WRITE_BLOCK_SIZE];

mmap_t mmap_open(INT32U address)
{
    MMapContext* context = &g_mmap_context;

    if (context == NULL || context->block_address) return NULL;

    memset(context, 0, sizeof(MMapContext));

    context->block_address = address;
    context->block_current_pos = 0;

    return context;
}

Result mmap_write(mmap_t mmap, PINT8U buffer, INT16U size)
{
    INT16U  write_size;
    INT16U pos = 0;
    Result res = RES_OK;
    INT16U remain_size = size;

    if (mmap == NULL) return ERR_INVALID_ARG;

    while (remain_size > 0)
    {
        write_size = PFLASH_WRITE_BLOCK_SIZE - mmap->buffer_pos;
        if (remain_size < write_size) write_size = remain_size;
        remain_size -= write_size;

#if 0
        if (mmap->buffer_pos == 0 && write_size == PFLASH_WRITE_BLOCK_SIZE)
        {
            /* Directly write */
            safe_memcpy((PINT8U)g_mmap_block_buffer, buffer + pos, write_size);
            res = mmap_write_block(mmap, g_mmap_block_buffer, write_size);
            if (res != RES_OK) break;

            pos += write_size;
            continue;
        }
#endif

        safe_memcpy((PINT8U)mmap->buffer + mmap->buffer_pos, buffer + pos, write_size);
        pos += write_size;
        mmap->buffer_pos += write_size;

        if (mmap->buffer_pos >= PFLASH_WRITE_BLOCK_SIZE)
        {
            res = mmap_write_block(mmap, mmap->buffer, mmap->buffer_pos);
            if (res != RES_OK) 
            {
                bcu_flag_sys_exception_flags(SYS_EXCEPTION_FLASH_ERROR);
                break;
            }

            mmap->buffer_pos = 0;
        }
    }

    mmap->total_write_size += pos;

    return res;
}

void mmap_close(mmap_t mmap)
{
    if (mmap == NULL) return;

    mmap_write_tail(mmap);
    mmap->block_address = 0;
    mmap->block_current_pos = 0;
}

INT16U mmap_size(mmap_t mmap)
{
    if (mmap == NULL) return 0;
    return mmap->block_current_pos;
}

Result mmap_write_block(mmap_t mmap, INT8U* buffer, INT8U size)
{
    INT8U err;
    INT32U sector;
    OS_INIT_CRITICAL();

    if (size != PFLASH_WRITE_BLOCK_SIZE) return RES_ERROR;

    /* Check the flash and format it if needed*/
    sector = (mmap->block_address + mmap->block_current_pos) / 0x0400;
    if (sector != mmap->last_sector)
    {
        OS_ENTER_CRITICAL();
        err = PFlash_EraseSector(sector * 0x0400);
        OS_EXIT_CRITICAL();
        if (err != 0) return RES_ERROR;

        mmap->last_sector = sector;
    }

    OS_ENTER_CRITICAL();
    err = PFlash_Program(mmap->block_address + mmap->block_current_pos, (UINT16*)buffer, size);
    OS_EXIT_CRITICAL();

    if (err == 0) mmap->block_current_pos += size;

    return (err == 0) ? RES_OK : RES_ERROR;
}

Result mmap_write_tail(mmap_t mmap)
{
    Result res;
    if (mmap->buffer_pos == 0) return RES_OK;

    for (; mmap->buffer_pos < PFLASH_WRITE_BLOCK_SIZE; mmap->buffer_pos++)
    {
        mmap->buffer[mmap->buffer_pos] = 0xFF;
    }

    res = mmap_write_block(mmap, mmap->buffer, PFLASH_WRITE_BLOCK_SIZE);
    if (res == 0) mmap->buffer_pos = 0;

    return res;
}

INT8U* _PAGED mmap_flash_to_logical_address(INT32U flash_address)
{
    //INT32U ref_address;
    //INT32U logical_address;

    return (INT8U* _PAGED)flash_address;

/*
    if (flash_address < RULE_FLASH_START_ADDRESS) return NULL;
    ref_address = flash_address - RULE_FLASH_START_ADDRESS;
    logical_address = (((ref_address / RULE_FLASH_BLOCK_SIZE) + LOGICAL_START_BLOCK) << 16) + LOGICAL_OFFSET_ADDRESS
            + (ref_address % RULE_FLASH_BLOCK_SIZE);

    return (INT8U* _PAGED)logical_address;
*/
}

