/*******************************************************************************
**                       ������������Դ���޹�˾ Copyright (c)
**                            http://www.ligoo.cn/ 
** @file	 �b   can_hardware.h                                       

** @brief       1.���CANģ��ײ���������͵Ķ��� 
                2.���CANģ��ײ�ĳ�ʼ��
                3.���CANģ��ײ�ı����շ��ȵĹ��ܺ���
** @copyright		Ligoo Inc.         
** @date		    2017-03-23.
** @author      �� ��      
*******************************************************************************/ 


#include "can_hardware.h"
#include "can_nm.h"

#if( SWITCH_ON == UDS_SWITCH_CONFIG)
#include "CanIf.h"
#include "bms_system_voltage.h"
#include "bms_ems_impl.h"
#include "Dtc_code.h"
#include "bms_work_state_impl.h"
#endif
extern Result can_re_init_with_dev(CanDev dev); 
  
static INT8U    Canload_Up;                    //CAN ���߸������������
static INT8U    Canload_Down; 
Can_Stau g_can_stau[kCanDevMaxCount];         // �������״̬����

#if( SWITCH_ON == UDS_SWITCH_CONFIG)
#pragma DATA_SEG __RPAGE_SEG PAGED_RAM
static c_boff_descriptor can_boff_ems;
static INT8U Can_Voltage = CAN_VOLT_OK;
static INT8U Can_Voltage_counter = 0;
static INT8U Com_TxRun = COM_TX_RUN;
static INT8U Com_RxRun = COM_RX_RUN;
static INT32U g_ems_can_stop_tick = 0;
static INT32U g_ems_can_stop_rel_tick = 0;
static INT8U ems_can_stop_flag = 0;
#pragma DATA_SEG DEFAULT
#endif
typedef Result (*P_Rx_Handler)(CanDev dev);   //����һ������ָ�� ���� ���ڵײ��ڲ�ѯ��ʽ�µ���CAN���պ���

/************************************************************************
  * @brief           ��CPU�Ĵ�����д����
  * @param[in]       �Ĵ�����
  * @return          ��
***********************************************************************/
CAN_DEFINE_SET(CTL0)
CAN_DEFINE_SET(CTL1)
CAN_DEFINE_SET(BTR0)
CAN_DEFINE_SET(BTR1)
CAN_DEFINE_SET(IDAC)
CAN_DEFINE_SET(IDAR0)
CAN_DEFINE_SET(IDAR1)
CAN_DEFINE_SET(IDAR2)
CAN_DEFINE_SET(IDAR3)
CAN_DEFINE_SET(IDAR4)
CAN_DEFINE_SET(IDAR5)
CAN_DEFINE_SET(IDAR6)
CAN_DEFINE_SET(IDAR7)
CAN_DEFINE_SET(IDMR0)
CAN_DEFINE_SET(IDMR1)
CAN_DEFINE_SET(IDMR2)
CAN_DEFINE_SET(IDMR3)
CAN_DEFINE_SET(IDMR4)
CAN_DEFINE_SET(IDMR5)
CAN_DEFINE_SET(IDMR6)
CAN_DEFINE_SET(IDMR7)
CAN_DEFINE_SET(RFLG)
CAN_DEFINE_SET(RIER)
CAN_DEFINE_SET(TBSEL)
CAN_DEFINE_SET(TFLG)
CAN_DEFINE_SET(TARQ)
CAN_DEFINE_SET(TXIDR0)
CAN_DEFINE_SET(TXDSR0)
CAN_DEFINE_SET(TXDLR)
CAN_DEFINE_SET(TXTBPR)
CAN_DEFINE_SET(RFLG_CSCIF)
#if( SWITCH_ON == UDS_SWITCH_CONFIG)
CAN_DEFINE_SET(MISC)
#endif
CAN_DEFINE_GET(RXDSR0)
CAN_DEFINE_GET(RXDSR1)
CAN_DEFINE_GET(RXDSR2)
CAN_DEFINE_GET(RXDSR3)
CAN_DEFINE_GET(RXDSR4)
CAN_DEFINE_GET(RXDSR5)
CAN_DEFINE_GET(RXDSR6)
CAN_DEFINE_GET(RXDSR7)
CAN_DEFINE_GET(RXDLR)
CAN_DEFINE_GET(CTL1)
CAN_DEFINE_GET(CTL1_INITAK)
CAN_DEFINE_GET(CTL1_SLPAK)
CAN_DEFINE_GET(TFLG)  
CAN_DEFINE_GET(TBSEL)
CAN_DEFINE_GET(TAAK)
CAN_DEFINE_GET(TXIDR0)        
CAN_DEFINE_GET(RFLG_RXF)    
CAN_DEFINE_GET(RFLG_CSCIF)
CAN_DEFINE_GET(RFLG_TSTAT)
CAN_DEFINE_GET(RFLG_RSTAT)
CAN_DEFINE_GET(RXIDR0)
CAN_DEFINE_GET(RXIDR1)
CAN_DEFINE_GET(RXIDR2)
CAN_DEFINE_GET(RXIDR3)
CAN_DEFINE_GET(RXIDR1_IDE)
CAN_DEFINE_GET(RIER_RXFIE)
CAN_DEFINE_GET(RIER_OVRIE)

