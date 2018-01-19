/*******************************************************************************
**                       安徽力高新能源有限公司 Copyright (c)
**                            http://www.ligoo.cn/ 
** @file	    can_intermediate.c                                       

** @brief       1.完成CAN模块中间层的数据收发 
                2.完成CAN模块中间层的数据缓冲区的处理
                3.完成CAN模块中间层的其他相关功能函数        
                
** @copyright   	Ligoo Inc.         
** @date		    2017-04-25.
** @author            
*******************************************************************************/ 

#include "can_intermediate.h"
#include "bms_job.h"


#pragma MESSAGE DISABLE C1420 // Result of function-call is ignored
#pragma MESSAGE DISABLE C1860 // Pointer conversion: possible loss of data
#pragma MESSAGE DISABLE C5703 // Parameter is not used
#pragma MESSAGE DISABLE C12056// SP debug info incorrect because of optimization or inline assembler

#if BMS_SUPPORT_CAN

#pragma DATA_SEG __RPAGE_SEG PAGED_RAM
static INT32U can_dev1_heart_beat_tick;
static INT32U can_dev2_heart_beat_tick;
static INT32U can_dev3_heart_beat_tick;
static INT32U can_dev4_heart_beat_tick;
static INT32U can_dev5_heart_beat_tick;
static INT8U can_channel_used_flag = 0;

CanDriveInfo re_CanDriveInfo;

#pragma DATA_SEG DEFAULT


 CanMessage g_can_buffer_message[kCanDevMaxCount];

 CanContext* g_can_contexts[kCanDevMaxCount] = { NULL };


/************************************************************************
  * @brief           轮巡发接收CAN报文句柄函数
  * @param[in]       dev CAN设备   
  * @return          0：成功  1：失败
***********************************************************************/ 
Result  Poll_Rx_Can  (CanDev dev)
  {	
     if(dev >= kCanDevMaxCount) return RES_INVALID_PARAM;   
     
     can_update_re_init_heart_beat_tick(dev);
    return  RES_OK;
  }
  
void can_re_init_heart_beat(void* data);  


/************************************************************************
  * @brief           通过设备号重新初始化CAN设备
  * @param[in]       dev CAN设备   
  * @return          0：成功  1：失败
***********************************************************************/ 
Result can_re_init_with_dev(CanDev dev)
{
    can_t context;
   
    context = can_get_context(dev);
    if(context == NULL)return RES_OK;
    return can_re_init_with_context(context);
   
}

