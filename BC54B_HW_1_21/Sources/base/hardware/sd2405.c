/**
 *
 * Copyright (c) 2011 Ligoo Inc.
 *
 * @file  IIC.c
 * @brief SD2405��IIC��д����
 * @note ���� http://www.whwave.com.cn/download/sd2400/sd2400_avr.c
 * @version 1.00
 * @author
 * @date 2011/11/30
 *
 */
#include "includes.h"
#include "sd2405.h"

#pragma MESSAGE DISABLE C1420 // Result of function-call is ignored
#pragma MESSAGE DISABLE C12056 // SP debug info incorrect because of optimization or inline assembler

#if BMS_SUPPORT_CLOCK == BMS_CLOCK_I2C


static I2C_HANDLE   g_i2cTime_handle = 0;

void i2c_time_delay_us(INT16U us);
void i2c_time_init(void)
{
    I2C_REG  g_i2cTimeReg = {0};
    
    g_i2cTimeReg.scl_pos = 7;
    g_i2cTimeReg.sda_pos = 6;
    g_i2cTimeReg.scl_dat.data = (INT32U*)0x00000268UL;
    g_i2cTimeReg.sda_dat.data = (INT32U*)0x00000268UL;
    g_i2cTimeReg.scl_dir.data = (INT32U*)0x0000026AUL;
    g_i2cTimeReg.sda_dir.data = (INT32U*)0x0000026AUL;
    
    g_i2cTime_handle = i2c_init(&g_i2cTimeReg);
    
    i2c_time_write_enable();
}

/*
 * @brief        д������ַ�������ֵ�ADS1110
 * @note         ��
 * @param[in]    ��
 * @param[out]   ��
 * @return       ��
 *
 */
INT8U i2c_time_write(INT8U addr, INT8U data)
{
    i2c_start(g_i2cTime_handle);

    if (i2c_send_byte_with_ack(g_i2cTime_handle, CLOCK_ADDRESS)) return 1; //д������ַ��д��

    if (i2c_send_byte_with_ack(g_i2cTime_handle, addr)) return 1;

    if (i2c_send_byte_with_ack(g_i2cTime_handle, data)) return 1;

    i2c_stop(g_i2cTime_handle); //����д

    return 0;
}

INT8U i2c_time_write_item_start(void)
{
    i2c_start(g_i2cTime_handle); //��ʼд

    if (i2c_send_byte_with_ack(g_i2cTime_handle, CLOCK_ADDRESS)) return 1; //д������ַ��д��

    return 0;
}

INT8U i2c_time_write_item_stop(void)
{
    i2c_stop(g_i2cTime_handle); //����д

    return 0;
}

INT8U i2c_time_write_item(INT8U data)
{
    if (i2c_send_byte_with_ack(g_i2cTime_handle, data)) return 1;

    return 0;
}

INT8U i2c_time_read(INT8U *time, INT8U size)
{
    if (time == NULL || size > CLOCK_BIT_LEN) return 1;

    i2c_start(g_i2cTime_handle);
    i2c_send_byte_with_ack(g_i2cTime_handle, CLOCK_ADDRESS + 1);

    time[0] = i2c_receive_byte(g_i2cTime_handle);
    i2c_send_ack(g_i2cTime_handle);

    time[1] = i2c_receive_byte(g_i2cTime_handle);
    i2c_send_ack(g_i2cTime_handle);

    time[2] = i2c_receive_byte(g_i2cTime_handle);
    i2c_send_ack(g_i2cTime_handle);

    time[3] = i2c_receive_byte(g_i2cTime_handle);
    i2c_send_ack(g_i2cTime_handle);

    time[4] = i2c_receive_byte(g_i2cTime_handle);
    i2c_send_ack(g_i2cTime_handle);

    time[5] = i2c_receive_byte(g_i2cTime_handle);
    i2c_send_ack(g_i2cTime_handle);

    time[6] = i2c_receive_byte(g_i2cTime_handle);
    i2c_send_ack(g_i2cTime_handle);

    i2c_stop(g_i2cTime_handle); //����д

    return 0;

}

INT8U i2c_time_write_enable(void)
{
    // ��WRTC1=1
    if (i2c_time_write(0x10, 0x80)) return 1;

    // ��WRTC2,WRTC3=1
    if (i2c_time_write(0x0F, 0x84)) return 1;

    return 0;
}

INT8U i2c_time_write_disable(void)
{
    i2c_start(g_i2cTime_handle);

    if (i2c_send_byte_with_ack(g_i2cTime_handle, CLOCK_ADDRESS)) return 1; //д������ַ��д��

    // ����д��ʼ��ַ0FH
    if (i2c_send_byte_with_ack(g_i2cTime_handle, 0x0F)) return 1;

    //��WRTC2,WRTC3=0
    if (i2c_send_byte_with_ack(g_i2cTime_handle, 0x00)) return 1;

    //��WRTC1=0(10H��ַ)
    if (i2c_send_byte_with_ack(g_i2cTime_handle, 0x00)) return 1;

    i2c_stop(g_i2cTime_handle); //����д

    return 0;
}

#else


INT8U iic_time_write_enable(void);

void iic_time_init(void)
{
    iic_init();
    
    iic_time_write_enable();
}

/*
 * @brief        д������ַ��������
 * @note         ��
 * @param[in]    ��
 * @param[out]   ��
 * @return       ��
 *
 */
INT8U iic_time_write(INT8U addr, INT8U data)
{
    uchar res;
    
    iic_start();

    res = iic_write_byte(CLOCK_ADDRESS, addr, data);
    
    iic_stop(); //����д

    if(res == I2C_SUCCESS) return 0;
    
    return 1;
}

INT8U iic_time_write_item_start(void)
{
    iic_start(); //��ʼд

    iic_send_data(CLOCK_ADDRESS); //���ʹӻ�д��ַ
    if (iic_wait_ack()) return 1;
    
    return 0;
}

INT8U iic_time_write_item_stop(void)
{
    iic_stop(); //����д

    return 0;
}

INT8U iic_time_write_item(INT8U data)
{
    iic_send_data(data); //���ʹӻ�д��ַ
    if (iic_wait_ack()) return 1;
    
    return 0;
}

INT8U iic_time_read(INT8U *time, INT8U size)
{
    if (time == NULL || size > CLOCK_BIT_LEN) return 1;

    if(iic_read(CLOCK_ADDRESS, CLOCK_BIT_START, time, size)) return 1;
    
    return 0;

}

INT8U iic_time_write_enable(void)
{
    // ��WRTC1=1
    if (iic_time_write(0x10, 0x80)) return 1;

    // ��WRTC2,WRTC3=1
    if (iic_time_write(0x0F, 0x84)) return 1;

    return 0;
}

INT8U iic_time_write_disable(void)
{
    iic_start();

    if (iic_time_write_item_start()) return 1; //д������ַ��д��

    // ����д��ʼ��ַ0FH
    if (iic_time_write_item(0x0F)) return 1;

    //��WRTC2,WRTC3=0
    if (iic_time_write_item(0x00)) return 1;

    //��WRTC1=0(10H��ַ)
    if (iic_time_write_item(0x00)) return 1;

    iic_time_write_item_stop(); //����д

    return 0;
}


#endif

