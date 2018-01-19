//#if 0
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
#include "nandflash_intermediate.h"
#include "spi_interface.h"
#include "nandflash_interface.h"
#include "nandflash_hardware.h"
#include "nandflash_interface_platform.h "
#include "spi_flash_impl.h"
#include "logger.h"
#include <string.h>
#include "stdint.h"
#include <stdio.h>
#include <stdlib.h>

#pragma MESSAGE DISABLE C3604 //static 'xxxx' was not referenced
#pragma MESSAGE DISABLE C4001 //Condition always false

#ifdef WIN32
    #include <time.h>
    #include <Winsock2.h>
    #define PACKED __attribute__((packed))
#else
    #include "bms_bcu.h"
    #define PACKED
    #define ntohl(a)    a
    #define ntohs(a)    a
    #define htons(a)    a
    #define htonl(a)    a
#endif

#define RECORD_HEAD     0xFEFE
#define INVALID_ID      0x00000000UL
#define IDLE_ID         0xFFFFFFFFUL
#define ECC_SKIP        1

static uint32_t get_system_time(void) {
#ifdef WIN32
    return time(NULL);
#else
    uint32_t t;
    struct datetime time; 
    bcu_get_system_time(&time);
    datetime_to_second(&t, &time);
    /* datetime_to_second(&t, &(bcu_get_bms_info()->system_time)); */
    return t;
#endif
}
/*********************************************************************************************/
#pragma push
#pragma DATA_SEG __RPAGE_SEG PAGED_RAM_SAVE

static uint8_t cache[512];
static uint8_t spare[4];
/************************************************
  * @struct  RecordInfo
  * @brief   用来描述sector的信息
 ***********************************************/
static struct StorageContext storage_ctx = {
	cache,
	spare,
};

#pragma pop
/************************************************
  * @struct  RecordInfo
  * @brief   用来描述nandflash组成的信息
 ***********************************************/
const struct StorageObject record_storage_obj = {
    &nand,
    64,
    1023,
    &storage_ctx
};
/************************************************
  * @struct  RecordInfo
  * @brief   用来描述记录的信息
 ***********************************************/
struct RecordInfo {
    uint16_t head;
    uint16_t len;
    uint32_t timestamp;
    uint16_t type;
} PACKED;

struct StorageSearchItem {
    uint32_t index;
    uint32_t id;
};

struct StorageSearchCtx {
    const struct StorageObject *__FAR obj;
    struct StorageSearchItem left;
    struct StorageSearchItem right;
};

/************************************************************************
  * @brief           获取存储的块的ID
  * @param[in]       flash       包含nandflash的所有属性和操作方法
  * @param[in]       index       块的地址
  * @return          0：无效的ID   id：块的id值
***********************************************************************/
static uint32_t storage_get_id_with_block(const struct spi_flash *__FAR flash, uint32_t index) {
    int ret = 0;
    uint16_t i;
    uint32_t id = INVALID_ID;
    struct spi_nand_info *__FAR info = (struct spi_nand_info * __FAR) spi_flash_get_chipinfo(flash);
    uint32_t page_addr = index * info->pages_per_block;
    uint16_t offset = info->bytes_per_page + info->page_layout->oobfree[0].offset;
    for (i = 0; i < info->pages_per_block; i++) {
        ret = spi_flash_read_page(flash, page_addr + i);
        if (ret == 0) {
            break;
        }
    }
    if (i < info->pages_per_block) {
        spi_flash_read_page_data(flash, offset, (uint8_t *__FAR)&id, sizeof(id));
        return ntohl(id);
    }
    return INVALID_ID;
}

