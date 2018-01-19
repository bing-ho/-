/**
  * @file       ctrl_mgmt.c
  * @brief      逻辑控制流程处理
  *	@copyright	Ligoo Inc.
  *	@date       2017-03-03
  *	@author     Ligoo软件逻辑组
  */

#include "ctrl_mgmt.h"
#include "relay_fault_check.h"


#pragma MESSAGE DISABLE C4800
#pragma MESSAGE DISABLE C12056 // SP debug info incorrect because of optimization or inline assembler
/***************************************************************
内部数据结构定义
***************************************************************/

#pragma DATA_SEG __RPAGE_SEG PAGED_RAM_LOGIC_CTRL
/// 受控对象状态控制空间分配
static ObjectStateControl		obj_state_ctrl[MAX_OBJ_COUNT*MAX_CTRL_ON_OBJ];

/// 控制管理空间分配
static ControlManagement		ctrl_mgmt_arr[MAX_OBJ_COUNT];

/// 控制管理链表
static SDLIST_ITEM  ptr_ctrl_mgmt_list = {0};

/// 可分配的控制管理空间
static SDLIST_ITEM  free_ctrl_mgmt = {0};

/// 可分配的受控对象状态控制空间
static SDLIST_ITEM  free_obj_state_ctrl = {0};
#pragma DATA_SEG __RPAGE_SEG DEFAULT


/**
  * @enum   FIND_TYPE
  * @brief  受控对象与流程的查找结果
  */
typedef enum 
{
    OBJECT_NULL = 0,										///< 控制管理层没有该对象的控制
    OBJECT_EXIST_PROCESS_NULL,								///< 控制管理层有该对象的控制，但没有该流程的控制
    OBJECT_AND_PROCESS_EXIST								///< 控制管理层有该对象的控制，也有该流程的控制
}FIND_TYPE;

/**
  * @enum   RELEASE_CTRL_RESULT
  * @brief  释放控制处理的结果
  */
typedef enum
{
    RELEASE_VOID = 0,                                       ///< 没有找到需释放的节点
    RELEASE_OBJ_CTRL,                                       ///< 释放单个流程对某个对象的控制
    RELEASE_CTRL_MGMT                                       ///< 释放了对象上的控制管理节点
}RELEASE_CTRL_RESULT;


/***************************************************************
内部函数声明
***************************************************************/



/***************************************************************
函数实现
***************************************************************/
void reset_obj_ctrl(ControlManagement *_FAR ptr_ctrl_mgmt, ObjectStateControl*_FAR ptr_obj_state_ctrl)
{
    if(ptr_ctrl_mgmt != NULL)
    {
        ptr_ctrl_mgmt->next = NULL;
        ptr_ctrl_mgmt->object_type = LCOT_VOID;
        ptr_ctrl_mgmt->object_id = 0;																							//初始化值待确定
        ptr_ctrl_mgmt->cur_state = STATE_INVALID;
    }

    if(ptr_obj_state_ctrl != NULL)
    {
        ptr_obj_state_ctrl->next = NULL;
        ptr_obj_state_ctrl->ptr_ctrl_process = NULL;
        ptr_obj_state_ctrl->func_ctrl_preempt = NULL;
        ptr_obj_state_ctrl->func_state_ctrl = NULL;
        ptr_obj_state_ctrl->state_to_switch = STATE_INVALID;
        ptr_obj_state_ctrl->ctrl_prio = PRIO_INVALID;
    }
}
/**
  * @brief	控制链表初始化
  *	@return	无
 */
void ctrl_mgmt_init(void)
{
    INT16U i = 0;

    for(i=0; i<MAX_OBJ_COUNT; i++)
    {
        list_add_to_head(&free_ctrl_mgmt, (SDLIST_ITEM *_FAR)&ctrl_mgmt_arr[i]);
    }

    for(i=0; i<MAX_OBJ_COUNT*MAX_CTRL_ON_OBJ; i++)
    {
        list_add_to_head(&free_obj_state_ctrl, (SDLIST_ITEM *_FAR)&obj_state_ctrl[i]);
    }
}
/**
  * @brief	       单个被控对象的处理
  * @param[in]	   ptr_ctrl_mgmt	单个受控对象上的控制管理数据结构
  *	@return	无
 */
