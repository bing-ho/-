/**
*
* Copyright (c) 2012 Ligoo Inc.
*
* @file bms_event_impl.h
* @brief
* @note
* @author
* @date 2012-5-16
*
*/

#ifndef BMS_EVENT_IMPL_H_
#define BMS_EVENT_IMPL_H_
#include "bms_event.h"     

typedef struct
{
    INT8U next;
    EventHandler handler;
    void* user_data;
}EventHandlerItem;

typedef struct
{
    INT8U start_handle;
}EventTypeItem;


EventHandlerId event_handler_find_tail(EventTypeId id);
EventHandlerId event_handler_find_availabled(void);


#endif /* BMS_EVENT_IMPL_H_ */
