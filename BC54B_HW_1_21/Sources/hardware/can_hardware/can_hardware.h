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

#ifndef __CAN_HARDWARE_H__
#define __CAN_HARDWARE_H__


#include "os_cpu.h"
#include "mc9s12xep100.h"
#include "bms_base_cfg.h"
#include "ucos_ii.h"
#include "bms_memory.h"
 
typedef INT16U Result;
typedef struct _CanContext* can_t; 

#define CAN_DATA_MAX_LEN 8 // the maximum size of CAN frame

 #if( SWITCH_ON == UDS_SWITCH_CONFIG)     
#define COM_TX_RUN  0x0
#define COM_TX_STOP 0x1
#define COM_RX_RUN  0x0
#define COM_RX_STOP 0x1

#define CAN_VOLT_OK  0x0
#define CAN_VOLT_NOK 0x1
#endif
/* Synch Jump = 2 Tq clock Cycles
 *
 *  0b01000011
 *    ||||||||__
 *    |||||||___\
                                *    ||||||____ |
 *    |||||_____ |_ CAN Clock Prescaler = 4
 *    ||||______ |
 *    |||_______/
 *    ||________
 *    |_________>- SJW = 2
 */
#if BMS_BUS_CLOCK == 16000000UL

#define CAN_DEV_CONFIG_CLOCK_CYCLES_50 0x67
#define CAN_DEV_CONFIG_CLOCK_CYCLES_100 0x53
#define CAN_DEV_CONFIG_CLOCK_CYCLES_125 0x4F
#define CAN_DEV_CONFIG_CLOCK_CYCLES_250 0x47
#if( SWITCH_ON == UDS_SWITCH_CONFIG)     
#define CAN_DEV_CONFIG_CLOCK_CYCLES_500 0x41  //zmx  43->41
#else
#define CAN_DEV_CONFIG_CLOCK_CYCLES_500 0x43
#endif
#define CAN_DEV_CONFIG_CLOCK_CYCLES_800 0x41
#define CAN_DEV_CONFIG_CLOCK_CYCLES_1000 0x41

#elif BMS_BUS_CLOCK == 48000000UL

#define CAN_DEV_CONFIG_CLOCK_CYCLES_50 0x67
#define CAN_DEV_CONFIG_CLOCK_CYCLES_100 0x53
#define CAN_DEV_CONFIG_CLOCK_CYCLES_125 0x4F
#define CAN_DEV_CONFIG_CLOCK_CYCLES_250 0x47
#if( SWITCH_ON == UDS_SWITCH_CONFIG)   
#define CAN_DEV_CONFIG_CLOCK_CYCLES_500 0x45 //zmx  43->45
#else
#define CAN_DEV_CONFIG_CLOCK_CYCLES_500 0x43
#endif
#define CAN_DEV_CONFIG_CLOCK_CYCLES_800 0x43
#define CAN_DEV_CONFIG_CLOCK_CYCLES_1000 0x41

#endif
/* Set Number of samples per bit, TSEG1 and TSEG2
 * bit rate=Fclk/pres v/(1+TSEG1+TSEG2)=16M/4/(1+2+5)=500kbps
 *  0b00010100
 *    ||||||||__
 *    |||||||___\
                                *    ||||||____ |- TSEG1 = 5
 *    |||||_____/
 *    ||||______
 *    |||_______\_ TSEG2 = 2
 *    ||________/
 *    |_________ One sample per bit
 */
#if BMS_BUS_CLOCK == 16000000UL

#define CAN_DEV_CONFIG_SAMPLES_50 0x14
#define CAN_DEV_CONFIG_SAMPLES_100 0x14
#define CAN_DEV_CONFIG_SAMPLES_125 0x14
#define CAN_DEV_CONFIG_SAMPLES_250 0x14
#if( SWITCH_ON == UDS_SWITCH_CONFIG)
#define CAN_DEV_CONFIG_SAMPLES_500 0x3A  //zmx  14->3A
#else
#define CAN_DEV_CONFIG_SAMPLES_500 0x14
#endif
#define CAN_DEV_CONFIG_SAMPLES_800 0x16
#define CAN_DEV_CONFIG_SAMPLES_1000 0x14

#elif BMS_BUS_CLOCK == 48000000UL

