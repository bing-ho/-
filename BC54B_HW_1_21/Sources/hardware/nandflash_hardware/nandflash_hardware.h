/*******************************************************************************
**                       °²»ÕÁ¦¸ßĞÂÄÜÔ´ÓĞÏŞ¹«Ë¾ Copyright (c)
**                            http://www.ligoo.cn/ 
** @file	 £b   nandflash_hardware.c                                       

** @brief       1.Íê³Énandflash¶Á¡¢Ğ´ºÍ²Á³ı¹¦ÄÜº¯Êı 
                2.Íê³ÉnandflashÄ£¿éµÄ³õÊ¼»¯
                3.Íê³ÉnandflashÄ£¿éµÄ¸÷¹¦ÄÜº¯Êı½á¹¹ÌåµÄ¶¨Òå
** @copyright		Ligoo Inc.         
** @date		    2017-04-12.
** @author      À´Ïè     
*******************************************************************************/ 
#ifndef _nandflash_hardware_H__
#define _nandflash_hardware_H__


#include "stdint.h"
/************************************************
  * @struct  nand_oob
  * @brief   ÓÃÀ´ÃèÃ¿¸ösectorÈßÓàÇøĞÅÏ¢
 ***********************************************/
struct nand_oob {
    uint8_t offset;
    uint8_t length;
};
/************************************************
  * @struct  nand_page_layout
  * @brief   ÓÃÀ´ÃèÊöNandflashµÄÒ³ĞÅÏ¢
 ***********************************************/
struct nand_page_layout {
    uint16_t sector_size;
    uint16_t sector_count;
    const struct nand_oob *__FAR oobfree;
};
/************************************************
  * @struct  spi_nand_info
  * @brief   ÓÃÀ´ÃèÊöNandflashĞ¾Æ¬µÄĞÅÏ¢
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
  * @brief   ÓÃÀ´ÃèÊöNandflash¹¦ÄÜµÄ½á¹¹Ìå¢
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
