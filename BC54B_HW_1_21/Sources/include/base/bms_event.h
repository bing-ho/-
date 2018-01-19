/**
*
* Copyright (c) 2012 Ligoo Inc.
*
* @file bms_event.h
* @brief
* @note
* @author
* @date 2012-5-16
*
*/

#ifndef BMS_EVENT_H_
#define BMS_EVENT_H_
#include "bms_defs.h"

#ifndef BMS_SUPPORT_EVENT
#define BMS_SUPPORT_EVENT       1
#endif

#ifndef BMS_EVENT_HANDLER_MAX_COUNT
#define BMS_EVENT_HANDLER_MAX_COUNT     100
#endif

#ifndef BMS_EVENT_TYPE_MAX_COUNT
#define BMS_EVENT_TYPE_MAX_COUNT        100
#endif

typedef INT8U EventHandlerId;
typedef INT8U EventTypeId;

typedef Result (*EventHandler)(EventTypeId event, void* event_data, void* user_data);
typedef void (*EventBareHandler)(void);

/**
 *
 */
void event_init(void);
/**
 *
 */
void event_uninit(void);

/**
 *
 * @param event
 * @param user_data
 * @return
 */
Result event_observe(EventTypeId event, EventHandler handler, void* user_data);
/**
 *
 * @param event
 * @return
 */
Result event_unobserve(EventTypeId event, EventHandler handler);

/**
 *
 * @param event
 * @param event_data
 * @return
 */
Result event_dispatch(EventTypeId event, void* event_data);

Result event_deliver(EventTypeId event);


#endif /* BMS_EVENT_H_ */
