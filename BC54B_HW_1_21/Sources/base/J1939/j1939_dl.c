/*******************************************************************************
**                       安徽力高新能源有限公司 Copyright (c)
**                            http://www.ligoo.cn/ 
**文 件 名j1939_dl.c                                       
**作    者：董丽伟
**创建日期：2012.05.25
**修改记录：
**文件说明:CAN通信标准J1939数据链路层驱动源文件
**版    本:V1.0
**备    注：
*******************************************************************************/
#ifndef J1939_DL_C_
#define J1939_DL_C_

#pragma MESSAGE DISABLE C1860
#pragma MESSAGE DISABLE C12056

#include "j1939_cfg.h"
#include "run_mode.h"

#define J1939_DL_GLOBALS


//函数名称：J1939_dl_with_buff_Init               
//功    能：J1939数据缓存初始化
//参    数：context  CAN通信用的数据变量指针
//返 回 值：初始化成功则返回J1939_OK，否则返回失败原因
//备    注：
J1939Result J1939_dl_with_buff_Init(J1939CanContext* _PAGED context)
{
	if(context == NULL)
		return J1939_ERR_PTR;
	//初始化发送帧缓存
	if(context->send_frame_buff == NULL)
	{
		return J1939_ERR_RAM;
	}
	if(context->send_frame_buff->buffer == NULL)
	{
		return J1939_ERR_RAM;
	}
	context->send_frame_buff->read_index = 0;
	context->send_frame_buff->write_index = 0;
	context->send_frame_buff->frame_cnt = 0;
	
	//初始化接收PDU缓存
	if(context->rec_pdu_buff == NULL)
	{
		return J1939_ERR_RAM;
	}
	if(context->rec_pdu_buff->buffer == NULL)
	{
		return J1939_ERR_RAM;
	}
	context->rec_pdu_buff->read_index = 0;
	context->rec_pdu_buff->write_index = 0;
	context->rec_pdu_buff->frame_cnt = 0;
	//初始化接收帧缓存
	if(context->rec_frame_buff == NULL)
	{
		return J1939_ERR_RAM;
	}

	if(context->rec_frame_buff->buffer == NULL)
	{
		return J1939_ERR_RAM;
	}
	context->rec_frame_buff->read_index = 0;
	context->rec_frame_buff->write_index = 0;
	context->rec_frame_buff->frame_cnt = 0;
	
	//初始化发送消息缓存
	if(context->send_message_buff == NULL)
	{
		return J1939_ERR_RAM;
	}

	if(context->rec_temp_mess_buff == NULL)
	{
		return J1939_ERR_RAM;
	}
	if(context->rec_temp_mess_buff->buffer == NULL)
	{
		return J1939_ERR_RAM;
	}
	//J1939MemSet(context->rec_temp_mess_buff->buffer->id.data, 0, sizeof(J1939RecMessage)*J1939_TP_REC_MAX);
	
	//初始化接收消息缓存
	if(context->rec_message_buff == NULL)
	{
		return J1939_ERR_RAM;
	}
	if(context->rec_message_buff->buffer == NULL)
	{
		return J1939_ERR_RAM;
	}
	
	context->rec_message_buff->message_cnt = 0;
	context->rec_message_buff->read_index = 0;
	context->rec_message_buff->write_index = 0;

	return J1939_OK;
}
//函数名称：J1939_dl_write_buff               
//功    能：J1939写数据帧缓存
//参    数：can_buff 要写入的缓存的指针
//          msg  待写入的CAN帧的指针
//返 回 值：成功则返回J1939_OK，否则返回失败原因
//备    注：将数据帧放入指定缓存，先入先出原则
J1939Result J1939_dl_write_buff(J1939CanFrameBuff* _PAGED can_buff, J1939CanFrame* _PAGED msg)
{
    OS_CPU_SR cpu_sr=0;
    
    if(can_buff == NULL)
    {
        return J1939_ERR_PTR;   
    }
    if(msg == NULL)
    {
        return J1939_ERR_MSG_PTR;   
    }
    OS_ENTER_CRITICAL();
    //memcpy(&can_buff->buffer[can_buff->write_index], msg, sizeof(J1939CanFrame));
    (void)J1939CanFrameCopy(&can_buff->buffer[can_buff->write_index], msg);
    can_buff->write_index++;
    if(can_buff->write_index >= can_buff->buffer_cnt)
    {
        can_buff->write_index = 0;   
    }
        
    if(can_buff->frame_cnt < can_buff->buffer_cnt)
    {
        can_buff->frame_cnt++;
        if(can_buff->frame_cnt > can_buff->buffer_cnt)
        {
            can_buff->frame_cnt = can_buff->buffer_cnt;
        }
        OS_EXIT_CRITICAL();
        return J1939_OK;
    }
    else
    {
        can_buff->read_index++;
        if(can_buff->read_index >= can_buff->buffer_cnt)
        {
            can_buff->read_index = 0;   
        }
        OS_EXIT_CRITICAL();
        return J1939_ERR_BUFF_FULL;   
    }
}

