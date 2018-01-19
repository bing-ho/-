/**
  * @file       ctrl_mgmt.c
  * @brief      �߼��������̴���
  *	@copyright	Ligoo Inc.
  *	@date       2017-03-03
  *	@author     Ligoo����߼���
  */

#include "ctrl_mgmt.h"
#include "relay_fault_check.h"


#pragma MESSAGE DISABLE C4800
#pragma MESSAGE DISABLE C12056 // SP debug info incorrect because of optimization or inline assembler
/***************************************************************
�ڲ����ݽṹ����
***************************************************************/

#pragma DATA_SEG __RPAGE_SEG PAGED_RAM_LOGIC_CTRL
/// �ܿض���״̬���ƿռ����
static ObjectStateControl		obj_state_ctrl[MAX_OBJ_COUNT*MAX_CTRL_ON_OBJ];

/// ���ƹ���ռ����
static ControlManagement		ctrl_mgmt_arr[MAX_OBJ_COUNT];

/// ���ƹ�������
static SDLIST_ITEM  ptr_ctrl_mgmt_list = {0};

/// �ɷ���Ŀ��ƹ���ռ�
static SDLIST_ITEM  free_ctrl_mgmt = {0};

/// �ɷ�����ܿض���״̬���ƿռ�
static SDLIST_ITEM  free_obj_state_ctrl = {0};
#pragma DATA_SEG __RPAGE_SEG DEFAULT


/**
  * @enum   FIND_TYPE
  * @brief  �ܿض��������̵Ĳ��ҽ��
  */
typedef enum 
{
    OBJECT_NULL = 0,										///< ���ƹ����û�иö���Ŀ���
    OBJECT_EXIST_PROCESS_NULL,								///< ���ƹ�����иö���Ŀ��ƣ���û�и����̵Ŀ���
    OBJECT_AND_PROCESS_EXIST								///< ���ƹ�����иö���Ŀ��ƣ�Ҳ�и����̵Ŀ���
}FIND_TYPE;

/**
  * @enum   RELEASE_CTRL_RESULT
  * @brief  �ͷſ��ƴ���Ľ��
  */
typedef enum
{
    RELEASE_VOID = 0,                                       ///< û���ҵ����ͷŵĽڵ�
    RELEASE_OBJ_CTRL,                                       ///< �ͷŵ������̶�ĳ������Ŀ���
    RELEASE_CTRL_MGMT                                       ///< �ͷ��˶����ϵĿ��ƹ���ڵ�
}RELEASE_CTRL_RESULT;


/***************************************************************
�ڲ���������
***************************************************************/



