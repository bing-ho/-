#include "async.h"
#include "async_timer.h"
#include "async_event_call.h"
#include "unity_fixture.h"
#include "ucos_ii.h"


TEST_GROUP(async);

unsigned short h;
unsigned short w;

ASYNC_TIMER_TIMEOUT_TYPE hello(async_timer_t *__FAR t) {
    static unsigned long last_tick = 0;
    unsigned short *__FAR p = async_timer_get_data(t);

    unsigned long now = OSTimeGet();
    unsigned long diff = now - last_tick;

    (void)printf("hello %d, escaped: %d, expect %d\n", *p, (unsigned short)diff, (*p) * 100);
    *p += 1;
    last_tick = OSTimeGet();
    return (*p) * 100;
}

ASYNC_TIMER_TIMEOUT_TYPE world(async_timer_t *__FAR t) {
    static unsigned long last_tick = 0;
    unsigned short *__FAR p = async_timer_get_data(t);

    unsigned long now = OSTimeGet();
    unsigned long diff = now - last_tick;

    (void)printf("world %d, escaped: %d, expect %d\n", *p, (unsigned short)diff, (*p) * 200);
    *p += 2;
    last_tick = OSTimeGet();
    return (*p) * 200;
}


TEST_SETUP(async) {
}

TEST_TEAR_DOWN(async) {
}


TEST(async, timer) {
    (void)async_timer_register(hello, 100, &h);
    (void)async_timer_register(&world, 200, &w);
}


char event_call_hello(void *__FAR dat) {
    static int i = 0;
    (void)dat;
    ++i;
    (void)printf("event_call_hello  %d\n", i);
    return 1;
}


ASYNC_TIMER_TIMEOUT_TYPE trigger(async_timer_t *__FAR t) {
    async_event_call_t *__FAR call = async_timer_get_data(t);
    (void)async_event_call_trigger(call);
    return 200;
}

TEST(async, event_call) {
    async_event_call_t *__FAR call = async_event_call_post(event_call_hello, 0);
    (void)async_timer_register(trigger, 200, call);
}

TEST_GROUP_RUNNER(async) {
    async_init();
    RUN_TEST_CASE(async, timer);
    RUN_TEST_CASE(async, event_call);
    for(;;) {
        OSTimeDly(100);
    }
}

