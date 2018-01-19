/*******************************************************************************
**                       安徽力高新能源有限公司 Copyright (c)
**                            http://www.ligoo.cn/ 
** @file	    spi_hardware.c                                       

** @brief       1.完成SPI模块的数据类型的定义 
                2.完成SPI模块的初始化
                3.完成SPI模块的各种采集模式的功能函数
** @copyright		Ligoo Inc.    MODRR_MODRR4 = 0; //使用PS4~PS7 == SPI0   
** @date		    2017-03-30.   MODRR_MODRR5 = 1; //使用PH0~PH3 == SPI1 
** @author      zhangtian     MODRR_MODRR6 = 1; //使用PH4~PH7 == SPI2  
*******************************************************************************/ 
#include "spi_hardware.h"

#define SPI0ADDR    ((void *)0xD8)     //spi 寄存器首地址

#define SPI1ADDR    ((void *)0xF0)

#define SPI2ADDR    ((void *)0xF8)



/************************************************************************
  * @brief           spi模块硬件初始化函数 
  * @param[in]       platform  spi配置参数 flags 标志
  * @return          1:OK   0:err
***********************************************************************/
int hcs12_spi_init(const struct hcs12_spi_platform *__FAR platform, uint8_t flags) 
{
    uint8_t temp;
    

    if(platform->regs == SPI0ADDR) 
    {
      MODRR_MODRR4 = 0;      //20170508  liqing
       	SPI0SR = 0xB0 ; //清除所有标志   
    }
    else if(platform->regs == SPI1ADDR) 
    {
      MODRR_MODRR5 = 1; 
       	SPI1SR = 0xB0 ; //清除所有标志  
    }
    else if(platform->regs == SPI2ADDR) 
    {
      MODRR_MODRR6 = 1; 
       	SPI2SR = 0xB0 ; //清除所有标志   
    }
    else return 1;
    
    temp = (1 << 6) | (1 << 4); //使能,主机模式
    if (flags & SPI_FLAG_CLK_IDLE_HIGH) temp |= 1 << 3;
    if ((flags & SPI_FLAG_CLK_FIRST_EDGE) == 0) temp |= 1 << 2;
    if (flags & SPI_FLAG_LSB_FIRST) temp |= 1 << 0;
    platform->regs->cr1.Byte = temp;   //查询模式

    temp = (1 << 1); //8bit,停止等待模式
    platform->regs->cr2.Byte = temp;
    platform->regs->br.Byte = platform->baud_reg_value;

    for (temp = 0; temp < platform->cs_num; ++temp) 
    {
        (void)hcs12_gpio_init(&platform->cs_pins[temp], GPIO_MODE_OUTPUT_PUSHPULL);
        (void)hcs12_gpio_set_output(&platform->cs_pins[temp], 1);
    }
    return 0;
}





/************************************************************************
  * @brief           spi模块硬件未初始化函数 
  * @param[in]       platform  spi配置参数
  * @return          0
***********************************************************************/
void hcs12_spi_deinit(const struct hcs12_spi_platform *__FAR platform) 
{
    (void)platform;
}




/************************************************************************
  * @brief           spi模块硬件重新初始化函数 
  * @param[in]       platform  spi配置参数  flags 标志
  * @return          0
***********************************************************************/
int hcs12_spi_reinit(const struct hcs12_spi_platform *__FAR platform, uint8_t flags) 
{
    uint8_t temp;
    
    platform->regs->cr1.Byte = 0;
    platform->regs->cr2.Byte = 0;
    temp = (1 << 6) | (1 << 4); //使能，主机模式
    if (flags & SPI_FLAG_CLK_IDLE_HIGH) temp |= 1 << 3;
    if ((flags & SPI_FLAG_CLK_FIRST_EDGE) == 0) temp |= 1 << 2;
    if (flags & SPI_FLAG_LSB_FIRST) temp |= 1 << 0;
    platform->regs->cr1.Byte = temp;

    temp = (1 << 1); //8bit,停止等待模式
    platform->regs->cr2.Byte = temp;
    platform->regs->br.Byte = platform->baud_reg_value;

    platform->regs->sr.Byte = 0xB0;

    return 0;
} 




