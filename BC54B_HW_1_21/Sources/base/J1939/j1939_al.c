/*******************************************************************************
**                       安徽力高新能源有限公司 Copyright (c)
**                            http://www.ligoo.cn/ 
**文 件 名j1939.c                                       
**作    者：董丽伟
**创建日期：2012.05.25
**修改记录：
**文件说明:CAN通信标准J1939应用层驱动源文件
**版    本:V1.0
**备    注：使用方法：
**          1.完成j1939_hal层的函数驱动，使CAN设备可以正常发送接收CAN帧，调用J1939Init
**            函数初始化CAN设备，获取CAN操作句柄。若要自定义缓存，则需要将所有缓存定义
**            并将指针赋于结构体的指针。需要定义的缓存见头al文件。
**          2.J1939CanHeartBeat周期性调用，以1ms为单位。
**          3.周期性调用函数J1939CanPoll来发送和接收数据，As fast as you can!建议不要超
**            过5ms，以免发生数据丢失。
**          4.使用J1939CanSendMessage来发送任何长度的数据，当然不能超过定义的长度最大值。
**          5.使用J1939CanRecMessageWithWait来读取消息数据。
*******************************************************************************/
#ifndef J1939_AL_C_
#define J1939_AL_C_

#pragma MESSAGE DISABLE C1860
#pragma MESSAGE DISABLE C12056

#include "j1939_cfg.h"

#define J1939_AL_GLOBALS

#define J1939_GET_PRI_FROM_ID(ID)   (ID >> 26 & 0x07)

J1939CanTranPro* _PAGED J1939GetFreeSendTP(J1939CanContext* _PAGED context);

//函数名称：J1939InitWithBuff               
//功    能：J1939协议初始化
//参    数：can_info  CAN通信用的数据变量指针
//返 回 值：返回CAN操作上下文指针
//备    注：
J1939Result J1939InitWithBuff(J1939CanContext* _PAGED context, J1939CanInfo* _PAGED can_info)
{
	J1939Result res;

	if(can_info == NULL)
	    return J1939_ERR_PTR;
	if(context == NULL)
		return J1939_ERR_PTR;
	context->send_event_sem->use_flag = 0;   //正在发送TP时此值为1，表示信号量被占用
	context->receive_event_sem->use_flag = 1;//默认使用接收信号量

	(void)J1939MemCopy((INT8U* _PAGED)context->can_info, (INT8U* _PAGED)can_info, sizeof(J1939CanInfo));
	context->heart_beat = 0;
	
	context->can_info->rec_mask_group_item.flag = 1;
	context->can_info->rec_mask_group_item.start_addr = context->can_info->addr;
	context->can_info->rec_mask_group_item.stop_addr = context->can_info->addr;
    
	res = J1939_tl_with_buff_init(context);
	if(res != J1939_OK)
	{
		return res;
	}
	res = J1939_dl_with_buff_Init(context);
	if(res != J1939_OK)
	{
		return res;
	}
	res = J1939HalWithBuffInit(context);
	if(res != J1939_OK)
	{
		return res;
	}
	res = j1939_dtc_init(context);
	
	return res;
}

//函数名称：J1939UnInit               
//功    能：J1939协议反初始化
//参    数：context  CAN通信用的数据变量指针
//返 回 值：反初始化成功则返回J1939_OK，否则返回失败原因
//备    注：
J1939Result J1939UnInit(J1939CanContext* _PAGED context)
{
    if(context ==NULL)
        return J1939_ERR_PTR;
    
    return J1939_OK;
}
//函数名称：J1939CanPoll               
//功    能：J1939周期处理函数
//参    数：context  CAN设备的数据变量指针
//返 回 值：无
//备    注：此函数需要周期性的调用，以让驱动包处理CAN发送和接收的数据
void J1939CanPoll(J1939CanContext* _PAGED context)
{
    unsigned int elap;
    
    if(context == NULL)
        return;
    elap = J1939CanGetHeartBeat(context);
    J1939CanSetHeartBeat(context, 0);
    (void)J1939_tl_send_poll(context, elap);
    (void)J1939_tl_rec_poll(context, elap);
    J1939_dl_send_poll(context);
    j1939_dtc_poll(context, get_tick_count());
}