CAN_DEFINE_GET(RIER_TSTATE)
CAN_DEFINE_GET(RIER_RSTATE)

#if( SWITCH_ON == UDS_SWITCH_CONFIG)
CAN_DEFINE_GET(RFLG)
CAN_DEFINE_GET(MISC)
//CAN_DEFINE_GET(CTL1)

#endif 
CAN_DEFINE_GET(CTL0)
CAN_DEFINE_GET_ADDRESS(TXDSR0)
CAN_DEFINE_GET_ADDRESS(TXIDR0)

/************************************************************************
  * @brief           ������ ���ò�����
  * @param[in]       candr_info
  * @return          ��
***********************************************************************/
#define CAN_SET_BPS(CONTEXT) \
    switch(CONTEXT->bps)\
    {\
    case kCan50kBps:\
        can_set_BTR0(CONTEXT->dev, CAN_DEV_CONFIG_CLOCK_CYCLES_50);\
        can_set_BTR1(CONTEXT->dev, CAN_DEV_CONFIG_SAMPLES_50);\
        break;\
    case kCan100kBps:\
        can_set_BTR0(CONTEXT->dev, CAN_DEV_CONFIG_CLOCK_CYCLES_100);\
        can_set_BTR1(CONTEXT->dev, CAN_DEV_CONFIG_SAMPLES_100);\
        break;\
    case kCan125kBps:\
        can_set_BTR0(CONTEXT->dev, CAN_DEV_CONFIG_CLOCK_CYCLES_125);\
        can_set_BTR1(CONTEXT->dev, CAN_DEV_CONFIG_SAMPLES_125);\
        break;\
    case kCan250kBps:\
        can_set_BTR0(CONTEXT->dev, CAN_DEV_CONFIG_CLOCK_CYCLES_250);\
        can_set_BTR1(CONTEXT->dev, CAN_DEV_CONFIG_SAMPLES_250);\
        break;\
    case kCan500kBps:\
        can_set_BTR0(CONTEXT->dev, CAN_DEV_CONFIG_CLOCK_CYCLES_500);\
        can_set_BTR1(CONTEXT->dev, CAN_DEV_CONFIG_SAMPLES_500);\
        break;\
    case kCan800kBps:\
        can_set_BTR0(CONTEXT->dev, CAN_DEV_CONFIG_CLOCK_CYCLES_800);\
        can_set_BTR1(CONTEXT->dev, CAN_DEV_CONFIG_SAMPLES_800);\
        break;\
    case kCan1000kBps:\
        can_set_BTR0(CONTEXT->dev, CAN_DEV_CONFIG_CLOCK_CYCLES_1000);\
        can_set_BTR1(CONTEXT->dev, CAN_DEV_CONFIG_SAMPLES_1000);\
        break;\
    default:\
        can_set_BTR0(CONTEXT->dev, CAN_DEV_CONFIG_CLOCK_CYCLES_250);\
        can_set_BTR1(CONTEXT->dev, CAN_DEV_CONFIG_SAMPLES_250);\
        break;\
    }

