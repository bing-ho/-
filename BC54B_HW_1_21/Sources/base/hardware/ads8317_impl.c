/**
 *
 * Copyright (c) 2014 Ligoo Inc.
 *
 * @file  ads8137_impl.c
 * @brief ADS8137µÄSPI¶ÁÐ´Çý¶¯
 * @note 
 * @version 1.00
 * @author Liwei Dong
 * @date 2014/9/17
 *
 */
 
#include "ads8317_impl.h"


void ads8317_init(void)
{
    ADS8317_CS_HIGH();
    SPI1_Init();
}

INT16U ads8317_read(void)
{
    OS_CPU_SR cpu_sr = 0;
    static INT32U ad_data = 0;
    
    OS_ENTER_CRITICAL();
    ADS8317_CS_LOW();

    ad_data = SPI1_SendByte(0xFF);
    ad_data <<= 8;
    ad_data += SPI1_SendByte(0xFF);
    ad_data <<= 8;
    ad_data += SPI1_SendByte(0xFF);

    ADS8317_CS_HIGH();
    OS_EXIT_CRITICAL();
    ad_data >>= 2;
    
    return ((INT16U)ad_data);
}