/************************************************************************
  * @brief           获取存储的扇区的ID
  * @param[in]       flash       包含nandflash的所有属性和操作方法
  * @param[in]       index       扇区的地址
  * @return          0：无效的ID   id：扇区的ID
***********************************************************************/
static uint32_t storage_get_id_with_sector(const struct spi_flash *__FAR flash, uint32_t index) {
    uint32_t id = INVALID_ID;
    struct spi_nand_info *__FAR info = (struct spi_nand_info * __FAR) spi_flash_get_chipinfo(flash);
    uint32_t page_addr = index / info->page_layout->sector_count;
    uint16_t oob_index = (uint16_t)(index % info->page_layout->sector_count);
    uint16_t offset = info->bytes_per_page + info->page_layout->oobfree[oob_index].offset;
    int ret = spi_flash_read_page(flash, page_addr);

    if (ret == 0) {
        spi_flash_read_page_data(flash, offset, (uint8_t *__FAR) &id, sizeof(id));
        return ntohl(id);
    }
    return INVALID_ID;
}
/************************************************************************
  * @brief           更新输入对象的ID
  * @param[in]       thiz       用于比较的回调函数
  * @param[in]       left       左值索引号
  * @param[in]       right      右值索引号
  * @param[in]       (const struct spi_flash *__FAR , uint32_t)       输入对象的ID值
  * @return          无返回
***********************************************************************/
inline static void storage_update_id(const struct BSearch *__FAR thiz, uint32_t left, uint32_t right, uint32_t (*get_id)(const struct spi_flash *__FAR , uint32_t)) {
    struct StorageSearchCtx *__FAR ctx = (struct StorageSearchCtx * __FAR) thiz->private_data;
    struct StorageSearchItem tmp = ctx->left;
    if (left != ctx->left.index) {
        if (left != ctx->right.index) {
            ctx->left.index = left;
            ctx->left.id = get_id(ctx->obj->flash, left);
        } else {
            ctx->left = ctx->right;
        }
    }
    if (right != ctx->right.index) {
        if (right != tmp.index) {
            ctx->right.index = right;
            ctx->right.id = get_id(ctx->obj->flash, right);
        } else {
            ctx->right = tmp;
        }
    }
}

