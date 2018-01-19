#include "async_private.h"
#include "ring_buffer.h"
#include "ucos_ii.h"

static OS_EVENT *mEvent;
RINGBUFFER_DEFINE_STATIC(sem_data_buffer, 100);


char async_post_sem(struct async_sem_private *priv) {
    RINGBUFFER_SIZE_TYPE ret;
    OS_INIT_CRITICAL();

    OS_ENTER_CRITICAL();
    ret = RingBuffer_Write(&sem_data_buffer, (unsigned char *__FAR)priv, sizeof(*priv));
    OS_EXIT_CRITICAL();

    if (ret != sizeof(*priv)) {
        return 0;
    }

    return OS_ERR_NONE == OSSemPost(mEvent);
}

static void task(void *nouse) {
    unsigned char tmp;
    unsigned long pre_time;
    unsigned short escaped;
    ASYNC_TIMER_TIMEOUT_TYPE wait_time = -1;
    OS_INIT_CRITICAL();

    (void) nouse;

    for(;;) {
        pre_time = OSTimeGet();
        OSSemPend(mEvent, wait_time, &tmp);
        if (tmp != OS_ERR_NONE) {
            goto __update_wait_time;
        } else {
            struct async_sem_private priv;
            OS_ENTER_CRITICAL();
            tmp = (unsigned char)RingBuffer_Read(&sem_data_buffer, (unsigned char *__FAR)&priv, sizeof(priv));
            OS_EXIT_CRITICAL();

            if (tmp != sizeof(priv)) {
                OS_ENTER_CRITICAL();
                RingBuffer_Clear(&sem_data_buffer);
                OSSemSet(mEvent, 0, &tmp);
                OS_EXIT_CRITICAL();
                goto __update_wait_time;
            }

            if (priv.type == PRIVATE_SEM_TYPE_TRIGGER_EVENT_CALL) {
                async_event_call_exec(priv.data.event_call);
                goto __update_wait_time;
            } else if (priv.type == PRIVATE_SEM_TYPE_ADD_TIMER) {
                escaped = (unsigned short)(OSTimeGet() - pre_time);
                wait_time = async_timer_add_timer(priv.data.timer, escaped);
                continue;
            }

        }
__update_wait_time:
        escaped = (unsigned short)(OSTimeGet() - pre_time);
        if (wait_time <= escaped) {
            wait_time = async_timer_exec(escaped);
        } else {
            wait_time -= escaped;
        }

    }
}

#pragma push
#pragma DATA_SEG DEFAULT
static INT8U mTaskStack[ASYNC_HELP_TASK_STACK_SIZE];
#pragma pop

void async_init(void) {
    mEvent = OSSemCreate(0);
    async_event_call_init();
    async_timer_init();
    (void)OSTaskCreate(task, (void *)0, (OS_STK *)&mTaskStack[sizeof(mTaskStack) - 1], RES_SCAN_TASK_PRIO);
}

