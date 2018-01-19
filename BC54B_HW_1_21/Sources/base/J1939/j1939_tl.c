/*******************************************************************************
**                       安徽力高新能源有限公司 Copyright (c)
**                            http://www.ligoo.cn/ 
**文 件 名j1939_tl.c                                       
**作    者：董丽伟
**创建日期：2012.05.25
**修改记录：
**文件说明:CAN通信标准J1939传输层驱动源文件
**版    本:V1.0
**备    注：
*******************************************************************************/
#ifndef J1939_TL_C_
#define J1939_TL_C_

#pragma MESSAGE DISABLE C12056

#include "j1939_cfg.h"
#include "run_mode.h"

#define J1939_TL_GLOBALS

const unsigned int G_TP_REC_STS[16] = {
0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f, 0xff,
0x1ff, 0x3ff, 0x7ff, 0xfff, 0x1fff, 0x3fff, 0x7fff, 0xffff
};

J1939Result J1939_tl_tx_idle_process(J1939CanContext* _PAGED context);
J1939Result J1939_tl_tx_send_rts_message(J1939CanContext* _PAGED context);
J1939Result J1939_tl_tx_send_normal_rts_message(J1939CanContext* _PAGED context);
J1939Result J1939_tl_tx_send_broadcast_bam_message(J1939CanContext* _PAGED context);
J1939Result J1939_tl_tx_wait_cts_process(J1939CanContext* _PAGED context, unsigned int elap);
J1939Result J1939_tl_tx_send_dt_message(J1939CanContext* _PAGED context, unsigned int elap);
J1939Result J1939_tl_tx_send_normal_dt_message(J1939CanContext* _PAGED context, unsigned int elap);
J1939Result J1939_tl_tx_send_broadcast_dt_message(J1939CanContext* _PAGED context, unsigned int elap);
J1939Result J1939_tl_tx_wait_eom_process(J1939CanContext* _PAGED context, unsigned int elap);
J1939Result J1939_tl_tx_message_abort_process(J1939CanContext* _PAGED context);
J1939Result J1939_tl_tx_send_message_finish_process(J1939CanContext* _PAGED context);

J1939Result J1939_tl_receive_tp_message_process(J1939CanContext* _PAGED context, J1939CanFrame* _PAGED can_msg);
J1939Result J1939_tl_rec_tp_update(J1939CanContext* _PAGED context, J1939CanFrame* _PAGED can_msg, unsigned int elap);
J1939Result J1939_tl_rec_tp_create(J1939CanContext* _PAGED context, J1939CanFrame* _PAGED can_msg);
J1939Result J1939_tl_rx_receive_rts_message(J1939CanContext* _PAGED context, J1939CanFrame* _PAGED can_msg);
INT16U      J1939_tl_get_rec_tp_buffer_index_with_rts(J1939CanContext* _PAGED context, J1939CanFrame* _PAGED can_msg);
J1939Result J1939_tl_rx_receive_normal_rts_message(J1939CanContext* _PAGED context, J1939CanFrame* _PAGED can_msg, INT16U tp_index);
J1939Result J1939_tl_rx_receive_broadcast_rts_message(J1939CanContext* _PAGED context, J1939CanFrame* _PAGED can_msg, INT16U tp_index);
J1939Result J1939_tl_rx_receive_cts_message(J1939CanContext* _PAGED context, J1939CanFrame* _PAGED can_msg);
J1939Result J1939_tl_update_send_tp_with_cts_message(J1939CanContext* _PAGED context, J1939CanFrame* _PAGED can_msg);
J1939Result J1939_tl_rx_receive_eom_message(J1939CanContext* _PAGED context, J1939CanFrame* _PAGED can_msg);
J1939Result J1939_tl_rx_receive_abort_message(J1939CanContext* _PAGED context, J1939CanFrame* _PAGED can_msg);
J1939Result J1939_tl_rx_receive_dt_message(J1939CanContext* _PAGED context, J1939CanFrame* _PAGED can_msg);
J1939Result J1939_tl_rx_receive_dt_frame(J1939CanContext* _PAGED context, J1939CanFrame* _PAGED can_msg, INT16U tp_index);
J1939Result J1939_tl_rx_prepare_to_next_normal_dt_frame(J1939CanContext* _PAGED context, INT16U tp_index);
J1939Result J1939_tl_rx_prepare_to_next_broadcast_dt_frame(J1939CanContext* _PAGED context, INT16U tp_index);
J1939Result J1939_tl_rx_receive_pdu_message(J1939CanContext* _PAGED context, J1939CanFrame* _PAGED can_msg);

void J1939_clear_send_message(J1939SendMessageBuff* _PAGED send_index);
void J1939_clear_rec_message(J1939RecMessage* _PAGED rec_index);

//函数名称：J1939_tl_with_buff_init           
//功    能：J1939传输协议传输初始化
//参    数：context  CAN通信用的数据变量指针
//返 回 值：初始化成功则返回J1939_OK，否则返回失败原因
//备    注：
J1939Result J1939_tl_with_buff_init(J1939CanContext* _PAGED context)
{
	unsigned int i;
	
	if(context == NULL)
	{
		return J1939_ERR_PTR;
	}
	//初始化CAN发送传输协议参数
	if(context->send_tp_index == NULL)
	{
		return J1939_ERR_RAM;
	}
	(void)J1939MemSet((INT8U* _PAGED)context->send_tp_index, 0, sizeof(J1939CanTranPro));
	//初始化CAN接收传输协议传输参数
	if(context->rec_tp_buff == NULL)
	{
		return J1939_ERR_RAM;
	}

	if(context->rec_tp_buff->buffer == NULL)
	{
		return J1939_ERR_RAM;
	}
	(void)J1939MemSet((INT8U* _PAGED)(&context->rec_tp_buff->buffer->tp), 0, sizeof(J1939TPChain)*context->rec_tp_buff->tp_max);
	context->rec_tp_buff->buffer_cnt = context->rec_tp_buff->tp_max;
	context->rec_tp_buff->tp_cnt = 0;
	context->rec_tp_buff->free_index = 0;
	//初始化传输协议传输接收链
	if(context->rec_tp_buff->tp_max == 1)
	{
		context->rec_tp_buff->buffer[0].before = 0;
		context->rec_tp_buff->buffer[0].after = 0;
	}
	else
	{
		context->rec_tp_buff->buffer[0].before = context->rec_tp_buff->tp_max-1;
		context->rec_tp_buff->buffer[0].after = 1;
		context->rec_tp_buff->buffer[context->rec_tp_buff->tp_max-1].before = context->rec_tp_buff->tp_max-2;
		context->rec_tp_buff->buffer[context->rec_tp_buff->tp_max-1].after = 0;
	}
    if(context->rec_tp_buff->tp_max > 2)
    {
    	for(i = 1; i < context->rec_tp_buff->tp_max-1; i++)
    	{
    		context->rec_tp_buff->buffer[i].before = i - 1;
    		context->rec_tp_buff->buffer[i].after = i + 1;
    	}
    }

	return J1939_OK;
}
//函数名称：J1939_tl_send_poll              
//功    能：J1939传输协议传输发送数据处理函数
//参    数：context  CAN通信用的数据变量指针
//          elap 上次调用到本次调用逝去的时间,单位：ms
//返 回 值：成功则返回J1939_OK，否则返回失败原因
//备    注：
J1939Result J1939_tl_send_poll(J1939CanContext* _PAGED context, unsigned int elap)
{
    J1939Result res = J1939_OK;
    
    if(context == NULL)
    {
        return J1939_ERR_PTR;   
    }
    
    switch(context->send_tp_index->status)
    {
        case STATUS_TP_IDLE:
            res = J1939_tl_tx_idle_process(context);
            break;
        case STATUS_TP_RTS:
            res = J1939_tl_tx_send_rts_message(context);
            break;
        case STATUS_TP_CTS:
            res = J1939_tl_tx_wait_cts_process(context, elap);
            break;
        case STATUS_TP_DT:
            res = J1939_tl_tx_send_dt_message(context, elap);
            break;
        case STATUS_TP_EOM:
            res = J1939_tl_tx_wait_eom_process(context, elap);
            break;
        case STATUS_TP_ABORT:
            res = J1939_tl_tx_message_abort_process(context);
            break;
        case STATUS_TP_FINISH:
            res = J1939_tl_tx_send_message_finish_process(context);
            break;
        case STATUS_TP_BAM:
            res = J1939_tl_tx_send_broadcast_bam_message(context);
            break;
        default:
            context->send_tp_index->status = STATUS_TP_ABORT;
            break;
    }
    return res;
}

