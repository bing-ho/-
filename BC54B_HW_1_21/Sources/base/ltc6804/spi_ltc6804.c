/*******************************************************************************
 **                       ��������Դ�������޹�˾ Copyright (c)
 **                            http://www...
 **�� �� ��:spi_ltc6804.c
 **��    ��:
 **��������:2016.12.15
 **�ļ�˵��:��6804ͨѶ��SPI�ڶ���
 **�޸ļ�¼:
 **��    ��:V1.0
 **��    ע:
 *******************************************************************************/
#include "spi_interface.h"

//SPI�����ʶ���
#define SPI_HIGH_SPEED_BAUD_REG_VALUE  0x53 //500K
#define SPI_LOW_SPEED_BAUD_REG_VALUE   0x55 //125K

//Ƭѡ���Ŷ���
static const struct hcs12_gpio ltc6804_cs_pin = {&PTIH, &PTH, &DDRH, 1 << 3 };
//����ͨѶ
static const struct hcs12_spi_platform ltc6804_high_speed_spi =
{
    (struct hcs12_spi_regs *)0xF0,  // SPI1
    SPI_HIGH_SPEED_BAUD_REG_VALUE, 
    1,
    &ltc6804_cs_pin,
};
//����ͨѶ
static const struct hcs12_spi_platform ltc6804_low_speed_spi =
{
    (struct hcs12_spi_regs *)0xF0,  // SPI1
    SPI_LOW_SPEED_BAUD_REG_VALUE, 
    1,
    &ltc6804_cs_pin,
};
//��������
const struct spi_bus ltc6804_high_speed_spi_bus = 
{
  &ltc6804_high_speed_spi, &hcs12_spi_ops
};
//��������
const struct spi_bus ltc6804_low_speed_spi_bus = 
{
  &ltc6804_low_speed_spi, &hcs12_spi_ops
};
