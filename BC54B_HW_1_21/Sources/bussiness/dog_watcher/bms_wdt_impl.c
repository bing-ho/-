/**
 *
 * Copyright (c) 2012 Ligoo Inc.
 *
 * @file bms_alert_impl.c
 * @brief
 * @note
 * @author
 * @date 2012-5-31
 *
 */
#include "bms_wdt_impl.h"


#pragma MESSAGE DISABLE C4000 // Condition always TRUE
#pragma MESSAGE DISABLE C5703 // Parameter is not used
#pragma DATA_SEG __RPAGE_SEG PAGED_RAM


INT8U g_dog_watcher_timeout[WDT_MAX_COUNT] = {0};
INT8U g_dog_watcher_tick[WDT_MAX_COUNT] = {0};
INT8U g_dog_watcher_is_enabled = 1;

#pragma DATA_SEG DEFAULT


void watch_dog_init(void)
{
    wdt_init();
    (void)job_schedule(MAIN_JOB_GROUP, FEEDDOG_JOB_PERIODIC, dog_watcher_heart_beat, NULL);
}

void watch_dog_unit(void)
{

}

void watch_dog_disable(void)
{
    g_dog_watcher_is_enabled = 0;
}

void watch_dog_enable(void)
{
    INT8U index;
    if (g_dog_watcher_is_enabled) return;

    for (index = 0; index < WDT_MAX_COUNT; index++)
    {
        g_dog_watcher_tick[index] = g_dog_watcher_timeout[index];
    }

    g_dog_watcher_is_enabled = 1;
}

void watch_dog_register(INT8U id, INT8U timeout)
{
    OS_INIT_CRITICAL();

    if (id >= WDT_MAX_COUNT) return;

    OS_ENTER_CRITICAL();
    g_dog_watcher_timeout[id] = timeout;
    g_dog_watcher_tick[id] = timeout;
    OS_EXIT_CRITICAL();
}

void watch_dog_feed(INT8U id)
{
    OS_INIT_CRITICAL();

    if (id >= WDT_MAX_COUNT) return;

    OS_ENTER_CRITICAL();
    g_dog_watcher_tick[id] = g_dog_watcher_timeout[id];
    OS_EXIT_CRITICAL();
}

void dog_watcher_heart_beat(void* data)
{
    INT8U index;
    INT8U feed = 1;
    OS_INIT_CRITICAL();

    if (!g_dog_watcher_is_enabled)
    {
        wdt_feed_dog();    //看门狗没有使能的话定期喂狗，防止复位
        return;
    }

    OS_ENTER_CRITICAL();
    for (index = 0; index < WDT_MAX_COUNT; index++)
    {
        if (g_dog_watcher_timeout[index] == 0) continue;
        if (g_dog_watcher_tick[index] > 0) g_dog_watcher_tick[index]--;

        if (g_dog_watcher_tick[index] == 0) //正常情况下g_dog_watcher_tick[index]会在各自的任务中进行赋值，如果跑飞的话则不会赋值并自减直至为0
        {
            feed = 0;      
        }
    }
    OS_EXIT_CRITICAL();

    if (feed) wdt_feed_dog();    //如果feed为0的话则不会进行喂狗，导致复位
}

void watch_dog_run(void)
{
    while (1)
    {
        //dog_watcher_heart_beat();
        sleep(1000);
    }
}

