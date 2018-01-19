#include "adc_xgate.h"
#include "unity_fixture.h"
#include "ucos_ii.h"

#if  0

TEST_GROUP(xadc0);

TEST_SETUP(xadc0) {
}


TEST_TEAR_DOWN(xadc0) {
}

static const struct xadc0_param param = {
    2, // 1ms
    40, // 40次滤波
    10, // 丢弃10个最大值
    10, // 丢弃10个最小值
    15, // cahnnel
};


static unsigned long tick;

TEST(xadc0, get_xadc0_value) {
    uint16_t vvv;

    TEST_ASSERT(xadc0_init());

    for (;;) {
        TEST_ASSERT(xadc0_start(&param));
        TEST_ASSERT(xadc0_wait(&vvv, 1000))
        tick = OSTimeGet();
        (void)printf("xadc0: %d\n", vvv);
    }
}


TEST_GROUP_RUNNER(xadc0) {
    RUN_TEST_CASE(xadc0, get_xadc0_value);
}

#endif