J1939Result J1939_tl_tx_idle_process(J1939CanContext* _PAGED context)
{
    if(context == NULL) return J1939_ERR_PTR;
    
    context->send_tp_index->delay = 0;
    
    return J1939_OK;
}

J1939Result J1939_tl_tx_send_rts_message(J1939CanContext* _PAGED context)
{
    J1939Result res = J1939_OK;
    
    //if(context->send_message_buff->type == kBroadcastType)
    //    res = J1939_tl_tx_send_broadcast_bam_message(context);
    //else
        res = J1939_tl_tx_send_normal_rts_message(context);
    
    return res;
}

J1939Result J1939_tl_tx_send_normal_rts_message(J1939CanContext* _PAGED context)
{
    J1939Result res = J1939_OK;
    J1939CanFrame g_canFrame;
    
    if(context == NULL) return J1939_ERR_PTR;
    
    context->send_tp_index->pgn = context->send_message_buff->pgn;
    context->send_tp_index->frame_cnt = (INT8U)((context->send_message_buff->byte_cnt + 6) / 7);
    context->send_tp_index->byte_cnt = context->send_message_buff->byte_cnt;
    context->send_tp_index->source_addr = context->send_message_buff->id.data[3];
    context->send_tp_index->dest_addr = context->send_message_buff->id.data[2];
    context->send_tp_index->delay = 0;
    context->send_tp_index->start_frame = 1;
    context->send_tp_index->frame_index = 1;
    context->send_tp_index->stop_frame = context->send_tp_index->frame_cnt;
    J1939_tl_config_rts(&g_canFrame, context->send_tp_index);
    res = J1939_dl_write_buff(context->send_frame_buff, &g_canFrame);
    if(res == J1939_OK) context->send_tp_index->status = STATUS_TP_CTS;
    
    return res;
}

J1939Result J1939_tl_tx_send_broadcast_bam_message(J1939CanContext* _PAGED context)
{
    J1939Result res = J1939_OK;
    J1939CanFrame g_canFrame;
    
    if(context == NULL) return J1939_ERR_PTR;
    
    context->send_tp_index->pgn = context->send_message_buff->pgn;
    context->send_tp_index->frame_cnt = (INT8U)((context->send_message_buff->byte_cnt + 6) / 7);
    context->send_tp_index->byte_cnt = context->send_message_buff->byte_cnt;
    context->send_tp_index->source_addr = context->send_message_buff->id.data[3];
    context->send_tp_index->dest_addr = J1939_BROADCAST_ID;
    context->send_tp_index->delay = 0;
    context->send_tp_index->start_frame = 1;
    context->send_tp_index->frame_index = 1;
    context->send_tp_index->stop_frame = context->send_tp_index->frame_cnt;
    J1939_tl_config_bam(&g_canFrame, context->send_tp_index);
    res = J1939_dl_write_buff(context->send_frame_buff, &g_canFrame);
    if(res == J1939_OK) context->send_tp_index->status = STATUS_TP_DT;
    
    return res;
}

J1939Result J1939_tl_tx_wait_cts_process(J1939CanContext* _PAGED context, unsigned int elap)
{
    J1939Result res = J1939_OK;
    
    if(context == NULL) return J1939_ERR_PTR;
    
    context->send_tp_index->delay += elap;//超时计数
    if(context->send_tp_index->delay > TP_CTS_REC_DLY)
    {
        context->send_tp_index->delay = 0;
        context->send_tp_index->status = STATUS_TP_ABORT;
        res = J1939_ERR_CTS;   
    }
    
    return res;
}

J1939Result J1939_tl_tx_send_dt_message(J1939CanContext* _PAGED context, unsigned int elap)
{
    J1939Result res = J1939_OK;
    
    if(context->send_message_buff->type == kBroadcastType)
        res = J1939_tl_tx_send_broadcast_dt_message(context, elap);
    else
        res = J1939_tl_tx_send_normal_dt_message(context, elap);
    
    return res;
}

J1939Result J1939_tl_tx_send_normal_dt_frame(J1939CanContext* _PAGED context, unsigned int elap, unsigned int send_cycle)
{
    J1939Result res = J1939_OK;
    J1939CanFrame g_canFrame;
    INT8U i, len = J1939_CAN_FRAME_MAX;
    
    if(context == NULL) return J1939_ERR_PTR;
    
    context->send_tp_index->delay += elap;//超时计数
    if(context->send_tp_index->delay > send_cycle)
    {
        context->send_tp_index->delay = 0;
        g_canFrame.id.value = TP_DT_ID_BASE;
        g_canFrame.id.data[2] = context->send_tp_index->dest_addr; 
        g_canFrame.id.data[3] = context->send_tp_index->source_addr;
        if((context->send_tp_index->frame_index > 0)&&
            (context->send_tp_index->frame_index < context->send_tp_index->frame_cnt + 1))
        {
            if(context->send_tp_index->frame_index != context->send_tp_index->frame_cnt)//帧长度判断 
            {
                g_canFrame.byte_cnt = 8;
            }
            else//最后一帧 
            {
                len = context->send_tp_index->byte_cnt - 
                                      (context->send_tp_index->frame_index - 1) * 7 + 1;
                if(len > J1939_CAN_FRAME_MAX)
                {
                    len = J1939_CAN_FRAME_MAX;
                }
                g_canFrame.byte_cnt = J1939_CAN_FRAME_MAX;
            }
            
            g_canFrame.data[0] = context->send_tp_index->frame_index;
            (void)J1939MemCopy(&g_canFrame.data[1],
                    context->send_message_buff->data + (context->send_tp_index->frame_index - 1) * 7,
                    g_canFrame.byte_cnt - 1);
            for(i = len; i < J1939_CAN_FRAME_MAX; i++) g_canFrame.data[i] = J1939_VALUE_DEFAULT; 
            
            res = J1939_dl_write_buff(context->send_frame_buff, &g_canFrame);
        }
        else//帧编号异常，结束通信
        {
             res = J1939_ERR_FRAME_NUM; 
        }
    }
    else
        res = J1939_ERR;
    return res;
}

