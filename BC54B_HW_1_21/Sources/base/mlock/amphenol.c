/**
 *
 * Copyright (c) 2016 Ligoo Inc.
 *
 * @file bms_mlock_impl.c
 * @brief
 * @note
 * @author
 * @date 2016-9-26
 *
 */

#include "bms_mlock.h"
#include "bms_relay.h"
#include "test_io.h"
#include "bms_config.h"


#pragma MESSAGE DISABLE C5703

#define MIN_PULL_UP_TIME         200 //ms
#define MAX_PULL_UP_TIME         800 //ms
#define PULL_UP_DELAY_FLAG      0x80
#define LOCK_ON                 0x00 //DO1/DO2 输入反向
#define LOCK_OFF                0x01


#define LOCK_IO_NAME        "DO1"
#define UNLOCK_IO_NAME      "DO2"
#define STATUS_IO_NAME      "SW_IN1"

ChgLock_StateType mlock_amphenol_get_status(void);


static void mlock_reset_io_status( void* user_data)
{
    (void*)user_data;
    output_ctrl_set_with_name(LOCK_IO_NAME,   LOCK_OFF);
    output_ctrl_set_with_name(UNLOCK_IO_NAME, LOCK_OFF);
}

void mlock_amphenol_uninit(void)
{
  mlock_reset_io_status(NULL);
}

void mlock_amphenol_init(void* pvdata)
{
}

Result mlock_amphenol_lock(void)   //加锁进程
{
    ChgLock_ConfigType config=chglock_config_get();
    INT16U lock_on_time =config.lock_time;// g_ChgLock_Config.lock_time;//config_get(kMlockParam1); //从EEPROM中获取加锁时间由上位机配置过来

    if(lock_on_time > MAX_PULL_UP_TIME)
    {
        lock_on_time = MAX_PULL_UP_TIME;
    }
    else if(lock_on_time < MIN_PULL_UP_TIME)
    {
        lock_on_time = MIN_PULL_UP_TIME;
    }
    chglock_get_end_timer(lock_on_time); //mlock_timer_end_tick = OSTimeGet() + lock_on_time;
    if(output_ctrl_set_with_name(LOCK_IO_NAME, LOCK_ON) == RES_FALSE)//判断IO口配置是否成功 ，低电平有效 所以 LOCK_ON 配置为00
    {
        return 0;
    }
    return 1;
}

Result mlock_amphenol_unlock(void)
{
    ChgLock_ConfigType config=chglock_config_get();
    INT16U lock_off_time =config.unlock_time;// g_ChgLock_Config.unlock_time;//config_get(kMlockParam2);

    if(lock_off_time > MAX_PULL_UP_TIME)
    {
        lock_off_time = MAX_PULL_UP_TIME;
    }
    else if(lock_off_time < MIN_PULL_UP_TIME)
    {
        lock_off_time = MIN_PULL_UP_TIME;
    }
    chglock_get_end_timer(lock_off_time);//mlock_timer_end_tick = OSTimeGet()+ lock_off_time;
    if(output_ctrl_set_with_name(UNLOCK_IO_NAME, LOCK_ON) == RES_FALSE)
    {
        return 0;
    }
    return 1;
}

//电子锁状态
ChgLock_StateType mlock_amphenol_get_status(void)
{
    ChgLock_StateType ret = chglock_get_logic_state();
    //INT8U feedback =g_ChgLock_Config.feedback_state;// (INT8U)config_get(kMlockParam3);

    if(input_signal_is_high_by_name("SW_IN1"))
    {
        //ret = feedback?kChgUnLock:kChgLock;
        ret=kChgLock;
    }
    else
    {
        //ret = feedback?kChgLock:kChgUnLock; 
        ret=kChgUnLock;
    }
    return ret;
}
