/// \file ring_buffer.h
/// \brief 循环Buffer的声明
/// \author Xi Qingping, qingping.xi@ligoo.cn
/// \version
/// \date 2015-08-26

#ifndef __RING_BUFFER_H__
#define __RING_BUFFER_H__

#ifndef RINGBUFFER_SIZE_TYPE
/** 循环Buffer长度的类型. */
#define RINGBUFFER_SIZE_TYPE unsigned int
#endif

#ifndef __FAR
#define __FAR
#endif

#ifndef RINGBUFFER_CHECK
#define RINGBUFFER_CHECK 1
#endif


#define RINGBUFFER_CHECK_MAGIC (0xfade7643UL)


/** 循环Buffer结构体. */
typedef struct ring_buffer {
#if RINGBUFFER_CHECK != 0
    unsigned long magic;
#endif
    RINGBUFFER_SIZE_TYPE cap;
    RINGBUFFER_SIZE_TYPE len;
    RINGBUFFER_SIZE_TYPE in;
    RINGBUFFER_SIZE_TYPE out;
    unsigned char *__FAR buf;
} ring_buffer_t;

/** 定义静态循环Buffer的宏. */
#if RINGBUFFER_CHECK != 0
#define RINGBUFFER_DEFINE_STATIC(name, size)  \
static unsigned char __##name##_buffer[size]; \
static struct ring_buffer name = {            \
    (unsigned long)RINGBUFFER_CHECK_MAGIC,    \
    size,                                     \
    0,                                        \
    0,                                        \
    0,                                        \
    __##name##_buffer,                        \
}
#else
#define RINGBUFFER_DEFINE_STATIC(name, size)  \
static unsigned char __##name##_buffer[size]; \
static struct ring_buffer name = {            \
    size,                                     \
    0,                                        \
    0,                                        \
    0,                                        \
    __##name##_buffer,                        \
}
#endif                                        \

/** 定义循环Buffer的宏. */
#if RINGBUFFER_CHECK != 0
#define RINGBUFFER_DEFINE(name, size)         \
static unsigned char __##name##_buffer[size]; \
ring_buffer_t name = {                        \
    (unsigned long)RINGBUFFER_CHECK_MAGIC,    \
    size,                                     \
    0,                                        \
    0,                                        \
    0,                                        \
    __##name##_buffer,                        \
}
#else
#define RINGBUFFER_DEFINE(name, size)         \
static unsigned char __##name##_buffer[size]; \
ring_buffer_t name = {                        \
    size,                                     \
    0,                                        \
    0,                                        \
    0,                                        \
    __##name##_buffer,                        \
}
#endif                                        \

/// \brief RingBuffer_Clear 清空循环Buffer
///
/// \param rb 需要清空的循环Buffer.
void RingBuffer_Clear(ring_buffer_t *__FAR rb);

/// \brief RingBuffer_PutData 向循环Buffer追加指定长度的相同数据.
///
/// \param rb 需要追加的循环Buffer.
/// \param uch 追加的数据.
/// \param len 追加的长度.
///
/// \return 追加的长度, 如果循环Buffer剩余的空间不足, 可能会 < 传入的长度.
RINGBUFFER_SIZE_TYPE RingBuffer_PutData(ring_buffer_t *__FAR rb, unsigned char uch, RINGBUFFER_SIZE_TYPE len);

/// \brief RingBuffer_TryRead 尝试从循环Buffer读取指定长度的数据.
///
/// \param rb 需要读取的循环Buffer.
/// \param buf 用于保存读取的数据指针.
/// \param len 尝试读取的长度.
///
/// \return 返回已经读取的长度, 如果循环Buffer里保存的数据 < 尝试读取的长度, 该返回值会 < 尝试读取的长度.
RINGBUFFER_SIZE_TYPE RingBuffer_TryRead(ring_buffer_t *__FAR rb, unsigned char *__FAR buf, RINGBUFFER_SIZE_TYPE len);

