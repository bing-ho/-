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
  
static INT8U    Canload_Up;                    //CAN 总线负载溢出计数器
static INT8U    Canload_Down; 
Can_Stau g_can_stau[kCanDevMaxCount];         // 存放总线状态数组

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
typedef Result (*P_Rx_Handler)(CanDev dev);   //定义一个函数指针 类型 用于底层在查询方式下调用CAN接收函数

/************************************************************************
  * @brief           对CPU寄存器读写操作
  * @param[in]       寄存器名
  * @return          无
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
  * @brief           函数宏 设置波特率
  * @param[in]       candr_info
  * @return          无
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
  * @brief           在32位滤波器模式下，将扩展帧滤波ID转换到MSCAN格式，
                     用于直接填写对应寄存器。
  * @param[in]       id 扩展帧滤波ID
  * @return          MSCAN格式的滤波ID 
***********************************************************************/
INT32U FilteId_extend_32to_MSCAN (INT32U id)
{	
  INT32U temp_id = 0;
  temp_id = id << 1;
  temp_id = ((temp_id & 0x3ff80000) << 2) | (temp_id & 0x000fffff | 0x00180000);
  return temp_id;
}

/************************************************************************
  * @brief           在32位滤波器模式下，标准帧滤波ID转换到MSCAN格式，
                     用于直接填写对应寄存器。
  * @param[in]       id 标准帧滤波ID
  * @return          MSCAN格式的滤波ID 
***********************************************************************/
INT32U FilteId_std_32to_MSCAN (INT32U id)
{	
  INT32U temp_id = 0;
  temp_id = (id << 21) & 0xFFE00000;
  return temp_id;
}
/************************************************************************
  * @brief           在2个16位滤波器模式下，扩展帧滤波ID转换到MSCAN格式 
                     用于直接填写对应寄存器。
                     注意：该种模式只能滤波高14位ID
  * @param[in]       id 扩展帧滤波ID
  * @return          MSCAN格式的滤波ID 
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
  * @brief           在2个16位滤波器模式下，标准帧滤波ID转换到MSCAN格式 
                     用于直接填写对应寄存器。
  * @param[in]       id 标准帧滤波ID
  * @return          MSCAN格式的滤波ID 
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
  * @brief           设置并使能中断源
  * @param[in]       dev CAN设备号
  * @param[in]       context 需要配置的中断源  
  * @return          无
***********************************************************************/
void Can_EnableInterrupts(CanDev dev,INT8U context)
{ 
  can_set_RIER(dev, context);
}
/************************************************************************
  * @brief           取消并关闭中断源
  * @param[in]       dev CAN设备号
  * @param[in]       context 需要配置的中断源  
  * @return          无
***********************************************************************/
void Can_DisableInterrupts(CanDev dev,INT8U context)
{ 
  can_set_RIER(dev, ~context); 
}


