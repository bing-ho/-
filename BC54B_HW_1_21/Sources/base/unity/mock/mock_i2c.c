#include "mock_i2c.h"
#include "unity.h"

void mock_i2c_expect_read(const mock_i2c_t *__FAR i2c, unsigned char addr, const unsigned char *__FAR dat, unsigned char len) {
    addr = addr << 1;
    addr += 1;
    TEST_ASSERT_EQUAL(1, RingBuffer_Write(i2c->buf, &addr, 1));
    TEST_ASSERT_EQUAL(len, RingBuffer_Write(i2c->buf, dat, len));
}

void mock_i2c_expect_write(const mock_i2c_t *__FAR i2c, unsigned char addr, const unsigned char *__FAR dat, unsigned char len) {
    addr = addr << 1;
    TEST_ASSERT_EQUAL(1, RingBuffer_Write(i2c->buf, &addr, 1));
    TEST_ASSERT_EQUAL(len, RingBuffer_Write(i2c->buf, dat, len));
}


void mock_i2c_init(const mock_i2c_t *__FAR i2c) {
    RingBuffer_Clear(i2c->buf);
}

unsigned char mock_i2c_write(const mock_i2c_t *__FAR i2c, unsigned char addr, const unsigned char *__FAR dat, unsigned char len) {
    unsigned char left = len;
    unsigned char rdat[10];
    TEST_ASSERT_EQUAL(1, RingBuffer_Read(i2c->buf, rdat, 1));
    TEST_ASSERT_EQUAL(rdat[0], (addr << 1));
    while (left > 0) {
        unsigned char thisLen = left > sizeof(rdat) ? sizeof(rdat) : left;
        TEST_ASSERT_EQUAL(thisLen, RingBuffer_Read(i2c->buf, rdat, thisLen));
        TEST_ASSERT_EQUAL_INT8_ARRAY(rdat, dat, thisLen);
        left -= thisLen;
        dat += thisLen;
    }
    return len;
}

unsigned char mock_i2c_read(const mock_i2c_t *__FAR i2c, unsigned char addr, unsigned char *__FAR dat, unsigned char len) {
    unsigned char left = len;
    unsigned char rdat[10];
    TEST_ASSERT_EQUAL(1, RingBuffer_Read(i2c->buf, rdat, 1));
    TEST_ASSERT_EQUAL(rdat[0], (addr << 1) + 1);
    TEST_ASSERT_EQUAL(len, RingBuffer_Read(i2c->buf, dat, len));
    return len;
}

void mock_i2c_assert_finished(const mock_i2c_t *__FAR i2c) {
    TEST_ASSERT_EQUAL(0, i2c->buf->len);
}
