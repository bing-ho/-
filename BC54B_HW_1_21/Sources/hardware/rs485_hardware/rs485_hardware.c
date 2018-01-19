/*******************************************************************************
**                       ������������Դ���޹�˾ Copyright (c)
**                            http://www.ligoo.cn/ 
** @file	 �b   rs485_hardware.c                                       

** @brief       1.���RS485ģ����������͵Ķ��� 
                2.���RS485ģ��ĳ�ʼ��
                3.���RS485ģ��ķ��͡����չ��ܺ���
** @copyright		Ligoo Inc.         
** @date		    2017-03-27.
** @author      ������      
*******************************************************************************/ 

#include "rs485_hardware.h"

#pragma MESSAGE DISABLE C1420 // Result of function-call is ignored
#pragma MESSAGE DISABLE C2705 // Possible loss of data
#pragma MESSAGE DISABLE C4001 // Condition always FALSE


#pragma DATA_SEG __RPAGE_SEG PAGED_RAM

rs485_t g_rs485_contexts[kRs485MaxDev] = { NULL };   //�洢RS485�豸��Ϣ
Rs485Context g_rs485_default_handlers[kRs485MaxDev];  //�洢RS485ͨѶ��Ϣ

#pragma DATA_SEG DEFAULT


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



/************************************************************************
  * @brief           RS485ͨѶӲ����ʼ��
  * @param[in]       dev Rs485�豸��
  * @param[in]       rs485_param Rs485Param�ṹ��ָ��
  * @param[in]       func Rs485���ջص�����
  * @param[in]       user_data Rs485�û�����
  * @return          Rs485Context�ṹ��ָ��
***********************************************************************/
rs485_t Rs485_Hardware_Init(Rs485Dev dev, Rs485Param *rs485_param , Rs485ReceiveHandler func, void* user_data)
{  
    INT32U BR; //�����ʳ���
    rs485_t context;

    /* check the dev */
    if (dev < kRs485Dev0 || dev > kRs485Dev3)
    {
        return NULL;
    }
    /* failed if the dev is opened */
    if (g_rs485_contexts[dev]) return NULL;

    /* create the handler */
    context = &(g_rs485_default_handlers[dev]);
    if (context == NULL) return context;

    /* init the handler */
    safe_memset(context, 0, sizeof(Rs485Context));
    
    context->dev = dev;
    context->receive_handler = func;
    context->receive_user_data = user_data;
    
    BR = (INT32U)((BMS_BUS_CLOCK + (rs485_param->band<<3)) / (rs485_param->band<<4)); //���㲨����
    RS485_DISPATCH(RS485_INIT_PARAM);

    g_rs485_contexts[dev] = context;
    
    return context;
}

/************************************************************************
  * @brief           ���һ��RS485����
  * @param[in]       context Rs485Context�ṹ��
  * @return          0���ɹ� 1:ʧ��
***********************************************************************/
Result rs485_uninit(rs485_t context)
{
    if (context == NULL) return 1;

    g_rs485_contexts[context->dev] = NULL;

    return 0;
}



/************************************************************************
  * @brief           RS485�˿�0�жϺ���
  * @param[in]       ��
  * @return          ��
***********************************************************************/
void SCI0_ISR_Handler(void)
{
    RS485_ISR_HANDLER(0);
}


/************************************************************************
  * @brief           RS485�˿�1�жϺ���
  * @param[in]       ��
  * @return          ��
***********************************************************************/
void SCI1_ISR_Handler(void)
{
    RS485_ISR_HANDLER(1);
}


/************************************************************************
  * @brief           RS485�˿�3�жϺ���
  * @param[in]       ��
  * @return          ��
***********************************************************************/
void SCI3_ISR_Handler(void)
{
    RS485_ISR_HANDLER(3);
}


/************************************************************************
  * @brief           RS485�����жϴ�����
  * @param[in]       dev Rs485�豸��
  * @param[in]       chr Rs485�����ַ�
  * @return          ��
***********************************************************************/
void rs485_interrupt_rx(Rs485Dev dev, char chr)
{
    rs485_t context = g_rs485_contexts[dev];
    if (context == NULL) return;

    // callback for user
    if (context->receive_handler)
    {
        context->receive_handler(context->dev, chr, context->receive_user_data);  
        return;
    }
} 


