/*******************************************************************************
**                       安徽力高新能源有限公司 Copyright (c)
**                            http://www.ligoo.cn/ 
** @file	    bts724g_intermediate.c                                       

** @brief       1.完成bts724g模块继电器控制接口函数处理
                2.完成bts724g模块控制任务函数
                
** @copyright		Ligoo Inc.         
** @date		    2017-04-11.
** @author            
*******************************************************************************/ 
#include "bts724g_intermediate.h"

#include "bms_job.h"
//#include "bms_base_cfg.h"
#include "ect_intermediate.h"
#include "bms_input_signal_impl.h"
#include "os_cpu.h"
//#include "bms_system_impl.h"

//#include "bms_relay.h"
#include "bms_base_cfg.h"
#include "bms_system.h"

#include "bts724g_hardware.h"



#ifndef BMS_HARDWARE_PLATFORM
#define BMS_HARDWARE_PLATFORM       1
#endif

#ifndef BMS_RELAY_LOAD_RELAY_ON_MIN_INTERVAL
  #define BMS_RELAY_LOAD_RELAY_ON_MIN_INTERVAL 150
#endif



#if BMS_HARDWARE_PLATFORM

#pragma MESSAGE DISABLE C1420 // Result of function-call is ignored
#pragma MESSAGE DISABLE C1860 // Result of array assigned to pointer


#define VALUE_MEANS_RELAY_ON(VALUE) \
    ((VALUE == kRelayOn || VALUE == kRelayForceOn) ? 1 : 0)    

#define LOAD_RELAY_START_ID      1
#define LOAD_RELAY_END_ID        8

#define PERIOD_IN_SHORT_CIRCUIT_MAX    900
#define PERIOD_IN_SHORT_CIRCUIT_MIN    100






#pragma DATA_SEG __RPAGE_SEG PAGED_RAM

static INT8U g_relay_status[BMS_RELAY_MAX_COUNT + 1] = {0};
static INT32U g_last_load_relay_tick = 0;
static INT8U g_relay_last_normal_ctl_flag[BMS_RELAY_MAX_COUNT + 1] = {0};
static INT32U g_relay_last_tick[BMS_RELAY_MAX_COUNT + 1] = {0};
static RelayTroubleStatus g_relay_trouble_status[BMS_RELAY_MAX_COUNT + 1] = {0};


//const struct hcs12_gpio_detail *pt_relay_input_pins = NULL;

#pragma DATA_SEG DEFAULT

LIST_ITEM* g_relay_pending_list = NULL;
RelayPendingItem g_relay_pending_item[BMS_RELAY_MAX_COUNT + 1];

/*
 void  relay_input_pin_set(INT8U id, INT8U value) 
{
    gpio_set_output(&hcs12_gpio_ops,&pt_relay_input_pins[id - 1].io, value);
     g_relay_last_tick[id]  = get_tick_count();
} */

/***********************************************************************
  * @brief           用于对bts724g模块以及工作任务初始化
  * @param[in]       无
  * @return          无
***********************************************************************/
void relay_init(void)
{
    INT8U i = 0;

    bts724g_init();
    g_relay_pending_list = list_init();  //初始化继电器工作链表
    g_relay_pending_list->next = NULL;
    
    for (i = 0; i <= BMS_RELAY_MAX_COUNT; i++)
    {
        g_relay_pending_item[i].next = NULL;
        g_relay_pending_item[i].id = i;
        g_relay_pending_item[i].value = 0xFF;
    }
    job_schedule(MAIN_JOB_GROUP, RELAY_PENDING_CHECK_CYCLE, relay_pending_check, NULL);
    st_pwm_start();
}

/***********************************************************************
  * @brief           设置指定ID继电器输出指定状态
  * @param[in]       id  继电器编号
  * @param[in]       value  继电器编号
  * @return          RES_OK： 成功  RES_ERROR：失败
***********************************************************************/
Result relay_io_set(INT8U id, INT8U value)
{
    INT8U flag = 0;
    
    if( (id ==0) || (id > BMS_RELAY_MAX_COUNT)) 
    {
        return RES_ERROR;
    }
    
    if (VALUE_MEANS_RELAY_ON(value))
    {
        flag = 1;
    }

    relay_input_pin_set(id,     flag);
   g_relay_last_tick[id]  = get_tick_count();
    relay_pending_list_remove(id);
    g_relay_status[id] = value;

    return RES_OK;
}

