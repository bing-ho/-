#include "process_mgmt.h"
#include "ctrl_exec.h"
#include "relay_fault_check.h"
#include "bms_system.h"
#include "single_direct_list.h"
#include "logic_ctrl_process_start.h"
#include "bms_relay_control.h"

#pragma MESSAGE DISABLE C4800
#pragma MESSAGE DISABLE C1860
#pragma MESSAGE DISABLE C12056 // SP debug info incorrect because of optimization or inline assembler
/***************************************************************
�ڲ����ݽṹ����
***************************************************************/

//ÿ�����̽ڵ����������Ŀ
#define MAX_PROCESS_NODE_RUN_COUNT       30


#pragma DATA_SEG __RPAGE_SEG PAGED_RAM_LOGIC_CTRL
/// �������̣�����֧�ָ���Ϊ20��
static CtrlProcessMgmtContext     ctrl_process_mgmt[PROCESS_LOGIC_CTRL_CONTEXT_MAX_NUM];
/// ���̹�������
static CtrlProcessMgmtContext *_FAR  ptr_ctrl_process_mgmt_list[MAIN_PROCESS_MAX_NUM+1];
static CtrlProcessMgmtContext *_FAR  need_run_process_array[MAIN_PROCESS_MAX_NUM+1];

/// ��ʹ�õ����̹���ռ�
static SDLIST_ITEM  free_ctrl_process_mgmt_list = {0};
#pragma DATA_SEG __RPAGE_SEG DEFAULT

/***************************************************************
�ڲ���������
***************************************************************/


/***************************************************************
����ʵ��
***************************************************************/

/**
  * @brief	   ������̹����е�����
  * @param[in] mgmt
  *	@return	   ��
 */
void  ctrl_process_mgmt_data_clear(CtrlProcessMgmtContext * _FAR mgmt)
{
    if(mgmt == NULL)
      return;
    
    mgmt->ptr_process = NULL;      
    mgmt->process_node_num = 0;     
    mgmt->next_node_id = NODE_ID_INIT;         
    mgmt->status = LCPS_End;              
    mgmt->time_start = 0;          
    mgmt->time_out = 0;              
    mgmt->name = NULL;            
    mgmt->process_id = 0;
    mgmt->type = LCNT_PROCESS_MAIN;                
    mgmt->sub_process_mgmt = NULL;
    mgmt->apply_ptr_process = NULL;  
}

/**
  * @brief	���̹��������ʼ��
  *	@return	��
 */
void process_mgmt_init(void)
{
    INT8U i;

    for(i=0; i<PROCESS_LOGIC_CTRL_CONTEXT_MAX_NUM; i++)
    {
        ctrl_process_mgmt_data_clear((CtrlProcessMgmtContext * _FAR)&ctrl_process_mgmt[i]);
        list_add_to_head(&free_ctrl_process_mgmt_list, (SDLIST_ITEM * _FAR)&ctrl_process_mgmt[i]);
    }
}


/**
  * @brief          ��ȡ����id��
  *	@param[in]      ptr_process             ���������׵�ַ
  *	@return         ����id        0Ϊ��ЧID
  * @note
  * @li             
 */
INT8U get_process_id(const LogicControlNodeContext *_FAR ptr_process,ProcessType type)
{
    const ProcessAddrNameMap *_FAR map = NULL; 
    INT8U process_index = 0;
    INT8U num = 0;
    if(ptr_process == NULL)
    {
        return 0;
    }
    if(type == LCNT_PROCESS_MAIN)
    {
       map = main_process_addr_name_map;
       num = MAIN_PROCESS_MAX_NUM; 
    }
    else
    {
       map = sub_process_addr_name_map; 
       num = SUB_PROCESS_MAX_NUM; 
    }
    for(process_index = 0;process_index < num; process_index++)
    {
        if(map[process_index].ptr_ctrl_process == ptr_process)
        {
            return process_index+1;
        }
    }
    return 0;
}

/**
  * @brief          ��ȡ��������
  *	@param[in]      ptr_process             ���������׵�ַ
  *	@return         ��������
  * @note
  * @li             
 */
const char* _FAR get_process_name(const LogicControlNodeContext *_FAR ptr_process,ProcessType type)
{

    INT8U process_index = 0;
    const ProcessAddrNameMap *_FAR map = NULL; 
    INT8U num = 0;
    if(ptr_process == NULL)
    {
        return NULL;
    }
    if(type == LCNT_PROCESS_MAIN)
    {
       map = main_process_addr_name_map;
       num = MAIN_PROCESS_MAX_NUM; 
    }
    else
    {
       map = sub_process_addr_name_map; 
       num = SUB_PROCESS_MAX_NUM; 
    }
    for(process_index = 0;process_index < num; process_index++)
    {
        if(map[process_index].ptr_ctrl_process == ptr_process)
        {
            return map[process_index].process_name;
        }
    }
    return NULL;
}
/**
  * @brief          �ɽڵ��Ų��ҽڵ�
  *	@param[in]      ptr_process             ���������׵�ַ
  *	@param[in]      node_num                �����еĽڵ���Ŀ
  *	@param[in]      id                      ���ƽڵ�ı��
  *	@return         ���ƽڵ�ָ��
  * @note
  * @li             ����ΪNULL��ʾδ���ҵ�ָ��id�Ľڵ�
 */
