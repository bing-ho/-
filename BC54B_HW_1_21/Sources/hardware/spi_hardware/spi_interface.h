/*******************************************************************************
**                       安徽力高新能源有限公司 Copyright (c)
**                            http://www.ligoo.cn/ 
** @file	    spi_interface.h                                        

** @brief       1.完成SPI模块的数据类型的定义 
                2.完成SPI模块的初始化
                3.完成SPI模块的各种采集模式的功能函数
** @copyright		Ligoo Inc.         
** @date		    2017-03-30.
** @author      zhangtian      
*******************************************************************************/ 
#ifndef __SPI_INTERFACE_H__
#define __SPI_INTERFACE_H__

#include "stdint.h"

#include "spi_hardware.h"

#ifndef __FAR
#define __FAR
#endif
                    
#define inline


typedef const struct spi_bus *__FAR spi_bus_t;  

//定义SPI初始化函数指针
typedef int (*__FAR spi_init_func)(void *__FAR private_data, uint8_t flags);


//定义SPI释放函数指针
typedef void (*__FAR spi_deinit_func)(void *__FAR private_data);


//定义SPI选择函数指针
typedef int (*__FAR spi_select_func)(void *__FAR private_data, uint8_t which, uint8_t is_select);


//定义SPI传输一个字节函数指针
typedef int (*__FAR spi_transmit_byte_func)(void *__FAR private_data, uint8_t *b);


//定义SPI传输函数指针
typedef uint16_t (*__FAR spi_transfer_func)(const struct hcs12_spi_platform *__FAR private_data, uint8_t *__FAR r, const uint8_t *__FAR w, uint16_t len);


//定义SPI重新初始化函数指针
typedef int (*__FAR spi_reinit_func)(const struct hcs12_spi_platform *__FAR platform, uint8_t flags) ;


//定义SPI写数组函数指针
typedef int (*__FAR spi_write_array_func)(const struct hcs12_spi_platform *far platform,uint16_t len,uint8_t *far data);


//定义SPI写和读数组函数指针
typedef int (*__FAR spi_write_read_array_func)(const struct hcs12_spi_platform *far platform,uint8_t *far tx_data, uint16_t tx_len, uint8_t *far rx_data, uint16_t rx_len);



/************************************************
  * @struct   spi_bus
  * @brief   SPI总线参数
 ***********************************************/
struct spi_bus 
{
    void *__FAR private_data;
    const struct spi_operations *__FAR ops;
};


/************************************************
  * @struct   spi_operations
  * @brief   SPI的操作接口函数
 ***********************************************/
struct spi_operations 
{
    spi_init_func init;     //spi初始化函数 
    spi_deinit_func deinit; //spi释放函数 
    spi_select_func select; //spi模块选择
    spi_transmit_byte_func transmit_byte;//spi传输一个字节
    spi_transfer_func transfer;//
    spi_reinit_func   reinitialize;//spi重新初始化
    spi_write_array_func write_array;//
    spi_write_read_array_func  write_read_array;//
};




/************************************************************************
  * @brief           spi初始化函数 
  * @param[in]       bus spi总线 flags 标志
  * @return          0
***********************************************************************/
inline int spi_init(spi_bus_t bus, uint8_t flags);




/************************************************************************
  * @brief           spi未初始化函数 
  * @param[in]       bus spi总线 
  * @return          0
***********************************************************************/
inline void spi_deinit(spi_bus_t bus);




/************************************************************************
  * @brief           spi模块选择 
  * @param[in]       bus spi总线  which 引脚  is_select 是否选择
  * @return          0
***********************************************************************/
inline int spi_select(spi_bus_t bus, uint8_t which, uint8_t is_select);




/************************************************************************
  * @brief           spi传输一个字节
  * @param[in]       bus spi总线  which 引脚  is_select 是否选择
  * @return          0
***********************************************************************/
inline int spi_transmit_byte(const struct spi_bus *__FAR bus, uint8_t *b);




/************************************************************************
  * @brief           spi传输
  * @param[in]       bus spi总线  read 读  write 写  len  长度
  * @return          0
***********************************************************************/
inline uint16_t spi_transfer(spi_bus_t bus, uint8_t *__FAR read, const uint8_t *__FAR write, uint16_t len);




/***********************************************************************
  * @brief           spi重新初始化
  * @param[in]       bus spi总线 flags 标志
  * @return          0
***********************************************************************/
inline int spi_reinit(spi_bus_t bus, uint8_t flags);




/***********************************************************************
  * @brief           spi写一个数组
  * @param[in]       bus spi总线 len 发送长度  data 写入数据
  * @return          0
***********************************************************************/
inline int spi_write_array(spi_bus_t bus,uint16_t len,uint8_t *far data);




/***********************************************************************
  * @brief           spi读或写一个数组
  * @param[in]       bus spi总线  tx_data 发送数据  tx_len 发送长度  rx_data, rx_len
  * @return          0
***********************************************************************/
inline int spi_write_read_array(spi_bus_t bus,uint8_t *far tx_data, uint16_t tx_len, uint8_t *far rx_data, uint16_t rx_len);




/***********************************************************************
  * @brief           spi转换一个字节
  * @param[in]       bus spi总线 flags 标志
  * @return          0
***********************************************************************/
inline uint16_t spi_transfer_use_transmit_onebyte(spi_bus_t bus, const struct hcs12_spi_platform *__FAR private_data,  uint8_t *__FAR read, const uint8_t *__FAR write, uint16_t len);


#endif

