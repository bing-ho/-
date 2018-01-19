#include "nandflash_interface.h"
#include "spi_interface.h"
#include "nandflash_hardware.h"
#include "gpio_hardware.h"
#include "spi_hardware.h"

static const struct hcs12_gpio cs_pin = {&PTIS, &PTS, &DDRS, 1 << 7 };

static const struct hcs12_spi_platform this_hcs12_spi = {
    (struct hcs12_spi_regs *)0xD8,  // SPI1
    (0 << 4) | (0 << 0), //baud_reg_value
    1,
    &cs_pin,
};

#define this_spi this_hcs12_spi
#define this_spi_ops hcs12_spi_ops

static const struct spi_bus this_spi_bus = 
{
  &this_spi, &this_spi_ops
};
static struct spi_nand_ctx nand_ctx = {
    0,
    SPI_FLAG_CLK_IDLE_LOW | SPI_FLAG_CLK_FIRST_EDGE,
    NULL
};

static const struct spi_flash_platform nand_platform = {
    &nand_ctx,
    &this_spi_bus,
};

const struct spi_flash nand = {
    NULL,
    &spi_nand_ops,
    &nand_platform
};