//函数名称：J1939CanHeartBeat               
//功    能：J1939心跳函数
//参    数：context  CAN设备的数据变量指针
//返 回 值：无
//备    注：此函数需要1ms的调用，作为驱动包的心跳
void J1939CanHeartBeat(J1939CanContext* _PAGED context)
{
    if(context == NULL)
        return;
    context->heart_beat++;   
}

//函数名称：J1939CanSendMessage               
//功    能：J1939发送消息函数
//参    数：context  CAN设备的数据变量指针
//          msg 待发送的消息的指针
//返 回 值：成功则返回J1939_OK，否则返回失败原因
//备    注：此函数需要周期性的调用，以让驱动包处理CAN发送和接收的数据
J1939Result J1939CanSendMessage(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg)
{
    J1939Result res = J1939_OK;
    J1939CanTranPro* _PAGED send_tp_ptr;
    OS_CPU_SR cpu_sr = 0;
    
    if(context == NULL)
        return J1939_ERR_PTR;
    if(msg == NULL)
        return J1939_ERR_MSG_PTR;
    if(msg->byte_cnt > context->send_message_buff->byte_max)
        return J1939_ERR_MSG_LEN;
    if(msg->byte_cnt < 9)
    {
        J1939CanFrame message;
        
        OS_ENTER_CRITICAL();
        message.byte_cnt = (unsigned char)msg->byte_cnt;
        //memcpy(message.data, msg->data, msg->byte_cnt);
        (void)J1939MemCopy(message.data, msg->data, msg->byte_cnt);
        message.id = msg->id;
        if(msg->type == kBroadcastType) message.id.data[2] = J1939_BROADCAST_ID;
        
        (void)J1939_dl_write_buff(context->send_frame_buff, &message);
        OS_EXIT_CRITICAL();
        
        return res;   
    }
    else
    {
        OS_ENTER_CRITICAL();
        send_tp_ptr = J1939GetFreeSendTP(context);
        if(send_tp_ptr != NULL)
        {
            context->send_event_sem->use_flag = 0;
            (void)J1939CanSendMessageCopy(context->send_message_buff, msg);
            if(context->send_message_buff->type == kBroadcastType)
                send_tp_ptr->status = STATUS_TP_BAM;
            else
                send_tp_ptr->status = STATUS_TP_RTS;
            res = J1939_OK;
        }
        else
        {
            res = J1939_ERR_TP_BUSY;   
        }
        OS_EXIT_CRITICAL();
        return res;
    }
}
//函数名称：J1939CanSendMessageWithWait               
//功    能：J1939发送消息并等待发送结果函数
//参    数：context  CAN设备的数据变量指针
//          msg 待发送的消息的指针
//          timeout 等待超时时间
//返 回 值：成功则返回J1939_TP_FINISH,否则返回失败原因
//备    注：此函数需要周期性的调用，以让驱动包处理CAN发送和接收的数据
J1939Result J1939CanSendMessageWithWait(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED msg, unsigned int timeout)
{
    J1939Result res = J1939_OK;
    unsigned char err = J1939_EVENT_NO_ERR;
    J1939CanTranPro* _PAGED send_tp_ptr;
    OS_CPU_SR cpu_sr = 0;
    
    if(context == NULL)
        return J1939_ERR_PTR;
    if(msg == NULL)
        return J1939_ERR_MSG_PTR;
    if(msg->byte_cnt > context->send_message_buff->byte_max)
        return J1939_ERR_MSG_LEN;
    if(msg->byte_cnt < 9)
    {
        J1939CanFrame message;
        
        OS_ENTER_CRITICAL();
        message.byte_cnt = (unsigned char)msg->byte_cnt;
        //memcpy(message.data, msg->data, msg->byte_cnt);
        (void)J1939MemCopy(message.data, msg->data, msg->byte_cnt);
        message.id = msg->id;
        if(msg->type == kBroadcastType) message.id.data[2] = J1939_BROADCAST_ID;
        
        res=J1939_dl_write_buff(context->send_frame_buff, &message);
        OS_EXIT_CRITICAL();
        
        return res;   
    }
    else
    {
        OS_ENTER_CRITICAL();
        send_tp_ptr = J1939GetFreeSendTP(context);
        
        if(send_tp_ptr == NULL) 
        {
            OS_EXIT_CRITICAL();
            return J1939_ERR_TP_BUSY;
        }
        
        context->send_event_sem->use_flag = 1;
        (void)J1939CanSendMessageCopy(context->send_message_buff, msg);
        if(context->send_message_buff->type == kBroadcastType)
            send_tp_ptr->status = STATUS_TP_BAM;
        else
            send_tp_ptr->status = STATUS_TP_RTS;
        OS_EXIT_CRITICAL();
        J1939PendSem(context->send_event_sem->event, timeout, &err);
        context->send_event_sem->use_flag = 0;
        //J1939SendSem(context->send_event_sem->event);
        if(err != J1939_EVENT_NO_ERR)
        {
            send_tp_ptr->status = STATUS_TP_IDLE;
            
            return J1939_ERR_SEND_TIMEOUT;   
        }
        else
        {   
            if(send_tp_ptr->status == STATUS_TP_ABORT) res = J1939_ERR_ABORT;
            else res = J1939_OK;
            
            //send_tp_ptr->status = STATUS_TP_IDLE;
        } 
    }
    return res;
}

