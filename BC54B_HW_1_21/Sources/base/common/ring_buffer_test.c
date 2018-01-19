#include "ring_buffer.h"
#include "unity_fixture.h"

#pragma DATA_SEG DEFAULT

#define TEST_RINGBUFFER_SIZE 128

RINGBUFFER_DEFINE_STATIC(forTest, TEST_RINGBUFFER_SIZE);

TEST_GROUP(ringbuffer);

TEST_SETUP(ringbuffer) {
}

TEST_TEAR_DOWN(ringbuffer) {
}

TEST(ringbuffer, aaa) {

    unsigned int len;
    char buf1[20];
    char buf2[20];

    (void)memset(buf2, 0x28, sizeof(buf2));

    RingBuffer_Clear(&forTest);
    TEST_ASSERT_EQUAL(TEST_RINGBUFFER_SIZE, RingBuffer_LeftSpace(&forTest));
    TEST_ASSERT_EQUAL(56, RingBuffer_PutData(&forTest, 0x28, 56));
    TEST_ASSERT_EQUAL(TEST_RINGBUFFER_SIZE - 56, RingBuffer_LeftSpace(&forTest));

    for (len = 0; len < 56; ) {
        int read = 56 - len;
        read = read > sizeof(buf1) ? sizeof(buf1) : read;
        TEST_ASSERT_EQUAL(read, RingBuffer_Read(&forTest, (unsigned char *__FAR)buf1, read));
        TEST_ASSERT_EQUAL_UINT8_ARRAY(buf1, buf2, read);
        len += read;
    }


    (void)strcpy(buf1, "Hello world\n");
    TEST_ASSERT_EQUAL(strlen(buf1) + 1, RingBuffer_Write(&forTest, (unsigned char *__FAR)buf1, strlen(buf1) + 1));
    TEST_ASSERT_EQUAL(strlen(buf1) + 1, RingBuffer_PopString(&forTest, buf2, sizeof(buf2)));
    TEST_ASSERT_EQUAL_STRING(buf1, buf2);



    (void)strcpy(buf1, "Hello world\n");
    len = sizeof(buf2);
    TEST_ASSERT_EQUAL(strlen(buf1), RingBuffer_Write(&forTest, (unsigned char *__FAR)buf1, strlen(buf1)));
    TEST_ASSERT(RingBuffer_PopStringIsStartWith(&forTest, "Hello", buf2, &len));
    TEST_ASSERT_EQUAL(strlen(buf1), len);
    TEST_ASSERT_EQUAL_STRING(buf1, buf2);

    (void)strcpy(buf1, "Hello world\n");
    len = sizeof(buf2);
    TEST_ASSERT_EQUAL(strlen(buf1), RingBuffer_Write(&forTest, (unsigned char *__FAR)buf1, strlen(buf1)));
    TEST_ASSERT_FALSE(RingBuffer_PopStringIsStartWith(&forTest, "world", buf2, &len));
    TEST_ASSERT_EQUAL(strlen(buf1), len);
    TEST_ASSERT_EQUAL_STRING(buf1, buf2);
}

TEST_GROUP_RUNNER(ringbuffer) {
    RUN_TEST_CASE(ringbuffer, aaa);
}