J1939Result J1939_tl_tx_send_broadcast_dt_frame(J1939CanContext* _PAGED context, unsigned int elap, unsigned int send_cycle)
{
    INT8U i = 0, len;
    J1939Result res = J1939_OK;
    J1939CanFrame g_canFrame;
    
    if(context == NULL) return J1939_ERR_PTR;
    
    context->send_tp_index->delay += elap;//超时计数
    if(context->send_tp_index->delay > send_cycle)
    {
        context->send_tp_index->delay = 0;
        g_canFrame.id.value = TP_DT_ID_BASE;
        g_canFrame.id.data[2] = context->send_tp_index->dest_addr; 
        g_canFrame.id.data[3] = context->send_tp_index->source_addr;
        if((context->send_tp_index->frame_index > 0)&&
            (context->send_tp_index->frame_index < context->send_tp_index->frame_cnt + 1))
        {
            /*if(context->send_tp_index->frame_index != context->send_tp_index->frame_cnt)//帧长度判断 
            {
                g_canFrame.byte_cnt = 8;
            }
            else//最后一帧 
            {
                g_canFrame.byte_cnt = context->send_tp_index->byte_cnt - 
                                      (context->send_tp_index->frame_index - 1) * 7 + 1;
                if(g_canFrame.byte_cnt > 8)
                {
                    g_canFrame.byte_cnt = 8;
                }
            }*/
            g_canFrame.byte_cnt = J1939_CAN_FRAME_MAX;
            len = context->send_tp_index->byte_cnt - 
                    (context->send_tp_index->frame_index - 1) * 7 + 1;
            if(len > J1939_CAN_FRAME_MAX) len = J1939_CAN_FRAME_MAX;
            
            g_canFrame.data[0] = context->send_tp_index->frame_index;
            (void)J1939MemCopy(&g_canFrame.data[1],
                    context->send_message_buff->data + (context->send_tp_index->frame_index - 1) * 7,
                    g_canFrame.byte_cnt - 1);
            for(i = len; i < J1939_CAN_FRAME_MAX; i++) g_canFrame.data[i] = J1939_VALUE_DEFAULT; 
            
            res = J1939_dl_write_buff(context->send_frame_buff, &g_canFrame);
        }
        else//帧编号异常，结束通信
        {
             res = J1939_ERR_FRAME_NUM; 
        }
    }
    else
        res = J1939_ERR;
    return res;
}
void J1939_tl_next_normal_dt_frame(J1939CanContext* _PAGED context)
{
    if(context == NULL) return;
    
    if(context->send_tp_index->frame_index >= context->send_tp_index->frame_cnt)//DT发送完成，等待EOM
    {
        context->send_tp_index->status = STATUS_TP_EOM;   
    }
    else
    {
        if(context->send_tp_index->frame_index == context->send_tp_index->stop_frame)//DT未发送完成，等待CTS
        {
            context->send_tp_index->status = STATUS_TP_CTS;      
        }
        else
        {
            context->send_tp_index->frame_index++;   
        }
    }
}

void J1939_tl_next_broadcast_dt_frame(J1939CanContext* _PAGED context)
{
    if(context == NULL) return;
    
    if(context->send_tp_index->frame_index >= context->send_tp_index->frame_cnt)//DT发送完成，等待EOM
    {
#if J1939_OS_EVENT_EN > 0
        if(context->send_event_sem->use_flag == 1)
        {
            J1939SendSem(context->send_event_sem->event);
            context->send_event_sem->use_flag = 0;
        }
#endif
        context->send_tp_index->status = STATUS_TP_FINISH;   
    }
    else
    {
        context->send_tp_index->frame_index++;
    }
}

J1939Result J1939_tl_tx_send_normal_dt_message(J1939CanContext* _PAGED context, unsigned int elap)
{
    J1939Result res = J1939_OK;
    
    if(context == NULL) return J1939_ERR_PTR;
    if(mode == RUN_MODE_READ)
    {
        res = J1939_tl_tx_send_normal_dt_frame(context, elap, 0); 
    } 
    else
    {
    res = J1939_tl_tx_send_normal_dt_frame(context, elap, TP_DT_SEND_DLY);
    }
    
    
    if(res == J1939_OK) J1939_tl_next_normal_dt_frame(context);
    
    return res;
}

J1939Result J1939_tl_tx_send_broadcast_dt_message(J1939CanContext* _PAGED context, unsigned int elap)
{
    J1939Result res = J1939_OK;
    
    if(context == NULL) return J1939_ERR_PTR;
    
    res = J1939_tl_tx_send_broadcast_dt_frame(context, elap, TP_BROADCAST_DT_SEND_DLY);
    
    if(res == J1939_OK) J1939_tl_next_broadcast_dt_frame(context);
    
    return res;
}

J1939Result J1939_tl_tx_wait_eom_process(J1939CanContext* _PAGED context, unsigned int elap)
{
    J1939Result res = J1939_OK;
    
    if(context == NULL) return J1939_ERR_PTR;
    
    context->send_tp_index->delay += elap; 
    if(context->send_tp_index->delay > TP_EOM_REC_DLY)
    {
        context->send_tp_index->delay = 0;
        context->send_tp_index->status = STATUS_TP_ABORT;
        res = J1939_ERR_EOM;   
    }
    
    return res;
}

J1939Result J1939_tl_tx_message_abort_process(J1939CanContext* _PAGED context)
{
    J1939Result res = J1939_OK;
    
    if(context == NULL) return J1939_ERR_PTR;
    
#if J1939_OS_EVENT_EN > 0
    if(context->send_event_sem->use_flag == 1)
    {
        J1939SendSem(context->send_event_sem->event);
        context->send_event_sem->use_flag = 0;
    }
#endif   
    context->send_tp_index->delay = 0;
    
    return res;
}

J1939Result J1939_tl_tx_send_message_finish_process(J1939CanContext* _PAGED context)
{
    J1939Result res = J1939_OK;
    
    if(context == NULL) return J1939_ERR_PTR;
    
    context->send_tp_index->delay = 0;
    return res;
}

//函数名称：J1939_tl_rec_poll          
//功    能：J1939传输协议传输接收数据处理函数
//参    数：context  CAN通信用的数据变量指针
//          elap 上次调用到本次调用逝去的时间,单位：ms
//返 回 值：初始化成功则返回J1939_OK，否则返回失败原因
//备    注：

