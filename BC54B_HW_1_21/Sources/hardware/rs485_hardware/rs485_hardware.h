/*******************************************************************************
**                       安徽力高新能源有限公司 Copyright (c)
**                            http://www.ligoo.cn/ 
** @file	    rs485_hardware.h                                       

** @brief       1.完成RS485模块的数据类型的定义 
                2.完成RS485模块的初始化
                3.完成RS485模块的发送、接收功能函数
** @copyright		Ligoo Inc.         
** @date		    2017-03-27.
** @author      李美玲      
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


#define   RS485_RX_FRAME         1   //RS485接收帧数
#define   RS485_RX_FRAME_SIZE    74  //RS485接收帧长度
#define   RS485_TX_FRAME_SIZE    74  //RS485发送帧长度 


/************************************************
  * @enum   Rs485DataFormat
  * @brief  RS485通讯字符帧格式
  ***********************************************/
typedef enum
{
   kRs485Data8Bits,// 1个起始位，8个数据位，1个停止位
   kRs485Data9Bits // 1个起始位，9个数据位，1个停止位
}Rs485DataFormat;


/************************************************
  * @enum   Rs485ParityEnable
  * @brief  使能RS485通讯奇偶校验功能
  ***********************************************/
typedef enum
{
   kRs485ParityDisable,//禁止奇偶校验
   kRs485ParityEnable  //允许奇偶校验
}Rs485ParityEnable;


/************************************************
  * @enum   Rs485ParityType
  * @brief  RS485通讯奇偶校验方式
  ***********************************************/
typedef enum
{
   kRs485EvenParity,//偶校验
   kRs485OddParity  //奇校验
}Rs485ParityType;


/************************************************
  * @enum   Rs485Dev
  * @brief  Rs485通道号
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
  * @brief  RS485通讯的初始化配置
  ***********************************************/
typedef struct
{
    INT32U band; //波特率
    Rs485DataFormat data_format; //字符帧格式
    Rs485ParityEnable parity_enable; //奇偶校验使能与否
    Rs485ParityType parity_type; //奇偶校验方式
}Rs485Param;


/************************************************
  * @func   Rs485ReceiveHandler
  * @brief  RS485接收回调函数
  ***********************************************/
typedef int (*Rs485ReceiveHandler)(Rs485Dev dev, char chr, void* user_data); 


/************************************************
  * @func   Rs485TransmitHandler
  * @brief  RS485发送回调函数
  ***********************************************/
typedef void (*Rs485TransmitHandler)(Rs485Dev dev);


/************************************************
  * @struct   Rs485Context
  * @brief  RS485通讯信息
  ***********************************************/