/***********************************************************************
  * @brief           设置指定名称的继电器输出指定状态
  * @param[in]       name  继电器名称
  * @param[in]       value  继电器编号
  * @return          RES_OK： 成功  RES_ERROR：失败
***********************************************************************/
Result relay_io_set_by_name(char* name, INT8U value)
{
    INT8U i;
    INT8U flag = 0;

    if(pt_relay_input_pins == NULL)
    {
        return RES_ERROR;  
    }

    for(i=0; i<BMS_RELAY_MAX_COUNT; i++)
    {
        if (strcmp(name, pt_relay_input_pins[i].name) == 0)
        {
            if (VALUE_MEANS_RELAY_ON(value))
            {
                flag = 1;
            }

            relay_input_pin_set(i+1,     flag);
            g_relay_last_tick[i+1]  = get_tick_count();
            g_relay_status[i+1] = value;

            return RES_OK;
        }
    }
    
    return RES_ERROR;    
}

/***********************************************************************
  * @brief           设置指定id继电器规定时间后输出指定状态
  * @param[in]       id     继电器id
  * @param[in]       value  继电器编号
  * @param[in]       delay  继电器动作等待时间
  * @return          RES_OK： 成功  RES_ERROR：失败
***********************************************************************/
Result relay_set(INT8U id, INT8U value, INT16U delay)
{
    if (id > BMS_RELAY_MAX_COUNT) return RES_ERROR;
    
    if (!(id >= LOAD_RELAY_START_ID && id <= LOAD_RELAY_END_ID) && // 非负载继电器
        delay == 0)
    { 
        relay_io_set(id, value);
        return RES_OK;
    }
    // 负载继电器 or pending delay
    if (value != relay_get_pending_status(id)) //no pending, relay on delay
    {
        relay_pending_para_update(id, value, delay, 1);
    }
    return RES_OK;
}

/***********************************************************************
  * @brief           更新继电器最后控制的状态
  * @param[in]       id     继电器编号
  * @param[in]       value  继电器编号
  * @return          RES_OK： 成功  RES_ERROR：失败
***********************************************************************/
Result relay_update_last_normal_clt_flag(INT8U id, INT8U value)
{
    if (id > BMS_RELAY_MAX_COUNT) return 0;
    g_relay_last_normal_ctl_flag[id] = value;
    
    return RES_OK;
}

/***********************************************************************
  * @brief           判断继电器是否工作在强制状态
  * @param[in]       id     继电器编号  
  * @return          1： 强制  0：普通
***********************************************************************/
INT8U relay_is_force_status(INT8U id)
{
    if (id > BMS_RELAY_MAX_COUNT) return 0;
    if(g_relay_status[id] >= kRelayForceStart) return 1;
    
    return 0;
}


/***********************************************************************
  * @brief           获取继电器强制工作状态
  * @param[in]       id     继电器编号  
  * @return          0：失败 >0: 工作状态
***********************************************************************/
INT8U relay_get_force_status(INT8U id)
{
    if (id > BMS_RELAY_MAX_COUNT) return 0;
    
    if(!relay_is_force_status(id)) return 0;
    
    return g_relay_status[id];
}

/***********************************************************************
  * @brief           闭合继电器
  * @param[in]       id     继电器编号  
  * @return          RES_OK： 成功  RES_ERROR：失败
***********************************************************************/
Result relay_on(INT8U id)
{
    relay_update_last_normal_clt_flag(id, kRelayOn);
    if(relay_is_force_status(id) || relay_is_pending_force(id)) return RES_EXCEED_LIMITED;
    return relay_set(id, kRelayOn, 0);
}

/***********************************************************************
  * @brief           断开继电器
  * @param[in]       id     继电器编号  
  * @return          RES_OK： 成功  RES_ERROR：失败
***********************************************************************/
Result relay_off(INT8U id)
{
    relay_update_last_normal_clt_flag(id, kRelayOff);
    if(relay_is_force_status(id) || relay_is_pending_force(id)) return RES_EXCEED_LIMITED;
    return relay_set(id, kRelayOff, 0);
}

/***********************************************************************
  * @brief           强制闭合继电器
  * @param[in]       id     继电器编号  
  * @return          RES_OK： 成功  RES_ERROR：失败
***********************************************************************/
Result relay_force_on(INT8U id)
{
    INT8U flag = 0;
    
    if (id > BMS_RELAY_MAX_COUNT) return 0;
    if(relay_is_pending_force_on(id)) return RES_OK;
    
    //relay_pending_para_update(id, kRelayForceOn, 0, 1);
    
    return relay_set(id, kRelayForceOn, 0);
}

