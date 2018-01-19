/**
*
* Copyright (c) 2012 Ligoo Inc.
*
* @file bms_event_impl.c
* @brief
* @note
* @author
* @date 2012-5-16
*
*/
#include "bms_event_impl.h"

#if BMS_SUPPORT_EVENT

#pragma MESSAGE DISABLE C1420 // Result of function-call is ignored

#pragma DATA_SEG __RPAGE_SEG PAGED_RAM_EVENT
EventTypeItem g_event_types[BMS_EVENT_TYPE_MAX_COUNT];
EventHandlerItem g_event_handlers[BMS_EVENT_HANDLER_MAX_COUNT];
#pragma DATA_SEG DEFAULT

void event_init(void)
{
}

void event_uninit(void)
{
}

Result event_observe(EventTypeId event_id, EventHandler handler, void* user_data)
{
    EventHandlerId avaliable_id = 0;
    EventHandlerId tail_id = 0;
    if (event_id >= BMS_EVENT_TYPE_MAX_COUNT) return ERR_INVALID_ARG;
    if (handler == NULL) return ERR_INVALID_ARG;

    /** find an available handler item */
    avaliable_id = event_handler_find_availabled();
    if (avaliable_id == 0) return RES_ERR;

    /** set the handler & data */
    g_event_handlers[avaliable_id].handler = handler;
    g_event_handlers[avaliable_id].user_data = user_data;

    tail_id = event_handler_find_tail(event_id);

    if (tail_id == 0)//event handler is empty
    {
        g_event_types[event_id].start_handle = avaliable_id;//update the first event handler
    }
    else
    {
        g_event_handlers[tail_id].next = avaliable_id;//update the last event handler
    }

    return RES_OK;
}

Result event_unobserve(EventTypeId event_id, EventHandler handler)
{
    EventHandlerId id = 0;
    EventHandlerId last_id = 0;
    if (event_id >= BMS_EVENT_HANDLER_MAX_COUNT) return ERR_INVALID_ARG;

    id = g_event_types[event_id].start_handle;
    while (id > 0)
    {
        if (g_event_handlers[id].handler == handler) break;//the handler id has been found
        last_id = id;
        id = g_event_handlers[id].next;
    }

    if (id == 0) return RES_OK;//not find

    if (last_id == 0)//the event is the only one
        g_event_types[event_id].start_handle = 0;
    else
        g_event_handlers[last_id].next = g_event_handlers[id].next;

    g_event_handlers[id].handler = NULL;
    g_event_handlers[id].user_data = NULL;
    g_event_handlers[id].next = 0;

    return RES_OK;
}

/**
 *
 * @param event
 * @param event_data
 * @return
 */
Result event_dispatch(EventTypeId event_id, void* event_data)
{
    EventHandlerId id = 0;
    EventHandlerId next_id = 0;
    if (event_id >= BMS_EVENT_HANDLER_MAX_COUNT) return ERR_INVALID_ARG;

    id = g_event_types[event_id].start_handle;

    while (id > 0)
    {
        next_id = g_event_handlers[id].next;
        g_event_handlers[id].handler(event_id, event_data, g_event_handlers[id].user_data);
        id = next_id;
    }

    return RES_OK;
}

Result event_deliver(EventTypeId event)
{
    return event_dispatch(event, NULL);
}

EventHandlerId event_handler_find_tail(EventTypeId id)
{
    EventHandlerId next_id = g_event_types[id].start_handle;
    while (next_id > 0)
    {
        if (g_event_handlers[next_id].next == 0)
            break;
        next_id = g_event_handlers[next_id].next;
    }
    return next_id;
}

EventHandlerId event_handler_find_availabled(void)
{
    EventHandlerId index = 0;
    for (index = 1; index < BMS_EVENT_HANDLER_MAX_COUNT; ++index)
    {
        if (g_event_handlers[index].handler == NULL)
            return index;
    }

    return 0;
}


#endif

