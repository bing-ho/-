#include <hidef.h>      /* common defines and macros */
#include "..\ucos_source\cpu\includes.h"
#include "bms_system.h"
#include "bms_main.h"
#include "bms_upper_computer_j1939.h"
#include "bms_timer.h"
#include "xgate.h"
#include "bms_work_state.h"
#include "bms_dtu.h"
#include "ads1015_intermediate.h"
#include "unit_test.h"
#include "run_mode.h"
#include "test_mode.h"
#include "wakeup_mode.h"
#include "read_mode.h"
#include "detect_mode.h"

#pragma MESSAGE DISABLE C1420 // Result of function-call is ignored

#pragma CONST_SEG   PAGED_VERSION
volatile const INT16U g_bms_template_version[] = //模板版本
    {DEVICE_SW_MAJOR_VER, DEVICE_SW_MINOR_VER, DEVICE_SW_REV_VER, DEVICE_SW_BUILD_VER};
volatile const INT16U g_bms_order_version[] =  //订单版本
    {ORDER_SW_MAJOR_VER, ORDER_SW_MINOR_VER, ORDER_SW_REV_VER, ORDER_SW_BUILD_VER};;
volatile const INT8U g_bms_hardware_name[] = {DEVICE_COMMON_NAME};
volatile const INT8U g_bms_hardware_version[]= //硬件版本号
    {DEVICE_HW_MAJOR_VER,DEVICE_HW_MINOR_VER};
#pragma CONST_SEG   DEFAULT


#define SET_INTERRUPT_VECTOR_BASE_ADDR(ADDR)    IVBR = ADDR

OS_STK g_start_task_stack[OS_TASK_START_STK_SIZE] =
{ 0 };

/*
 *********************************************************************************************************
 *                                        BSP_CPU_ClkFreq()
 *
 * Description : Returns the CPU operating frequency in Hz. (This is NOT the CPU BUS frequency)
 *               However, the bus frequency is (clk_frq / 2)
 *********************************************************************************************************
 */

INT32U BSP_BUS_ClkFreq(void)
{
    INT32U clk_freq;
    INT8U mul;
    INT8U div;

    mul = (SYNR & 0x3F) + 1;
    div = (REFDV & 0x3F) + 1;

    if ((CLKSEL & CLKSEL_PLLSEL_MASK) > 0)
    {
        clk_freq = ((OSCFREQ * 2) * mul) / div;
    }
    else
    {
        clk_freq = OSCFREQ;
    }
    return clk_freq / 2;
}
/*
 *********************************************************************************************************
 *                                     uC/OS-II TICK ISR INITIALIZATION
 *
 * Description : This function is used to initialize one of the eight output compares to generate an
 *               interrupt at the desired tick rate.  You must decide which output compare you will be
 *               using by setting the configuration variable OS_TICK_OC (see OS_CFG.H and also OS_CPU_A.S)
 *               to 0..7 depending on which output compare to use.
 *                   OS_TICK_OC set to 4 chooses output compare #4 as the ticker source
 *                   OS_TICK_OC set to 5 chooses output compare #5 as the ticker source
 *                   OS_TICK_OC set to 6 chooses output compare #6 as the ticker source
 *                   OS_TICK_OC set to 7 chooses output compare #7 as the ticker source
 * Arguments   : none
 * Notes       : 1) It is assumed that you have set the prescaler rate of the free running timer within
 *                  the first 64 E clock cycles of the 68HC12.
 *               2) CPU registers are define in IO.H (see COSMIC compiler) and in OS_CPU_A.S.
 *********************************************************************************************************
 */
#if 1
void OSTickISR_Init(void) {
    RTICTL = (1<<7) | (1<<4) | (3<<0); // RTI FREQUCES = OSCCLK / (8*10^3)
    PLLCTL |= (1<<2);
    CRGINT |= (1<<7);
    CLKSEL |= (1<<1);

    ECT_TIE = 0;
    ECT_TIOS = 0;
    ECT_TSCR1 = 0xC0; /* Enable counter & disable counter in background mode     */
}

