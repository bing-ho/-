#ifndef _BMS_TIMER_IMPL_H__
#define _BMS_TIMER_IMPL_H__

#include "bms_timer.h"
#include "bms_list.h"

typedef struct _TIMER_ITEM
{
    LIST_ITEM list;
    TimerId id; // timer id
    TimerFunc func; // func
    void* user_data;
    INT16U interval; // interval

    INT16U remain; // remain time for the next trapping
} TIMER_ITEM;


#endif
