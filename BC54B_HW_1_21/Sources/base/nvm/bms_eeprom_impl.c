/**
 *
 * Copyright (c) 2011 Ligoo Inc.
 *
 * @file  Eeprom.c
 * @brief Eeprom�ײ�����
 * @note
 * @version 1.00
 * @author
 * @date 2011/08/16
 *
 */
#include "bms_eeprom_impl.h"


#if BMS_SUPPORT_EEPROM

#pragma MESSAGE DISABLE C1420 // Result of function-call is ignored

#pragma DATA_SEG __RPAGE_SEG IEE1_DATA

#define EEPROM_ONE_PAGE_STORE_INDEX_MAX     512

const INT16U g_eeprom_block[4] =
{ 0x000, 0x800, 0x1000, 0x1800}; /*EEPROM�����׵�ַ*/

word g_backup_array[128]; /* Array for backup data from erased sector */

#pragma DATA_SEG DEFAULT

#pragma CODE_SEG IEE1_CODE


// TODO: ���¶��ֽ���Ҫ�Ż�
void eeprom_init(void)
{
    iee1_init();
    iee1_wait_ready();
}

void eeprom_uninit(void)
{
}

Result eeprom_wait_ready(void)
{
    return iee1_wait_ready();
}

Result eeprom_raw_write_int16(INT16U dst, INT16U value)
{
    Result res = RES_ERR;
    INT8U retry_cnt = 3;
    OS_CPU_SR cpu_sr = 0;

    while(res != ERR_OK && retry_cnt--)
    {
        OS_ENTER_CRITICAL();
        iee1_wait_ready();
        wdt_feed_dog();
        res = iee1_set_word((IEE1TAddress)(IEE1_AREA_ADRESS_START + dst), value);
        OS_EXIT_CRITICAL();
    }
    return res;
}

Result eeprom_raw_read_int16(INT16U dst, INT16U* _PAGED data)
{
    Result res = RES_ERR;
    INT8U retry_cnt = 3;
    OS_CPU_SR cpu_sr = 0;

    if (data == NULL) return ERR_INVALID_ARG;
    while(res !=  ERR_OK && retry_cnt--)
    {
        OS_ENTER_CRITICAL();
        iee1_wait_ready();
        res = iee1_get_word((IEE1TAddress)(IEE1_AREA_ADRESS_START + dst), data);
        OS_EXIT_CRITICAL();
    }
    return res;
}

/*
 ************************************************************************
 �������ƣ�eeprom_save_int16u_with_validation
 ��    �ܣ�����һ���޷����������ݵ�EEPROM��
 ��    ����unsigned int addr_base ������EEPROM�еĻ���ַ
 unsigned char offset   ���ݵ�ƫ�Ƶ�ַ
 unsigned int* value    ���ݵĴ��ָ��
 �� �� ֵ��unsigned char ���������Ƿ�ɹ�
 ��    �ߣ�����ΰ
 ����ʱ�䣺2011��7��1��
 �޸�ʱ�䣺
 ��    ע��������4�ֽڶ����ŵģ���2�ֽڴ��ԭ�룬��2�ֽڴ�ŷ���
 ************************************************************************
 */
Result eeprom_save_int16u_with_validation(INT16U addr_base, INT16U index, INT16U value) //�洢����У��
{
    Result res;
    INT16U nvalue, addr;

    /** дһ��������Ҫ4�ֽڿռ䣬���ռ��Ƿ��������� */
    if(index >= 512) 
    {
      addr = addr_base + EEPROM_INT16_SAVE_SIZE * (index-512);
    } 
    else 
    {
      addr = addr_base + EEPROM_INT16_SAVE_SIZE * index;
    }
    if (addr + EEPROM_INT16_SAVE_SIZE > addr_base + BMS_EEPROM_MAX_ADDRESS_OFFSET) 
        return EEPROM_ADDR_ILL;
    /** �������� */
    nvalue = ~value;

    /** дԭʼ��ֵ */
    res = eeprom_raw_write_int16(addr, value);
    if (res != RES_OK) return res;

    /** ����һλ��д���� */
    res = eeprom_raw_write_int16(addr + 2, nvalue);
    return res;
}

/*
 ************************************************************************
 �������ƣ�eeprom_load_int16u_with_validation
 ��    �ܣ���ȡEEPROM�е�һ���޷�����������
 ��    ����unsigned int addr_base ������EEPROM�еĻ���ַ
 unsigned char offset   ���ݵ�ƫ�Ƶ�ַ
 unsigned int* data    ��ȡ�����ݵĴ��ָ��
 �� �� ֵ��unsigned char ��ȡ�������Ƿ���Ч(�ɹ�)
 ��    �ߣ�����ΰ
 ����ʱ�䣺2011��7��1��
 �޸�ʱ�䣺
 ��    ע��������4�ֽڶ����ŵģ���2�ֽڴ��ԭ�룬��2�ֽڴ�ŷ���
 ************************************************************************
 */
Result eeprom_load_int16u_with_validation(INT16U addr_base, INT16U index, INT16U* _PAGED data)
{
    INT16U value = 0, nvalue;
    Result res;
    INT16U addr = 0;

    if(index >= 512) 
    {
      addr = addr_base + EEPROM_INT16_SAVE_SIZE * (index-512);
    } 
    else 
    {
      addr = addr_base + EEPROM_INT16_SAVE_SIZE * index;
    }
    if (data == NULL) return ERR_INVALID_ARG;
    if (addr + EEPROM_INT16_SAVE_SIZE > addr_base + BMS_EEPROM_MAX_ADDRESS_OFFSET) return EEPROM_ADDR_ILL;

    res = eeprom_raw_read_int16(addr, &value);
    if (res != RES_OK) return res;

    res = eeprom_raw_read_int16(addr + 2, &nvalue);
    if (res != RES_OK) return res;

    if (value + nvalue != 0xFFFF) return ERR_EEPROM_DATA_FAIL;

    *data = value;

    return RES_OK;
}

