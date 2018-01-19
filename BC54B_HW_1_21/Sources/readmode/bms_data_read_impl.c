/**
* Copyright (c) 2012 Ligoo Inc.
*
* @file bms_data_read_impl.c
* @brief   读nandflash数据
* @author  Bing He
* @date 2017-10-24
*
*/

#include "bms_data_read_impl.h"

#pragma DATA_SEG __RPAGE_SEG PAGED_RAM

static INT32U g_start_page_addr = 0;
static INT32U g_stop_page_addr = 0;

#pragma DATA_SEG DEFAULT


INT8U g_data_scan_require = 0;
INT32U g_start_date = 0;
INT32U g_end_date = 0;

INT32U bms_get_date_with_sector(const struct spi_flash *__FAR flash, uint32_t index);
INT16S block_is_bad(const struct StorageObject *__FAR obj, INT32U sector_addr); 
INT32U binary_search(const struct StorageObject *__FAR obj, INT32U start_required_date, INT32U stop_required_date, INT32U first_sector_addr, INT32U second_sector_addr);
INT32U sector_addr_find_in_flash(const struct StorageObject *__FAR obj, INT32U start_required_date, INT32U stop_required_date);
void bms_set_nand_start_page_addr(INT32U page_addr);
void bms_set_nand_stop_page_addr(INT32U page_addr);
/************************************************************************
  * @brief           获取对应扇区地址所存储的时间
  * @param[in]       platform               包含nandflash的所有属性和操作方法
  * @param[in]       start_required_date    请求的起始时间
  * @param[in]       start_required_date    请求的终止时间  
  * return           0：没有找到     其它： 请求日期的起始页地址 
***********************************************************************/
INT32U bms_get_date_with_sector(const struct spi_flash *__FAR flash, uint32_t index) 
{
    INT32U id = INVALID_ID;
    struct spi_nand_info *__FAR info = (struct spi_nand_info * __FAR) spi_flash_get_chipinfo(flash);
    INT32U page_addr = index / info->page_layout->sector_count;       
    INT16U oob_index = (INT16U)(index % info->page_layout->sector_count);  
    INT16U offset = info->bytes_per_page + info->page_layout->oobfree[oob_index].offset;   

    int ret = spi_flash_read_page(flash, page_addr);  
    if (ret == 0) {                   
        spi_flash_read_page_data(flash, offset, (uint8_t *__FAR) &id, sizeof(id));  
        return ntohl(id);
    }
    return INVALID_ID;
} 


/************************************************************************
  * @brief           判断是否为坏块
  * @param[in]       platform               包含nandflash的所有属性和操作方法
  * @param[in]       sector_addr            扇区地址
  * return           0：正常         -1： error      -128:坏块
***********************************************************************/
INT16S block_is_bad(const struct StorageObject *__FAR obj, INT32U sector_addr) 
{
    INT16U bad_block_marker = 0;
    INT32U new_page_addr;
    struct spi_nand_info *__FAR info = (struct spi_nand_info * __FAR) spi_flash_get_chipinfo(obj->flash);
    INT32U page_addr = sector_addr / info->page_layout->sector_count;
    INT32U block_addr = page_addr / info->pages_per_block;

    new_page_addr = block_addr * info->pages_per_block;
    
    if (spi_flash_read_page(obj->flash, new_page_addr)) {    
        return -1;
    }
    if (spi_flash_read_page_data(obj->flash, info->bytes_per_page, (uint8_t *__FAR)&bad_block_marker, sizeof(bad_block_marker)) != sizeof(bad_block_marker)) {
        return -1;                     
    }
    if (bad_block_marker != 0xffff) {    //bad block标记不为0xffff，则此block为坏块
        return -128;
    }else{
        return 0;
    }
}

/************************************************************************
  * @brief           二分法寻找包含请求日期的页地址
  * @param[in]       platform               包含nandflash的所有属性和操作方法
  * @param[in]       start_required_date    请求的起始时间
  * @param[in]       start_required_date    请求的终止时间  
  * return           0：寻找结束并没有找到     其它： 请求日期的页地址 
***********************************************************************/
INT32U binary_search(const struct StorageObject *__FAR obj, INT32U start_required_date, INT32U stop_required_date, INT32U first_sector_addr, INT32U second_sector_addr)
{
    INT32U middle_sector_addr = 0; 
    INT32U date = 0;
    
    while(second_sector_addr >= first_sector_addr)
    {
        middle_sector_addr =(first_sector_addr + second_sector_addr)/2;   
        while(block_is_bad(obj, middle_sector_addr))  //坏块
        {
            middle_sector_addr++;    
        }
        date = bms_get_date_with_sector(obj->flash, middle_sector_addr);
        
        if(date > stop_required_date)
        {
            second_sector_addr = middle_sector_addr - 1;
        } 
        else 
        {
            if(date < start_required_date || date == IDLE_ID)
            {
                first_sector_addr = middle_sector_addr + 1;
            }
            else 
            {
                if((date >= start_required_date) && (date <= stop_required_date))
                {
                    return middle_sector_addr;
                }
            }
        }
    }
    return 0;
}

