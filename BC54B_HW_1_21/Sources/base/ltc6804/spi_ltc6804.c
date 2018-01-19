/*******************************************************************************
 **                       安徽新能源技术有限公司 Copyright (c)
 **                            http://www...
 **文 件 名:spi_ltc6804.c
 **作    者:
 **创建日期:2016.12.15
 **文件说明:与6804通讯的SPI口定义
 **修改记录:
 **版    本:V1.0
 **备    注:
 *******************************************************************************/
#include "spi_interface.h"

//SPI波特率定义
#define SPI_HIGH_SPEED_BAUD_REG_VALUE  0x53 //500K
#define SPI_LOW_SPEED_BAUD_REG_VALUE   0x55 //125K

//片选引脚定义
static const struct hcs12_gpio ltc6804_cs_pin = {&PTIH, &PTH, &DDRH, 1 << 3 };
//高速通讯
static const struct hcs12_spi_platform ltc6804_high_speed_spi =
{
    (struct hcs12_spi_regs *)0xF0,  // SPI1
    SPI_HIGH_SPEED_BAUD_REG_VALUE, 
    1,
    &ltc6804_cs_pin,
};
//低速通讯
static const struct hcs12_spi_platform ltc6804_low_speed_spi =
{
    (struct hcs12_spi_regs *)0xF0,  // SPI1
    SPI_LOW_SPEED_BAUD_REG_VALUE, 
    1,
    &ltc6804_cs_pin,
};
//高速总线
const struct spi_bus ltc6804_high_speed_spi_bus = 
{
  &ltc6804_high_speed_spi, &hcs12_spi_ops
};
//低速总线
const struct spi_bus ltc6804_low_speed_spi_bus = 
{
  &ltc6804_low_speed_spi, &hcs12_spi_ops
};
