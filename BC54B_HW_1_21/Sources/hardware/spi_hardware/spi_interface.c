/*******************************************************************************
**                       安徽力高新能源有限公司 Copyright (c)
**                            http://www.ligoo.cn/ 
** @file	    spi_interface.c                                       

** @brief       1.完成SPI模块的数据类型的定义 
                2.完成SPI模块的初始化
                3.完成SPI模块的各种采集模式的功能函数
** @copyright		Ligoo Inc.         
** @date		    2017-03-30.
** @author      zhangtian      
*******************************************************************************/ 
#include "spi_interface.h"


/************************************************************************
  * @brief           spi初始化函数 
  * @param[in]       bus spi总线 flags 标志
  * @return          0
***********************************************************************/
inline int spi_init(spi_bus_t bus, uint8_t flags) 
{
    return bus->ops->init(bus->private_data, flags);
}


/************************************************************************
  * @brief           spi释放函数 
  * @param[in]       bus spi总线 
  * @return          0
***********************************************************************/
inline void spi_deinit(spi_bus_t bus) 
{
    bus->ops->deinit(bus->private_data);
}



/************************************************************************
  * @brief           spi模块选择 
  * @param[in]       bus spi总线  which 引脚  is_select 是否选择
  * @return          0
***********************************************************************/
inline int spi_select(spi_bus_t bus, uint8_t which, uint8_t is_select) 
{
    return bus->ops->select(bus->private_data, which, is_select);
}


/************************************************************************
  * @brief           spi传输一个字节
  * @param[in]       bus spi总线  which 引脚  is_select 是否选择
  * @return          0
***********************************************************************/
inline int spi_transmit_byte(spi_bus_t bus, uint8_t *b) 
{
    return bus->ops->transmit_byte(bus->private_data, b);
}



/************************************************************************
  * @brief           spi传输数据
  * @param[in]       bus spi总线  read 读  write 写  len  长度
  * @return          0
***********************************************************************/
inline uint16_t spi_transfer(spi_bus_t bus, uint8_t *__FAR read, const uint8_t *__FAR write, uint16_t len) 
{
    return bus->ops->transfer(bus->private_data, read, write, len);
}





  // liqing 20170511
/***********************************************************************
  * @brief           spi转换一个字节
  * @param[in]       bus spi总线 flags 标志
  * @return          0
***********************************************************************/
inline uint16_t spi_transfer_use_transmit_onebyte(spi_bus_t bus, const struct hcs12_spi_platform *__FAR private_data,  uint8_t *__FAR read, const uint8_t *__FAR write, uint16_t len) 
{
  //  if (read && write) return send_and_recv(private_data, ops, read, write, len);
  //  if (read) return recv(private_data, ops, read, len);
  //  if (write) return send(private_data, ops, write, len);
  //  return 0;
  
  // liqing 20170511
  
  return  bus->ops->transfer(private_data, read, write, len);
}



/***********************************************************************
  * @brief           spi重新初始化
  * @param[in]       bus spi总线 flags 标志
  * @return          0
***********************************************************************/
inline int spi_reinit(spi_bus_t bus, uint8_t flags) 
{
    return bus->ops->reinitialize(bus->private_data, flags);
}





/***********************************************************************
  * @brief           spi写一个数组
  * @param[in]       bus spi总线 len 发送长度  data 写入数据
  * @return          0
***********************************************************************/
inline int spi_write_array(spi_bus_t bus,uint16_t len,uint8_t *far data)
{
    return bus->ops->write_array(bus->private_data,len,data);
}




/***********************************************************************
  * @brief           spi读或写一个数组
  * @param[in]       bus spi总线  tx_data 发送数据  tx_len 发送长度  rx_data, rx_len
  * @return          0
***********************************************************************/
inline int spi_write_read_array(spi_bus_t bus,uint8_t *far tx_data, uint16_t tx_len, uint8_t *far rx_data, uint16_t rx_len)
{
    return bus->ops->write_read_array(bus->private_data,tx_data,tx_len,rx_data,rx_len);
}


/************************************************************************
  * @brief           spi操作函数
  * @param[in]       
  * @return          0
***********************************************************************/
struct spi_operations hcs12_spi_ops = 
{
    hcs12_spi_init,
    hcs12_spi_deinit,
    hcs12_spi_select,
    hcs12_spi_transmit_byte,
    hcs12_spi_transfer,
    hcs12_spi_reinit,
    hcs12_spi_write_array,
    hcs12_spi_write_read_array,
};

