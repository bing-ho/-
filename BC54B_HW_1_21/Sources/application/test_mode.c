#include "test_mode.h"
#include "stdint.h"
#include "run_mode.h"
#include "ucos_ii.h"
#include "bms_config.h"
#include "stdio_impl.h"
#include "adc_xgate.h"
#include "ads1015_intermediate.h"
#include "bts724g_intermediate.h"
#include "test_io.h"
#include "bsu_sample.h"
#include "mcp3421_hardware.h"
#include "mcp3421_intermediate.h" 
#include "hcf4052_intermediate.h"


extern void shell_impl_create_shell_task(void);
#if BMS_SUPPORT_DATA_SAVE
//extern void test_sd_init(void);
#endif
extern void test_can_init(void);
extern void test_rs485_init(void);
extern void test_dtu_init(void);
extern void test_hall_init(void);
extern void test_cppwm_init(void);
extern void test_nand_init(void);
extern void adc_onchip_init(void);
extern void hw_board_temp_init(void);
extern void relay_control_init(void);


static unsigned char heartbeat_timeout_flag = 0;

//测试模式的高可用集群
void test_mode_heartbeat(void) {
    heartbeat_timeout_flag = 0;
}
//检测测试模式的高可用集群
static void check_heartbeat() {
    for (;;) {
#if !(defined(RUN_TEST_MODE_ALWAYS) && RUN_TEST_MODE_ALWAYS != 0)
        OS_CPU_SR cpu_sr = 0;
        unsigned char timeout;
        OS_ENTER_CRITICAL();
        timeout = heartbeat_timeout_flag;
        OS_EXIT_CRITICAL();
        if (timeout) {
            runmode_reset_to(RUN_MODE_NORMAL);
        }

        OS_ENTER_CRITICAL();
        heartbeat_timeout_flag = 1;
        OS_EXIT_CRITICAL();
#endif
        OSTimeDly(800);
    }
}

void test_spi_init(void);
//测试模式运行
void test_mode_run(void) {
    bms_main_job_init();
    //config_init();
    stdio_impl_init();
    input_gpio_init();
    relay_control_init();
    output_ctrl_init();
    clock_init();
    if ((hardware_io_revision_get() >= HW_VER_115) && 
        input_signal_rtc_interrupt_is_assert()) 
    { // RTC interrupt
        printf("Wakeup by RTC, hold the power then clear the interrupt.");
        output_ctrl_set_with_name("SYSTEM_POWER_LATCH", 1); // 自锁电源.
        OSTimeDly(10);
        clock_clear_interrupt();
        OSTimeDly(10); 
        output_ctrl_set_with_name("SYSTEM_POWER_LATCH", 0); // 自锁电源.
    }
    
    eeprom_init();    
    hcf4052_impl_init();    
    hw_board_temp_init();
    test_nand_init();
#if BMS_SUPPORT_DATA_SAVE
   // test_sd_init();
#endif
 //SPI1_Init();       //liqing  20170508
    test_spi_init();      //add xiede 2017.05
    test_can_init();
    test_dtu_init();
    test_rs485_init();
    test_hall_init();
    test_cppwm_init();
    xadc0_init();
    ADS1015_Init(&g_ADS1015_BspInterface);
    adc_onchip_init();

#if BMS_SUPPORT_BY5248D == 1
    MCP3421_Hardware_Init(&g_Mcp3421_BspInterface);
    config_set(kBCUCommModeIndex,1);             //BY5248D
    config_set(kBYUVoltTempCnt1Index,3076);                 
    config_set(kBYUVoltTempCnt2Index,3076);
    config_set(kBYUVoltTempCnt3Index,3076);
    config_set(kBYUVoltTempCnt4Index,3076);
    BSUVoltTempSampleTask_Init();
#endif  

    shell_impl_create_shell_task();

    check_heartbeat();
}

