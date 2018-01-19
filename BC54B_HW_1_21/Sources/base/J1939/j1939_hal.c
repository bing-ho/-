/*******************************************************************************
**                       ������������Դ���޹�˾ Copyright (c)
**                            http://www.ligoo.cn/ 
**�� �� ���SJ1939_hal.c                                       
**��    �ߣ�����ΰ
**�������ڣ�2012.05.25
**�޸ļ�¼��
**�ļ�˵��:CANͨ�ű�׼J1939Ӳ�����������Դ�ļ�
**��    ��:V1.0
**��    ע��
*******************************************************************************/
#ifndef J1939_HAL_C_
#define J1939_HAL_C_

#include "j1939_cfg.h"
#include "bms_upper_computer_j1939.h"
#define   J1939_HAL_GLOBALS

#pragma MESSAGE DISABLE C2705

can_t g_can_info[kCanDevMaxCount]={NULL};//��ʱ����CAN��ʼ���ķ���ָ�룬�����ڱ�����

J1939Result J1939CanCheckRecID(J1939CanContext* _PAGED j1939_context, INT8U dest_id);

//�������ƣ�J1939HalWithBuffInit
//��    �ܣ�Ӳ���������CANģ���ʼ��                         
//��    ����context CANͨ���õ����ݱ���ָ��
//�� �� ֵ����ʼ���ɹ��򷵻�J1939_OK�����򷵻�ʧ��ԭ��
//��    ע��
J1939Result J1939HalWithBuffInit(J1939CanContext* _PAGED context)
{
	CanInfo can_info;
	CanMessage msg;
	if(context == NULL)
		return J1939_ERR_PTR;
	can_info.buffers = &msg;
	can_info.buffer_count = 1;
	can_info.dev = context->can_info->dev;
	can_info.receive_id = context->can_info->receive_id.value;
	can_info.mask_id = context->can_info->mask_id.value;
	can_info.bps = context->can_info->bps;
	can_info.mode = context->can_info->id_style;
	can_info.receive_userdata = context->can_info->receive_userdata;
  can_info.receive_callback = context->can_info->receive_callback;
  can_info.filtemode=CanFilte32;
#if J1939_BROADCAST_MESSAGE_RECEIVE_SUPPORT > 0
    //��Ҫ��Ӧ�ò����Ӷ�0xFFĿ�ĵ�ַ���ܵ�֧��
#endif

	g_can_info[context->can_info->dev] = can_init(&can_info);//TODO:�����޸�Ϊ_PAGED
    if(g_can_info[context->can_info->dev])
    	return J1939_OK;
}

//�������ƣ�J1939CanSendFrame
//��    �ܣ�Ӳ���������CANģ�鷢��һ֡����                         
//��    ����can_info CANͨ���õ����ݱ���ָ�� 
//          msg �����͵���Ϣָ��
//�� �� ֵ���ɹ�����0��ʧ�ܷ��ض�Ӧֵ
//��    ע��ֱ�ӷ���һ֡����

J1939Result J1939CanSendFrame(J1939CanContext* _PAGED j1939_context, J1939CanFrame* _PAGED msg)
{
    CanMessage g_canMessage;
    
    if(msg == NULL)
        return J1939_ERR_MSG_PTR;
    if(j1939_context == NULL)
    {
        return J1939_ERR_PTR;   
    }
    
    g_canMessage.id.value = can_id_from_extend_id(msg->id.value);
    g_canMessage.len = msg->byte_cnt;
    //memcpy(g_canMessage.data, msg->data, msg->byte_cnt);
    (void)J1939MemCopy(g_canMessage.data, msg->data, msg->byte_cnt);
    return can_send(g_can_info[j1939_context->can_info->dev], &g_canMessage);
}

//�������ƣ�J1939CanRecFrame
//��    �ܣ�Ӳ���������CANģ�����һ֡����                         
//��    ����can_info CANͨ���õ����ݱ���ָ�� 
//          msg ������Ϣ����ָ��
//          timeout ���ճ�ʱʱ��
//�� �� ֵ���ɹ�����0��ʧ�ܷ��ض�Ӧֵ
//��    ע���˺���Ҫ��ʵ�ֵ�CAN�豸�յ�һ֡���ݺ�ֱ�ӵ��ô˺���������֡������ջ��棬
//          ��˱�Ҫ�Ļ����Խ��˺�����������ж��С�
J1939Result J1939CanRecFrame(J1939CanContext* _PAGED j1939_context, J1939CanFrame* _PAGED msg)
{
    J1939Result res = J1939_OK;
    
    if(msg == NULL)
    {
        return J1939_ERR_MSG_PTR;   
    }
    if(j1939_context == NULL)
    {
        return J1939_ERR_PTR;   
    }
    
    if(J1939CanCheckRecID(j1939_context, msg->id.data[2]) == J1939_OK)
        res = J1939_dl_write_buff(j1939_context->rec_frame_buff, msg);
    
    return res;
}

J1939Result J1939CanCheckRecID(J1939CanContext* _PAGED j1939_context, INT8U dest_id)
{
    J1939CanRecMaskGroupItem* _PAGED item = NULL;
    
    if(j1939_context == NULL)
    {
        return J1939_ERR_PTR;   
    }
    
    item = (J1939CanRecMaskGroupItem* _PAGED )&j1939_context->can_info->rec_mask_group_item;
    while(item)
    {
        if(item->flag == 1)
        {
            if(dest_id >= item->start_addr && dest_id <= item->stop_addr) return J1939_OK;
        }
        item = (J1939CanRecMaskGroupItem* _PAGED)item->next;
    }
    
    return J1939_ERR;
}


#endif
