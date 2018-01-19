/**
* Copyright (c) 2012 Ligoo Inc.
*
* @file bms_data_read_impl.c
* @brief   ��nandflash����
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
  * @brief           ��ȡ��Ӧ������ַ���洢��ʱ��
  * @param[in]       platform               ����nandflash���������ԺͲ�������
  * @param[in]       start_required_date    �������ʼʱ��
  * @param[in]       start_required_date    �������ֹʱ��  
  * return           0��û���ҵ�     ������ �������ڵ���ʼҳ��ַ 
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
  * @brief           �ж��Ƿ�Ϊ����
  * @param[in]       platform               ����nandflash���������ԺͲ�������
  * @param[in]       sector_addr            ������ַ
  * return           0������         -1�� error      -128:����
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
    if (bad_block_marker != 0xffff) {    //bad block��ǲ�Ϊ0xffff�����blockΪ����
        return -128;
    }else{
        return 0;
    }
}

/************************************************************************
  * @brief           ���ַ�Ѱ�Ұ����������ڵ�ҳ��ַ
  * @param[in]       platform               ����nandflash���������ԺͲ�������
  * @param[in]       start_required_date    �������ʼʱ��
  * @param[in]       start_required_date    �������ֹʱ��  
  * return           0��Ѱ�ҽ�����û���ҵ�     ������ �������ڵ�ҳ��ַ 
***********************************************************************/
INT32U binary_search(const struct StorageObject *__FAR obj, INT32U start_required_date, INT32U stop_required_date, INT32U first_sector_addr, INT32U second_sector_addr)
{
    INT32U middle_sector_addr = 0; 
    INT32U date = 0;
    
    while(second_sector_addr >= first_sector_addr)
    {
        middle_sector_addr =(first_sector_addr + second_sector_addr)/2;   
        while(block_is_bad(obj, middle_sector_addr))  //����
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
  * @brief           ��FLASH��Ѱ���������ڵ�ҳ��ַ
  * @param[in]       platform               ����nandflash���������ԺͲ�������
  * @param[in]       start_required_date    �������ʼʱ��
  * @param[in]       start_required_date    �������ֹʱ��  
  * return           0��û���ҵ�     ������ �������ڵ�ҳ��ַ 
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
    date_last = bms_get_date_with_sector(obj->flash, last_sector_addr);  //����д�������������ȡ
    
    while(block_is_bad(obj, first_sector_addr) 
          || (bms_get_date_with_sector(obj->flash, first_sector_addr) > date_last))
    {
        first_sector_addr++;
    }
    date_first = bms_get_date_with_sector(obj->flash, first_sector_addr); //��ʼ������������ȡ  
    
    while(block_is_bad(obj, finally_sector_addr)){
        finally_sector_addr--;         
    }
               
    if(start_required_date > date_last)    
    {
        return 0;
    } 
    else
    {
        if(date_first <= stop_required_date) //λ��  64 -- last block
        {
            ret = binary_search(obj, start_required_date, stop_required_date, first_sector_addr, last_sector_addr);   
            return ret;   
        }
        else           // λ�� last block -- 1023
        {
            ret = binary_search(obj, start_required_date, stop_required_date, last_sector_addr, finally_sector_addr);        
            return ret;
        } 
    }   
}


/************************************************************************
  * @brief           Ѱ���������ڵ���ʼҳ��ַ
  * @param[in]       platform               ����nandflash���������ԺͲ�������
  * @param[in]       start_required_date    �������ʼʱ��
  * @param[in]       start_required_date    �������ֹʱ��  
  * return           0��û���ҵ�            1�� �ҵ� 
***********************************************************************/
INT8U page_addr_extract(const struct StorageObject *__FAR obj, INT32U start_required_date, INT32U stop_required_date)
{   
    INT32U sector_addr, temp, date;
    INT32U old_block_addr, new_block_addr;
    struct spi_nand_info *__FAR info = (struct spi_nand_info * __FAR) spi_flash_get_chipinfo(obj->flash);
    INT32U first_sector_addr = obj->first_block * info->pages_per_block * info->page_layout->sector_count;   //64*64*4
    INT32U finally_sector_addr = ((INT32U)(obj->last_block + 1) * info->pages_per_block * info->page_layout->sector_count - 1);   //1024*64*4-1
 
    sector_addr = sector_addr_find_in_flash(obj, start_required_date, stop_required_date);   //Ѱ�Ұ����������ڵ�ҳ
    date = bms_get_date_with_sector(obj->flash, sector_addr);
    temp = sector_addr;
    
    if(sector_addr > 0)   
    {
        //��ʼҳ��ַ��ȡ
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
                    date = INVALID_ID;  //����
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
        
        
        //��ֹҳ��ַ��ȡ
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


 
