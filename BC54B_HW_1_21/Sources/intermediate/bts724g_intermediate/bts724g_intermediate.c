/*******************************************************************************
**                       ������������Դ���޹�˾ Copyright (c)
**                            http://www.ligoo.cn/ 
** @file	 �b   bts724g_intermediate.c                                       

** @brief       1.���bts724gģ��̵������ƽӿں�������
                2.���bts724gģ�����������
                
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
  * @brief           ���ڶ�bts724gģ���Լ����������ʼ��
  * @param[in]       ��
  * @return          ��
***********************************************************************/
void relay_init(void)
{
    INT8U i = 0;

    bts724g_init();
    g_relay_pending_list = list_init();  //��ʼ���̵�����������
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
  * @brief           ����ָ��ID�̵������ָ��״̬
  * @param[in]       id  �̵������
  * @param[in]       value  �̵������
  * @return          RES_OK�� �ɹ�  RES_ERROR��ʧ��
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
  * @brief           ����ָ�����Ƶļ̵������ָ��״̬
  * @param[in]       name  �̵�������
  * @param[in]       value  �̵������
  * @return          RES_OK�� �ɹ�  RES_ERROR��ʧ��
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
  * @brief           ����ָ��id�̵����涨ʱ������ָ��״̬
  * @param[in]       id     �̵���id
  * @param[in]       value  �̵������
  * @param[in]       delay  �̵��������ȴ�ʱ��
  * @return          RES_OK�� �ɹ�  RES_ERROR��ʧ��
***********************************************************************/
Result relay_set(INT8U id, INT8U value, INT16U delay)
{
    if (id > BMS_RELAY_MAX_COUNT) return RES_ERROR;
    
    if (!(id >= LOAD_RELAY_START_ID && id <= LOAD_RELAY_END_ID) && // �Ǹ��ؼ̵���
        delay == 0)
    { 
        relay_io_set(id, value);
        return RES_OK;
    }
    // ���ؼ̵��� or pending delay
    if (value != relay_get_pending_status(id)) //no pending, relay on delay
    {
        relay_pending_para_update(id, value, delay, 1);
    }
    return RES_OK;
}

/***********************************************************************
  * @brief           ���¼̵��������Ƶ�״̬
  * @param[in]       id     �̵������
  * @param[in]       value  �̵������
  * @return          RES_OK�� �ɹ�  RES_ERROR��ʧ��
***********************************************************************/
Result relay_update_last_normal_clt_flag(INT8U id, INT8U value)
{
    if (id > BMS_RELAY_MAX_COUNT) return 0;
    g_relay_last_normal_ctl_flag[id] = value;
    
    return RES_OK;
}

/***********************************************************************
  * @brief           �жϼ̵����Ƿ�����ǿ��״̬
  * @param[in]       id     �̵������  
  * @return          1�� ǿ��  0����ͨ
***********************************************************************/
INT8U relay_is_force_status(INT8U id)
{
    if (id > BMS_RELAY_MAX_COUNT) return 0;
    if(g_relay_status[id] >= kRelayForceStart) return 1;
    
    return 0;
}


/***********************************************************************
  * @brief           ��ȡ�̵���ǿ�ƹ���״̬
  * @param[in]       id     �̵������  
  * @return          0��ʧ�� >0: ����״̬
***********************************************************************/
INT8U relay_get_force_status(INT8U id)
{
    if (id > BMS_RELAY_MAX_COUNT) return 0;
    
    if(!relay_is_force_status(id)) return 0;
    
    return g_relay_status[id];
}

/***********************************************************************
  * @brief           �պϼ̵���
  * @param[in]       id     �̵������  
  * @return          RES_OK�� �ɹ�  RES_ERROR��ʧ��
***********************************************************************/
Result relay_on(INT8U id)
{
    relay_update_last_normal_clt_flag(id, kRelayOn);
    if(relay_is_force_status(id) || relay_is_pending_force(id)) return RES_EXCEED_LIMITED;
    return relay_set(id, kRelayOn, 0);
}

/***********************************************************************
  * @brief           �Ͽ��̵���
  * @param[in]       id     �̵������  
  * @return          RES_OK�� �ɹ�  RES_ERROR��ʧ��
***********************************************************************/
Result relay_off(INT8U id)
{
    relay_update_last_normal_clt_flag(id, kRelayOff);
    if(relay_is_force_status(id) || relay_is_pending_force(id)) return RES_EXCEED_LIMITED;
    return relay_set(id, kRelayOff, 0);
}

/***********************************************************************
  * @brief           ǿ�Ʊպϼ̵���
  * @param[in]       id     �̵������  
  * @return          RES_OK�� �ɹ�  RES_ERROR��ʧ��
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
  * @brief           ǿ�ƶϿ��̵���
  * @param[in]       id     �̵������  
  * @return          RES_OK�� �ɹ�  RES_ERROR��ʧ��
***********************************************************************/
Result relay_force_off(INT8U id)
{
    if (id > BMS_RELAY_MAX_COUNT) return 0;
    if(relay_is_pending_force_off(id)) return RES_OK;
    
    return relay_set(id, kRelayForceOff, 0);
}

/***********************************************************************
  * @brief           ȡ��ǿ�ƿ���״̬
  * @param[in]       id     �̵������  
  * @return          RES_OK�� �ɹ�  RES_ERROR��ʧ��
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
  * @brief           �жϼ̵����Ƿ�պ�
  * @param[in]       id     �̵������  
  * @return          1�� �պ�  0��ʧ��
***********************************************************************/
INT8U relay_is_on(INT8U id)
{
    if (id > BMS_RELAY_MAX_COUNT) return 0;
    if(VALUE_MEANS_RELAY_ON(g_relay_status[id])) return 1;
    
    return 0;
}

/***********************************************************************
  * @brief           �жϼ̵����ȴ����Ƶ�״̬
  * @param[in]       id     �̵������  
  * @return          RelayCtlStatus�ͼ̵���״̬
***********************************************************************/
RelayCtlStatus relay_get_pending_status(INT8U id)
{
    if (id > BMS_RELAY_MAX_COUNT) return kRelayOff;
    return g_relay_pending_item[id].value;
}

/***********************************************************************
  * @brief           �жϼ̵����Ƿ�Ϊ�ȴ��պ�״̬
  * @param[in]       id     �̵������  
  * @return          1���պ�  0���Ͽ�
***********************************************************************/
INT8U relay_is_pending_on(INT8U id)
{
    if (id > BMS_RELAY_MAX_COUNT) return 0;
    if (VALUE_MEANS_RELAY_ON(g_relay_pending_item[id].value)) return 1;
    
    return 0;
}

/***********************************************************************
  * @brief           �жϼ̵����Ƿ�Ϊ�ȴ��Ͽ�״̬
  * @param[in]       id     �̵������  
  * @return          1���Ͽ�  0���պ�
***********************************************************************/
INT8U relay_is_pending_off(INT8U id)
{
    if (id > BMS_RELAY_MAX_COUNT) return 0;
    if (VALUE_MEANS_RELAY_ON(g_relay_pending_item[id].value) == 0)
        return 1;
    
    return 0;
}

/***********************************************************************
  * @brief           �жϼ̵����Ƿ�Ϊ�ȴ�ǿ�Ʊպ�״̬
  * @param[in]       id     �̵������  
  * @return          1����  0������
***********************************************************************/
INT8U relay_is_pending_force_on(INT8U id)
{
    if (id > BMS_RELAY_MAX_COUNT) return 0;
    if (g_relay_pending_item[id].value == kRelayForceOn) return 1;
    
    return 0;
}

/***********************************************************************
  * @brief           �жϼ̵����Ƿ�Ϊ�ȴ�ǿ�ƶϿ�״̬
  * @param[in]       id     �̵������  
  * @return          1����  0������
***********************************************************************/
INT8U relay_is_pending_force_off(INT8U id)
{
    if (id > BMS_RELAY_MAX_COUNT) return 0;
    if (g_relay_pending_item[id].value == kRelayForceOff) return 1;
    
    return 0;
}

/***********************************************************************
  * @brief           �жϼ̵����Ƿ�Ϊ�����ȴ�ǿ��״̬
  * @param[in]       id     �̵������  
  * @return          1����  0������
***********************************************************************/
INT8U relay_is_pending_force(INT8U id)
{
    if(relay_is_pending_force_on(id)) return 1;
    if(relay_is_pending_force_off(id)) return 1;
    
    return 0;
}

/***********************************************************************
  * @brief           ���ؼ̵�������
  * @param[in]       �� 
  * @return          BMS_RELAY_MAX_COUNT
***********************************************************************/
INT8U relay_count(void)
{
    return BMS_RELAY_MAX_COUNT;
}

/***********************************************************************
  * @brief           ��ȡ�̵���ʵʱ����״̬
  * @param[in]       id   �̵������
  * @param[in]       input_signal_id   �����źű��  
  * @return          RelayTroubleStatus �͹�������
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
  * @brief           ��ȡ�̵������ϲɼ�ͨ��
  * @param[in]       id   �̵������   
  * @return          �ɼ�ͨ����
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
  * @brief           ��ȡ�̵�������
  * @param[in]       id   �̵������   
  * @return          RelayTroubleStatus��������
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
  * @brief           ���ü̵�������״̬
  * @param[in]       id   �̵������   
  * @return          RelayTroubleStatus��������
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
  * @brief           ��ȡ�̵���ʵʱ����״̬
  * @param[in]       id   �̵������
  * @param[in]       input_signal_id   �����źű��  
  * @return          RelayTroubleStatus �͹�������
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
  * @brief           �ȴ����м̵���������
  * @param[in]       pdata  �յĺ���ָ�� 
  * @return          ��
***********************************************************************/
void relay_pending_check(void* pdata)
{
    INT32U now_tick = 0;
    RelayPendingItem* item = NULL;
    OS_CPU_SR cpu_sr = 0;
    
    pdata = pdata;
        
    if (g_relay_pending_list->next == NULL) return;
    now_tick = get_tick_count();                  //��ȡϵͳʱ�ӽ���
    
    if (get_interval_by_tick(g_last_load_relay_tick, now_tick) >= BMS_RELAY_LOAD_RELAY_ON_MIN_INTERVAL) //�ж��Ƿ�ﵽʱ��
    {
        OS_ENTER_CRITICAL();
        item = (RelayPendingItem*)(g_relay_pending_list->next);
        while(item)
        {
            if(get_interval_by_tick(item->last_tick, now_tick) >= item->delay)
            {
                if(VALUE_MEANS_RELAY_ON(item->value)) g_last_load_relay_tick = now_tick; //only on update tick
                relay_io_set(item->id, item->value); //���Ƽ̵���IO
            }
            item = (RelayPendingItem*)(item->next);
        }
        OS_EXIT_CRITICAL();
    }
    
}

/***********************************************************************
  * @brief           ���Ӵ����Ƶļ̵�����������
  * @param[in]       pdata  �յĺ���ָ�� 
  * @return          1��Ok  0�� Err
***********************************************************************/
Result relay_pending_list_add(INT8U id)
{
    OS_CPU_SR cpu_sr = 0;
    
    if(id > BMS_RELAY_MAX_COUNT) return 0;
    
    OS_ENTER_CRITICAL();
    if (list_find(g_relay_pending_list, (LIST_ITEM*)(&g_relay_pending_item[id])) == 0)
    {
        list_add(g_relay_pending_list, (LIST_ITEM*)(&g_relay_pending_item[id])); //����ȴ��б�
    }
    OS_EXIT_CRITICAL();
    
    return 1;
}

/***********************************************************************
  * @brief           �����Ƶļ̵����Ӷ�����ɾ��
  * @param[in]       pdata  �յĺ���ָ�� 
  * @return          1��Ok  0�� Err
***********************************************************************/
Result relay_pending_list_remove(INT8U id)
{
    OS_CPU_SR cpu_sr = 0;
    RelayPendingItem* item = NULL;
    
    if(id > BMS_RELAY_MAX_COUNT) return 0;
    OS_ENTER_CRITICAL();
    item = (RelayPendingItem*)(&g_relay_pending_item[id]);
    list_remove(g_relay_pending_list, (LIST_ITEM*)item); //�ӵȴ��б����Ƴ�
    item->next = NULL;
    item->value = 0xFF;
    item->delay = 0;
    OS_EXIT_CRITICAL();
    
    return 1;
}

/***********************************************************************
  * @brief           ��ʱ�պϼ̵���
  * @param[in]       id  �̵������
  * @param[in]       delay  ��ʱʱ��    
  * @return          RES_OK�� �ɹ�  RES_ERROR��ʧ��
***********************************************************************/
Result relay_delay_on(INT8U id, INT16U delay)
{
    relay_update_last_normal_clt_flag(id, kRelayOn);
    if(relay_is_force_status(id) || relay_is_pending_force(id)) return RES_EXCEED_LIMITED;
    return relay_set(id, kRelayOn, delay);
}

/***********************************************************************
  * @brief           ��ʱ�Ͽ��̵���
  * @param[in]       id  �̵������
  * @param[in]       delay  ��ʱʱ��    
  * @return          RES_OK�� �ɹ�  RES_ERROR��ʧ��
***********************************************************************/
Result relay_delay_off(INT8U id, INT16U delay)
{
    relay_update_last_normal_clt_flag(id, kRelayOff);
    if(relay_is_force_status(id) || relay_is_pending_force(id)) return RES_EXCEED_LIMITED;
    return relay_set(id, kRelayOff, delay);
}

/***********************************************************************
  * @brief           ���¼̵����ȴ�����״̬
  * @param[in]       id  �̵������
  * @param[in]       value  �̵�������״̬  
  * @param[in]       delay  ��ʱʱ�� 
  * @param[in]       restart  �Ƿ�������    
  * @return          ��
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