/************************************************************************
  * @brief           对获取的ID的有效性进行检测
  * @param[in]       ctx         包含ID的所有信息
  * @param[in]       shift       用于返回索引号微调步长
  * @return          0：无效的ID   1：溢出 -1：ID有效
***********************************************************************/
inline static int storage_cmp(const struct StorageSearchCtx *__FAR ctx, int *shift) {
    if (ctx->left.id == INVALID_ID) {
        *shift = 1;
        return 0;
    }
    if (ctx->right.id == INVALID_ID) {
        *shift = -1;
        return 0;
    }
    *shift = 0;
    if (ctx->right.id == IDLE_ID) { return 1; }
    /**
     * 在16位的平台上会产生溢出
     * return (int)(ctx->left.id - ctx->right.id);
     */
    return ctx->left.id > ctx->right.id ? 1 : -1;
}
/************************************************************************
  * @brief           更新输入块对象的ID,并进行有效性检测
  * @param[in]       thiz       用于比较的回调函数
  * @param[in]       left       左值索引号
  * @param[in]       right      右值索引号
  * @param[in]       shift       用于返回索引号微调步长
  * @return          0：无效的ID   1：溢出 -1：ID有效
***********************************************************************/
static int storage_cmp_with_block(const struct BSearch *__FAR thiz, uint32_t left, uint32_t right, int *shift) {
    storage_update_id(thiz, left, right, storage_get_id_with_block);
    return storage_cmp(thiz->private_data, shift);
}
/************************************************************************
  * @brief           更新输入扇区对象的ID,并进行有效性检测
  * @param[in]       thiz       用于比较的回调函数
  * @param[in]       left       左值索引号
  * @param[in]       right      右值索引号
  * @param[in]       shift      用于返回索引号微调步长
  * @return          0：无效的ID   1：溢出 -1：ID有效
***********************************************************************/
static int storage_cmp_with_sector(const struct BSearch *__FAR thiz, uint32_t left, uint32_t right, int *shift) {
    storage_update_id(thiz, left, right, storage_get_id_with_sector);
    return storage_cmp(thiz->private_data, shift);
}
/************************************************************************
  * @brief           指向下一个sector的ID用于存储数据
  * @param[in]       obj        存储对象
  * @return          0：成功   -1：全是坏块
***********************************************************************/
static int storage_next_sector(const struct StorageObject *__FAR obj) {
    struct spi_nand_info *__FAR info = (struct spi_nand_info * __FAR) spi_flash_get_chipinfo(obj->flash);
    uint32_t old_page_addr = obj->ctx->last_sector_addr / info->page_layout->sector_count;
    uint32_t new_page_addr = (obj->ctx->last_sector_addr + 1) / info->page_layout->sector_count;
    uint32_t bad_block_count = 0;
    obj->ctx->current_index = 0;
    obj->ctx->last_sector_addr++;

    if (old_page_addr != new_page_addr) {
        uint32_t old_block_addr = old_page_addr / info->pages_per_block;
        uint32_t new_block_addr = new_page_addr / info->pages_per_block;

        if (old_block_addr != new_block_addr) {
            do {
                if (new_block_addr > obj->last_block) {
                    new_block_addr = obj->first_block;
                }
                new_page_addr =  new_block_addr * info->pages_per_block;
                obj->ctx->last_sector_addr = new_page_addr * info->page_layout->sector_count;
                new_block_addr++;
                if (bad_block_count++ > (obj->last_block - obj->first_block)) {
                    // 全部都是坏块?
                    return -1;
                }
            } while (spi_flash_erase_block(obj->flash, new_page_addr));
            LOG(INFO, ("new block addr = %ld", new_block_addr));
        }
    }
    return 0;
}
/********************************************************************************************************************************
  * @brief           找到存储入口，即可以被使用的第一个sector
  * @param[in]       obj        存储对象
  * @return          0：表明该存储区域为全新,直接使用查找到的扇区   -1：该扇区为最后写入扇区,需要准备下一个扇区,并指向下一个扇区
**********************************************************************************************************************************/
static int storage_find_entry(const struct StorageObject *__FAR obj) {
    struct StorageSearchCtx test_search_ctx = {
        NULL,
        {IDLE_ID, IDLE_ID},
        {IDLE_ID, IDLE_ID},
    };
    struct BSearch test_search_block = {
        storage_cmp_with_block,
        &test_search_ctx,
    };
    struct BSearch test_search_sector = {
        storage_cmp_with_sector,
        &test_search_ctx
    };
    uint32_t ret = 0;
    struct spi_nand_info *__FAR info = (struct spi_nand_info * __FAR) spi_flash_get_chipinfo(obj->flash);
    test_search_ctx.obj = obj;
    {
        ret = binary_search_max(&test_search_block, obj->first_block, obj->last_block);
        LOG(DEBUG, ("search block = %ld", ret));
    }
    {
        uint16_t sector_per_block = info->pages_per_block * info->page_layout->sector_count;
        uint32_t first_sector = ret * sector_per_block;
        uint32_t last_sector = first_sector + sector_per_block - 1;
        ret = binary_search_max(&test_search_sector, first_sector, last_sector);
        LOG(DEBUG, ("search sector = %ld", ret));
    }
    obj->ctx->last_sector_id = storage_get_id_with_sector(obj->flash, ret);
    obj->ctx->last_sector_addr = ret;
    obj->ctx->current_index = 0;
    /**
     * last_sector_id == 0xFFFFFFFF 表明该存储区域为全新,直接使用查找到的扇区
     * last_sector_id != 0xFFFFFFFF 表明该扇区为最后写入扇区,需要准备下一个扇区,并指向下一个扇区
     */
    if (obj->ctx->last_sector_id != IDLE_ID) {
        if (storage_next_sector(obj)) {
            return -1;
        }
    }
    LOG(DEBUG, ("last_sector_id = %ld", obj->ctx->last_sector_id));
    LOG(DEBUG, ("last_sector_addr = %ld", obj->ctx->last_sector_addr));
    return 0;
}
/************************************************************************
  * @brief           数据写入
  * @param[in]       obj 存储对象
  * @return          0：写操作完成
***********************************************************************/
static int storage_buffer_sync(const struct StorageObject *__FAR obj) {
    uint32_t sector_addr = obj->ctx->last_sector_addr;
    struct spi_nand_info *__FAR info = (struct spi_nand_info * __FAR) spi_flash_get_chipinfo(obj->flash);
    uint32_t page_addr = sector_addr / info->page_layout->sector_count;
    uint16_t oob_index = (uint16_t)(sector_addr % info->page_layout->sector_count);
    uint16_t offset = info->bytes_per_page + info->page_layout->oobfree[oob_index].offset;

    LOG(DEBUG, ("sector_addr = %ld", sector_addr));
    LOG(DEBUG, ("page_addr = %ld", page_addr));
    LOG(DEBUG, ("oob_index = %d", oob_index));
    LOG(DEBUG, ("offset = %d", offset));

    spi_flash_read_page(obj->flash, page_addr);
    spi_flash_write_page_data(obj->flash, info->page_layout->sector_size * oob_index, obj->ctx->cache, obj->ctx->current_index);
    spi_flash_write_page_data(obj->flash, info->bytes_per_page + info->page_layout->oobfree[oob_index].offset, obj->ctx->spare,
                              info->page_layout->oobfree[oob_index].length);
    spi_flash_write_page(obj->flash, page_addr);
    return 0;
}
/************************************************************************
  * @brief           向spare区写入数据
  * @param[in]       obj     存储对象
  * @param[in]       pdata   待写入数据
  * @param[in]       size    待写入数据长度
  * @return          0：待写入数据长度
***********************************************************************/
static uint16_t storage_buffer_write_spare(const struct StorageObject *__FAR obj, const uint8_t *__FAR pdata, uint16_t size) {
    uint16_t i;
    for (i = 0; i < size; i++) {
        obj->ctx->spare[i] = pdata[i];
    }
    return i;
}
/*static uint16_t storage_buffer_write_cache(const struct StorageObject *__FAR obj, uint8_t *__FAR pdata, uint16_t size) {
    uint16_t i;
    for (i = 0; i < size; i++) {
        obj->ctx->cache[i] = pdata[i];
    }
    return i;
}*/
/************************************************************************
  * @brief           数据写入
  * @param[in]       obj   存储对象
  * @param[in]       pdata 待写入数据
  * @param[in]       size  待写入数据长度
  * @param[in]       callback  回调函数
  * @return          -1：写操作完成
***********************************************************************/
static uint16_t storage_buffer_append_cache(const struct StorageObject *__FAR obj, const uint8_t *__FAR pdata, uint16_t size, int (*callback)(const struct StorageObject *__FAR obj)) {
    uint16_t i;
    struct spi_nand_info *__FAR info = (struct spi_nand_info * __FAR) spi_flash_get_chipinfo(obj->flash);
    for (i = 0; i < size; i++) {
        obj->ctx->cache[obj->ctx->current_index++] = pdata[i];
        if (obj->ctx->current_index >= (info->page_layout->sector_size - ECC_SKIP)) 
        {
            callback(obj);
            storage_buffer_sync(obj);
            if (storage_next_sector(obj)) //obj->ctx->current_index will be reset here
            {
                return -1;
            }
        }
    }
    return i;
}
/************************************************************************
  * @brief           回调函数，用于存储sector的地址
  * @param[in]       obj   存储对象
  * @return          0：操作完成
***********************************************************************/
static int storage_buffer_sync_callback(const struct StorageObject *__FAR obj) {
    uint32_t now = get_system_time();
    uint32_t tmp_id;
    LOG(DEBUG, ("last sector id = %ld", obj->ctx->last_sector_id));
    if ((now > obj->ctx->last_sector_id)
        || (obj->ctx->last_sector_id == IDLE_ID)
        || (obj->ctx->last_sector_id == INVALID_ID)) 
    {
        obj->ctx->last_sector_id = now ? now : 1;
    } 
    else 
    {
        obj->ctx->last_sector_id++;
    }
    LOG(DEBUG, ("current sector id = %ld", obj->ctx->last_sector_id));
    tmp_id = htonl(obj->ctx->last_sector_id);
    storage_buffer_write_spare(obj, (const uint8_t *__FAR) &tmp_id, sizeof(tmp_id));
    return 0;
}