J1939Result J1939_tl_rec_poll(J1939CanContext* _PAGED context, unsigned int elap)
{
    unsigned char frame_num = 0, pri = 0;
    J1939Result res = J1939_OK;
    J1939CanFrame g_canFrame;
    
    if(context == NULL)
    {
        return J1939_ERR_PTR;   
    }
    frame_num = context->rec_frame_buff->frame_cnt;
    while(frame_num--)
    {
        res = J1939_dl_read_buff(context->rec_frame_buff, &g_canFrame);
        if(res != J1939_OK) continue;
        
        pri = J1939_GET_PRI(g_canFrame.id.value);
        if(pri == TP_PRI)
        {
            switch(g_canFrame.id.data[1])
            {
                case PF_TP://TP中的连接控制报文
                    res = J1939_tl_receive_tp_message_process(context, &g_canFrame);
                    break;
                case PF_DT://TP中的数据报文
    			    res = J1939_tl_rx_receive_dt_message(context, &g_canFrame);
    			    break;
    			default://其他报文
    			    res = J1939_tl_rx_receive_pdu_message(context, &g_canFrame);
    			    break;
            }
        }
        else
        {
            res = J1939_tl_rx_receive_pdu_message(context, &g_canFrame);
        }
    }
    res = J1939_tl_rec_tp_update(context, &g_canFrame, elap);
    
    return res;
}

J1939Result J1939_tl_receive_tp_message_process(J1939CanContext* _PAGED context, J1939CanFrame* _PAGED can_msg)
{
    J1939Result res = J1939_OK;
    
    if(context == NULL || can_msg == NULL) return J1939_ERR_PTR;
    
    switch(can_msg->data[0])
    {
    case TP_CMD_RTS:
        res = J1939_tl_rx_receive_rts_message(context, can_msg);
        break;
    case TP_CMD_CTS:
        res = J1939_tl_rx_receive_cts_message(context, can_msg);
        break;
    case TP_CMD_EOM:
        res = J1939_tl_rx_receive_eom_message(context, can_msg);
        break;
    case TP_CMD_ABORT:  
        res = J1939_tl_rx_receive_abort_message(context, can_msg);
        break;
    default:
        break;
    }
    return res;
}

J1939Result J1939_tl_rec_tp_update(J1939CanContext* _PAGED context, J1939CanFrame* _PAGED can_msg, unsigned int elap)
{
    J1939Result res = J1939_OK;
    unsigned char num;
    unsigned char tp_index;
    
    if(context == NULL || can_msg == NULL) return J1939_ERR_PTR;
    
    //每个TP的状态更新
    num = context->rec_tp_buff->buffer[context->rec_tp_buff->free_index].before;
    tp_index = context->rec_tp_buff->tp_cnt;
    while(tp_index--)
    {
        switch(context->rec_tp_buff->buffer[num].tp.status)   
        {
            case STATUS_TP_IDLE:
                context->rec_tp_buff->buffer[num].tp.delay = 0;
                break;
            case STATUS_TP_DT:
                context->rec_tp_buff->buffer[num].tp.delay += elap;
                if(context->rec_tp_buff->buffer[num].tp.delay > TP_DT_REC_DLY)
                {
                    if(context->rec_temp_mess_buff->buffer[num].type == kNormalType)
                    {
                        J1939_tl_config_abort(can_msg, &context->rec_tp_buff->buffer[num].tp);
                        res = J1939_dl_write_buff(context->send_frame_buff, can_msg);
                    }
                    context->rec_tp_buff->buffer[num].tp.delay = 0;
                    context->rec_tp_buff->buffer[num].tp.status = STATUS_TP_ABORT;  
                }
                break;
            case STATUS_TP_EOM:
                J1939_tl_clear_tp_index(&context->rec_tp_buff->buffer[num].tp);
                (void)J1939_dl_write_message(context->rec_message_buff, &context->rec_temp_mess_buff->buffer[num]);
                #if J1939_OS_EVENT_EN > 0
                if(context->receive_event_sem->use_flag == 1)
                    J1939SendSem(context->receive_event_sem->event);
                #endif
                J1939_dl_put_tp_chain(context, num);
                context->rec_tp_buff->buffer[num].tp.status = STATUS_TP_FINISH;
                break;
            case STATUS_TP_ABORT:
                J1939_tl_clear_tp_index(&context->rec_tp_buff->buffer[num].tp);
                J1939_clear_rec_message(&context->rec_temp_mess_buff->buffer[num]);
                J1939_dl_put_tp_chain(context, num);
                context->rec_tp_buff->buffer[num].tp.status = STATUS_TP_IDLE;
                break;
            case STATUS_TP_FINISH:
                context->rec_tp_buff->buffer[num].tp.delay = 0;
                break;
            default:
                
                break;    
        }
        num = context->rec_tp_buff->buffer[num].before;
    }

    return res;
}

J1939Result J1939_tl_rx_receive_rts_message(J1939CanContext* _PAGED context, J1939CanFrame* _PAGED can_msg)
{
    J1939Result res = J1939_OK;
    
    if(context == NULL || can_msg == NULL) return J1939_ERR_PTR;
    
    res = J1939_tl_rec_tp_create(context, can_msg);
    
    return res;
}

J1939Result J1939_tl_rec_tp_create(J1939CanContext* _PAGED context, J1939CanFrame* _PAGED can_msg)
{
    INT16U tp_index;
    J1939Result res = J1939_OK;
    
    if(context == NULL || can_msg == NULL) return J1939_ERR_PTR;
    
    tp_index = J1939_tl_get_rec_tp_buffer_index_with_rts(context, can_msg);

#if J1939_BROADCAST_MESSAGE_RECEIVE_SUPPORT > 0
    if(can_msg->id.data[2] == J1939_BROADCAST_ID)
        res = J1939_tl_rx_receive_broadcast_rts_message(context, can_msg, tp_index);
    else
#endif
        res = J1939_tl_rx_receive_normal_rts_message(context, can_msg, tp_index);
    
    /*can_tp.pgn = ((unsigned long)can_msg->data[7]<<16)+((unsigned int)can_msg->data[6]<<8)+can_msg->data[5];
    can_tp.source_addr = can_msg->id.data[3];
    can_tp.dest_addr = can_msg->id.data[2];
    
    rec_byte_num = ((unsigned int)can_msg->data[2] << 8)+can_msg->data[1];
    rec_frame_num = can_msg->data[3];
    
    num = J1939_dl_search_tp_with_source(context, can_msg->id.data[3]);
    
    if(num < context->rec_tp_buff->tp_max)
    {
        if(context->rec_tp_buff->buffer[num].tp.pgn != can_tp.pgn)//上次PGN与本次不同，则结束上次PGN连接
    	{
        	J1939_tl_config_abort(can_msg, &context->rec_tp_buff->buffer[num].tp);
        	(void)J1939_dl_write_buff(context->send_frame_buff, can_msg);
    	}
        J1939_tl_clear_tp_index(&context->rec_tp_buff->buffer[num].tp);
        tp_index = num;
    }
    else
        tp_index = J1939_dl_get_tp_chain(context);
    if(rec_frame_num == (rec_byte_num+6)/7)
    {
        if(tp_index < context->rec_tp_buff->tp_max)
        {   //获取接收TP缓存
            context->rec_tp_buff->buffer[tp_index].tp.pgn = can_tp.pgn;
            context->rec_tp_buff->buffer[tp_index].tp.frame_cnt = rec_frame_num;
            context->rec_tp_buff->buffer[tp_index].tp.byte_cnt = rec_byte_num;
            context->rec_tp_buff->buffer[tp_index].tp.source_addr = can_tp.source_addr;
            context->rec_tp_buff->buffer[tp_index].tp.dest_addr = can_tp.dest_addr;
            context->rec_tp_buff->buffer[tp_index].tp.delay = 0;
            context->rec_tp_buff->buffer[tp_index].tp.rec_sts = 0;
            context->rec_tp_buff->buffer[tp_index].tp.start_frame = 1;
            context->rec_tp_buff->buffer[tp_index].tp.frame_index = 1;
            if(context->rec_tp_buff->buffer[tp_index].tp.frame_cnt <= J1939_TP_DT_CNT_MAX)
            {
                context->rec_tp_buff->buffer[tp_index].tp.stop_frame = context->rec_tp_buff->buffer[tp_index].tp.frame_cnt;
            }
            else
            {
                context->rec_tp_buff->buffer[tp_index].tp.stop_frame = J1939_TP_DT_CNT_MAX;
            }
            J1939_tl_config_cts(can_msg, &context->rec_tp_buff->buffer[tp_index].tp);
            //初始化消息接收缓存
			context->rec_temp_mess_buff->buffer[tp_index].pgn = context->rec_tp_buff->buffer[tp_index].tp.pgn;
			//context->rec_temp_mess_buff->buffer[tp_index].user_data = can_msg->data[3];//TODO:无意义
			context->rec_tp_buff->buffer[tp_index].tp.status = STATUS_TP_DT; 
        }
        else//缓存已满
        {
             J1939_tl_config_abort(can_msg, &can_tp);
        }   
    }
    else
    {
        J1939_tl_config_abort(can_msg, &can_tp);
    }
    res = J1939_dl_write_buff(context->send_frame_buff, can_msg);
    */
    return res;
}

