/*******************************************************************************
**                       ������������Դ���޹�˾ Copyright (c)
**                            http://www.ligoo.cn/ 
** @file	 �b   spi_hardware.h                                       

** @brief       1.���SPIģ����������͵Ķ��� 
                2.���SPIģ��ĳ�ʼ��
                3.���SPIģ��ĸ��ֲɼ�ģʽ�Ĺ��ܺ���
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
  * @brief   SPI�Ĳ����ӿں�������
 ***********************************************/
extern struct spi_operations hcs12_spi_ops;


/************************************************
  * @struct   hcs12_spi_t
  * @brief   spiģ�������ָ�����
 ***********************************************/
typedef const struct hcs12_spi_platform *__FAR hcs12_spi_t;


/************************************************
  * @struct   hcs12_spi_regs
  * @brief   spiģ��Ĵ���
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
  * @brief   spiģ�����
 ***********************************************/
struct hcs12_spi_platform 
{
    struct hcs12_spi_regs *regs; //Spi�Ĵ���
    uint8_t baud_reg_value;      //�����ʼĴ���ֵ
    uint8_t cs_num;              //cs
    const struct hcs12_gpio *__FAR cs_pins;//cs����
};




#define SPI_FLAG_CLK_IDLE_HIGH     0x01 //SPI ʱ�ӵ���Ч(���ø�̬)
#define SPI_FLAG_CLK_IDLE_LOW      0x00 //SPI ʱ�Ӹ���Ч(���õ�̬)
#define SPI_FLAG_CLK_FIRST_EDGE    0x02 //
#define SPI_FLAG_CLK_SECOND_EDGE   0x00 //
#define SPI_FLAG_LSB_FIRST         0x04 //LSB ����λ(�������ݴ���ʼ�����λ)
#define SPI_FLAG_MSB_FIRST         0x00 //MSB ����λ(�������ݴ���ʼ�����λ)



/************************************************************************
  * @brief           spiģ��Ӳ����ʼ������ 
  * @param[in]       platform  spi���ò��� flags ��־
  * @return          0
***********************************************************************/
int hcs12_spi_init(hcs12_spi_t spi, uint8_t flags);

/************************************************************************
  * @brief           spiģ��Ӳ��δ��ʼ������ 
  * @param[in]       platform  spi���ò���
  * @return          0
***********************************************************************/
void hcs12_spi_deinit(const struct hcs12_spi_platform *__FAR platform);


/************************************************************************
  * @brief           spiģ��Ӳ��ѡ�� 
  * @param[in]       platform  spi���ò���  which ����  is_select �Ƿ�ѡ��
  * @return          0
***********************************************************************/
int hcs12_spi_select(const struct hcs12_spi_platform *__FAR platform, uint8_t which, uint8_t is_select);


/************************************************************************
  * @brief           spi����һ���ֽ�
  * @param[in]       platform  spi���ò���  dat д������
  * @return          0
***********************************************************************/
int hcs12_spi_transmit_byte(hcs12_spi_t spi, uint8_t *dat);


/************************************************************************
  * @brief           spiд��Ͷ�һ������
  * @param[in]       platform  spi���ò���  read-�� write-д len-����
  * @return          0
***********************************************************************/
uint16_t hcs12_spi_transfer(const struct hcs12_spi_platform *__FAR platform, uint8_t *__FAR read, const uint8_t *__FAR write, uint16_t len);


/************************************************************************
  * @brief           spiд��Ͷ�һ������
  * @param[in]       platform  spi���ò���  tx_data tx_lenr x_data rx_len
  * @return          0
***********************************************************************/
int hcs12_spi_write_read_array(const struct hcs12_spi_platform *far platform,uint8_t *far tx_data, uint16_t tx_len, uint8_t *far rx_data, uint16_t rx_len);


/************************************************************************
  * @brief           spiд��һ������
  * @param[in]       platform  spi���ò���  len ���ݳ��� dat д������
  * @return          0
***********************************************************************/
int hcs12_spi_write_array(const struct hcs12_spi_platform *far platform,uint16_t len,uint8_t *far data);


/************************************************************************
  * @brief           spiģ��Ӳ�����³�ʼ������ 
  * @param[in]       platform  spi���ò���  flags ��־
  * @return          0
***********************************************************************/
int hcs12_spi_reinit(const struct hcs12_spi_platform *__FAR platform, uint8_t flags) ;

#endif