/************************************************************************
  * @brief           获取CAN context  数组
  * @param[in]       dev CAN设备   
  * @return          context can报文结构体 
***********************************************************************/ 
can_t can_get_context(CanDev dev)
{ 
    if (dev >= kCanDevMaxCount) return NULL;
      
      return g_can_contexts[dev];
}

 
/************************************************************************
  * @brief           通过 context重新初始化CAN设备
  * @param[in]       context can报文结构体   
  * @return          0：成功  1：失败
***********************************************************************/ 
Result can_re_init_with_context(can_t  context)
{                                     
//    INT8U can_tx_flag;
//    Result res;

    CanDriveInfo* _PAGED re_pCanDriveInfo;
 
   /** check the parameter */
    if (!context) return RES_ERR;
    if (context->dev < 0 || context->dev >= kCanDevMaxCount) return RES_ERR;
                
 
   
     re_CanDriveInfo.dev=context->dev;
     re_CanDriveInfo.receive_0_id= context->info.receive_id;
     re_CanDriveInfo.receive_1_id= context->info.receive_1_id;
     re_CanDriveInfo.mask_0_id= context->info.mask_id;
     re_CanDriveInfo.mask_1_id= context->info.mask_1_id;    
     re_CanDriveInfo.bps=context->info.bps;
     re_CanDriveInfo.filtemode=context->info.filtemode;
     
     re_CanDriveInfo.filtemode=CanFilte32;   //WLLLLLLLLLLLLLLLLLLLLLLLL
     
   
      re_pCanDriveInfo=&re_CanDriveInfo;
     
     Can_Hardware_Init(re_pCanDriveInfo);
}

 
/************************************************************************
  * @brief           CAN 报文初始化 
  * @param[in]       CanInfo CAN信息属性（二级接口）  
  * @return          can_t 结构体（一级接口） 
***********************************************************************/ 
can_t can_init(CanInfo* _PAGED can_info)
{
   INT32U Id_AR0;
   INT32U Id_AR1;
   INT32U Id_MR0;
   INT32U Id_MR1;
  
    CanContext* context;
    Result res;
    CanDriveInfo  pCanDriveInfo;
    /** check the parameter */
    if (!can_info) return NULL;
    if (can_info->dev < 0 || can_info->dev >= kCanDevMaxCount) return NULL;

    /** create the context */
    context = (CanContext*) MALLOC(sizeof(CanContext));
    if (context == NULL) return NULL;

    /** set the context */
    memset(context, 0, sizeof(CanContext));
    context->dev = can_info->dev;
    //context->info = *can_info;
    safe_memcpy((PINT8U)&(context->info), (PINT8U)can_info, sizeof(CanInfo));
    if (context->info.receive_callback == NULL) // enable sync receiving mode
    {
        context->read_buffer_event = OSSemCreate(0);
    }
#if( SWITCH_ON == UDS_SWITCH_CONFIG) 
    if(context->dev == kCanDev2)
    {
       context->read_buffer_event = OSSemCreate(0);
    }
#endif
    g_can_contexts[can_info->dev] = context;
    /** get the can receive/mask id */
    if (is_extend_mode(context))
    {
        context->receive_id = can_id_from_extend_id(context->info.receive_id);
        context->mask_id = can_id_from_extend_id(context->info.mask_id);
        if (context->info.filtemode<CanFilteClose)
        {
            switch (context->info.filtemode) 
            { 
              case CanFilte32:
              Id_AR0 = FilteId_extend_32to_MSCAN(context->info.receive_id);
              Id_MR0 = FilteId_extend_32to_MSCAN(context->info.mask_id);
              break; 
              case CanFilte16:
              Id_AR0 = FilteId_extend_2_16to_MSCAN(context->info.receive_id);
              Id_MR0 = FilteId_extend_2_16to_MSCAN(context->info.mask_id);
              break; 
              case CanFilte8:
              Id_AR0 = context->info.receive_id;
              Id_MR0 = context->info.mask_id;  
              break; 
              default:
              break;                      
            }
        }
    }
   else
    {
        context->receive_id = can_id_from_std_id(context->info.receive_id);
        context->mask_id = can_id_from_std_id(context->info.mask_id);
       if (context->info.filtemode<CanFilteClose)
        {
            switch (context->info.filtemode) 
            { 
              case CanFilte32:
              Id_AR0 = FilteId_std_32to_MSCAN(context->info.receive_id);
              Id_MR0 = FilteId_std_32to_MSCAN(context->info.mask_id);
              break; 
              case CanFilte16:
              Id_AR0 = FilteId_std_2_16to_MSCAN (context->info.receive_id);
              Id_MR0 = FilteId_std_2_16to_MSCAN (context->info.mask_id);
              break; 
              case CanFilte8:
              Id_AR0 = context->info.receive_id;
              Id_MR0 = context->info.mask_id;  
              break;  
              default:
              break;                     
            }
        }   
     }

    if (is_extend_mode_1(context))
    {
        context->receive_1_id = can_id_from_extend_id(context->info.receive_1_id);
        context->mask_1_id = can_id_from_extend_id(context->info.mask_1_id);
       if (context->info.filtemode<CanFilteClose)
        {
            switch (context->info.filtemode) 
            { 
              case CanFilte32:
              Id_AR1 = FilteId_extend_32to_MSCAN(context->info.receive_1_id);
              Id_MR1 = FilteId_extend_32to_MSCAN(context->info.mask_1_id);
              break;
              case CanFilte16:
              Id_AR1 = FilteId_extend_2_16to_MSCAN(context->info.receive_1_id);
              Id_MR1 = FilteId_extend_2_16to_MSCAN(context->info.mask_1_id);
              break;
              case CanFilte8:
              Id_AR1 = context->info.receive_1_id;
              Id_MR1 = context->info.mask_1_id;  
              break;
              default:
              break;                      
            }
        }
        
    }
    else
    {
        context->receive_1_id = can_id_from_std_id(context->info.receive_1_id);
        context->mask_1_id = can_id_from_std_id(context->info.mask_1_id);
      if (context->info.filtemode<CanFilteClose)
        {
            switch (context->info.filtemode) 
            { 
              case CanFilte32:
              Id_AR1 = FilteId_std_32to_MSCAN(context->info.receive_1_id);
              Id_MR1 = FilteId_std_32to_MSCAN(context->info.mask_1_id);
              break;
              case CanFilte16:
              Id_AR1 = FilteId_std_2_16to_MSCAN (context->info.receive_1_id);
              Id_MR1 = FilteId_std_2_16to_MSCAN (context->info.mask_1_id);
              break;
              case CanFilte8:
              Id_AR1 = context->info.receive_1_id;
              Id_MR1 = context->info.mask_1_id;   
              break; 
              default:
              break;                     
            }
        }  
    }

    res = can_check_buffers(context);
    if (res != RES_OK)
    {
      FREE(context);
      return NULL;
    } 
                                                   
   
     pCanDriveInfo.dev=can_info->dev;
     pCanDriveInfo.receive_0_id= Id_AR0;//context->info.receive_id;
     pCanDriveInfo.receive_1_id= Id_AR1;//context->info.receive_1_id;
     pCanDriveInfo.mask_0_id= Id_MR0;//context->info.mask_id;
     pCanDriveInfo.mask_1_id= Id_MR1;//context->info.mask_1_id;    
     pCanDriveInfo.bps= context->info.bps;
     pCanDriveInfo.filtemode= context->info.filtemode;

     Can_Hardware_Init(&pCanDriveInfo);
    /** Configuration CAN device*/
   //wlllllllllllllllll can_configurate_device(context);
     
    return context;
}