/************************************************************************
  * @brief           RS485�����жϴ�����
  * @param[in]       dev Rs485�豸��
  * @return          ��
***********************************************************************/
void rs485_interrupt_tx(Rs485Dev dev)
{
    OS_CPU_SR cpu_sr = 0;
    rs485_t context = g_rs485_contexts[dev];
    if (context == NULL) return;

    OS_ENTER_CRITICAL(); /* Save the PS register */
    if (context->tx_counter > 0)
    {
        unsigned char chr;
        context->serial_flag |= FULL_TX; /* Set the flag "full TX buffer" */
        context->tx_counter--;
        chr = context->tx_buf[context->tx_outptr++];
        if (context->tx_outptr >= RS485_TX_FRAME_SIZE)
            context->tx_outptr = 0;
        RS485_DISPATCH(RS485_SEND_HARDWARE);
    }
    else
    {
        context->serial_flag &= ~FULL_TX; /* Reset flag "full TX buffer" */
        RS485_DISPATCH(RS485_DISABLE_SENDING);
    }
    OS_EXIT_CRITICAL(); /* Restore the PS register */
}


/************************************************************************
  * @brief           RS485���ͺ���
  * @param[in]       context Rs485Context�ṹ��ָ��
  * @param[in]       chr RS485�����ַ�
  * @return          0���ɹ���1��ʧ�ܡ�
***********************************************************************/
Result rs485_send(rs485_t context, char chr)
{
    /* variables */
    OS_CPU_SR cpu_sr = 0;
    if (context == NULL) return 1;
    
    OS_ENTER_CRITICAL();
    for(;;) 
    {
        if ((context->serial_flag & FULL_TX) == 0) {
            context->serial_flag |= FULL_TX; /* Set the flag "full TX buffer" */
            RS485_DISPATCH(RS485_SEND_HARDWARE);
            break;
        }
        if (context->tx_counter < RS485_TX_FRAME_SIZE) {
            context->tx_buf[context->tx_inptr++] = chr;
            if (context->tx_inptr >= RS485_TX_FRAME_SIZE)
                context->tx_inptr = 0;
            context->tx_counter++;
            break;
        }
        OS_EXIT_CRITICAL();
        OSTimeDly(1);
        OS_ENTER_CRITICAL(); 
    }
    OS_EXIT_CRITICAL(); 
    return 0;
} 


/************************************************************************
  * @brief           ����RS485���պ���
  * @param[in]       context Rs485Context�ṹ��ָ��
  * @param[in]       func Rs485Context���պ���
  * @param[in]       user_data �û�����
  * @return          0���ɹ���1��ʧ�ܡ�
***********************************************************************/
Result rs485_set_recv_handler(rs485_t context, Rs485ReceiveHandler func, void* user_data) 
{
    OS_CPU_SR cpu_sr = 0;
    
    if (context == NULL) return 1;
    
    OS_ENTER_CRITICAL();
    context->receive_handler = func;
    context->receive_user_data = user_data;
    OS_EXIT_CRITICAL();
    return 0;
} 


/************************************************************************
  * @brief           ��ȡRs485Context�ṹ��ָ��
  * @param[in]       dev Rs485�豸��
  * @return          Rs485Context�ṹ��ָ��
***********************************************************************/
rs485_t rs485_get_context(Rs485Dev dev)
{
    if (dev >= kRs485MaxDev) return NULL;

    return g_rs485_contexts[dev];
}


/************************************************************************
  * @brief           ��ȡRS485������
  * @param[in]       bps_index ������������
  * @return          RS485������
***********************************************************************/
INT32U rs485_get_bps(INT8U bps_index)
{
    INT32U bps;
    
    switch(bps_index)
    {
        case 1: bps = 2400;break;
        case 2: bps = 4800;break;
        case 3: bps = 9600;break;
        case 4: bps = 115200;break;
        default: bps = 9600;break;
    }
    return bps;
} 