/***************************************************************
����ʵ��
***************************************************************/
void reset_obj_ctrl(ControlManagement *_FAR ptr_ctrl_mgmt, ObjectStateControl*_FAR ptr_obj_state_ctrl)
{
    if(ptr_ctrl_mgmt != NULL)
    {
        ptr_ctrl_mgmt->next = NULL;
        ptr_ctrl_mgmt->object_type = LCOT_VOID;
        ptr_ctrl_mgmt->object_id = 0;																							//��ʼ��ֵ��ȷ��
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
  * @brief	���������ʼ��
  *	@return	��
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
  * @brief	       �������ض���Ĵ���
  * @param[in]	   ptr_ctrl_mgmt	�����ܿض����ϵĿ��ƹ������ݽṹ
  *	@return	��
 */
static void ctrl_on_sigle_object(ControlManagement *_FAR ptr_ctrl_mgmt)
{
    INT16U ctrl_state = STATE_INVALID;		//����״̬��ʱ����
    ObjectStateControl  *_FAR ptr_ctrl = NULL;
    ObjectStateControl  *_FAR ptr_ctrl_real = NULL;

    ptr_ctrl = (ObjectStateControl *_FAR)ptr_ctrl_mgmt->ctrl_list.next;
    if(ptr_ctrl == NULL)
    {
        return;
    }
    //����������ȼ��Ŀ���
    while(NULL != ptr_ctrl)
    {
        if(NULL != ptr_ctrl->ptr_ctrl_process)			//��ʾ���̶Կ��ƶ����п���
        {
            switch(ptr_ctrl_mgmt->object_type)				//�жϿ��ƶ�������
            {
            case LCOT_RELAY:							//�̵���
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

            case LCOT_VOLTAGE:							//��ѹ������
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

    //ִ��������ȼ��Ŀ��ƣ���ռ����
    ptr_ctrl = (ObjectStateControl *_FAR)ptr_ctrl_mgmt->ctrl_list.next;
    while(ptr_ctrl != NULL)
    {
        //ִ�п�����ռ������
        if(ptr_ctrl->state_to_switch != ctrl_state)
        {
            //��ռ�ص�
            ptr_ctrl->func_ctrl_preempt(ptr_ctrl->ptr_ctrl_process);

            //�ͷŽڵ�
            list_release(&(ptr_ctrl_mgmt->ctrl_list), (SDLIST_ITEM *_FAR)ptr_ctrl, &free_obj_state_ctrl);
        }

        ptr_ctrl = (ObjectStateControl *_FAR)ptr_ctrl->next;
    }
    
    /*(void)dbgprintf("cur_state=%d,object_type=%d,object_id=%d\n",
            ptr_ctrl_mgmt->cur_state, 
            ptr_ctrl_mgmt->object_type,
            ptr_ctrl_mgmt->object_id);    */

    //���տ���״̬�뵱ǰ״̬��ͬ
    if(ptr_ctrl_mgmt->cur_state == ctrl_state)
    {
        return;
    }

    //�������տ���״̬
    ptr_ctrl_mgmt->cur_state = ctrl_state;

    //ִ�еײ�Ŀ��ƺ���
    (void)ptr_ctrl_real->func_state_ctrl(ptr_ctrl_mgmt->object_type, ptr_ctrl_mgmt->object_id, ptr_ctrl_mgmt->cur_state);
}
/**
  * @brief        ���ƹ�������
  *	@return         ��
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
        ctrl_on_sigle_object((ControlManagement *_FAR)object_list);    //��ѯ���п��ƶ���
    }
}

/**
  * @brief				���ҿ��ƹ����������Ƿ��Ѿ��жԸö���Ŀ���
  *	@param[in]			ptr_ctrl_process		���̵�ָ��
  *	@param[in]			object_type				�ܿض��������
  *	@param[in]			object_id				�ܿض����ID
  *	@param[out]			pp_ctrl_mgmt            ��������Ŀ��ƹ���
  *	@param[out]			pp_obj_state_ctrl       �������̶Ե�������Ŀ���
  *	@return				�ܿض��������̵Ĳ��ҽ��
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

                    return OBJECT_AND_PROCESS_EXIST; //����͹�������
                }

                ptr_obj_state_ctrl = (ObjectStateControl *_FAR)ptr_obj_state_ctrl->next; //��һ�����ƶ���
            }

            *pp_ctrl_mgmt = ptr_ctrl_mgmt;
            return OBJECT_EXIST_PROCESS_NULL; 
        }

        ptr_ctrl_mgmt = (ControlManagement *_FAR)ptr_ctrl_mgmt->next; //��һ�����ƹ���
    }

    return OBJECT_NULL;
}

/**
  * @brief				���뵥�����̶Ե����ܿض����״̬����
  *	@param[in]			ptr_ctrl_process		���̵�ָ��
  *	@param[in]			func_ctrl_preempt       ��ռ�ص�����
  *	@param[in]			func_state_ctrl			���ƺ���
  *	@param[in]			ctrl_prio				���Ƶ����ȼ�
  *	@param[in]			object_type				�ܿض��������
  *	@param[in]			object_id				�ܿض����ID
  *	@param[in]			state_to_switch			�ܿض���Ҫ�ﵽ��״̬
  *	@return	            ��
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
    
    //���ҿ��ƹ����������Ƿ��Ѿ��иö���Ŀ���
    find_result = find_object((void*_FAR)ptr_ctrl_process, object_type, object_id,
                              &ptr_ctrl_mgmt, &ptr_obj_state_ctrl);
    
    //����и����̶Ըö�����ƣ�ֻ����¿��Ƶ���ز���
    switch(find_result)
    {
    case OBJECT_AND_PROCESS_EXIST: //��������̿��ƶ�����
        break;

    case OBJECT_EXIST_PROCESS_NULL: //�������
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
  * @brief				�ͷŵ������̶Ե����ܿض����״̬����
  *	@param[in]			ptr_ctrl_process		���̵�ָ��
  *	@param[in]			ptr_ctrl_mgmt			�����ܿض���Ŀ��ƹ���ָ��
  *	@return
  * \li RELEASE_VOID        û���ҵ����ͷŵĽڵ�
  * \li RELEASE_OBJ_CTRL    �ͷŵ������̶�ĳ������Ŀ���
  * \li RELEASE_CTRL_MGMT   �ͷ��˶����ϵĿ��ƹ���ڵ�
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

    //���������ϵĿ�������
    while(NULL != ptr_obj_ctrl)
    {
        if( ptr_ctrl_process == ptr_obj_ctrl->ptr_ctrl_process)
        {
            uRet = RELEASE_OBJ_CTRL;

            //���̿��ƶ��󣺲��ҳɹ�
            ptr_obj_ctrl->ptr_ctrl_process = NULL;

            //�޸�����
            ptr_obj_ctrl_prev->next = ptr_obj_ctrl->next;

            //���տռ�
            ptr_obj_ctrl->next = free_obj_state_ctrl.next;
            free_obj_state_ctrl.next = (SDLIST_ITEM *_FAR)ptr_obj_ctrl;

            //������û�����̶�����п���
            if(NULL == ptr_ctrl_mgmt->ctrl_list.next)
            {
                uRet = RELEASE_CTRL_MGMT;

                //�޸�����
                ptr_ctrl_mgmt_prev->next = ptr_ctrl_mgmt->next;

                //���տռ�
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
  * @brief				�ͷŵ������̶Ե����ܿض����״̬����
  *	@param[in]			ptr_ctrl_process		���̵�ָ��
  *	@param[in]			object_type				�ܿض��������
  *	@param[in]			object_id				�ܿض����ID
  *	@return	��
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

    //���������ϵĿ��Ƽ���
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
  * @brief				�ͷŵ������̶������ܿض����״̬����
  *	@param[in]			ptr_ctrl_process		���̵�ָ��
  *	@return	��
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

    //���������ϵĿ��Ƽ���
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


