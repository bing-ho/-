/*******************************************************************************
**                       ������������Դ���޹�˾ Copyright (c)
**                            http://www.ligoo.cn/ 
** @file	 �b   rs485_hardware.h                                       

** @brief       1.���RS485ģ����������͵Ķ��� 
                2.���RS485ģ��ĳ�ʼ��
                3.���RS485ģ��ķ��͡����չ��ܺ���
** @copyright		Ligoo Inc.         
** @date		    2017-03-27.
** @author      ������      
*******************************************************************************/ 

#ifndef __RS485_HARDWARE_H__
#define __RS485_HARDWARE_H__

#include "Types.h"
#include "os_cpu.h"
#include "bms_defs.h"
#include "bms_util.h"
#include "MC9S12XEP100.h"


#ifndef BMS_BUS_CLOCK  
#define BMS_BUS_CLOCK 48000000UL
#endif


#define OVERRUN_ERR      1            // Overrun error flag bit  
#define COMMON_ERR       2            // Common error of RX    
#define CHAR_IN_RX       4            // Char is in the RX buffer  
#define FULL_TX          8            // Full transmit buffer  


#define   RS485_RX_FRAME         1   //RS485����֡��
#define   RS485_RX_FRAME_SIZE    74  //RS485����֡����
#define   RS485_TX_FRAME_SIZE    74  //RS485����֡���� 


/************************************************
  * @enum   Rs485DataFormat
  * @brief  RS485ͨѶ�ַ�֡��ʽ
  ***********************************************/
typedef enum
{
   kRs485Data8Bits,// 1����ʼλ��8������λ��1��ֹͣλ
   kRs485Data9Bits // 1����ʼλ��9������λ��1��ֹͣλ
}Rs485DataFormat;


/************************************************
  * @enum   Rs485ParityEnable
  * @brief  ʹ��RS485ͨѶ��żУ�鹦��
  ***********************************************/
typedef enum
{
   kRs485ParityDisable,//��ֹ��żУ��
   kRs485ParityEnable  //������żУ��
}Rs485ParityEnable;


/************************************************
  * @enum   Rs485ParityType
  * @brief  RS485ͨѶ��żУ�鷽ʽ
  ***********************************************/
typedef enum
{
   kRs485EvenParity,//żУ��
   kRs485OddParity  //��У��
}Rs485ParityType;


/************************************************
  * @enum   Rs485Dev
  * @brief  Rs485ͨ����
  ***********************************************/
typedef enum
{
   kRs485Dev0 = 0,
   kRs485Dev1,
   kRs485Dev2,
   kRs485Dev3,
   kRs485MaxDev,
}Rs485Dev;


/************************************************
  * @struct   Rs485Param
  * @brief  RS485ͨѶ�ĳ�ʼ������
  ***********************************************/
typedef struct
{
    INT32U band; //������
    Rs485DataFormat data_format; //�ַ�֡��ʽ
    Rs485ParityEnable parity_enable; //��żУ��ʹ�����
    Rs485ParityType parity_type; //��żУ�鷽ʽ
}Rs485Param;


/************************************************
  * @func   Rs485ReceiveHandler
  * @brief  RS485���ջص�����
  ***********************************************/
typedef int (*Rs485ReceiveHandler)(Rs485Dev dev, char chr, void* user_data); 


/************************************************
  * @func   Rs485TransmitHandler
  * @brief  RS485���ͻص�����
  ***********************************************/
typedef void (*Rs485TransmitHandler)(Rs485Dev dev);


/************************************************
  * @struct   Rs485Context
  * @brief  RS485ͨѶ��Ϣ
  ***********************************************/
typedef struct _Rs485Context
{
    Rs485Dev dev;  // ͨ����
    Rs485ReceiveHandler receive_handler;  //���ӽ��ջص�����
    Rs485TransmitHandler transmit_handler;  //���ӷ��ͻص�����
    void* receive_user_data;
    INT8U rx_counter;
    INT8U rx_inptr;
    INT8U rx_outptr;
    INT8U rx_index; //0 ~ RS485_RX_FRAME_SIZE-1
    INT8U rx_buf[RS485_RX_FRAME][RS485_RX_FRAME_SIZE];
    INT8U tx_counter;
    INT8U tx_inptr;
    INT8U tx_outptr;
    INT8U tx_buf[RS485_TX_FRAME_SIZE];
    INT8U serial_flag; //Flags for serial communication
                      // Bit 0 - Overrun error 
                      // Bit 1 - Common error of RX 
                      // Bit 2 - Char in the RX buffer 
                      // Bit 3 - Full TX buffer
}Rs485Context, *_PAGED rs485_t;