#define CAN_DEV_CONFIG_SAMPLES_50 0x7E
#define CAN_DEV_CONFIG_SAMPLES_100 0x7E
#define CAN_DEV_CONFIG_SAMPLES_125 0x7E
#define CAN_DEV_CONFIG_SAMPLES_250 0x7E
#if( SWITCH_ON == UDS_SWITCH_CONFIG)
#define CAN_DEV_CONFIG_SAMPLES_500 0x2B  //zmx  7E->2B
#else
#define CAN_DEV_CONFIG_SAMPLES_500 0x7E
#endif
#define CAN_DEV_CONFIG_SAMPLES_800 0x39
#define CAN_DEV_CONFIG_SAMPLES_1000 0x7E

#endif

/* Set two 32-bit Filters
 *
 *  0b00000000
 *    ||||||||__
 *    |||||||___\_ Filter Hit Indicator
 *    ||||||____/
 *    |||||_____ Unimplemented
 *    ||||______
 *    |||_______>- two 32-bit Acceptance Filters
 *    ||________
 *    |_________>- Unimplemented
 */
#define CAN_DEV_CONFIG_FILTERS 0x00

/* Reset Receiver Flags
 *
 *  0b11000011
 *    ||||||||__ Receive Buffer Full Flag
 *    |||||||___ Overrun Interrupt Flag
 *    ||||||____
 *    |||||_____>- Transmitter Status Bits
 *    ||||______
 *    |||_______>- Receiver Status Bits
 *    ||________ CAN Status Change Interrupt Flag
 *    |_________ Wake-Up Interrupt Flag
 */
#define CAN_DEV_CONFIG_RESET_RECEIVER_FLAGS 0xC3


/* Enable Receive Buffer Full Interrupt  CAN0RIER = 0x01;
                              *  0b00101001
                              *  0b00000001
                              *    ||||||||__ Receive Buffer Full Int enabled
                              *    |||||||___ Overrun Int disabled
                              *    ||||||____
                              *    |||||_____>- Tx Status Change disabled
                              *    ||||______
                              *    |||_______>- Rx Status Change disabled
                              *    ||________ Status Change Int disabled
                              *    |_________ Wake-Up Int disabled
                              */
#if( SWITCH_ON == UDS_SWITCH_CONFIG)
#define CAN_DEV_CONFIG_RECEIVE_FULL_INTERRUPT 0x05
#else
#define CAN_DEV_CONFIG_RECEIVE_FULL_INTERRUPT 0x29
#endif

#if( SWITCH_ON == UDS_SWITCH_CONFIG)
 typedef enum{CAN_NORMAL,CAN_BOFF}c_busoff_status;
 typedef enum{BUSOFF_DTC_NOK,BUSOFF_DTC_OK}c_busoff_dtc_status;
 
 typedef struct{
 	c_busoff_status   can_boff_state;
 	c_busoff_dtc_status   canboff_dtc_state;
	INT16U             busoff_time_counte;
	INT16U             can_boff_counte;
	INT16U             no_busoff_time_counte;
}c_boff_descriptor;

#define   CAN_BOFF_TIME_COUNTE    97u
#define   CAN_BOFF_DTC_COUNTE    1u
#define   CAN_BOFF_DTC_TIME_COUNTE    500u
/* Bitmasks of CANRFLG: */
#define TSTAT0_UDS   ((uint8)0x04)    /* transmitter status bits TSTAT[0:1]           */
#define TSTAT1_UDS   ((uint8)0x08)    /* TSTAT[0:1] =11,bus off                       */
#define CSCIF_UDS    ((uint8)0x40)    /* can status change  flag                      */
#define BOFFIF   ((uint8)(TSTAT0_UDS|TSTAT1_UDS))  /* BusOff flag*/

   
#define CANCTL1_BORM  ((INT8U)(1u << 3u))
#define CANMISC_BOHOLD ((INT8U)(1u << 0u))

 #endif
/**********************************************
 *
 * CAN Configuration
 *
 ***********************************************/
/* Enter Initialization Mode
 *  0b00000001
 *    ||||||||__ Enter Initialization Mode
 *    |||||||___ Sleep Mode Request bit
 *    ||||||____ Wake-Up disabled
 *    |||||_____ Time stamping disabled
 *    ||||______ Synchronized Status
 *    |||_______ CAN not affected by Wait
 *    ||________ Receiver Active Status bit
 *    |_________ Received Frame Flag bit
 */