J1939CanTranPro* _PAGED  J1939GetFreeSendTP(J1939CanContext* _PAGED context)
{
    if(context == NULL) return NULL;
    
    if(context->send_event_sem->use_flag == 0 &&
        (context->send_tp_index->status == STATUS_TP_IDLE ||
        context->send_tp_index->status == STATUS_TP_FINISH ||
        context->send_tp_index->status == STATUS_TP_ABORT))
        return context->send_tp_index;
    
    return NULL;
}

//函数名称：J1939CanSendMessageWithFinish
//功    能：J1939发送消息并等待发送完成函数
//参    数：context  CAN设备的数据变量指针
//          msg 待发送的消息的指针
//          timeout 等待超时时间
//返 回 值：成功则返回J1939_OK，否则返回失败原因
//备    注：此函数需要周期性的调用，以让驱动包处理CAN发送和接收的数据
J1939Result J1939CanSendMessageWithFinish(J1939CanContext* _PAGED context, J1939SendMessageBuff* _PAGED buff, unsigned int timeout)
{
	INT8U cnt=0;
	J1939Result result;
	
    do
	{	
	    result = J1939CanSendMessageWithWait(context, buff, timeout);
		if((result == STATUS_TP_FINISH)||(result == J1939_OK))
			return J1939_OK;
		else
			sleep(10);
	
	}while(++cnt < J1939_SEND_RETRY_MAX);
	
    return result;
}

//函数名称：J1939CanRecMessageWithWait               
//功    能：J1939接收消息函数
//参    数：context  CAN设备的数据变量指针
//          msg 接收的消息的指针
//          timeout 超时时间
//返 回 值：成功则返回J1939_OK，否则返回失败原因
//备    注：
J1939Result J1939CanRecMessageWithWait(J1939CanContext* _PAGED context, J1939RecMessage* _PAGED msg, INT16U timeout)
{
    unsigned char err = J1939_EVENT_NO_ERR;
    
    if(context == NULL)
        return J1939_ERR_PTR;
    if(msg == NULL)
        return J1939_ERR_MSG_PTR;
    
    #if J1939_OS_EVENT_EN > 0
    J1939PendSem(context->receive_event_sem->event, timeout, &err);
    if(err != J1939_EVENT_NO_ERR)
    {
        return J1939_ERR_REC_TIMEOUT;   
    }
    #endif
    
    if(context->rec_message_buff->message_cnt)
    {
        (void)J1939_dl_read_message(context->rec_message_buff, msg);
        
        return J1939_OK;
    }
    else if(context->rec_pdu_buff->frame_cnt)
    {
        J1939CanFrame frame;
        (void)J1939_dl_read_buff(context->rec_pdu_buff, &frame);
        msg->byte_cnt = frame.byte_cnt;
        //memcpy(msg->data, frame.data, msg->byte_cnt);
        (void)J1939MemCopy(msg->data, frame.data, msg->byte_cnt);
        msg->id.value = frame.id.value;
        msg->pgn = (unsigned int)frame.id.data[1] << 8;
        if(frame.id.data[1] > 239)
        {
            msg->pgn += frame.id.data[2];   
        }
        return J1939_OK;
    }
    else
    {
        return J1939_ERR_BUFF_EMPTY;
    }
}

