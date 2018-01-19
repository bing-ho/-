/*******************************************************************************
**                       安徽力高新能源有限公司 Copyright (c)
**                            http://www.ligoo.cn/ 
**文 件 名j1939_dtc.c                                       
**作    者：董丽伟
**创建日期：2013.09.14
**修改记录：
**文件说明:CAN通信标准J1939应用层故障诊断驱动源文件
**版    本:V1.0
**备    注：
*******************************************************************************/
#ifndef J1939_DTC_C_
#define J1939_DTC_C_

#pragma MESSAGE DISABLE C4001  // Condition always FALSE
#pragma MESSAGE DISABLE C12056 //SP debug info incorrect because of optimization or inline assembler

#include "j1939_cfg.h"

/***************************************dtc_message_function declaration**************************************/
J1939Result j1939_dtc_trouble_on(J1939CanContext* _PAGED context, J1939_DTC_INFO* _PAGED dtc_info);
Bool j1939_dtc_dtc_send_check_with_item(J1939_DTC_ITEM* _PAGED item);
J1939Result j1939_dtc_trouble_on_update(J1939_DTC_ITEM* _PAGED item, J1939_DTC_INFO* _PAGED dtc_info);
J1939Result j1939_dtc_all_dtc_send(J1939CanContext* _PAGED context, INT16U* dtc_cnt);
void j1939_dtc_clear_dtc_one_second_cnt(J1939CanContext* _PAGED context);
void j1939_dtc_oc_last_second_update(J1939CanContext* _PAGED context);
J1939Result j1939_dtc_trouble_off(J1939CanContext* _PAGED context, J1939_DTC_INFO* _PAGED dtc_info);
J1939Result j1939_dtc_trouble_off_update(J1939_DTC_ITEM* _PAGED item, J1939_DTC_INFO* _PAGED dtc_info);
INT16U j1939_dtc_get_dtc_on_num(J1939CanContext* _PAGED context);
Bool j1939_dtc_dtc_none_send_check(J1939CanContext* _PAGED context);
J1939Result j1939_dtc_send_dtc_none(J1939CanContext* _PAGED context);

/***************************************dtc_item_function declaration*****************************************/
J1939_DTC_ITEM* _PAGED j1939_dtc_search_dtc_item(J1939CanContext* _PAGED context, J1939_DTC_INFO* _PAGED dtc_info);
J1939_DTC_ITEM*  _PAGED j1939_dtc_get_dtc_item_head(J1939CanContext* _PAGED context);
J1939_DTC_ITEM* _PAGED j1939_dtc_get_dtc_item_tail(J1939CanContext* _PAGED context);
J1939_DTC_ITEM*  _PAGED j1939_dtc_get_free_dtc_item(J1939CanContext* _PAGED context);
J1939Result j1939_dtc_reset_dtc_item(J1939_DTC_ITEM*  _PAGED item);
J1939Result j1939_dtc_add_dtc_item(J1939CanContext* _PAGED context, J1939_DTC_ITEM*  _PAGED item);
J1939Result j1939_dtc_remove_dtc_item(J1939CanContext* _PAGED context, J1939_DTC_ITEM*  _PAGED item);
J1939_DTC_ITEM*  _PAGED  j1939_dtc_add_dtc(J1939CanContext* _PAGED context, J1939_DTC_INFO* _PAGED dtc_info);
J1939Result j1939_dtc_release_dtc(J1939CanContext* _PAGED context, J1939_DTC_INFO* _PAGED dtc_info);


/***************************************dtc_message_function**************************************************/