void OSTickISR_Handler(void)
{
    //CRGFLG_RTIF = 1;
    CRGFLG = 0x80;
#if BMS_SUPPORT_J1939_COMM
    J1939CanHeartBeat(g_slaveCanContextPtr);
    J1939CanHeartBeat(g_chargerCanContextPtr);
#if BMS_SUPPORT_DTU == BMS_DTU_CAN
    J1939CanHeartBeat(g_dtuContext.can_cxt);
#endif /* BMS_SUPPORT_DTU == BMS_DTU_CAN */
#endif /* BMS_SUPPORT_J1939_COMM */

#if BMS_SUPPORT_TIMER
    timer_check(1);
#endif
}

interrupt void near ECT_TC7_ISR(void) {    
    ECT_TFLG1 |= 0x80;
    ECT_TIE &= ~0x80;
    ECT_TIOS &= ~0x80;   
}

#else
static INT16U OSTickCnts;
void OSTickISR_Init(void)
{
    INT32U cpu_frq;
    INT32U bus_frq;
    INT8U ect_prescaler;

    cpu_frq = BSP_CPU_ClkFreq(); /* Get the current CPU frequency                           */
    //bus_frq = cpu_frq / 2;                  /* Derive the BUS frequency from the CPU frequency         */

    bus_frq = cpu_frq; /* Derive the BUS frequency from the CPU frequency         */
    ect_prescaler = ECT_TSCR2 & 0x07; /* Get the prescaler value in the control register         */

    ect_prescaler = (1 << ect_prescaler); /* Calculate the correct prescaler value from the reg val  */

    /* Calculate the nbr of ticks for the interrupt period     */
    OSTickCnts = (INT16U) ((bus_frq / (ect_prescaler * OS_TICKS_PER_SEC)) - 1);

#if OS_TICK_OC == 4
    ECT_TIOS |= 0x10; /* Make channel an output compare                          */
    ECT_TC4 = ECT_TCNT + OSTickCnts; /* Set TC4 to present time + OS_TICK_OC_CNTS               */
    ECT_TIE |= 0x10; /* Enable OC4 interrupt.                                   */
#endif

#if OS_TICK_OC == 5
    ECT_TIOS |= 0x20; /* Make channel an output compare                          */
    ECT_TC5 = ECT_TCNT + OSTickCnts; /* Set TC5 to present time + OS_TICK_OC_CNTS               */
    ECT_TIE |= 0x20; /* Enable OC5 interrupt.                                   */
#endif

#if OS_TICK_OC == 6
    ECT_TIOS |= 0x40; /* Make channel an output compare                          */
    ECT_TC6 = ECT_TCNT + OSTickCnts; /* Set TC6 to present time + OS_TICK_OC_CNTS               */
    ECT_TIE |= 0x40; /* Enable OC6 interrupt.                                   */
#endif

#if OS_TICK_OC == 7
    ECT_TIOS |= 0x80; /* Make channel an output compare                          */
    ECT_TC7 = ECT_TCNT + OSTickCnts; /* Set TC7 to present time + OS_TICK_OC_CNTS               */
    ECT_TIE |= 0x80; /* Enable OC7 interrupt.                                   */
#endif

    ECT_TSCR1 = 0xC0; /* Enable counter & disable counter in background mode     */
}

/*
 *********************************************************************************************************
 *                                      uC/OS-II TICK ISR HANDLER
 *
 * Description : This function is called by OSTickISR() when a tick interrupt occurs.
 *
 * Arguments   : none
 *********************************************************************************************************
 */