#define CAN_DEV_CONFIG_INIT_MODE    0x01
#define CAN_DEV_CONFIG_SLEEP_MODE   0x02
/* Enable MSCAN module and not LoopBack Mode   CAN0CTL1_CLKSRC   = 1;
 *
 *  0b10100000
 *    ||||||||__ Initialization Mode Acknowledge
 *    |||||||___ Sleep Mode Acknowledge
 *    ||||||____ Wake-up low-pass filter disabled
 *    |||||_____ Unimplemented
 *    ||||______ Listen Only Mode disabled
 *    |||_______ not Loop Back Mode enabled
 *    ||________ Ext Osc/Xtal as Clock Source
 *    |_________ MSCAN Module enabled
 */
#define CAN_DEV_CONFIG_MODE 0xc0
#define CAN_DEV_LISTEN_MODE 0xd0
#define CAN_SEND_MESSAGE_BUFF_FLAG_MASK     0x07
#define CAN_MODE_STATUS_OK                  0x00
#define CAN_MODE_STATUS_WARNNING            0x01
#define CAN_MODE_STATUS_ERR                 0x02
#define CAN_MODE_STATUS_BUFF_OFF            0x03

#define  OVRFLOW        1
#define  NOMAL          0
#define  CANLOAD_MAX       100          //CAN ���߸�������˲���ֵ 

/**********************************************
  * @brief  ������ ���ڶ�CPU�Ĵ�����д����
 ***********************************************/
#define CAN_DEFINE_SET(REGISTER) CAN_DEFINE_SET_EX(REGISTER, byte)

#define CAN_DEFINE_SET_EX(REGISTER, TYPE) \
void can_set_##REGISTER(CanDev dev, TYPE value) \
{ \
    switch(dev) \
    { \
    case kCanDev0: \
        CAN0##REGISTER = value; \
        break; \
    case kCanDev1: \
        CAN1##REGISTER = value; \
        break; \
    case kCanDev2: \
        CAN2##REGISTER = value; \
        break; \
    case kCanDev3: \
        CAN3##REGISTER = value; \
        break; \
    case kCanDev4: \
        CAN4##REGISTER = value; \
        break; \
    } \
}

#define CAN_DEFINE_GET(REGISTER) CAN_DEFINE_GET_EX(REGISTER, byte)

#define CAN_DEFINE_GET_EX(REGISTER, TYPE) \
TYPE can_get_##REGISTER(CanDev dev) \
{ \
    switch(dev) \
    { \
    case kCanDev0: \
        return CAN0##REGISTER; \
        break; \
    case kCanDev1: \
        return CAN1##REGISTER; \
        break; \
    case kCanDev2: \
        return CAN2##REGISTER; \
        break; \
    case kCanDev3: \
        return CAN3##REGISTER; \
        break; \
    case kCanDev4: \
        return CAN4##REGISTER; \
        break; \
    default: \
        return 0; \
    } \
}

#define CAN_DEFINE_GET_ADDRESS(REGISTER) \
byte* can_get_##REGISTER##_address(CanDev dev) \
{ \
    switch(dev) \
    { \
    case kCanDev0: \
        return &CAN0##REGISTER; \
        break; \
    case kCanDev1: \
        return &CAN1##REGISTER; \
        break; \
    case kCanDev2: \
        return &CAN2##REGISTER; \
        break; \
    case kCanDev3: \
        return &CAN3##REGISTER; \
        break; \
    case kCanDev4: \
        return &CAN4##REGISTER; \
        break; \
    default: \
        return NULL; \
    } \
}

/************************************************
  * @enum   CanDev
  * @brief  CAN�豸ͨ����
 ***********************************************/
 typedef enum
{  
    kCanDev0,           
    kCanDev1,
    kCanDev2,           
    kCanDev3,           
    kCanDev4,
    kCanDevMaxCount
} CanDev;
  
/************************************************
  * @enum   CanMode
  * @brief  CAN��ʶ��
 ***********************************************/
typedef enum
{
    kCanStandardMode = 0,  //
    kCanExtendMode = 1,   //
} CanMode;

/************************************************
  * @enum   CanBps
  * @brief  CAN���ò�����
 ***********************************************/
typedef enum
{
    kCan50kBps = 50,
    kCan100kBps = 100,
    kCan125kBps = 125,
    kCan250kBps = 250,
    kCan500kBps = 500,
    kCan800kBps = 800,
    kCan1000kBps = 1000
} CanBps;
 

/************************************************
  * @enum   CanFilteMode
  * @brief  CAN�˲�������ģʽ
 ***********************************************/
 typedef enum
{
   CanFilte32,    // 2��32λ���ձ�ʶ���˲���
   CanFilte16,    // 4��16λ���ձ�ʶ���˲���
   CanFilte8,     // 8��8λ���ձ�ʶ���˲���
   CanFilteClose  //��ʶ���˲��� �ر�
} CanFilteMode;