static LogicControlNodeContext*_FAR  get_node_from_id(LogicControlNodeContext *_FAR ptr_process, INT8U node_num, INT8U id)
{
    INT8U i = 0;

    if(NULL == ptr_process)
    {
        return NULL;
    }

    for(i=0; i< node_num; i++)
    {
        if(id == ptr_process[i].node_id)
        {
            return &ptr_process[i];
        }
    }

    return NULL;
}

/**
  * @brief	            ֹͣ�߼���������
  *	@param[in]			ptr_process		���̹���ڵ�
  *	@return	��
 */
void stop_logic_ctrl_process(CtrlProcessMgmtContext *_FAR ptr_ctrl_mgmt)
{
    if(ptr_ctrl_mgmt == NULL)
    {
        return;
    }
    if(ptr_ctrl_mgmt->sub_process_mgmt != NULL) 
    {
       release_state_ctrl_all(ptr_ctrl_mgmt->sub_process_mgmt->ptr_process);
       ctrl_process_mgmt_data_clear(ptr_ctrl_mgmt->sub_process_mgmt);
       list_add_to_head(&free_ctrl_process_mgmt_list, (SDLIST_ITEM * _FAR)ptr_ctrl_mgmt->sub_process_mgmt);
    }
    release_state_ctrl_all(ptr_ctrl_mgmt->ptr_process);
    ptr_ctrl_mgmt->time_start = 0;          
    ptr_ctrl_mgmt->time_out = 0;              
}

/**
  * @brief	            �����߼���������
  *	@param[in]			ptr_process		���̹���ڵ�
  *	@return	��
 */
void restart_logic_ctrl_process(CtrlProcessMgmtContext *_FAR ptr_ctrl_mgmt)
{
    if(ptr_ctrl_mgmt == NULL)
    {
        return;
    }
    if(ptr_ctrl_mgmt->type == LCNT_PROCESS_SUB)
    {
        return;
    }
   
   
    stop_logic_ctrl_process(ptr_ctrl_mgmt);
    ptr_ctrl_mgmt->next_node_id = NODE_ID_INIT;
    ptr_ctrl_mgmt->status = LCPS_Running;              
}

/**
  * @brief	            �����������׵�ַ���������̣�����������Ϊ�����̣����������ڵ�������
  *	@param[in]			ptr_process		���̹���ڵ�
  *	@return	��
 */
CtrlProcessMgmtContext *_FAR get_main_process_mgmt_by_process_addr(const LogicControlNodeContext *_FAR ptr_process)
{
    INT8U  process_index = 0;
     for(process_index = 1;process_index <= MAIN_PROCESS_MAX_NUM; process_index++)
    {
        if(ptr_ctrl_process_mgmt_list[process_index] == NULL)
            continue;
        if(ptr_ctrl_process_mgmt_list[process_index]->ptr_process == ptr_process)
        {
             return ptr_ctrl_process_mgmt_list[process_index];
        }
        else if(ptr_ctrl_process_mgmt_list[process_index]->sub_process_mgmt != NULL)
         {
             if(ptr_ctrl_process_mgmt_list[process_index]->sub_process_mgmt->ptr_process == ptr_process)
            {
                 return ptr_ctrl_process_mgmt_list[process_index];
            }
         }
    }
   
    return NULL;
}

/**
  * @brief	            ����������
  *	@param[in]			ptr_ctrl_process		���̹���ڵ�
  *	@return	��
 */

void set_process_active(LogicControlNodeContext *_FAR ptr_ctrl_process)
{

    CtrlProcessMgmtContext *_FAR mgmt = NULL;
    if(ptr_ctrl_process == NULL)
        return;
    mgmt = get_main_process_mgmt_by_process_addr(ptr_ctrl_process);
    if(mgmt == NULL)
      return;
    if(mgmt->type == LCNT_PROCESS_SUB)
        return;
    if(mgmt->ptr_process == NULL)
        return;
    if(mgmt->status != LCPS_Pending)
        return;
    
    mgmt->next_node_id = NODE_ID_INIT;         
    mgmt->status = LCPS_Running;              
    mgmt->time_start = 0;          
    mgmt->time_out = 0;              
    mgmt->sub_process_mgmt = NULL;  

}