/********************************************************************************************************************************
  * @brief           初始化存储设备
  * @param[in]       obj   存储对象
  * @return          0：表明该存储区域为全新,直接使用查找到的扇区   -1：该扇区为最后写入扇区,需要准备下一个扇区,并指向下一个扇区
*********************************************************************************************************************************/
int storage_init(const struct StorageObject *__FAR obj) {
    int ret = storage_find_entry(obj);
    return ret;
}

/************************************************************************
  * @brief          停止存储设备
  * @param[in]       obj   存储对象
  * @return          0：操作完成 其他:失败
***********************************************************************/
int storage_deinit(const struct StorageObject *__FAR obj) {
    int ret = storage_flush(obj);
    // TODO: 其他操作
    return ret;
}

/********************************************************************************************************************************
  * @brief           存储记录
  * @param[in]       obj   存储对象
  * @param[in]       type  数据类型
  * @param[in]       pdata 数据内容
  * @param[in]       size  数据长度
  * @return          >=0：操作完成  <0：表示失败
*********************************************************************************************************************************/
int storage_save(const struct StorageObject *__FAR obj, uint16_t type, const uint8_t *__FAR pdata, uint16_t size) {
    uint16_t  i,CalibrationValue=0;
    // 需要Flash相关的对象指针
    struct RecordInfo info;
    info.head = htons(RECORD_HEAD);
    info.len =  htons(size);    // size仅包含数据的长度
    info.timestamp = htonl(get_system_time());
    info.type = htons(type);
    for(i=0;i<size;i++){    //计算数据累加和  20170112
       CalibrationValue+=*(pdata+i);   
    }
    storage_buffer_append_cache(obj, (uint8_t *__FAR) &info, sizeof(info), storage_buffer_sync_callback);
    storage_buffer_append_cache(obj, pdata, size, storage_buffer_sync_callback);
    storage_buffer_append_cache(obj, (const uint8_t *__FAR)&CalibrationValue,2, storage_buffer_sync_callback);//将校验值放到数据结尾  20170112
    return 0;
}
 /********************************************************************************************************************************
  * @brief           存储记录,用于缓存比较小的场合,通过回调的方式分次存储记录
  * @param[in]       obj   存储对象
  * @param[in]       type  数据类型
  * @param[in]       size  数据长度
  * @param[in]       cb    分次存储的回调函数,返回缓存的指针以及缓存的长度
  * @return          0：操作完成
*********************************************************************************************************************************/
int storage_save_ex(const struct StorageObject *__FAR obj, uint16_t type, uint16_t size, uint8_t *__FAR(*cb)(uint16_t *__FAR, void *__FAR), void *__FAR attach) {
    // 需要Flash相关的对象指针
    struct RecordInfo rInfo;
    uint8_t *__FAR pdata;
    uint16_t len;
    uint16_t count = 0;

    rInfo.head = htons(RECORD_HEAD);
    rInfo.len =  htons(size);    // size仅包含数据长度
    rInfo.timestamp = htonl(get_system_time());
    rInfo.type = htons(type);

    storage_buffer_append_cache(obj, (uint8_t *__FAR) &rInfo, sizeof(rInfo), storage_buffer_sync_callback);

    for (;;) {
        pdata = cb(&len, attach);
        if (pdata != NULL && len != 0) {
            count += len;
            if (count <= size) {
                storage_buffer_append_cache(obj, pdata, len, storage_buffer_sync_callback);
                if(count < size) {
                    continue;
                }
            }
        }
        break;
    }
    return 0;
}
 /********************************************************************************************************************************
  * @brief           刷新缓存,将缓存中的数据同步到存储设备上
  * @param[in]       obj   存储对象
  * @return          0：操作完成
*********************************************************************************************************************************/
int storage_flush(const struct StorageObject *__FAR obj) {
    if (obj->ctx->current_index) {
        storage_buffer_sync_callback(obj);
        storage_buffer_sync(obj);
        storage_next_sector(obj);
    }
    return 0;
}
/************************************************************************
  * @brief           对获取的ID的有效性进行检测
  * @param[in]       ctx         包含ID的所有信息
  * @param[in]       shift       用于返回索引号微调步长
  * @return          0：无效的ID   1：溢出 -1：ID有效
***********************************************************************/
inline static int storage_cmp_min(const struct StorageSearchCtx *__FAR ctx, int *shift) {
    if (ctx->left.id == INVALID_ID) {
        *shift = 1;
        return 0;
    }
    if (ctx->right.id == INVALID_ID) {
        *shift = -1;
        return 0;
    }
    *shift = 0;
    if (ctx->right.id == IDLE_ID) {return -1;}
    /**
     * 16位平台上可能会丢数据
     * return (int)(ctx->left.id - ctx->right.id);
     */
    return ctx->left.id > ctx->right.id ? 1 : -1;
}
/************************************************************************
  * @brief           更新输入块对象的ID,并进行有效性检测
  * @param[in]       thiz       查找对象
  * @param[in]       left       左值索引号
  * @param[in]       right      右值索引号
  * @param[in]       shift       用于返回索引号微调步长
  * @return          0：无效的ID   1：溢出 -1：ID有效
***********************************************************************/
static int storage_cmp_min_with_block(const struct BSearch *__FAR thiz, uint32_t left, uint32_t right, int *shift) {
    storage_update_id(thiz, left, right, storage_get_id_with_block);
    return storage_cmp_min(thiz->private_data, shift);
}
/************************************************************************
  * @brief           更新输入扇区对象的ID,并进行有效性检测
  * @param[in]       thiz       查找对象
  * @param[in]       left       左值索引号
  * @param[in]       right      右值索引号
  * @param[in]       shift      用于返回索引号微调步长
  * @return          0：无效的ID   1：溢出 -1：ID有效
***********************************************************************/
static int storage_cmp_min_with_sector(const struct BSearch *__FAR thiz, uint32_t left, uint32_t right, int *shift) {
    storage_update_id(thiz, left, right, storage_get_id_with_sector);
    return storage_cmp_min(thiz->private_data, shift);
}
/********************************************************************************************************************************
  * @brief           找到存储入口，即上次最后存储的sector
  * @param[in]       obj        存储对象
  * @return          可以被使用的第一个sector的id
**********************************************************************************************************************************/
static uint32_t storage_find_first(const struct StorageObject *__FAR obj) {
    struct StorageSearchCtx test_search_ctx = {
        NULL,
        {IDLE_ID, IDLE_ID},
        {IDLE_ID, IDLE_ID},
    };
    struct BSearch test_search_block = {
        storage_cmp_min_with_block,
        &test_search_ctx,
    };
    struct BSearch test_search_sector = {
        storage_cmp_min_with_sector,
        &test_search_ctx
    };
    uint32_t ret = 0;
    struct spi_nand_info *__FAR info = (struct spi_nand_info * __FAR) spi_flash_get_chipinfo(obj->flash);
    test_search_ctx.obj = obj;
    {
        ret = binary_search_min(&test_search_block, obj->first_block, obj->last_block);
        LOG(DEBUG, ("search block = %ld", ret));
    }
    {
        uint16_t sector_per_block = info->pages_per_block * info->page_layout->sector_count;
        uint32_t first_sector = ret * sector_per_block;
        uint32_t last_sector = first_sector + sector_per_block - 1;
        ret = binary_search_min(&test_search_sector, first_sector, last_sector);
        obj->ctx->current_index = 0;
        obj->ctx->last_sector_addr = ret;
        obj->ctx->last_sector_id = storage_get_id_with_sector(obj->flash, ret);
        LOG(DEBUG, ("last_sector_id = %ld", obj->ctx->last_sector_id));
        LOG(DEBUG, ("last_sector_addr = %ld", obj->ctx->last_sector_addr));
    }
    return obj->ctx->last_sector_id;
}
/************************************************************************
  * @brief           获取扇区的存储数据
  * @param[in]       flash      包含nandflash所有属性和操作方法
  * @param[in]       index      地址
  * @param[in]       offset     偏移量
  * @param[in]       size       数据长度
  * @return          -1：读取失败，否则返回读取的数据值
***********************************************************************/
static ssize_t storage_get_sector_data(const struct spi_flash *__FAR flash, uint32_t index, uint16_t offset, uint8_t *buffer, uint16_t size) {
    struct spi_nand_info *__FAR info = (struct spi_nand_info * __FAR) spi_flash_get_chipinfo(flash);
    uint32_t page_addr = index / info->page_layout->sector_count;
    uint16_t oob_index = (uint16_t)(index % info->page_layout->sector_count);

    if (spi_flash_read_page(flash, page_addr)) {
        return -1;
    }
    offset += (info->page_layout->sector_size * oob_index);
    return spi_flash_read_page_data(flash, offset, buffer, size);
}
/************************************************************************
  * @brief           获取扇区的存储数据
  * @param[in]       obj      存储对象
  * @return          RecordInfo结构体
***********************************************************************/
static struct RecordInfo *storage_find_package(const struct StorageObject *__FAR obj) {
#ifdef WIN32
    static const uint8_t head[] = {0xFE, 0xFE};
    struct spi_nand_info *__FAR info = (struct spi_nand_info * __FAR) spi_flash_get_chipinfo(obj->flash);
    uint16_t sector_size = info->page_layout->sector_size - 1;
    uint32_t count = sector_size;
    uint16_t left_size = sector_size - obj->ctx->current_index;
    char *buffer = malloc(count);
    uint16_t index = 0;
    struct RecordInfo *pInfo = NULL;
    uint32_t last_sector_addr = obj->ctx->last_sector_addr;
    uint32_t length;
    for (;;) {
        ssize_t ret;
        if (buffer == NULL) {
            LOG(ERROR, ("malloc error %d", count));
            return NULL;
        }
        if ((index + sector_size) > count) {
            count = index + sector_size;
            buffer = realloc(buffer, count);
            if (buffer == NULL) {
                LOG(ERROR, ("malloc error %d", count));
                return NULL;
            }
            if (pInfo) {
                pInfo = (struct RecordInfo *)buffer;
            }
        }
        if (last_sector_addr != obj->ctx->last_sector_addr) {
            uint32_t id = storage_get_id_with_sector(obj->flash, obj->ctx->last_sector_addr);
            if ((id == INVALID_ID)
                || (id == IDLE_ID)
                || (id < obj->ctx->last_sector_id)) {
                goto __error;
            }
            last_sector_addr = obj->ctx->last_sector_addr;
        }
        LOG(VERBOSE, ("index = %d, left_size = %d, count = %d", index, left_size, count));
        ret = storage_get_sector_data(obj->flash, obj->ctx->last_sector_addr, obj->ctx->current_index, &buffer[index], left_size);
        if (ret == left_size) {
            index += left_size;
            if (pInfo) {
                length = pInfo->len + sizeof(struct RecordInfo);
                if (length <= index) {
                    obj->ctx->current_index = sector_size - (index - length);
                    return pInfo;
                }
            } else {
                ret = memfind(buffer, index, head, sizeof(head));
                if (ret >= 0) {
                    index -= ret;
                    memmove(buffer, &buffer[ret], index);
                    length = sizeof(struct RecordInfo);
                    if (length <= index) {
                        pInfo = (struct RecordInfo *)buffer;

                        pInfo->len = ntohs(pInfo->len);
                        pInfo->type = ntohs(pInfo->type);
                        pInfo->timestamp = ntohl(pInfo->timestamp);

                        length += pInfo->len;
                        if (length <= index) {
                            if (obj->ctx->current_index) {
                                obj->ctx->current_index += length;
                            } else {
                                obj->ctx->current_index = sector_size - (index - length);
                            }
                            return pInfo;
                        }
                    }
                }
            }
        }
        obj->ctx->last_sector_addr++;
        obj->ctx->current_index = 0;
        left_size = sector_size;
        {
            /**
             * 查找到最后一个块,需要调头啦
             */
            uint32_t new_page_addr = obj->ctx->last_sector_addr / info->page_layout->sector_count;
            uint16_t new_block_addr = new_page_addr / info->pages_per_block;
            if (new_block_addr > obj->last_block) {
                obj->ctx->last_sector_addr = obj->first_block * info->pages_per_block * info->page_layout->sector_count;
            }
        }
    }
__error:
    free(buffer);
#else
    (void)obj;
#endif
    return NULL;
}