J1939Result j1939_dtc_init(J1939CanContext* _PAGED context)
{
    if(context == NULL) return J1939_ERR;
    
    if(context->dtc_context == NULL) 
    {
#if J1939_AUTO_MALLOC_EN > 0
        context->dtc_context = (J1939_DTC_CONTEXT* _PAGED )MALLOC(sizeof(J1939_DTC_CONTEXT));
        if(context->dtc_context == NULL) return J1939_ERR;
        
        (void)J1939MemSet((INT8U* _PAGED)context->dtc_context, 0, sizeof(J1939_DTC_CONTEXT));
#else
        return J1939_ERR;
#endif
    }
    
    if(context->dtc_context->send_buff == NULL)
    {
#if J1939_AUTO_MALLOC_EN > 0
        context->dtc_context->send_buff = (INT8U* _PAGED )MALLOC(J1939_DTC_SEND_BUFF_MAX * sizeof(INT8U));
        if(context->dtc_context->send_buff == NULL) return J1939_ERR;
        
        context->dtc_context->send_byte_max = J1939_DTC_SEND_BUFF_MAX;
        (void)J1939MemSet((INT8U* _PAGED)context->dtc_context->send_buff, 0, J1939_DTC_SEND_BUFF_MAX*sizeof(INT8U));
#else
        return J1939_ERR;
#endif
    }
    
    if(context->dtc_context->dtc_item_buff == NULL)
    {
#if J1939_AUTO_MALLOC_EN > 0
        context->dtc_context->dtc_item_buff = (J1939_DTC_ITEM* _PAGED )MALLOC(J1939_DTC_MAX * sizeof(J1939_DTC_ITEM));
        if(context->dtc_context->dtc_item_buff == NULL) return J1939_ERR;
        
        context->dtc_context->dtc_item_max = J1939_DTC_MAX;
        (void)J1939MemSet((INT8U* _PAGED)context->dtc_context->dtc_item_buff, 0, J1939_DTC_MAX*sizeof(J1939_DTC_ITEM));
#else
        return J1939_ERR;
#endif
    }
    context->dtc_context->dtc_item_start = context->dtc_context->dtc_item_buff;
    context->dtc_context->is_start = TRUE;
    
    return J1939_OK;
}

J1939Result j1939_dtc_trouble_on(J1939CanContext* _PAGED context, J1939_DTC_INFO* _PAGED dtc_info)
{
    J1939Result res = J1939_OK;
    J1939_DTC_ITEM* _PAGED item;
    
    if(context == NULL || context->dtc_context == NULL || dtc_info == NULL) return J1939_ERR;
    
    item = j1939_dtc_add_dtc(context, dtc_info);
    
    if(item == NULL) return J1939_ERR;
    
    if(j1939_dtc_trouble_on_update(item, dtc_info) != J1939_OK) return J1939_OK;
    
    if(context->dtc_context->is_start && j1939_dtc_dtc_send_check_with_item(item))
    {
        res = j1939_dtc_all_dtc_send(context, NULL);
    
        if(res == J1939_OK) item->send_flag = 0;
        else context->dtc_context->send_failure_flag = 1;
    }
    context->dtc_context->occure_cnt_one_second++;
    
    return res;
}

J1939Result j1939_dtc_trouble_on_update(J1939_DTC_ITEM* _PAGED item, J1939_DTC_INFO* _PAGED dtc_info)
{
    INT8U oc, oc_last_second;
    INT32U now_tick, last_tick;
    
    if(item == NULL || dtc_info == NULL) return J1939_ERR;
    
    oc = item->info.oc;
    oc_last_second = item->oc_last_second;
    now_tick = get_tick_count();
    last_tick = item->last_tick;
    
    if(item->status == kDTCOn &&
        item->info.spn == dtc_info->spn &&
        item->info.fmi == dtc_info->fmi) return J1939_ERR;
    
    item->info.spn = dtc_info->spn;
    item->info.fmi = dtc_info->fmi;
    item->info.cm = dtc_info->cm;
    if(item->info.oc < J1939_OC_MAX) item->info.oc++;
    item->status = kDTCOn;
    item->last_tick = now_tick;
    
    if(oc_last_second == oc || //1秒内第一次触发故障
        oc_last_second == 0 || //此故障第一次发生
        now_tick % J1939_DTC_DM_BROADCAST_INTERVAL < J1939_DTC_DM_BROADCAST_INTERVAL - 10) //不与1秒发送冲突
    {
        item->send_flag = 1;
    }
    
    return J1939_OK;
}

Bool j1939_dtc_dtc_send_check_with_item(J1939_DTC_ITEM* _PAGED item)
{
    if(item == NULL) return FALSE;
    
    if(item->send_flag == 0) return FALSE;
    
    return TRUE;
}

Bool j1939_dtc_dtc_send_check(J1939CanContext* _PAGED context)
{
    J1939_DTC_ITEM* _PAGED item;
    
    if(context == NULL) return FALSE;
    
    item = context->dtc_context->dtc_item_start;
    while(item)
    {
        if(item->send_flag) return TRUE;
        
        item = (J1939_DTC_ITEM* _PAGED)item->next;
    }
    
    return FALSE;
}