void OSTickISR_Handler(void)
{
    INT8U i = 0;

#if OS_TICK_OC == 4
    ECT_TFLG1 |= 0x10; /* Clear interrupt                                         */
    ECT_TC4 += OSTickCnts; /* Set TC4 to present time + OS_TICK_OC_CNTS               */
#endif

#if OS_TICK_OC == 5
    ECT_TFLG1 |= 0x20; /* Clear interrupt                                         */
    ECT_TC5 += OSTickCnts; /* Set TC5 to present time + OS_TICK_OC_CNTS               */
#endif

#if OS_TICK_OC == 6
    ECT_TFLG1 |= 0x40; /* Clear interrupt                                         */
    ECT_TC6 += OSTickCnts; /* Set TC6 to present time + OS_TICK_OC_CNTS               */
#endif

#if OS_TICK_OC == 7
    ECT_TFLG1 |= 0x80; /* Clear interrupt                                         */
    ECT_TC7 += OSTickCnts; /* Set TC7 to present time + OS_TICK_OC_CNTS               */
#endif

#if BMS_SUPPORT_J1939_COMM
    J1939CanHeartBeat(g_slaveCanContextPtr);
    J1939CanHeartBeat(g_chargerCanContextPtr);
#if BMS_SUPPORT_DTU == BMS_DTU_CAN
    J1939CanHeartBeat(g_dtuContext.can_cxt);
#endif /* BMS_SUPPORT_DTU == BMS_DTU_CAN */
#endif /* BMS_SUPPORT_J1939_COMM */

#if BMS_SUPPORT_TIMER
    timer_check(1);
#endif
}
#endif

/*********************************************************************************************************
 ** 函数名称: sys_clock_init()
 ** 功能描述: 时钟配置函数,晶振频率8.000 MHz	总线时钟16M PLL时钟32M
 ** 输　  入:
 ** 输　  出:
 ** 全局变量:
 ** 调用模块:
 ** 作　  者: 
 ** 日　  期:
 ** 返    回: ==0 CLOCK在这之前没有配置, !=0 CLOCK在这之前已经配置正常.
 **-------------------------------------------------------------------------------------------------------
 ** 修 改 人:
 ** 日　  期:
 **------------------------------------------------------------------------------------------------------
 ********************************************************************************************************/
void sys_clock_init(void)
{
    CLKSEL = 0x00; // disengage PLL to system
    PLLCTL_PLLON = 0; // turn off PLL
#if BMS_BUS_CLOCK == 16000000UL
    SYNR = 0x03;
    REFDV = 0x01; //PLLCLK=2*OSCCLK*((SYNR+1)/(REFDV+1)) =96MHz
               //BUSCLK=48MHz      每个机器周期为 0.0208us
               //OSCCLK=8 MHz
#else #if BMS_BUS_CLOCK == 48000000UL
    SYNR = 0xC5;
    REFDV = 0x80; //PLLCLK=2*OSCCLK*((SYNR+1)/(REFDV+1)) =96MHz
               //BUSCLK=48MHz      每个机器周期为 0.0208us
               //OSCCLK=8 MHz
#endif
    PLLCTL_PLLON = 1; // turn on PLL
    _asm(nop);
    _asm(nop);
    while (!(CRGFLG_LOCK == 1))
        ;
    CLKSEL |= 0x80; //bus clock=pllclk/2;
}

unsigned long ect_clock;

void timers_clock_init(void) {
    INT32U bus_clk;
    bus_clk = BSP_BUS_ClkFreq();
    // init Periodic Interrupt Timer
    PITMTLD0 = (bus_clk / 1000000) - 1; // 1M
    PITMTLD1 = (bus_clk / 200000) - 1;  // 200K

    // init Enhanced Capture Timer
    if (bus_clk >= 64000000UL) {
        ECT_TSCR2_PR = 0x04; // bus_clk / 16
        ect_clock = bus_clk /16;
    } else if (bus_clk >= 32000000UL){
        ECT_TSCR2_PR = 0x03; // bus_clk / 8 
        ect_clock = bus_clk /8;
    } else if (bus_clk >= 16000000UL) {
        ECT_TSCR2_PR = 0x02; // bus_clk / 4
        ect_clock = bus_clk /4;
    }else if (bus_clk >= 8000000UL) {
        ECT_TSCR2_PR = 0x01; // bus_clk / 2
        ect_clock = bus_clk /2;
    }else {        
        ECT_TSCR2_PR = 0x00; // bus_clk 
        ect_clock = bus_clk;
    }
}

