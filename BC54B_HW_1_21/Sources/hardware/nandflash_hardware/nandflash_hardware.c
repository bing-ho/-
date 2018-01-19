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
#include "nandflash_hardware.h"
#include "spi_interface.h"
#include "nandflash_interface_platform.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "logger.h"
#include "util.h"

#pragma MESSAGE DISABLE C4001 //Condition always false

#ifndef __func__
    #define __func__ ""
#endif

static const struct nand_oob oob_style_0[] = {
    {0x04, 4},    {0x14, 4},    {0x24, 4},    {0x34, 4},
};
static const struct nand_page_layout page_style_0 = {
    512,
    ARRAY_SIZE(oob_style_0),
    oob_style_0
};

static const struct nand_oob oob_style_1[] = {
    {0x04, 12},    {0x14, 12},    {0x24, 12},    {0x34, 12},
};
static const struct nand_page_layout page_style_1 = {
    512,
    ARRAY_SIZE(oob_style_1),
    oob_style_1
};

static const struct nand_oob oob_style_2[] = {
    {0x04, 4},
};
static const struct nand_page_layout page_style_2 = {
    500,
    ARRAY_SIZE(oob_style_2),
    oob_style_2
};

static const struct spi_nand_info support_nands[] = {
    /*
    uint32_t id;
                uint32_t id_mask
                            uint16_t bytes_per_page;
                                    uint16_t bytes_per_spare;
                                        uint16_t pages_per_block;
                                            uint16_t block_per_chip;
                                                struct nand_page_layout *page_layout;
                                                                const char *part_num;
    */
#ifdef WIN32
    {0xABCD0000, 0xFFFF0000, 2048, 64, 64, 1024, &page_style_0, "SimNandFlash"},
    {0xAAAA0000, 0xFFFF0000, 500, 12, 16, 128, &page_style_2, "SimSDCard"},
#endif
    {0x2C220000, 0xFFFF0000, 2048, 64, 64, 1024, &page_style_0, "MT29F2G01"},
    {0x2C120000, 0xFFFF0000, 2048, 64, 64, 2048, &page_style_0, "MT29F1G01"},
    {0xC8D10000, 0xFFFF0000, 2048, 64, 64, 1024, &page_style_1, "GD5F1GQ4U"},
    {0xC8D20000, 0xFFFF0000, 2048, 64, 64, 2048, &page_style_1, "GD5F2GQ4U"},
    {0xC8C10000, 0xFFFF0000, 2048, 64, 64, 1024, &page_style_1, "GD5F1GQ4R"},
    {0xC8C20000, 0xFFFF0000, 2048, 64, 64, 2048, &page_style_1, "GD5F2GQ4R"},
    {0xC2120000, 0xFFFF0000, 2048, 64, 64, 1024, &page_style_0, "MX35LF1GE4AB"},
    {0xC2220000, 0xFFFF0000, 2048, 64, 64, 2048, &page_style_0, "MX35LF2GE4AB"},
    {0xEFAA2100, 0xFFFFFF00, 2048, 64, 64, 1024, &page_style_0, "W25N01GV"},
};

#ifdef WIN32
    const struct spi_nand_info *const SimNandFlashInfo = &support_nands[0];
    const struct spi_nand_info *const SimSDCardInfo = &support_nands[1];
#endif

/************************************************************************
  * @brief           nandflashģ��оƬ��ѯ
  * @param[in]       id nandflash��id����ͬ��оƬid��ͬ
  * @return          ����ҵ������ظ�оƬinfo������Ҳ�������null
***********************************************************************/
static const struct spi_nand_info *__FAR find_info(uint32_t id) {
    uint8_t i;
    uint32_t tmp;
    for (i = 0; i < ARRAY_SIZE(support_nands); ++i) {
        tmp = id & support_nands[i].id_mask;
        if (tmp == support_nands[i].id) {
            return &support_nands[i];
        }
    }
    return NULL;
}

#define CS_CTRL_ASSERT 0x01
#define CS_CTRL_DEASSERT 0x02

static ssize_t spi_nand_read_page_data(const struct spi_flash_platform *__FAR platform, uint16_t offset, uint8_t *__FAR buffer, uint16_t size);