//函数名称：J1939_dl_read_buff               
//功    能：J1939读数据帧缓存
//参    数：can_buff 要读取的缓存的指针
//          msg  读取的CAN帧的保存位置指针
//返 回 值：成功则返回J1939_OK，否则返回失败原因
//备    注：从指定缓存读取CAN数据帧，先入先出原则
J1939Result J1939_dl_read_buff(J1939CanFrameBuff* _PAGED can_buff, J1939CanFrame* _PAGED msg)
{
    OS_CPU_SR cpu_sr=0;
    
    if(can_buff == NULL)
    {
        return J1939_ERR_PTR;   
    }
    if(msg == NULL)
    {
        return J1939_ERR_PTR;
    }
    
    OS_ENTER_CRITICAL();
    if(can_buff->frame_cnt > 0)
    {
        
        //memcpy(msg, &can_buff->buffer[can_buff->read_index], sizeof(J1939CanFrame));
        (void)J1939CanFrameCopy(msg, &can_buff->buffer[can_buff->read_index]);
        can_buff->read_index++;
        if(can_buff->read_index >= can_buff->buffer_cnt)
        {
            can_buff->read_index = 0;   
        }
        
        can_buff->frame_cnt--;
        if(can_buff->frame_cnt==0)
            can_buff->write_index = can_buff->read_index;
        OS_EXIT_CRITICAL();        
        return J1939_OK;
    }
    else
    {
        OS_EXIT_CRITICAL();        
        return J1939_ERR_BUFF_EMPTY;  
    }
}
//函数名称：J1939_dl_write_message               
//功    能：J1939写消息缓存
//参    数：souce 要写入的消息的指针
//          dest  待写入的消息的指针
//返 回 值：成功则返回J1939_OK，否则返回失败原因
//备    注：将消息放入指定缓存，先入先出原则
J1939Result J1939_dl_write_message(J1939RecMessageBuff* _PAGED dest, J1939RecMessage* _PAGED source)
{
    if(dest == NULL)
    {
        return J1939_ERR_PTR;   
    }
    if(source == NULL)
    {
        return J1939_ERR_MSG_PTR;   
    }

    dest->buffer[dest->write_index].byte_cnt = source->byte_cnt;
    //memcpy(dest->buffer[dest->write_index].data, source->data, source->byte_cnt);
    (void)J1939MemCopy(dest->buffer[dest->write_index].data, source->data, source->byte_cnt);
    dest->buffer[dest->write_index].id.value = source->id.value;
    dest->buffer[dest->write_index].pgn = source->pgn;
	dest->buffer[dest->write_index].user_data = source->user_data;
	dest->buffer[dest->write_index].type = source->type;
                 
    dest->write_index++;
    if(dest->write_index >= dest->buffer_cnt)
    {
        dest->write_index = 0;   
    }
        
    if(dest->message_cnt < dest->buffer_cnt)
    {
        dest->message_cnt++;
        if(dest->message_cnt > dest->buffer_cnt)
        {
            dest->message_cnt = dest->buffer_cnt;
        }
        
        return J1939_OK;
    }
    else
    {
        return J1939_ERR_BUFF_FULL;   
    }
}
//函数名称：J1939_dl_read_message               
//功    能：J1939读消息缓存
//参    数：source 要读取的消息的指针
//          dest  读取的消息的保存位置指针
//返 回 值：成功则返回J1939_OK，否则返回失败原因
//备    注：从指定缓存读取消息，先入先出原则
J1939Result J1939_dl_read_message(J1939RecMessageBuff* _PAGED source, J1939RecMessage* _PAGED dest)
{
    if(source == NULL)
    {
        return J1939_ERR_PTR;   
    }
    if(dest == NULL)
    {
        return J1939_ERR_PTR;
    }
    
    if(source->message_cnt > 0)
    {
        //memcpy(dest, &source->buffer[source->read_index], sizeof(J1939RecMessage));
        (void)J1939CanRecMessageCopy(dest, &source->buffer[source->read_index]);
        source->read_index++;
        if(source->read_index >= source->buffer_cnt)
        {
            source->read_index = 0;   
        }
        
        source->message_cnt--;
                
        return J1939_OK;
    }
    else
    {
         return J1939_ERR_BUFF_EMPTY;  
    }
}

