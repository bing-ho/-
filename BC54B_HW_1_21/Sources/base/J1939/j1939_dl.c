/*******************************************************************************
**                       ������������Դ���޹�˾ Copyright (c)
**                            http://www.ligoo.cn/ 
**�� �� ���Sj1939_dl.c                                       
**��    �ߣ�����ΰ
**�������ڣ�2012.05.25
**�޸ļ�¼��
**�ļ�˵��:CANͨ�ű�׼J1939������·������Դ�ļ�
**��    ��:V1.0
**��    ע��
*******************************************************************************/
#ifndef J1939_DL_C_
#define J1939_DL_C_

#pragma MESSAGE DISABLE C1860
#pragma MESSAGE DISABLE C12056

#include "j1939_cfg.h"
#include "run_mode.h"

#define J1939_DL_GLOBALS


//�������ƣ�J1939_dl_with_buff_Init               
//��    �ܣ�J1939���ݻ����ʼ��
//��    ����context  CANͨ���õ����ݱ���ָ��
//�� �� ֵ����ʼ���ɹ��򷵻�J1939_OK�����򷵻�ʧ��ԭ��
//��    ע��
J1939Result J1939_dl_with_buff_Init(J1939CanContext* _PAGED context)
{
	if(context == NULL)
		return J1939_ERR_PTR;
	//��ʼ������֡����
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
	
	//��ʼ������PDU����
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
	//��ʼ������֡����
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
	
	//��ʼ��������Ϣ����
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
	
	//��ʼ��������Ϣ����
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
//�������ƣ�J1939_dl_write_buff               
//��    �ܣ�J1939д����֡����
//��    ����can_buff Ҫд��Ļ����ָ��
//          msg  ��д���CAN֡��ָ��
//�� �� ֵ���ɹ��򷵻�J1939_OK�����򷵻�ʧ��ԭ��
//��    ע��������֡����ָ�����棬�����ȳ�ԭ��
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

//�������ƣ�J1939_dl_read_buff               
//��    �ܣ�J1939������֡����
//��    ����can_buff Ҫ��ȡ�Ļ����ָ��
//          msg  ��ȡ��CAN֡�ı���λ��ָ��
//�� �� ֵ���ɹ��򷵻�J1939_OK�����򷵻�ʧ��ԭ��
//��    ע����ָ�������ȡCAN����֡�������ȳ�ԭ��
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
//�������ƣ�J1939_dl_write_message               
//��    �ܣ�J1939д��Ϣ����
//��    ����souce Ҫд�����Ϣ��ָ��
//          dest  ��д�����Ϣ��ָ��
//�� �� ֵ���ɹ��򷵻�J1939_OK�����򷵻�ʧ��ԭ��
//��    ע������Ϣ����ָ�����棬�����ȳ�ԭ��
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
//�������ƣ�J1939_dl_read_message               
//��    �ܣ�J1939����Ϣ����
//��    ����source Ҫ��ȡ����Ϣ��ָ��
//          dest  ��ȡ����Ϣ�ı���λ��ָ��
//�� �� ֵ���ɹ��򷵻�J1939_OK�����򷵻�ʧ��ԭ��
//��    ע����ָ�������ȡ��Ϣ�������ȳ�ԭ��
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

//�������ƣ�J1939_dl_send_poll               
//��    �ܣ�J1939ʵʱ��ѯ����������֡
//��    ����context CAN���豸�����ָ��
//�� �� ֵ����
//��    ע�������Ժ���

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
//�������ƣ�J1939_dl_get_tp_chain               
//��    �ܣ�J1939��ȡ���е�tp
//��    ����context CAN���豸�����ָ��
//�� �� ֵ��tp ����tp�ı��
//��    ע��
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
//�������ƣ�J1939_dl_put_tp_chain               
//��    �ܣ�J1939�ͷ�tp�����е�tp��
//��    ����context CAN���豸�����ָ��
//          tp Ҫ�ͷŵ�TP������
//�� �� ֵ����
//��    ע��
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
//�������ƣ�J1939_dl_search_tp_with_source               
//��    �ܣ�J1939����Դ��ַ����tp��
//��    ����context CAN���豸�����ָ��
//          addr Դ��ַ
//�� �� ֵ����ӦԴ��ַ��tp��������δ�ҵ���ӦTP����0xFF
//��    ע��
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
