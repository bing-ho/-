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
内部数据结构定义
***************************************************************/

//每次流程节点运行最大数目
#define MAX_PROCESS_NODE_RUN_COUNT       30


#pragma DATA_SEG __RPAGE_SEG PAGED_RAM_LOGIC_CTRL
/// 定义流程，流程支持个数为20个
static CtrlProcessMgmtContext     ctrl_process_mgmt[PROCESS_LOGIC_CTRL_CONTEXT_MAX_NUM];
/// 流程管理数组
static CtrlProcessMgmtContext *_FAR  ptr_ctrl_process_mgmt_list[MAIN_PROCESS_MAX_NUM+1];
static CtrlProcessMgmtContext *_FAR  need_run_process_array[MAIN_PROCESS_MAX_NUM+1];

/// 待使用的流程管理空间
static SDLIST_ITEM  free_ctrl_process_mgmt_list = {0};
#pragma DATA_SEG __RPAGE_SEG DEFAULT

/***************************************************************
内部函数声明
***************************************************************/


/***************************************************************
函数实现
***************************************************************/

/**
  * @brief	   清除流程管理中的数据
  * @param[in] mgmt
  *	@return	   无
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
  * @brief	流程管理链表初始化
  *	@return	无
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
  * @brief          获取流程id号
  *	@param[in]      ptr_process             流程数组首地址
  *	@return         流程id        0为无效ID
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
  * @brief          获取流程名字
  *	@param[in]      ptr_process             流程数组首地址
  *	@return         流程名字
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
  * @brief          由节点编号查找节点
  *	@param[in]      ptr_process             流程数组首地址
  *	@param[in]      node_num                流程中的节点数目
  *	@param[in]      id                      控制节点的编号
  *	@return         控制节点指针
  * @note
  * @li             返回为NULL表示未查找到指定id的节点
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
  * @brief	            停止逻辑控制流程
  *	@param[in]			ptr_process		流程管理节点
  *	@return	无
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
  * @brief	            重启逻辑控制流程
  *	@param[in]			ptr_process		流程管理节点
  *	@return	无
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
  * @brief	            用流程数组首地址查找主流程，若流程数组为子流程，查找它所在的主流程
  *	@param[in]			ptr_process		流程管理节点
  *	@return	无
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
  * @brief	            唤醒主流程
  *	@param[in]			ptr_ctrl_process		流程管理节点
  *	@return	无
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
  * @brief 抢占控制       子流程被抢占，所在的主流程重启      //TODO 主流程和子流程都可能被强占
  * @param[in]  ptr_process 逻辑控制节点指针
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
  * @brief	            单个流程控制管理
  *	@param[in]			ptr_process_mgmt        流程管理节点
  *	@param[in]			ptr_logic_ctrl          控制流程当前正在执行的节点
  *	@return
  *        1  执行成功
  *        0  执行失败
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

    case LCOT_RELAY://继电器类型

        switch(ptr_cur_node->state_ctrl.object_state_to_switch)
        {
        case STATE_FAULT_ADHESION://粘连检测
        case STATE_FAULT_OPEN: //开路检测
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

        default://继电器控制
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

    case LCOT_VOLTAGE://控制电压
    case LCOT_CURRENT://控制电流
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

    //执行消息处理函数
    if(NULL != ptr_cur_node->state_ctrl.func_msg)
    {
        ptr_cur_node->state_ctrl.func_msg();
    } 


    //启动延时节点，写入起始时间和时长
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
  * @brief 流程处于running状态，执行流程控制
  * @param[in]  ptr_cur_node 当前节点   ptr_mgmt  当前流程
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

        case LCNT_END://节点类型为空的处理：结束流程
            //结束子流程
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
            
        case LCNT_CONDITION_SELECT:	//条件选择
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
            {   //条件需要在一段时间内判断
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

        case LCNT_STATE_CONTROL://状态控制节点，申请状态控制
            //启动子流程
            dbgprintf("-->LCNT_LCNT_STATE_CONTROL\n");
            if(NULL != ptr_cur_node->next_ctrl_process)
            {
                if(ptr_mgmt->type != LCNT_PROCESS_SUB)
                {
                    ptr_mgmt->sub_process_mgmt = start_sub_logic_ctrl_process((LogicControlNodeContext *_FAR)ptr_cur_node->next_ctrl_process, ptr_cur_node->next_ctrl_process_node_count,LCPS_Running,LCNT_PROCESS_SUB,ptr_mgmt->ptr_process);
                    //不超时
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

        //循环判断
            ptr_mgmt->next_node_id = next_node_id;
            ptr_cur_node = get_node_from_id(ptr_mgmt->ptr_process, ptr_mgmt->process_node_num, ptr_mgmt->next_node_id);
            if(NULL == ptr_cur_node)
            {
            return;
        }

    }
}

/**
  * @brief 流程处于subprocess状态，执行流程控制
  * @param[in]  ptr_cur_node 当前节点   ptr_mgmt  当前流程
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
  * @brief 流程处于relay_fault_check_waiting状态，执行流程控制
  * @param[in]  ptr_cur_node 当前节点   ptr_mgmt  当前流程
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
  * @brief 流程处于delay状态，执行流程控制
  * @param[in]  ptr_cur_node 当前节点   ptr_mgmt  当前流程
 */
