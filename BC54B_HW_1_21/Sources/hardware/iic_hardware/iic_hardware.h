/*******************************************************************************
**                       ������������Դ���޹�˾ Copyright (c)
**                            http://www.ligoo.cn/ 
** @file	 �b   iic_hardware.h                                       

** @brief       1.���IICģ����������͵Ķ��� 
                2.���IICģ��ĳ�ʼ��
                3.���IICģ������ݶ�д
** @copyright		Ligoo Inc.         
** @date		    2017-03-28.
** @author           
*******************************************************************************/ 
#ifndef __IIC_HARDWARE_H__
#define __IIC_HARDWARE_H__

#include "stdint.h"
#include "MC9S12XEP100.h"
#include "includes.h"
#ifndef __FAR
#define __FAR
#endif



/************************************************
  * @struct   hcs12_i2c_regs
  * @brief    iicģ��Ӳ���Ĵ���
 ***********************************************/
struct hcs12_i2c_regs {
    volatile IIC1_IBADSTR address;                //�ӵ�ַ
    volatile IIC1_IBFDSTR freq_div;               //Ƶ�ʷ�Ƶ�Ĵ���
    
    /* bit7      bit6      bit5    bit4    bit3          bit2              bit1     bit0
    0  iic�ر�  �жϹر�   ��ģʽ  ����    ��9λ����ack  ��                ����     ��
    1  ����     ʹ��       ��ģʽ  ����    ����          �������¿�ʼ����  ����     ����ȴ�ģʽ
    */
    volatile IIC1_IBCRSTR control;                // ���ƼĴ���
    
    /* bit7       bit6        bit5    bit4    bit3    bit2        bit1     bit0
    0  ���ڴ���   ������      ����    �ٲ�    ����    �ӽ���      ��       �յ�ack
    1  �������   �ӵ�ַ����  æ      �ٲ�    ����    �ӷ���      �ж�     û���յ�
    */
    volatile IIC1_IBSRSTR status;                //״̬�Ĵ���
    volatile IIC1_IBDRSTR dat;                    //���ݼĴ���
    
    /* bit7       bit6        bit5    bit4    bit3    bit[2 : 0]   
    0  ȫ���ر�   7λ��ַ     ����    ����    ����    
    1  ȫ��ʹ��   10λ��ַ    ����    ����    ����    ��ʾ10λ��ַ�ĸ���λ
    */
    volatile IIC1_IBCR2STR control2;             //���ƼĴ���
};

/************************************************
  * @struct   hcs12_i2c_platform
  * @brief    iicģ��Ӳ���豸�ṹ��
 ***********************************************/
struct hcs12_i2c_platform {
    struct hcs12_i2c_regs *regs;
    uint8_t freq_div;
};

/************************************************
  * @struct   hcs12_i2c
  * @brief    iicģ��Ӳ����������
 ***********************************************/
struct hcs12_i2c {   
   const struct hcs12_i2c_platform *platform;  //ָ��iic�Ĵ����Ľṹ��ָ��
    union {
        struct {
            uint8_t aquired: 1;
            uint8_t txing: 1;
            uint8_t rxing: 1;
            uint8_t txing_addr: 1;
            uint8_t txed_addr: 1;
        } bits;
        uint8_t byte;
    } flags;

    const uint8_t *__FAR tx_data;
    uint8_t *__FAR rx_data;
    uint8_t tx_length;
    uint8_t tx_index;
    uint8_t rx_length;
    uint8_t rx_index;
    uint8_t slave_addr;
    OS_EVENT * muxtex_sem;

};

/************************************************
  * @typedef   hcs12_i2c_t
  * @brief     ���� hcs12_i2c ����
 ***********************************************/
typedef struct hcs12_i2c *__FAR hcs12_i2c_t;

/************************************************************************
  * @brief           iicģ��Ӳ����ʼ������ 
  * @param[in]       hcs12_i2c iicģ���������
  * @return          1��Init_OK   
***********************************************************************/
uint8_t hcs12_i2c_init(struct hcs12_i2c *__FAR i2c); 

/************************************************************************
  * @brief           ���iicģ��Ӳ������ 
  * @param[in]       hcs12_i2c iicģ���������
  * @return          ��  
***********************************************************************/
void hcs12_i2c_deinit(hcs12_i2c_t *__FAR i2c) ;


/************************************************************************
  * @brief           iic��д����
  * @param[in]       hcs12_i2c iicģ����������
  * @param[in]       addr ��������ַ��
  * @param[in]       w ��Ҫ���͵�����ָ��
  * @param[in]       wlen �������ݵĳ���  
  * @param[in]       r ���ض�ȡ���ݵĵ�ַָ��
  * @param[in]       rlen ��ȡ���ݵĳ���     
  * @return          len���շ��ɹ������ݳ���  
***********************************************************************/
uint8_t hcs12_i2c_write_then_read(hcs12_i2c_t i2c,
                                  uint8_t addr,
                                  const uint8_t *__FAR w,
                                  uint8_t wlen,
                                  uint8_t *__FAR r,
                                  uint8_t rlen);





#pragma push
#pragma DATA_SEG __RPAGE_SEG PAGED_RAM

extern struct hcs12_i2c hcs12_impl_i2c0;

extern struct hcs12_i2c hcs12_impl_i2c1;

#pragma pop


#endif