/************************************************************************
  * @brief           根据索引设置CAN波特率 
  * @param[in]       bps_index 波特率索引
  * @return          波特率 
***********************************************************************/
CanBps can_get_bps(INT8U bps_index)
{
    CanBps bps;
    
    switch(bps_index)
    {
      case 1: bps = kCan50kBps;break;
	    case 2: bps = kCan100kBps;break;
	    case 3: bps = kCan125kBps;break;
	    case 4: bps = kCan250kBps;break;
	    case 5: bps = kCan500kBps;break;
	    case 6: bps = kCan800kBps;break;
	    case 7: bps = kCan1000kBps;break;
	    default: bps = kCan250kBps;break;
    }
    return bps;
}


/************************************************************************
  * @brief           重新初始化各CAN口的心跳包
                     can_devN_heart_beat_tick 每接收到一帧报文会更新一次
  * @param[in]       无   
  * @return          无
***********************************************************************/ 
void can_re_init_init(void)
{
    INT32U tick=get_tick_count();
    
    can_dev1_heart_beat_tick = tick;
    can_dev2_heart_beat_tick = tick;
    can_dev3_heart_beat_tick = tick;
    
    job_schedule(MAIN_JOB_GROUP, CAN_RE_INIT_JOB_PERIODIC, can_re_init_heart_beat, NULL);  
}