/************************************************************************
  * @brief           ��32λ�˲���ģʽ�£�����չ֡�˲�IDת����MSCAN��ʽ��
                     ����ֱ����д��Ӧ�Ĵ�����
  * @param[in]       id ��չ֡�˲�ID
  * @return          MSCAN��ʽ���˲�ID 
***********************************************************************/
INT32U FilteId_extend_32to_MSCAN (INT32U id)
{	
  INT32U temp_id = 0;
  temp_id = id << 1;
  temp_id = ((temp_id & 0x3ff80000) << 2) | (temp_id & 0x000fffff | 0x00180000);
  return temp_id;
}

/************************************************************************
  * @brief           ��32λ�˲���ģʽ�£���׼֡�˲�IDת����MSCAN��ʽ��
                     ����ֱ����д��Ӧ�Ĵ�����
  * @param[in]       id ��׼֡�˲�ID
  * @return          MSCAN��ʽ���˲�ID 
***********************************************************************/
INT32U FilteId_std_32to_MSCAN (INT32U id)
{	
  INT32U temp_id = 0;
  temp_id = (id << 21) & 0xFFE00000;
  return temp_id;
}
/************************************************************************
  * @brief           ��2��16λ�˲���ģʽ�£���չ֡�˲�IDת����MSCAN��ʽ 
                     ����ֱ����д��Ӧ�Ĵ�����
                     ע�⣺����ģʽֻ���˲���14λID
  * @param[in]       id ��չ֡�˲�ID
  * @return          MSCAN��ʽ���˲�ID 
***********************************************************************/
INT32U FilteId_extend_2_16to_MSCAN (INT32U id)
{	
  INT32U temp1_id = 0;    
  INT32U temp2_id = 0; 
  temp1_id= id; 
  temp2_id= id;
  temp1_id = ((temp1_id & 0x3ff80000) << 2) | ((temp1_id & 0x00070000));
  temp2_id = ((temp2_id & 0x00003ff8) << 2) | ((temp2_id & 0x00000007));
  temp1_id= temp1_id| temp2_id;
  return temp1_id;
}	

/************************************************************************
  * @brief           ��2��16λ�˲���ģʽ�£���׼֡�˲�IDת����MSCAN��ʽ 
                     ����ֱ����д��Ӧ�Ĵ�����
  * @param[in]       id ��׼֡�˲�ID
  * @return          MSCAN��ʽ���˲�ID 
***********************************************************************/
INT32U FilteId_std_2_16to_MSCAN (INT32U id)
{	
  INT32U temp1_id = 0;    
  INT32U temp2_id = 0;  
  temp1_id= id; 
  temp2_id= id;
  temp1_id = ((temp1_id & 0xffff0000) << 5);
  temp2_id = ((temp2_id & 0x0000ffff) << 5);
  temp1_id= temp1_id| temp2_id;
  return temp1_id;
}
/************************************************************************
  * @brief           ���ò�ʹ���ж�Դ
  * @param[in]       dev CAN�豸��
  * @param[in]       context ��Ҫ���õ��ж�Դ  
  * @return          ��
***********************************************************************/
void Can_EnableInterrupts(CanDev dev,INT8U context)
{ 
  can_set_RIER(dev, context);
}
/************************************************************************
  * @brief           ȡ�����ر��ж�Դ
  * @param[in]       dev CAN�豸��
  * @param[in]       context ��Ҫ���õ��ж�Դ  
  * @return          ��
***********************************************************************/
void Can_DisableInterrupts(CanDev dev,INT8U context)
{ 
  can_set_RIER(dev, ~context); 
}


