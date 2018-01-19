
#ifndef __nandflash_intermediate_H__
#define __nandflash_intermediate_H__

#include "stdint.h"
/************************************************
  * @struct  StorageContext
  * @brief   ��������sector����Ϣ
 ***********************************************/
struct StorageContext {
    uint8_t *__FAR cache;
    uint8_t *__FAR spare;
    uint32_t last_sector_id;
    uint32_t last_sector_addr;
    uint16_t current_index;
};
/************************************************
  * @struct  StorageObject
  * @brief   ��������nandflash�Ŀ��������Ϣ
 ***********************************************/
struct StorageObject {
    const struct spi_flash *__FAR flash;
    uint16_t first_block;   /** �Կ��ַΪ��λ���Ա��������� */
    uint16_t last_block;
    struct StorageContext *__FAR ctx;
};

/********************************************************************************************************************************
  * @brief           ��ʼ���洢�豸
  * @param[in]       obj   �洢����
  * @return          0�������ô洢����Ϊȫ��,ֱ��ʹ�ò��ҵ�������   -1��������Ϊ���д������,��Ҫ׼����һ������,��ָ����һ������
*********************************************************************************************************************************/
int storage_init(const struct StorageObject *__FAR obj);

/************************************************************************
  * @brief          ֹͣ�洢�豸
  * @param[in]       obj   �洢����
  * @return          0��������� ����:ʧ��
***********************************************************************/
int storage_deinit(const struct StorageObject *__FAR obj);

/********************************************************************************************************************************
  * @brief           �洢��¼
  * @param[in]       obj   �洢����
  * @param[in]       type  ��������
  * @param[in]       pdata ��������
  * @param[in]       size  ���ݳ���
  * @return          >=0���������  <0����ʾʧ��
*********************************************************************************************************************************/
int storage_save(const struct StorageObject *__FAR obj, uint16_t type, const uint8_t *__FAR pdata, uint16_t size);

/********************************************************************************************************************************
  * @brief           �洢��¼,���ڻ���Ƚ�С�ĳ���,ͨ���ص��ķ�ʽ�ִδ洢��¼
  * @param[in]       obj   �洢����
  * @param[in]       type  ��������
  * @param[in]       size  ���ݳ���
  * @param[in]       cb    �ִδ洢�Ļص�����,���ػ����ָ���Լ�����ĳ���
  * @return          0���������
*********************************************************************************************************************************/
int storage_save_ex(const struct StorageObject *__FAR obj, uint16_t type, uint16_t size, uint8_t *__FAR(*cb)(uint16_t *__FAR, void *__FAR), void *__FAR attach);

/********************************************************************************************************************************
  * @brief           ˢ�»���,�������е�����ͬ�����洢�豸��
  * @param[in]       obj   �洢����
  * @return          0��������� ����:ʧ��
*********************************************************************************************************************************/
int storage_flush(const struct StorageObject *__FAR obj);
/************************************************
  * @struct  StorageContext
  * @brief   ���ڱȽϵĻص�����
 ***********************************************/
struct BSearch {
    int (*cmp)(const struct BSearch *__FAR thiz, uint32_t left, uint32_t right, int *shift);
    const void *private_data;
};
/************************************************************************
  * @brief           ���ַ��������ֵ
  * @param[in]       thiz       ���Ҷ���
  * @param[in]       start      ��ʼ��ַ
  * @param[in]       end        ������ַ
  * @return          ���ֵ
***********************************************************************/
uint32_t binary_search_max(const struct BSearch *__FAR thiz, uint32_t start, uint32_t end);
/************************************************************************
  * @brief           ���ַ�������Сֵ
  * @param[in]       thiz       ���Ҷ���
  * @param[in]       start      ��ʼ��ַ
  * @param[in]       end        ������ַ
  * @return          ��Сֵ
***********************************************************************/
uint32_t binary_search_min(const struct BSearch *__FAR thiz, uint32_t start, uint32_t end);
/************************************************
  * @struct  StorageContext
  * @brief   �ڴ���Һ���
 ***********************************************/
ssize_t memfind(const uint8_t *__FAR dest, ssize_t dest_size, const uint8_t *__FAR src, ssize_t src_size);
extern const struct StorageObject record_storage_obj;

#endif
