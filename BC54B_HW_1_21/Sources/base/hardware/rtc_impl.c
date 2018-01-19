#include "rtc_impl.h"
#include "sd2405al_impl.h"
#include "ucos_ii.h"

static unsigned char test_mode;
static unsigned long fake_time;
static unsigned long tick_update_fake_time;

static char is_valid_mode(unsigned char mode) {
    return mode < RTC_TEST_MODE_MAX;
}

char rtc_init(void) {
    test_mode = RTC_TEST_NONE;
    fake_time = 0;
    tick_update_fake_time = 0;

    return sd2405_init(sd2405);
}

char rtc_set_test(unsigned char mode) {
    if (!is_valid_mode(mode)) {
        return 0;
    }
    test_mode = mode;
}

char rtc_write_time(unsigned long time) {
    if (test_mode == RTC_TEST_NONE) {
        return sd2405_write_time(sd2405, time);
    } else {
        fake_time = time;
        tick_update_fake_time = OSTimeGet();
        return 1;
    }
}

char rtc_read_time(unsigned long *__FAR time) {
    if (test_mode == RTC_TEST_NONE) {
        return sd2405_read_time(sd2405, time);
    } else if (test_mode == RTC_TEST_FAKE_RUN) {
        unsigned long tick_now = OSTimeGet();
        fake_time += (tick_now - tick_update_fake_time + OS_TICKS_PER_SEC / 2) / OS_TICKS_PER_SEC;
        tick_update_fake_time = tick_now;
    }
    if (time) {
        *time = fake_time;
    }
    return 1;
}

