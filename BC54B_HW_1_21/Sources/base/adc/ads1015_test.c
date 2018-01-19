#if  0

#include "ads1015_intermediate.h"
#include "unity_fixture.h"
#include "ring_buffer.h"
#include "ucos_ii.h"

#include "mock_i2c.h"



#include "bms_job.h"
#include "bms_util.h"
#define memset safe_memset
#define memcmp safe_memcmp
#define memcpy safe_memcpy
#define strcpy safe_strcpy
#define strlen safe_strlen
#define strcmp safe_strcmp


#pragma DATA_SEG __RPAGE_SEG PAGED_RAM


static void init(const ads1015_platform_t *__FAR platform) {
    mock_i2c_t *__FAR i2c = platform->platform_pdata;
    mock_i2c_init(i2c);
}

static unsigned char i2c_write(const ads1015_platform_t *__FAR platform, const unsigned char *__FAR dat, unsigned char len) {
    mock_i2c_t *__FAR i2c = platform->platform_pdata;
    return mock_i2c_write(i2c, 0x00, dat, len);
}

static unsigned char i2c_read(const ads1015_platform_t *__FAR platform, unsigned char *__FAR dat, unsigned char len) {
    mock_i2c_t *__FAR i2c = platform->platform_pdata;
    return mock_i2c_read(i2c, 0x00, dat, len);
}

static void delay_ms(const ads1015_platform_t *__FAR platform, unsigned short howmany) {
    (void)platform;
    OSTimeDly(howmany);
}

// do not support isr
RINGBUFFER_DEFINE_STATIC(i2c_mock_buf, 200);
MOCK_I2C_DEFINE_STATIC(i2c, &i2c_mock_buf);



const ads1015_platform_t ads1015_platform = {
    &i2c,
    init,
    i2c_write,
    i2c_read,
    delay_ms,
    NULL,
};
ads1015_t thisads1015 = {
#if ADS1015_CHECK != 0
    ADS1015_CHECK_MAGIC,
#endif
    &ads1015_platform,
};

// do not support isr
RINGBUFFER_DEFINE_STATIC(i2c_mock_buf_isr, 200);
MOCK_I2C_DEFINE_STATIC(i2c_isr, &i2c_mock_buf_isr);

static OS_EVENT *mEvent;

static char wait_isr(const ads1015_platform_t *__FAR platform, unsigned short ms) {
    unsigned char err;
    (void)platform;
    OSSemSet(mEvent, 0, &err);

    OSSemPend(mEvent, ms, &err);
    return err == OS_ERR_NONE;

}


const ads1015_platform_t ads1015_platform_isr = {
    &i2c_isr,
    init,
    i2c_write,
    i2c_read,
    delay_ms,
    wait_isr,
};
ads1015_t thisads1015_isr = {
#if ADS1015_CHECK != 0
    ADS1015_CHECK_MAGIC,
#endif
    &ads1015_platform_isr,
};


TEST_GROUP(ads1015);

TEST_SETUP(ads1015) {
    mEvent = OSSemCreate(0);
    ads1015_init(&thisads1015);
    ads1015_init(&thisads1015_isr);
}

TEST_TEAR_DOWN(ads1015) {
}

TEST(ads1015, single_convert) {
    unsigned short val;
    const unsigned char w1[] = {0x01, 0x43, 0x03};
    const unsigned char w2[] = {0x01, 0xC3, 0x03};
    const unsigned char w3[] = {0x01};
    const unsigned char r3[] = {0x43,  0x03};
    const unsigned char w4[] = {0x01};
    const unsigned char r4[] = {0x43,  0x03};
    const unsigned char w5[] = {0x01};
    const unsigned char r5[] = {0x43,  0x03};
    const unsigned char w6[] = {0x01};
    const unsigned char r6[] = {0xC3,  0x03};
    const unsigned char w7[] = {0x00};
    const unsigned char r7[] = {0x00,  0x8F};
    mock_i2c_expect_write(&i2c, 0x00, w1, sizeof(w1));
    mock_i2c_expect_write(&i2c, 0x00, w2, sizeof(w2));
    mock_i2c_expect_write(&i2c, 0x00, w3, sizeof(w3));
    mock_i2c_expect_read(&i2c, 0x00, r3, sizeof(r3));
    mock_i2c_expect_write(&i2c, 0x00, w4, sizeof(w4));
    mock_i2c_expect_read(&i2c, 0x00, r4, sizeof(r4));
    mock_i2c_expect_write(&i2c, 0x00, w5, sizeof(w5));
    mock_i2c_expect_read(&i2c, 0x00, r5, sizeof(r5));
    mock_i2c_expect_write(&i2c, 0x00, w6, sizeof(w6));
    mock_i2c_expect_read(&i2c, 0x00, r6, sizeof(r6));
    mock_i2c_expect_write(&i2c, 0x00, w7, sizeof(w7));
    mock_i2c_expect_read(&i2c, 0x00, r7, sizeof(r7));
    val = ads1015_single_convert(&thisads1015, ADS1015_CHANNEL_0, ADS1015_FULL_SCALE_4096_MV);
    TEST_ASSERT_EQUAL(0x008F, val);
    mock_i2c_assert_finished(&i2c);
}


static void isr(void *aa) {
    (void)aa;
    (void)OSSemPost(mEvent);
}

TEST(ads1015, single_convert_isr) {
    JobId id;
    unsigned short val;
    const unsigned char w1[] = {0x01, 0x43, 0x03};
    const unsigned char w2[] = {0x01, 0xC3, 0x03};
    const unsigned char w7[] = {0x00};
    const unsigned char r7[] = {0x00,  0x8F};
    mock_i2c_expect_write(&i2c_isr, 0x00, w1, sizeof(w1));
    mock_i2c_expect_write(&i2c_isr, 0x00, w2, sizeof(w2));
    mock_i2c_expect_write(&i2c_isr, 0x00, w7, sizeof(w7));
    mock_i2c_expect_read(&i2c_isr, 0x00, r7, sizeof(r7));

    id = job_schedule(MAIN_JOB_GROUP, BCU_UPDATE_JOB_PERIODIC, isr, NULL);

    val = ads1015_single_convert(&thisads1015_isr, ADS1015_CHANNEL_0, ADS1015_FULL_SCALE_4096_MV);
    TEST_ASSERT_EQUAL(0x008F, val);
    mock_i2c_assert_finished(&i2c_isr);
    job_cancel(MAIN_JOB_GROUP, id);
}


TEST_GROUP_RUNNER(ads1015) {
    RUN_TEST_CASE(ads1015, single_convert);
    RUN_TEST_CASE(ads1015, single_convert_isr);
}

#endif 