/**
  * @brief ��ռ����       �����̱���ռ�����ڵ�����������      //TODO �����̺������̶����ܱ�ǿռ
  * @param[in]  ptr_process �߼����ƽڵ�ָ��
 */
static void preempt_ctrl_process(const LogicControlNodeContext *_FAR ptr_process)
{
    CtrlProcessMgmtContext *_FAR ptr_ctrl_mgmt = NULL;
    if(ptr_process == NULL)
    {
        return;
    }
    ptr_ctrl_mgmt = get_main_process_mgmt_by_process_addr(ptr_process);
    if(ptr_ctrl_mgmt == NULL)
        return;
    
    (void)dbgprintf("--------------preempt process name %s----------------\n",(char *)ptr_ctrl_mgmt->name);
    restart_logic_ctrl_process(ptr_ctrl_mgmt);
}
/**
  * @brief	            �������̿��ƹ���
  *	@param[in]			ptr_process_mgmt        ���̹���ڵ�
  *	@param[in]			ptr_logic_ctrl          �������̵�ǰ����ִ�еĽڵ�
  *	@return
  *        1  ִ�гɹ�
  *        0  ִ��ʧ��
 */
static INT8U exe_state_ctrl_node(CtrlProcessMgmtContext *_FAR ptr_process_mgmt, LogicControlNodeContext *_FAR ptr_cur_node)
{
    INT8U object_id = 0;
    INT8U uRet = 0;
    RelayFaultCheckParam check_param = {0};
    
    if(NULL == ptr_process_mgmt || NULL == ptr_cur_node)
    {
        return SCER_FAIL;
    }
    (void)dbgprintf("-------->object_state_to_switch=%d, ctrl_prio=%d, object_id=%d\n", 
                ptr_cur_node->state_ctrl.object_state_to_switch, 
                ptr_cur_node->state_ctrl.ctrl_prio,
                ptr_cur_node->state_ctrl.object_id);

    object_id = ptr_cur_node->state_ctrl.object_id;
    switch(ptr_cur_node->state_ctrl.object_type)
    {
    case LCOT_VOID:
        break;

    case LCOT_RELAY://�̵�������

        switch(ptr_cur_node->state_ctrl.object_state_to_switch)
        {
        case STATE_FAULT_ADHESION://ճ�����
        case STATE_FAULT_OPEN: //��·���
            check_param.check_time = ptr_cur_node->state_ctrl.time_check;
            check_param.confirm_time = ptr_cur_node->state_ctrl.time_confirm;
            check_param.cond_func = ptr_cur_node->cond_selection.func_cond_select;
            if(ptr_cur_node->state_ctrl.object_state_to_switch == STATE_FAULT_ADHESION)
                check_param.type = kRelayFaultCheckAdhesion;
            else
                check_param.type = kRelayFaultCheckOpenCircuit;
            
            start_relay_fault_check(relay_control_get_id(object_id),&check_param);

            ptr_process_mgmt->time_start = get_tick_count();
            ptr_process_mgmt->time_out = ptr_cur_node->state_ctrl.time_check*3;
            
            if(ptr_cur_node->state_ctrl.object_state_to_switch == STATE_FAULT_ADHESION)
                ptr_process_mgmt->status = LCPS_Adhesion_Fault_Check_Waiting;
            else
                ptr_process_mgmt->status = LCPS_Open_Fault_Check_Waiting;
            return SCER_WAITING;
            break;

        default://�̵�������
            uRet = apply_state_ctrl(ptr_process_mgmt->apply_ptr_process,
                                    preempt_ctrl_process,
                                    atom_ctrl_operate,
                                    ptr_cur_node->state_ctrl.ctrl_prio,
                                    ptr_cur_node->state_ctrl.object_type,
                                    relay_control_get_id(object_id),
                                    ptr_cur_node->state_ctrl.object_state_to_switch);
            if(0 == uRet)
            {
                return SCER_FAIL;
            }

            break;
        }
        if(ptr_cur_node->state_ctrl.ctrl_delay != NULL)
        {
            
            ptr_process_mgmt->time_out = ptr_cur_node->state_ctrl.ctrl_delay();
            if(ptr_process_mgmt->time_out != 0) 
        {
            ptr_process_mgmt->time_start = get_tick_count();
            ptr_process_mgmt->status = LCPS_Relay_Ctrl_Waiting;
            return SCER_WAITING;
        }
        }
        break;

    case LCOT_VOLTAGE://���Ƶ�ѹ
    case LCOT_CURRENT://���Ƶ���
        uRet = apply_state_ctrl(ptr_process_mgmt->apply_ptr_process,
                                preempt_ctrl_process,
                                atom_ctrl_operate,
                                ptr_cur_node->state_ctrl.ctrl_prio,
                                ptr_cur_node->state_ctrl.object_type,
                                object_id,
                                ptr_cur_node->state_ctrl.object_state_to_switch);
        if(0 == uRet)
        {
            return SCER_FAIL;
        }
        
        break;

    default:
        break;
    }

    //ִ����Ϣ������
    if(NULL != ptr_cur_node->state_ctrl.func_msg)
    {
        ptr_cur_node->state_ctrl.func_msg();
    } 


    //������ʱ�ڵ㣬д����ʼʱ���ʱ��
    if(NULL != ptr_cur_node->state_ctrl.ctrl_delay)
    {
        ptr_process_mgmt->time_out = ptr_cur_node->state_ctrl.ctrl_delay();
        if(ptr_process_mgmt->time_out != 0) 
    {
        ptr_process_mgmt->time_start = get_tick_count();
        ptr_process_mgmt->status = LCPS_Delay;
        return SCER_DELAY;
    }
    }
    return SCER_SUCC;
}