/************************************************************************
  * @brief           检测各CAN口的心跳包是否正常用于检测各CAN口是否工作正常
                     can_devN_heart_beat_tick 每接收到一帧报文会更新一次
  * @param[in]       无   
  * @return          无
***********************************************************************/
void can_re_init_heart_beat(void* data) 
{
    INT32U tick=get_tick_count();
    
    if(get_interval_by_tick(can_dev1_heart_beat_tick, tick) > CAN_DEV1_RE_INIT_DELAY)
    {
        can_dev1_heart_beat_tick = tick;
        can_re_init_with_dev(kCanDev0);
    }
    
    if(get_interval_by_tick(can_dev2_heart_beat_tick, tick) > CAN_DEV2_RE_INIT_DELAY)
    {
        can_dev2_heart_beat_tick = tick;
        can_re_init_with_dev(kCanDev1);
    }
    
    if(get_interval_by_tick(can_dev3_heart_beat_tick, tick) > CAN_DEV3_RE_INIT_DELAY)
    {
        can_dev3_heart_beat_tick = tick;
        can_re_init_with_dev(kCanDev2);
    }
    
    if(get_interval_by_tick(can_dev4_heart_beat_tick, tick) > CAN_DEV4_RE_INIT_DELAY)
    {
        can_dev4_heart_beat_tick = tick;
        can_re_init_with_dev(kCanDev3);
    }
    
    if(get_interval_by_tick(can_dev5_heart_beat_tick, tick) > CAN_DEV5_RE_INIT_DELAY)
    {
        can_dev5_heart_beat_tick = tick;
        can_re_init_with_dev(kCanDev4);
    }
}


/************************************************************************
  * @brief           更新各CAN口的心跳包是否正常用于检测各CAN口是否工作正常
                     can_devN_heart_beat_tick 每接收到一帧报文会更新一次
  * @param[in]       dev  CAN 设备口号   
  * @return          0：成功  1：失败
***********************************************************************/ 
Result can_update_re_init_heart_beat_tick(CanDev dev)
{
    if (dev >= kCanDevMaxCount) return RES_ERR;
    
    switch(dev)
    {
        case kCanDev0:
            can_dev1_heart_beat_tick = get_tick_count();
            break;
        case kCanDev1:
            can_dev2_heart_beat_tick = get_tick_count();
            break;
        case kCanDev2:
            can_dev3_heart_beat_tick = get_tick_count();
            break;
        case kCanDev3:
            can_dev3_heart_beat_tick = get_tick_count();
            break;
        case kCanDev4:
            can_dev4_heart_beat_tick = get_tick_count();
            break;
    }
    return RES_OK;
}

/************************************************************************
  * @brief           检测接收环形缓冲区的状态
  * @param[in]       can_t 结构体（一级接口）对应初始化结构体   
  * @return          0：成功  1：失败
***********************************************************************/ 
Result can_check_buffers(can_t context)
{
    INT8U buffer_count = context->info.buffer_count;

    /** check the buffer */
    if (context->info.buffers) return RES_OK;
    if (buffer_count == 0) buffer_count = CAN_DEFAULT_BUFFER_MESSAGE_COUNT;

    context->info.buffers = MALLOC(sizeof(CanMessage) * buffer_count);
    if(context->info.buffers == NULL) return RES_ERROR;///////////////////////////////?

    context->info.buffer_count = buffer_count;

    context->buffers_is_allocated_by_self = TRUE;

    return RES_OK;
}
/*wlllllllllllllllllllll
Result can_send(can_t context, CanMessage* _PAGED msg)
{
    INT8U index; // number for read message
    INT8U tbsel = 0; // symbol for CAN0TBSEL
    INT8U* address;
    OS_CPU_SR cpu_sr = 0;
    
    if (!can_get_TFLG(context->dev)) return RES_ERR;                      // Is Transmit Buffer full??
    
    OS_ENTER_CRITICAL();
    //int cnt=0;
    can_set_TBSEL(context->dev, can_get_TFLG(context->dev)); // Select lowest empty buffer

    tbsel = can_get_TBSEL(context->dev); // Backup selected buffer

    // Load Id to IDR Registers
    *((INT32U *) ((INT32U) (can_get_TXIDR0_address(context->dev)))) = msg->id.value;
    if (msg->len > CAN_DATA_MAX_LEN) msg->len = CAN_DATA_MAX_LEN;

    address = can_get_TXDSR0_address(context->dev);
    for (index = 0; index < msg->len; index++)
    {
        *(address + index) = msg->data[index]; // Load data to Tx buffer  Data Segment Registers(ONLY 8 BYTES?)
    }

    can_set_TXDLR(context->dev, msg->len); // Set Data Length Code

    can_set_TXTBPR(context->dev, 0x00); // Set Priority
    can_set_TFLG(context->dev, tbsel); // Start transmission
    // while ((CAN0TFLG & tbsel) != tbsel);
    //if(++cnt>10000) return 1;                   //  Wait for Transmission completion
    // return 0;
    OS_EXIT_CRITICAL();
    
    return RES_OK;
}
 wlllllllllllllllllll*/
 