/************************************************************************
  * @brief           CANģ��Ӳ����ʼ������ 
  * @param[in]       candr_info CANģ����������
  * @return          1��CAN_ERROR   0��CAN_OK
***********************************************************************/
Result Can_Hardware_Init (p_CanDriveInfo candr_info)
{
  if(candr_info->dev >= kCanDevMaxCount) return RES_INVALID_PARAM;  
  can_set_CTL0(candr_info->dev, CAN_DEV_CONFIG_INIT_MODE);
  while (can_get_CTL1_INITAK(candr_info->dev) != 1); // Wait for Initialization Mode acknowledge INITRQ bit = 1
  
  can_set_CTL1(candr_info->dev, CAN_DEV_CONFIG_MODE); // Enable MSCAN module and not LoopBack Mode
  
  
  #if( SWITCH_ON == UDS_SWITCH_CONFIG)
    if(candr_info->dev == kCanDev2)   //���ΪEMS_CAN ������busoff�ֶ��ָ�
    {
       can_set_CTL1(candr_info->dev, (can_get_CTL1(candr_info->dev)|CANCTL1_BORM));

       if((can_get_MISC(candr_info->dev) & CANMISC_BOHOLD) == 1)  ///���busoff�ֶ��ָ���־
       {
          can_set_MISC(candr_info->dev, (can_get_MISC(candr_info->dev)|CANMISC_BOHOLD));
       }
    }
  #endif
#if BMS_SUPPORT_CAN_WKUP      //Ӳ��֧��CAN���ѣ���ʼ��������EN/STB���� 
    if(candr_info->dev == kCanDev2)   //���ΪEMS_CAN ��can�շ�������normalģʽ
    {
       Can_nm_init();
    }
#endif 
  CAN_SET_BPS(candr_info);
  can_set_IDAC(candr_info->dev, (byte)candr_info->filtemode); // Set  Filters
 
  can_set_IDAR0(candr_info->dev, ((candr_info->receive_0_id & 0xFF000000)>>24));
  can_set_IDAR1(candr_info->dev, ((candr_info->receive_0_id & 0x00FF0000)>>16)); 
  can_set_IDAR2(candr_info->dev, ((candr_info->receive_0_id & 0x0000FF00)>>8)); 
  can_set_IDAR3(candr_info->dev, (candr_info->receive_0_id & 0x000000FF)); 
  can_set_IDAR4(candr_info->dev, ((candr_info->receive_1_id & 0xFF000000)>>24));
  can_set_IDAR5(candr_info->dev, ((candr_info->receive_1_id & 0x00FF0000)>>16)); 
  can_set_IDAR6(candr_info->dev, ((candr_info->receive_1_id & 0x0000FF00)>>8)); 
  can_set_IDAR7(candr_info->dev, (candr_info->receive_1_id & 0x000000FF)); 
      
  can_set_IDMR0(candr_info->dev, ((candr_info->mask_0_id & 0xFF000000)>>24));
  can_set_IDMR1(candr_info->dev, ((candr_info->mask_0_id & 0x00FF0000)>>16)); 
  can_set_IDMR2(candr_info->dev, ((candr_info->mask_0_id & 0x0000FF00)>>8)); 
  can_set_IDMR3(candr_info->dev, (candr_info->mask_0_id & 0x000000FF)); 
  can_set_IDMR4(candr_info->dev, ((candr_info->mask_1_id & 0xFF000000)>>24));
  can_set_IDMR5(candr_info->dev, ((candr_info->mask_1_id & 0x00FF0000)>>16)); 
  can_set_IDMR6(candr_info->dev, ((candr_info->mask_1_id & 0x0000FF00)>>8)); 
  can_set_IDMR7(candr_info->dev, (candr_info->mask_1_id & 0x000000FF));     
      
    
     
  can_set_CTL1(candr_info->dev, CAN_DEV_CONFIG_MODE); // Enable MSCAN module and not LoopBack Mode

  can_set_CTL0(candr_info->dev, 0x00); // Exit Initialization Mode Request
  while (can_get_CTL1_INITAK(candr_info->dev) == 1);
   
  can_set_RFLG(candr_info->dev, CAN_DEV_CONFIG_RESET_RECEIVER_FLAGS); // Reset Receiver Flags
  Can_EnableInterrupts(candr_info->dev,CAN_DEV_CONFIG_RECEIVE_FULL_INTERRUPT);          

}



/************************************************************************
  * @brief           ȡ�����ͱ���
  * @param[in]       dev CAN�豸   
  * @return          0���ɹ�  1��ʧ��
***********************************************************************/ 
Result CAN_Cancel_HardwareTx (CanDev dev)
{	
  INT8U can_tx_flag;
  if(dev >= kCanDevMaxCount) return RES_INVALID_PARAM;
  can_tx_flag=can_get_TFLG(dev)^CAN_SEND_MESSAGE_BUFF_FLAG_MASK;
  can_set_TARQ(dev, can_tx_flag);
  return RES_OK;
}
/************************************************************************
  * @brief           ʹ CAN�豸��������ģʽ
  * @param[in]       dev CAN�豸   
  * @return          0���ɹ�  1��ʧ��
***********************************************************************/ 
 Result Can_Sleep (CanDev dev)
{ 
   if(dev >= kCanDevMaxCount) return RES_INVALID_PARAM;
   can_set_CTL0(dev, CAN_DEV_CONFIG_SLEEP_MODE);
   while(can_get_CTL1_SLPAK(dev) != 1);
   
   return RES_OK;
}