typedef struct _Rs485Context
{
    Rs485Dev dev;  // 通道号
    Rs485ReceiveHandler receive_handler;  //增加接收回调函数
    Rs485TransmitHandler transmit_handler;  //增加发送回调函数
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

extern rs485_t g_rs485_contexts[kRs485MaxDev];   //存储RS485设备信息
extern Rs485Context g_rs485_default_handlers[kRs485MaxDev];  //存储RS485通讯信息

#pragma DATA_SEG DEFAULT 




#define setReg(reg, val)       (reg = (word)(val))   //配置寄存器
#define getReg(reg)            (reg)                 //读取寄存器

#if 1

/************************************************************************
  * @brief           RS485通讯寄存器配置
  * @param[in]       DEV：设备编号
  * @return          无
***********************************************************************/
#define RS485_INIT_PARAM(DEV)  \
do{ \
    SCI##DEV##BDH = (INT8U)(BR >> 8) & 0xff; \
    SCI##DEV##BDL = (INT8U)BR & 0xff; \
    SCI##DEV##CR1 = (rs485_param->data_format<<4)|(rs485_param->parity_enable<<1) | (rs485_param->parity_type); /*设置字符帧格式、奇偶校验功能及方式*/  \
    SCI##DEV##CR2 |= (SCI##DEV##CR2_TE_MASK | SCI##DEV##CR2_RE_MASK | SCI##DEV##CR2_RIE_MASK); \
}while(0)


/************************************************************************
  * @brief           RS485发送字节数据
  * @param[in]       DEV：设备编号
  * @return          无
***********************************************************************/
#define RS485_SEND_HARDWARE(DEV)  \
do{ \
    (void)SCI##DEV##SR1;  /*清零中断请求标志*/   \
    SCI##DEV##DRL = (INT8U)chr;  /*数据存入发送数据寄存器*/   \
    SCI##DEV##CR2_TIE = 1;  /*使能发送中断*/   \
    SCI##DEV##CR2_TE = 1;   /*使能发送*/    \
}while(0)
  
  
/************************************************************************
  * @brief           RS485接收字节数据
  * @param[in]       DEV：设备编号
  * @param[in]       rxch：接收字符
  * @return          无
***********************************************************************/
#define RS485_RECV_HARDWARE(DEV, rxch){ rxch = SCI##DEV##DRL; /*读取接收数据寄存器内容*/ }


/************************************************************************
  * @brief           禁止发送
  * @param[in]       DEV：设备编号
  * @return          无
***********************************************************************/
#define RS485_DISABLE_SENDING(DEV) \
do{ \
    SCI##DEV##CR2_TIE = 0;  /* Disable transmit interrupt */   \
    SCI##DEV##CR2_TE = 0;   /* Disable transmit */   \
}while(0)


/************************************************************************
  * @brief           通过通道号选择功能函数
  * @param[in]       FUN：RS485功能函数
  * @return          无
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
  * @brief           RS485中断服务处理程序
  * @param[in]       DEV：设备编号
  * @return          无
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
  * @brief           RS485通讯硬件初始化
  * @param[in]       dev Rs485设备号
  * @param[in]       rs485_param Rs485Param结构体指针
  * @param[in]       func Rs485接收回调函数
  * @param[in]       user_data Rs485用户数据
  * @return          Rs485Context结构体指针
***********************************************************************/
rs485_t Rs485_Hardware_Init(Rs485Dev dev, Rs485Param *rs485_param , Rs485ReceiveHandler func, void* user_data);


/************************************************************************
  * @brief           解除一个RS485任务
  * @param[in]       context Rs485Context结构体
  * @return          0：成功 1:失败
***********************************************************************/
Result rs485_uninit(rs485_t context);


/************************************************************************
  * @brief           RS485端口0中断函数
  * @param[in]       无
  * @return          无
***********************************************************************/
void SCI0_ISR_Handler(void);

/************************************************************************
  * @brief           RS485端口1中断函数
  * @param[in]       无
  * @return          无
***********************************************************************/
void SCI1_ISR_Handler(void);


/************************************************************************
  * @brief           RS485端口3中断函数
  * @param[in]       无
  * @return          无
***********************************************************************/
void SCI3_ISR_Handler(void);


/************************************************************************
  * @brief           RS485接收中断处理函数
  * @param[in]       dev Rs485设备号
  * @param[in]       chr Rs485接收字符
  * @return          无
***********************************************************************/
void rs485_interrupt_rx(Rs485Dev dev, char chr);


/************************************************************************
  * @brief           RS485发送中断处理函数
  * @param[in]       dev Rs485设备号
  * @return          无
***********************************************************************/
void rs485_interrupt_tx(Rs485Dev dev);


/************************************************************************
  * @brief           RS485发送函数
  * @param[in]       context Rs485Context结构体指针
  * @param[in]       chr RS485发送字符
  * @return          0：成功，1：失败。
***********************************************************************/
Result rs485_send(rs485_t context, char chr);


/************************************************************************
  * @brief           设置RS485接收函数
  * @param[in]       context Rs485Context结构体指针
  * @param[in]       func Rs485Context接收函数
  * @param[in]       user_data 用户数据
  * @return          0：成功，1：失败。
***********************************************************************/
Result rs485_set_recv_handler(rs485_t context, Rs485ReceiveHandler func, void* user_data);


/************************************************************************
  * @brief           获取Rs485Context结构体指针
  * @param[in]       dev Rs485设备号
  * @return          Rs485Context结构体指针
***********************************************************************/
rs485_t rs485_get_context(Rs485Dev dev);


/************************************************************************
  * @brief           获取RS485波特率
  * @param[in]       bps_index 波特率索引号
  * @return          RS485波特率
***********************************************************************/
INT32U rs485_get_bps(INT8U bps_index);



#endif