/// \brief RingBuffer_Read 从循环Buffer读取指定长度的数据.
///
/// \param rb 需要读取的循环Buffer.
/// \param buf 用于保存读取的数据指针.
/// \param len 读取的长度.
///
/// \return 返回读取的长度, 如果循环Buffer里面存数的数据 < 读取的长度, 将返回0, 循环Buffer里面的数据不会被读取.
RINGBUFFER_SIZE_TYPE RingBuffer_Read(ring_buffer_t *__FAR rb, unsigned char *__FAR buf, RINGBUFFER_SIZE_TYPE len);

/// \brief RingBuffer_TryWrite 尝试向循环Buffer读取指定长度的数据.
///
/// \param rb 需要写入的循环Buffer.
/// \param buf 需要写入数据的指针.
/// \param len 尝试写入的长度.
///
/// \return 返回已写入的长度, 如果循环Buffer的空闲空间 < 尝试读取的长度, 该返回值 < 尝试写入的长度.
RINGBUFFER_SIZE_TYPE RingBuffer_TryWrite(ring_buffer_t *__FAR rb, const unsigned char *__FAR dat, RINGBUFFER_SIZE_TYPE len);

/// \brief RingBuffer_Write 向循环Buffer写入指定长度的数据.
///
/// \param rb 需要写入的循环Buffer.
/// \param buf 需要写入的数据指针.
/// \param len 写入的长度.
///
/// \return 返回已写入的长度, 如果循环Buffer的空闲空间 < 尝试读取的长度, 将返回0, 循环Buffer不会写入数据.
RINGBUFFER_SIZE_TYPE RingBuffer_Write(ring_buffer_t *__FAR rb, const unsigned char *__FAR dat, RINGBUFFER_SIZE_TYPE len);

/// \brief RingBuffer_LeftSpace 查询循环Buffer空闲的空间.
///
/// \param rb 需要查询的循环Buffer
///
/// \return 返回空闲的大小.
RINGBUFFER_SIZE_TYPE RingBuffer_LeftSpace(const ring_buffer_t *__FAR rb);

/// \brief RingBuffer_PopString 从循环Buffer里面读取一个字符串.
///
/// \param rb 需要读取的循环Buffer
/// \param buf 保存读取的字符串的指针, 如果该指针为空, 将丢弃读取到的数据.
/// \param bufSize 保存字符串的执行内存的大小, 如果读取的数据长度 > bufSize, 多余的数据将被丢弃.
///
/// \return 保存到buf中的数据长度.
RINGBUFFER_SIZE_TYPE RingBuffer_PopString(ring_buffer_t *__FAR rb, char *__FAR buf, RINGBUFFER_SIZE_TYPE bufSize);

/// \brief RingBuffer_PopStringIsStartWith 从循环Buffer里面读取一个字符串, 并判断该字符串是否以prefix开头.
///
/// \param rb 需要读取的循环Buffer
/// \param prefix 需要判断的开头的字符串.
/// \param buf 保存读取的字符串的指针, 如果该指针为空, 将丢弃读取到的数据.
/// \param bufSize (*bufSize)保存字符串的指向内存的大小, 如果读取的数据长度 > bufSize, 多余的数据将被丢弃. 同时该值还返回保存到buf中的数据的长度.
///
/// \return 1 读取到的字符串是以prefix开头; 0 读取到的字符串不以prefix开头.
char RingBuffer_PopStringIsStartWith(ring_buffer_t *__FAR rb, const char *__FAR prefix, char *__FAR buf, RINGBUFFER_SIZE_TYPE *__FAR size);

RINGBUFFER_SIZE_TYPE RingBuffer_DropDataFromTail(ring_buffer_t *__FAR rb, RINGBUFFER_SIZE_TYPE size);
RINGBUFFER_SIZE_TYPE RingBuffer_DropDataFromHead(ring_buffer_t *__FAR rb, RINGBUFFER_SIZE_TYPE size);

#endif