/**
  * @brief ���̴���running״̬��ִ�����̿���
  * @param[in]  ptr_cur_node ��ǰ�ڵ�   ptr_mgmt  ��ǰ����
 */
void  process_mgmt_runing_handle(CtrlProcessMgmtContext *_FAR ptr_mgmt,LogicControlNodeContext  *_FAR ptr_cur_node)
{
    INT8U iLoop = 0;
    ConditionSelectFuncHandler  func = NULL;
    INT8U   next_node_id = 0;
    
    dbgprintf("-->LCPS_Running\n");

    for(iLoop = 0; iLoop < MAX_PROCESS_NODE_RUN_COUNT; iLoop++)
    {
        dbgprintf("---->process_id=%d,process_name=%s,node_type=%d, node_id=%d, next_node_id=%d\n",
                ptr_mgmt->process_id,
                (char *)ptr_mgmt->name,
                ptr_cur_node->node_type, 
                ptr_cur_node->node_id, 
                ptr_cur_node->next_node_id);
        

        switch(ptr_cur_node->node_type)
        {

        case LCNT_END://�ڵ�����Ϊ�յĴ�����������
            //����������
            dbgprintf("-->LCNT_END\n");
            
            if(ptr_mgmt->type == LCNT_PROCESS_MAIN)
            {
                ptr_mgmt->time_out = PROCESS_END_DELAY;
                ptr_mgmt->time_start = get_tick_count();
                ptr_mgmt->status = LCPS_End_Delay; 
                
                //stop_logic_ctrl_process(ptr_mgmt);      
                //ptr_mgmt->status = LCPS_Pending;
            }
            else  if(ptr_mgmt->type == LCNT_PROCESS_SUB)
            {
                //stop_logic_ctrl_process(ptr_mgmt);      
                ptr_mgmt->status = LCPS_End;
            }
            return;
            break;
            
        case LCNT_CONDITION_SELECT:	//����ѡ��
            dbgprintf("-->LCNT_CONDITION_SELECT\n");	
            func = ptr_cur_node->cond_selection.func_cond_select;
            if(func == NULL)
            {
                ptr_mgmt->status = LCPS_End;
                stop_logic_ctrl_process(ptr_mgmt);
                return;
            }

            if(func())
            {
                next_node_id = ptr_cur_node->next_node_id;
            }
            else
            {   //������Ҫ��һ��ʱ�����ж�
                if(NULL != ptr_cur_node->state_ctrl.ctrl_delay)
                {
                    ptr_mgmt->time_out = ptr_cur_node->state_ctrl.ctrl_delay();
                    if(ptr_mgmt->time_out != 0)
                {
                    ptr_mgmt->time_start = get_tick_count();
                    ptr_mgmt->status = LCPS_Delay;
                    return;
                }
                }
                next_node_id = ptr_cur_node->cond_selection.false_next_node_id;
            }
            break;

        case LCNT_STATE_CONTROL://״̬���ƽڵ㣬����״̬����
            //����������
            dbgprintf("-->LCNT_LCNT_STATE_CONTROL\n");
            if(NULL != ptr_cur_node->next_ctrl_process)
            {
                if(ptr_mgmt->type != LCNT_PROCESS_SUB)
                {
                    ptr_mgmt->sub_process_mgmt = start_sub_logic_ctrl_process((LogicControlNodeContext *_FAR)ptr_cur_node->next_ctrl_process, ptr_cur_node->next_ctrl_process_node_count,LCPS_Running,LCNT_PROCESS_SUB,ptr_mgmt->ptr_process);
                    //����ʱ
                    ptr_mgmt->time_out = 0;
                    ptr_mgmt->status = LCPS_Subprocess_Waiting;
                }
                return;
            }
            switch(exe_state_ctrl_node(ptr_mgmt, ptr_cur_node))
            {
                case SCER_SUCC:
                    next_node_id = ptr_cur_node->next_node_id;
                    break;
                case SCER_FAIL: 
                    next_node_id = NODE_ID_INIT;
                    break;
                case SCER_WAITING:  
                case SCER_DELAY:
                    return; 
                default:
                    break;
            }
            break;
        }

        //ѭ���ж�
            ptr_mgmt->next_node_id = next_node_id;
            ptr_cur_node = get_node_from_id(ptr_mgmt->ptr_process, ptr_mgmt->process_node_num, ptr_mgmt->next_node_id);
            if(NULL == ptr_cur_node)
            {
            return;
        }

    }
}

