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
#ifndef _nandflash_hardware_H__
#define _nandflash_hardware_H__


#include "stdint.h"
/************************************************
  * @struct  nand_oob
  * @brief   ������ÿ��sector��������Ϣ
 ***********************************************/
struct nand_oob {
    uint8_t offset;
    uint8_t length;
};
/************************************************
  * @struct  nand_page_layout
  * @brief   ��������Nandflash��ҳ��Ϣ
 ***********************************************/
struct nand_page_layout {
    uint16_t sector_size;
    uint16_t sector_count;
    const struct nand_oob *__FAR oobfree;
};
/************************************************
  * @struct  spi_nand_info
  * @brief   ��������NandflashоƬ����Ϣ
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
  * @brief   ��������Nandflash���ܵĽṹ��
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
