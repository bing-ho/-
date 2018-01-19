/**
*
* Copyright (c) 2012 Ligoo Inc.
*
* @file bms_system_impl.c
* @brief
* @note
* @author
* @date 2012-5-3
*
*/

#include "bms_system_impl.h"
#include "bms_base_cfg.h"
#include "gpio_interface.h"
#include "bms_bcu.h"

#if BMS_SUPPORT_SYSTEM

#pragma MESSAGE DISABLE C4001 // Condition always FALSE
#pragma MESSAGE DISABLE C1420 // Result of function-call is ignored

lock_t g_global_lock = NULL;

#define	TimerInit(ch)				__TimerInit(ch)
#define TimerReload(ch, reload)		__TimerReload(ch, reload)
#define TimerOn(ch) 				__TimerOn(ch)
#define TimerOff(ch) 				__TimerOff(ch)
#define TimerIsOn(ch)				__TimerIsOn(ch)

#undef USE_TIMER  
/**
 * 指定定时器中断的通道
 */
#define CHx		4
/**
 * 计算蜂鸣器输出频率对应的反转周期
 */
#define CYCLE(hz)	((BMS_BUS_CLOCK/((hz))))
/**
 * 输出的频率
 */
#define FREQ		2730
/**
 * 输出占空比 DUTY_H / (DUTY_H + DUTY_L)
 */
#define DUTY_H		1
#define DUTY_L		1

static const INT16U duty[2] = { 
	(INT16U)(CYCLE(FREQ) * DUTY_L / (DUTY_H + DUTY_L)),
	(INT16U)(CYCLE(FREQ) * DUTY_H / (DUTY_H + DUTY_L)), 
};

#define STRING_BEEP      "BEEP"
const struct hcs12_gpio_detail beep_output_pins_1_14[] = {
    { STRING_BEEP,           { &PORTA,   &PORTA,     &DDRA,      1 << 0}, 0 , 0}, //PA0 ==> PA2
    {(const char *)0,   { (volatile uint8_t *)0, (volatile uint8_t *)0, (volatile uint8_t *)0, 0}, 0, 0},
};

const struct hcs12_gpio_detail beep_output_pins_1_16[] = {
    { STRING_BEEP,           { &PORTA,   &PORTA,     &DDRA,      1 << 2}, 0 , 0},
    {(const char *)0,   { (volatile uint8_t *)0, (volatile uint8_t *)0, (volatile uint8_t *)0, 0}, 0, 0},
};
#pragma push
#pragma DATA_SEG __RPAGE_SEG PAGED_RAM
const struct hcs12_gpio_detail *beep_output_pins = NULL;
#pragma pop

/**
 * 定时器中断服务函数,修改定时器通道需要同时修改该服务函数名
 */
void TimerCh4ISR_Handler(void){
#ifdef USE_TIMER
    uint8_t is_beep_on = hcs12_gpio_output_is_high(&beep_output_pins[0].io);
    is_beep_on = !is_beep_on;
    hcs12_gpio_set_output(&beep_output_pins[0].io, !is_beep_on);

    TimerReload(CHx, duty[is_beep_on]);
#endif
}

void beep_init(void)
{
#ifdef USE_TIMER
    TimerInit(CHx);	
	ECT_TSCR1 = 0xC0;
#endif

#if BMS_SUPPORT_BY5248D == 0

#if BMS_SUPPORT_HARDWARE_BEFORE == 1   // yang 20161213
    if(hardware_io_revision_get() >= HW_VER_116)
    {
        beep_output_pins = beep_output_pins_1_16;
    }
    else
    {
        beep_output_pins = beep_output_pins_1_14;
    }
#else
    beep_output_pins = beep_output_pins_1_16;
#endif
  
#else
    beep_output_pins = beep_output_pins_1_16;
#endif

    hcs12_gpio_init(&beep_output_pins[0].io, GPIO_MODE_OUTPUT_PUSHPULL);
    hcs12_gpio_set_output(&beep_output_pins[0].io, beep_output_pins[0].def_val);    
}

