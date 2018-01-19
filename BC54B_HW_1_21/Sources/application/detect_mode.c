/**
 *
 * Copyright (c) 2017 Ligoo Inc.
 *
 * @file detect_mode.c
 * @brief
 * @note
 * @author Han Zhaoxia
 * @date 2017-11-1
 *
 */    
#include "includes.h"
#include "j1939_poll.h"
#include "bms_stat.h"
#include "bms_hmi.h"
#include "bms_ems.h"
#include "bms_dtu.h"
#include "bms_soh.h"
#include "bms_data_save.h"
#include "bms_diagnosis.h"
#include "bms_byu.h"
#include "bms_system_voltage.h"
#include "bms_board_temperature.h"
#include "bms_insulation.h"
#include "adc.h"
#include "bms_dtu_buildin_impl.h"
#include "bms_analog_inputs.h"
#include "ect_intermediate.h"
#include "ect_xgate.h"
#include "power_down_data_save.h"
#include "adc_xgate.h"
#include "bms_mlock_impl.h"
#include "bsu_sample.h"
#include "bms_bsu.h"
#include "test_io.h"
//#include "logic_ctrl_task.h"
#if( SWITCH_ON == UDS_SWITCH_CONFIG)
#include "CanTp.h"
#include "Dcm.h"
#include "CanIf.h"
#include "Dtc_code.h"
#include "detect_mode.h"
#endif
#define BMS_GLOBALS


void detect_mode_run(void)      //系统初始化
{
    OS_CPU_SR cpu_sr = 0;

    alert_init();

    mcu_init();
    
    pwm_init();

    clock_init();

    log_init();

    event_init();

    //config_init();

    timer_init();
    //output_ctrl_init();

    bcu_init();

    bmu_init();
#if BMS_SUPPORT_HARDWARE_LTC6804 == 1
    bsu_init();
#else 
    byu_init();
#endif    
    //adc_para_init() ;
    
    //adc0_init() ;
    xadc0_init();
    
    //power_down_data_save_init();

#if BMS_SUPPORT_DATA_SAVE
    data_save_init();
#endif

#if( SWITCH_ON == UDS_SWITCH_CONFIG)
    //UDS related init
    CanIf_Init();
    CanTp_Init(&CanTp_Cfg);
    Dcm_Init(NULL);
#endif

#if BMS_SUPPORT_UPPER_COMPUTER
    upper_computer_init();
#endif

#if BMS_SUPPORT_J1939_COMM
    j1939_poll_task_create();
#endif

    config_eeeprom_init_check();

    hardware_revision_check();
    
#if BMS_SUPPORT_HMI
    hmi_init();
#endif

#if BMS_SUPPORT_MOTOR
    motor_init();
#endif

#if BMS_SUPPORT_EMS
    detect_ems_init();//ems_init();
#endif

#if BMS_SUPPORT_RELAY
    relay_control_init();
#endif

#if BMS_SUPPORT_CHARGER
    charger_init();
#endif

#if BMS_SUPPORT_INPUT_SIGNAL
    input_signal_control_init();
#endif

    //rule_engine_init();

#if BMS_SUPPORT_DTU
    dtu_init();
#endif

#if BMS_SUPPORT_SOC
    soc_init();
#endif

#if BMS_SUPPORT_SOH
    soh_init();
#endif

#if BMS_SUPPORT_SYSTEM_VOLTAGE
    system_voltage_init();
#endif

#if BMS_SUPPORT_BOARD_TEMPERATURE
    board_temperature_init();
#endif
    
    bms_main_job_init();

#if BMS_SUPPORT_STAT
    stat_init();
#endif

#if BMS_SUPPORT_DOG_WATCHER
   watch_dog_init();
#endif

#if BMS_SUPPORT_CAN_RE_INIT
    can_re_init_init();
#endif 

#if BMS_SUPPORT_INSULATION_DIAGNOSE
    insu_res_analyze_init();
    //insu_res_analyze_init_xx();
#endif

    detect_diagnosis_init();//bms_diagnosis_init();

    output_ctrl_init();
    mlock_init();//bms_ChargerLock_init();
    //logic_ctrl_init();

#if (SWITCH_ON == UDS_SWITCH_CONFIG)
    Init_Uds();   /*对UDS进行初始化操作*/
#endif
   
    //上电初始化完成通告
    //alert_long_beep(SYSTEM_STARTUP_BEEP_TIME);
    if(config_get(kOtaErrFlag)) 
    {
        config_save(kOtaErrFlag,0);    
    }    
    
    event_deliver(kInfoEventSystemStartup);

    DEBUG("main", "finish the init.");

#if BMS_SUPPORT_SOC
    soc_daemon_run();
#endif
    
}
