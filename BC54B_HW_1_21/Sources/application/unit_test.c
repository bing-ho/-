#include "unit_test.h"
#include "unity_fixture.h"
#include "bms_base_cfg.h"
#include "dtu_m35_impl.h"
#include "xgate.h"
#include "bms_system.h"
#include "bms_main_job.h"


#if 0
void TERMIO_PutChar(char c) {
    while (SCI1SR1_TC == 0);
    SCI1DRL = c;
    if (c == '\n') {
        while (SCI1SR1_TC == 0);
        SCI1DRL = '\r';
    }
}

#endif

static void run_all_test(void) {
    //RUN_TEST_GROUP(second_datetime);
    //RUN_TEST_GROUP(ex_string);
    //RUN_TEST_GROUP(ringbuffer);
    //RUN_TEST_GROUP(dtu_atcmd);
    //RUN_TEST_GROUP(mock_i2c)
    //RUN_TEST_GROUP(rtc);
    //RUN_TEST_GROUP(ads1015);
    //RUN_TEST_GROUP(input_signal);
    //RUN_TEST_GROUP(cp_pwm);
    //RUN_TEST_GROUP(async);
    //RUN_TEST_GROUP(insu_res_unbalanced_bridge);
    //RUN_TEST_GROUP(sd2405al);
    //RUN_TEST_GROUP(soc_adc);
    //RUN_TEST_GROUP(xadc0);
    RUN_TEST_GROUP(dtu_m35_impl);
}


static void init_debug_uart(void) {
    unsigned long br = (unsigned long)((BMS_BUS_CLOCK + (unsigned long)9600 * 8UL) / ((unsigned long)9600 * 16UL));
    SCI1BDH = (unsigned char)(br >> 8);
    SCI1BDL = (unsigned char)br;
    SCI1CR1 = 0x00;
    SCI1CR2 = 0x00;
    SCI1CR2 |= (SCI1CR2_TE_MASK);
}

void sys_clock_init(void);
void OSTickISR_Init(void);
void timers_clock_init(void);

void unit_test_task(void *pdata) {
    (void)pdata;

    mcu_init();
    sys_clock_init();
    timers_clock_init();
    SetupXGATE();
    OSTickISR_Init();
    init_debug_uart();
    bms_main_job_init();
    OSTimeDly(10);
    //ADS1015_TestInit();
    //DTUM35_Init(&g_DTUM35_BspInterface);
    (void)UnityMain(1, NULL, run_all_test);
    for(;;) {
        wdt_feed_dog();
        OSTimeDly(100);
    }
}