static struct RecordInfo *storage_read(const struct StorageObject *__FAR obj, uint16_t type) {
    for (;;) {
        struct RecordInfo *pInfo = storage_find_package(obj);
        if (pInfo) {
            if ((type == 0xFFFF) || (type == pInfo->type)) {
                return pInfo;
            } else {
                free(pInfo);
            }
        } else {
            break;
        }
    }
    return NULL;
}

char *storage_take(const struct StorageObject *__FAR obj, uint16_t *size, uint16_t type) {
#ifdef WIN32
    struct RecordInfo *pInfo = storage_read(obj, type);
    if (pInfo) {
        char *buf = malloc(pInfo->len + 1);
        LOG(INFO, ("info .type = %d, .len = %d, .timestamp = %ld", pInfo->type, pInfo->len, pInfo->timestamp));
        if (buf == NULL) {
            LOG(ERROR, ("malloc error %d", pInfo->len + 1));
            free(pInfo);
            return NULL;
        }
        memcpy(buf, (char *)pInfo + sizeof(*pInfo), pInfo->len);
        if (size) {
            *size = pInfo->len;
        }
        buf[pInfo->len] = 0;
        free(pInfo);
        return buf;

    }
#else
    (void)obj;
    (void)size;
    (void)type;
#endif
    return NULL;
}

