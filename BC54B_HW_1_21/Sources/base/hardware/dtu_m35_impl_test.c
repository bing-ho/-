#include "dtu_m35_impl.h"
#include "bms_config.h"
#include "unity_fixture.h"


TEST_GROUP(dtu_m35_impl);

TEST_SETUP(dtu_m35_impl) {
    config_init();
}

TEST_TEAR_DOWN(dtu_m35_impl) {
}

const simcard_data_t test = {
    "1234567890ABCDEFGH01",
    "13812345678",
};

TEST(dtu_m35_impl, nvm) {
    simcard_data_t a;
    TEST_ASSERT(g_DTUM35_BspInterface.nvm_store_data(&test));
    TEST_ASSERT(g_DTUM35_BspInterface.nvm_restore_data(&a));
    TEST_ASSERT_EQUAL_STRING(test.cimi, a.cimi);
    TEST_ASSERT_EQUAL_STRING(test.phone, a.phone);
}

TEST_GROUP_RUNNER(dtu_m35_impl) {
    RUN_TEST_CASE(dtu_m35_impl, nvm);
}