/************************************************************************
  * @brief           spi模块硬件选择 
  * @param[in]       platform  spi配置参数  which 引脚  is_select 是否选择
  * @return          0
***********************************************************************/
int hcs12_spi_select(const struct hcs12_spi_platform *__FAR platform, uint8_t which, uint8_t is_select) 
{
    if (which >= platform->cs_num) return -1;
    (void)hcs12_gpio_set_output(&platform->cs_pins[which], 0 == is_select);
    return 0;
}




/************************************************************************
  * @brief           spi传输一个字节
  * @param[in]       platform  spi配置参数  dat 写入数据
  * @return          0
***********************************************************************/
int hcs12_spi_transmit_byte(const struct hcs12_spi_platform *__FAR platform, uint8_t *dat) 
{
    uint16_t i;

    i = 0;
    for (;;) 
    {   //wait transmit emtpy
        if (platform->regs->sr.Bits.SPTEF) break;
        if (i > 10000) return -1;
        ++i;
    }

    platform->regs->dr.Overlap_STR.SPI0DRLSTR.Byte = *dat;

    i = 0;
    for (;;) 
    { 
        //wait data recved
        if (platform->regs->sr.Bits.SPIF) break;
        if (i > 10000) return -1;
        ++i;
    }

    *dat = platform->regs->dr.Overlap_STR.SPI0DRLSTR.Byte;
    return 0;
}




/************************************************************************
  * @brief           spi写入一个数组
  * @param[in]       platform  spi配置参数  len 数据长度 dat 写入数据
  * @return          0
***********************************************************************/
int hcs12_spi_write_array(const struct hcs12_spi_platform *far platform,uint16_t len,uint8_t *far data)
{
	uint16_t i = 0;
	uint16_t time_out = 0;
	volatile uint8_t rx_tmp = 0;
	if ((platform == (void *) 0)||(len == 0)||(data == (void *) 0))
		return -1;
	
	for (i = 0; i < len; i++)
	{
        time_out = 0;
        for (;;) 
        {   
            //wait transmit emtpy
            if (platform->regs->sr.Bits.SPTEF) break;
            if (time_out > 10000) return -1;
                ++time_out;
        }
        platform->regs->dr.Overlap_STR.SPI0DRLSTR.Byte = data[i];
        
        time_out = 0;
        for (;;) 
        {   
            //wait data recved
            if (platform->regs->sr.Bits.SPIF) break;
            if (time_out > 10000) return -1;
                ++time_out;
        }

        rx_tmp = platform->regs->dr.Overlap_STR.SPI0DRLSTR.Byte;
	}
    return 0;
}