/************************************************************************
  * @brief           通过CAN设备口启动CAN发送
  * @param[in]       dev CAN设备口 
                     msg 发送报文
  * @return          0：成功  1：失败
***********************************************************************/ 
Result dev_can_send(CanDev dev, CanMessage* _PAGED msg)
{
    if(msg == NULL) return RES_INVALID_HANLDE;
    if(dev >= kCanDevMaxCount) return RES_INVALID_PARAM;
    if(g_can_contexts[dev] == NULL) return RES_INVALID_HANLDE;
    
    //wlllllllll  return can_send(g_can_contexts[dev], msg);
    return Can_Hardware_Tx (g_can_contexts[dev]->dev, msg);
}

/************************************************************************
  * @brief      从接收缓冲区里读取数据     
  * @param[in]  can_t 结构体（一级接口）
                msg 读出的数据存入的结构体，用于返回读取值
                timeout 系统事件超时设置 该事件信号量在底层接收中断函数触发时创建    
  * @return          0：成功  1：失败
***********************************************************************/  
Result can_receive(can_t context, CanMessage* _PAGED msg, INT16U timeout)
{
    INT8U err;
    INT16U res = RES_ERROR;
    OS_CPU_SR cpu_sr = 0;
    if (context == NULL) return ERR_INVALID_ARG;
    if (context->read_buffer_event == NULL) return RES_ERROR;

    OSSemPend(context->read_buffer_event, timeout, &err);
    if (err == OS_ERR_TIMEOUT) return ERR_TIMEOUT;
    if (err != OS_ERR_NONE) return RES_ERROR;

    OS_ENTER_CRITICAL();
    if (context->read_buffer_count > 0)
    {
        --context->read_buffer_count;
        if (msg) *msg = context->info.buffers[context->read_buffer_read_pos];
        context->read_buffer_read_pos = (INT8U)((context->read_buffer_read_pos + 1) % (context->info.buffer_count));
        res = RES_OK;
    }
    OS_EXIT_CRITICAL();

    return res;
}

/************************************************************************
  * @brief           CAN 接口的析构函数  释放资源
  * @param[in]       can_t 结构体（一级接口）
  * @return          0：成功  1：失败
***********************************************************************/ 
Result can_uninit(can_t context)
{
    INT8U err;
    if (context == NULL) return RES_OK;

    if (context->buffers_is_allocated_by_self && context->info.buffers)
    {
        FREE(context->info.buffers);
        context->info.buffers = NULL;

        context->buffers_is_allocated_by_self = FALSE;
    }

    if (context->read_buffer_event)
    {
        OSSemDel(context->read_buffer_event, 0, &err);
        context->read_buffer_event = NULL;
    }

    FREE(context);

    return RES_OK;
}

/************************************************************************
  * @brief           增加接收ID 修改屏蔽码
  * @param[in]       can_info 结构体（二级接口） 
                     id  增加id码
  * @return          0：成功  1：失败
***********************************************************************/
Result can_add_receive_id(CanInfo* _PAGED can_info, INT32U id)
{
    INT32U mask_id;
    
    if (can_info == NULL) return RES_ERR;
    
    mask_id = id ^ can_info->receive_id;
    can_info->mask_id |= mask_id;
    
    return RES_OK;
}