int storage_read_reset(const struct StorageObject *__FAR obj) {
#ifdef WIN32
    uint32_t id = storage_find_first(obj);
    if ((id == INVALID_ID)
        || (id == IDLE_ID)) {
        LOG(DEBUG, ("not found data"));
        return -1;
    }
    return 0;
#else
    (void)obj;
    return -1;
#endif
}

int storage_dump_id(const struct StorageObject *__FAR obj, uint16_t block) {
    uint16_t i;
    uint16_t first_block = obj->first_block;
    uint16_t last_block = obj->last_block;
    struct spi_nand_info *__FAR info = (struct spi_nand_info * __FAR)spi_flash_get_chipinfo(obj->flash);
    if (block == 0xFFFF) {
        for (i = first_block; i <= last_block; i++) {
            printf("[%4d].id = %10d\n", i, storage_get_id_with_block(obj->flash, i));
        }
        return 0;
    } else {
        if ((block >= first_block) && (block <= last_block)) {
            uint16_t sector_count = info->pages_per_block * info->page_layout->sector_count;
            uint32_t sector_addr = block * sector_count;
            for (i = 0; i < sector_count; i++, sector_addr++) {
                printf("[%6d].id = %10d\n", sector_addr, storage_get_id_with_sector(obj->flash, sector_addr));
            }
            return 0;
        }
    }
    return -1;
}

