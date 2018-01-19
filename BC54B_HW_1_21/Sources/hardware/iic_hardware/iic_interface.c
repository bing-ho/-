/*******************************************************************************
**                       ������������Դ���޹�˾ Copyright (c)
**                            http://www.ligoo.cn/ 
** @file	 �b   iic.c                                       

** @brief       1.���IICģ����������͵Ķ��� 
                2.���IICģ��ĳ�ʼ��
                3.���IICģ������ݶ�д
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
  * @brief           iic���߳�ʼ��
  * @param[in]       i2c_bus bus���ߺ�  
  * @return          ��  
***********************************************************************/
uint8_t bus_i2c_init(const struct i2c_bus *__FAR bus) 
{
    void *__FAR private_data = bus->private_data;
    
    return bus->ops->init(private_data);
}


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
 // hcs12_i2c_ops��ʼ��
struct i2c_operations hcs12_i2c_ops = 
{
    hcs12_i2c_init,
    hcs12_i2c_deinit,
    hcs12_i2c_write_then_read,
};
 #pragma DATA_SEG DEFAULT
#pragma pop