static void ctrl_on_sigle_object(ControlManagement *_FAR ptr_ctrl_mgmt)
{
    INT16U ctrl_state = STATE_INVALID;		//控制状态临时变量
    ObjectStateControl  *_FAR ptr_ctrl = NULL;
    ObjectStateControl  *_FAR ptr_ctrl_real = NULL;

    ptr_ctrl = (ObjectStateControl *_FAR)ptr_ctrl_mgmt->ctrl_list.next;
    if(ptr_ctrl == NULL)
    {
        return;
    }
    //查找最高优先级的控制
    while(NULL != ptr_ctrl)
    {
        if(NULL != ptr_ctrl->ptr_ctrl_process)			//表示流程对控制对象有控制
        {
            switch(ptr_ctrl_mgmt->object_type)				//判断控制对象类型
            {
            case LCOT_RELAY:							//继电器
                 if( STATE_INVALID == ctrl_state )
                {
                    ctrl_state = ptr_ctrl->state_to_switch;
                    ptr_ctrl_real = ptr_ctrl;
                }
                else
                {
                    if( ptr_ctrl->state_to_switch == STATE_OPEN )
                    {
                        ctrl_state = STATE_OPEN;
                        ptr_ctrl_real = ptr_ctrl;
                    }
                }

                break;

            case LCOT_VOLTAGE:							//电压、电流
            case LCOT_CURRENT:
                if(ptr_ctrl->state_to_switch < ctrl_state)
                {
                    ctrl_state = ptr_ctrl->state_to_switch;
                    ptr_ctrl_real = ptr_ctrl;
                }
                break;

            default:
                break;
            }
        }

        ptr_ctrl = (ObjectStateControl *_FAR)ptr_ctrl->next;
    }

    //执行最高优先级的控制，抢占处理
    ptr_ctrl = (ObjectStateControl *_FAR)ptr_ctrl_mgmt->ctrl_list.next;
    while(ptr_ctrl != NULL)
    {
        //执行控制抢占处理函数
        if(ptr_ctrl->state_to_switch != ctrl_state)
        {
            //抢占回调
            ptr_ctrl->func_ctrl_preempt(ptr_ctrl->ptr_ctrl_process);

            //释放节点
            list_release(&(ptr_ctrl_mgmt->ctrl_list), (SDLIST_ITEM *_FAR)ptr_ctrl, &free_obj_state_ctrl);
        }

        ptr_ctrl = (ObjectStateControl *_FAR)ptr_ctrl->next;
    }
    
    /*(void)dbgprintf("cur_state=%d,object_type=%d,object_id=%d\n",
            ptr_ctrl_mgmt->cur_state, 
            ptr_ctrl_mgmt->object_type,
            ptr_ctrl_mgmt->object_id);    */

    //最终控制状态与当前状态相同
    if(ptr_ctrl_mgmt->cur_state == ctrl_state)
    {
        return;
    }

    //保存最终控制状态
    ptr_ctrl_mgmt->cur_state = ctrl_state;

    //执行底层的控制函数
    (void)ptr_ctrl_real->func_state_ctrl(ptr_ctrl_mgmt->object_type, ptr_ctrl_mgmt->object_id, ptr_ctrl_mgmt->cur_state);
}
/**
  * @brief        控制管理任务
  *	@return         无
 */
void basic_logic_control_run(void)
{
    SDLIST_ITEM *_FAR object_list_next = NULL;
    SDLIST_ITEM *_FAR object_list = NULL;

    object_list_next = ptr_ctrl_mgmt_list.next;
    (void)dbgprintf("\nobject control \n");
    while(object_list_next != NULL)
    {
        object_list = object_list_next;
        object_list_next = object_list_next->next;
        ctrl_on_sigle_object((ControlManagement *_FAR)object_list);    //轮询所有控制对象
    }
}

