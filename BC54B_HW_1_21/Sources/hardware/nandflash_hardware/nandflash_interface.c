#include "nandflash_interface.h"

int spi_flash_init(const struct spi_flash *__FAR flash) {
    return flash->ops->init(flash->platform);
}

void spi_flash_deinit(const struct spi_flash *__FAR flash) {
    flash->ops->deinit(flash->platform);
}

int spi_flash_read_page(const struct spi_flash *__FAR flash, uint32_t page_addr) {
    return flash->ops->read_page(flash->platform, page_addr);
}

int spi_flash_write_page(const struct spi_flash *__FAR flash, uint32_t page_addr) {
    return flash->ops->write_page(flash->platform, page_addr);
}

int spi_flash_erase_block(const struct spi_flash *__FAR flash, uint32_t page_addr) {
    return flash->ops->erase_block(flash->platform, page_addr);
}

ssize_t spi_flash_read_page_data(const struct spi_flash *__FAR flash, uint16_t offset, uint8_t *__FAR buffer, uint16_t size) {
    return flash->ops->read_page_data(flash->platform, offset, buffer, size);
}

ssize_t spi_flash_write_page_data(const struct spi_flash *__FAR flash, uint16_t offset, const uint8_t *__FAR buffer, uint16_t size) {
    return flash->ops->write_page_data(flash->platform, offset, buffer, size);
}

const void *__FAR spi_flash_get_chipinfo(const struct spi_flash *__FAR flash) {
    return flash->ops->get_chipinfo(flash->platform);
}
