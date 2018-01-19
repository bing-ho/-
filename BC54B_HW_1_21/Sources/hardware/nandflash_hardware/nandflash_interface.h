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
#ifndef __NANDFLASH_INTERFACE_H__
#define __NANDFLASH_INTERFACE_H__

#include <stdio.h>
#include "stdint.h"
#include "nandflash_interface_platform.h"

/************************************************************************
  * @brief           �ú������ڶ�nandflash�豸��Ӳ����ʼ��
  * @param[in]       platform    ����nandflash���������ԺͲ�������
  * @return          -1��NandFlash_ERROR   0��NandFlash_OK
***********************************************************************/
int spi_flash_init(const struct spi_flash *__FAR flash);


/************************************************************************
  * @brief           �ú������ڶ�nandflash��ʼ����ָ����г�ʼ��
  * @param[in]       platform    ����nandflash���������ԺͲ�������
  * @return          �޷���ֵ
***********************************************************************/
void spi_flash_deinit(const struct spi_flash *__FAR flash);


/******************************************************************************************************
  * @brief           ���Ͷ�ҳ����ָ��  id=13H  ��������Ƿ���ָ�������������spi_nand_read_page_data
  * @param[in]       platform    ����nandflash���������ԺͲ�������
  * @param[in]       page_addr   ��ȡҳ��ַ
  * @return          -1��NandFlash_ERROR   0��NandFlash_OK
*******************************************************************************************************/
int spi_flash_read_page(const struct spi_flash *__FAR flash, uint32_t page_addr);


/******************************************************************************************************
  * @brief           ����дҳ����ָ��  id=10H  ��������Ƿ���ָ�����д������spi_nand_write_page_data
  * @param[in]       platform    ����nandflash���������ԺͲ�������
  * @param[in]       page_addr   д����ҳ��ַ
  * @return          -1��NandFlash_ERROR   0��NandFlash_OK
*******************************************************************************************************/
int spi_flash_write_page(const struct spi_flash *__FAR flash, uint32_t page_addr);



/******************************************************************************************************
  * @brief           ��д1block����
  * @param[in]       platform    ����nandflash���������ԺͲ�������
  * @param[in]       page_addr   һ��block����ʼҳ��ַ
  * @return          -1��NandFlash_ERROR   0��NandFlash_OK
*******************************************************************************************************/
int spi_flash_erase_block(const struct spi_flash *__FAR flash, uint32_t page_addr);



/******************************************************************************************************
  * @brief           ��ȡĳһҳ��ָ���������� id=03H ǰ�����Ѿ�ִ���� ��ҳ��������id=13H,����ָ����ҳ�ĵ�ַ
  * @param[in]       platform    ����nandflash���������ԺͲ�������
  * @param[in]       offset      ��ȡ�������ڵ���ҳ��ַ��ƫ����
  * @param[in]       buffer      ��ȡ������
  * @param[in]       size        ��ȡ����
  * @return          -1��NandFlash_ERROR   0��NandFlash_OK
*******************************************************************************************************/
ssize_t spi_flash_read_page_data(const struct spi_flash *__FAR flash, uint16_t offset, uint8_t *__FAR buffer, uint16_t size);




/******************************************************************************************************
  * @brief           д ĳһҳ��ָ���������� id=84H ǰ�����Ѿ�ִ���˶�ҳ�������� id=10H,����ָ����ҳ�ĵ�ַ
  * @param[in]       platform    ����nandflash���������ԺͲ�������
  * @param[in]       offset      д�������ڵ���ҳ��ַ��ƫ����
  * @param[in]       buffer      д������
  * @param[in]       size        д����
  * @return          -1��NandFlash_ERROR   0��NandFlash_OK
*******************************************************************************************************/
ssize_t spi_flash_write_page_data(const struct spi_flash *__FAR flash, uint16_t offset, const uint8_t *__FAR buffer, uint16_t size);





/******************************************************************************************************
  * @brief           ��ȡnandflashоƬ��Ϣ
  * @param[in]       platform    ����nandflash���������ԺͲ�������
  * @return          оƬ��Ϣ
*******************************************************************************************************/
const void *__FAR spi_flash_get_chipinfo(const struct spi_flash *__FAR flash);

#endif