#pragma DATA_SEG __RPAGE_SEG PAGED_RAM

extern rs485_t g_rs485_contexts[kRs485MaxDev];   //�洢RS485�豸��Ϣ
extern Rs485Context g_rs485_default_handlers[kRs485MaxDev];  //�洢RS485ͨѶ��Ϣ

#pragma DATA_SEG DEFAULT 




#define setReg(reg, val)       (reg = (word)(val))   //���üĴ���
#define getReg(reg)            (reg)                 //��ȡ�Ĵ���

#if 1

/************************************************************************
  * @brief           RS485ͨѶ�Ĵ�������
  * @param[in]       DEV���豸���
  * @return          ��
***********************************************************************/
#define RS485_INIT_PARAM(DEV)  \
do{ \
    SCI##DEV##BDH = (INT8U)(BR >> 8) & 0xff; \
    SCI##DEV##BDL = (INT8U)BR & 0xff; \
    SCI##DEV##CR1 = (rs485_param->data_format<<4)|(rs485_param->parity_enable<<1) | (rs485_param->parity_type); /*�����ַ�֡��ʽ����żУ�鹦�ܼ���ʽ*/  \
    SCI##DEV##CR2 |= (SCI##DEV##CR2_TE_MASK | SCI##DEV##CR2_RE_MASK | SCI##DEV##CR2_RIE_MASK); \
}while(0)


/************************************************************************
  * @brief           RS485�����ֽ�����
  * @param[in]       DEV���豸���
  * @return          ��
***********************************************************************/
#define RS485_SEND_HARDWARE(DEV)  \
do{ \
    (void)SCI##DEV##SR1;  /*�����ж������־*/   \
    SCI##DEV##DRL = (INT8U)chr;  /*���ݴ��뷢�����ݼĴ���*/   \
    SCI##DEV##CR2_TIE = 1;  /*ʹ�ܷ����ж�*/   \
    SCI##DEV##CR2_TE = 1;   /*ʹ�ܷ���*/    \
}while(0)
  
  
/************************************************************************
  * @brief           RS485�����ֽ�����
  * @param[in]       DEV���豸���
  * @param[in]       rxch�������ַ�
  * @return          ��
***********************************************************************/
#define RS485_RECV_HARDWARE(DEV, rxch){ rxch = SCI##DEV##DRL; /*��ȡ�������ݼĴ�������*/ }


/************************************************************************
  * @brief           ��ֹ����
  * @param[in]       DEV���豸���
  * @return          ��
***********************************************************************/
#define RS485_DISABLE_SENDING(DEV) \
do{ \
    SCI##DEV##CR2_TIE = 0;  /* Disable transmit interrupt */   \
    SCI##DEV##CR2_TE = 0;   /* Disable transmit */   \
}while(0)


/************************************************************************
  * @brief           ͨ��ͨ����ѡ���ܺ���
  * @param[in]       FUN��RS485���ܺ���
  * @return          ��
***********************************************************************/
//#ifndef RS485_DISPATCH(FUN)
#define RS485_DISPATCH(FUN) \
do{ \
  if(context->dev == kRs485Dev0) \
    FUN(0); \
  if(context->dev == kRs485Dev1) \
    FUN(1); \
  else if(context->dev == kRs485Dev2) \
    FUN(2); \
  else if(context->dev == kRs485Dev3) \
    FUN(3); \
}while(0)
//#endif 


