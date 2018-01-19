/*******************************************************************************
**                       ������������Դ���޹�˾ Copyright (c)
**                            http://www.ligoo.cn/ 
** @file	 �b   spi_interface.h                                        

** @brief       1.���SPIģ����������͵Ķ��� 
                2.���SPIģ��ĳ�ʼ��
                3.���SPIģ��ĸ��ֲɼ�ģʽ�Ĺ��ܺ���
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

//����SPI��ʼ������ָ��
typedef int (*__FAR spi_init_func)(void *__FAR private_data, uint8_t flags);


//����SPI�ͷź���ָ��
typedef void (*__FAR spi_deinit_func)(void *__FAR private_data);


//����SPIѡ����ָ��
typedef int (*__FAR spi_select_func)(void *__FAR private_data, uint8_t which, uint8_t is_select);


//����SPI����һ���ֽں���ָ��
typedef int (*__FAR spi_transmit_byte_func)(void *__FAR private_data, uint8_t *b);


//����SPI���亯��ָ��
typedef uint16_t (*__FAR spi_transfer_func)(const struct hcs12_spi_platform *__FAR private_data, uint8_t *__FAR r, const uint8_t *__FAR w, uint16_t len);


//����SPI���³�ʼ������ָ��
typedef int (*__FAR spi_reinit_func)(const struct hcs12_spi_platform *__FAR platform, uint8_t flags) ;


//����SPIд���麯��ָ��
typedef int (*__FAR spi_write_array_func)(const struct hcs12_spi_platform *far platform,uint16_t len,uint8_t *far data);


//����SPIд�Ͷ����麯��ָ��
typedef int (*__FAR spi_write_read_array_func)(const struct hcs12_spi_platform *far platform,uint8_t *far tx_data, uint16_t tx_len, uint8_t *far rx_data, uint16_t rx_len);



/************************************************
  * @struct   spi_bus
  * @brief   SPI���߲���
 ***********************************************/
struct spi_bus 
{
    void *__FAR private_data;
    const struct spi_operations *__FAR ops;
};


/************************************************
  * @struct   spi_operations
  * @brief   SPI�Ĳ����ӿں���
 ***********************************************/
struct spi_operations 
{
    spi_init_func init;     //spi��ʼ������ 
    spi_deinit_func deinit; //spi�ͷź��� 
    spi_select_func select; //spiģ��ѡ��
    spi_transmit_byte_func transmit_byte;//spi����һ���ֽ�
    spi_transfer_func transfer;//
    spi_reinit_func   reinitialize;//spi���³�ʼ��
    spi_write_array_func write_array;//
    spi_write_read_array_func  write_read_array;//
};




/************************************************************************
  * @brief           spi��ʼ������ 
  * @param[in]       bus spi���� flags ��־
  * @return          0
***********************************************************************/
inline int spi_init(spi_bus_t bus, uint8_t flags);




/************************************************************************
  * @brief           spiδ��ʼ������ 
  * @param[in]       bus spi���� 
  * @return          0
***********************************************************************/
inline void spi_deinit(spi_bus_t bus);




/************************************************************************
  * @brief           spiģ��ѡ�� 
  * @param[in]       bus spi����  which ����  is_select �Ƿ�ѡ��
  * @return          0
***********************************************************************/
inline int spi_select(spi_bus_t bus, uint8_t which, uint8_t is_select);




/************************************************************************
  * @brief           spi����һ���ֽ�
  * @param[in]       bus spi����  which ����  is_select �Ƿ�ѡ��
  * @return          0
***********************************************************************/
inline int spi_transmit_byte(const struct spi_bus *__FAR bus, uint8_t *b);




/************************************************************************
  * @brief           spi����
  * @param[in]       bus spi����  read ��  write д  len  ����
  * @return          0
***********************************************************************/
inline uint16_t spi_transfer(spi_bus_t bus, uint8_t *__FAR read, const uint8_t *__FAR write, uint16_t len);




/***********************************************************************
  * @brief           spi���³�ʼ��
  * @param[in]       bus spi���� flags ��־
  * @return          0
***********************************************************************/
inline int spi_reinit(spi_bus_t bus, uint8_t flags);




/***********************************************************************
  * @brief           spiдһ������
  * @param[in]       bus spi���� len ���ͳ���  data д������
  * @return          0
***********************************************************************/
inline int spi_write_array(spi_bus_t bus,uint16_t len,uint8_t *far data);




/***********************************************************************
  * @brief           spi����дһ������
  * @param[in]       bus spi����  tx_data ��������  tx_len ���ͳ���  rx_data, rx_len
  * @return          0
***********************************************************************/
inline int spi_write_read_array(spi_bus_t bus,uint8_t *far tx_data, uint16_t tx_len, uint8_t *far rx_data, uint16_t rx_len);




/***********************************************************************
  * @brief           spiת��һ���ֽ�
  * @param[in]       bus spi���� flags ��־
  * @return          0
***********************************************************************/
inline uint16_t spi_transfer_use_transmit_onebyte(spi_bus_t bus, const struct hcs12_spi_platform *__FAR private_data,  uint8_t *__FAR read, const uint8_t *__FAR write, uint16_t len);


#endif