/***********************************************************************
  * @brief           强制断开继电器
  * @param[in]       id     继电器编号  
  * @return          RES_OK： 成功  RES_ERROR：失败
***********************************************************************/
Result relay_force_off(INT8U id)
{
    if (id > BMS_RELAY_MAX_COUNT) return 0;
    if(relay_is_pending_force_off(id)) return RES_OK;
    
    return relay_set(id, kRelayForceOff, 0);
}

/***********************************************************************
  * @brief           取消强制控制状态
  * @param[in]       id     继电器编号  
  * @return          RES_OK： 成功  RES_ERROR：失败
***********************************************************************/
Result relay_force_cancle(INT8U id)
{
    if (id > BMS_RELAY_MAX_COUNT) return 0;
    if (relay_is_force_status(id)==0 && relay_is_pending_force(id)==0) return 0;
    
    if(g_relay_last_normal_ctl_flag[id] != kRelayOn && g_relay_last_normal_ctl_flag[id] != kRelayOff) 
        g_relay_last_normal_ctl_flag[id] = kRelayOff;
    
    //relay_pending_para_update(id, g_relay_last_normal_ctl_flag[id], g_relay_pending_item[id].delay, 0);
    
    return relay_set(id, g_relay_last_normal_ctl_flag[id], 1);
}

/***********************************************************************
  * @brief           判断继电器是否闭合
  * @param[in]       id     继电器编号  
  * @return          1： 闭合  0：失败
***********************************************************************/
INT8U relay_is_on(INT8U id)
{
    if (id > BMS_RELAY_MAX_COUNT) return 0;
    if(VALUE_MEANS_RELAY_ON(g_relay_status[id])) return 1;
    
    return 0;
}

/***********************************************************************
  * @brief           判断继电器等待控制的状态
  * @param[in]       id     继电器编号  
  * @return          RelayCtlStatus型继电器状态
***********************************************************************/
RelayCtlStatus relay_get_pending_status(INT8U id)
{
    if (id > BMS_RELAY_MAX_COUNT) return kRelayOff;
    return g_relay_pending_item[id].value;
}

/***********************************************************************
  * @brief           判断继电器是否为等待闭合状态
  * @param[in]       id     继电器编号  
  * @return          1：闭合  0：断开
***********************************************************************/
INT8U relay_is_pending_on(INT8U id)
{
    if (id > BMS_RELAY_MAX_COUNT) return 0;
    if (VALUE_MEANS_RELAY_ON(g_relay_pending_item[id].value)) return 1;
    
    return 0;
}

/***********************************************************************
  * @brief           判断继电器是否为等待断开状态
  * @param[in]       id     继电器编号  
  * @return          1：断开  0：闭合
***********************************************************************/
INT8U relay_is_pending_off(INT8U id)
{
    if (id > BMS_RELAY_MAX_COUNT) return 0;
    if (VALUE_MEANS_RELAY_ON(g_relay_pending_item[id].value) == 0)
        return 1;
    
    return 0;
}

/***********************************************************************
  * @brief           判断继电器是否为等待强制闭合状态
  * @param[in]       id     继电器编号  
  * @return          1：是  0：不是
***********************************************************************/
INT8U relay_is_pending_force_on(INT8U id)
{
    if (id > BMS_RELAY_MAX_COUNT) return 0;
    if (g_relay_pending_item[id].value == kRelayForceOn) return 1;
    
    return 0;
}

/***********************************************************************
  * @brief           判断继电器是否为等待强制断开状态
  * @param[in]       id     继电器编号  
  * @return          1：是  0：不是
***********************************************************************/
INT8U relay_is_pending_force_off(INT8U id)
{
    if (id > BMS_RELAY_MAX_COUNT) return 0;
    if (g_relay_pending_item[id].value == kRelayForceOff) return 1;
    
    return 0;
}

/***********************************************************************
  * @brief           判断继电器是否为工作等待强制状态
  * @param[in]       id     继电器编号  
  * @return          1：是  0：不是
***********************************************************************/
INT8U relay_is_pending_force(INT8U id)
{
    if(relay_is_pending_force_on(id)) return 1;
    if(relay_is_pending_force_off(id)) return 1;
    
    return 0;
}

/***********************************************************************
  * @brief           返回继电器数量
  * @param[in]       无 
  * @return          BMS_RELAY_MAX_COUNT
***********************************************************************/
INT8U relay_count(void)
{
    return BMS_RELAY_MAX_COUNT;
}

