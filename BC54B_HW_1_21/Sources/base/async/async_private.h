#ifndef __ASYNC_PRIVATE_H__
#define __ASYNC_PRIVATE_H__

#ifndef ASYNC_HELP_TASK_STACK_SIZE
#define ASYNC_HELP_TASK_STACK_SIZE 320
#endif


#ifndef ASYNC_TIMER_SIZE
#define ASYNC_TIMER_SIZE 20
#endif


#ifndef ASYNC_EVENT_CALL_SIZE
#define ASYNC_EVENT_CALL_SIZE 20
#endif

#include "async_event_call.h"
#include "async_timer.h"

struct async_sem_private {
    unsigned char type;
    union {
        async_event_call_t *__FAR event_call;
        async_timer_t *__FAR timer;
    } data;
};

#define PRIVATE_SEM_TYPE_TRIGGER_EVENT_CALL 0
#define PRIVATE_SEM_TYPE_ADD_TIMER 1
#define PRIVATE_SEM_TYPE_EXEC_TIMER

void async_timer_init(void);
void async_event_call_init(void);

char async_post_sem(struct async_sem_private *priv);

ASYNC_TIMER_TIMEOUT_TYPE async_timer_exec(ASYNC_TIMER_TIMEOUT_TYPE escaped);
ASYNC_TIMER_TIMEOUT_TYPE async_timer_add_timer(async_timer_t *__FAR timer, ASYNC_TIMER_TIMEOUT_TYPE escaped);

void async_event_call_exec(async_event_call_t *__FAR event_call);


#pragma DATA_SEG __RPAGE_SEG PAGED_RAM

#endif