INT16U J1939_tl_get_rec_tp_buffer_index_with_rts(J1939CanContext* _PAGED context, J1939CanFrame* _PAGED can_msg)
{
    INT16U tp_index;
    
    if(context == NULL || can_msg == NULL) return -1;
    
    tp_index = J1939_dl_search_tp_with_source(context, can_msg->id.data[3]);
    
    if(tp_index < context->rec_tp_buff->tp_max)
    {
        //if(context->rec_tp_buff->buffer[tp_index].tp.pgn != (((unsigned long)can_msg->data[7]<<16)+((unsigned int)can_msg->data[6]<<8)+can_msg->data[5]))//上次PGN与本次不同，则结束上次PGN连接
    	//{
        //	J1939_tl_config_abort(can_msg, &context->rec_tp_buff->buffer[tp_index].tp);
        //	(void)J1939_dl_write_buff(context->send_frame_buff, can_msg);
    	//}
        J1939_tl_clear_tp_index(&context->rec_tp_buff->buffer[tp_index].tp);
    }
    else
        tp_index = J1939_dl_get_tp_chain(context);
    
    return tp_index;
}

J1939Result J1939_tl_rx_update_receive_tp_with_rts(J1939CanTranPro* _PAGED can_tp, J1939CanFrame* _PAGED can_msg)
{
    if(can_tp == NULL || can_msg == NULL) return J1939_ERR;
    
    can_tp->pgn = ((unsigned long)can_msg->data[7]<<16)+((unsigned int)can_msg->data[6]<<8)+can_msg->data[5];
    can_tp->source_addr = can_msg->id.data[3];
    can_tp->dest_addr = can_msg->id.data[2];
    can_tp->byte_cnt = ((unsigned int)can_msg->data[2] << 8)+can_msg->data[1];
    can_tp->frame_cnt = can_msg->data[3];
    can_tp->delay = 0;
    can_tp->rec_sts = 0;
    can_tp->start_frame = 1;
    can_tp->frame_index = 1;
    can_tp->stop_frame = can_tp->frame_cnt;
    
    return J1939_OK;
}

J1939Result J1939_tl_rx_receive_normal_rts_message(J1939CanContext* _PAGED context, J1939CanFrame* _PAGED can_msg, INT16U tp_index)
{
    J1939Result res = J1939_OK;
    J1939CanTranPro can_tp;
    
    if(context == NULL || can_msg == NULL) return J1939_ERR;
    if(tp_index >= context->rec_tp_buff->tp_max)
    {   //TP缓存已满
        can_tp.source_addr = can_msg->id.data[3];
        can_tp.dest_addr = can_msg->id.data[2];
        can_tp.pgn = J1939_GET_PGN(can_msg->data[7], can_msg->data[6], can_msg->data[5]);
        J1939_tl_config_abort(can_msg, &can_tp);
        res = J1939_dl_write_buff(context->send_frame_buff, can_msg);
        return J1939_ERR;
    }
    
    res = J1939_tl_rx_update_receive_tp_with_rts(&can_tp, can_msg);
    context->rec_tp_buff->buffer[tp_index].tp = can_tp;
        
    if(context->rec_tp_buff->buffer[tp_index].tp.frame_cnt <= J1939_TP_DT_CNT_MAX)
        context->rec_tp_buff->buffer[tp_index].tp.stop_frame = context->rec_tp_buff->buffer[tp_index].tp.frame_cnt;
    else
        context->rec_tp_buff->buffer[tp_index].tp.stop_frame = J1939_TP_DT_CNT_MAX;
    
    if(can_tp.frame_cnt != (can_tp.byte_cnt + 6) / 7)
    {
        J1939_tl_config_abort(can_msg, &can_tp);
        context->rec_tp_buff->buffer[tp_index].tp.status = STATUS_TP_ABORT;
    }
    else
    {
        J1939_tl_config_cts(can_msg, &context->rec_tp_buff->buffer[tp_index].tp);
        //初始化消息接收缓存
    	context->rec_temp_mess_buff->buffer[tp_index].pgn = context->rec_tp_buff->buffer[tp_index].tp.pgn;
    	context->rec_temp_mess_buff->buffer[tp_index].type = kNormalType;
    	context->rec_temp_mess_buff->buffer[tp_index].byte_cnt = 0;
    	//context->rec_temp_mess_buff->buffer[tp_index].user_data = can_msg->data[3];//TODO:无意义
    	context->rec_tp_buff->buffer[tp_index].tp.status = STATUS_TP_DT;
    }
    
    res = J1939_dl_write_buff(context->send_frame_buff, can_msg);
    
    return res;
}