/***********************************************************************
  * @brief           获取继电器实时故障状态
  * @param[in]       id   继电器编号
  * @param[in]       input_signal_id   输入信号编号  
  * @return          RelayTroubleStatus 型故障类型
***********************************************************************/
RelayTroubleStatus relay_get_instant_trouble_status(INT8U id, INT8U input_signal_id)
{
    RelayTroubleStatus status = kRelayNormal;
    INT8U bts724g_st_num = 0;
    unsigned short st_duty = 0;

    if(id > BMS_RELAY_MAX_COUNT) return kRelayNormal;
    if(input_signal_id > input_signal_get_count()) return kRelayNormal;
    
    bts724g_st_num = relay_get_724g_st_num(id);
    if(bts724g_st_num == 0) return kRelayNormal;

    
    if(relay_is_on(id) == 1)
    {
    
        st_duty = st_pwm_get_duty(bts724g_st_num);
        if(st_duty <= PERIOD_IN_SHORT_CIRCUIT_MAX
          && st_duty >= PERIOD_IN_SHORT_CIRCUIT_MIN)
        {
            status = kRelayShortCircuit;
        }
        
        if(input_signal_is_high(bts724g_st_num) == 0)
        {
            status = kRelayOverTemperature;
        }
    }
    else
    {
        if(input_signal_is_high(bts724g_st_num) == 0)
        {
            status = kRelayOpenCircuitTrouble;
        }
    }
    
    return status;
}

/***********************************************************************
  * @brief           获取继电器故障采集通道
  * @param[in]       id   继电器编号   
  * @return          采集通道号
***********************************************************************/
INT8U relay_get_724g_st_num(INT8U id)
{
    INT8U st_num = 0;
    switch (id)
    {
    case 1:
        st_num = 9;
        break;
    case 2:
        st_num = 9;
        break;
    case 3:
        st_num = 10;
        break;
    case 4:
        st_num = 10;
        break;
    case 5:
        st_num = 11;
        break;
    case 6:
        st_num = 11;
        break;
    case 7:
        st_num = 12;
        break;
    case 8:
        st_num = 12;
        break;
    default:
        break;
    }
    
    return st_num;
}

/***********************************************************************
  * @brief           获取继电器故障
  * @param[in]       id   继电器编号   
  * @return          RelayTroubleStatus故障类型
***********************************************************************/
RelayTroubleStatus relay_get_trouble(INT8U id)
{
    OS_CPU_SR cpu_sr = 0;
    RelayTroubleStatus status = kRelayTroubleNoCheck;
    
    if (id > BMS_RELAY_MAX_COUNT) return kRelayNormal;
    OS_ENTER_CRITICAL();
    status = g_relay_trouble_status[id];
    OS_EXIT_CRITICAL();
    
    return status;
}

/***********************************************************************
  * @brief           设置继电器故障状态
  * @param[in]       id   继电器编号   
  * @return          RelayTroubleStatus故障类型
***********************************************************************/
void relay_set_trouble(INT8U id, RelayTroubleStatus status)
{
    OS_CPU_SR cpu_sr = 0;
    
    if (id > BMS_RELAY_MAX_COUNT) return;
    if (status >= kRelayTroubleMax) return;
    
    OS_ENTER_CRITICAL();
    g_relay_trouble_status[id] = status;
    OS_EXIT_CRITICAL();
}

/***********************************************************************
  * @brief           获取继电器实时故障状态
  * @param[in]       id   继电器编号
  * @param[in]       input_signal_id   输入信号编号  
  * @return          RelayTroubleStatus 型故障类型
***********************************************************************/
RelayTroubleStatus relay_get_trouble_status(INT8U id, INT8U input_signal_id)
{
    if(id > BMS_RELAY_MAX_COUNT) return kRelayNormal;
    if(input_signal_id > input_signal_get_count()) return kRelayNormal;
    
    if(g_relay_last_tick[id] == 0) return kRelayNormal;
    
    if(get_interval_by_tick(g_relay_last_tick[id], get_tick_count()) < BMS_LOAD_RELAY_ON_TIME) return g_relay_trouble_status[id];
    
    g_relay_trouble_status[id] = relay_get_instant_trouble_status(id, input_signal_id);
    
    return g_relay_trouble_status[id];
}

