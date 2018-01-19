/**
 *
 * Copyright (c) 2012 Ligoo Inc.
 *
 * @file read_mode.c
 * @brief
 * @note
 * @author
 * @date 2017-11-15
 *
 */    
#include "includes.h"
#include "j1939_poll.h"
#include "bms_data_save.h"
#include "test_io.h"

#define BMS_GLOBALS

#pragma MESSAGE DISABLE C1420
#pragma MESSAGE DISABLE C4200
#pragma MESSAGE DISABLE C2705
#pragma MESSAGE DISABLE C5919
#pragma MESSAGE DISABLE C12056

void read_mode_run(void)      //系统初始化
{
    OS_CPU_SR cpu_sr = 0;

    clock_init();

    //config_init();
    
    output_ctrl_init();

    bcu_init();

#if BMS_SUPPORT_DATA_SAVE
    data_save_init();
#endif

#if BMS_SUPPORT_UPPER_COMPUTER
    upper_computer_init();
#endif

#if BMS_SUPPORT_J1939_COMM
    j1939_poll_task_create();
#endif

    config_eeeprom_init_check();

#if BMS_SUPPORT_RELAY
    relay_control_init();
#endif
    
    bms_main_job_init();

#if BMS_SUPPORT_CAN_RE_INIT
    can_re_init_init();
#endif 

    output_ctrl_init();  
    
    event_deliver(kInfoEventSystemStartup);

    DEBUG("main", "finish the init.");
    
}
