/**
*
* Copyright (c) 2012 Ligoo Inc.
*
* @file bms_bcu_reset_impl.c
* @brief
* @note
* @author Liwei Dong
* @date 2016-1-7
*
*/

#include "bms_bcu_reset_impl.h"

static INT8U g_bcu_reset_command = 0; //∏¥Œª√¸¡Ó

void bcu_reset_init(void)
{
    
}

void bcu_reset_uninit(void)
{
    
}

void bcu_start_reset(void)
{
    g_bcu_reset_command = TRUE;
    bms_system_protect();
}

void bcu_stop_reset(void)
{
    g_bcu_reset_command = FALSE;
    bms_system_unprotect();
}

INT8U bcu_reset_is_start(void)
{
    return g_bcu_reset_command;
}

void bcu_reset_check(void)
{
    if(g_bcu_reset_command == TRUE)
    {
        if(bms_system_power_off_is_allowed())
        {
            power_down_store_data();        
#if 0//BMS_SUPPORT_DATA_SAVE
            if(data_save_get_sd_write_state() == kSDWriteWriting) //check sd is stop
                data_save_set_sd_write_state(kSDWriteDisable);
#endif
            mcu_reset();
        }
    }
}