J1939Result J1939_tl_rx_receive_broadcast_rts_message(J1939CanContext* _PAGED context, J1939CanFrame* _PAGED can_msg, INT16U tp_index)
{
    J1939Result res = J1939_OK;
    J1939CanTranPro can_tp;
    
    if(context == NULL || can_msg == NULL) return J1939_ERR;
    if(tp_index >= context->rec_tp_buff->tp_max) return J1939_ERR;//TP缓存已满
    
    res = J1939_tl_rx_update_receive_tp_with_rts(&can_tp, can_msg);
    context->rec_tp_buff->buffer[tp_index].tp = can_tp;
    
    if(can_tp.frame_cnt == (can_tp.byte_cnt + 6) / 7)
    {
        //初始化消息接收缓存
    	context->rec_temp_mess_buff->buffer[tp_index].pgn = context->rec_tp_buff->buffer[tp_index].tp.pgn;
    	context->rec_temp_mess_buff->buffer[tp_index].type = kBroadcastType;
    	context->rec_temp_mess_buff->buffer[tp_index].byte_cnt = 0;
    	//context->rec_temp_mess_buff->buffer[tp_index].user_data = can_msg->data[3];//TODO:无意义
    	context->rec_tp_buff->buffer[tp_index].tp.status = STATUS_TP_DT;
    }
    
    return res;
}

J1939Result J1939_tl_rx_receive_cts_message(J1939CanContext* _PAGED context, J1939CanFrame* _PAGED can_msg)
{
    J1939Result res = J1939_OK;
    
    if(context == NULL || can_msg == NULL) return J1939_ERR_PTR;
    
    res = J1939_tl_update_send_tp_with_cts_message(context, can_msg);
    
    return res;
}

J1939Result J1939_tl_update_send_tp_with_cts_message(J1939CanContext* _PAGED context, J1939CanFrame* _PAGED can_msg)
{
    J1939Result res = J1939_OK;
    J1939CanTranPro can_tp;
    unsigned long rec_pgn;
    
    if(context == NULL || can_msg == NULL) return J1939_ERR_PTR;
    
    can_tp.pgn = ((unsigned long)can_msg->data[7]<<16)+((unsigned int)can_msg->data[6]<<8)+can_msg->data[5];
    if((can_msg->id.data[3] == context->send_tp_index->dest_addr)&&
        (can_tp.pgn==context->send_tp_index->pgn))
    {
        rec_pgn = can_msg->data[5]+((unsigned int)can_msg->data[6]<<8)+((unsigned long)can_msg->data[7]<<16);
        if(rec_pgn == context->send_tp_index->pgn)//PGN一致，更新发送TP参数
        {
            context->send_tp_index->delay = 0;
            context->send_tp_index->start_frame = can_msg->data[2];
            context->send_tp_index->frame_index = context->send_tp_index->start_frame;
            context->send_tp_index->stop_frame = can_msg->data[2] + can_msg->data[1] - 1;
            
            if(context->send_tp_index->start_frame == 0) context->send_tp_index->status = STATUS_TP_ABORT;
            
            if(context->send_tp_index->stop_frame > context->send_tp_index->frame_cnt)
            {
                context->send_tp_index->stop_frame = context->send_tp_index->frame_cnt;
            }
            if((context->send_tp_index->status == STATUS_TP_CTS)||
                (context->send_tp_index->status == STATUS_TP_DT)||
                (context->send_tp_index->status == STATUS_TP_EOM))
            {
                context->send_tp_index->status = STATUS_TP_DT;
            }
        }
    }
    
    return res;
}

J1939Result J1939_tl_rx_receive_eom_message(J1939CanContext* _PAGED context, J1939CanFrame* _PAGED can_msg)
{
    J1939Result res = J1939_OK;
    J1939CanTranPro can_tp;
    
    if(context == NULL || can_msg == NULL) return J1939_ERR_PTR;
    
    can_tp.byte_cnt = ((unsigned int)can_msg->data[2] << 8) + can_msg->data[1];
    can_tp.pgn = ((unsigned long)can_msg->data[7]<<16)+((unsigned int)can_msg->data[6]<<8)+can_msg->data[5];
                                   
    if((can_msg->id.data[3] == context->send_tp_index->dest_addr)&&
        (can_tp.byte_cnt == context->send_tp_index->byte_cnt)&&
        (context->send_tp_index->frame_cnt == can_msg->data[3])&&
        (can_tp.pgn == context->send_tp_index->pgn))
    {   //EOM信息与RTS时一致
        J1939_tl_clear_tp_index(context->send_tp_index);
        J1939_clear_send_message(context->send_message_buff);
#if J1939_OS_EVENT_EN > 0
        if(context->send_event_sem->use_flag == 1)
        {
            J1939SendSem(context->send_event_sem->event);
            context->send_event_sem->use_flag = 0;
        }
#endif
        context->send_tp_index->status = STATUS_TP_FINISH;
    }
    
    return res;
}

J1939Result J1939_tl_rx_receive_abort_message(J1939CanContext* _PAGED context, J1939CanFrame* _PAGED can_msg)
{
    J1939Result res = J1939_OK;
    J1939CanTranPro can_tp;
    
    if(context == NULL || can_msg == NULL) return J1939_ERR_PTR;
    
    can_tp.pgn = ((unsigned long)can_msg->data[7]<<16)+((unsigned int)can_msg->data[6]<<8)+can_msg->data[5];
    if((can_msg->id.data[3] == context->send_tp_index->dest_addr)&&
        (can_tp.pgn==context->send_tp_index->pgn))
    {
        J1939_tl_clear_tp_index(context->send_tp_index);
        
        context->send_tp_index->status = STATUS_TP_ABORT;
    }
    
    return res;
}

J1939Result J1939_tl_rx_receive_dt_message(J1939CanContext* _PAGED context, J1939CanFrame* _PAGED can_msg)
{
    J1939Result res = J1939_OK;
    unsigned char tp_index;
    
    if(context == NULL || can_msg == NULL) return J1939_ERR_PTR;
    
    tp_index = J1939_dl_search_tp_with_source(context, can_msg->id.data[3]);
    
    res = J1939_tl_rx_receive_dt_frame(context, can_msg, tp_index);
    
    if(res != J1939_OK) return res;
    
    if(context->rec_temp_mess_buff->buffer[tp_index].type == kBroadcastType)
        res = J1939_tl_rx_prepare_to_next_broadcast_dt_frame(context, tp_index);
    else
        res = J1939_tl_rx_prepare_to_next_normal_dt_frame(context, tp_index);
    
    return res;
}

J1939Result J1939_tl_rx_receive_dt_frame(J1939CanContext* _PAGED context, J1939CanFrame* _PAGED can_msg, INT16U tp_index)
{
    J1939Result res = J1939_OK;
    
    if(context == NULL || can_msg == NULL) return J1939_ERR_PTR;
    
    if(tp_index >= context->rec_tp_buff->tp_max) return J1939_ERR;
    
    if(context->rec_tp_buff->buffer[tp_index].tp.status != STATUS_TP_DT) return J1939_ERR;
	
	context->rec_tp_buff->buffer[tp_index].tp.frame_index = can_msg->data[0];
	
	if(context->rec_tp_buff->buffer[tp_index].tp.frame_index > context->rec_tp_buff->buffer[tp_index].tp.stop_frame) return J1939_ERR;
	
	if(context->rec_tp_buff->buffer[tp_index].tp.frame_index == 0) return J1939_ERR;
	
	if((can_msg->data[0]-1)*7 + can_msg->byte_cnt - 1 >= context->rec_temp_mess_buff->buffer[tp_index].byte_max) return J1939_ERR; //接收缓存溢出
	
	context->rec_tp_buff->buffer[tp_index].tp.rec_sts |= ((unsigned int)1 << 
	                                                    (INT8U)((context->rec_tp_buff->buffer[tp_index].tp.frame_index-
	                                                    context->rec_tp_buff->buffer[tp_index].tp.start_frame)));
	(void)J1939MemCopy(&context->rec_temp_mess_buff->buffer[tp_index].data[(can_msg->data[0]-1)*7],
		&can_msg->data[1], can_msg->byte_cnt-1);
    context->rec_temp_mess_buff->buffer[tp_index].byte_cnt += can_msg->byte_cnt;
    context->rec_temp_mess_buff->buffer[tp_index].id.value = can_msg->id.value;	
	
	return J1939_OK;
}

