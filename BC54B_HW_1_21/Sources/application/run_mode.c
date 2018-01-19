#include "run_mode.h"
#include "ucos_ii.h"
#include "stdint.h"
#include "bms_bcu_reset.h"

#define RUN_MODE_FLAG_TEST_1 0x123abc56
#define RUN_MODE_FLAG_TEST_2 0x8908abcd

#define RUN_MODE_FLAG_READ_1 0x735da84b
#define RUN_MODE_FLAG_READ_2 0x67ad82f4

#define RUN_MODE_FLAG_DETECT 0x12ab
#define RUN_MODE_FLAG_NORMAL 0x0000

#define RUN_MODE_FLAG_NORMAL_1 0x00000000
#define RUN_MODE_FLAG_NORMAL_2 0x00000000

enum runmode mode;

static unsigned long run_mode_flag_1 @0xFD1FFC;
static unsigned long run_mode_flag_2 @0xFD1FF8;

extern void mcu_reset(void);
//运行模式复位
void runmode_reset_to(enum runmode this_mode) 
{
    switch(this_mode) 
    {
        case RUN_MODE_TEST:
            run_mode_flag_1 = RUN_MODE_FLAG_TEST_1; 
            run_mode_flag_2 = RUN_MODE_FLAG_TEST_2;
            break;
        case RUN_MODE_READ:
            run_mode_flag_1 = RUN_MODE_FLAG_READ_1;
            run_mode_flag_2 = RUN_MODE_FLAG_READ_2;  
            break;
        case RUN_MODE_DETECT:
            config_save(kDetectFlagIndex,RUN_MODE_FLAG_DETECT);
            break;
        default:
            run_mode_flag_1 = RUN_MODE_FLAG_NORMAL_1; 
            run_mode_flag_2 = RUN_MODE_FLAG_NORMAL_2;
            config_save(kDetectFlagIndex,RUN_MODE_FLAG_NORMAL);
            break;
    }
    if(mode == RUN_MODE_TEST) 
    {
         mcu_reset();
    } 
    else 
    {
        bcu_start_reset();
    }
}

extern void input_signal_init(void);
INT8U input_signal_is_high(INT8U id);
  
//rtc唤醒  
static uint8_t is_rtc_wakeup(void) {
    // PT1 24V_INT == 0, 24V唤醒
    // PAD20 KEY_ON == 0, KEY_ON唤醒
    // PAD167 CHARGER_READY == 0, CHARGER_READY唤醒
    // PP2 SIG1 == 0, SIG1唤醒
    // PT2 CP_MCU == 0, CP唤醒
    // PP7 RTC_INT == 0, RTC唤醒
    
    DDRT_DDRT1 = 0; //PT1    
    ATD1DIENL_IEN4 = 1; //PAD20
    DDR1AD1_DDR1AD14 = 0; //PAD20
    ATD1DIENL_IEN1 = 1; //PAD17
    DDR1AD1_DDR1AD11 = 0; //PAD17
    DDRP_DDRP2 = 0; //PP2
    DDRT_DDRT2 = 0; //PT2
    DDRP_DDRP7 = 0; //PP7

    if (PTIT_PTIT1 == 0) return 0;
    if (PT1AD1_PT1AD14 == 0) return 0;
    if (PT1AD1_PT1AD11 == 0) return 0;
    if (PTIP_PTIP2 == 0) return 0;
    if (PTIT_PTIT2 == 0) return 0;
    if (PTIP_PTIP7 == 0) return 1;
    return 0;    
}        
        
//运行模式初始化        
void runmode_init(void) 
{
#if !(defined(RUN_TEST_MODE_ALWAYS) && RUN_TEST_MODE_ALWAYS != 0)
    if (RUN_MODE_FLAG_TEST_1 == run_mode_flag_1
        && RUN_MODE_FLAG_TEST_2 == run_mode_flag_2) 
        {
        mode = RUN_MODE_TEST;
        return;
    }
    
    if (RUN_MODE_FLAG_READ_1 == run_mode_flag_1
        && RUN_MODE_FLAG_READ_2 == run_mode_flag_2) 
    {
        mode = RUN_MODE_READ;
        return;
    }
    
    if(config_get(kDetectFlagIndex) == RUN_MODE_FLAG_DETECT)
    {
        mode = RUN_MODE_DETECT;
        return;
    }
        
    if ((hardware_io_revision_get() >= HW_VER_115) &&  is_rtc_wakeup()) 
    {
        mode = RUN_MODE_WAKEUP;
        return;
    }
    
    mode = RUN_MODE_NORMAL;
#else
    mode = RUN_MODE_TEST;
#endif
}


#if !(defined(RUN_TEST_MODE_ALWAYS) && RUN_TEST_MODE_ALWAYS != 0)
enum runmode runmode_get(void) {
    return mode;
}
#endif