/************************************************************************
  * @brief           spi写入和读一个数组
  * @param[in]       platform  spi配置参数  tx_data 发送数据 tx_len 发送长度 rx_data 接收数据 rx_len 接收数据
  * @return          0
***********************************************************************/
int hcs12_spi_write_read_array(const struct hcs12_spi_platform *far platform,uint8_t *far tx_data, uint16_t tx_len, uint8_t *far rx_data, uint16_t rx_len)
{
	uint16_t i = 0;
	uint16_t time_out = 0;
	volatile uint8_t rx_tmp = 0;
	if(platform == (void *) 0)
	       return -1;
	if ((tx_len == 0)||(tx_data == (void *) 0))
		return -1;
	if ((rx_len == 0)||(rx_data == (void *) 0))
		return -1;
	
	for (i = 0; i < tx_len; i++)
	{
        time_out = 0;
        for (;;) 
        {   
            //wait transmit emtpy
            if (platform->regs->sr.Bits.SPTEF) break;
            if (time_out > 10000) return -1;
                ++time_out;
        }
        platform->regs->dr.Overlap_STR.SPI0DRLSTR.Byte = tx_data[i];
        time_out = 0;
        for (;;) 
        {   
            //wait data recved
            if (platform->regs->sr.Bits.SPIF) break;
            if (time_out > 10000) return -1;
                ++time_out;
        }

        rx_tmp = platform->regs->dr.Overlap_STR.SPI0DRLSTR.Byte;
	}
    for (i = 0; i < rx_len; i++)
    {
        time_out = 0;
        for (;;) 
        {   
            //wait transmit emtpy
            if (platform->regs->sr.Bits.SPTEF) break;
            if (time_out > 10000) return -1;
                ++time_out;
        }
        platform->regs->dr.Overlap_STR.SPI0DRLSTR.Byte = 0xFF;
        
        
        time_out = 0;
        for (;;) 
        {   
            //wait data recved
            if (platform->regs->sr.Bits.SPIF) break;
            if (time_out > 10000) return -1;
                ++time_out;
        }
        rx_data[i] = platform->regs->dr.Overlap_STR.SPI0DRLSTR.Byte;
    }
    return 0;
}


/************************************************************************
  * @brief           spi发送数据
  * @param[in]       private_data spi私有数据  ops 操作函数  write 写  len  长度
  * @return          0
***********************************************************************/
static uint16_t send(const struct hcs12_spi_platform *__FAR private_data,  const uint8_t *__FAR write, uint16_t len) 
{
    uint8_t temp;
    uint16_t i;

    //spi_transmit_byte_func transmit_byte = ops->transmit_byte;

    for (i = 0; i < len; ++i) 
    {
        temp = *write++;
        if (hcs12_spi_transmit_byte(private_data, &temp)) 
        {
            break;
        }
    }
    return i;
}




/************************************************************************
  * @brief           spi接收数据
  * @param[in]       private_data spi私有数据  ops 操作函数  read 读  len  长度
  * @return          0
***********************************************************************/
static uint16_t recv(const struct hcs12_spi_platform *__FAR private_data, uint8_t *__FAR read, uint16_t len) 
{
    uint8_t temp;
    uint16_t i;

   // spi_transmit_byte_func transmit_byte = ops->transmit_byte;

    for (i = 0; i < len; ++i) 
    {
        if (hcs12_spi_transmit_byte(private_data, &temp)) 
        {
            break;
        }
        *read++ = temp;
    }
    return i;
}




/************************************************************************
  * @brief           spi发送和接收
  * @param[in]       private_data spi私有数据  ops 操作函数  read 读  write 写  len  长度
  * @return          0
***********************************************************************/
static uint16_t send_and_recv(const struct hcs12_spi_platform *__FAR private_data, uint8_t *__FAR read, const uint8_t *__FAR write, uint16_t len) 
{
    uint8_t temp;
    uint16_t i;

   // spi_transmit_byte_func transmit_byte = ops->transmit_byte;

    for (i = 0; i < len; ++i) 
    {
        temp = *write++;
        if (hcs12_spi_transmit_byte(private_data, &temp)) 
        {
            break;
        }
        *read++ = temp;
    }
    return i;
}


/************************************************************************
  * @brief           spi写入和读一个数组
  * @param[in]       platform  spi配置参数  read-读 write-写 len-长度
  * @return          0
***********************************************************************/

//  liqing  20170511
uint16_t hcs12_spi_transfer(const struct hcs12_spi_platform *__FAR platform, uint8_t *__FAR read, const uint8_t *__FAR write, uint16_t len) 
{
  //  return spi_transfer_use_transmit_onebyte((void *__FAR )platform, &hcs12_spi_ops, read, write, len);

    if (read && write) return send_and_recv(platform, read, write, len);
    if (read) return recv(platform, read, len);
    if (write) return send(platform, write, len);
    return 0;

}