//函数名称：J1939_dl_send_poll               
//功    能：J1939实时查询并发送数据帧
//参    数：context CAN的设备缓存的指针
//返 回 值：无
//备    注：周期性函数

void J1939_dl_send_poll(J1939CanContext* _PAGED context)
{
    J1939Result res;
    J1939CanFrame frame;
    
    if(context == NULL)
        return;
    res = J1939_dl_read_buff(context->send_frame_buff, &frame);
    while(res == J1939_OK)
    {   
        (void)J1939CanSendFrame(context, &frame);
        res = J1939_dl_read_buff(context->send_frame_buff, &frame);
        if(res == J1939_OK)
            J1939Sleep(1);
    }
}
//函数名称：J1939_dl_get_tp_chain               
//功    能：J1939获取空闲的tp
//参    数：context CAN的设备缓存的指针
//返 回 值：tp 空闲tp的编号
//备    注：
unsigned char J1939_dl_get_tp_chain(J1939CanContext* _PAGED context)
{
    unsigned char free;
    
    if(context == NULL)
    {
        return 0xFF;   
    }
    if(context->rec_tp_buff->tp_cnt >= context->rec_tp_buff->tp_max)
    {
        return 0xFF;   
    }
    free = context->rec_tp_buff->free_index;
    context->rec_tp_buff->free_index = context->rec_tp_buff->buffer[context->rec_tp_buff->free_index].after;

    context->rec_tp_buff->tp_cnt++;
    return  free;
}
//函数名称：J1939_dl_put_tp_chain               
//功    能：J1939释放tp到空闲的tp链
//参    数：context CAN的设备缓存的指针
//          tp 要释放的TP的索引
//返 回 值：无
//备    注：
void J1939_dl_put_tp_chain(J1939CanContext* _PAGED context, unsigned char tp)
{
    if(context == NULL)
    {
        return;   
    }
    if(tp >= context->rec_tp_buff->tp_max)
    {
        return;   
    }
    if(context->rec_tp_buff->tp_cnt >= context->rec_tp_buff->tp_max)
    {
        context->rec_tp_buff->free_index = tp;   
    }
    else
    {
        context->rec_tp_buff->buffer[context->rec_tp_buff->buffer[tp].before].after = context->rec_tp_buff->buffer[tp].after;
        context->rec_tp_buff->buffer[context->rec_tp_buff->buffer[tp].after].before = context->rec_tp_buff->buffer[tp].before;
        context->rec_tp_buff->buffer[context->rec_tp_buff->buffer[context->rec_tp_buff->free_index].after].before = tp;
        context->rec_tp_buff->buffer[tp].after = context->rec_tp_buff->buffer[context->rec_tp_buff->free_index].after;
        context->rec_tp_buff->buffer[context->rec_tp_buff->free_index].after = tp;
        context->rec_tp_buff->buffer[tp].before = context->rec_tp_buff->free_index;   
    }
  
    if(context->rec_tp_buff->tp_cnt > 0)
        context->rec_tp_buff->tp_cnt--;
}
//函数名称：J1939_dl_search_tp_with_source               
//功    能：J1939根据源地址查找tp链
//参    数：context CAN的设备缓存的指针
//          addr 源地址
//返 回 值：对应源地址的tp的索引，未找到对应TP返回0xFF
//备    注：
unsigned char J1939_dl_search_tp_with_source(J1939CanContext* _PAGED context, unsigned char addr)
{
    unsigned char num = 0, cnt;
    
    if(context == NULL)
    {
        return 0xFF;   
    }  
    num = context->rec_tp_buff->buffer[context->rec_tp_buff->free_index].before;
    cnt = context->rec_tp_buff->tp_cnt;
    while(cnt--)
    {
        if(context->rec_tp_buff->buffer[num].tp.source_addr == addr)
        {
            return num;   
        }
        num = context->rec_tp_buff->buffer[num].before;
    }
    return 0xFF;
}

#endif