/**
  * @brief ���̴���subprocess״̬��ִ�����̿���
  * @param[in]  ptr_cur_node ��ǰ�ڵ�   ptr_mgmt  ��ǰ����
 */
 void  process_mgmt_subprocess_waiting_handle(CtrlProcessMgmtContext *_FAR ptr_mgmt,LogicControlNodeContext     *_FAR ptr_cur_node)
{
    dbgprintf("-->LCPS_Subprocess_Waiting\n");
    if(ptr_mgmt->sub_process_mgmt == NULL)
    {
        ptr_mgmt->status = LCPS_Running;
        ptr_mgmt->next_node_id = ptr_cur_node->next_node_id; 
    }
    else if(ptr_mgmt->sub_process_mgmt->status == LCPS_End)
    {
       //release_state_ctrl_all(ptr_mgmt->sub_process_mgmt->ptr_process);
       ctrl_process_mgmt_data_clear(ptr_mgmt->sub_process_mgmt);
       list_add_to_head(&free_ctrl_process_mgmt_list, (SDLIST_ITEM * _FAR)ptr_mgmt->sub_process_mgmt); 
       ptr_mgmt->sub_process_mgmt = NULL;
       ptr_mgmt->status = LCPS_Running;
       ptr_mgmt->next_node_id = ptr_cur_node->next_node_id; 
    }
}

/**
  * @brief ���̴���relay_fault_check_waiting״̬��ִ�����̿���
  * @param[in]  ptr_cur_node ��ǰ�ڵ�   ptr_mgmt  ��ǰ����
 */
void  process_mgmt_relay_fault_check_waiting_handle(CtrlProcessMgmtContext *_FAR ptr_mgmt,LogicControlNodeContext     *_FAR ptr_cur_node)
{
    RelayFaultCheckStatus status;
    RelayFaultCheckType type;
    dbgprintf("-->LCPS_Fault_Check_Waiting\n");
    type =  (ptr_mgmt->status == LCPS_Adhesion_Fault_Check_Waiting)?kRelayFaultCheckAdhesion:kRelayFaultCheckOpenCircuit;
    status = get_relay_fault_status(relay_control_get_id(ptr_cur_node->state_ctrl.object_id),type);
    
    if(status != kRelayFaultCheck_Checking)
    {
        ptr_mgmt->status = LCPS_Running;
        ptr_mgmt->next_node_id = ptr_cur_node->next_node_id;
    }
}


/**
  * @brief ���̴���delay״̬��ִ�����̿���
  * @param[in]  ptr_cur_node ��ǰ�ڵ�   ptr_mgmt  ��ǰ����
 */
void  process_mgmt_delay_handle(CtrlProcessMgmtContext *_FAR ptr_mgmt,LogicControlNodeContext *_FAR ptr_cur_node)
{
    ConditionSelectFuncHandler  func = NULL;
    INT32U now_tick = 0;
    dbgprintf("-->LCPS_Delay\n");
    now_tick = get_tick_count();
    func = ptr_cur_node->cond_selection.func_cond_select;
    //��һ��ʱ�����ж������ڵ�
    if(func != NULL)
    {
        if(func())
        {
            ptr_mgmt->next_node_id = ptr_cur_node->next_node_id;
            ptr_mgmt->time_start = 0;
            ptr_mgmt->time_out = 0;
            ptr_mgmt->status = LCPS_Running;
        }
        else
        {
            if(get_interval_by_tick(ptr_mgmt->time_start, now_tick) >= ptr_mgmt->time_out)
            {
                ptr_mgmt->next_node_id = ptr_cur_node->cond_selection.false_next_node_id;
                ptr_mgmt->time_start = 0;
                ptr_mgmt->time_out = 0;
                ptr_mgmt->status = LCPS_Running;
            }
        }
    }
    else
    {
        if(get_interval_by_tick(ptr_mgmt->time_start, now_tick) >= ptr_mgmt->time_out)
        {
            ptr_mgmt->next_node_id = ptr_cur_node->next_node_id;
            ptr_mgmt->time_start = 0;
            ptr_mgmt->time_out = 0;
            ptr_mgmt->status = LCPS_Running;
        }
    }
}

/**
  * @brief ���̴���end_delay״̬��ִ�����̿���
  * @param[in]  ptr_cur_node ��ǰ�ڵ�   ptr_mgmt  ��ǰ����
 */