/************************************************
  * @enum   CanRTStau
  * @brief  CAN����/������״̬
 ***********************************************/
typedef enum
{
   CanOK,    // CAN�豸������/��������
   CanWRN,    // CAN�豸����/����������
   CanERR,     // CAN�豸����/����������
   CanBusOff   // CAN�豸����/���������� 
} CanRTStau;

/************************************************
  * @union   Can32Bit
  * @brief   CAN id����
 ***********************************************/
 typedef union
{
    INT32U value; // 32-bit Data Type
    INT16U words[2]; // 16-bit Data Type
    INT8U bytes[4]; // Four 8-bit Data Type
} Can32Bit;


/************************************************
  * @struct   CanDriveInfo
  * @brief    CANģ��Ӳ����ʼ����������
 ***********************************************/
typedef  struct
{
    CanDev  dev;   //CAN�豸��
    INT32U  receive_0_id; // ��1���˲�ҳ��ʶ������
    INT32U  mask_0_id;  // ��1���˲���ҳ��ʶ����
    INT32U  receive_1_id; // ��2���˲�ҳ��ʶ��������
    INT32U  mask_1_id;  // ��2���˲���ҳ��ʶ����
    CanBps  bps;  //����������ֵ
    CanFilteMode  filtemode; // CAN�˲�������ģʽ  

} CanDriveInfo,*p_CanDriveInfo;


/************************************************
  * @struct   Can_Stau
  * @brief    ����CAN���ߵ�״̬
 ***********************************************/
typedef  struct
{
    CanRTStau   rxstau;        //CAN������״̬
    CanRTStau   txstau;        //CAN������״̬
    INT8U       canload;       //CAN����״̬ 0:���� 1:���
} Can_Stau;
 
/************************************************
  * @struct   CanMessage
  * @brief    CAN��������
 ***********************************************/
typedef struct
{
   /* Can32Bit id;
    Can32Bit mask;
    INT8U len; //���ĳ���
    INT8U data[CAN_DATA_MAX_LEN]; //CAN Message data
  */  
    Can32Bit   id;  // CAN����id 
    Can32Bit mask;
    INT8U    len;  // CAN ���ĳ��� 
    INT8U    data[CAN_DATA_MAX_LEN]; //CAN��������������8
    INT8U    piro;  //�������ȼ�
    CanMode  canidmode;
    
} CanMessage;


 /************************************************
  * @struct   CanHandler
  * @brief    CAN�ӿڻص�����
 ***********************************************/
 typedef INT16U (*CanOnReceiveFunc)(can_t handle, CanMessage* msg, void* userdata);
 typedef struct
 {
   CanOnReceiveFunc func;
    void* user_data;
 } CanHandler, *CanHandlerPtr;

 /************************************************
  * @struct   CanInfo
  * @brief    CAN��Ϣ���ԣ������ӿڣ�
 ***********************************************/
 typedef struct
{
    CanDev dev; //< CAN Device
    INT32U receive_id; //< CAN Receive ID
    INT32U mask_id; // < CAN Mask ID 
    INT32U receive_1_id; //< CAN Receive ID
    INT32U mask_1_id; // < CAN Mask ID
    CanBps bps; //< bit rate, the value is not supported in the version
    CanFilteMode  filtemode; // CAN�˲�������ģʽ  
    
    CanMode mode; //< the mode        
    CanMode mode_1; //< the mode

    CanOnReceiveFunc receive_callback; //< the callback handling function
    void* receive_userdata; //< the userdata of the callback function

    CanMessage* _PAGED buffers; //< the mode
    INT8U buffer_count; //< the mode
} CanInfo;


 /************************************************
  * @struct   CanInfo
  * @brief    CAN�������ԣ�һ���ӿڣ�
 ***********************************************/
typedef struct _CanContext
{
    CanDev dev;
    CanInfo info;

    INT32U receive_id;
    INT32U mask_id;
    INT32U receive_1_id;
    INT32U mask_1_id;
    INT8U read_buffer_count;
    INT8U read_buffer_write_pos;
    INT8U read_buffer_read_pos;
    OS_EVENT* read_buffer_event;

    BOOLEAN     buffers_is_allocated_by_self; // �ڲ��������buffer
} CanContext, *can_t;
 /************************************************
  * @struct   udsbuffrerinfo
  * @brief    UDS CAN�������ԣ�һ���ӿڣ�
 ***********************************************/

