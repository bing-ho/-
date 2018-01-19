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


/// \brief mock_i2c_init 初始化总线.
///
/// \param i2c 底层接口.
void mock_i2c_init(const mock_i2c_t *__FAR i2c);

/// \brief mock_i2c_write 通过I2C总线写数据.
///
/// \param i2c 底层接口.
/// \param addr 从设备的地址, 最终在总线上的第一个字节为((addr << 1) + RW)
/// \param dat 需要写的数据.
/// \param len 需要写的数据的长度.
///
/// \return 从设备应答的数据长度.
unsigned char mock_i2c_write(const mock_i2c_t *__FAR i2c, unsigned char addr, const unsigned char *__FAR dat, unsigned char len);

/// \brief mock_i2c_read 通过I2C总线读数据.
///
/// \param i2c 底层接口.
/// \param addr 从设备的地址, 最终在总线上的第一个字节为((addr << 1) + RW)
/// \param dat 读到的数据保存的地址.
/// \param len 需要读的数据的长度.
///
/// \return 读取到的数据长度.
unsigned char mock_i2c_read(const mock_i2c_t *__FAR i2c, unsigned char addr, unsigned char *__FAR dat, unsigned char len);


#endif