/***********************************************************************
  * @brief           CAN0口中断入口函数
  * @param[in]       无
  * @return          无
***********************************************************************/
void CAN0_RX_ISR_Handler(void)
{
    can_rx_isr_handler(g_can_contexts[kCanDev0]);
    can_update_re_init_heart_beat_tick(kCanDev0);
}

/***********************************************************************
  * @brief           CAN1口中断入口函数
  * @param[in]       无
  * @return          无
***********************************************************************/
void CAN1_RX_ISR_Handler(void)
{
    can_rx_isr_handler(g_can_contexts[kCanDev1]);
    can_update_re_init_heart_beat_tick(kCanDev1);
}
/***********************************************************************
  * @brief           CAN2口中断入口函数
  * @param[in]       无
  * @return          无
***********************************************************************/
void CAN2_RX_ISR_Handler(void)
{
  can_rx_isr_handler(g_can_contexts[kCanDev2]);
  can_update_re_init_heart_beat_tick(kCanDev2);
}
/***********************************************************************
  * @brief           CAN3口中断入口函数
  * @param[in]       无
  * @return          无
***********************************************************************/
void CAN3_RX_ISR_Handler(void)
{
  can_rx_isr_handler(g_can_contexts[kCanDev3]);
  can_update_re_init_heart_beat_tick(kCanDev3);
}
/***********************************************************************
  * @brief           CAN4口中断入口函数
  * @param[in]       无
  * @return          无
***********************************************************************/
void CAN4_RX_ISR_Handler(void)
{
  can_rx_isr_handler(g_can_contexts[kCanDev4]);
  can_update_re_init_heart_beat_tick(kCanDev4);
}

/***********************************************************************
  * @brief           中断接收处理函数
  * @param[in]       can_t 结构体（一级接口）
  * @return          无
***********************************************************************/
void can_rx_isr_handler(can_t context)
{
    OS_CPU_SR cpu_sr = 0;

    if (context->info.receive_callback != NULL)
    {
        //wlllllllllllllll  can_rx_receive(context, &g_can_buffer_message[context->info.dev]);
      #if( SWITCH_ON == UDS_SWITCH_CONFIG)
       if(RES_TRUE == Can_Hardware_Rx(context->info.dev, &g_can_buffer_message[context->info.dev]))
        {
      #else 
        Can_Hardware_Rx(context->info.dev, &g_can_buffer_message[context->info.dev]); //wlllllllllllllll
      #endif        
        context->info.receive_callback(context, &g_can_buffer_message[context->info.dev],
                context->info.receive_userdata);
      #if( SWITCH_ON == UDS_SWITCH_CONFIG)
        }
      #endif
    }
    else if (context->info.buffers)
    {
        OS_ENTER_CRITICAL();
        
        //wlllllllllllllll can_rx_receive(context, &(context->info.buffers[context->read_buffer_write_pos]));
       #if( SWITCH_ON == UDS_SWITCH_CONFIG)
        if(RES_TRUE == Can_Hardware_Rx(context->info.dev, &(context->info.buffers[context->read_buffer_write_pos])))
         {
       #else
         Can_Hardware_Rx(context->info.dev, &(context->info.buffers[context->read_buffer_write_pos])); //wlllllllllllllll
       #endif  
        context->read_buffer_write_pos = (INT8U)((1 + context->read_buffer_write_pos) % (context->info.buffer_count));
        if (context->read_buffer_count < context->info.buffer_count)
        {
            ++context->read_buffer_count;
            OS_EXIT_CRITICAL();
            OSSemPost(context->read_buffer_event);
        }
        else
        {
            context->read_buffer_read_pos = (INT8U)((context->read_buffer_read_pos + 1) % (context->info.buffer_count));
            OS_EXIT_CRITICAL();
            DEBUG("can", "drop a frame because the buffers are full.");
        }
   #if( SWITCH_ON == UDS_SWITCH_CONFIG)
        }
   #endif
    }
}
/*wlllllllllllllllllll
void can_rx_receive(can_t context, CanMessage* _PAGED message)
{
    // NOTE: The routine should include the following actions to obtain
    //       correct functionality of the hardware.
    //
    //      The ISR is invoked by RXF flag. The RXF flag is cleared
    //      if a "1" is written to the flag in CAN0RFLG register.
    //      Example: CAN0RFLG = CAN0RFLG_RXF_MASK;
    if (can_get_RFLG_RXF(context->dev) && message) //接收缓冲区满
    {
        message->id.bytes[0] = can_get_RXIDR0(context->dev);
        message->id.bytes[1] = can_get_RXIDR1(context->dev);
        if (is_extend_mode(context))
        {
            message->id.bytes[2] = can_get_RXIDR2(context->dev);
            message->id.bytes[3] = can_get_RXIDR3(context->dev);
        }
        message->data[0] = can_get_RXDSR0(context->dev);
        message->data[1] = can_get_RXDSR1(context->dev);
        message->data[2] = can_get_RXDSR2(context->dev);
        message->data[3] = can_get_RXDSR3(context->dev);
        message->data[4] = can_get_RXDSR4(context->dev);
        message->data[5] = can_get_RXDSR5(context->dev);
        message->data[6] = can_get_RXDSR6(context->dev);
        message->data[7] = can_get_RXDSR7(context->dev);
        message->len = can_get_RXDLR(context->dev) & 0x0F;
        if(message->len > CAN_DATA_MAX_LEN) message->len = CAN_DATA_MAX_LEN;
    }
    can_set_RFLG(context->dev, 1);
}
wlllllllllllllll*/