#if( SWITCH_ON == UDS_SWITCH_CONFIG)
typedef  struct _udsbuffrerinfo
 {
   CanMessage* _PAGED buffers; //< the mode
   INT8U buffer_count;
   INT8U read_buffer_count;
   INT8U read_buffer_write_pos;
   INT8U read_buffer_read_pos;
 }udsbuffrerinfo,*__far udsbuffrerinfo_t;

#endif
/************************************************************************
  * @brief           CANģ��Ӳ����ʼ������ 
  * @param[in]       candr_info CANģ����������
  * @return          1��CAN_ERROR   0��CAN_OK
***********************************************************************/
Result Can_Hardware_Init (p_CanDriveInfo candr_info);

/************************************************************************
  * @brief           ��32λ�˲���ģʽ�£�����չ֡�˲�IDת����MSCAN��ʽ��
                     ����ֱ����д��Ӧ�Ĵ�����
  * @param[in]       id ��չ֡�˲�ID
  * @return          MSCAN��ʽ���˲�ID 
***********************************************************************/
INT32U FilteId_extend_32to_MSCAN (INT32U id);

/************************************************************************
  * @brief           ��32λ�˲���ģʽ�£���׼֡�˲�IDת����MSCAN��ʽ��
                     ����ֱ����д��Ӧ�Ĵ�����
  * @param[in]       id ��׼֡�˲�ID
  * @return          MSCAN��ʽ���˲�ID 
***********************************************************************/
INT32U FilteId_std_32to_MSCAN (INT32U id);
/************************************************************************
  * @brief           ��2��16λ�˲���ģʽ�£���չ֡�˲�IDת����MSCAN��ʽ 
                     ����ֱ����д��Ӧ�Ĵ�����
                     ע�⣺����ģʽֻ���˲���14λID
  * @param[in]       id ��׼֡�˲�ID
  * @return          MSCAN��ʽ���˲�ID 
***********************************************************************/
INT32U FilteId_extend_2_16to_MSCAN (INT32U id);
/************************************************************************
  * @brief           ��2��16λ�˲���ģʽ�£���׼֡�˲�IDת����MSCAN��ʽ 
                     ����ֱ����д��Ӧ�Ĵ�����
  * @param[in]       id ��׼֡�˲�ID
  * @return          MSCAN��ʽ���˲�ID 
***********************************************************************/
INT32U FilteId_std_2_16to_MSCAN (INT32U id);

/************************************************************************
  * @brief           ���ò�ʹ���ж�Դ
  * @param[in]       dev CAN�豸��
  * @param[in]       context ��Ҫ���õ��ж�Դ  
  * @return          ��
***********************************************************************/
void Can_EnableInterrupts(CanDev dev,INT8U context);

/************************************************************************
  * @brief           ȡ�����ر��ж�Դ
  * @param[in]       dev CAN�豸��
  * @param[in]       context ��Ҫ���õ��ж�Դ  
  * @return          ��
***********************************************************************/
void Can_DisableInterrupts(CanDev dev,INT8U context);
/************************************************************************
  * @brief           ��CAN�豸dev Ӳ������ܻ��������ݴ���message �ش����м�� 
  * @param[in]       dev CAN�豸   message �������ݲ������м��
  * @return          ��
***********************************************************************/
#if( SWITCH_ON == UDS_SWITCH_CONFIG)
Result Can_Hardware_Rx(CanDev dev,CanMessage* __far message);
#else
void Can_Hardware_Rx(CanDev dev,CanMessage* __far message);
#endif
/************************************************************************
  * @brief           ���м��Ҫ���͵ı�������message�����ݸ�can dev��,��ͨ���ýӿڴ��ݸ�Ӳ����ִ�� 
  * @param[in]       dev CAN�豸   message �������ݲ������м��
  * @return          ��
***********************************************************************/
Result Can_Hardware_Tx (CanDev dev, CanMessage* __far message);

/************************************************************************
  * @brief           ʹ CAN�豸��������ģʽ
  * @param[in]       dev CAN�豸   
  * @return          0���ɹ�  1��ʧ��
***********************************************************************/ 
 Result Can_Sleep (CanDev dev);
 /************************************************************************
  * @brief           ʹ CAN�豸����
  * @param[in]       dev CAN�豸   
  * @return          0���ɹ�  1��ʧ��
***********************************************************************/  
Result Can_Wakeup (CanDev dev);
/************************************************************************
  * @brief           ʹ CAN�豸��������ģʽ
  * @param[in]       dev CAN�豸   
  * @return          0���ɹ�  1��ʧ��
 ***********************************************************************/ 