J1939Result J1939_tl_rx_prepare_to_next_normal_dt_frame(J1939CanContext* _PAGED context, INT16U tp_index)
{
    J1939Result res = J1939_OK;
    J1939CanFrame can_msg;
    
    if(context == NULL) return J1939_ERR_PTR;
    if(tp_index >= context->rec_tp_buff->tp_max) return J1939_ERR;
    if(context->rec_tp_buff->buffer[tp_index].tp.frame_index == 0) return J1939_ERR;
    
	if(context->rec_tp_buff->buffer[tp_index].tp.frame_index == context->rec_tp_buff->buffer[tp_index].tp.stop_frame)
	{   //本次DT最后一帧
	 	if(context->rec_tp_buff->buffer[tp_index].tp.rec_sts == G_TP_REC_STS[context->rec_tp_buff->buffer[tp_index].tp.stop_frame-
	 	                                                        context->rec_tp_buff->buffer[tp_index].tp.start_frame])
	 	{   //所有帧已接收
		 	if(context->rec_tp_buff->buffer[tp_index].tp.frame_index == context->rec_tp_buff->buffer[tp_index].tp.frame_cnt)
			{   //传输完成,发送EOM
			 	context->rec_temp_mess_buff->buffer[tp_index].byte_cnt = context->rec_tp_buff->buffer[tp_index].tp.byte_cnt;
				J1939_tl_config_eom(&can_msg, &context->rec_tp_buff->buffer[tp_index].tp);
				res = J1939_dl_write_buff(context->send_frame_buff, &can_msg);
                context->rec_tp_buff->buffer[tp_index].tp.status = STATUS_TP_EOM;
			}
			else//未完成，发送CTS
			{
                context->rec_tp_buff->buffer[tp_index].tp.start_frame = context->rec_tp_buff->buffer[tp_index].tp.stop_frame + 1;
                context->rec_tp_buff->buffer[tp_index].tp.frame_index = context->rec_tp_buff->buffer[tp_index].tp.start_frame;
                if(context->rec_tp_buff->buffer[tp_index].tp.frame_cnt - context->rec_tp_buff->buffer[tp_index].tp.stop_frame > J1939_TP_DT_CNT_MAX)
                {
                    context->rec_tp_buff->buffer[tp_index].tp.stop_frame = context->rec_tp_buff->buffer[tp_index].tp.start_frame + J1939_TP_DT_CNT_MAX-1;    
                }
                else
                {
                    context->rec_tp_buff->buffer[tp_index].tp.stop_frame = context->rec_tp_buff->buffer[tp_index].tp.frame_cnt;
                }
                J1939_tl_config_cts(&can_msg, &context->rec_tp_buff->buffer[tp_index].tp);
                res = J1939_dl_write_buff(context->send_frame_buff, &can_msg);
                context->rec_tp_buff->buffer[tp_index].tp.status = STATUS_TP_DT;
            }
	 	}
	 	else//有漏帧，重新请求数据
	 	{
	 	    context->rec_tp_buff->buffer[tp_index].tp.frame_index = context->rec_tp_buff->buffer[tp_index].tp.start_frame;
	 	    J1939_tl_config_cts(&can_msg, &context->rec_tp_buff->buffer[tp_index].tp);
            res = J1939_dl_write_buff(context->send_frame_buff, &can_msg);
            context->rec_tp_buff->buffer[tp_index].tp.status = STATUS_TP_DT;
	 	}
	 	context->rec_tp_buff->buffer[tp_index].tp.rec_sts = 0;
	}
	
	return res;
}

J1939Result J1939_tl_rx_prepare_to_next_broadcast_dt_frame(J1939CanContext* _PAGED context, INT16U tp_index)
{
    J1939Result res = J1939_OK;
    
    if(context == NULL) return J1939_ERR_PTR;
    if(tp_index >= context->rec_tp_buff->tp_max) return J1939_ERR;
    if(context->rec_tp_buff->buffer[tp_index].tp.frame_index == 0) return J1939_ERR;
    
	if(context->rec_tp_buff->buffer[tp_index].tp.frame_index == context->rec_tp_buff->buffer[tp_index].tp.stop_frame)
	{   //本次DT最后一帧
	 	if(context->rec_tp_buff->buffer[tp_index].tp.rec_sts == G_TP_REC_STS[context->rec_tp_buff->buffer[tp_index].tp.stop_frame-
	 	                                                        context->rec_tp_buff->buffer[tp_index].tp.start_frame])
	 	{   //所有帧已接收
		 	if(context->rec_tp_buff->buffer[tp_index].tp.frame_index == context->rec_tp_buff->buffer[tp_index].tp.frame_cnt)
			{   //传输完成
			 	context->rec_temp_mess_buff->buffer[tp_index].byte_cnt = context->rec_tp_buff->buffer[tp_index].tp.byte_cnt;
				context->rec_tp_buff->buffer[tp_index].tp.status = STATUS_TP_EOM;
			}
	 	}
	 	else//有漏帧,忽略本次数据
	 	{
	 	    context->rec_tp_buff->buffer[tp_index].tp.frame_index = context->rec_tp_buff->buffer[tp_index].tp.start_frame;
	 	    context->rec_tp_buff->buffer[tp_index].tp.status = STATUS_TP_IDLE;
	 	}
	 	context->rec_tp_buff->buffer[tp_index].tp.rec_sts = 0;
	}
	
	return res;
}