void wait_for_work_state()
{
    /*INT8U stop_inited = 0;
    while (bcu_get_mcu_work_state() != kMCUWorkStateWork)
    {
        sleep(1000);
        diagnose_work_state();

        if (stop_inited == 0 && bcu_get_mcu_work_state() == kMCUWorkStateStop)
        {
            stop_inited = 1;
            mcu_switch_to_stop_mode();
        }
    }
    */
}


void bms_check_power_on_type(void)
{
    if(CRGFLG_LVRF == 1)
    {
        CRGFLG |= 0x20; //清除LVRF标志        
#if 0//BMS_SUPPORT_DATA_SAVE
        data_save_set_sd_write_state(kSDWriteEnable);
#endif
    }
}

/*
 *******************************************************************************
 **函    数:	start_task
 **功    能:
 **参    数：无
 **返 回 值：无
 **说    明：
 *******************************************************************************
 */
void start_task(void *pdata)
{
    enum runmode mode;
    pdata = pdata; /* Prevent compiler warning for not using 'pdata'     */
    
    g_bms_hardware_version[0]; //防止被优化
    g_bms_template_version[0];
    g_bms_order_version[0];
    g_bms_hardware_name[0];
    //   initializes the General Purpose Timer Unit (GPT1)
    mcu_init();
    
    timers_clock_init();
    
    bms_check_power_on_type();
    
    SetupXGATE();
    
    OSTickISR_Init(); //Timer initialization

    //OSStatInit();
    config_init();
    
    runmode_init();
    mode = runmode_get();                        
    if (RUN_MODE_TEST == mode) 
    {
        test_mode_run();
    } 
     
    else if (mode == RUN_MODE_WAKEUP) {
        bms_work_state_init();
        wakeup_mode_run();
    } 
     
    else if(mode == RUN_MODE_READ)
    {
        bms_work_state_init();    
#if BMS_SUPPORT_SLEEP_MODE
        wait_for_work_state();
#endif
        read_mode_run();    
    }
    
    else if(mode == RUN_MODE_DETECT)
    {
        bms_work_state_init();    
#if BMS_SUPPORT_SLEEP_MODE
        wait_for_work_state();
#endif
        detect_mode_run();
    }
     
    else 
    {
        bms_work_state_init();    
#if BMS_SUPPORT_SLEEP_MODE
        wait_for_work_state();
#endif
        system_init(); //OSTickISR_Init()放在本函数前面，否则本函数不能调用
    }
    
    for (;;)
    {
        OSTaskDel(OS_PRIO_SELF);
    }
}



void main(void) 
{
    /* clear bootloader interrupt and uninit ect、can */
    DisableInterrupts;
    ECT_TIE = 0;
    ECT_TSCR1 = 0;
    CAN0RIER_RXFIE = 0;
    
    /* redirect interrupt vector */
    SET_INTERRUPT_VECTOR_BASE_ADDR(0x7f);
    sys_clock_init();
    
    OSInit(); // OS init
    
#if defined(UNIT_TEST) && (0 != UNIT_TEST)
    {
    static OS_STK unit_test_stack[1024];
    OSTaskCreate(unit_test_task, (void *) 0, (OS_STK *) &unit_test_stack[sizeof(unit_test_stack)-1],
            OS_TASK_START_PRIO);
    }
#else
    

    OSTaskCreate(start_task, (void *) 0, (OS_STK *) &g_start_task_stack[OS_TASK_START_STK_SIZE - 1],
            OS_TASK_START_PRIO);
#endif

    OSStart();
    
    /* please make sure that you never leave main */
}
