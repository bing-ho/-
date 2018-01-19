/**
  * @file			process_mgmt.h
  * @brief			�߼��������̴������̹���ͷ�ļ�
  *	@copyright		Ligoo Inc.
  *	@date			2017-03-03
  *	@author
  */
#ifndef	PROCESS_MGMT_H
#define	PROCESS_MGMT_H

#include "ctrl_mgmt.h"
#include "includes.h"

/***************************************************************
���ݽṹ����
***************************************************************/

/// ���̹��������ϵĽڵ���Ŀ���ֵ(���֧�ֶ��ٸ�����)
#define MAIN_PROCESS_MAX_NUM                   10
#define SUB_PROCESS_MAX_NUM                    MAIN_PROCESS_MAX_NUM
#define PROCESS_LOGIC_CTRL_CONTEXT_MAX_NUM    (MAIN_PROCESS_MAX_NUM + SUB_PROCESS_MAX_NUM)
/// �̵������Ƴ�ʱ
#define PROCESS_END_DELAY       1000
///< ��Ч�ڵ�ID
#define NODE_ID_INVALID         0xFF            

/**
  * @enum   LogicControlNodeType
  * @brief  �߼����ƽڵ�����
  */
typedef enum
{
    LCNT_VOID = 0,			    ///< ��
    LCNT_END,               ///< ����
    LCNT_CONDITION_SELECT,	///< ����ѡ��
    LCNT_STATE_CONTROL		  ///< ״̬����
}LogicControlNodeType;

/**
  * @enum   LogicControlPriority
  * @brief  �������ȼ�
  */
typedef enum
{
    LCP_VOID = 0,			///< ��ֵԽС�����ȼ�Խ��
    LCP_OPEN,			  	///< �Ͽ��̵���
    LCP_CLOSE         ///< �պϼ̵���
}LogicControlPriority;

/**
  * @enum   StateCtrlExeResult
  * @brief  ״̬����ִ�н��
  */
typedef enum
{
    SCER_SUCC = 0,		    	///< ִ�гɹ�
    SCER_FAIL,              ///< ִ��ʧ��
    SCER_WAITING,           ///< ִ�к�ȴ�
    SCER_DELAY              ///< ִ�к���ʱ
}StateCtrlExeResult;

/// ����ѡ����
typedef  INT8U (*_FAR ConditionSelectFuncHandler)(void);             ///< ��ʽ��ȷ��

/// ��Ϣ�ص�����
typedef  void (*_FAR MsgFuncHandler)(void);                          ///< ��ʽ��ȷ��

/// ��ʱ�ص�����
typedef  INT32U (*_FAR DelayFuncHandler)(void);

/// ״̬��ѯ�ص�����
typedef  void (*_FAR StateQueryFuncHandler)(void);                   ///< ��ʽ��ȷ��


/**
  * @struct     ConditionSelection
  * @brief      ����ѡ��
  */
typedef struct
{
    INT8U                           false_next_node_id; 	///< ��ǰ�ڵ�Ϊ�٣���һ��Ҫִ�еĽڵ���
    ConditionSelectFuncHandler      func_cond_select;		///< ����ѡ��ڵ� ����
}ConditionSelection;

/**
  * @struct     StateControl
  * @brief      ״̬����
  */
typedef struct
{
    LogicControlObjectType		object_type;			///< ���ض�������
    INT8U                       object_id;              ///< ���ض����ţ���������Ϊ�̵���ʱ�����ض�����ָenum RelayControlType����ļ̵������ͣ�

    /**
      * @brief ��ǰ״̬
      *
      * @par ״̬���壺
      * - STATE_INVALID����Ч
      * - object_type=LCOT_RELAY��
      *     -# STATE_OPEN��				�Ͽ�
      *     -# STATE_CLOSE��				�պ�
      *     -# STATE_FAULT_ADHESION��	ճ�����ϼ��
      *     -# STATE_FAULT_OPEN��		��·���ϼ��
      * - object_type=LCOT_VOLTAGE��		��ѹֵ����λ��0.001V��
      * - object_type=LCOT_CURRENT��		����ֵ����λ��0.001A��
	  *	- STATE_RELEASE��״̬�ͷ�
      */
    INT16U                      object_state_to_switch;

    INT8U                       ctrl_prio;              ///< ���ȼ�����ֵԽС���ȼ�Խ�ߣ�����ͬ

    //INT32U                      ctrl_delay;             ///< ��ʱ����λ��ms��
    DelayFuncHandler         ctrl_delay;            ///< ��ʱ����

    INT16U                      time_check;             ///< ���ʱ�䣨��λ��ms��
    INT16U                      time_confirm;           ///< ȷ��ʱ�䣨��λ��ms��

    MsgFuncHandler  			func_msg;               ///< ��Ϣ����
}StateControl;


/**
  * @struct     LogicControlNodeContext
  * @brief      �߼����ƽڵ�
  */
