#include "bms_timer_impl.h"


#if BMS_SUPPORT_TIMER

#pragma MESSAGE DISABLE C1420 // Result of function-call is ignored

LIST_ITEM* g_timer_list = NULL;
INT16U g_timer_min_remain = 0;
INT16U g_timer_tick = 0;
lock_t g_timer_lock = NULL;

#define MIN(a, b) (a < b ? a : b)

/**
 * @brief initialize the timer module
 */
void timer_init(void)
{
    g_timer_list = list_init();
    g_timer_tick = 0;
    g_timer_lock = lock_create();
}

/**
 * @brief uninitialize the timer module
 */
void timer_uninit(void)
{
    list_uninit(g_timer_list);
    g_timer_list = NULL;

    lock_destroy(g_timer_lock);
    g_timer_lock = NULL;

}

/**
 * @brief start a timer
 *
 * @param[in] func - the callback function.
 * @param[in] time - the interval time, the unit is related with Tick
 * @return -the timer id, return 0 if failed.
 */
TimerId timer_start(TimerFunc func, void* user_data, INT16U time)
{
    TIMER_ITEM* item;
    if (func == NULL || time == 0) return 0;
    if (g_timer_list == NULL) return 0;

    item = (TIMER_ITEM*) MALLOC(sizeof(TIMER_ITEM));
    if (item == NULL)
    {
        return 0;
    }

    item->list.next = NULL;
    item->interval = time;
    item->remain = time + g_timer_tick;
    item->func = func;
    item->user_data = user_data;

    list_add(g_timer_list, (LIST_ITEM*) item);
    if (g_timer_min_remain == 0 || item->remain < g_timer_min_remain) g_timer_min_remain = item->remain;

    return item;
}

/**
 * @brief start a timer
 *
 * @param[in] func - the callback function.
 * @param[in] time - the interval time, the unit is related with Tick
 * @return -the timer id, return 0 if failed.
 */
void timer_restart(TimerId id, TimerFunc func, void* user_data, INT16U time)
{
    TIMER_ITEM* item = (TIMER_ITEM*) id;

    item->interval = time;
    item->remain = time + g_timer_tick;
    item->func = func;
    item->user_data = user_data;

    list_add(g_timer_list, (LIST_ITEM*) item);
    if (g_timer_min_remain == 0 || item->remain < g_timer_min_remain) 
    {
        g_timer_min_remain = item->remain;
    }
}

int timer_reset(TimerId id)
{
    TIMER_ITEM* item = (TIMER_ITEM*) id;
    if (g_timer_list == NULL || id == NULL) return -1;

    item->remain = item->interval + g_timer_tick;

    return 0;
}

/**
 * @brief stop a timer
 *
 * @param[in] id - the timer id.
 */
void timer_stop(TimerId id)
{
    if (g_timer_list == NULL || id == NULL) return;

    list_remove(g_timer_list, (LIST_ITEM*)id);
    FREE(id);
}

/**
 * @brief Check the timer table
 *
 */
void timer_check(INT16U tick)
{
    LIST_ITEM* item;

    g_timer_tick += tick;

    /* pre-check for improving the performance   */
    if (g_timer_min_remain > 0 && g_timer_min_remain > tick)
    {
        g_timer_min_remain -= tick;
        return;
    }
    g_timer_min_remain = 0;

    /* check every timer */
    item = list_next(g_timer_list);
    while (item)
    {
        TIMER_ITEM* timer = (TIMER_ITEM*) item;
        item = list_next(item);
        if (timer->remain <= g_timer_tick)
        {
            /* the timer is trigger */
            timer->remain = timer->interval;
            timer->func(timer, timer->user_data);
        }
        else
        {
            timer->remain -= g_timer_tick;
        }
        if (g_timer_min_remain == 0 || timer->remain < g_timer_min_remain) g_timer_min_remain = timer->remain;
    }

    g_timer_tick = 0;
}
#endif