/************************************************************************
  * @brief           CAN模块硬件初始化函数 
  * @param[in]       candr_info CAN模块配置属性
  * @return          1：CAN_ERROR   0：CAN_OK
***********************************************************************/
Result Can_Hardware_Init (p_CanDriveInfo candr_info)
{
  if(candr_info->dev >= kCanDevMaxCount) return RES_INVALID_PARAM;  
  can_set_CTL0(candr_info->dev, CAN_DEV_CONFIG_INIT_MODE);
  while (can_get_CTL1_INITAK(candr_info->dev) != 1); // Wait for Initialization Mode acknowledge INITRQ bit = 1
  
  can_set_CTL1(candr_info->dev, CAN_DEV_CONFIG_MODE); // Enable MSCAN module and not LoopBack Mode
  
  
  #if( SWITCH_ON == UDS_SWITCH_CONFIG)
    if(candr_info->dev == kCanDev2)   //如果为EMS_CAN 则设置busoff手动恢复
    {
       can_set_CTL1(candr_info->dev, (can_get_CTL1(candr_info->dev)|CANCTL1_BORM));

       if((can_get_MISC(candr_info->dev) & CANMISC_BOHOLD) == 1)  ///清除busoff手动恢复标志
       {
          can_set_MISC(candr_info->dev, (can_get_MISC(candr_info->dev)|CANMISC_BOHOLD));
       }
    }
  #endif
#if BMS_SUPPORT_CAN_WKUP      //硬件支持CAN唤醒，初始化需配置EN/STB引脚 
    if(candr_info->dev == kCanDev2)   //如果为EMS_CAN 则can收发器进入normal模式
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
  * @brief           取消发送报文
  * @param[in]       dev CAN设备   
  * @return          0：成功  1：失败
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
  * @brief           使 CAN设备进入休眠模式
  * @param[in]       dev CAN设备   
  * @return          0：成功  1：失败
***********************************************************************/ 
 Result Can_Sleep (CanDev dev)
{ 
   if(dev >= kCanDevMaxCount) return RES_INVALID_PARAM;
   can_set_CTL0(dev, CAN_DEV_CONFIG_SLEEP_MODE);
   while(can_get_CTL1_SLPAK(dev) != 1);
   
   return RES_OK;
}


/************************************************************************
  * @brief           使 CAN设备唤醒
  * @param[in]       dev CAN设备   
  * @return          0：成功  1：失败
***********************************************************************/  
Result Can_Wakeup (CanDev dev) 
{
 /* if(dev >= kCanDevMaxCount) return RES_INVALID_PARAM;
    can_set_CTL0(dev, 0x00);
  // while(can_get_CTL1_SLPAK(dev) != 1);
   return RES_OK;
  */ 
   	
    if(dev >= kCanDevMaxCount) return RES_INVALID_PARAM;
    if (Can_Sleep(dev))  return  RES_ERR; // 使CAN设备进入休眠模式
    
    can_set_CTL0(dev, CAN_DEV_CONFIG_INIT_MODE);// 请求进入inint模式
    while (can_get_CTL1_INITAK(dev) != 1); // 等待进入init模式
    
    can_set_CTL1(dev, CAN_DEV_CONFIG_MODE);
    
    can_set_CTL0(dev, 0x00); // CAN设备退出INIT模式
    Can_EnableInterrupts(dev,CAN_DEV_CONFIG_RECEIVE_FULL_INTERRUPT);      
    return  RES_OK;
  
}
/************************************************************************
  * @brief           使 CAN设备进入侦听模式
  * @param[in]       dev CAN设备   
  * @return          0：成功  1：失败
 ***********************************************************************/ 
Result  Can_ListenOnlyMode  (CanDev dev)
  {
    	
    if(dev >= kCanDevMaxCount) return RES_INVALID_PARAM;
    if (Can_Sleep(dev))  return  RES_ERR; // 使CAN设备进入休眠模式
    
    can_set_CTL0(dev, CAN_DEV_CONFIG_INIT_MODE);// 请求进入inint模式
    while (can_get_CTL1_INITAK(dev) != 1); // 等待进入init模式
    can_set_CTL1(dev, CAN_DEV_LISTEN_MODE);// 请求进入侦听模式
    
    can_set_CTL0(dev, 0x00); // CAN设备退出INIT模式
    
    
    return  RES_OK;
  }

/************************************************************************
  * @brief           使 CAN设备进入正常模式
  * @param[in]       dev CAN设备   
  * @return          0：成功  1：失败
***********************************************************************/  
Result Can_NomalMode (CanDev dev) 
{  	
    if(dev >= kCanDevMaxCount) return RES_INVALID_PARAM;
    if (Can_Sleep(dev))  return  RES_ERR; // 使CAN设备进入休眠模式
    
    can_set_CTL0(dev, CAN_DEV_CONFIG_INIT_MODE);// 请求进入inint模式
    while (can_get_CTL1_INITAK(dev) != 1); // 等待进入init模式
    
    can_set_CTL1(dev, CAN_DEV_CONFIG_MODE);
    
    can_set_CTL0(dev, 0x00); // CAN设备退出INIT模式
    Can_EnableInterrupts(dev,CAN_DEV_CONFIG_RECEIVE_FULL_INTERRUPT);      
    return  RES_OK;
  
}
/************************************************************************
  * @brief           将CAN设备dev 硬件层接受缓冲区数据存入message 回传给中间层 
  * @param[in]       dev CAN设备   message 保存数据并传给中间层
  * @return          无
***********************************************************************/
#if( SWITCH_ON == UDS_SWITCH_CONFIG)
Result Can_Hardware_Rx(CanDev dev, CanMessage* __far message)
#else
 void Can_Hardware_Rx (CanDev dev, CanMessage* __far message)
#endif
{
  INT32U id_temp=0;
   
    if (can_get_RFLG_RXF(dev) && message) //接收缓冲区满
    {
        message->id.bytes[0] = can_get_RXIDR0(dev);
        message->id.bytes[1] = can_get_RXIDR1(dev);

        if (can_get_RXIDR1_IDE(dev))     //接收的是扩展帧 
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
    
  #if( SWITCH_ON == UDS_SWITCH_CONFIG)   ///该部分代码用于实现UDS 28服务
    if(dev != kCanDev2)   //如果为EMS_CAN 则不对报文进行过滤
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
  * @brief           将中间层要发送的报文内容message，传递给can dev口,
                     并通过该接口传递给硬件层执行 
  * @param[in]       dev CAN设备   message 保存数据并传给中间层
  * @return          无
***********************************************************************/
Result Can_Hardware_Tx (CanDev dev, CanMessage* __far message)
{
    INT8U index; 
    INT8U tbsel = 0; 
    INT8U* address;
    OS_CPU_SR cpu_sr = 0;
    
    if(message == NULL) return RES_INVALID_HANLDE;
    if(dev >= kCanDevMaxCount) return RES_INVALID_PARAM;
    
    if (!can_get_TFLG(dev)) return RES_ERR;      // 发送缓冲区已满
     
    #if( SWITCH_ON == UDS_SWITCH_CONFIG)
    #if (STANDARD_ID == UDS_CAN_ID_TYPE) 
    if((dev != kCanDev2) ||(COM_TX_RUN == Com_TxRun) || ( can_id_to_std_id(message->id.value) == CanIfTxPduConfig[0].CanId) ) 
    #else
    if((dev != kCanDev2) ||(COM_TX_RUN == Com_TxRun) || ( can_id_to_extend_id(message->id.value) == CanIfTxPduConfig[0].CanId) )
    #endif
    {  
    #endif 
    OS_ENTER_CRITICAL();
    
    can_set_TBSEL(dev, can_get_TFLG(dev)); // 选择空缓冲区

    tbsel = can_get_TBSEL(dev); 

    // 写入ID 寄存器
    *((INT32U *) ((INT32U) (can_get_TXIDR0_address(dev)))) = message->id.value;
    
    if (message->len > CAN_DATA_MAX_LEN) message->len = CAN_DATA_MAX_LEN;

    address = can_get_TXDSR0_address(dev);
    for (index = 0; index < message->len; index++)
    {
        *(address + index) = message->data[index]; // 写入数据到发送缓冲区
    }

    can_set_TXDLR(dev, message->len); // 设置发送数据长度
    can_set_TXTBPR(dev, message->piro); // 设置优先级
    can_set_TFLG(dev, tbsel); // 开始发送

    OS_EXIT_CRITICAL();
   #if( SWITCH_ON == UDS_SWITCH_CONFIG)  
  }
#endif   
      return RES_OK;
                                    
}
 


/************************************************************************
  * @brief           状态标志轮询
  * @param[in]       dev CAN设备   
  * @return          无
***********************************************************************/ 
void  Can_Flg_Poll  (CanDev dev, P_Rx_Handler p_rx_handler )
{
  if (can_get_RIER_RXFIE(dev)==0)  //非中断方式，CAN采用轮询方式接收
    {
       if (can_get_RFLG_RXF(dev))             //接收缓冲区已满
       {      
          (void)(*p_rx_handler)(dev);
       } 
    }
      
        
  if (can_get_RIER_OVRIE(dev))         //计算并判断CAN设备是否负载溢出
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
  
   
   switch (can_get_RIER_TSTATE(dev)) // 读取TSTAT 标志位 并设定CAN设备相应状态  
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
   switch  (can_get_RIER_RSTATE(dev))// 读取RSTAT 标志位 并设定CAN设备相应状态
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
  * @brief           状态标志轮询
  * @param[in]       dev CAN设备   
  * @return          无
***********************************************************************/ 
void can_re_init_emscan(void)  //对emscan进行重新初始化，用于busoff恢复通讯
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


void can_disconnect_emscan(void)  //对emscan进入初始化模式，用于高低压停止通讯
{
   can_disconnect_with_dev(kCanDev2);
}
  #if( SWITCH_ON == UDS_SWITCH_CONFIG)
/********************************/
/*          can_busoff          */
/********************************/

static Result can_busoff_check_with_dev(CanDev dev)  //CAN busoff检查
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
 
static Result can_busoff_handler_with_dev(CanDev dev)  //CAN busoff检查
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
    	   if(can_boff_ems.no_busoff_time_counte >= CAN_BOFF_DTC_TIME_COUNTE) //连续5s没有busoff则清楚busoff故障
    	   {
    	     can_boff_ems.no_busoff_time_counte = 0;
    	     can_boff_ems.can_boff_counte = 0;
    	     can_boff_ems.canboff_dtc_state =  BUSOFF_DTC_NOK;
    	   }
    	 }
	    break;
      case CAN_BOFF :

    	  if( ++ can_boff_ems.busoff_time_counte >= CAN_BOFF_TIME_COUNTE)  ///busoff恢复时间为1000ms
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
  if(bms_get_power_state() == 0)  ///检测到所有唤醒源失效，则停止ems can
  {
      g_ems_can_stop_rel_tick = now_tick;
  
      if(ems_can_stop_flag == 0)
      {
         
          if(get_interval_by_tick(g_ems_can_stop_tick, now_tick) >= 50)
          {                     
                  //can_disconnect_emscan();
                  Uds_DTC_save();    //下电保存DTC故障码，平时保存由于eeprom操作时间长导致报文周期增大13ms，CAN周期测试不通过

                  ems_can_stop_flag = 1; 
          }
        
      }          

  }
  else    ///检测到唤醒源有效，则启动ems can
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
 