typedef	struct _LogicControlNodeContext
{
    const struct _LogicControlNodeContext    *_FAR next_ctrl_process;       ///< ��һ���߼��������̣��������������̣��ǿ�ʱ��Ч��
    INT8U                                   next_ctrl_process_node_count;   ///< ��һ���߼����������еĽڵ���Ŀ��next_ctrl�ǿ�ʱ��Ч��

    LogicControlNodeType                    node_type;                      ///< �߼����ƽڵ�����
    INT8U                                   node_id;                        ///< ���ڵ���
    INT8U                                   next_node_id;                   ///< ��һ��Ҫִ�еĽڵ���(������ѡ��ڵ㣬ָ����Ϊ�����һ��ִ�нڵ���)

    /// ����ѡ��
    ConditionSelection                      cond_selection;

    /// ״̬����
    StateControl                            state_ctrl;
 
}LogicControlNodeContext;

/**
  * @enum   LogicCtrlProcessStatus
  * @brief  ��������ִ��״̬
  */
typedef enum
{
    LCPS_End = 0,
    LCPS_Running,                               ///< ����
    LCPS_Adhesion_Fault_Check_Waiting,          ///< ճ�����ϼ��ȴ�
    LCPS_Open_Fault_Check_Waiting,              ///< ��·���ϼ��ȴ�
    LCPS_Relay_Ctrl_Waiting,              ///< ��·���ϼ��ȴ�
    LCPS_Subprocess_Waiting,                    ///< �����̵ȴ�
    LCPS_Delay,                                 ///< ��ʱ
    LCPS_Pending,                               ///< ����
    LCPS_End_Delay                              ///< ������ʱ�ȴ�
}LogicCtrlProcessStatus;

/**
  * @enum   ProcessType
  * @brief  ��������
  */
typedef enum
{
    LCNT_PROCESS_MAIN = 0,	  ///< ������
    LCNT_PROCESS_SUB,         ///< ������
}ProcessType;

 /**
  * @struct LogicCtrlProcessContext
  * @brief  �߼�����ִ�й���
  */
typedef struct  _CtrlProcessMgmtContext
{
    //SDLIST_ITEM      *_FAR next;                           ///< ������һ������ָ��

    LogicControlNodeContext     *_FAR ptr_process;           ///< ���������׵�ַ
    INT8U                       process_node_num;            ///< ���̽ڵ�����

    INT8U                       next_node_id;                ///< ��һ��Ҫִ�нڵ�ı��

    LogicCtrlProcessStatus      status;                      ///< ���̿���״̬

    INT32U                      time_start;                  ///< ��ʼʱ�̣���λ��tick��
    INT32U                      time_out;                    ///< ��ʱ����ʱʱ�䣨��λ��tick��
    const char                  *_FAR name;                  ///< ��ǰ��������
    INT8U                       process_id;
    ProcessType                 type;                        ///< ����������������
    struct _CtrlProcessMgmtContext  *_FAR sub_process_mgmt;  ///< ����������
    LogicControlNodeContext     *_FAR apply_ptr_process;     ///< �������ʹ�õ����������׵�ַ
}CtrlProcessMgmtContext;

/***************************************************************
�궨��
***************************************************************/
/**
  * ����ѡ�����
  * @a THIS_ID          ���ڵ���
  * @a TRUE_NEXT_ID     ����Ϊ����ִ�еĽڵ���
  * @a FALSE_NEXT_ID    ����Ϊ����ִ�еĽڵ���
  * @a SELECT_COND      ��������
  */
#define LOGIC_ADD_COND_SELECT_NODE(THIS_ID, TRUE_NEXT_ID, FALSE_NEXT_ID, SELECT_COND) {\
    NULL, 0, LCNT_CONDITION_SELECT, THIS_ID, TRUE_NEXT_ID, \
    {FALSE_NEXT_ID, SELECT_COND},\
    {LCOT_VOID, 0, STATE_INVALID, PRIO_INVALID, NULL, 0, 0, NULL}}

/**
  * ״̬�������
  * @a THIS_ID          ���ڵ���
  * @a NEXT_ID          ��һ��Ҫִ�нڵ���
  * @a OBJ_TYPE         ���ض�������
  * @a OBJ_ID           ���ض�����
  * @a STATE_SWITCH     Ҫ�ﵽ״̬
  * @a PRIO             ���ȼ�
  * @a DLY_TIME         ��ʱʱ�䣨��λ��ms��
  */
#define LOGIC_ADD_STATE_CTRL_NODE(THIS_ID, NEXT_ID, OBJ_TYPE, OBJ_ID, STATE_SWITCH, PRIO, DLY_TIME) {\
    NULL, 0, LCNT_STATE_CONTROL, THIS_ID, NEXT_ID, \
    {NODE_ID_INVALID, NULL},\
    {OBJ_TYPE, OBJ_ID, STATE_SWITCH, PRIO, DLY_TIME, 0, 0, NULL}}

