/*******************************************************************************
**                       安徽力高新能源有限公司 Copyright (c)
**                            http://www.ligoo.cn/ 
** @file	    can_hardware.h                                       

** @brief       1.完成CAN模块底层的数据类型的定义 
                2.完成CAN模块底层的初始化
                3.完成CAN模块底层的报文收发等的功能函数
** @copyright		Ligoo Inc.         
** @date		    2017-03-23.
** @author      王 磊      
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
#define  CANLOAD_MAX       100          //CAN 总线负载溢出滤波阀值 

/**********************************************
  * @brief  宏声明 用于对CPU寄存器读写操作
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
  * @brief  CAN设备通道号
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
  * @brief  CAN标识符
 ***********************************************/
typedef enum
{
    kCanStandardMode = 0,  //
    kCanExtendMode = 1,   //
} CanMode;

/************************************************
  * @enum   CanBps
  * @brief  CAN常用波特率
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
  * @brief  CAN滤波器工作模式
 ***********************************************/
 typedef enum
{
   CanFilte32,    // 2个32位接收标识符滤波器
   CanFilte16,    // 4个16位接收标识符滤波器
   CanFilte8,     // 8个8位接收标识符滤波器
   CanFilteClose  //标识符滤波器 关闭
} CanFilteMode;

/************************************************
  * @enum   CanRTStau
  * @brief  CAN接收/发送器状态
 ***********************************************/
typedef enum
{
   CanOK,    // CAN设备接收器/发送正常
   CanWRN,    // CAN设备接收/发送器警告
   CanERR,     // CAN设备接收/发送器错误
   CanBusOff   // CAN设备接收/发送器掉线 
} CanRTStau;

/************************************************
  * @union   Can32Bit
  * @brief   CAN id描述
 ***********************************************/
 typedef union
{
    INT32U value; // 32-bit Data Type
    INT16U words[2]; // 16-bit Data Type
    INT8U bytes[4]; // Four 8-bit Data Type
} Can32Bit;


/************************************************
  * @struct   CanDriveInfo
  * @brief    CAN模块硬件初始化配置属性
 ***********************************************/
typedef  struct
{
    CanDev  dev;   //CAN设备号
    INT32U  receive_0_id; // 第1个滤波页标识符接收
    INT32U  mask_0_id;  // 第1个滤波器页标识符掩
    INT32U  receive_1_id; // 第2个滤波页标识符接收码
    INT32U  mask_1_id;  // 第2个滤波器页标识符掩
    CanBps  bps;  //波特率设置值
    CanFilteMode  filtemode; // CAN滤波器工作模式  

} CanDriveInfo,*p_CanDriveInfo;


/************************************************
  * @struct   Can_Stau
  * @brief    描述CAN总线的状态
 ***********************************************/
typedef  struct
{
    CanRTStau   rxstau;        //CAN接收器状态
    CanRTStau   txstau;        //CAN发送器状态
    INT8U       canload;       //CAN负载状态 0:正常 1:溢出
} Can_Stau;
 
/************************************************
  * @struct   CanMessage
  * @brief    CAN报文属性
 ***********************************************/