J1939Result j1939_dtc_all_dtc_send(J1939CanContext* _PAGED context, INT16U* dtc_cnt)
{
    INT16U index = 0, dtc_on_cnt = 0;
    J1939_DTC_ITEM* _PAGED item;
    J1939SendMessageBuff send_buff;
    
    if(context == NULL) return J1939_ERR;
    
    send_buff.byte_max = context->dtc_context->send_byte_max;
    send_buff.data = context->dtc_context->send_buff;
    send_buff.id.value = ((INT32U)J1939_DM1_PRI << 26)+((INT32U)J1939_DM1_PGN << 8)+context->can_info->addr;
    send_buff.pgn = J1939_DM1_PGN;
    send_buff.type = kNormalType;
    
    WRITE_LT_INT8U_WITH_BUFF_SIZE(send_buff.data, index, 0xFF, send_buff.byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(send_buff.data, index, 0xFF, send_buff.byte_max);
    
    item = context->dtc_context->dtc_item_start;
    while(item)
    {
        if(item->status == kDTCOn)
        {
            dtc_on_cnt++;
            WRITE_DTC(send_buff, index, send_buff.byte_max, item->info.spn, item->info.fmi, item->info.oc, item->info.cm)
        }
        item = (J1939_DTC_ITEM* _PAGED)item->next;
    }
    if(dtc_cnt) *dtc_cnt = dtc_on_cnt;
    if(dtc_on_cnt == 0) return J1939_OK;
    
    send_buff.byte_cnt = index;
    if(index > J1939_CAN_FRAME_MAX)    
        send_buff.type = kBroadcastType;
    
    return J1939CanSendMessage(context, &send_buff);
}

J1939Result j1939_dtc_send_dtc_none(J1939CanContext* _PAGED context)
{
    INT16U index = 0;
    J1939SendMessageBuff send_buff;
    
    if(context == NULL) return J1939_ERR;
    
    send_buff.byte_max = context->dtc_context->send_byte_max;
    send_buff.data = context->dtc_context->send_buff;
    send_buff.id.value = ((INT32U)J1939_DM1_PRI << 26)+((INT32U)J1939_DM1_PGN << 8)+context->can_info->addr;
    send_buff.pgn = J1939_DM1_PGN;
    send_buff.type = kNormalType;
    
    WRITE_LT_INT8U_WITH_BUFF_SIZE(send_buff.data, index, 0xFF, send_buff.byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(send_buff.data, index, 0xFF, send_buff.byte_max);
    WRITE_DTC_NONE(send_buff, index, send_buff.byte_max)
    send_buff.byte_cnt = index;
    
    return J1939CanSendMessage(context, &send_buff);
}

J1939Result j1939_dtc_trouble_off(J1939CanContext* _PAGED context, J1939_DTC_INFO* _PAGED dtc_info)
{
    J1939Result res = J1939_OK;
    J1939_DTC_ITEM* _PAGED item;
    
    if(context == NULL || context->dtc_context == NULL || dtc_info == NULL) return J1939_ERR;
    
    item = j1939_dtc_search_dtc_item(context, dtc_info);
    
    if(item == NULL) return J1939_ERR;
    
    if(j1939_dtc_trouble_off_update(item, dtc_info) != J1939_OK) return J1939_OK;
    
    context->dtc_context->release_cnt_one_second++;
    if(context->dtc_context->is_start == 0) return J1939_OK;
    
    if(j1939_dtc_dtc_none_send_check(context)) 
    {
        res = j1939_dtc_send_dtc_none(context);
        context->dtc_context->release_cnt_one_second = 0;
    }
    else
    {
        if(context->dtc_context->occure_cnt_one_second == 0)
        {
            if(j1939_dtc_all_dtc_send(context, NULL) != J1939_OK)
                context->dtc_context->send_failure_flag = 1; //启动重发机制
        }
    }
    
    return res;
}

J1939Result j1939_dtc_trouble_off_update(J1939_DTC_ITEM* _PAGED item, J1939_DTC_INFO* _PAGED dtc_info)
{
    if(item == NULL || dtc_info == NULL) return J1939_ERR;
    
    if(item->status == kDTCOff &&
        item->info.spn == dtc_info->spn &&
        item->info.fmi == dtc_info->fmi) return J1939_ERR;
    
    item->info.spn = dtc_info->spn;
    item->info.fmi = dtc_info->fmi;
    item->info.cm = dtc_info->cm;
    item->status = kDTCOff;
    item->last_tick = get_tick_count();
    
    return J1939_OK;
}

INT16U j1939_dtc_get_dtc_on_num(J1939CanContext* _PAGED context)
{
    INT16U dtc_on_cnt = 0;
    J1939_DTC_ITEM* _PAGED item;
    
    if(context == NULL) return 0;
    
    item = context->dtc_context->dtc_item_start;
    while(item)
    {
        if(item->status == kDTCOn) dtc_on_cnt++;
        item = (J1939_DTC_ITEM* _PAGED)item->next;
    }
    
    return dtc_on_cnt;
}

Bool j1939_dtc_dtc_none_send_check(J1939CanContext* _PAGED context)
{
    if(context == NULL) return FALSE;
    
    if(j1939_dtc_get_dtc_on_num(context) != 0 ||
        context->dtc_context->occure_cnt_one_second != 0) return FALSE;
    
    return TRUE;
}

J1939Result j1939_dtc_clear_all_dtc_send_flag(J1939CanContext* _PAGED context)
{
    J1939_DTC_ITEM* _PAGED item;
    
    if(context == NULL) return J1939_ERR;
    
    item = context->dtc_context->dtc_item_start;
    while(item)
    {
        if(item->send_flag) item->send_flag = 0;
        item = (J1939_DTC_ITEM* _PAGED)item->next;
    }
    
    return J1939_OK;
}

void j1939_dtc_send_dm1_check(J1939CanContext* _PAGED context, INT32U tick)
{
    INT8U one_second_flag = 0;
    INT16U dtc_cnt;
    OS_CPU_SR cpu_sr = 0;
    
    if(context == NULL) return;
    
    if(context->dtc_context->last_tick == 0) context->dtc_context->last_tick = tick / 1000 * 1000;
    if(get_interval_by_tick(context->dtc_context->last_tick, tick) >= J1939_DTC_DM_BROADCAST_INTERVAL)
    {
        context->dtc_context->last_tick = get_tick_count();
        one_second_flag = 1;
    }
    if(context->dtc_context->is_start == 0) return;
    
    if(context->dtc_context->send_failure_flag == 1 ||
        one_second_flag)
    {
        OS_ENTER_CRITICAL();
        if(j1939_dtc_all_dtc_send(context, &dtc_cnt) != J1939_OK)
        {
            context->dtc_context->send_failure_flag = 1;
            OS_EXIT_CRITICAL();
            return;
        }
        (void)j1939_dtc_clear_all_dtc_send_flag(context);
        OS_EXIT_CRITICAL();
        context->dtc_context->send_failure_flag = 0;
        
        if(dtc_cnt == 0 && context->dtc_context->release_cnt_one_second && one_second_flag) 
        {
            (void)j1939_dtc_send_dtc_none(context);
            context->dtc_context->release_cnt_one_second = 0;
        }
    }
    if(one_second_flag)
    {
        j1939_dtc_clear_dtc_one_second_cnt(context);
        j1939_dtc_oc_last_second_update(context);
    }
}

void j1939_dtc_clear_dtc_one_second_cnt(J1939CanContext* _PAGED context)
{
    if(context == NULL) return;
    
    context->dtc_context->occure_cnt_one_second = 0;
    context->dtc_context->release_cnt_one_second = 0;
}

void j1939_dtc_oc_last_second_update(J1939CanContext* _PAGED context)
{
    J1939_DTC_ITEM* _PAGED item;
    
    if(context == NULL) return;
    
    item = (J1939_DTC_ITEM* _PAGED)context->dtc_context->dtc_item_start;
    while(item)
    {
        item->oc_last_second = item->info.oc;
        item = (J1939_DTC_ITEM* _PAGED)item->next;
    }
}

J1939Result j1939_dtc_reset_all_dtc(J1939CanContext* _PAGED context)
{
    J1939_DTC_ITEM* _PAGED item;
    
    if(context == NULL) return J1939_ERR;
    
    item = context->dtc_context->dtc_item_start;
    while(item)
    {
        if(item->status == kDTCOn) item->status = kDTCOff;
        item = (J1939_DTC_ITEM* _PAGED)item->next;
    }
    
    return J1939_OK;
}

void j1939_dtc_poll(J1939CanContext* _PAGED context, INT32U tick)
{
    if(context == NULL || context->dtc_context == NULL) return;
    
    if(context->dtc_context->is_start) j1939_dtc_send_dm1_check(context, tick);
}

/***************************************dtc_item_function*******************************************************/

J1939_DTC_ITEM* _PAGED j1939_dtc_search_dtc_item(J1939CanContext* _PAGED context, J1939_DTC_INFO* _PAGED dtc_info)
{
    J1939_DTC_ITEM* _PAGED item;
    
    if(context == NULL || dtc_info == NULL) return NULL;
    
    item = (J1939_DTC_ITEM* _PAGED)context->dtc_context->dtc_item_start;
    while(item)
    {
        if(item->info.spn == dtc_info->spn/* && item->info.fmi == dtc_info->fmi*/) break;
        
        item = (J1939_DTC_ITEM* _PAGED)item->next;
    }
    
    return item;
}

J1939_DTC_ITEM*  _PAGED j1939_dtc_get_dtc_item_head(J1939CanContext* _PAGED context)
{
    if(context == NULL) return NULL;
    
    return context->dtc_context->dtc_item_start;
}

J1939_DTC_ITEM* _PAGED j1939_dtc_get_dtc_item_tail(J1939CanContext* _PAGED context)
{
    J1939_DTC_ITEM* _PAGED item = NULL;
    
    if(context == NULL) return NULL;
    
    item = (J1939_DTC_ITEM* _PAGED)context->dtc_context->dtc_item_start;
    if(item == NULL) return NULL;
    
    while(item->next)
    {
        item = (J1939_DTC_ITEM* _PAGED)item->next;
    }
    
    return item;
}

J1939_DTC_ITEM*  _PAGED j1939_dtc_get_free_dtc_item(J1939CanContext* _PAGED context)
{
    INT8U i, len = context->dtc_context->dtc_item_max;
    
    if(context == NULL) return NULL;
    
    for(i=0; i<len; i++)
    {
        if(context->dtc_context->dtc_item_buff[i].status == kDTCNone) 
            return (J1939_DTC_ITEM*  _PAGED)(&context->dtc_context->dtc_item_buff[i]);
    }
    
    return NULL;
}

J1939Result j1939_dtc_reset_dtc_item(J1939_DTC_ITEM*  _PAGED item)
{
    if(item == NULL) return J1939_ERR;
    
    (void)J1939MemSet((INT8U* _PAGED)item, 0, sizeof(J1939_DTC_ITEM));
    
    return J1939_OK;
}

J1939Result j1939_dtc_add_dtc_item(J1939CanContext* _PAGED context, J1939_DTC_ITEM*  _PAGED item)
{
    J1939_DTC_ITEM* _PAGED tail_item;
    
    if(context == NULL || item == NULL) return J1939_ERR;
    
    tail_item = j1939_dtc_get_dtc_item_tail(context);
    if(tail_item == NULL) return J1939_ERR;
    
    if(tail_item != item) tail_item->next = (LIST_ITEM* _PAGED)item;
    item->next = NULL;
    context->dtc_context->dtc_item_cnt++;
    
    return J1939_OK;
}

J1939Result j1939_dtc_remove_dtc_item(J1939CanContext* _PAGED context, J1939_DTC_ITEM*  _PAGED item)
{
    J1939_DTC_ITEM*  _PAGED header_item;
    J1939_DTC_ITEM* _PAGED tail_item;
    
    if(context == NULL || item == NULL) return J1939_ERR;
    
    header_item = j1939_dtc_get_dtc_item_head(context);
    if(header_item == NULL) return J1939_ERR;
    
    tail_item = j1939_dtc_get_dtc_item_tail(context);
    if(tail_item == NULL) return J1939_ERR;
    
    while ((J1939_DTC_ITEM*  _PAGED)header_item->next != item && header_item->next != NULL)
        header_item = (J1939_DTC_ITEM*  _PAGED)header_item->next;
    if (header_item->next == item) header_item->next = item->next;
    
    return J1939_OK;
}

J1939_DTC_ITEM*  _PAGED j1939_dtc_add_dtc(J1939CanContext* _PAGED context, J1939_DTC_INFO* _PAGED dtc_info)
{
    J1939_DTC_ITEM* _PAGED free_item;
    
    if(context == NULL || dtc_info == NULL) return NULL;
    
    free_item = j1939_dtc_search_dtc_item(context, dtc_info);
    if(free_item == NULL)
    {
        free_item = j1939_dtc_get_free_dtc_item(context);
        if(free_item == NULL) return NULL;
        (void)j1939_dtc_reset_dtc_item(free_item);
        (void)j1939_dtc_add_dtc_item(context, free_item);
    }
    return free_item;
}

J1939Result j1939_dtc_release_dtc(J1939CanContext* _PAGED context, J1939_DTC_INFO* _PAGED dtc_info)
{
    J1939Result res = J1939_OK;
    J1939_DTC_ITEM* _PAGED item;
    
    if(context == NULL || dtc_info == NULL) return J1939_ERR;
    
    item = j1939_dtc_search_dtc_item(context, dtc_info);
    if(item == NULL) return J1939_OK;
    
    res = j1939_dtc_remove_dtc_item(context, item);
    (void)j1939_dtc_reset_dtc_item(item);
    
    return res;
}

#endif