/************************************************************************
  * @brief           ʹ CAN�豸����
  * @param[in]       dev CAN�豸   
  * @return          0���ɹ�  1��ʧ��
***********************************************************************/  
Result Can_Wakeup (CanDev dev) 
{
 /* if(dev >= kCanDevMaxCount) return RES_INVALID_PARAM;
    can_set_CTL0(dev, 0x00);
  // while(can_get_CTL1_SLPAK(dev) != 1);
   return RES_OK;
  */ 
   	
    if(dev >= kCanDevMaxCount) return RES_INVALID_PARAM;
    if (Can_Sleep(dev))  return  RES_ERR; // ʹCAN�豸��������ģʽ
    
    can_set_CTL0(dev, CAN_DEV_CONFIG_INIT_MODE);// �������inintģʽ
    while (can_get_CTL1_INITAK(dev) != 1); // �ȴ�����initģʽ
    
    can_set_CTL1(dev, CAN_DEV_CONFIG_MODE);
    
    can_set_CTL0(dev, 0x00); // CAN�豸�˳�INITģʽ
    Can_EnableInterrupts(dev,CAN_DEV_CONFIG_RECEIVE_FULL_INTERRUPT);      
    return  RES_OK;
  
}
/************************************************************************
  * @brief           ʹ CAN�豸��������ģʽ
  * @param[in]       dev CAN�豸   
  * @return          0���ɹ�  1��ʧ��
 ***********************************************************************/ 
Result  Can_ListenOnlyMode  (CanDev dev)
  {
    	
    if(dev >= kCanDevMaxCount) return RES_INVALID_PARAM;
    if (Can_Sleep(dev))  return  RES_ERR; // ʹCAN�豸��������ģʽ
    
    can_set_CTL0(dev, CAN_DEV_CONFIG_INIT_MODE);// �������inintģʽ
    while (can_get_CTL1_INITAK(dev) != 1); // �ȴ�����initģʽ
    can_set_CTL1(dev, CAN_DEV_LISTEN_MODE);// �����������ģʽ
    
    can_set_CTL0(dev, 0x00); // CAN�豸�˳�INITģʽ
    
    
    return  RES_OK;
  }

/************************************************************************
  * @brief           ʹ CAN�豸��������ģʽ
  * @param[in]       dev CAN�豸   
  * @return          0���ɹ�  1��ʧ��
***********************************************************************/  
Result Can_NomalMode (CanDev dev) 
{  	
    if(dev >= kCanDevMaxCount) return RES_INVALID_PARAM;
    if (Can_Sleep(dev))  return  RES_ERR; // ʹCAN�豸��������ģʽ
    
    can_set_CTL0(dev, CAN_DEV_CONFIG_INIT_MODE);// �������inintģʽ
    while (can_get_CTL1_INITAK(dev) != 1); // �ȴ�����initģʽ
    
    can_set_CTL1(dev, CAN_DEV_CONFIG_MODE);
    
    can_set_CTL0(dev, 0x00); // CAN�豸�˳�INITģʽ
    Can_EnableInterrupts(dev,CAN_DEV_CONFIG_RECEIVE_FULL_INTERRUPT);      
    return  RES_OK;
  
}
/************************************************************************
  * @brief           ��CAN�豸dev Ӳ������ܻ��������ݴ���message �ش����м�� 
  * @param[in]       dev CAN�豸   message �������ݲ������м��
  * @return          ��
***********************************************************************/
#if( SWITCH_ON == UDS_SWITCH_CONFIG)
Result Can_Hardware_Rx(CanDev dev, CanMessage* __far message)
#else
 void Can_Hardware_Rx (CanDev dev, CanMessage* __far message)