typedef struct
{
   /* Can32Bit id;
    Can32Bit mask;
    INT8U len; //报文长度
    INT8U data[CAN_DATA_MAX_LEN]; //CAN Message data
  */  
    Can32Bit   id;  // CAN报文id 
    Can32Bit mask;
    INT8U    len;  // CAN 报文长度 
    INT8U    data[CAN_DATA_MAX_LEN]; //CAN报文数据域数组8
    INT8U    piro;  //报文优先级
    CanMode  canidmode;
    
} CanMessage;


 /************************************************
  * @struct   CanHandler
  * @brief    CAN接口回调函数
 ***********************************************/
 typedef INT16U (*CanOnReceiveFunc)(can_t handle, CanMessage* msg, void* userdata);
 typedef struct
 {
   CanOnReceiveFunc func;
    void* user_data;
 } CanHandler, *CanHandlerPtr;

 /************************************************
  * @struct   CanInfo
  * @brief    CAN信息属性（二级接口）
 ***********************************************/
 typedef struct
{
    CanDev dev; //< CAN Device
    INT32U receive_id; //< CAN Receive ID
    INT32U mask_id; // < CAN Mask ID 
    INT32U receive_1_id; //< CAN Receive ID
    INT32U mask_1_id; // < CAN Mask ID
    CanBps bps; //< bit rate, the value is not supported in the version
    CanFilteMode  filtemode; // CAN滤波器工作模式  
    
    CanMode mode; //< the mode        
    CanMode mode_1; //< the mode

    CanOnReceiveFunc receive_callback; //< the callback handling function
    void* receive_userdata; //< the userdata of the callback function

    CanMessage* _PAGED buffers; //< the mode
    INT8U buffer_count; //< the mode
} CanInfo;


 /************************************************
  * @struct   CanInfo
  * @brief    CAN内容属性（一级接口）
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

    BOOLEAN     buffers_is_allocated_by_self; // 内部分配接受buffer
} CanContext, *can_t;
 /************************************************
  * @struct   udsbuffrerinfo
  * @brief    UDS CAN内容属性（一级接口）
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
  * @brief           CAN模块硬件初始化函数 
  * @param[in]       candr_info CAN模块配置属性
  * @return          1：CAN_ERROR   0：CAN_OK
***********************************************************************/
Result Can_Hardware_Init (p_CanDriveInfo candr_info);

/************************************************************************
  * @brief           在32位滤波器模式下，将扩展帧滤波ID转换到MSCAN格式，
                     用于直接填写对应寄存器。
  * @param[in]       id 扩展帧滤波ID
  * @return          MSCAN格式的滤波ID 
***********************************************************************/
INT32U FilteId_extend_32to_MSCAN (INT32U id);

/************************************************************************
  * @brief           在32位滤波器模式下，标准帧滤波ID转换到MSCAN格式，
                     用于直接填写对应寄存器。
  * @param[in]       id 标准帧滤波ID
  * @return          MSCAN格式的滤波ID 
***********************************************************************/
INT32U FilteId_std_32to_MSCAN (INT32U id);
/************************************************************************
  * @brief           在2个16位滤波器模式下，扩展帧滤波ID转换到MSCAN格式 
                     用于直接填写对应寄存器。
                     注意：该种模式只能滤波高14位ID
  * @param[in]       id 标准帧滤波ID
  * @return          MSCAN格式的滤波ID 
***********************************************************************/
INT32U FilteId_extend_2_16to_MSCAN (INT32U id);
/************************************************************************
  * @brief           在2个16位滤波器模式下，标准帧滤波ID转换到MSCAN格式 
                     用于直接填写对应寄存器。
  * @param[in]       id 标准帧滤波ID
  * @return          MSCAN格式的滤波ID 
***********************************************************************/
INT32U FilteId_std_2_16to_MSCAN (INT32U id);

/************************************************************************
  * @brief           设置并使能中断源
  * @param[in]       dev CAN设备号
  * @param[in]       context 需要配置的中断源  
  * @return          无
***********************************************************************/
void Can_EnableInterrupts(CanDev dev,INT8U context);

/************************************************************************
  * @brief           取消并关闭中断源
  * @param[in]       dev CAN设备号
  * @param[in]       context 需要配置的中断源  
  * @return          无
***********************************************************************/
void Can_DisableInterrupts(CanDev dev,INT8U context);
/************************************************************************
  * @brief           将CAN设备dev 硬件层接受缓冲区数据存入message 回传给中间层 
  * @param[in]       dev CAN设备   message 保存数据并传给中间层
  * @return          无
***********************************************************************/
#if( SWITCH_ON == UDS_SWITCH_CONFIG)
Result Can_Hardware_Rx(CanDev dev,CanMessage* __far message);
#else
void Can_Hardware_Rx(CanDev dev,CanMessage* __far message);
#endif
/************************************************************************
  * @brief           将中间层要发送的报文内容message，传递给can dev口,并通过该接口传递给硬件层执行 
  * @param[in]       dev CAN设备   message 保存数据并传给中间层
  * @return          无
***********************************************************************/
Result Can_Hardware_Tx (CanDev dev, CanMessage* __far message);

