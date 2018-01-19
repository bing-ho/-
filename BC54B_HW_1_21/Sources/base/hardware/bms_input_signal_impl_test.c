#include "bms_input_signal_impl.h"
#include "unity_fixture.h"

TEST_GROUP(input_signal);

TEST_SETUP(input_signal) {
    input_signal_init();
}

TEST_TEAR_DOWN(input_signal) {
}

TEST(input_signal, read) {
    unsigned char i;
    for (i = 1; i <= input_signal_get_count(); ++i) {
        TEST_ASSERT_EQUAL(input_signal_is_low(i), !input_signal_is_high(i));
    }
}

TEST(input_signal, input_get_count) {
    TEST_ASSERT_EQUAL(BMS_INPUT_SIGNAL_MAX_COUNT, input_signal_get_count()); 
}


TEST_GROUP_RUNNER(input_signal) {
    RUN_TEST_CASE(input_signal, input_get_count);
    RUN_TEST_CASE(input_signal, read);
}

