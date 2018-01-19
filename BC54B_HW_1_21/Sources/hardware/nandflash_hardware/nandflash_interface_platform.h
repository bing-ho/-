/*******************************************************************************
**                       ������������Դ���޹�˾ Copyright (c)
**                            http://www.ligoo.cn/ 
** @file	 �b   nandflash_hardware.c                                       

** @brief       1.���nandflash����д�Ͳ������ܺ��� 
                2.���nandflashģ��ĳ�ʼ��
                3.���nandflashģ��ĸ����ܺ����ṹ��Ķ���
** @copyright		Ligoo Inc.         
** @date		    2017-04-12.
** @author      ����     
*******************************************************************************/ 
#ifndef __SPI_FLASH_PLATFORM_H__
#define __SPI_FLASH_PLATFORM_H__
                                                     
#include "stdint.h"
#include <stdio.h>
/************************************************
  * @struct  spi_flash_platform
  * @brief   ��������SpiFlash ͨ�����Խṹ��
 ***********************************************/
struct spi_flash_platform {
    void *__FAR private_data;
    const struct spi_bus *__FAR bus;
};
/************************************************
  * @struct  spi_flash_operations
  * @brief   ��������Nandflash ������ṹ�壬�ӿں���
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
  * @brief   ��������Nandflash����SPI�Ľӿں���
 ***********************************************/
struct spi_flash {
    void *__FAR private_data;
    const struct spi_flash_operations *__FAR ops;
    const struct spi_flash_platform *__FAR platform;
};

#endif