/**
  * @brief				查找控制管理链表中是否已经有对该对象的控制
  *	@param[in]			ptr_ctrl_process		流程的指针
  *	@param[in]			object_type				受控对象的类型
  *	@param[in]			object_id				受控对象的ID
  *	@param[out]			pp_ctrl_mgmt            单个对象的控制管理
  *	@param[out]			pp_obj_state_ctrl       单个流程对单个对象的控制
  *	@return				受控对象与流程的查找结果
 */
static FIND_TYPE find_object(void *_FAR ptr_ctrl_process,
                      LogicControlObjectType object_type,
                      INT8U object_id,
                      ControlManagement *_FAR*_FAR pp_ctrl_mgmt,
                      ObjectStateControl *_FAR*_FAR pp_obj_state_ctrl)
{
    ControlManagement *_FAR ptr_ctrl_mgmt = (ControlManagement *_FAR)ptr_ctrl_mgmt_list.next;
    ObjectStateControl *_FAR ptr_obj_state_ctrl = NULL;

    while(ptr_ctrl_mgmt != NULL)
    {
        if((ptr_ctrl_mgmt->object_type == object_type) && (ptr_ctrl_mgmt->object_id == object_id))
        {
            ptr_obj_state_ctrl = (ObjectStateControl *_FAR)ptr_ctrl_mgmt->ctrl_list.next;

            while(ptr_obj_state_ctrl != NULL)
            {
                if(ptr_obj_state_ctrl->ptr_ctrl_process == ptr_ctrl_process)
                {
                    *pp_ctrl_mgmt = ptr_ctrl_mgmt;
                    *pp_obj_state_ctrl = ptr_obj_state_ctrl;

                    return OBJECT_AND_PROCESS_EXIST; //对象和管理都存在
                }

                ptr_obj_state_ctrl = (ObjectStateControl *_FAR)ptr_obj_state_ctrl->next; //下一个控制对象
            }

            *pp_ctrl_mgmt = ptr_ctrl_mgmt;
            return OBJECT_EXIST_PROCESS_NULL; 
        }

        ptr_ctrl_mgmt = (ControlManagement *_FAR)ptr_ctrl_mgmt->next; //下一个控制管理
    }

    return OBJECT_NULL;
}

/**
  * @brief				申请单个流程对单个受控对象的状态控制
  *	@param[in]			ptr_ctrl_process		流程的指针
  *	@param[in]			func_ctrl_preempt       抢占回调函数
  *	@param[in]			func_state_ctrl			控制函数
  *	@param[in]			ctrl_prio				控制的优先级
  *	@param[in]			object_type				受控对象的类型
  *	@param[in]			object_id				受控对象的ID
  *	@param[in]			state_to_switch			受控对象要达到的状态
  *	@return	            无
 */