void beep_on(void)
{
#if BMS_BEEP_SLIENT == 0
	#ifdef USE_TIMER
        uint8_t is_beep_on = hcs12_gpio_output_is_high(&beep_output_pins[0].io);
        TimerReload(CHx, duty[is_beep_on]);        
		TimerOn(CHx);
	#else
        hcs12_gpio_set_output(&beep_output_pins[0].io, 1);
	#endif
#endif
}

void beep_off(void)
{
#ifdef USE_TIMER
	TimerOff(CHx);
#endif
    hcs12_gpio_set_output(&beep_output_pins[0].io, 0);
}

BOOLEAN beep_is_on(void)
{
#ifdef USE_TIMER	
	return TimerIsOn(CHx);
#else
    return hcs12_gpio_output_is_high(&beep_output_pins[0].io);
#endif
}

lock_t lock_create(void)
{
    return (lock_t)OSSemCreate(1);
}

int lock_destroy(lock_t lock)
{
	INT8U err;

	UNUSED(lock);
    /* cannot free in ucos */
    OSSemDel((OS_EVENT*)lock, OS_DEL_ALWAYS, &err);//TODO
    return 0;
}

int lock_acquire(lock_t lock)
{
    INT8U err;
    if (lock == NULL) return RES_ERROR;

    OSSemPend((OS_EVENT*)lock, 0, &err);
    return err;
}

int lock_release(lock_t lock)
{
    INT8U err;
    if (lock == NULL) return RES_ERROR;

    err = OSSemPost((OS_EVENT*)lock);
    return err;
}

Result global_lock_init(void)
{
    if (g_global_lock == NULL) g_global_lock = lock_create();

    return g_global_lock ? RES_OK : RES_ERROR;
}

Result global_lock_acquire(void)
{
    if (g_global_lock == NULL) g_global_lock = lock_create();
    if (g_global_lock == NULL) return RES_ERROR;

    return lock_acquire(g_global_lock);
}

Result global_lock_release(void)
{
    if (g_global_lock == NULL) return RES_ERROR;
    return lock_release(g_global_lock);
}

int atom_inc(int* value, int num)
{
    OS_CPU_SR cpu_sr = 0;
    int new_value;
    OS_ENTER_CRITICAL();
    *value += num;
    new_value = *value;
    OS_EXIT_CRITICAL();
    return new_value;
}

void usleep(INT32U us)
{
    sleep(us / 1000);
}

void sleep(INT32U ms)
{
    INT16U index, times;
    if (ms <= MAXINT16U)
    {
        OSTimeDly((INT16U)ms);
        return;
    }

    times = (ms >> 16);
    for (index = 0; index < times; index++)
    {
        OSTimeDly(MAXINT16U);
        OSTimeDly(1);
    }

    OSTimeDly((INT16U)(ms & 0xFFFF));
}

INT32U get_tick_count(void)
{
    return OSTimeGet();
}

INT32U get_elapsed_tick_count(INT32U last_tick)
{
    return get_interval_by_tick(last_tick, get_tick_count());
}

INT32U get_interval_by_tick(INT32U old_tick, INT32U new_tick)
{
    if(new_tick >= old_tick) return new_tick - old_tick;

    return new_tick + ((INT32U)0xFFFFFFFF - old_tick);
}

INT32U get_elapsed_tick_count_with_init(INT32U* _PAGED last_tick)
{
    if (last_tick == NULL) return 0;
    if (*last_tick == 0)
    {
        *last_tick = get_tick_count();
        return 0;
    }
    else
    {
        return get_interval_by_tick(*last_tick, get_tick_count());
    }
}


void mcu_init(void)
{
}

void mcu_reset(void)
{
    COPCTL = 0x01;        //enable watchdog
    ARMCOP = 0x00;
    //value written to ARMCOP register different from 0xAA or 0x55 will reset
    //the MCU immediately. The pushbutton on pin PP0 is supposed to be in default
    //state (logic '1'), so user application will be executed.
}

void mcu_switch_to_stop_mode(void)
{
}

void mcu_switch_to_work_mode(void)
{
}
void wdt_init(void)
{
    COPCTL_CR  = 7;  //2.048S
    COPCTL_WCOP =0;
}

void wdt_feed_dog(void)
{
    (ARMCOP =0x55,ARMCOP = 0xAA);
}

#endif