int storage_dump_record(const struct StorageObject *__FAR obj, uint16_t type) {
    int ret = -1;
#ifdef WIN32
    for (;;) {
        struct RecordInfo *pInfo = storage_read(obj, type);
        if (pInfo) {
            int i;
            uint8_t *pdata = (uint8_t *)pInfo + sizeof(struct RecordInfo);
            printf("0x%04X,", pInfo->head);
            printf("%ld,", pInfo->timestamp);
            printf("0x%04X,", pInfo->type);
            printf("%d,", pInfo->len);
            printf("[");
            for (i = 0; i < pInfo->len; i++) {
                printf("%02X ", pdata[i]);
            }
            printf("]\n");
            free(pInfo);
            ret = 0;
        } else {
            break;
        }
    }
#else
    (void)obj;
    (void)type;
#endif
    return ret;
}

int storage_cat(const struct StorageObject *__FAR obj) {
    int ret = -1;
#ifdef WIN32
    uint16_t size = 0;
    char *buf = NULL;
    while ((buf = storage_take(obj, &size, 0xABCD)) != NULL) {
        printf("%s\n", buf);
        free(buf);
        buf = NULL;
        ret = 0;
    }
#else
    (void)obj;
#endif
    return ret;
}
/************************************************************************
  * @brief           二分法查找最大值
  * @param[in]       thiz       查找对象
  * @param[in]       start      起始地址
  * @param[in]       end        结束地址
  * @return          最大值
***********************************************************************/
uint32_t binary_search_max(const struct BSearch *__FAR thiz, uint32_t start, uint32_t end) {
    uint32_t middle = (start + end) >> 1;
    int ret = 0;
    int shift = 0;
    if (middle == 0) return 0;
    while (start < middle) {
        ret = thiz->cmp(thiz, start, middle, &shift);
        if (shift) {
            if (shift > 0) start += shift;
            else middle += shift;
        } else {
            if (ret < 0) {
                start = middle;
            } else {
                end = middle;
            }
            middle = (start + end) >> 1;
        }
        LOG(TRACE, ("start: %d, middle: %d, end: %d", start, middle, end));
    }
    ret = thiz->cmp(thiz, middle, end, &shift);
    if (shift) {
        return shift > 0 ? end : middle;
    }
    return ret >= 0 ? middle : end;
}
/************************************************************************
  * @brief           二分法查找最小值
  * @param[in]       thiz       查找对象
  * @param[in]       start      起始地址
  * @param[in]       end        结束地址
  * @return          最小值
***********************************************************************/
uint32_t binary_search_min(const struct BSearch *__FAR thiz, uint32_t start, uint32_t end) {
    uint32_t middle = (start + end) >> 1;
    int ret = 0;
    int shift = 0;
    if (middle == 0) return 0;
    while(middle < end) {
        ret = thiz->cmp(thiz, middle, end, &shift);
        if (shift) {
            if (shift > 0) middle += shift;
            else end += shift;
        }else{
            if(ret < 0) {
                end = middle;
            }else{
                start = middle;
            }
            middle = (start + 1 + end) >> 1;
        }
        LOG(TRACE, ("start: %d, middle: %d, end: %d", start, middle, end));
    }
    ret = thiz->cmp(thiz, start, middle, &shift);
    if(shift) {
        return shift > 0 ? middle : start;
    }
    return ret < 0 ? start : middle;
}
/************************************************************************
  * @brief           将对象值写入缓存
  * @param[in]       dest       目标缓存
  * @param[in]       dest_size  目标缓存长度
  * @param[in]       src        对象值
  * @param[in]       src_size   对象长度
  * @return          -1：操作失败 其他：成功写入
***********************************************************************/
ssize_t memfind(const uint8_t *__FAR dest, ssize_t dest_size, const uint8_t *__FAR src, ssize_t src_size) {
    ssize_t i = 0;
    ssize_t j;
    if (dest && src && src_size) {
        if (dest_size >= src_size) {
            dest_size -= src_size;
            for (i = 0; i <= dest_size; i++) {
                for (j = 0; (dest[i + j] == src[j]) && (j < src_size); j++) {}
                if (j == src_size) {
                    return i;
                }
            }
        }
    }
    return -1;
}
//#endif
