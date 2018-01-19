/**
 *
 * Copyright (c) 2014 Ligoo Inc.
 *
 * @file  ads1013_impl.c
 * @brief ADS1013的IIC读写驱动
 * @note 使用连续转换模式
 * @version 1.00
 * @author Liwei Dong
 * @date 2014/3/3
 *
 */
 
#include "ads1013_impl.h"

#pragma MESSAGE DISABLE C5703 // Parameter 'xx' declared in function 'xx' but not referenced

#pragma DATA_SEG __RPAGE_SEG PAGED_RAM
 
static INT8U g_ads1013InitFlag = 0;
static INT8U g_ads1013CxtCnt = 0;
static INT8U g_ads1013UsingFlag = 0;
static ADS1013_CXT g_ads1013CxtBuff[ADS1013_SUPPORT_DEV_MAX];

#pragma DATA_SEG DEFAULT

#if 0

ADS1013_HANDLE ads1013_init(I2C_REG* reg, INT16U config)
{
    ads1013_cxt_t cxt = NULL;
    
    //cxt init
    if(g_ads1013CxtCnt >= ADS1013_SUPPORT_DEV_MAX) goto Ads1013ErrSeg;
    
    cxt = ads1013_get_free_cxt();
    if(cxt == NULL) goto Ads1013ErrSeg;
    
    g_ads1013CxtCnt++;
    cxt->id = g_ads1013CxtCnt;
    cxt->i2c_handle = i2c_init(reg);
    if(cxt->i2c_handle == 0xFF) goto Ads1013ErrSeg;
    
    cxt->i2c_reg = *reg;
    
    //ads1013 init
    i2c_start(cxt->i2c_handle); //开始写
    if (i2c_send_byte_with_ack(cxt->i2c_handle, ADS1013_ADDR)) //写器件地址（写）
        goto Ads1013ErrSeg;

    if (i2c_send_byte_with_ack(cxt->i2c_handle, ADS1013_CONFIG_ADDR)) 
        goto Ads1013ErrSeg;

    if (i2c_send_byte_with_ack(cxt->i2c_handle, config >> 8)) //04
        goto Ads1013ErrSeg;

    if (i2c_send_byte_with_ack(cxt->i2c_handle, (INT8U)config)) //E3
        goto Ads1013ErrSeg;
    i2c_stop(cxt->i2c_handle);

    i2c_start(cxt->i2c_handle);
    if (i2c_send_byte_with_ack(cxt->i2c_handle, ADS1013_ADDR)) //写器件地址（写）
        goto Ads1013ErrSeg;

    if (i2c_send_byte_with_ack(cxt->i2c_handle, ADS1013_CONVERSION_ADDR)) 
        goto Ads1013ErrSeg;

    i2c_stop(cxt->i2c_handle); //结束写
    
    return cxt->id;
    
Ads1013ErrSeg:
    return 0xFF;
}

ads1013_cxt_t ads1013_get_free_cxt(void)
{
    INT8U i = 0;
    
    for(; i < ADS1013_SUPPORT_DEV_MAX; i++)
    {
        if(g_ads1013CxtBuff[i].id == 0) return &g_ads1013CxtBuff[i];
    }
    
    return NULL;
}

/*
 * @brief        从ADS1013中读出数据
 * @note         无
 * @param[in]    无
 * @param[out]   读取的AD转换结果
 * @return       无
 *
 */
INT16U ads1013_read(ADS1013_HANDLE handle)
{
    ads1013_cxt_t cxt = NULL;
    INT16U adc_hi=0;
    INT8U adc_lo=0;
    INT8U cnt = 0;
    
    if(handle == 0 || handle > ADS1013_SUPPORT_DEV_MAX) return 0;
    cxt = &g_ads1013CxtBuff[handle - 1];

Ads1013RestartSeg: 
    i2c_start(cxt->i2c_handle); //开始读
    if (i2c_send_byte_with_ack(cxt->i2c_handle, ADS1013_ADDR + 1)) //写器件地址（读）

    {
        if (++cnt < 10) goto Ads1013RestartSeg;

        return 0;

    }

    adc_hi = i2c_receive_byte(cxt->i2c_handle); // 读高8位
    i2c_send_ack(cxt->i2c_handle); //连续读
    adc_lo = i2c_receive_byte(cxt->i2c_handle); //读低8位
    i2c_send_ack(cxt->i2c_handle); //连续读

    i2c_stop(cxt->i2c_handle); //结束读
    
    adc_hi = ((adc_hi << 8) + adc_lo) >> 4;

    return adc_hi;
}

#else


ADS1013_HANDLE ads1013_init(I2C_REG* reg, INT16U config)
{
    OS_CPU_SR cpu_sr = 0;
    
    if(g_ads1013InitFlag) return g_ads1013CxtCnt;
    //iic init
    iic0_init();
    
    //ads1013 init
    OS_ENTER_CRITICAL();
    if(iic0_write(ADS1013_ADDR, ADS1013_CONFIG_ADDR, (uchar*)(&config), 2) != I2C_SUCCESS) 
    {
        OS_EXIT_CRITICAL();
        return 0xFF;
    }
    OS_EXIT_CRITICAL();
    
    g_ads1013InitFlag = 1;
    
    return g_ads1013CxtCnt;
}

ads1013_cxt_t ads1013_get_free_cxt(void)
{
    return &g_ads1013CxtBuff[0];
}

INT16U ads1013_read(ADS1013_HANDLE handle)
{
    OS_CPU_SR cpu_sr = 0;
    INT8U adc_data[2]={0};
    INT8U cnt = 0;
    
Ads1013RestartSeg: 
    OS_ENTER_CRITICAL();    
    if(iic0_read(ADS1013_ADDR, ADS1013_CONVERSION_ADDR, adc_data, 2) != I2C_SUCCESS)
    {
        OS_EXIT_CRITICAL();
        if (++cnt < 3) 
        {
            goto Ads1013RestartSeg;
        }
        return 0;
    }
    OS_EXIT_CRITICAL();
    
    return (((INT16U)adc_data[0] << 8) + adc_data[1]) >> 4;
}

INT8U ads1013_config(ADS1013_HANDLE handle, INT16U config)
{
    OS_CPU_SR cpu_sr = 0;
    
    OS_ENTER_CRITICAL();
    if(iic0_write(ADS1013_ADDR, ADS1013_CONFIG_ADDR, (uchar*)(&config), 2) != I2C_SUCCESS) 
    {
        OS_EXIT_CRITICAL();
        return 0;
    }
    OS_EXIT_CRITICAL();
    
    return 1;
}

#endif

INT8U ads1013_require(void)
{
    OS_CPU_SR cpu_sr = 0;
    
    OS_ENTER_CRITICAL();
    if(g_ads1013UsingFlag == 0) 
    {
        g_ads1013UsingFlag = 1;
        OS_EXIT_CRITICAL();
        return 1;
    }
    OS_EXIT_CRITICAL();
    
    return 0;
}

void ads1013_release(void)
{
    OS_CPU_SR cpu_sr = 0;
    
    OS_ENTER_CRITICAL();
    g_ads1013UsingFlag = 0;
    OS_EXIT_CRITICAL();
}

INT8U ads1013_value_is_negative(INT16U value)
{
    if(value > ADS1013_AD_MAX) return TRUE;
    
    return FALSE;
}