/************************************************************************
  * @brief           使 CAN设备进入休眠模式
  * @param[in]       dev CAN设备   
  * @return          0：成功  1：失败
***********************************************************************/ 
 Result Can_Sleep (CanDev dev);
 /************************************************************************
  * @brief           使 CAN设备唤醒
  * @param[in]       dev CAN设备   
  * @return          0：成功  1：失败
***********************************************************************/  
Result Can_Wakeup (CanDev dev);
/************************************************************************
  * @brief           使 CAN设备进入侦听模式
  * @param[in]       dev CAN设备   
  * @return          0：成功  1：失败
 ***********************************************************************/ 
Result  Can_ListenOnlyMode  (CanDev dev);
/************************************************************************
  * @brief           使 CAN设备进入正常模式
  * @param[in]       dev CAN设备   
  * @return          0：成功  1：失败
***********************************************************************/  
Result Can_NomalMode (CanDev dev);
/************************************************************************
  * @brief           取消发送报文
  * @param[in]       dev CAN设备   
  * @return          0：成功  1：失败
***********************************************************************/ 
Result CAN_Cancel_HardwareTx (CanDev dev);

/************************************************************************
  * @brief           状态标志轮询
  * @param[in]       dev CAN设备   
  * @return          无
***********************************************************************/ 
extern typedef Result (*P_Rx_Handler)(CanDev);   //定义一个函数指针 类型 用于底层在查询方式下调用CAN接收函数

void  Can_Flg_Poll  (CanDev dev, P_Rx_Handler p_rx_handler );
  /************************************************************************
  * @brief           重新初始化整车CAN，用于busoff策略恢复报文发送
  * @param[in]       无  
  * @return          无
***********************************************************************/ 
extern void can_re_init_emscan(void);
  /************************************************************************
  * @brief           断开整车CAN，用于busoff策略停止报文发送
  * @param[in]       无  
  * @return          无
***********************************************************************/ 
extern void can_disconnect_emscan(void);

#if( SWITCH_ON == UDS_SWITCH_CONFIG)
/************************************************************************
  * @brief           使能整车CAN接收报文用于28服务与busoff策略
  * @param[in]       无  
  * @return          无
***********************************************************************/ 
extern void Com_RxStart(void);
/************************************************************************
  * @brief           禁止整车CAN接收报文 用于28服务与busoff策略
  * @param[in]       无  
  * @return          无
***********************************************************************/ 
extern void Com_RxStop(void);
/************************************************************************
  * @brief           使能整车CAN发送报文 用于28服务与busoff策略
  * @param[in]       无  
  * @return          无
***********************************************************************/ 
extern void Com_TxStart(void);
/************************************************************************
  * @brief           禁止整车CAN发送报文 用于28服务与busoff策略
  * @param[in]       无  
  * @return          无
***********************************************************************/ 
extern void Com_TxStop(void);

 /************************************************************************
  * @brief           获取busoff状态
  * @param[in]       无  
  * @return          0：非busoff  1：busoff
***********************************************************************/ 
extern Result getBusOffstate(void);
 /************************************************************************
  * @brief           获取busoff故障码状态
  * @param[in]       无  
  * @return          0：不设置busoff故障码  1：设置busoff故障码
***********************************************************************/ 
extern INT8U DTCBusOff(void) ;

 /************************************************************************
  * @brief           监控上下电，及时停止整车CAN通讯并记录故障码
  * @param[in]       无  
  * @return          无
***********************************************************************/ 
extern void CanPower_MainFunction(void);
 /************************************************************************
  * @brief           初始化整车CAN，用于busoff策略恢复报文发送
  * @param[in]       无  
  * @return          无
***********************************************************************/ 
//void can_re_init_emscan(void);

 /************************************************************************
  * @brief           busoff检测主函数
  * @param[in]       无  
  * @return          0：未检测到busoff  1：检测到busoff
***********************************************************************/ 
extern Result Canbusoff_MainFunction(void);
 /************************************************************************
  * @brief           busoff处理主函数
  * @param[in]       无  
  * @return          无 
***********************************************************************/ 
extern void Canbusoff_process(void);
 /************************************************************************
  * @brief           CAN通讯电压检测主函数，用于高压或低压停止CAN通讯
  * @param[in]       无  
  * @return          无 
***********************************************************************/ 
//extern void CanVoltagecheck_MainFunction(void);
 /************************************************************************
  * @brief           CAN通讯电压检测主函数，用于高压或低压停止CAN通讯
  * @param[in]       无  
  * @return          无 
***********************************************************************/ 

#endif
#endif
