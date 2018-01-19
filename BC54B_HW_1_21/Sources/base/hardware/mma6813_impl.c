/**
*
* Copyright (c) 2015 Ligoo Inc.
*
* @file mma6813_impl.c
* @brief
* @note
* @author 
* @date 2015-8-18
*
*/
#include "mma6813_impl.h"

#pragma MESSAGE DISABLE C1420 // Result of function-call is ignored
void mma6813_register_write(INT8U reg_addr,INT8U reg_data);
INT8U data_odd_parity_bit(INT16U data);

void mma6813_init(void)
{
    SPI1_Init();
    
    mma6813_register_write(MMA6813_REG_DEVCTL_ADDR,0x00);  //reset
    mma6813_register_write(MMA6813_REG_DEVCTL_ADDR,0xc0);
    mma6813_register_write(MMA6813_REG_DEVCTL_ADDR,0x40);
    mma6813_register_write(MMA6813_REG_DEVCFG_ADDR,0x13);  //SD:1 OFMON BIT:0 A_CFG[2:0]:011
    
}

void mma6813_register_write(INT8U reg_addr,INT8U reg_data)
{
    INT16U write_data = 0;
    INT8U parity = 1;  //初始判断标记

    write_data = (2<<12)|(reg_addr<<7)|reg_data; //AX:1 A:0
    
    parity = data_odd_parity_bit(write_data);

    write_data = (parity<<14)|write_data;
    
    MMA6813_CS_HIGH();
    SPI1_SendByte((unsigned char)write_data);
    SPI1_SendByte((unsigned char)(write_data>>8));
    MMA6813_CS_LOW();
    
    MMA6813_CS_HIGH();
    write_data = SPI1_SendByte(0xff);
    write_data = write_data+(((INT16U)SPI1_SendByte(0xff))<<8);
    MMA6813_CS_LOW();
}

INT16U mma6813_register_read(INT8U reg_addr)
{
    INT16U write_data = 0;
    INT8U parity = 1;  //初始判断标记

    write_data = reg_addr<<7; //AX:0 A:0 D7~D0:0
    
    parity = data_odd_parity_bit(write_data);

    write_data = (parity<<14)|write_data;
    
    MMA6813_CS_HIGH();
    SPI1_SendByte((unsigned char)write_data);
    SPI1_SendByte((unsigned char)(write_data>>8));
    MMA6813_CS_LOW();
    
    MMA6813_CS_HIGH();
    write_data = SPI1_SendByte(0xff);
    write_data = write_data+(((INT16U)SPI1_SendByte(0xff))<<8);
    MMA6813_CS_LOW();
    
    return write_data;
}

INT16U mma6813_accel_data_request(INT8U axis)
{
    INT16U write_data = 0;
    INT8U parity = 1;  //初始判断标记

    if(axis == 0)
        write_data = 0x2006; //AX:0 A:1 OC:0 SD:1 ARM:1
    else
        write_data = 0x6007;
        
    MMA6813_CS_HIGH();
    SPI1_SendByte((unsigned char)write_data);
    SPI1_SendByte((unsigned char)(write_data>>8));
    MMA6813_CS_LOW();
    
    MMA6813_CS_HIGH();
    write_data = SPI1_SendByte(0xff);
    write_data = write_data+(((INT16U)SPI1_SendByte(0xff))<<8);
    MMA6813_CS_LOW();
    
    return write_data;
}


INT8U data_odd_parity_bit(INT16U data)
{
    INT8U parity = 1;  //初始判断标记
    while (data)       //奇校验位 
    {
        parity = !parity;
        data = data & (data - 1);
    }
    return parity;
}