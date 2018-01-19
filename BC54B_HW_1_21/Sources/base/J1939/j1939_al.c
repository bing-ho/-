/*******************************************************************************
**                       ������������Դ���޹�˾ Copyright (c)
**                            http://www.ligoo.cn/ 
**�� �� ���Sj1939.c                                       
**��    �ߣ�����ΰ
**�������ڣ�2012.05.25
**�޸ļ�¼��
**�ļ�˵��:CANͨ�ű�׼J1939Ӧ�ò�����Դ�ļ�
**��    ��:V1.0
**��    ע��ʹ�÷�����
**          1.���j1939_hal��ĺ���������ʹCAN�豸�����������ͽ���CAN֡������J1939Init
**            ������ʼ��CAN�豸����ȡCAN�����������Ҫ�Զ��建�棬����Ҫ�����л��涨��
**            ����ָ�븳�ڽṹ���ָ�롣��Ҫ����Ļ����ͷal�ļ���
**          2.J1939CanHeartBeat�����Ե��ã���1msΪ��λ��
**          3.�����Ե��ú���J1939CanPoll�����ͺͽ������ݣ�As fast as you can!���鲻Ҫ��
**            ��5ms�����ⷢ�����ݶ�ʧ��
**          4.ʹ��J1939CanSendMessage�������κγ��ȵ����ݣ���Ȼ���ܳ�������ĳ������ֵ��
**          5.ʹ��J1939CanRecMessageWithWait����ȡ��Ϣ���ݡ�
*******************************************************************************/
#ifndef J1939_AL_C_
#define J1939_AL_C_

#pragma MESSAGE DISABLE C1860
#pragma MESSAGE DISABLE C12056

#include "j1939_cfg.h"

#define J1939_AL_GLOBALS

#define J1939_GET_PRI_FROM_ID(ID)   (ID >> 26 & 0x07)

J1939CanTranPro* _PAGED J1939GetFreeSendTP(J1939CanContext* _PAGED context);