/************************************************************************
  * @brief           在FLASH中寻找请求日期的页地址
  * @param[in]       platform               包含nandflash的所有属性和操作方法
  * @param[in]       start_required_date    请求的起始时间
  * @param[in]       start_required_date    请求的终止时间  
  * return           0：没有找到     其它： 请求日期的页地址 
***********************************************************************/
INT32U sector_addr_find_in_flash(const struct StorageObject *__FAR obj, INT32U start_required_date, INT32U stop_required_date)
{ 
    INT32U ret, date_last, date_first, first_sector_addr, finally_sector_addr;
    INT32U last_sector_addr = obj->ctx->last_sector_addr - 1;
    
    struct spi_nand_info *__FAR info = (struct spi_nand_info * __FAR) spi_flash_get_chipinfo(obj->flash);
    first_sector_addr = obj->first_block * info->pages_per_block * info->page_layout->sector_count;    
    finally_sector_addr = ((INT32U)(obj->last_block + 1) * info->pages_per_block * info->page_layout->sector_count - 1);
                                                                                   
    while(block_is_bad(obj, last_sector_addr)){
        last_sector_addr--;
    }
    date_last = bms_get_date_with_sector(obj->flash, last_sector_addr);  //最新写入的扇区日期提取
    
    while(block_is_bad(obj, first_sector_addr) 
          || (bms_get_date_with_sector(obj->flash, first_sector_addr) > date_last))
    {
        first_sector_addr++;
    }
    date_first = bms_get_date_with_sector(obj->flash, first_sector_addr); //起始扇区的日期提取  
    
    while(block_is_bad(obj, finally_sector_addr)){
        finally_sector_addr--;         
    }
               
    if(start_required_date > date_last)    
    {
        return 0;
    } 
    else
    {
        if(date_first <= stop_required_date) //位置  64 -- last block
        {
            ret = binary_search(obj, start_required_date, stop_required_date, first_sector_addr, last_sector_addr);   
            return ret;   
        }
        else           // 位置 last block -- 1023
        {
            ret = binary_search(obj, start_required_date, stop_required_date, last_sector_addr, finally_sector_addr);        
            return ret;
        } 
    }   
}


/************************************************************************
  * @brief           寻找请求日期的起始页地址
  * @param[in]       platform               包含nandflash的所有属性和操作方法
  * @param[in]       start_required_date    请求的起始时间
  * @param[in]       start_required_date    请求的终止时间  
  * return           0：没有找到            1： 找到 
***********************************************************************/
INT8U page_addr_extract(const struct StorageObject *__FAR obj, INT32U start_required_date, INT32U stop_required_date)
{   
    INT32U sector_addr, temp, date;
    INT32U old_block_addr, new_block_addr;
    struct spi_nand_info *__FAR info = (struct spi_nand_info * __FAR) spi_flash_get_chipinfo(obj->flash);
    INT32U first_sector_addr = obj->first_block * info->pages_per_block * info->page_layout->sector_count;   //64*64*4
    INT32U finally_sector_addr = ((INT32U)(obj->last_block + 1) * info->pages_per_block * info->page_layout->sector_count - 1);   //1024*64*4-1
 
    sector_addr = sector_addr_find_in_flash(obj, start_required_date, stop_required_date);   //寻找包含请求日期的页
    date = bms_get_date_with_sector(obj->flash, sector_addr);
    temp = sector_addr;
    
    if(sector_addr > 0)   
    {
        //起始页地址提取
        do{
            old_block_addr = sector_addr / info->page_layout->sector_count / info->pages_per_block;            
            sector_addr = sector_addr - info->page_layout->sector_count;
            if(sector_addr < first_sector_addr)
            {
                sector_addr = finally_sector_addr;
            }
            new_block_addr = sector_addr / info->page_layout->sector_count / info->pages_per_block;           
            if(new_block_addr != old_block_addr)
            {
                if(block_is_bad(obj, sector_addr) == 0)
                {
                    date = bms_get_date_with_sector(obj->flash, sector_addr);
                }else
                {
                    date = INVALID_ID;  //坏块
                }   
            }else
            {
                if(date != INVALID_ID)
                {
                    date = bms_get_date_with_sector(obj->flash, sector_addr);
                }                 
            }
        }while((date >= start_required_date && date != IDLE_ID) || date == INVALID_ID);
        bms_set_nand_start_page_addr(sector_addr / info->page_layout->sector_count);
        
        
        //终止页地址提取
        do{
            old_block_addr = temp / info->page_layout->sector_count / info->pages_per_block;
            
            temp = temp + info->page_layout->sector_count; 
            if(temp > finally_sector_addr)                
            {
                temp = first_sector_addr;
            }
            new_block_addr = temp / info->page_layout->sector_count / info->pages_per_block;
   
            if(new_block_addr != old_block_addr)
            {
                if(block_is_bad(obj, temp) == 0)
                {
                    date = bms_get_date_with_sector(obj->flash, temp);
                }else
                {
                    date = INVALID_ID;
                }   
            }else
            {
                if(date != INVALID_ID)
                {
                    date = bms_get_date_with_sector(obj->flash, temp);
                }
                    
            }
        }while(date <= stop_required_date || date == INVALID_ID);
        bms_set_nand_stop_page_addr(temp / info->page_layout->sector_count);
        
        return 1;
              
    } else
    {
        bms_set_nand_start_page_addr(0);
        bms_set_nand_stop_page_addr(0);
        
        return 0;
    }
    
}


void bms_set_nand_start_page_addr(INT32U page_addr)
{
    g_start_page_addr = page_addr;
}

INT32U bms_get_nand_start_page_addr(void)
{
    return g_start_page_addr;
}


void bms_set_nand_stop_page_addr(INT32U page_addr)
{
    g_stop_page_addr = page_addr;
}

INT32U bms_get_nand_stop_page_addr(void)
{
    return g_stop_page_addr;
}


void bms_set_nandflash_scan_require(INT32U start_date,INT32U end_date) 
{
    g_data_scan_require = 1;
    g_start_date = start_date;
    g_end_date = end_date;    
}


INT8U bms_get_nandflash_scan_require(void)
{
    return g_data_scan_require;
}


 