#endif
{
  INT32U id_temp=0;
   
    if (can_get_RFLG_RXF(dev) && message) //���ջ�������
    {
        message->id.bytes[0] = can_get_RXIDR0(dev);
        message->id.bytes[1] = can_get_RXIDR1(dev);

        if (can_get_RXIDR1_IDE(dev))     //���յ�����չ֡ 
        {   
            message->id.bytes[2] = can_get_RXIDR2(dev);
            message->id.bytes[3] = can_get_RXIDR3(dev);
        }
        message->data[0] = can_get_RXDSR0(dev);
        message->data[1] = can_get_RXDSR1(dev);
        message->data[2] = can_get_RXDSR2(dev);
        message->data[3] = can_get_RXDSR3(dev);
        message->data[4] = can_get_RXDSR4(dev);
        message->data[5] = can_get_RXDSR5(dev);
        message->data[6] = can_get_RXDSR6(dev);
        message->data[7] = can_get_RXDSR7(dev);
        message->len = can_get_RXDLR(dev) & 0x0F;
        if(message->len > CAN_DATA_MAX_LEN) message->len = CAN_DATA_MAX_LEN;
    }
    can_set_RFLG(dev, 1);
    
  #if( SWITCH_ON == UDS_SWITCH_CONFIG)   ///�ò��ִ�������ʵ��UDS 28����
    if(dev != kCanDev2)   //���ΪEMS_CAN �򲻶Ա��Ľ��й���
    {
         return RES_TRUE;
    }
    if(COM_RX_RUN == Com_RxRun)
    {
         return RES_TRUE;
    }
    else
    {
       #if (STANDARD_ID == UDS_CAN_ID_TYPE)
       if(( can_id_to_std_id(message->id.value) == CanIfRxPduConfig[0].CanId)  || ( can_id_to_std_id(message->id.value) == CanIfRxPduConfig[1].CanId) ) 
      #else
       if(( can_id_to_extend_id(message->id.value) == CanIfRxPduConfig[0].CanId)  || ( can_id_to_extend_id(message->id.value) == CanIfRxPduConfig[1].CanId) )
      #endif 
       {
          return RES_TRUE;
       }
       else
       {
          return RES_FALSE;
       }
    }
#endif
}
                                                   
/************************************************************************
  * @brief           ���м��Ҫ���͵ı�������message�����ݸ�can dev��,
                     ��ͨ���ýӿڴ��ݸ�Ӳ����ִ�� 
  * @param[in]       dev CAN�豸   message �������ݲ������м��
  * @return          ��
***********************************************************************/
Result Can_Hardware_Tx (CanDev dev, CanMessage* __far message)
{
    INT8U index; 
    INT8U tbsel = 0; 
    INT8U* address;
    OS_CPU_SR cpu_sr = 0;
    
    if(message == NULL) return RES_INVALID_HANLDE;
    if(dev >= kCanDevMaxCount) return RES_INVALID_PARAM;
    
    if (!can_get_TFLG(dev)) return RES_ERR;      // ���ͻ���������
     
    #if( SWITCH_ON == UDS_SWITCH_CONFIG)
    #if (STANDARD_ID == UDS_CAN_ID_TYPE) 
    if((dev != kCanDev2) ||(COM_TX_RUN == Com_TxRun) || ( can_id_to_std_id(message->id.value) == CanIfTxPduConfig[0].CanId) ) 
    #else
    if((dev != kCanDev2) ||(COM_TX_RUN == Com_TxRun) || ( can_id_to_extend_id(message->id.value) == CanIfTxPduConfig[0].CanId) )
    #endif
    {  
    #endif 
    OS_ENTER_CRITICAL();
    
    can_set_TBSEL(dev, can_get_TFLG(dev)); // ѡ��ջ�����

    tbsel = can_get_TBSEL(dev); 

    // д��ID �Ĵ���
    *((INT32U *) ((INT32U) (can_get_TXIDR0_address(dev)))) = message->id.value;
    
    if (message->len > CAN_DATA_MAX_LEN) message->len = CAN_DATA_MAX_LEN;

    address = can_get_TXDSR0_address(dev);
    for (index = 0; index < message->len; index++)
    {
        *(address + index) = message->data[index]; // д�����ݵ����ͻ�����
    }

    can_set_TXDLR(dev, message->len); // ���÷������ݳ���
    can_set_TXTBPR(dev, message->piro); // �������ȼ�
    can_set_TFLG(dev, tbsel); // ��ʼ����

    OS_EXIT_CRITICAL();
   #if( SWITCH_ON == UDS_SWITCH_CONFIG)  
  }
#endif   
      return RES_OK;
                                    
}
 


