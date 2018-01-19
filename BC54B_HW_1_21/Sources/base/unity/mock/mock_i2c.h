#ifndef __MOCK_I2C_H__
#define __MOCK_I2C_H__

#include "ring_buffer.h"

#ifndef __FAR
#define __FAR
#endif

typedef struct mock_i2c {
    ring_buffer_t *__FAR buf;
} mock_i2c_t;

#define MOCK_I2C_DEFINE_STATIC(name, ringbuffer) \
static mock_i2c_t name = { \
	ringbuffer, \
}

#define MOCK_I2C_DEFINE(name, ringbuffer) \
mock_i2c_t name = { \
	ringbuffer, \
}


void mock_i2c_expect_read(const mock_i2c_t *__FAR i2c, unsigned char addr, const unsigned char *__FAR dat, unsigned char len);
void mock_i2c_expect_write(const mock_i2c_t *__FAR i2c, unsigned char addr, const unsigned char *__FAR dat, unsigned char len);
void mock_i2c_assert_finished(const mock_i2c_t *__FAR i2c);


/// \brief mock_i2c_init ��ʼ������.
///
/// \param i2c �ײ�ӿ�.
void mock_i2c_init(const mock_i2c_t *__FAR i2c);

/// \brief mock_i2c_write ͨ��I2C����д����.
///
/// \param i2c �ײ�ӿ�.
/// \param addr ���豸�ĵ�ַ, �����������ϵĵ�һ���ֽ�Ϊ((addr << 1) + RW)
/// \param dat ��Ҫд������.
/// \param len ��Ҫд�����ݵĳ���.
///
/// \return ���豸Ӧ������ݳ���.
unsigned char mock_i2c_write(const mock_i2c_t *__FAR i2c, unsigned char addr, const unsigned char *__FAR dat, unsigned char len);

/// \brief mock_i2c_read ͨ��I2C���߶�����.
///
/// \param i2c �ײ�ӿ�.
/// \param addr ���豸�ĵ�ַ, �����������ϵĵ�һ���ֽ�Ϊ((addr << 1) + RW)
/// \param dat ���������ݱ���ĵ�ַ.
/// \param len ��Ҫ�������ݵĳ���.
///
/// \return ��ȡ�������ݳ���.
unsigned char mock_i2c_read(const mock_i2c_t *__FAR i2c, unsigned char addr, unsigned char *__FAR dat, unsigned char len);


#endif
