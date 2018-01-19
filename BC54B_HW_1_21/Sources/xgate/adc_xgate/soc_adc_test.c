#include "adc_xgate.h"
#include "unity_fixture.h"
#include "ucos_ii.h"


#if 0
TEST_GROUP(soc_adc);

TEST_SETUP(soc_adc) {
}


TEST_TEAR_DOWN(soc_adc) {
}

static const struct soc_adc_param param = {
    2, // 1ms
    40, // 40次滤波
    10, // 丢弃10个最大值
    10, // 丢弃10个最小值
};


static unsigned long tick;

TEST(soc_adc, get_soc_adc_value) {
    uint16_t c1;
    uint16_t c2;
    uint16_t c3;
    TEST_ASSERT(soc_adc_init());
    TEST_ASSERT(soc_adc_start(&param));

    for (;;) {
        TEST_ASSERT(soc_wait(&c1, &c2,&c3))
        tick = OSTimeGet();
        (void)printf("got_soc: %d %d\n", c1, c2);
    }
}


TEST_GROUP_RUNNER(soc_adc) {
    RUN_TEST_CASE(soc_adc, get_soc_adc_value);
}
#endif 