/************************************************************************
  * @brief           ״̬��־��ѯ
  * @param[in]       dev CAN�豸   
  * @return          ��
***********************************************************************/ 
void  Can_Flg_Poll  (CanDev dev, P_Rx_Handler p_rx_handler )
{
  if (can_get_RIER_RXFIE(dev)==0)  //���жϷ�ʽ��CAN������ѯ��ʽ����
    {
       if (can_get_RFLG_RXF(dev))             //���ջ���������
       {      
          (void)(*p_rx_handler)(dev);
       } 
    }
      
        
  if (can_get_RIER_OVRIE(dev))         //���㲢�ж�CAN�豸�Ƿ������
    {
      Canload_Up ++;
      if (Canload_Up>=CANLOAD_MAX) 
      {
        g_can_stau[dev].canload = OVRFLOW ; 
        Canload_Up= CANLOAD_MAX;
        Canload_Down=0;
      }
    }
  else 
    { 
      Canload_Down ++;
      if (Canload_Down>=CANLOAD_MAX) 
      {
        g_can_stau[dev].canload = NOMAL ; 
        Canload_Down= CANLOAD_MAX;
        Canload_Up= 0;
      }
    }
  
   
   switch (can_get_RIER_TSTATE(dev)) // ��ȡTSTAT ��־λ ���趨CAN�豸��Ӧ״̬  
    {
      case 0: g_can_stau[dev].txstau =CanOK;
      break;
      case 1: g_can_stau[dev].txstau =CanWRN;
      break;
      case 2: g_can_stau[dev].txstau =CanERR;
      break;
      case 3: g_can_stau[dev].txstau =CanBusOff;  
      break; 
      default:
      break;
    }       
   switch  (can_get_RIER_RSTATE(dev))// ��ȡRSTAT ��־λ ���趨CAN�豸��Ӧ״̬
    {
      case 0: g_can_stau[dev].rxstau =CanOK; 
      break;
      case 1: g_can_stau[dev].rxstau =CanWRN;
      break;
      case 2: g_can_stau[dev].rxstau =CanERR;
      break;
      case 3: g_can_stau[dev].rxstau =CanBusOff;
      break; 
      default:
      break;
    }

}


 

/************************************************************************
  * @brief           ״̬��־��ѯ
  * @param[in]       dev CAN�豸   
  * @return          ��
***********************************************************************/ 
void can_re_init_emscan(void)  //��emscan�������³�ʼ��������busoff�ָ�ͨѶ
{
   (void)can_re_init_with_dev(kCanDev2);
}

/********************************/
/*          can_disconnect      */
/********************************/

 void can_disconnect_with_dev(CanDev dev)
{
   can_set_CTL0(dev, (can_get_CTL0(dev)|CAN_DEV_CONFIG_INIT_MODE));
}


void can_disconnect_emscan(void)  //��emscan�����ʼ��ģʽ�����ڸߵ�ѹֹͣͨѶ
{
   can_disconnect_with_dev(kCanDev2);
}
  #if( SWITCH_ON == UDS_SWITCH_CONFIG)
/********************************/
/*          can_busoff          */
/********************************/

static Result can_busoff_check_with_dev(CanDev dev)  //CAN busoff���
{

   if((can_get_RFLG(dev) & BOFFIF) == BOFFIF )
   /*this controller has entered the bus off state*/
   { 
     Com_TxStop();                                             
     can_re_init_emscan(); 
     
     return RES_TRUE; 
   }

   return RES_FALSE; 
}


Result can_busoff_check_emscan(void)      ///zmx
 {
 
   return can_busoff_check_with_dev(kCanDev2) ;
 }
 
static Result can_busoff_handler_with_dev(CanDev dev)  //CAN busoff���
{

   if(((can_get_RFLG_CSCIF(dev) & CSCIF_UDS)== CSCIF_UDS))
   /*this controller has entered the bus off state*/
   {                                              
      
       can_set_RFLG(dev,CSCIF_UDS); 
     return RES_TRUE; 
   }

   return RES_FALSE; 
}


 Result Canbusoff_MainFunction(void)    ///zmx
 {
 
   return can_busoff_handler_with_dev(kCanDev2) ;
 }


