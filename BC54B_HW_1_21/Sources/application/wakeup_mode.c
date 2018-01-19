#include "wakeup_mode.h"

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
#include "ect_xgate.h"
//#include "pwm_ect.h"
#include "power_down_data_save.h"
#include "adc_xgate.h"
#include "test_io.h"
#include "bms_mlock_impl.h"
#include "bms_bsu.h"
void wakeup_mode_run(void)
{
    OS_CPU_SR cpu_sr = 0;

    //alert_init();

    mcu_init();
    
    //pwm_init();

    clock_init();

    log_init();

    event_init();

    //config_init();

    timer_init();

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


#if BMS_SUPPORT_UPPER_COMPUTER
    upper_computer_init();
#endif

#if BMS_SUPPORT_J1939_COMM
    j1939_poll_task_create();
#endif

    config_eeeprom_init_check();

    hardware_revision_check();
    
#if BMS_SUPPORT_HMI
    //hmi_init();
#endif

#if BMS_SUPPORT_MOTOR
    //motor_init();
#endif

#if BMS_SUPPORT_EMS
    //ems_init();
#endif

#if BMS_SUPPORT_RELAY
    relay_control_init();
#endif

#if BMS_SUPPORT_CHARGER
    //charger_init();
#endif

#if BMS_SUPPORT_INPUT_SIGNAL
    input_signal_control_init();
#endif

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
    
    //rule_engine_init();

    bms_main_job_init();

#if BMS_SUPPORT_STAT
    stat_init();
#endif

#if BMS_SUPPORT_DOG_WATCHER
    watch_dog_init();
#endif

#if BMS_SUPPORT_CAN_RE_INIT
    //can_re_init_init();
#endif 

#if BMS_SUPPORT_INSULATION_DIAGNOSE
    insu_res_analyze_init();
    //insu_res_analyze_init_xx();
#endif

    bms_diagnosis_init();

    output_ctrl_init(); 
   #if BMS_SUPPORT_MLOCK    //高精度一体机取消电子锁
    mlock_init();
   #endif
    
    //上电初始化完成通告
    //alert_long_beep(SYSTEM_STARTUP_BEEP_TIME);
    
    event_deliver(kInfoEventSystemStartup);

    DEBUG("main", "finish the init.");

#if BMS_SUPPORT_SOC
    soc_daemon_run();
#endif
    
}
