#include "rtc_impl.h"
#include "unity_fixture.h"

TEST_GROUP(rtc);

TEST_SETUP(rtc) {
}

TEST_TEAR_DOWN(rtc) {
}


TEST(rtc, test_fake_run) {
    unsigned long t = 1000000;
    unsigned long n;
    TEST_ASSERT(rtc_set_test(RTC_TEST_FAKE_RUN));
    TEST_ASSERT(rtc_write_time(t));
    OSTimeDly(3000); // delay 3 second
    TEST_ASSERT(rtc_read_time(&n));
    TEST_ASSERT_EQUAL(t + 3, n);
    

    TEST_ASSERT(rtc_write_time(t));
    OSTimeDly(1800); // delay 1.8 second
    TEST_ASSERT(rtc_read_time(&n));
    TEST_ASSERT_EQUAL(t + 2, n);
}

TEST(rtc, test_fake_stop) {
    unsigned long t = 1000000;
    unsigned long n;
    TEST_ASSERT(rtc_set_test(RTC_TEST_FAKE_STOP));
    TEST_ASSERT(rtc_write_time(t));
    OSTimeDly(3000); // delay 3 second
    TEST_ASSERT(rtc_read_time(&n));
    TEST_ASSERT_EQUAL(t, n);
    

    TEST_ASSERT(rtc_write_time(t));
    OSTimeDly(1800); // delay 1.8 second
    TEST_ASSERT(rtc_read_time(&n));
    TEST_ASSERT_EQUAL(t, n);
}


TEST_GROUP_RUNNER(rtc) {
    RUN_TEST_CASE(rtc, test_fake_run);
    RUN_TEST_CASE(rtc, test_fake_stop);
}