INT8U apply_state_ctrl(void *_FAR ptr_ctrl_process,
                      PreemptFuncHandler func_ctrl_preempt,
                      ObjectStateCtrlFuncHandler func_state_ctrl,
                      INT8U ctrl_prio,
                      LogicControlObjectType object_type,
                      INT8U object_id,
                      INT16U state_to_switch)
{
    ControlManagement *_FAR ptr_ctrl_mgmt = NULL;
    ObjectStateControl *_FAR ptr_obj_state_ctrl = NULL;
    FIND_TYPE find_result = 0;
    
    if((ptr_ctrl_process == NULL)||
    (func_ctrl_preempt == NULL)||
    (func_state_ctrl == NULL))
       return 0;
    
    //(void)dbgprintf("apply object_id=%d\n",object_id);
    
    //查找控制管理链表中是否已经有该对象的控制
    find_result = find_object((void*_FAR)ptr_ctrl_process, object_type, object_id,
                              &ptr_ctrl_mgmt, &ptr_obj_state_ctrl);
    
    //如果有该流程对该对象控制，只需更新控制的相关参数
    switch(find_result)
    {
    case OBJECT_AND_PROCESS_EXIST: //对象和流程控制都存在
        break;

    case OBJECT_EXIST_PROCESS_NULL: //对象存在
        ptr_obj_state_ctrl = (ObjectStateControl *_FAR)list_apply(&free_obj_state_ctrl);
        if(NULL == ptr_obj_state_ctrl)
        {
            //TODO
            return 0;
        }

        list_add_to_head(&(ptr_ctrl_mgmt->ctrl_list), (SDLIST_ITEM *_FAR)ptr_obj_state_ctrl);
        break;

    case OBJECT_NULL:
        ptr_ctrl_mgmt = (ControlManagement *_FAR)list_apply(&free_ctrl_mgmt);
        if(NULL == ptr_ctrl_mgmt)
        {
            return 0;
        }
        reset_obj_ctrl(ptr_ctrl_mgmt,NULL);

        ptr_obj_state_ctrl = (ObjectStateControl *_FAR)list_apply(&free_obj_state_ctrl);
        if(NULL == ptr_obj_state_ctrl)
        {
            return 0;
        }

        list_add_to_head(&ptr_ctrl_mgmt_list, (SDLIST_ITEM*_FAR)ptr_ctrl_mgmt);
        list_add_to_head(&(ptr_ctrl_mgmt->ctrl_list), (SDLIST_ITEM*_FAR)ptr_obj_state_ctrl);

        ptr_ctrl_mgmt->object_type = object_type;
        ptr_ctrl_mgmt->object_id = object_id;
        ptr_ctrl_mgmt->ctrl_list.next = (SDLIST_ITEM*_FAR)ptr_obj_state_ctrl;
        break;

    default:
        break;
    }

    ptr_obj_state_ctrl->ptr_ctrl_process = ptr_ctrl_process;
    ptr_obj_state_ctrl->func_ctrl_preempt = func_ctrl_preempt;
    ptr_obj_state_ctrl->func_state_ctrl = func_state_ctrl;
    ptr_obj_state_ctrl->state_to_switch = state_to_switch;
    ptr_obj_state_ctrl->ctrl_prio = ctrl_prio;
    
    return 1;
}
/**
  * @brief				释放单个流程对单个受控对象的状态控制
  *	@param[in]			ptr_ctrl_process		流程的指针
  *	@param[in]			ptr_ctrl_mgmt			单个受控对象的控制管理指针
  *	@return
  * \li RELEASE_VOID        没有找到需释放的节点
  * \li RELEASE_OBJ_CTRL    释放单个流程对某个对象的控制
  * \li RELEASE_CTRL_MGMT   释放了对象上的控制管理节点
 */
static RELEASE_CTRL_RESULT release_state_ctrl_from_ctrl_mgmt(void *_FAR ptr_ctrl_process, ControlManagement *_FAR ptr_ctrl_mgmt, ControlManagement *_FAR ptr_ctrl_mgmt_prev)
{
    ObjectStateControl *_FAR ptr_obj_ctrl = NULL;
    ObjectStateControl *_FAR ptr_obj_ctrl_prev = NULL;
    RELEASE_CTRL_RESULT  uRet = RELEASE_VOID;

    if((ptr_ctrl_process == NULL)||(ptr_ctrl_mgmt == NULL)||(ptr_ctrl_mgmt_prev == NULL))
    {
        return uRet;
    }

    ptr_obj_ctrl_prev = (ObjectStateControl *_FAR)&(ptr_ctrl_mgmt->ctrl_list);
    ptr_obj_ctrl = (ObjectStateControl *_FAR)ptr_ctrl_mgmt->ctrl_list.next;

    //单个对象上的控制流程
    while(NULL != ptr_obj_ctrl)
    {
        if( ptr_ctrl_process == ptr_obj_ctrl->ptr_ctrl_process)
        {
            uRet = RELEASE_OBJ_CTRL;

            //流程控制对象：查找成功
            ptr_obj_ctrl->ptr_ctrl_process = NULL;

            //修改链表
            ptr_obj_ctrl_prev->next = ptr_obj_ctrl->next;

            //回收空间
            ptr_obj_ctrl->next = free_obj_state_ctrl.next;
            free_obj_state_ctrl.next = (SDLIST_ITEM *_FAR)ptr_obj_ctrl;

            //对象上没有流程对其进行控制
            if(NULL == ptr_ctrl_mgmt->ctrl_list.next)
            {
                uRet = RELEASE_CTRL_MGMT;

                //修改链表
                ptr_ctrl_mgmt_prev->next = ptr_ctrl_mgmt->next;

                //回收空间
                ptr_ctrl_mgmt->next = free_ctrl_mgmt.next;
                free_ctrl_mgmt.next = (SDLIST_ITEM *_FAR)ptr_ctrl_mgmt;
            }

            break;
        }

        ptr_obj_ctrl_prev = ptr_obj_ctrl;
        ptr_obj_ctrl = (ObjectStateControl *_FAR)ptr_obj_ctrl->next;
    }

    return uRet;
}
/**
  * @brief				释放单个流程对单个受控对象的状态控制
  *	@param[in]			ptr_ctrl_process		流程的指针
  *	@param[in]			object_type				受控对象的类型
  *	@param[in]			object_id				受控对象的ID
  *	@return	无
 */
