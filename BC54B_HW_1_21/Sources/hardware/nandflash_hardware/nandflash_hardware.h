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
#ifndef _nandflash_hardware_H__
#define _nandflash_hardware_H__


#include "stdint.h"
/************************************************
  * @struct  nand_oob
  * @brief   用来描每个sector冗余区信息
 ***********************************************/
struct nand_oob {
    uint8_t offset;
    uint8_t length;
};
/************************************************
  * @struct  nand_page_layout
  * @brief   用来描述Nandflash的页信息
 ***********************************************/
struct nand_page_layout {
    uint16_t sector_size;
    uint16_t sector_count;
    const struct nand_oob *__FAR oobfree;
};
/************************************************
  * @struct  spi_nand_info
  * @brief   用来描述Nandflash芯片的信息
 ***********************************************/
struct spi_nand_info {
    uint32_t id;
    uint32_t id_mask;
    uint16_t bytes_per_page;
    uint16_t bytes_per_spare;
    uint16_t pages_per_block;
    uint16_t blocks_per_chip;
    const struct nand_page_layout *__FAR page_layout;
    const char *part_num;
};
/************************************************
  * @struct  spi_nand_ctx
  * @brief   用来描述Nandflash功能的结构体�
 ***********************************************/
struct spi_nand_ctx {
    uint8_t cs_index;
    uint8_t spi_init_flag;
    const struct spi_nand_info *__FAR chipinfo;    
#ifdef WIN32
    uint8_t *buffer;
#endif    
};

extern const struct spi_flash_operations spi_nand_ops;

#endif