J1939Result J1939_tl_rx_receive_pdu_message(J1939CanContext* _PAGED context, J1939CanFrame* _PAGED can_msg)
{
    J1939Result res = J1939_OK;
    
    if(context == NULL || can_msg == NULL) return J1939_ERR_PTR;
    
    res = J1939_dl_write_buff(context->rec_pdu_buff, can_msg);
                    
    #if J1939_OS_EVENT_EN > 0
    if((res==J1939_OK)&&(context->receive_event_sem->use_flag == 1))
        J1939SendSem(context->receive_event_sem->event);
    #endif

    return res;
}
//函数名称：J1939_clear_send_message               
//功    能：J1939清零发送缓存
//参    数：send_index 待操作的参数指针
//返 回 值：无
//备    注：
void J1939_clear_send_message(J1939SendMessageBuff* _PAGED send_index)
{
    if(send_index == NULL)
        return;
    
    send_index->byte_cnt = 0;
    //send_index->dest_addr = 0;
    send_index->id.value = 0;
    send_index->pgn = 0;
    //send_index->pri = 0;
}
//函数名称：J1939_clear_rec_message               
//功    能：J1939清零接收缓存
//参    数：rec_index 待操作的参数指针
//返 回 值：无
//备    注：
void J1939_clear_rec_message(J1939RecMessage* _PAGED rec_index)
{
    if(rec_index == NULL)
        return;
    
    rec_index->byte_cnt = 0;
    rec_index->id.value = 0;
    rec_index->pgn = 0;
    rec_index->user_data = 0;
}
//函数名称：J1939_tl_clear_tp_index               
//功    能：J1939传输协议传输eom帧数据配置函数
//参    数：tp_index 待操作的参数指针
//返 回 值：无
//备    注：
void J1939_tl_clear_tp_index(J1939CanTranPro* _PAGED tp_index)
{
    if(tp_index == NULL)
        return;
    
    tp_index->pgn = 0;
    tp_index->frame_cnt = 0;
    tp_index->byte_cnt = 0;
    tp_index->source_addr = 0;
    tp_index->dest_addr = 0;
    tp_index->delay = 0;
    tp_index->start_frame = 0;
    tp_index->frame_index = 0;
    tp_index->stop_frame = 0;
    tp_index->rec_sts = 0;   
}

//函数名称：J1939_tl_config_eom               
//功    能：J1939传输协议传输eom帧数据配置函数
//参    数：g_canFrame 待配置的数据帧
//          tp_index  tp参数指针
//返 回 值：无
//备    注：
void J1939_tl_config_eom(J1939CanFrame* _PAGED frame, J1939CanTranPro* _PAGED tp_index)
{
    if(frame == NULL)
        return;
    if(tp_index == NULL)
        return;
    
    frame->id.value = TP_CM_ID_BASE;
    frame->id.data[2] = tp_index->source_addr;
    frame->id.data[3] = tp_index->dest_addr;
    frame->byte_cnt = 8;
    frame->data[0] = TP_CMD_EOM;
    frame->data[1] = (INT8U)(tp_index->byte_cnt);
    frame->data[2] = tp_index->byte_cnt >> 8;
    frame->data[3] = tp_index->frame_cnt;
    frame->data[4] = 0xFF;
    frame->data[5] = (INT8U)(tp_index->pgn);
    frame->data[6] = (INT8U)(tp_index->pgn >> 8);
    frame->data[7] = (INT8U)(tp_index->pgn >> 16);
    
}
//函数名称：J1939_tl_config_cts               
//功    能：J1939传输协议传输cts帧数据配置函数
//参    数：frame 待配置的数据帧
//          tp_index  tp参数指针
//返 回 值：无
//备    注：
void J1939_tl_config_cts(J1939CanFrame* _PAGED frame, J1939CanTranPro* _PAGED tp_index)
{
    if(frame == NULL)
        return;
    if(tp_index == NULL)
        return;
    
    frame->id.value = TP_CM_ID_BASE;
    frame->id.data[2] = tp_index->source_addr;
    frame->id.data[3] = tp_index->dest_addr;
    frame->byte_cnt = 8;
    frame->data[0] = TP_CMD_CTS;
    frame->data[1] = tp_index->stop_frame - tp_index->start_frame + 1;
    frame->data[2] = tp_index->start_frame;
    frame->data[3] = 0xFF;
    frame->data[4] = 0xFF;
    frame->data[5] = (INT8U)(tp_index->pgn);
    frame->data[6] = (INT8U)(tp_index->pgn >> 8);
    frame->data[7] = (INT8U)(tp_index->pgn >> 16);
}
//函数名称：J1939_tl_config_abort               
//功    能：J1939传输协议传输abort帧数据配置函数
//参    数：frame 待配置的数据帧
//          tp_index  tp参数指针
//返 回 值：无
//备    注：
void J1939_tl_config_abort(J1939CanFrame* _PAGED frame, J1939CanTranPro* _PAGED tp_index)
{
    if(frame == NULL)
        return;
    if(tp_index == NULL)
        return;
    
    frame->id.value = TP_CM_ID_BASE;
    frame->id.data[2] = tp_index->source_addr;
    frame->id.data[3] = tp_index->dest_addr;
    frame->byte_cnt = 8;
    frame->data[0] = TP_CMD_ABORT;
    frame->data[1] = 0xFF;
    frame->data[2] = 0xFF;
    frame->data[3] = 0xFF;
    frame->data[4] = 0xFF;
    frame->data[5] = (INT8U)(tp_index->pgn);
    frame->data[6] = (INT8U)(tp_index->pgn >> 8);
    frame->data[7] = (INT8U)(tp_index->pgn >> 16);          
}
//函数名称：J1939_tl_config_rts               
//功    能：J1939传输协议传输rts帧数据配置函数
//参    数：frame 待配置的数据帧
//          tp_index  tp参数指针
//返 回 值：无
//备    注：
void J1939_tl_config_rts(J1939CanFrame* _PAGED frame, J1939CanTranPro* _PAGED tp_index)
{
    if(frame == NULL)
        return;
    if(tp_index == NULL)
        return;
    
    frame->id.value = TP_CM_ID_BASE;
    frame->id.data[2] = tp_index->dest_addr; 
    frame->id.data[3] = tp_index->source_addr;
    frame->byte_cnt = 8;
    frame->data[0] = TP_CMD_RTS;
    frame->data[1] = (INT8U)(tp_index->byte_cnt);
    frame->data[2] = tp_index->byte_cnt >> 8;
    frame->data[3] = tp_index->frame_cnt;
    frame->data[4] = 0xFF;
    frame->data[5] = (INT8U)(tp_index->pgn);
    frame->data[6] = (INT8U)(tp_index->pgn >> 8);
    frame->data[7] = (INT8U)(tp_index->pgn >> 16);
}
//函数名称：J1939_tl_config_bam               
//功    能：J1939传输协议传输bam帧数据配置函数
//参    数：frame 待配置的数据帧
//          tp_index  tp参数指针
//返 回 值：无
//备    注：
void J1939_tl_config_bam(J1939CanFrame* _PAGED frame, J1939CanTranPro* _PAGED tp_index)
{
    if(frame == NULL)
        return;
    if(tp_index == NULL)
        return;
    
    frame->id.value = TP_CM_ID_BASE;
    frame->id.data[2] = tp_index->dest_addr; 
    frame->id.data[3] = tp_index->source_addr;
    frame->byte_cnt = 8;
    frame->data[0] = TP_CMD_BAM;
    frame->data[1] = (INT8U)(tp_index->byte_cnt);
    frame->data[2] = tp_index->byte_cnt >> 8;
    frame->data[3] = tp_index->frame_cnt;
    frame->data[4] = 0xFF;
    frame->data[5] = (INT8U)(tp_index->pgn);
    frame->data[6] = (INT8U)(tp_index->pgn >> 8);
    frame->data[7] = (INT8U)(tp_index->pgn >> 16);
}

#endif
