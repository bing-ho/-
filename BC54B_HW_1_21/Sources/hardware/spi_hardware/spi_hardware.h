/*******************************************************************************
**                       安徽力高新能源有限公司 Copyright (c)
**                            http://www.ligoo.cn/ 
** @file	    spi_hardware.h                                       

** @brief       1.完成SPI模块的数据类型的定义 
                2.完成SPI模块的初始化
                3.完成SPI模块的各种采集模式的功能函数
** @copyright		Ligoo Inc.         
** @date		    2017-03-30.
** @author      zhangtian      
*******************************************************************************/ 
#ifndef __SPI_HARDWARE_H__
#define __SPI_HARDWARE_H__

#include "stdint.h"
#include "gpio_interface.h"
#include "derivative.h"
//#include "spi_interface.h"


/************************************************
  * @struct   hcs12_spi_ops
  * @brief   SPI的操作接口函数变量
 ***********************************************/
extern struct spi_operations hcs12_spi_ops;


/************************************************
  * @struct   hcs12_spi_t
  * @brief   spi模块参数的指针变量
 ***********************************************/
typedef const struct hcs12_spi_platform *__FAR hcs12_spi_t;


/************************************************
  * @struct   hcs12_spi_regs
  * @brief   spi模块寄存器
 ***********************************************/
struct hcs12_spi_regs 
{
    volatile SPI0CR1STR cr1;
    volatile SPI0CR2STR cr2;
    volatile SPI0BRSTR br;
    volatile SPI0SRSTR sr;
    volatile SPI0DRSTR dr;
};



/************************************************
  * @struct   hcs12_spi_platform
  * @brief   spi模块参数
 ***********************************************/
struct hcs12_spi_platform 
{
    struct hcs12_spi_regs *regs; //Spi寄存器
    uint8_t baud_reg_value;      //波特率寄存器值
    uint8_t cs_num;              //cs
    const struct hcs12_gpio *__FAR cs_pins;//cs引脚
};




#define SPI_FLAG_CLK_IDLE_HIGH     0x01 //SPI 时钟低有效(闲置高态)
#define SPI_FLAG_CLK_IDLE_LOW      0x00 //SPI 时钟高有效(闲置低态)
#define SPI_FLAG_CLK_FIRST_EDGE    0x02 //
#define SPI_FLAG_CLK_SECOND_EDGE   0x00 //
#define SPI_FLAG_LSB_FIRST         0x04 //LSB 先移位(串行数据传输始于最低位)
#define SPI_FLAG_MSB_FIRST         0x00 //MSB 先移位(串行数据传输始于最高位)



/************************************************************************
  * @brief           spi模块硬件初始化函数 
  * @param[in]       platform  spi配置参数 flags 标志
  * @return          0
***********************************************************************/
int hcs12_spi_init(hcs12_spi_t spi, uint8_t flags);

/************************************************************************
  * @brief           spi模块硬件未初始化函数 
  * @param[in]       platform  spi配置参数
  * @return          0
***********************************************************************/
void hcs12_spi_deinit(const struct hcs12_spi_platform *__FAR platform);


/************************************************************************
  * @brief           spi模块硬件选择 
  * @param[in]       platform  spi配置参数  which 引脚  is_select 是否选择
  * @return          0
***********************************************************************/
int hcs12_spi_select(const struct hcs12_spi_platform *__FAR platform, uint8_t which, uint8_t is_select);


/************************************************************************
  * @brief           spi传输一个字节
  * @param[in]       platform  spi配置参数  dat 写入数据
  * @return          0
***********************************************************************/
int hcs12_spi_transmit_byte(hcs12_spi_t spi, uint8_t *dat);


/************************************************************************
  * @brief           spi写入和读一个数组
  * @param[in]       platform  spi配置参数  read-读 write-写 len-长度
  * @return          0
***********************************************************************/
uint16_t hcs12_spi_transfer(const struct hcs12_spi_platform *__FAR platform, uint8_t *__FAR read, const uint8_t *__FAR write, uint16_t len);


/************************************************************************
  * @brief           spi写入和读一个数组
  * @param[in]       platform  spi配置参数  tx_data tx_lenr x_data rx_len
  * @return          0
***********************************************************************/
int hcs12_spi_write_read_array(const struct hcs12_spi_platform *far platform,uint8_t *far tx_data, uint16_t tx_len, uint8_t *far rx_data, uint16_t rx_len);


/************************************************************************
  * @brief           spi写入一个数组
  * @param[in]       platform  spi配置参数  len 数据长度 dat 写入数据
  * @return          0
***********************************************************************/
int hcs12_spi_write_array(const struct hcs12_spi_platform *far platform,uint16_t len,uint8_t *far data);


/************************************************************************
  * @brief           spi模块硬件重新初始化函数 
  * @param[in]       platform  spi配置参数  flags 标志
  * @return          0
***********************************************************************/
int hcs12_spi_reinit(const struct hcs12_spi_platform *__FAR platform, uint8_t flags) ;

#endif