void  process_end_delay_handle(CtrlProcessMgmtContext *_FAR ptr_mgmt)
{
    INT32U now_tick = 0;
    dbgprintf("-->LCPS_End_Delay\n");
    now_tick = get_tick_count();

    if(get_interval_by_tick(ptr_mgmt->time_start, now_tick) >= ptr_mgmt->time_out)
    {
        ptr_mgmt->time_start = 0;
        ptr_mgmt->time_out = 0;
        stop_logic_ctrl_process(ptr_mgmt);      
        ptr_mgmt->status = LCPS_Pending;
    }
}

/**
  * @brief ���̴���delay״̬��ִ�����̿���
  * @param[in]  ptr_cur_node ��ǰ�ڵ�   ptr_mgmt  ��ǰ����
 */
void  process_mgmt_relay_ctrl_waiting_handle(CtrlProcessMgmtContext *_FAR ptr_mgmt,LogicControlNodeContext *_FAR ptr_cur_node)
{
    INT32U now_tick = 0;
    INT8U relay_id = 0;
    dbgprintf("---->process_id=%d,process_name=%s,process status is LCPS_Relay_Ctrl_Waiting\n",
                ptr_mgmt->process_id,
                (char *)ptr_mgmt->name);
    now_tick = get_tick_count();
    relay_id = relay_control_get_id(ptr_cur_node->state_ctrl.object_id);
    if(relay_id == 0)
    {
        ptr_mgmt->next_node_id = ptr_cur_node->next_node_id;
        ptr_mgmt->time_start = 0;
        ptr_mgmt->time_out = 0;
        ptr_mgmt->status = LCPS_Running;
        return;
    }
    if(relay_is_on(relay_id) == ptr_cur_node->state_ctrl.object_state_to_switch)
    {
        ptr_mgmt->time_start = 0;
        ptr_mgmt->time_out = 0;
        ptr_mgmt->status = LCPS_Running;
        ptr_mgmt->next_node_id = ptr_cur_node->next_node_id;  
    }
    else
    {
        if(get_interval_by_tick(ptr_mgmt->time_start, now_tick) >= ptr_mgmt->time_out)
        {
            //ptr_mgmt->next_node_id = NODE_ID_INIT;
            //ptr_mgmt->time_start = 0;
            //ptr_mgmt->time_out = 0;
            //ptr_mgmt->status = LCPS_Running;
            restart_logic_ctrl_process(ptr_mgmt);
        }
    }
}
/**
  * @brief	            �������̿��ƹ���
  *	@param[in]			process_mgmt        ���̹���ڵ�
  *	@return	��
 */
static void single_ctrl_process_mgmt(CtrlProcessMgmtContext *_FAR ptr_mgmt)
{
    ConditionSelectFuncHandler  func = NULL;
    LogicControlNodeContext     *_FAR ptr_cur_node = NULL;

    if(ptr_mgmt == NULL)
       return;
    
    if(ptr_mgmt->status == LCPS_Pending)
       return;
    
    if(ptr_mgmt->type == LCNT_PROCESS_MAIN)
    {
        //���ҳ�ʼ�ڵ㣺Ϊһ������ѡ��ڵ�
        ptr_cur_node = get_node_from_id(ptr_mgmt->ptr_process, ptr_mgmt->process_node_num, NODE_ID_INIT);
        if(NULL == ptr_cur_node)
        {
            return;
        }
        func = ptr_cur_node->cond_selection.func_cond_select;
        if(NULL == func)
        {
            return;
        }
        //����������������Ƿ����(��ʼ�ڵ��е������ж�)�����������˳���һ��Ҫִ�еĽڵ㣬�ӳ�ʼ�ڵ㿪ʼ����
        if(!func())
        {
             if(ptr_mgmt->next_node_id == NODE_ID_INIT) 
             {
                 //ptr_mgmt->next_node_id = ptr_cur_node->cond_selection.false_next_node_id;
                 ptr_mgmt->status = LCPS_Running;
             } 
             else
             {
                 restart_logic_ctrl_process(ptr_mgmt);
             }  
             return;  
        }
        else if(ptr_mgmt->next_node_id == NODE_ID_INIT)
        {
            ptr_mgmt->next_node_id = ptr_cur_node->next_node_id;	 
        }
    }

    ptr_cur_node = get_node_from_id(ptr_mgmt->ptr_process, ptr_mgmt->process_node_num, ptr_mgmt->next_node_id);
    if(NULL == ptr_cur_node)
    {
        return;
    }
    
    dbgprintf("cur_process is %d\n",ptr_mgmt->process_id);
    
    switch(ptr_mgmt->status)//���ݵ�ǰ����״̬ѡ��ִ�з�֧
    {
    case LCPS_Running://����״̬
         process_mgmt_runing_handle(ptr_mgmt,ptr_cur_node);
        break;

    case LCPS_Subprocess_Waiting://�����̵ȴ�
        process_mgmt_subprocess_waiting_handle(ptr_mgmt,ptr_cur_node);
        break;

    case LCPS_Adhesion_Fault_Check_Waiting://ճ�����ϼ��ȴ�
    case LCPS_Open_Fault_Check_Waiting: //��·���ϼ��ȴ�
        process_mgmt_relay_fault_check_waiting_handle(ptr_mgmt,ptr_cur_node);
        break;
        
    case LCPS_Delay://�ж���ʱ�ﵽ
        process_mgmt_delay_handle(ptr_mgmt,ptr_cur_node);
        break;
    case LCPS_End://�����̽���
        dbgprintf("---->process_id=%d,process_name=%s,process status is LCPS_End\n",
                ptr_mgmt->process_id,
                (char *)ptr_mgmt->name);
        break;
    case LCPS_Pending://�����̹���
        dbgprintf("---->process_id=%d,process_name=%s,process status is LCPS_Pending\n",
                ptr_mgmt->process_id,
                (char *)ptr_mgmt->name);
        break;  
    case LCPS_Relay_Ctrl_Waiting:
         process_mgmt_relay_ctrl_waiting_handle(ptr_mgmt,ptr_cur_node);
         break;   
    case LCPS_End_Delay://�ж����̽�����ʱ�ﵽ
        process_end_delay_handle(ptr_mgmt);
        break;         
         
    default:
        break;
    }
}