void  process_mgmt_delay_handle(CtrlProcessMgmtContext *_FAR ptr_mgmt,LogicControlNodeContext *_FAR ptr_cur_node)
{
    ConditionSelectFuncHandler  func = NULL;
    INT32U now_tick = 0;
    dbgprintf("-->LCPS_Delay\n");
    now_tick = get_tick_count();
    func = ptr_cur_node->cond_selection.func_cond_select;
    //在一段时间内判断条件节点
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
  * @brief 流程处于end_delay状态，执行流程控制
  * @param[in]  ptr_cur_node 当前节点   ptr_mgmt  当前流程
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
  * @brief 流程处于delay状态，执行流程控制
  * @param[in]  ptr_cur_node 当前节点   ptr_mgmt  当前流程
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
  * @brief	            单个流程控制管理
  *	@param[in]			process_mgmt        流程管理节点
  *	@return	无
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
        //查找初始节点：为一个条件选择节点
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
        //检查流程运行条件是否成立(初始节点中的条件判断)，不成立则退出下一个要执行的节点，从初始节点开始流程
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
    
    switch(ptr_mgmt->status)//根据当前流程状态选择执行分支
    {
    case LCPS_Running://运行状态
         process_mgmt_runing_handle(ptr_mgmt,ptr_cur_node);
        break;

    case LCPS_Subprocess_Waiting://子流程等待
        process_mgmt_subprocess_waiting_handle(ptr_mgmt,ptr_cur_node);
        break;

    case LCPS_Adhesion_Fault_Check_Waiting://粘连故障检测等待
    case LCPS_Open_Fault_Check_Waiting: //开路故障检测等待
        process_mgmt_relay_fault_check_waiting_handle(ptr_mgmt,ptr_cur_node);
        break;
        
    case LCPS_Delay://判断延时达到
        process_mgmt_delay_handle(ptr_mgmt,ptr_cur_node);
        break;
    case LCPS_End://子流程结束
        dbgprintf("---->process_id=%d,process_name=%s,process status is LCPS_End\n",
                ptr_mgmt->process_id,
                (char *)ptr_mgmt->name);
        break;
    case LCPS_Pending://主流程挂起
        dbgprintf("---->process_id=%d,process_name=%s,process status is LCPS_Pending\n",
                ptr_mgmt->process_id,
                (char *)ptr_mgmt->name);
        break;  
    case LCPS_Relay_Ctrl_Waiting:
         process_mgmt_relay_ctrl_waiting_handle(ptr_mgmt,ptr_cur_node);
         break;   
    case LCPS_End_Delay://判断流程结束延时达到
        process_end_delay_handle(ptr_mgmt);
        break;         
         
    default:
        break;
    }
}

/**
  * @brief	            流程管理初始化
  *	@param[in]			ptr_process_mgmt        流程管理节点
  *	@param[in]			ptr_logic_ctrl          控制流程节点数组首地址
  *	@param[in]			process_node_num        控制流程节点数组节点数目
  *	@return	无
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
  * @brief	            启动逻辑控制流程
  *	@param[in]			ptr_logic_ctrl          控制流程节点数组首地址
  *	@param[in]			process_node_num        控制流程节点数组节点数目
  *	@return	无
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
    //写入流程初始节点信息
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
  * @brief	            流程管理初始化
  *	@param[in]			ptr_process_mgmt        流程管理节点
  *	@param[in]			ptr_logic_ctrl          控制流程节点数组首地址
  *	@param[in]			process_node_num        控制流程节点数组节点数目
  *	@return	无
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
  * @brief	            启动逻辑控制流程
  *	@param[in]			ptr_logic_ctrl          控制流程节点数组首地址
  *	@param[in]			process_node_num        控制流程节点数组节点数目
  *	@return	无
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
    //写入流程初始节点信息
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
  * @brief          逻辑控制流程运行
  *	@return         无
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
        if(ptr_ctrl_process_mgmt_list[pcount]->status == LCPS_Subprocess_Waiting)//处于等待子流程中 
        {
          sub_process_mgmt = ptr_ctrl_process_mgmt_list[pcount]->sub_process_mgmt;
          if((sub_process_mgmt != NULL)&&(sub_process_mgmt->status != LCPS_End))//子流程没有结束
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
