#include "sd2405al_impl.h"
#include "unity_fixture.h"

TEST_GROUP(sd2405al);

TEST_SETUP(sd2405al) {
}

TEST_TEAR_DOWN(sd2405al) {
}


TEST(sd2405al, write_read) {
    unsigned char i;
    unsigned long w = 10000;
    unsigned long r = 0;
    unsigned long lr = 0;

    TEST_ASSERT(sd2405_write_time(sd2405_impl, w));
    TEST_ASSERT(sd2405_read_time(sd2405_impl, &r));
    TEST_ASSERT_INT32_WITHIN(1, w, r);
    TEST_ASSERT(sd2405_read_time(sd2405_impl, &r));

    lr = r;
    for (i = 0; i < 5; ++i) {
        TEST_ASSERT(sd2405_read_time(sd2405_impl, &r));
        TEST_ASSERT_INT32_WITHIN(1, r, lr);
        lr = r;
    }
}

TEST(sd2405al, data_store) {
    unsigned char dat_w[12];
    unsigned char dat_r[12];
    char i;

    for (i = 0; i < sizeof(dat_w); ++i) {
        dat_w[i] = i * 2 + 2;
        dat_r[i] = i;
    }

    TEST_ASSERT(sd2405_store_data(sd2405_impl, dat_w, 0, 12));
    TEST_ASSERT(sd2405_restore_data(sd2405_impl, dat_r, 0, 12));
    TEST_ASSERT_EQUAL_UINT8_ARRAY(dat_w, dat_r, sizeof(dat_w));

    dat_w[5] = 88;
    dat_w[6] = 99;
    dat_w[7] = 100;
    for (i = 0; i < sizeof(dat_r); ++i) {
        dat_r[0] = 0;
    }

    TEST_ASSERT(sd2405_store_data(sd2405_impl, &dat_w[5], 5, 3));
    TEST_ASSERT(sd2405_restore_data(sd2405_impl, dat_r, 0, 12));
    TEST_ASSERT_EQUAL_UINT8_ARRAY(dat_w, dat_r, sizeof(dat_w));
}

TEST(sd2405al, init) {
    TEST_ASSERT(sd2405_init(sd2405_impl));
}


TEST_GROUP_RUNNER(sd2405al) {
    RUN_TEST_CASE(sd2405al, init);
    RUN_TEST_CASE(sd2405al, data_store);
    RUN_TEST_CASE(sd2405al, write_read);
}