//函数名称：J1939CanFrameCopy
//功    能：J1939帧内容复制
//参    数：dest:目标地址
//	        sour:源地址
//返 回 值：成功则返回J1939_OK，否则返回失败原因
//备    注：
J1939Result J1939CanFrameCopy(J1939CanFrame* _PAGED dest, J1939CanFrame* _PAGED source)
{
    unsigned char i;

	if((source == NULL)||(dest == NULL))
        return J1939_ERR_PTR;

    dest->byte_cnt = source->byte_cnt;
    for(i=0; i<8; i++)
    	dest->data[i] = source->data[i];
    dest->id.value = source->id.value;

    return J1939_OK;
}
//函数名称：J1939MemCopy
//功    能：J1939内存复制
//参    数：dest:目标地址
//	        sour:源地址
//          len:长度
//返 回 值：成功则返回J1939_OK，否则返回失败原因
//备    注：
J1939Result J1939MemCopy(INT8U* _PAGED dest, INT8U* _PAGED source, INT16U len)
{
    unsigned long i;

	if((source == NULL)||(dest == NULL))
        return J1939_ERR_PTR;

    for(i=0; i<len; i++)
    	dest[i] = source[i];
    
    return J1939_OK;
}
//函数名称：J1939MemSet
//功    能：J1939内存赋值
//参    数：dest:目标地址
//	        sour:源地址
//          len:长度
//返 回 值：成功则返回J1939_OK，否则返回失败原因
//备    注：
J1939Result J1939MemSet(INT8U* _PAGED dest, INT8U val, INT16U len)
{
    unsigned long i;

	if(dest == NULL)
        return J1939_ERR_PTR;

    for(i=0; i<len; i++)
    	dest[i] = val;

    return J1939_OK;
}
//函数名称：J1939CanSendMessageCopy
//功    能：J1939发送消息内容复制
//参    数：dest:目标地址
//	   sour:源地址
//返 回 值：成功则返回J1939_OK，否则返回失败原因
//备    注：
J1939Result J1939CanSendMessageCopy(J1939SendMessageBuff* _PAGED dest, J1939SendMessageBuff* _PAGED source)
{
    unsigned long i;
    
	if((source == NULL)||(dest == NULL))
        return J1939_ERR_PTR;

	dest->id.value = source->id.value;
    dest->pgn = source->pgn;
    //dest->pri = J1939_GET_PRI_FROM_ID(source->id.value);
    if(dest->byte_max >= source->byte_cnt)
	    dest->byte_cnt = source->byte_cnt;
    else
        dest->byte_cnt = dest->byte_max;
	//dest->dest_addr = source->dest_addr;
    for(i=0; i<dest->byte_cnt; i++)
    	dest->data[i] = source->data[i];
    
    dest->type = source->type;

    return J1939_OK;
}

//函数名称：J1939CanRecMessageCopy
//功    能：J1939接收消息内容复制
//参    数：dest:目标地址
//	   sour:源地址
//返 回 值：成功则返回J1939_OK，否则返回失败原因
//备    注：
J1939Result J1939CanRecMessageCopy(J1939RecMessage* _PAGED dest, J1939RecMessage* _PAGED source)
{
    unsigned long i;

	if((source == NULL)||(dest == NULL))
        return J1939_ERR_PTR;

	dest->id.value = source->id.value;
    dest->pgn = source->pgn;
    if(dest->byte_max >= source->byte_cnt)
	    dest->byte_cnt = source->byte_cnt;
    else
        dest->byte_cnt = dest->byte_max;
    for(i=0; i<dest->byte_cnt; i++)
    	dest->data[i] = source->data[i];

    return J1939_OK;
}

