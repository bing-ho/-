/*******************************************************************************
**                       ������������Դ���޹�˾ Copyright (c)
**                            http://www.ligoo.cn/ 
** @file	 �b   iic_interface.h                                       

** @brief       1.���IICģ����������͵Ķ��� 
                2.���IICģ��ĳ�ʼ��
                3.���IICģ������ݶ�д
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
  * @brief     ����һ������ָ������
 ***********************************************/
typedef uint8_t (*i2c_init_func)(void *__FAR private_data);

 /************************************************
  * @typedef   i2c_transmit_func
  * @brief     ����һ������ָ������
 ***********************************************/
typedef uint8_t (* i2c_transmit_func)(void *__FAR private_data,
                                     uint8_t addr,
                                     const uint8_t *__FAR w,
                                     uint8_t wlen,
                                     uint8_t *__FAR r,
                                     uint8_t rlen);
                                     
/************************************************
  * @typedef   i2c_deinit_func
  * @brief     ����һ������ָ������
 ***********************************************/                                     
typedef void (*i2c_deinit_func)(void *__FAR private_data);                                      

/************************************************
  * @struct   i2c_operations
  * @brief    iic���������ṹ��
 ***********************************************/
struct i2c_operations {
    i2c_init_func init;
    i2c_deinit_func deinit;
    i2c_transmit_func transmit;
};

/************************************************
  * @struct   i2c_bus
  * @brief    iic����Ӳ����������
 ***********************************************/
struct i2c_bus {
    void *__FAR private_data;
    const struct i2c_operations *__FAR ops;
};


/************************************************
  * @typedef   i2c_bus_t
  * @brief     ����һ��i2c_bus����
 ***********************************************/
typedef const struct i2c_bus *__FAR i2c_bus_t;






#pragma push
#pragma DATA_SEG __RPAGE_SEG PAGED_RAM
extern struct i2c_operations hcs12_i2c_ops; 
extern const struct i2c_bus hcs12_i2c_bus_0;
extern const struct i2c_bus hcs12_i2c_bus_1;

#pragma pop


/************************************************************************
  * @brief           iic���߳�ʼ��
  * @param[in]       i2c_bus bus���ߺ�  
  * @return          ��  
***********************************************************************/
uint8_t bus_i2c_init(i2c_bus_t bus);

/************************************************************************
  * @brief           iic��д����
  * @param[in]       i2c_bus bus���ߺ�
  * @param[in]       addr ��������ַ��
  * @param[in]       w ��Ҫ���͵�����ָ��
  * @param[in]       wlen �������ݵĳ���  
  * @param[in]       r ���ض�ȡ���ݵĵ�ַָ��
  * @param[in]       rlen ��ȡ���ݵĳ���     
  * @return          len���շ��ɹ������ݳ���  
***********************************************************************/
uint8_t bus_i2c_transmit(i2c_bus_t bus,
                            uint8_t addr,
                            const uint8_t *__FAR w,
                            uint8_t wlen,
                            uint8_t *__FAR r,
                            uint8_t rlen);

#endif