/**
  * ????????
  * @a THIS_ID          ?????
  * @a TRUE_NEXT_ID     ????????????
  * @a FALSE_NEXT_ID    ????????????
  * @a SELECT_COND      ????
  */
#define LOGIC_ADD_COND_DELAY_SELECT_NODE(THIS_ID, TRUE_NEXT_ID, FALSE_NEXT_ID, SELECT_COND, DLY_TIME) {\
    NULL, 0, LCNT_CONDITION_SELECT, THIS_ID, TRUE_NEXT_ID, \
    {FALSE_NEXT_ID, SELECT_COND},\
    {LCOT_VOID, 0, STATE_INVALID, PRIO_INVALID, DLY_TIME, 0, 0, NULL}} 
/**
  * �����ͷ����
  * @a THIS_ID          ���ڵ���
  * @a NEXT_ID          ��һ��Ҫִ�нڵ���
  * @a OBJ_TYPE         ���ض�������
  * @a OBJ_ID           ���ض�����
  */
#define LOGIC_ADD_CTRL_RELEASE_NODE(THIS_ID, NEXT_ID, OBJ_TYPE, OBJ_ID, STATE_SWITCH, PRIO) {\
    NULL, 0, LCNT_STATE_CONTROL, THIS_ID, NEXT_ID, \
    {NODE_ID_INVALID, NULL},\
    {OBJ_TYPE, OBJ_ID, STATE_INVALID, PRIO_INVALID, NULL, 0, 0, NULL} }

/**
  * ��ʱ���
  * @a THIS_ID          ���ڵ���
  * @a NEXT_ID          ��һ��Ҫִ�нڵ���
  * @a DLY_TIME         ��ʱʱ�䣨��λ��ms��
  */
#define LOGIC_ADD_DELAY_NODE(THIS_ID, NEXT_ID, DLY_TIME) {\
    NULL, 0, LCNT_STATE_CONTROL, THIS_ID, NEXT_ID, \
    {NODE_ID_INVALID, NULL},\
    {LCOT_VOID, 0, STATE_INVALID, PRIO_INVALID, DLY_TIME, 0, 0, NULL}}

/**
  * ��Ϣ���
  * @a THIS_ID          ���ڵ���
  * @a NEXT_ID          ��һ��Ҫִ�нڵ���
  * @a FUNC_MSG         �ص���Ϣ����
  */
#define LOGIC_ADD_MSG_NODE(THIS_ID, NEXT_ID, FUNC_MSG) {\
    NULL, 0, LCNT_STATE_CONTROL, THIS_ID, NEXT_ID, \
    {NODE_ID_INVALID, NULL},\
    {LCOT_VOID, 0, STATE_INVALID, PRIO_INVALID, NULL, 0, 0, FUNC_MSG} }
    
/**
  * �������
  * @a THIS_ID          ���ڵ���
  * @a NEXT_ID          ��һ��Ҫִ�нڵ���
  * @a FUNC_MSG         �ص���Ϣ����
  */
#define LOGIC_ADD_LAUNCH_NODE(THIS_ID, NEXT_ID, PROCESS_NAME, NODE_COUNT_NUM) {\
    PROCESS_NAME, NODE_COUNT_NUM, LCNT_STATE_CONTROL, THIS_ID, NEXT_ID, \
    {NODE_ID_INVALID, NULL},\
    {LCOT_VOID, 0, STATE_INVALID, PRIO_INVALID, NULL, 0, 0, NULL} }

/**
  * �������
  * @a THIS_ID          ���ڵ���
  */    
#define LOGIC_ADD_END_NODE(THIS_ID) {\
    NULL, 0, LCNT_END, THIS_ID, 1, \
    {0, NULL},\
    {0, 0, 0, 0, NULL, 0, 0, NULL} }


    
/// ���̴���ʱ��ʼ�ڵ���
#define     NODE_ID_INIT    1

/***************************************************************
�ӿ�
***************************************************************/
void set_process_active(LogicControlNodeContext *_FAR ptr_ctrl_process);
/**
  * @brief	�����߼���������
  *	@param[in]			ptr_logic_ctrl          �������̽ڵ������׵�ַ
  *	@param[in]			process_node_num        �������̽ڵ�����ڵ���Ŀ
  *	@return	��
 */
CtrlProcessMgmtContext  *_FAR  start_logic_ctrl_process(LogicControlNodeContext *_FAR ptr_ctrl_process, INT8U process_node_num,LogicCtrlProcessStatus init_status,ProcessType type);
CtrlProcessMgmtContext  *_FAR  start_sub_logic_ctrl_process(LogicControlNodeContext *_FAR ptr_ctrl_process, INT8U process_node_num,LogicCtrlProcessStatus init_status,ProcessType type,LogicControlNodeContext *_FAR apply_ptr_ctrl_process);
/**
  * @brief          �߼�������������
  *	@return         ��
 */
void ctrl_process_mgmt_run(void);

/**
  * @brief	���̹��������ʼ��
  *	@return	��
 */
void process_mgmt_init(void);

#endif