/************************************************************************
  * @brief           RS485�жϷ��������
  * @param[in]       DEV���豸���
  * @return          ��
***********************************************************************/
#define RS485_ISR_HANDLER(DEV) \
  byte StatReg = getReg(SCI##DEV##SR1); \
  if (g_rs485_contexts[kRs485Dev##DEV] == NULL) return; \
  if (StatReg & (SCI##DEV##SR1_OR_MASK | SCI##DEV##SR1_FE_MASK | SCI##DEV##SR1_NF_MASK)) { /* Is any error flag set? */\
    g_rs485_contexts[kRs485Dev##DEV]->serial_flag |= COMMON_ERR;             /* If yes then set an internal flag */\
    (void) SCI##DEV##DRL;                    /* Dummy read of data register - clear error bits */ \
    StatReg &= ~SCI##DEV##SR1_RDRF_MASK;     /* Clear the receive data flag to discard the errorneous data */ \
  } \
  if (StatReg & SCI##DEV##SR1_RDRF_MASK) {   /* Is the receiver interrupt flag set? */ \
    rs485_interrupt_rx(kRs485Dev##DEV, SCI##DEV##DRL); /* If yes, then invoke the internal service routine. This routine is inlined. */ \
  } \
  if (SCI##DEV##CR2_TIE) {                   /* Is the transmitter interrupt enabled? */ \
    if (StatReg & SCI##DEV##SR1_TDRE_MASK) { /* Is the transmitter interrupt flag set? */ \
      rs485_interrupt_tx(kRs485Dev##DEV); /* If yes, then invoke the internal service routine. This routine is inlined. */ \
    } \
  }

#endif 

 /************************************************************************
  * @brief           RS485ͨѶӲ����ʼ��
  * @param[in]       dev Rs485�豸��
  * @param[in]       rs485_param Rs485Param�ṹ��ָ��
  * @param[in]       func Rs485���ջص�����
  * @param[in]       user_data Rs485�û�����
  * @return          Rs485Context�ṹ��ָ��
***********************************************************************/
rs485_t Rs485_Hardware_Init(Rs485Dev dev, Rs485Param *rs485_param , Rs485ReceiveHandler func, void* user_data);


/************************************************************************
  * @brief           ���һ��RS485����
  * @param[in]       context Rs485Context�ṹ��
  * @return          0���ɹ� 1:ʧ��
***********************************************************************/
Result rs485_uninit(rs485_t context);


/************************************************************************
  * @brief           RS485�˿�0�жϺ���
  * @param[in]       ��
  * @return          ��
***********************************************************************/
void SCI0_ISR_Handler(void);

/************************************************************************
  * @brief           RS485�˿�1�жϺ���
  * @param[in]       ��
  * @return          ��
***********************************************************************/
void SCI1_ISR_Handler(void);


/************************************************************************
  * @brief           RS485�˿�3�жϺ���
  * @param[in]       ��
  * @return          ��
***********************************************************************/
void SCI3_ISR_Handler(void);


/************************************************************************
  * @brief           RS485�����жϴ�����
  * @param[in]       dev Rs485�豸��
  * @param[in]       chr Rs485�����ַ�
  * @return          ��
***********************************************************************/
void rs485_interrupt_rx(Rs485Dev dev, char chr);


/************************************************************************
  * @brief           RS485�����жϴ�����
  * @param[in]       dev Rs485�豸��
  * @return          ��
***********************************************************************/
void rs485_interrupt_tx(Rs485Dev dev);


/************************************************************************
  * @brief           RS485���ͺ���
  * @param[in]       context Rs485Context�ṹ��ָ��
  * @param[in]       chr RS485�����ַ�
  * @return          0���ɹ���1��ʧ�ܡ�
***********************************************************************/
Result rs485_send(rs485_t context, char chr);


/************************************************************************
  * @brief           ����RS485���պ���
  * @param[in]       context Rs485Context�ṹ��ָ��
  * @param[in]       func Rs485Context���պ���
  * @param[in]       user_data �û�����
  * @return          0���ɹ���1��ʧ�ܡ�
***********************************************************************/
Result rs485_set_recv_handler(rs485_t context, Rs485ReceiveHandler func, void* user_data);


/************************************************************************
  * @brief           ��ȡRs485Context�ṹ��ָ��
  * @param[in]       dev Rs485�豸��
  * @return          Rs485Context�ṹ��ָ��
***********************************************************************/
rs485_t rs485_get_context(Rs485Dev dev);


/************************************************************************
  * @brief           ��ȡRS485������
  * @param[in]       bps_index ������������
  * @return          RS485������
***********************************************************************/
INT32U rs485_get_bps(INT8U bps_index);



#endif
