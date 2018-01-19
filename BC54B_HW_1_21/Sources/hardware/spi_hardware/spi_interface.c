/*******************************************************************************
**                       ������������Դ���޹�˾ Copyright (c)
**                            http://www.ligoo.cn/ 
** @file	 �b   spi_interface.c                                       

** @brief       1.���SPIģ����������͵Ķ��� 
                2.���SPIģ��ĳ�ʼ��
                3.���SPIģ��ĸ��ֲɼ�ģʽ�Ĺ��ܺ���
** @copyright		Ligoo Inc.         
** @date		    2017-03-30.
** @author      zhangtian      
*******************************************************************************/ 
#include "spi_interface.h"


/************************************************************************
  * @brief           spi��ʼ������ 
  * @param[in]       bus spi���� flags ��־
  * @return          0
***********************************************************************/
inline int spi_init(spi_bus_t bus, uint8_t flags) 
{
    return bus->ops->init(bus->private_data, flags);
}


/************************************************************************
  * @brief           spi�ͷź��� 
  * @param[in]       bus spi���� 
  * @return          0
***********************************************************************/
inline void spi_deinit(spi_bus_t bus) 
{
    bus->ops->deinit(bus->private_data);
}



/************************************************************************
  * @brief           spiģ��ѡ�� 
  * @param[in]       bus spi����  which ����  is_select �Ƿ�ѡ��
  * @return          0
***********************************************************************/
inline int spi_select(spi_bus_t bus, uint8_t which, uint8_t is_select) 
{
    return bus->ops->select(bus->private_data, which, is_select);
}


/************************************************************************
  * @brief           spi����һ���ֽ�
  * @param[in]       bus spi����  which ����  is_select �Ƿ�ѡ��
  * @return          0
***********************************************************************/
inline int spi_transmit_byte(spi_bus_t bus, uint8_t *b) 
{
    return bus->ops->transmit_byte(bus->private_data, b);
}



/************************************************************************
  * @brief           spi��������
  * @param[in]       bus spi����  read ��  write д  len  ����
  * @return          0
***********************************************************************/
inline uint16_t spi_transfer(spi_bus_t bus, uint8_t *__FAR read, const uint8_t *__FAR write, uint16_t len) 
{
    return bus->ops->transfer(bus->private_data, read, write, len);
}





  // liqing 20170511
/***********************************************************************
  * @brief           spiת��һ���ֽ�
  * @param[in]       bus spi���� flags ��־
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
  * @brief           spi���³�ʼ��
  * @param[in]       bus spi���� flags ��־
  * @return          0
***********************************************************************/
inline int spi_reinit(spi_bus_t bus, uint8_t flags) 
{
    return bus->ops->reinitialize(bus->private_data, flags);
}





/***********************************************************************
  * @brief           spiдһ������
  * @param[in]       bus spi���� len ���ͳ���  data д������
  * @return          0
***********************************************************************/
inline int spi_write_array(spi_bus_t bus,uint16_t len,uint8_t *far data)
{
    return bus->ops->write_array(bus->private_data,len,data);
}




/***********************************************************************
  * @brief           spi����дһ������
  * @param[in]       bus spi����  tx_data ��������  tx_len ���ͳ���  rx_data, rx_len
  * @return          0
***********************************************************************/
inline int spi_write_read_array(spi_bus_t bus,uint8_t *far tx_data, uint16_t tx_len, uint8_t *far rx_data, uint16_t rx_len)
{
    return bus->ops->write_read_array(bus->private_data,tx_data,tx_len,rx_data,rx_len);
}


/************************************************************************
  * @brief           spi��������
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