Result eeprom_save_int16u(INT16U index, INT16U value)
{
    Result res;

    INT8U CurPartitionIdx = index >> 9; //0~511 store to partion 0, 512~1023: partion 1

    eeeprom_wait_ftm_to_idle();

    // write into the primary area
    res = eeprom_save_int16u_with_validation(g_eeprom_block[CurPartitionIdx*2 + 0], index, value);
    if (res != RES_OK) 
    {
        return res;
    }

    // write into the backup area
    res = eeprom_save_int16u_with_validation(g_eeprom_block[CurPartitionIdx*2 + 1], index, value);
    if (res != RES_OK)
    {
        return res;
    }


    return RES_OK;
}

//��ȡһ������
Result eeprom_load_int16u(INT16U index, INT16U* _PAGED value)
{
    Result res;
    INT8U CurPartitionIdx = index >> 9; //0~511 store to partion 0, 512~1023: partion 1

    /** read the value from the primary area */
    res = eeprom_load_int16u_with_validation(g_eeprom_block[CurPartitionIdx*2 + 0], index, value);
    if (res == RES_OK)
    {
        return res;
    }

    /** read the value from the backup area */
    res = eeprom_load_int16u_with_validation(g_eeprom_block[CurPartitionIdx*2 + 1], index, value);
    if (res == RES_OK)
    {
        return res;
    }

    return res;
}

Result eeprom_repair_int16u_item(INT16U index, INT16U default_value)
{
    Result res;
    INT16U old_value = 0;
    INT16U primary_value, backup_value;
    Result primary_res, backup_res;

    INT8U CurPartitionIdx = index >> 9; //0~511 store to partion 0, 512~1023: partion 1
    
    /** read the data */
    primary_res = eeprom_load_int16u_with_validation(g_eeprom_block[CurPartitionIdx*2 + 0], index, &primary_value);
    backup_res = eeprom_load_int16u_with_validation(g_eeprom_block[CurPartitionIdx*2 + 1], index, &backup_value);

    old_value = default_value;
    if (primary_res == RES_OK)
    {
        old_value = primary_value;
    }
    else if(backup_res == RES_OK)
    {
        old_value = backup_value;
    }

    /** repair the primary area if needed */
    if(primary_res != RES_OK || primary_value != old_value)
    {
       res = eeprom_save_int16u_with_validation(g_eeprom_block[CurPartitionIdx*2 + 0], index, old_value);
       if (res != RES_OK) return res;
    }

    /** repair the backup area if needed */
    if(backup_res != RES_OK || backup_value != old_value)
    {
        eeprom_save_int16u_with_validation(g_eeprom_block[CurPartitionIdx*2 + 1], index, old_value);
        if (res != RES_OK) return res;
    }

    return RES_OK;
}
#if( SWITCH_ON == UDS_SWITCH_CONFIG)
/*
 ************************************************************************
 �������ƣ� eeprom_load_block_UDS
 ��    �ܣ�  ר�ô�UDS�����ȡ���� 
 ��    ���� 
 �� �� ֵ?
 ��    �ߣ�������
 ����ʱ�䣺2017��2��6��
 �޸�ʱ�䣺
 ��    ע�� 
 ************************************************************************
 */
 
 /*��EE��ַ����ֱ�Ӷ�ȡ256�ֽ�����*/
Result eeprom_save_block_uds(INT16U dst, INT16U* _PAGED data)
{
    Result res = RES_ERR;
    INT8U retry_cnt = 3;
    OS_CPU_SR cpu_sr = 0;

    while(res != ERR_OK && retry_cnt--)
    {
        OS_ENTER_CRITICAL();
        iee1_wait_ready();
        wdt_feed_dog();
        res = iee1_set_block((IEE1TAddress)(EEPROM_UDS_START_ADDRESS + dst), data);  /*ת���ɻ���ַ���в���*/
        OS_EXIT_CRITICAL();
    }
    return res;
}

Result eeprom_load_block_uds(INT16U dst, INT16U* _PAGED data)
{
    Result res = RES_ERR;
    INT8U retry_cnt = 3;
    OS_CPU_SR cpu_sr = 0;

    if (data == NULL) return ERR_INVALID_ARG;
    while(res !=  ERR_OK && retry_cnt--)
    {
        //OS_ENTER_CRITICAL();
        iee1_wait_ready();
        res = iee1_get_block((IEE1TAddress)(EEPROM_UDS_START_ADDRESS + dst), data);
        //OS_EXIT_CRITICAL();
    }
    return res;
}

Result eeprom_raw_read_int16_uds(INT16U dst, INT16U* _PAGED data)
{
    Result res = RES_ERR;
    INT8U retry_cnt = 3;
    OS_CPU_SR cpu_sr = 0;

    if (data == NULL) return ERR_INVALID_ARG;
    while(res !=  ERR_OK && retry_cnt--)
    {
        OS_ENTER_CRITICAL();
        iee1_wait_ready();
        res = iee1_get_word_uds((IEE1TAddress)(EEPROM_UDS_START_ADDRESS + dst), data);
        OS_EXIT_CRITICAL();
    }
    return res;
}

#endif

#endif
