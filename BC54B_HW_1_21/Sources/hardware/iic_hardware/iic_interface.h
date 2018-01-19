/*******************************************************************************
**                       安徽力高新能源有限公司 Copyright (c)
**                            http://www.ligoo.cn/ 
** @file	    iic_interface.h                                       

** @brief       1.完成IIC模块的数据类型的定义 
                2.完成IIC模块的初始化
                3.完成IIC模块的数据读写
** @copyright		Ligoo Inc.         
** @date		    2017-03-28.
** @author           
*******************************************************************************/ 
#ifndef __IIC_INTERFACE_H__
#define __IIC_INTERFACE_H__

#include "stdint.h"
//#include "iic_hardware.h"

/************************************************
  * @typedef   i2c_init_func
  * @brief     定义一个函数指针类型
 ***********************************************/
typedef uint8_t (*i2c_init_func)(void *__FAR private_data);

 /************************************************
  * @typedef   i2c_transmit_func
  * @brief     定义一个函数指针类型
 ***********************************************/
typedef uint8_t (* i2c_transmit_func)(void *__FAR private_data,
                                     uint8_t addr,
                                     const uint8_t *__FAR w,
                                     uint8_t wlen,
                                     uint8_t *__FAR r,
                                     uint8_t rlen);
                                     
/************************************************
  * @typedef   i2c_deinit_func
  * @brief     定义一个函数指针类型
 ***********************************************/                                     
typedef void (*i2c_deinit_func)(void *__FAR private_data);                                      

/************************************************
  * @struct   i2c_operations
  * @brief    iic操作函数结构体
 ***********************************************/
struct i2c_operations {
    i2c_init_func init;
    i2c_deinit_func deinit;
    i2c_transmit_func transmit;
};

/************************************************
  * @struct   i2c_bus
  * @brief    iic总线硬件操作属性
 ***********************************************/
struct i2c_bus {
    void *__FAR private_data;
    const struct i2c_operations *__FAR ops;
};


/************************************************
  * @typedef   i2c_bus_t
  * @brief     定义一个i2c_bus类型
 ***********************************************/
typedef const struct i2c_bus *__FAR i2c_bus_t;






#pragma push
#pragma DATA_SEG __RPAGE_SEG PAGED_RAM
extern struct i2c_operations hcs12_i2c_ops; 
extern const struct i2c_bus hcs12_i2c_bus_0;
extern const struct i2c_bus hcs12_i2c_bus_1;

#pragma pop


/************************************************************************
  * @brief           iic总线初始化
  * @param[in]       i2c_bus bus总线号  
  * @return          无  
***********************************************************************/
uint8_t bus_i2c_init(i2c_bus_t bus);

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
uint8_t bus_i2c_transmit(i2c_bus_t bus,
                            uint8_t addr,
                            const uint8_t *__FAR w,
                            uint8_t wlen,
                            uint8_t *__FAR r,
                            uint8_t rlen);

#endif