void Canbusoff_process(void)     ///zmx
{
   
     switch (can_boff_ems.can_boff_state)
  {
      case CAN_NORMAL :
    	 if(RES_TRUE == can_busoff_check_emscan()) 
    	 {
    	   Com_TxStop();  
    		 can_boff_ems.busoff_time_counte = 0u;    		 
    		 can_boff_ems.can_boff_state = CAN_BOFF; 
    		 can_boff_ems.no_busoff_time_counte = 0;
    		 if(can_boff_ems.can_boff_counte < CAN_BOFF_DTC_COUNTE)  
    		 {
    		    can_boff_ems.can_boff_counte ++;
    		    if(can_boff_ems.can_boff_counte == CAN_BOFF_DTC_COUNTE) 
    		    {
    		       can_boff_ems.canboff_dtc_state =  BUSOFF_DTC_OK;
    		       set_DTCBusOff();
    		    }
    		 }  		 
    	 }
    	 else
    	 {
    	   can_boff_ems.no_busoff_time_counte ++;
    	   if(can_boff_ems.no_busoff_time_counte >= CAN_BOFF_DTC_TIME_COUNTE) //����5sû��busoff�����busoff����
    	   {
    	     can_boff_ems.no_busoff_time_counte = 0;
    	     can_boff_ems.can_boff_counte = 0;
    	     can_boff_ems.canboff_dtc_state =  BUSOFF_DTC_NOK;
    	   }
    	 }
	    break;
      case CAN_BOFF :

    	  if( ++ can_boff_ems.busoff_time_counte >= CAN_BOFF_TIME_COUNTE)  ///busoff�ָ�ʱ��Ϊ1000ms
    	  {
    		  can_boff_ems.busoff_time_counte = 0u;
    		 
    		  Com_TxStart();   		  
    		  can_boff_ems.can_boff_state = CAN_NORMAL;  
    		  
    	  }
	    break;

      default :
    	  can_boff_ems.can_boff_state = CAN_NORMAL;  
      break;
  }
}



Result getBusOffCounter(void)
{
  return (can_boff_ems.can_boff_counte);
}

void resetBusOffCounter(void)
{
  can_boff_ems.can_boff_counte = 0;
}

Result getBusOffstate(void)
{
 
  if(can_boff_ems.can_boff_state == CAN_BOFF)
  {
      return RES_TRUE;
  }
  else
  {
    return RES_FALSE;
  }
}

INT8U DTCBusOff(void)
{

    return (INT8U)(can_boff_ems.canboff_dtc_state);
}





/********************************/
/*    can_voltagecheck          */
/********************************/
void CanPower_MainFunction(void)  //zmx
{ 

  INT32U now_tick = get_tick_count();    
  if(bms_get_power_state() == 0)  ///��⵽���л���ԴʧЧ����ֹͣems can
  {
      g_ems_can_stop_rel_tick = now_tick;
  
      if(ems_can_stop_flag == 0)
      {
         
          if(get_interval_by_tick(g_ems_can_stop_tick, now_tick) >= 50)
          {                     
                  //can_disconnect_emscan();
                  Uds_DTC_save();    //�µ籣��DTC�����룬ƽʱ��������eeprom����ʱ�䳤���±�����������13ms��CAN���ڲ��Բ�ͨ��

                  ems_can_stop_flag = 1; 
          }
        
      }          

  }
  else    ///��⵽����Դ��Ч��������ems can
  {
  
      g_ems_can_stop_tick = now_tick;
      if(ems_can_stop_flag == 1)
      {
         
          if(get_interval_by_tick(g_ems_can_stop_rel_tick, now_tick) >= 50)
          {
                  //can_re_init_emscan();
                  ems_can_stop_flag = 0; 
          }
        
      }  

      

  }

}

Result getCanVoltagestate(void)
{
 
  if(CAN_VOLT_OK == Can_Voltage)
  {
      return RES_TRUE;
  }
  else
  {
    return RES_FALSE;
  }
}


void  Com_RxStart( void )
{
   Com_RxRun = COM_RX_RUN;
}

void  Com_TxStart( void )
{
   Com_TxRun = COM_TX_RUN;
}

void Com_RxStop( void )
{
    Com_RxRun = COM_RX_STOP;
}
void Com_TxStop( void )
{
    Com_TxRun = COM_TX_STOP;
}

#endif
 










