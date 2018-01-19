/*******************************************************************************
**                       安徽力高新能源有限公司 Copyright (c)
**                            http://www.ligoo.cn/ 
** @file	    nandflash_hardware.c                                       

** @brief       1.完成nandflash读、写和擦除功能函数 
                2.完成nandflash模块的初始化
                3.完成nandflash模块的各功能函数结构体的定义
** @copyright		Ligoo Inc.         
** @date		    2017-04-12.
** @author      来翔     
*******************************************************************************/ 
#ifndef __SPI_FLASH_PLATFORM_H__
#define __SPI_FLASH_PLATFORM_H__
                                                     
#include "stdint.h"
#include <stdio.h>
/************************************************
  * @struct  spi_flash_platform
  * @brief   用来描述SpiFlash 通用属性结构体
 ***********************************************/
struct spi_flash_platform {
    void *__FAR private_data;
    const struct spi_bus *__FAR bus;
};
/************************************************
  * @struct  spi_flash_operations
  * @brief   用于描述Nandflash 操作项结构体，接口函数
 ***********************************************/
struct spi_flash_operations {
    int (*init)(const struct spi_flash_platform *__FAR platform);
    void (*deinit)(const struct spi_flash_platform *__FAR platform);
    int (*read_page)(const struct spi_flash_platform *__FAR platform, uint32_t page_addr);
    int (*write_page)(const struct spi_flash_platform *__FAR platform, uint32_t page_addr);
    int (*erase_block)(const struct spi_flash_platform *__FAR platform, uint32_t page_addr);
    ssize_t (*read_page_data)(const struct spi_flash_platform *__FAR platform, uint16_t offset, uint8_t *__FAR buffer, uint16_t size);
    ssize_t (*write_page_data)(const struct spi_flash_platform *__FAR platform, uint16_t offset, const uint8_t *__FAR buffer, uint16_t size);
    const void *__FAR (*get_chipinfo)(const struct spi_flash_platform *__FAR platform);
};
/************************************************
  * @struct  spi_flash
  * @brief   用于描述Nandflash包含SPI的接口函数
 ***********************************************/
struct spi_flash {
    void *__FAR private_data;
    const struct spi_flash_operations *__FAR ops;
    const struct spi_flash_platform *__FAR platform;
};

#endif