/**
  * @brief	            ���̹����ʼ��
  *	@param[in]			ptr_process_mgmt        ���̹���ڵ�
  *	@param[in]			ptr_logic_ctrl          �������̽ڵ������׵�ַ
  *	@param[in]			process_node_num        �������̽ڵ�����ڵ���Ŀ
  *	@return	��
 */
static void init_ctrl_process_mgmt(CtrlProcessMgmtContext *_FAR ptr_process_mgmt,
                            LogicControlNodeContext *_FAR ptr_logic_ctrl,
                            INT8U process_node_num,
                            LogicCtrlProcessStatus init_status,
                            ProcessType type)
{
    if((ptr_process_mgmt == NULL)||(ptr_logic_ctrl == NULL)||(process_node_num == 0))
        return;
    ptr_process_mgmt->ptr_process = ptr_logic_ctrl;
    ptr_process_mgmt->process_node_num = process_node_num;
    ptr_process_mgmt->next_node_id = NODE_ID_INIT;
    ptr_process_mgmt->status = init_status;
    ptr_process_mgmt->time_start = 0;
    ptr_process_mgmt->time_out = 0;
    ptr_process_mgmt->type = type;
    ptr_process_mgmt->process_id = get_process_id(ptr_logic_ctrl,type);
    ptr_process_mgmt->name = get_process_name(ptr_logic_ctrl,type);
    ptr_process_mgmt->sub_process_mgmt = NULL;
    ptr_process_mgmt->apply_ptr_process = ptr_logic_ctrl;
   
}

/**
  * @brief	            �����߼���������
  *	@param[in]			ptr_logic_ctrl          �������̽ڵ������׵�ַ
  *	@param[in]			process_node_num        �������̽ڵ�����ڵ���Ŀ
  *	@return	��
 */
CtrlProcessMgmtContext  *_FAR start_logic_ctrl_process( LogicControlNodeContext *_FAR ptr_ctrl_process, 
                                                        INT8U process_node_num,
                                                        LogicCtrlProcessStatus init_status,
                                                        ProcessType type)
{
    CtrlProcessMgmtContext *_FAR ptr_ctrl_mgmt = NULL;

    if((ptr_ctrl_process == NULL)||(process_node_num == 0))
    {
        return NULL;
    }
    if((init_status !=LCPS_Pending)&&(init_status !=LCPS_Running))
    {
        return NULL;
    }
    if((type != LCNT_PROCESS_MAIN)&&(type != LCNT_PROCESS_SUB))
    {
        return NULL;
    }
    ptr_ctrl_mgmt = (CtrlProcessMgmtContext*_FAR)list_apply(&free_ctrl_process_mgmt_list);
    if(NULL == ptr_ctrl_mgmt)
    {
        return NULL;
    }
    //д�����̳�ʼ�ڵ���Ϣ
    init_ctrl_process_mgmt(ptr_ctrl_mgmt, ptr_ctrl_process, process_node_num,init_status,type);
    if(type == LCNT_PROCESS_MAIN)
    {
        if(ptr_ctrl_mgmt->process_id <= MAIN_PROCESS_MAX_NUM)
        {
            ptr_ctrl_process_mgmt_list[ptr_ctrl_mgmt->process_id] = ptr_ctrl_mgmt; 
        }
    }
    return ptr_ctrl_mgmt;
}