Result  Can_ListenOnlyMode  (CanDev dev);
/************************************************************************
  * @brief           ʹ CAN�豸��������ģʽ
  * @param[in]       dev CAN�豸   
  * @return          0���ɹ�  1��ʧ��
***********************************************************************/  
Result Can_NomalMode (CanDev dev);
/************************************************************************
  * @brief           ȡ�����ͱ���
  * @param[in]       dev CAN�豸   
  * @return          0���ɹ�  1��ʧ��
***********************************************************************/ 
Result CAN_Cancel_HardwareTx (CanDev dev);

/************************************************************************
  * @brief           ״̬��־��ѯ
  * @param[in]       dev CAN�豸   
  * @return          ��
***********************************************************************/ 
extern typedef Result (*P_Rx_Handler)(CanDev);   //����һ������ָ�� ���� ���ڵײ��ڲ�ѯ��ʽ�µ���CAN���պ���

void  Can_Flg_Poll  (CanDev dev, P_Rx_Handler p_rx_handler );
  /************************************************************************
  * @brief           ���³�ʼ������CAN������busoff���Իָ����ķ���
  * @param[in]       ��  
  * @return          ��
***********************************************************************/ 
extern void can_re_init_emscan(void);
  /************************************************************************
  * @brief           �Ͽ�����CAN������busoff����ֹͣ���ķ���
  * @param[in]       ��  
  * @return          ��
***********************************************************************/ 
extern void can_disconnect_emscan(void);

#if( SWITCH_ON == UDS_SWITCH_CONFIG)
/************************************************************************
  * @brief           ʹ������CAN���ձ�������28������busoff����
  * @param[in]       ��  
  * @return          ��
***********************************************************************/ 
extern void Com_RxStart(void);
/************************************************************************
  * @brief           ��ֹ����CAN���ձ��� ����28������busoff����
  * @param[in]       ��  
  * @return          ��
***********************************************************************/ 
extern void Com_RxStop(void);
/************************************************************************
  * @brief           ʹ������CAN���ͱ��� ����28������busoff����
  * @param[in]       ��  
  * @return          ��
***********************************************************************/ 
extern void Com_TxStart(void);
/************************************************************************
  * @brief           ��ֹ����CAN���ͱ��� ����28������busoff����
  * @param[in]       ��  
  * @return          ��
***********************************************************************/ 
extern void Com_TxStop(void);

 /************************************************************************
  * @brief           ��ȡbusoff״̬
  * @param[in]       ��  
  * @return          0����busoff  1��busoff
***********************************************************************/ 
extern Result getBusOffstate(void);
 /************************************************************************
  * @brief           ��ȡbusoff������״̬
  * @param[in]       ��  
  * @return          0��������busoff������  1������busoff������
***********************************************************************/ 
extern INT8U DTCBusOff(void) ;

 /************************************************************************
  * @brief           ������µ磬��ʱֹͣ����CANͨѶ����¼������
  * @param[in]       ��  
  * @return          ��
***********************************************************************/ 
extern void CanPower_MainFunction(void);
 /************************************************************************
  * @brief           ��ʼ������CAN������busoff���Իָ����ķ���
  * @param[in]       ��  
  * @return          ��
***********************************************************************/ 
//void can_re_init_emscan(void);

 /************************************************************************
  * @brief           busoff���������
  * @param[in]       ��  
  * @return          0��δ��⵽busoff  1����⵽busoff
***********************************************************************/ 
extern Result Canbusoff_MainFunction(void);
 /************************************************************************
  * @brief           busoff����������
  * @param[in]       ��  
  * @return          �� 
***********************************************************************/ 
extern void Canbusoff_process(void);
 /************************************************************************
  * @brief           CANͨѶ��ѹ��������������ڸ�ѹ���ѹֹͣCANͨѶ
  * @param[in]       ��  
  * @return          �� 
***********************************************************************/ 
//extern void CanVoltagecheck_MainFunction(void);
 /************************************************************************
  * @brief           CANͨѶ��ѹ��������������ڸ�ѹ���ѹֹͣCANͨѶ
  * @param[in]       ��  
  * @return          �� 
***********************************************************************/ 

#endif
#endif