/***********************************************************************
  * @brief           将CAN的标识符转换成MSCAN格式
  * @param[in]       id 输入 
  * @return          返回转换后的ID
***********************************************************************/
INT32U can_id_from_extend_id(INT32U id)
{
    INT32U temp_id = 0;
    temp_id = id << 1;
    temp_id = ((temp_id & 0x3ff80000) << 2) | (temp_id & 0x000fffff | 0x00180000);
    return temp_id;
}

/***********************************************************************
  * @brief           将MSCAN的标识符转换成CAN格式
  * @param[in]       id 输入 
  * @return          返回转换后的ID
***********************************************************************/
INT32U can_id_to_extend_id(INT32U id)
{
    INT32U temp_id = 0;
    temp_id = ((id & 0x0007FFFe) >> 1) | ((id & 0xFFE00000) >> 3);
    return temp_id;
}

/***********************************************************************
  * @brief           将CAN的标识符转换成MSCAN格式
  * @param[in]       id 输入 
  * @return          返回转换后的ID
***********************************************************************/
INT32U can_id_from_std_id(INT32U id)
{
    INT32U temp_id = 0;

    temp_id = (id << 21) & 0xFFE00000;
    return temp_id;
}

/***********************************************************************
  * @brief           将MSCAN的标识符转换成CAN格式
  * @param[in]       id 输入 
  * @return          返回转换后的ID
***********************************************************************/
INT32U can_id_to_std_id(INT32U id)
{
    INT32U temp_id = 0;
    temp_id = (id >> 21) & 0x000007FF;
    return temp_id;
}

/***********************************************************************
  * @brief           判断是否扩展帧
  * @param[in]       can_t 结构体（一级接口）
  * @return          返回模式状态
***********************************************************************/

int is_extend_mode(can_t context)
{
    return context->info.mode & kCanExtendMode;
}

int is_extend_mode_1(can_t context)
{
    return context->info.mode_1 & kCanExtendMode;
}

/***********************************************************************
  * @brief           判断CAN通道口是否可用
  * @param[in]       channel 设备号
  * @return          1成功 0失败
***********************************************************************/
INT8U can_channel_is_valid(CanDev channel)
{
    INT8U res = TRUE;
    
    if(channel >= kCanDevMaxCount) res = FALSE;
    if(can_channel_used_flag & (1 << channel)) res = FALSE;
    can_channel_used_flag |= (1 << channel);
    
    return res;
}

 Result can_send(can_t context, CanMessage* _PAGED msg)
 {
  
  return Can_Hardware_Tx(context->dev,msg);
  
}

#endif