void release_state_ctrl(void *_FAR ptr_ctrl_process,
                        LogicControlObjectType object_type,
                        INT8U object_id)
{
    ControlManagement *_FAR ptr_ctrl_mgmt = NULL;
    ControlManagement *_FAR ptr_ctrl_mgmt_prev = NULL;
    INT8U uRet = 0;
    
    if(ptr_ctrl_process == NULL)
    {
        return;
    }

    ptr_ctrl_mgmt_prev = (ControlManagement *_FAR)&ptr_ctrl_mgmt_list;
    ptr_ctrl_mgmt = (ControlManagement *_FAR)ptr_ctrl_mgmt_list.next;

    //各个对象上的控制集合
    while(NULL != ptr_ctrl_mgmt)
    {
        if(object_type == ptr_ctrl_mgmt->object_type && object_id == ptr_ctrl_mgmt->object_id)
        {
            (void)release_state_ctrl_from_ctrl_mgmt(ptr_ctrl_process, ptr_ctrl_mgmt, ptr_ctrl_mgmt_prev);
            break;
        }

        ptr_ctrl_mgmt_prev = ptr_ctrl_mgmt;
        ptr_ctrl_mgmt = (ControlManagement *_FAR)ptr_ctrl_mgmt->next;
    }
}

/**
  * @brief				释放单个流程对所有受控对象的状态控制
  *	@param[in]			ptr_ctrl_process		流程的指针
  *	@return	无
 */
void release_state_ctrl_all(void *_FAR ptr_ctrl_process)
{
    ControlManagement *_FAR ptr_ctrl_mgmt = NULL;
    ControlManagement *_FAR ptr_ctrl_mgmt_prev = NULL;
    ControlManagement *_FAR ptr_ctrl_mgmt_next = NULL;
    RELEASE_CTRL_RESULT uRet = 0;
    
    if(ptr_ctrl_process == NULL)
    {
        return;
    }

    ptr_ctrl_mgmt_prev = (ControlManagement *_FAR)&ptr_ctrl_mgmt_list;
    ptr_ctrl_mgmt = (ControlManagement *_FAR)ptr_ctrl_mgmt_list.next;

    //各个对象上的控制集合
    while(NULL != ptr_ctrl_mgmt)
    {
        ptr_ctrl_mgmt_next = (ControlManagement *_FAR)ptr_ctrl_mgmt->next;

        uRet = release_state_ctrl_from_ctrl_mgmt(ptr_ctrl_process, ptr_ctrl_mgmt, ptr_ctrl_mgmt_prev);
        if( RELEASE_CTRL_MGMT == uRet )
        {
            ptr_ctrl_mgmt = ptr_ctrl_mgmt_next;
        }
        else
        {
            ptr_ctrl_mgmt_prev = ptr_ctrl_mgmt;
            ptr_ctrl_mgmt = (ControlManagement *_FAR)ptr_ctrl_mgmt->next;
        }
    }
}