/************************************************************************
  * @brief           ͨ��spi��nandflashģ�����ݽ��պͷ���
  * @param[in]       platform     ����nandflash���������ԺͲ�������
  * @param[in]       recv_buffer ���ջ�����
  * @param[in]       send_buffer ���ͻ�����
  * @param[in]       len         ���ͱ��ĳ��ȣ��ֽ����
  * @param[in]       cs_ctrl     ������
  * @return          -1��NandFlash_ERROR   0��NandFlash_OK
***********************************************************************/
static int send_and_recv(const struct spi_flash_platform *__FAR platform, uint8_t *__FAR recv_buffer, const uint8_t *__FAR send_buffer, uint16_t len, uint8_t cs_ctrl) {
    int rc;
    struct spi_nand_ctx *__FAR ctx = (struct spi_nand_ctx * __FAR) platform->private_data;
    if (cs_ctrl & CS_CTRL_ASSERT) {
        if (spi_select(platform->bus, ctx->cs_index, 1)) {
            return -1;
        }
    }

    rc = spi_transfer(platform->bus, recv_buffer, send_buffer, len);
    if (rc != len) {
        (void) spi_select(platform->bus, ctx->cs_index, 0);
        return -1;
    }
    if (cs_ctrl & CS_CTRL_DEASSERT) {
        if (spi_select(platform->bus, ctx->cs_index, 0)) {
            return -1;
        }
    }

    return 0;
}
/************************************************************************
  * @brief           ��ȡnandflash��id
  * @param[in]       platform    ����nandflash���������ԺͲ�������
  * @param[in]       id          ���ڷ��ض�����Ч��id
  * @return          -1��NandFlash_ERROR   0��NandFlash_OK
***********************************************************************/
static int nand_read_id(const struct spi_flash_platform *__FAR platform, uint32_t *id) {
    const uint8_t read_id[] = {0x9F, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t r[6];

    if (send_and_recv(platform, r, read_id, 6, CS_CTRL_ASSERT | CS_CTRL_DEASSERT)) {
        return -1;
    }
    *id = ((uint32_t) r[2] << 24) + ((uint32_t) r[3] << 16) + ((uint32_t) r[4] << 8) + r[5];
    return 0;
}
/************************************************************************
  * @brief           ��ȡ״̬�Ĵ���״ֵ̬
  * @param[in]       platform    ����nandflash���������ԺͲ�������
  * @param[in]       addr        ����ȡ״̬�Ĵ�����ַ
  * @param[in]       status      ����״ֵ̬
  * @return          -1��NandFlash_ERROR   0��NandFlash_OK
***********************************************************************/
static inline int nand_read_status(const struct spi_flash_platform *__FAR platform, uint8_t addr, uint8_t *status) {
    uint8_t w[3];
    uint8_t r[3];
    w[0] = 0x0F;
    w[1] = addr;
    w[2] = 0xFF;

    if (send_and_recv(platform, r, w, 3, CS_CTRL_ASSERT | CS_CTRL_DEASSERT)) {
        return -1;
    }

    *status = r[2];
    return 0;
}
/************************************************************************
  * @brief           д״̬�Ĵ��� 
  * @param[in]       platform    ����nandflash���������ԺͲ�������
  * @param[in]       addr        ��д��״̬�Ĵ�����ַ
  * @param[in]       feature     ״̬�Ĵ���д��ֵ
  * @return          -1��NandFlash_ERROR   0��NandFlash_OK
***********************************************************************/
static inline int nand_write_feature(const struct spi_flash_platform *__FAR platform, uint8_t addr, uint8_t feature) {
    uint8_t buffer[3];
    buffer[0] = 0x1F;
    buffer[1] = addr;
    buffer[2] = feature;

    if (send_and_recv(platform, NULL, buffer, sizeof(buffer), CS_CTRL_ASSERT | CS_CTRL_DEASSERT)) {
        return -1;
    }
    return 0;
}
/************************************************************************
  * @brief           дʹ��/��ֹ  id=06H дʹ��    id=04H д��ֹ
  * @param[in]       platform    ����nandflash���������ԺͲ�������
  * @param[in]       is_enable   дʹ��/��ֹ
  * @return          -1��NandFlash_ERROR   0��NandFlash_OK
***********************************************************************/
static inline int nand_write_enable(const struct spi_flash_platform *__FAR platform, uint8_t is_enable) {
    uint8_t buffer[1];
    // write enable
    buffer[0] = is_enable ? 0x06 : 0x04;
    if (send_and_recv(platform, NULL, buffer, 1, CS_CTRL_ASSERT | CS_CTRL_DEASSERT)) {
        return -1;
    }
    return 0;
}
/************************************************************************
  * @brief           nandflash��λ 
  * @param[in]       platform    ����nandflash���������ԺͲ�������
  * @return          -1��NandFlash_ERROR   0��NandFlash_OK
***********************************************************************/
static inline int reset_flash(const struct spi_flash_platform *__FAR platform) {
    uint8_t buffer[1];
    // write enable
    buffer[0] = 0xff;
    if (send_and_recv(platform, NULL, buffer, 1, CS_CTRL_ASSERT | CS_CTRL_DEASSERT)) {
        return -1;
    }
    return 0;
}
/************************************************************************
  * @brief           nandflash�ȴ�ĳ����ִ�н���
  * @param[in]       platform    ����nandflash���������ԺͲ�������
  * @param[in]       retry_cnt   �ظ��ȴ�����
   * @param[in]      pstatus     ����״ֵ̬
  * @return          -1��NandFlash_ERROR   0��NandFlash_OK
***********************************************************************/
static inline int nand_wait_done(const struct spi_flash_platform *__FAR platform, uint16_t  retry_cnt, uint8_t *pstatus) {
#ifndef WIN32
    uint16_t  i = 0;
    uint8_t status = 0;
    for (;;) {
        if (nand_read_status(platform, 0xC0, &status) == 0) {
            if ((status & 0x03)  == 0x00) {  // When finished WEL bit will be cleared, BUSY bit = 0;Even read, no WEL exist
                if (pstatus) {
                    *pstatus = status;
                }
                break;
            }
        }
        LOG(VERBOSE, ("read status[%02X] error, retry #%d still fail", status, i));

        if (++i > retry_cnt) {
            LOG(DEBUG, ("wait for cache read timeout"));
            return -1;
        }
    }
#else
    if (pstatus) {
        return nand_read_status(platform, 0xC0, pstatus);
    }
#endif
    return 0;
}
/************************************************************************
  * @brief           �ú������ڶ�nandflash��ʼ����ָ����г�ʼ��
  * @param[in]       platform    ����nandflash���������ԺͲ�������
  * @return          �޷���ֵ
***********************************************************************/
static void spi_nand_deinit(const struct spi_flash_platform *__FAR platform) {
    struct spi_nand_ctx *__FAR ctx = (struct spi_nand_ctx * __FAR) platform->private_data;
    spi_deinit(platform->bus);
#ifdef WIN32
    free(ctx->buffer);
    ctx->buffer = NULL;
#endif
}
/************************************************************************
  * @brief           �ú������ڶ�nandflash�豸��Ӳ����ʼ��
  * @param[in]       platform    ����nandflash���������ԺͲ�������
  * @return          -1��NandFlash_ERROR   0��NandFlash_OK
***********************************************************************/
static int spi_nand_hardware_init(const struct spi_flash_platform *__FAR platform) {
    uint32_t id = 0;
    uint8_t status = 0;
    struct spi_nand_ctx *__FAR ctx = (struct spi_nand_ctx * __FAR) platform->private_data;
    LOG(VERBOSE, (__func__));
    if (spi_init(platform->bus, ctx->spi_init_flag)) {
        LOG(ERROR, ("spi init error"));
        return -1;
    }
    if (reset_flash(platform)) {
        goto __error;
    }
    if (nand_read_id(platform, &id)) {
        goto __error;
    }

    ctx->chipinfo = find_info(id);
    if (ctx->chipinfo == NULL) {
        LOG(ERROR, ("Not support id: %08X", id));
        goto __error;
    }

#ifdef WIN32
    ctx->buffer = malloc(ctx->chipinfo->bytes_per_page + ctx->chipinfo->bytes_per_spare + 10);
    if (ctx->buffer == NULL) {
        goto __error;
    }
#endif

    LOG(DEBUG, ("ID: %08X", id));
    LOG(DEBUG, ("Name: %s", ctx->chipinfo->part_num));

    if (nand_write_feature(platform, 0xA0, 0x00)) {
        goto __error;
    }

    if (nand_read_status(platform, 0xA0, &status)) {
        goto __error;
    }

    if (status != 0x00) {
        goto __error;
    }

    return 0;

__error:
    LOG(ERROR, ("spi nand init error"));
    spi_nand_deinit(platform);
    return -1;
}
/******************************************************************************************************
  * @brief           ���Ͷ�ҳ����ָ��  id=13H  ��������Ƿ���ָ�������������spi_nand_read_page_data
  * @param[in]       platform    ����nandflash���������ԺͲ�������
  * @param[in]       page_addr   ��ȡҳ��ַ
  * @return          -1��NandFlash_ERROR   0��NandFlash_OK
*******************************************************************************************************/
static int spi_nand_read_page(const struct spi_flash_platform *__FAR platform, uint32_t page_addr) {
    int rc;
    uint8_t i = 0;
    uint8_t buffer[4];
    uint8_t status = 0;

    buffer[0] = 0x13;
    buffer[1] = (page_addr >> 16) & 0xFF;
    buffer[2] = (page_addr >> 8) & 0xFF;
    buffer[3] = page_addr & 0xFF;

    if (send_and_recv(platform, NULL, buffer, 4, CS_CTRL_ASSERT | CS_CTRL_DEASSERT)) {
        return -1;
    }

    rc = nand_wait_done(platform, 100, &status);
    if (rc) {
        return rc;
    }
    if ((status & 0x20) == 0x20) {
        LOG(WARN, ("read page %d ecc error", page_addr));
        return -3;
    }
    return 0;
}
/******************************************************************************************************
  * @brief           ����дҳ����ָ��  id=10H  ��������Ƿ���ָ�����д������spi_nand_write_page_data
  * @param[in]       platform    ����nandflash���������ԺͲ�������
  * @param[in]       page_addr   д����ҳ��ַ
  * @return          -1��NandFlash_ERROR   0��NandFlash_OK
*******************************************************************************************************/
static int spi_nand_write_page(const struct spi_flash_platform *__FAR platform, uint32_t page_addr) {
    uint16_t i = 0;
    uint8_t buffer[4];
    if (nand_write_feature(platform, 0xA0, 0x00)) {
        LOG(INFO, ("disable protect error"));
        return -1;
    }

    buffer[0] = 0x10;
    buffer[1] = (page_addr >> 16) & 0xFF;
    buffer[2] = (page_addr >> 8) & 0xFF;
    buffer[3] = (page_addr) & 0xFF;
    if (send_and_recv(platform, NULL, buffer, 4, CS_CTRL_ASSERT | CS_CTRL_DEASSERT)) {
        return -1;
    }
    // write disable
    if (nand_write_enable(platform, 0)) {
        LOG(INFO, ("write disable error"));
        return -1;
    }
    return nand_wait_done(platform, 1000, NULL);
}
/******************************************************************************************************
  * @brief           ��д1block����
  * @param[in]       platform    ����nandflash���������ԺͲ�������
  * @param[in]       page_addr   һ��block����ʼҳ��ַ
  * @return          -1��NandFlash_ERROR   0��NandFlash_OK
*******************************************************************************************************/
static int spi_nand_erase_block(const struct spi_flash_platform *__FAR platform, uint32_t page_addr) {
    uint16_t i = 0;
    uint8_t buffer[4];
    uint16_t bad_block_flag = 0;
    int rc;
    struct spi_nand_ctx *__FAR ctx = (struct spi_nand_ctx * __FAR) platform->private_data;
    if (nand_write_feature(platform, 0xA0, 0x00)) {
        LOG(INFO, ("disable protect error"));
        return -1;
    }
    if (nand_write_enable(platform, 1)) {
        LOG(INFO, ("write enable error"));
        return -1;
    }
    buffer[0] = 0xD8;
    buffer[1] = (page_addr >> 16) & 0xFF;
    buffer[2] = (page_addr >> 8) & 0xFF;
    buffer[3] = (page_addr) & 0xFF;

    if (send_and_recv(platform, NULL, buffer, sizeof(buffer), CS_CTRL_ASSERT | CS_CTRL_DEASSERT)) {
        return -1;
    }

    if(nand_write_enable(platform, 0)) {
        return -1;
    }

    rc = nand_wait_done(platform, 1000, NULL);
    if (rc) {
        return rc;
    }
    if (spi_nand_read_page(platform, page_addr)) {
        return -1;
    }
    if (spi_nand_read_page_data(platform, ctx->chipinfo->bytes_per_page, (uint8_t *__FAR)&bad_block_flag, sizeof(bad_block_flag)) != sizeof(bad_block_flag)) {
        return -1;
    }
    if (bad_block_flag == 0) {
        return -128;
    }
    return 0;
}
/******************************************************************************************************
  * @brief           ��ȡĳһҳ��ָ���������� id=03H ǰ�����Ѿ�ִ���� ��ҳ��������id=13H,����ָ����ҳ�ĵ�ַ
  * @param[in]       platform    ����nandflash���������ԺͲ�������
  * @param[in]       offset      ��ȡ�������ڵ���ҳ��ַ��ƫ����
  * @param[in]       buffer      ��ȡ������
  * @param[in]       size        ��ȡ����
  * @return          -1��NandFlash_ERROR   0��NandFlash_OK
*******************************************************************************************************/
static ssize_t spi_nand_read_page_data(const struct spi_flash_platform *__FAR platform, uint16_t offset, uint8_t *__FAR buffer, uint16_t size) {
    uint16_t i = 0;
    uint8_t  status2;
#ifdef WIN32
    struct spi_nand_ctx *__FAR ctx = (struct spi_nand_ctx * __FAR) platform->private_data;
    uint8_t *buf = ctx->buffer;
#else
    uint8_t buf[4];
#endif
    buf[0] = 0x03;
    buf[1] = (offset >> 8) & 0xFF;
    buf[2] = offset & 0xFF;
    buf[3] = 0xFF;

    if(nand_read_status(platform, 0xb0, &status2)==0) {
    }

    if(nand_write_feature(platform,0xb0,status2|0x08)==0){
    } 

#ifdef WIN32
    if(send_and_recv(platform, buf, buf, size + 4, CS_CTRL_ASSERT | CS_CTRL_DEASSERT)) {
        return -1;
    }
    memcpy(buffer, (uint8_t *)&buf[4], size);
#else
    if (send_and_recv(platform, NULL, buf, sizeof(buf), CS_CTRL_ASSERT)) {
        LOG(DEBUG, ("read page data error"));
        return -1;
    }
    if (send_and_recv(platform, buffer, NULL, size, CS_CTRL_DEASSERT)) {
        LOG(DEBUG, ("read page data error"));
        return -1;
    }
#endif    
    return size;
}
/******************************************************************************************************
  * @brief           д ĳһҳ��ָ���������� id=84H ǰ�����Ѿ�ִ���˶�ҳ�������� id=10H,����ָ����ҳ�ĵ�ַ
  * @param[in]       platform    ����nandflash���������ԺͲ�������
  * @param[in]       offset      д�������ڵ���ҳ��ַ��ƫ����
  * @param[in]       buffer      д������
  * @param[in]       size        д����
  * @return          -1��NandFlash_ERROR   0��NandFlash_OK
*******************************************************************************************************/
static ssize_t spi_nand_write_page_data(const struct spi_flash_platform *__FAR platform, uint16_t offset, const uint8_t *__FAR buffer, uint16_t size) {
    uint16_t i = 0;
    struct spi_nand_ctx *__FAR ctx = (struct spi_nand_ctx * __FAR) platform->private_data;
#ifdef WIN32
    uint8_t *buf = ctx->buffer;
#else
    uint8_t buf[3];
#endif
    if (nand_write_enable(platform, 1)) {
        return -1;
    }

    buf[0] = 0x84;
    buf[1] = (offset >> 8) & 0xFF;
    buf[2] = offset & 0xFF;
#ifdef WIN32
    memcpy(&buf[3], buffer, size);

    if (send_and_recv(platform, NULL, buf, size + 3, CS_CTRL_ASSERT | CS_CTRL_DEASSERT)) {
        return 0;
    }

#else

    if (send_and_recv(platform, NULL, buf, sizeof(buf), CS_CTRL_ASSERT)) {
        LOG(DEBUG, ("write page data error"));
        return -1;
    }
    if (send_and_recv(platform, NULL, buffer, size, CS_CTRL_DEASSERT)) {
        LOG(DEBUG, ("write page data error"));
        return -1;
    }
#endif
    return size;
}
/******************************************************************************************************
  * @brief           ��ȡnandflashоƬ��Ϣ
  * @param[in]       platform    ����nandflash���������ԺͲ�������
  * @return          оƬ��Ϣ
*******************************************************************************************************/
static const void *__FAR spi_nand_get_chipinfo(const struct spi_flash_platform *__FAR platform) {
    struct spi_nand_ctx *__FAR ctx = (struct spi_nand_ctx * __FAR) platform->private_data;
    return ctx->chipinfo;
}
/******************************************************************************************************
  * @brief           ����nandflash�ײ������ṹ���װ�����ӿں�������
*******************************************************************************************************/
const struct spi_flash_operations spi_nand_ops = {
    (int (*)(const struct spi_flash_platform * __FAR)) spi_nand_hardware_init,
    (void (*)(const struct spi_flash_platform * __FAR)) spi_nand_deinit,
    (int (*)(const struct spi_flash_platform * __FAR , uint32_t)) spi_nand_read_page,
    (int (*)(const struct spi_flash_platform * __FAR , uint32_t)) spi_nand_write_page,
    (int (*)(const struct spi_flash_platform * __FAR , uint32_t)) spi_nand_erase_block,
    (ssize_t (*)(const struct spi_flash_platform * __FAR , uint16_t, uint8_t *__FAR, uint16_t)) spi_nand_read_page_data,
    (ssize_t (*)(const struct spi_flash_platform * __FAR , uint16_t, const uint8_t *__FAR, uint16_t)) spi_nand_write_page_data,
    (const void *__FAR(*)(const struct spi_flash_platform * __FAR)) spi_nand_get_chipinfo,
};