/**
  * @brief	            ���̹����ʼ��
  *	@param[in]			ptr_process_mgmt        ���̹���ڵ�
  *	@param[in]			ptr_logic_ctrl          �������̽ڵ������׵�ַ
  *	@param[in]			process_node_num        �������̽ڵ�����ڵ���Ŀ
  *	@return	��
 */
static void init_sub_ctrl_process_mgmt(CtrlProcessMgmtContext *_FAR ptr_process_mgmt,
                            LogicControlNodeContext *_FAR ptr_logic_ctrl,
                            INT8U process_node_num,
                            LogicCtrlProcessStatus init_status,
                            ProcessType type,
                            LogicControlNodeContext *_FAR apply_ptr_ctrl_process)
{
    if((ptr_process_mgmt == NULL)||(ptr_logic_ctrl == NULL)||(process_node_num == 0))
        return;
    ptr_process_mgmt->ptr_process = ptr_logic_ctrl;
    ptr_process_mgmt->process_node_num = process_node_num;
    ptr_process_mgmt->next_node_id = NODE_ID_INIT;
    ptr_process_mgmt->status = init_status;
    ptr_process_mgmt->time_start = 0;
    ptr_process_mgmt->time_out = 0;
    ptr_process_mgmt->type = type;
    ptr_process_mgmt->process_id = get_process_id(ptr_logic_ctrl,type);
    ptr_process_mgmt->name = get_process_name(ptr_logic_ctrl,type);
    ptr_process_mgmt->sub_process_mgmt = NULL;
    ptr_process_mgmt->apply_ptr_process = apply_ptr_ctrl_process;
   
}

/**
  * @brief	            �����߼���������
  *	@param[in]			ptr_logic_ctrl          �������̽ڵ������׵�ַ
  *	@param[in]			process_node_num        �������̽ڵ�����ڵ���Ŀ
  *	@return	��
 */
CtrlProcessMgmtContext  *_FAR start_sub_logic_ctrl_process( LogicControlNodeContext *_FAR ptr_ctrl_process, 
                                                        INT8U process_node_num,
                                                        LogicCtrlProcessStatus init_status,
                                                        ProcessType type,
                                                        LogicControlNodeContext *_FAR apply_ptr_ctrl_process)
{
    CtrlProcessMgmtContext *_FAR ptr_ctrl_mgmt = NULL;

    if((ptr_ctrl_process == NULL)||(process_node_num == 0))
    {
        return NULL;
    }
    if((init_status !=LCPS_Pending)&&(init_status !=LCPS_Running))
    {
        return NULL;
    }
    if((type != LCNT_PROCESS_MAIN)&&(type != LCNT_PROCESS_SUB))
    {
        return NULL;
    }
    ptr_ctrl_mgmt = (CtrlProcessMgmtContext*_FAR)list_apply(&free_ctrl_process_mgmt_list);
    if(NULL == ptr_ctrl_mgmt)
    {
        return NULL;
    }
    //д�����̳�ʼ�ڵ���Ϣ
    init_sub_ctrl_process_mgmt(ptr_ctrl_mgmt, ptr_ctrl_process, process_node_num,init_status,type,apply_ptr_ctrl_process);
    if(type == LCNT_PROCESS_MAIN)
    {
        if(ptr_ctrl_mgmt->process_id <= MAIN_PROCESS_MAX_NUM)
        {
            ptr_ctrl_process_mgmt_list[ptr_ctrl_mgmt->process_id] = ptr_ctrl_mgmt; 
        }
    }
    return ptr_ctrl_mgmt;
}

/**
  * @brief          �߼�������������
  *	@return         ��
 */
void ctrl_process_mgmt_run(void)
{
    INT8U pcount = 0;
    CtrlProcessMgmtContext *_FAR sub_process_mgmt = NULL;

    for(pcount = 1; pcount <= MAIN_PROCESS_MAX_NUM; pcount++) 
    {
        need_run_process_array[pcount] = NULL;
        if(ptr_ctrl_process_mgmt_list[pcount] == NULL)
            continue;
        if(ptr_ctrl_process_mgmt_list[pcount]->status == LCPS_Subprocess_Waiting)//���ڵȴ��������� 
        {
          sub_process_mgmt = ptr_ctrl_process_mgmt_list[pcount]->sub_process_mgmt;
          if((sub_process_mgmt != NULL)&&(sub_process_mgmt->status != LCPS_End))//������û�н���
          { 
            need_run_process_array[pcount] = ptr_ctrl_process_mgmt_list[pcount]->sub_process_mgmt;
            continue;
          }
        }
        need_run_process_array[pcount] = ptr_ctrl_process_mgmt_list[pcount];
    }
    
    for(pcount = 1; pcount <= MAIN_PROCESS_MAX_NUM; pcount++) 
    {
        single_ctrl_process_mgmt(need_run_process_array[pcount]);
    }
}
