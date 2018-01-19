/*******************************************************************************
**                       安徽力高新能源有限公司 Copyright (c)
**                            http://www.ligoo.cn/ 
**文 件 名J1939_hal.c                                       
**作    者：董丽伟
**创建日期：2012.05.25
**修改记录：
**文件说明:CAN通信标准J1939硬件抽象层驱动源文件
**版    本:V1.0
**备    注：
*******************************************************************************/
#ifndef J1939_HAL_C_
#define J1939_HAL_C_

#include "j1939_cfg.h"
#include "bms_upper_computer_j1939.h"
#define   J1939_HAL_GLOBALS

#pragma MESSAGE DISABLE C2705

can_t g_can_info[kCanDevMaxCount]={NULL};//临时保存CAN初始化的返回指针，仅用于本程序

J1939Result J1939CanCheckRecID(J1939CanContext* _PAGED j1939_context, INT8U dest_id);

//函数名称：J1939HalWithBuffInit
//功    能：硬件驱动层的CAN模块初始化                         
//参    数：context CAN通信用的数据变量指针
//返 回 值：初始化成功则返回J1939_OK，否则返回失败原因
//备    注：
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
    //需要在应用层增加对0xFF目的地址接受的支持
#endif

	g_can_info[context->can_info->dev] = can_init(&can_info);//TODO:后期修改为_PAGED
    if(g_can_info[context->can_info->dev])
    	return J1939_OK;
}

//函数名称：J1939CanSendFrame
//功    能：硬件驱动层的CAN模块发送一帧数据                         
//参    数：can_info CAN通信用的数据变量指针 
//          msg 待发送的消息指针
//返 回 值：成功返回0，失败返回对应值
//备    注：直接发送一帧数据

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

//函数名称：J1939CanRecFrame
//功    能：硬件驱动层的CAN模块接收一帧数据                         
//参    数：can_info CAN通信用的数据变量指针 
//          msg 接收消息缓存指针
//          timeout 接收超时时间
//返 回 值：成功返回0，失败返回对应值
//备    注：此函数要能实现当CAN设备收到一帧数据后直接调用此函数将数据帧放入接收缓存，
//          因此必要的话可以将此函数放入接收中断中。
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