//�������ƣ�J1939InitWithBuff               
//��    �ܣ�J1939Э���ʼ��
//��    ����can_info  CANͨ���õ����ݱ���ָ��
//�� �� ֵ������CAN����������ָ��
//��    ע��
J1939Result J1939InitWithBuff(J1939CanContext* _PAGED context, J1939CanInfo* _PAGED can_info)
{
	J1939Result res;

	if(can_info == NULL)
	    return J1939_ERR_PTR;
	if(context == NULL)
		return J1939_ERR_PTR;
	context->send_event_sem->use_flag = 0;   //���ڷ���TPʱ��ֵΪ1����ʾ�ź�����ռ��
	context->receive_event_sem->use_flag = 1;//Ĭ��ʹ�ý����ź���

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

//�������ƣ�J1939UnInit               
//��    �ܣ�J1939Э�鷴��ʼ��
//��    ����context  CANͨ���õ����ݱ���ָ��
//�� �� ֵ������ʼ���ɹ��򷵻�J1939_OK�����򷵻�ʧ��ԭ��
//��    ע��
J1939Result J1939UnInit(J1939CanContext* _PAGED context)
{
    if(context ==NULL)
        return J1939_ERR_PTR;
    
    return J1939_OK;
}
//�������ƣ�J1939CanPoll               
//��    �ܣ�J1939���ڴ�����
//��    ����context  CAN�豸�����ݱ���ָ��
//�� �� ֵ����
//��    ע���˺�����Ҫ�����Եĵ��ã���������������CAN���ͺͽ��յ�����
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

//�������ƣ�J1939CanHeartBeat               
//��    �ܣ�J1939��������
//��    ����context  CAN�豸�����ݱ���ָ��
//�� �� ֵ����
//��    ע���˺�����Ҫ1ms�ĵ��ã���Ϊ������������
void J1939CanHeartBeat(J1939CanContext* _PAGED context)
{
    if(context == NULL)
        return;
    context->heart_beat++;   
}

//�������ƣ�J1939CanSendMessage               
//��    �ܣ�J1939������Ϣ����
//��    ����context  CAN�豸�����ݱ���ָ��
//          msg �����͵���Ϣ��ָ��
//�� �� ֵ���ɹ��򷵻�J1939_OK�����򷵻�ʧ��ԭ��
//��    ע���˺�����Ҫ�����Եĵ��ã���������������CAN���ͺͽ��յ�����
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
//�������ƣ�J1939CanSendMessageWithWait               
//��    �ܣ�J1939������Ϣ���ȴ����ͽ������
//��    ����context  CAN�豸�����ݱ���ָ��
//          msg �����͵���Ϣ��ָ��
//          timeout �ȴ���ʱʱ��
//�� �� ֵ���ɹ��򷵻�J1939_TP_FINISH,���򷵻�ʧ��ԭ��
//��    ע���˺�����Ҫ�����Եĵ��ã���������������CAN���ͺͽ��յ�����
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

//�������ƣ�J1939CanSendMessageWithFinish
//��    �ܣ�J1939������Ϣ���ȴ�������ɺ���
//��    ����context  CAN�豸�����ݱ���ָ��
//          msg �����͵���Ϣ��ָ��
//          timeout �ȴ���ʱʱ��
//�� �� ֵ���ɹ��򷵻�J1939_OK�����򷵻�ʧ��ԭ��
//��    ע���˺�����Ҫ�����Եĵ��ã���������������CAN���ͺͽ��յ�����
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

//�������ƣ�J1939CanRecMessageWithWait               
//��    �ܣ�J1939������Ϣ����
//��    ����context  CAN�豸�����ݱ���ָ��
//          msg ���յ���Ϣ��ָ��
//          timeout ��ʱʱ��
//�� �� ֵ���ɹ��򷵻�J1939_OK�����򷵻�ʧ��ԭ��
//��    ע��
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

//�������ƣ�J1939CanFrameCopy
//��    �ܣ�J1939֡���ݸ���
//��    ����dest:Ŀ���ַ
//	        sour:Դ��ַ
//�� �� ֵ���ɹ��򷵻�J1939_OK�����򷵻�ʧ��ԭ��
//��    ע��
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
//�������ƣ�J1939MemCopy
//��    �ܣ�J1939�ڴ渴��
//��    ����dest:Ŀ���ַ
//	        sour:Դ��ַ
//          len:����
//�� �� ֵ���ɹ��򷵻�J1939_OK�����򷵻�ʧ��ԭ��
//��    ע��
J1939Result J1939MemCopy(INT8U* _PAGED dest, INT8U* _PAGED source, INT16U len)
{
    unsigned long i;

	if((source == NULL)||(dest == NULL))
        return J1939_ERR_PTR;

    for(i=0; i<len; i++)
    	dest[i] = source[i];
    
    return J1939_OK;
}
//�������ƣ�J1939MemSet
//��    �ܣ�J1939�ڴ渳ֵ
//��    ����dest:Ŀ���ַ
//	        sour:Դ��ַ
//          len:����
//�� �� ֵ���ɹ��򷵻�J1939_OK�����򷵻�ʧ��ԭ��
//��    ע��
J1939Result J1939MemSet(INT8U* _PAGED dest, INT8U val, INT16U len)
{
    unsigned long i;

	if(dest == NULL)
        return J1939_ERR_PTR;

    for(i=0; i<len; i++)
    	dest[i] = val;

    return J1939_OK;
}
//�������ƣ�J1939CanSendMessageCopy
//��    �ܣ�J1939������Ϣ���ݸ���
//��    ����dest:Ŀ���ַ
//	   sour:Դ��ַ
//�� �� ֵ���ɹ��򷵻�J1939_OK�����򷵻�ʧ��ԭ��
//��    ע��
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

//�������ƣ�J1939CanRecMessageCopy
//��    �ܣ�J1939������Ϣ���ݸ���
//��    ����dest:Ŀ���ַ
//	   sour:Դ��ַ
//�� �� ֵ���ɹ��򷵻�J1939_OK�����򷵻�ʧ��ԭ��
//��    ע��
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

//�������ƣ�J1939CanGetHeartBeat               
//��    �ܣ�J1939��ȡ��ǰ������ʱֵ
//��    ����context  CAN�豸�����ݱ���ָ��
//�� �� ֵ����ǰ��������
//��    ע��
unsigned int J1939CanGetHeartBeat(J1939CanContext* _PAGED context)
{
    if(context == NULL)
        return 0xFFFF;
    return context->heart_beat;   
}
//�������ƣ�J1939CanSetHeartBeat               
//��    �ܣ�J1939���õ�ǰ������ʱֵ
//��    ����context  CAN�豸�����ݱ���ָ��
//          cnt Ҫ���õ�������ʱֵ
//�� �� ֵ����
//��    ע��
void J1939CanSetHeartBeat(J1939CanContext* _PAGED context, unsigned int cnt)
{
    if(context == NULL)
        return;
    context->heart_beat = cnt;
}
#if J1939_OS_EVENT_EN > 0
//�������ƣ�J1939CreateSem              
//��    �ܣ�J1939�����ź���
//��    ����sem 
//          �ź����ĳ�ֵ
//�� �� ֵ���������ź���
//��    ע��
J1939_EVENT* J1939CreateSem(unsigned char data)
{
    return OSSemCreate(data);
}

//�������ƣ�J1939SendSem              
//��    �ܣ�J1939�����ź���
//��    ����sem �����͵��ź���
//�� �� ֵ����
//��    ע��
void J1939SendSem(J1939_EVENT* _PAGED sem)
{
    (void)OSSemPost(sem);
}
//�������ƣ�J1939PendSem              
//��    �ܣ�J1939�ȴ��ź���
//��    ����sem �ȴ����ź���
//          timeout ��ʱʱ��
//          err �ȴ��������ʱ�򷵻س�ʱ����
//�� �� ֵ����
//��    ע��
void J1939PendSem(J1939_EVENT* _PAGED sem, unsigned int timeout, unsigned char* err)
{
    if(sem == NULL)
        return;
    OSSemPend(sem, timeout, err);   
}
//�������ƣ�J1939Sleep              
//��    �ܣ�J1939��ʱ
//��    ����time ��ʱʱ�� ��λ��ms
//�� �� ֵ����
//��    ע���������ʽ��ʱ�������Ӳ��ʱ
void J1939Sleep(unsigned int time)
{
    OSTimeDly(time);   
}

//�������ƣ�J1939AddReceiveID              
//��    �ܣ�J1939�����ӽ���ID
//��    ����context J1939��� id�������ӵĽ���ID
//�� �� ֵ���ɹ��򷵻�J1939_OK�����򷵻�ʧ��ԭ��
//��    ע��
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
    //���½���ID
    item->flag = 1;
    item->next = NULL;
    item->start_addr = (INT8U)(id >> 8);
    item->stop_addr = item->start_addr;
    //��������
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