/***********************************************************************
  * @brief           等待队列继电器处理函数
  * @param[in]       pdata  空的函数指针 
  * @return          无
***********************************************************************/
void relay_pending_check(void* pdata)
{
    INT32U now_tick = 0;
    RelayPendingItem* item = NULL;
    OS_CPU_SR cpu_sr = 0;
    
    pdata = pdata;
        
    if (g_relay_pending_list->next == NULL) return;
    now_tick = get_tick_count();                  //获取系统时钟节拍
    
    if (get_interval_by_tick(g_last_load_relay_tick, now_tick) >= BMS_RELAY_LOAD_RELAY_ON_MIN_INTERVAL) //判断是否达到时间
    {
        OS_ENTER_CRITICAL();
        item = (RelayPendingItem*)(g_relay_pending_list->next);
        while(item)
        {
            if(get_interval_by_tick(item->last_tick, now_tick) >= item->delay)
            {
                if(VALUE_MEANS_RELAY_ON(item->value)) g_last_load_relay_tick = now_tick; //only on update tick
                relay_io_set(item->id, item->value); //控制继电器IO
            }
            item = (RelayPendingItem*)(item->next);
        }
        OS_EXIT_CRITICAL();
    }
    
}

/***********************************************************************
  * @brief           增加待控制的继电器到队列中
  * @param[in]       pdata  空的函数指针 
  * @return          1：Ok  0： Err
***********************************************************************/
Result relay_pending_list_add(INT8U id)
{
    OS_CPU_SR cpu_sr = 0;
    
    if(id > BMS_RELAY_MAX_COUNT) return 0;
    
    OS_ENTER_CRITICAL();
    if (list_find(g_relay_pending_list, (LIST_ITEM*)(&g_relay_pending_item[id])) == 0)
    {
        list_add(g_relay_pending_list, (LIST_ITEM*)(&g_relay_pending_item[id])); //加入等待列表
    }
    OS_EXIT_CRITICAL();
    
    return 1;
}

/***********************************************************************
  * @brief           待控制的继电器从队列中删除
  * @param[in]       pdata  空的函数指针 
  * @return          1：Ok  0： Err
***********************************************************************/
Result relay_pending_list_remove(INT8U id)
{
    OS_CPU_SR cpu_sr = 0;
    RelayPendingItem* item = NULL;
    
    if(id > BMS_RELAY_MAX_COUNT) return 0;
    OS_ENTER_CRITICAL();
    item = (RelayPendingItem*)(&g_relay_pending_item[id]);
    list_remove(g_relay_pending_list, (LIST_ITEM*)item); //从等待列表中移除
    item->next = NULL;
    item->value = 0xFF;
    item->delay = 0;
    OS_EXIT_CRITICAL();
    
    return 1;
}

/***********************************************************************
  * @brief           延时闭合继电器
  * @param[in]       id  继电器编号
  * @param[in]       delay  延时时长    
  * @return          RES_OK： 成功  RES_ERROR：失败
***********************************************************************/
Result relay_delay_on(INT8U id, INT16U delay)
{
    relay_update_last_normal_clt_flag(id, kRelayOn);
    if(relay_is_force_status(id) || relay_is_pending_force(id)) return RES_EXCEED_LIMITED;
    return relay_set(id, kRelayOn, delay);
}

/***********************************************************************
  * @brief           延时断开继电器
  * @param[in]       id  继电器编号
  * @param[in]       delay  延时时长    
  * @return          RES_OK： 成功  RES_ERROR：失败
***********************************************************************/
Result relay_delay_off(INT8U id, INT16U delay)
{
    relay_update_last_normal_clt_flag(id, kRelayOff);
    if(relay_is_force_status(id) || relay_is_pending_force(id)) return RES_EXCEED_LIMITED;
    return relay_set(id, kRelayOff, delay);
}

/***********************************************************************
  * @brief           更新继电器等待控制状态
  * @param[in]       id  继电器编号
  * @param[in]       value  继电器操作状态  
  * @param[in]       delay  延时时长 
  * @param[in]       restart  是否加入队列    
  * @return          无
***********************************************************************/
void relay_pending_para_update(INT8U id, INT8U value, INT16U delay, INT8U restart)
{
    OS_CPU_SR cpu_sr = 0;
    
    if(id > BMS_RELAY_MAX_COUNT) return;
    
    OS_ENTER_CRITICAL();
    g_relay_pending_item[id].value = value;
    g_relay_pending_item[id].delay = delay;
    if(restart)
    {
        g_relay_pending_item[id].last_tick = get_tick_count();
        relay_pending_list_add(id);
    }
    OS_EXIT_CRITICAL();
}

#endif

