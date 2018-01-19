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
#ifndef __NANDFLASH_INTERFACE_H__
#define __NANDFLASH_INTERFACE_H__

#include <stdio.h>
#include "stdint.h"
#include "nandflash_interface_platform.h"

/************************************************************************
  * @brief           该函数用于对nandflash设备的硬件初始化
  * @param[in]       platform    包含nandflash的所有属性和操作方法
  * @return          -1：NandFlash_ERROR   0：NandFlash_OK
***********************************************************************/
int spi_flash_init(const struct spi_flash *__FAR flash);


/************************************************************************
  * @brief           该函数用于对nandflash初始化的指令进行初始化
  * @param[in]       platform    包含nandflash的所有属性和操作方法
  * @return          无返回值
***********************************************************************/
void spi_flash_deinit(const struct spi_flash *__FAR flash);


/******************************************************************************************************
  * @brief           发送读页操作指令  id=13H  这个仅仅是发送指令，真正读内容是spi_nand_read_page_data
  * @param[in]       platform    包含nandflash的所有属性和操作方法
  * @param[in]       page_addr   读取页地址
  * @return          -1：NandFlash_ERROR   0：NandFlash_OK
*******************************************************************************************************/
int spi_flash_read_page(const struct spi_flash *__FAR flash, uint32_t page_addr);


/******************************************************************************************************
  * @brief           发送写页操作指令  id=10H  这个仅仅是发送指令，真正写内容是spi_nand_write_page_data
  * @param[in]       platform    包含nandflash的所有属性和操作方法
  * @param[in]       page_addr   写操作页地址
  * @return          -1：NandFlash_ERROR   0：NandFlash_OK
*******************************************************************************************************/
int spi_flash_write_page(const struct spi_flash *__FAR flash, uint32_t page_addr);



/******************************************************************************************************
  * @brief           擦写1block数据
  * @param[in]       platform    包含nandflash的所有属性和操作方法
  * @param[in]       page_addr   一个block的起始页地址
  * @return          -1：NandFlash_ERROR   0：NandFlash_OK
*******************************************************************************************************/
int spi_flash_erase_block(const struct spi_flash *__FAR flash, uint32_t page_addr);



/******************************************************************************************************
  * @brief           读取某一页中指定区域内容 id=03H 前提是已经执行了 读页操作命令id=13H,其中指定了页的地址
  * @param[in]       platform    包含nandflash的所有属性和操作方法
  * @param[in]       offset      读取区域所在的首页地址的偏移量
  * @param[in]       buffer      读取缓存区
  * @param[in]       size        读取长度
  * @return          -1：NandFlash_ERROR   0：NandFlash_OK
*******************************************************************************************************/
ssize_t spi_flash_read_page_data(const struct spi_flash *__FAR flash, uint16_t offset, uint8_t *__FAR buffer, uint16_t size);




/******************************************************************************************************
  * @brief           写 某一页中指定区域内容 id=84H 前提是已经执行了读页操作命令 id=10H,其中指定了页的地址
  * @param[in]       platform    包含nandflash的所有属性和操作方法
  * @param[in]       offset      写区域所在的首页地址的偏移量
  * @param[in]       buffer      写缓存区
  * @param[in]       size        写长度
  * @return          -1：NandFlash_ERROR   0：NandFlash_OK
*******************************************************************************************************/
ssize_t spi_flash_write_page_data(const struct spi_flash *__FAR flash, uint16_t offset, const uint8_t *__FAR buffer, uint16_t size);





/******************************************************************************************************
  * @brief           获取nandflash芯片信息
  * @param[in]       platform    包含nandflash的所有属性和操作方法
  * @return          芯片信息
*******************************************************************************************************/
const void *__FAR spi_flash_get_chipinfo(const struct spi_flash *__FAR flash);

#endif
