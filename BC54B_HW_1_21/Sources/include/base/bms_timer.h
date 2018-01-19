#ifndef _BMS_TIMER_H__
#define _BMS_TIMER_H__
#include "includes.h"
#include "bms_system.h"
#include "bms_memory.h"

#ifndef BMS_SUPPORT_TIMER
#define BMS_SUPPORT_TIMER       1
#endif

typedef void* TimerId;

/** 
* @brief Callback function  
*  
* @param[in] id - the timer id.    
* @return - void
*/
typedef void (*TimerFunc)(TimerId id, void* user_data);


/** 
* @brief initialize the timer module    
*/
void timer_init(void);

/** 
* @brief uninitialize the timer module    
*/
void timer_uninit(void);


/** 
* @brief start a timer 
*  
* @param[in] func - the callback function.  
* @param[in] time - the interval time, the unit is related with Tick
* @return -the timer id, return 0 if failed. 
*/
TimerId timer_start(TimerFunc func, void* user_data, INT16U time);

/** 
* @brief restart a timer use old id
*  
* @param[in] func - the callback function.  
* @param[in] time - the interval time, the unit is related with Tick
* @return -0 
*/
void timer_restart(TimerId id, TimerFunc func, void* user_data, INT16U time);

/** 
* @brief reset the timer 
*  
* @param[in] id - the timer id.  
* @return - return 0 if success. 
*/
int timer_reset(TimerId id);


/** 
* @brief stop a timer 
*  
* @param[in] id - the timer id.  
*/
void timer_stop(TimerId id);


/** 
 * @brief Call the function to drive the timer 
 *
 */
void timer_check(INT16U tick);


#endif
