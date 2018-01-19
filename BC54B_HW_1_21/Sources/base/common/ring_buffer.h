/// \file ring_buffer.h
/// \brief ѭ��Buffer������
/// \author Xi Qingping, qingping.xi@ligoo.cn
/// \version
/// \date 2015-08-26

#ifndef __RING_BUFFER_H__
#define __RING_BUFFER_H__

#ifndef RINGBUFFER_SIZE_TYPE
/** ѭ��Buffer���ȵ�����. */
#define RINGBUFFER_SIZE_TYPE unsigned int
#endif

#ifndef __FAR
#define __FAR
#endif

#ifndef RINGBUFFER_CHECK
#define RINGBUFFER_CHECK 1
#endif


#define RINGBUFFER_CHECK_MAGIC (0xfade7643UL)


/** ѭ��Buffer�ṹ��. */
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

/** ���徲̬ѭ��Buffer�ĺ�. */
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

/** ����ѭ��Buffer�ĺ�. */
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

/// \brief RingBuffer_Clear ���ѭ��Buffer
///
/// \param rb ��Ҫ��յ�ѭ��Buffer.
void RingBuffer_Clear(ring_buffer_t *__FAR rb);

/// \brief RingBuffer_PutData ��ѭ��Buffer׷��ָ�����ȵ���ͬ����.
///
/// \param rb ��Ҫ׷�ӵ�ѭ��Buffer.
/// \param uch ׷�ӵ�����.
/// \param len ׷�ӵĳ���.
///
/// \return ׷�ӵĳ���, ���ѭ��Bufferʣ��Ŀռ䲻��, ���ܻ� < ����ĳ���.
RINGBUFFER_SIZE_TYPE RingBuffer_PutData(ring_buffer_t *__FAR rb, unsigned char uch, RINGBUFFER_SIZE_TYPE len);

/// \brief RingBuffer_TryRead ���Դ�ѭ��Buffer��ȡָ�����ȵ�����.
///
/// \param rb ��Ҫ��ȡ��ѭ��Buffer.
/// \param buf ���ڱ����ȡ������ָ��.
/// \param len ���Զ�ȡ�ĳ���.
///
/// \return �����Ѿ���ȡ�ĳ���, ���ѭ��Buffer�ﱣ������� < ���Զ�ȡ�ĳ���, �÷���ֵ�� < ���Զ�ȡ�ĳ���.
RINGBUFFER_SIZE_TYPE RingBuffer_TryRead(ring_buffer_t *__FAR rb, unsigned char *__FAR buf, RINGBUFFER_SIZE_TYPE len);

/// \brief RingBuffer_Read ��ѭ��Buffer��ȡָ�����ȵ�����.
///
/// \param rb ��Ҫ��ȡ��ѭ��Buffer.
/// \param buf ���ڱ����ȡ������ָ��.
/// \param len ��ȡ�ĳ���.
///
/// \return ���ض�ȡ�ĳ���, ���ѭ��Buffer������������� < ��ȡ�ĳ���, ������0, ѭ��Buffer��������ݲ��ᱻ��ȡ.
RINGBUFFER_SIZE_TYPE RingBuffer_Read(ring_buffer_t *__FAR rb, unsigned char *__FAR buf, RINGBUFFER_SIZE_TYPE len);

/// \brief RingBuffer_TryWrite ������ѭ��Buffer��ȡָ�����ȵ�����.
///
/// \param rb ��Ҫд���ѭ��Buffer.
/// \param buf ��Ҫд�����ݵ�ָ��.
/// \param len ����д��ĳ���.
///
/// \return ������д��ĳ���, ���ѭ��Buffer�Ŀ��пռ� < ���Զ�ȡ�ĳ���, �÷���ֵ < ����д��ĳ���.
RINGBUFFER_SIZE_TYPE RingBuffer_TryWrite(ring_buffer_t *__FAR rb, const unsigned char *__FAR dat, RINGBUFFER_SIZE_TYPE len);

/// \brief RingBuffer_Write ��ѭ��Bufferд��ָ�����ȵ�����.
///
/// \param rb ��Ҫд���ѭ��Buffer.
/// \param buf ��Ҫд�������ָ��.
/// \param len д��ĳ���.
///
/// \return ������д��ĳ���, ���ѭ��Buffer�Ŀ��пռ� < ���Զ�ȡ�ĳ���, ������0, ѭ��Buffer����д������.
RINGBUFFER_SIZE_TYPE RingBuffer_Write(ring_buffer_t *__FAR rb, const unsigned char *__FAR dat, RINGBUFFER_SIZE_TYPE len);

/// \brief RingBuffer_LeftSpace ��ѯѭ��Buffer���еĿռ�.
///
/// \param rb ��Ҫ��ѯ��ѭ��Buffer
///
/// \return ���ؿ��еĴ�С.
RINGBUFFER_SIZE_TYPE RingBuffer_LeftSpace(const ring_buffer_t *__FAR rb);

/// \brief RingBuffer_PopString ��ѭ��Buffer�����ȡһ���ַ���.
///
/// \param rb ��Ҫ��ȡ��ѭ��Buffer
/// \param buf �����ȡ���ַ�����ָ��, �����ָ��Ϊ��, ��������ȡ��������.
/// \param bufSize �����ַ�����ִ���ڴ�Ĵ�С, �����ȡ�����ݳ��� > bufSize, ��������ݽ�������.
///
/// \return ���浽buf�е����ݳ���.
RINGBUFFER_SIZE_TYPE RingBuffer_PopString(ring_buffer_t *__FAR rb, char *__FAR buf, RINGBUFFER_SIZE_TYPE bufSize);

/// \brief RingBuffer_PopStringIsStartWith ��ѭ��Buffer�����ȡһ���ַ���, ���жϸ��ַ����Ƿ���prefix��ͷ.
///
/// \param rb ��Ҫ��ȡ��ѭ��Buffer
/// \param prefix ��Ҫ�жϵĿ�ͷ���ַ���.
/// \param buf �����ȡ���ַ�����ָ��, �����ָ��Ϊ��, ��������ȡ��������.
/// \param bufSize (*bufSize)�����ַ�����ָ���ڴ�Ĵ�С, �����ȡ�����ݳ��� > bufSize, ��������ݽ�������. ͬʱ��ֵ�����ر��浽buf�е����ݵĳ���.
///
/// \return 1 ��ȡ�����ַ�������prefix��ͷ; 0 ��ȡ�����ַ�������prefix��ͷ.
char RingBuffer_PopStringIsStartWith(ring_buffer_t *__FAR rb, const char *__FAR prefix, char *__FAR buf, RINGBUFFER_SIZE_TYPE *__FAR size);

RINGBUFFER_SIZE_TYPE RingBuffer_DropDataFromTail(ring_buffer_t *__FAR rb, RINGBUFFER_SIZE_TYPE size);
RINGBUFFER_SIZE_TYPE RingBuffer_DropDataFromHead(ring_buffer_t *__FAR rb, RINGBUFFER_SIZE_TYPE size);

#endif
