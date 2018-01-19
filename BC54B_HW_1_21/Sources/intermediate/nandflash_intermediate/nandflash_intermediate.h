
#ifndef __nandflash_intermediate_H__
#define __nandflash_intermediate_H__

#include "stdint.h"
/************************************************
  * @struct  StorageContext
  * @brief   用来描述sector的信息
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
  * @brief   用来描述nandflash的块和扇区信息
 ***********************************************/
struct StorageObject {
    const struct spi_flash *__FAR flash;
    uint16_t first_block;   /** 以块地址为单位可以避免跨块问题 */
    uint16_t last_block;
    struct StorageContext *__FAR ctx;
};

/********************************************************************************************************************************
  * @brief           初始化存储设备
  * @param[in]       obj   存储对象
  * @return          0：表明该存储区域为全新,直接使用查找到的扇区   -1：该扇区为最后写入扇区,需要准备下一个扇区,并指向下一个扇区
*********************************************************************************************************************************/
int storage_init(const struct StorageObject *__FAR obj);

/************************************************************************
  * @brief          停止存储设备
  * @param[in]       obj   存储对象
  * @return          0：操作完成 其他:失败
***********************************************************************/
int storage_deinit(const struct StorageObject *__FAR obj);

/********************************************************************************************************************************
  * @brief           存储记录
  * @param[in]       obj   存储对象
  * @param[in]       type  数据类型
  * @param[in]       pdata 数据内容
  * @param[in]       size  数据长度
  * @return          >=0：操作完成  <0：表示失败
*********************************************************************************************************************************/
int storage_save(const struct StorageObject *__FAR obj, uint16_t type, const uint8_t *__FAR pdata, uint16_t size);

/********************************************************************************************************************************
  * @brief           存储记录,用于缓存比较小的场合,通过回调的方式分次存储记录
  * @param[in]       obj   存储对象
  * @param[in]       type  数据类型
  * @param[in]       size  数据长度
  * @param[in]       cb    分次存储的回调函数,返回缓存的指针以及缓存的长度
  * @return          0：操作完成
*********************************************************************************************************************************/
int storage_save_ex(const struct StorageObject *__FAR obj, uint16_t type, uint16_t size, uint8_t *__FAR(*cb)(uint16_t *__FAR, void *__FAR), void *__FAR attach);

/********************************************************************************************************************************
  * @brief           刷新缓存,将缓存中的数据同步到存储设备上
  * @param[in]       obj   存储对象
  * @return          0：操作完成 其他:失败
*********************************************************************************************************************************/
int storage_flush(const struct StorageObject *__FAR obj);
/************************************************
  * @struct  StorageContext
  * @brief   用于比较的回调函数
 ***********************************************/
struct BSearch {
    int (*cmp)(const struct BSearch *__FAR thiz, uint32_t left, uint32_t right, int *shift);
    const void *private_data;
};
/************************************************************************
  * @brief           二分法查找最大值
  * @param[in]       thiz       查找对象
  * @param[in]       start      起始地址
  * @param[in]       end        结束地址
  * @return          最大值
***********************************************************************/
uint32_t binary_search_max(const struct BSearch *__FAR thiz, uint32_t start, uint32_t end);
/************************************************************************
  * @brief           二分法查找最小值
  * @param[in]       thiz       查找对象
  * @param[in]       start      起始地址
  * @param[in]       end        结束地址
  * @return          最小值
***********************************************************************/
uint32_t binary_search_min(const struct BSearch *__FAR thiz, uint32_t start, uint32_t end);
/************************************************
  * @struct  StorageContext
  * @brief   内存查找函数
 ***********************************************/
ssize_t memfind(const uint8_t *__FAR dest, ssize_t dest_size, const uint8_t *__FAR src, ssize_t src_size);
extern const struct StorageObject record_storage_obj;

#endif
