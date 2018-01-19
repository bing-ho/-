#ifndef _BMS_SYSTEM_H__
#define _BMS_SYSTEM_H__

#include "includes.h"

#ifndef BMS_SUPPORT_SYSTEM
#define BMS_SUPPORT_SYSTEM      1
#endif
/**********************************************
 *
 * Beep Setting
 *
 ***********************************************/
/** 静音开关，开启后蜂鸣器将不会发声 */
#ifndef BMS_BEEP_SLIENT
#define BMS_BEEP_SLIENT         0
#endif

/**********************************************
 *
 * Beep Functions
 *
 ***********************************************/
void beep_init(void);
void beep_on(void);
void beep_off(void);
BOOLEAN beep_is_on(void);


/**********************************************
 *
 * Time Functions
 *
 ***********************************************/
void usleep(INT32U us);
void sleep(INT32U ms);

/**********************************************
 *
 * Tick Functions
 *
 ***********************************************/

/**
 * Get the tick count since the system is run
 * @return the tick value, the unit is ms
 */
INT32U get_tick_count(void);
/**
 * Get the elapsed tick count since the last tick
 * @param last_tick
 * @return
 */
INT32U get_elapsed_tick_count(INT32U last_tick);
INT32U get_interval_by_tick(INT32U old_tick, INT32U new_tick);

INT32U get_elapsed_tick_count_with_init(INT32U* _PAGED last_tick);

/**********************************************
 *
 * Lock functions
 *
 ***********************************************/
typedef void* lock_t;
lock_t lock_create(void);
int lock_destroy(lock_t lock);
int lock_acquire(lock_t lock);
int lock_release(lock_t lock);

Result global_lock_init(void);
Result global_lock_acquire(void);
Result global_lock_release(void);

/**********************************************
 *
 * Other functions
 *
 ***********************************************/
int atom_inc(int* value, int num);


/**********************************************
 *
 * Work State
 *
 ***********************************************/
void mcu_init(void);

void mcu_reset(void);
/**
 * 在进入休眠模式前控制IO口
 */
void mcu_switch_to_stop_mode(void);

/**
 * 进入正常工作模式前先控制IO口
 */
void mcu_switch_to_work_mode(void);

void wdt_init(void);
void wdt_feed_dog(void);

#endif
