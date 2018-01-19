/*******************************************************************************
**                       安徽力高新能源有限公司 Copyright (c)
**                            http://www.ligoo.cn/ 
** @file	    iic.c                                       

** @brief       1.完成IIC模块的数据类型的定义 
                2.完成IIC模块的初始化
                3.完成IIC模块的数据读写
** @copyright		Ligoo Inc.         
** @date		    2017-03-28.
** @author           
*******************************************************************************/ 

#include "iic_interface.h"
#include "iic_hardware.h"

#pragma push
#pragma DATA_SEG __RPAGE_SEG PAGED_RAM

const struct i2c_bus hcs12_i2c_bus_0 = 
{
  &hcs12_impl_i2c0,
  &hcs12_i2c_ops,  
};

const struct i2c_bus hcs12_i2c_bus_1 = 
{
  &hcs12_impl_i2c1,
  &hcs12_i2c_ops,     
};
#pragma DATA_SEG DEFAULT
#pragma pop



/************************************************************************
  * @brief           iic总线初始化
  * @param[in]       i2c_bus bus总线号  
  * @return          无  
***********************************************************************/
uint8_t bus_i2c_init(const struct i2c_bus *__FAR bus) 
{
    void *__FAR private_data = bus->private_data;
    
    return bus->ops->init(private_data);
}


/************************************************************************
  * @brief           iic读写数据
  * @param[in]       i2c_bus bus总线号
  * @param[in]       addr 子器件地址号
  * @param[in]       w 需要发送的数据指针
  * @param[in]       wlen 发送数据的长度  
  * @param[in]       r 返回读取数据的地址指针
  * @param[in]       rlen 读取数据的长度     
  * @return          len：收发成功的数据长度  
***********************************************************************/
uint8_t bus_i2c_transmit(const struct i2c_bus *__FAR bus,
                            uint8_t addr,
                            const uint8_t *__FAR w,
                            uint8_t wlen,
                            uint8_t *__FAR r,
                            uint8_t rlen) {
    return bus->ops->transmit(bus->private_data, addr, w, wlen, r, rlen);
}



#pragma push
#pragma DATA_SEG __RPAGE_SEG PAGED_RAM
 // hcs12_i2c_ops初始化
struct i2c_operations hcs12_i2c_ops = 
{
    hcs12_i2c_init,
    hcs12_i2c_deinit,
    hcs12_i2c_write_then_read,
};
 #pragma DATA_SEG DEFAULT
#pragma pop