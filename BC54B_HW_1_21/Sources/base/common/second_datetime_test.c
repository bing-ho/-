#include "second_datetime.h"
#include "unity_fixture.h"

/* ipython code
In [1]: import datetime
In [2]: t0=datetime.datetime(2000,1,1,0,0,0)
In [3]: t1=datetime.datetime(2013,12,8,19,35,55)
In [4]: t1.weekday()
Out[4]: 6
In [5]: (t1-t0).total_second()
Out[5]: 439846555.0
*/


TEST_GROUP(second_datetime);

TEST_SETUP(second_datetime) {
}

TEST_TEAR_DOWN(second_datetime) {
}

TEST(second_datetime, from_second_to_datetime) {
    second_t second;
    struct datetime t;

    second = 439846555;
    TEST_ASSERT(second_to_datetime(&t, second));
    TEST_ASSERT_EQUAL(13, t.year);
    TEST_ASSERT_EQUAL(12, t.month);
    TEST_ASSERT_EQUAL(8, t.day);
    TEST_ASSERT_EQUAL(19, t.hour);
    TEST_ASSERT_EQUAL(35, t.minute);
    TEST_ASSERT_EQUAL(55, t.second);
    TEST_ASSERT_EQUAL(6, t.weekday);
}


TEST(second_datetime, from_datetime_to_second) {
    second_t second = 439846555;
    struct datetime t = {
        13,
        12,
        8,
        6,
        19,
        35,
        55,
    };

    TEST_ASSERT(datetime_to_second(&second, &t));
    TEST_ASSERT_EQUAL(439846555, second);
}

TEST(second_datetime, from_datetime_to_second_with_invalid_datetime) {
    second_t second = 439846555;
    struct datetime t = {
        13,
        12,
        8,
        6,
        19,
        35 - 1, // sub 1 minute
        55 + 60, // add 60 second
    };
    TEST_ASSERT_FALSE(datetime_to_second(&second, &t));
}

TEST(second_datetime, null_paramters) {
    second_t second = 439846555;
    struct datetime t = {
        13,
        12,
        8,
        6,
        19,
        35,
        55,
    };

    TEST_ASSERT_FALSE(datetime_to_second(0, 0));
    TEST_ASSERT_FALSE(datetime_to_second(&second, 0));
    TEST_ASSERT_FALSE(datetime_to_second(0, &t));
    TEST_ASSERT_FALSE(second_to_datetime(0, second));
}


TEST(second_datetime, datetime_is_valid) {
    second_t second;
    struct datetime t;

    second = 439846555;
    TEST_ASSERT(second_to_datetime(&t, second));
    TEST_ASSERT(datetime_is_valid(&t, 0));
    TEST_ASSERT(datetime_is_valid(&t, 1));

    TEST_ASSERT(second_to_datetime(&t, second));
    t.weekday = 7;
    TEST_ASSERT(datetime_is_valid(&t, 0));
    TEST_ASSERT_FALSE(datetime_is_valid(&t, 1));

    TEST_ASSERT(second_to_datetime(&t, second));
    t.hour = 24;
    TEST_ASSERT_FALSE(datetime_is_valid(&t, 0));
    TEST_ASSERT_FALSE(datetime_is_valid(&t, 1));

    TEST_ASSERT(second_to_datetime(&t, second));
    t.month = 13;
    TEST_ASSERT_FALSE(datetime_is_valid(&t, 0));
    TEST_ASSERT_FALSE(datetime_is_valid(&t, 2));
}


TEST_GROUP_RUNNER(second_datetime) {
    RUN_TEST_CASE(second_datetime, from_second_to_datetime);
    RUN_TEST_CASE(second_datetime, from_datetime_to_second);
    RUN_TEST_CASE(second_datetime, from_datetime_to_second_with_invalid_datetime);
    RUN_TEST_CASE(second_datetime, null_paramters);
    RUN_TEST_CASE(second_datetime, datetime_is_valid);
}