//函数名称：J1939CanGetHeartBeat               
//功    能：J1939获取当前心跳计时值
//参    数：context  CAN设备的数据变量指针
//返 回 值：当前的心跳数
//备    注：
unsigned int J1939CanGetHeartBeat(J1939CanContext* _PAGED context)
{
    if(context == NULL)
        return 0xFFFF;
    return context->heart_beat;   
}
//函数名称：J1939CanSetHeartBeat               
//功    能：J1939设置当前心跳计时值
//参    数：context  CAN设备的数据变量指针
//          cnt 要设置的心跳计时值
//返 回 值：无
//备    注：
void J1939CanSetHeartBeat(J1939CanContext* _PAGED context, unsigned int cnt)
{
    if(context == NULL)
        return;
    context->heart_beat = cnt;
}
#if J1939_OS_EVENT_EN > 0
//函数名称：J1939CreateSem              
//功    能：J1939创建信号量
//参    数：sem 
//          信号量的初值
//返 回 值：创建的信号量
//备    注：
J1939_EVENT* J1939CreateSem(unsigned char data)
{
    return OSSemCreate(data);
}

//函数名称：J1939SendSem              
//功    能：J1939发送信号量
//参    数：sem 待发送的信号量
//返 回 值：无
//备    注：
void J1939SendSem(J1939_EVENT* _PAGED sem)
{
    (void)OSSemPost(sem);
}
//函数名称：J1939PendSem              
//功    能：J1939等待信号量
//参    数：sem 等待的信号量
//          timeout 超时时间
//          err 等待结果，超时则返回超时错误
//返 回 值：无
//备    注：
void J1939PendSem(J1939_EVENT* _PAGED sem, unsigned int timeout, unsigned char* err)
{
    if(sem == NULL)
        return;
    OSSemPend(sem, timeout, err);   
}
//函数名称：J1939Sleep              
//功    能：J1939延时
//参    数：time 延时时间 单位：ms
//返 回 值：无
//备    注：任务调度式延时，非软件硬延时
void J1939Sleep(unsigned int time)
{
    OSTimeDly(time);   
}

//函数名称：J1939AddReceiveID              
//功    能：J1939新增加接收ID
//参    数：context J1939句柄 id：新增加的接收ID
//返 回 值：成功则返回J1939_OK，否则返回失败原因
//备    注：
J1939Result J1939AddReceiveID(J1939CanInfo* _PAGED can_info, INT32U id)
{
    INT32U mask_id = 0;
    J1939CanRecMaskGroupItem* _PAGED item;
    
    if(can_info == NULL) return J1939_ERR_PTR;
    
    item = (J1939CanRecMaskGroupItem* _PAGED)can_info->rec_mask_group_item.next;
    if(item == NULL)
    {
        item = (J1939CanRecMaskGroupItem* _PAGED )MALLOC(sizeof(J1939CanRecMaskGroupItem));
        if(item) can_info->rec_mask_group_item.next = (LIST_ITEM* _PAGED )item;
        else return J1939_ERR;
    }
    else
    {
        while(item->next != NULL && item->flag == 1)
        {
            item = (J1939CanRecMaskGroupItem* _PAGED)can_info->rec_mask_group_item.next;
        }
        if(item->next == NULL)
        {
            item->next = (LIST_ITEM* _PAGED )MALLOC(sizeof(J1939CanRecMaskGroupItem));
            if(item->next) item = (J1939CanRecMaskGroupItem* _PAGED)item->next;
            else return J1939_ERR;
        }
    }
    //更新接收ID
    item->flag = 1;
    item->next = NULL;
    item->start_addr = (INT8U)(id >> 8);
    item->stop_addr = item->start_addr;
    //更新掩码
    mask_id = id ^ can_info->receive_id.value;
    can_info->mask_id.value = mask_id | can_info->mask_id.value;
    
    return J1939_OK;
}

J1939Result J1939RecMessage2CanMessage(J1939RecMessage* _PAGED j1939_msg, CanMessage* _PAGED can_msg)
{
    if((j1939_msg == NULL)||(can_msg == NULL))
        return J1939_ERR_PTR;

	can_msg->id.value = j1939_msg->id.value;
	can_msg->len = (INT8U)j1939_msg->byte_cnt;
	(void)J1939MemCopy(can_msg->data, j1939_msg->data, CAN_DATA_MAX_LEN);
	
    return J1939_OK;
}

#endif

#endif
