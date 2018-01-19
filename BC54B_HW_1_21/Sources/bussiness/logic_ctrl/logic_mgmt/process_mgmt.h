/**
  * @file			process_mgmt.h
  * @brief			逻辑控制流程处理：流程管理头文件
  *	@copyright		Ligoo Inc.
  *	@date			2017-03-03
  *	@author
  */
#ifndef	PROCESS_MGMT_H
#define	PROCESS_MGMT_H

#include "ctrl_mgmt.h"
#include "includes.h"

/***************************************************************
数据结构定义
***************************************************************/

/// 流程管理链表上的节点数目最大值(最大支持多少个流程)
#define MAIN_PROCESS_MAX_NUM                   10
#define SUB_PROCESS_MAX_NUM                    MAIN_PROCESS_MAX_NUM
#define PROCESS_LOGIC_CTRL_CONTEXT_MAX_NUM    (MAIN_PROCESS_MAX_NUM + SUB_PROCESS_MAX_NUM)
/// 继电器控制超时
#define PROCESS_END_DELAY       1000
///< 无效节点ID
#define NODE_ID_INVALID         0xFF            

/**
  * @enum   LogicControlNodeType
  * @brief  逻辑控制节点类型
  */
typedef enum
{
    LCNT_VOID = 0,			    ///< 空
    LCNT_END,               ///< 结束
    LCNT_CONDITION_SELECT,	///< 条件选择
    LCNT_STATE_CONTROL		  ///< 状态控制
}LogicControlNodeType;

/**
  * @enum   LogicControlPriority
  * @brief  控制优先级
  */
typedef enum
{
    LCP_VOID = 0,			///< 数值越小，优先级越高
    LCP_OPEN,			  	///< 断开继电器
    LCP_CLOSE         ///< 闭合继电器
}LogicControlPriority;

/**
  * @enum   StateCtrlExeResult
  * @brief  状态控制执行结果
  */
typedef enum
{
    SCER_SUCC = 0,		    	///< 执行成功
    SCER_FAIL,              ///< 执行失败
    SCER_WAITING,           ///< 执行后等待
    SCER_DELAY              ///< 执行后延时
}StateCtrlExeResult;

/// 条件选择函数
typedef  INT8U (*_FAR ConditionSelectFuncHandler)(void);             ///< 形式待确认

/// 消息回调函数
typedef  void (*_FAR MsgFuncHandler)(void);                          ///< 形式待确认

/// 延时回调函数
typedef  INT32U (*_FAR DelayFuncHandler)(void);

/// 状态查询回调函数
typedef  void (*_FAR StateQueryFuncHandler)(void);                   ///< 形式待确认


/**
  * @struct     ConditionSelection
  * @brief      条件选择
  */
typedef struct
{
    INT8U                           false_next_node_id; 	///< 当前节点为假，下一个要执行的节点编号
    ConditionSelectFuncHandler      func_cond_select;		///< 条件选择节点 函数
}ConditionSelection;

/**
  * @struct     StateControl
  * @brief      状态控制
  */
typedef struct
{
    LogicControlObjectType		object_type;			///< 被控对象类型
    INT8U                       object_id;              ///< 被控对象编号（被控类型为继电器时，被控对象编号指enum RelayControlType定义的继电器类型）

    /**
      * @brief 当前状态
      *
      * @par 状态含义：
      * - STATE_INVALID：无效
      * - object_type=LCOT_RELAY：
      *     -# STATE_OPEN：				断开
      *     -# STATE_CLOSE：				闭合
      *     -# STATE_FAULT_ADHESION：	粘连故障检测
      *     -# STATE_FAULT_OPEN：		开路故障检测
      * - object_type=LCOT_VOLTAGE：		电压值（单位：0.001V）
      * - object_type=LCOT_CURRENT：		电流值（单位：0.001A）
	  *	- STATE_RELEASE：状态释放
      */
    INT16U                      object_state_to_switch;

    INT8U                       ctrl_prio;              ///< 优先级，数值越小优先级越高，可相同

    //INT32U                      ctrl_delay;             ///< 延时（单位：ms）
    DelayFuncHandler         ctrl_delay;            ///< 延时函数

    INT16U                      time_check;             ///< 检测时间（单位：ms）
    INT16U                      time_confirm;           ///< 确认时间（单位：ms）

    MsgFuncHandler  			func_msg;               ///< 消息函数
}StateControl;


/**
  * @struct     LogicControlNodeContext
  * @brief      逻辑控制节点
  */
typedef	struct _LogicControlNodeContext
{
    const struct _LogicControlNodeContext    *_FAR next_ctrl_process;       ///< 下一个逻辑控制流程，用于启动子流程（非空时有效）
    INT8U                                   next_ctrl_process_node_count;   ///< 下一个逻辑控制流程中的节点数目（next_ctrl非空时有效）

    LogicControlNodeType                    node_type;                      ///< 逻辑控制节点类型
    INT8U                                   node_id;                        ///< 本节点编号
    INT8U                                   next_node_id;                   ///< 下一个要执行的节点编号(对条件选择节点，指条件为真的下一个执行节点编号)

    /// 条件选择
    ConditionSelection                      cond_selection;

    /// 状态控制
    StateControl                            state_ctrl;
 
}LogicControlNodeContext;

/**
  * @enum   LogicCtrlProcessStatus
  * @brief  控制流程执行状态
  */
typedef enum
{
    LCPS_End = 0,
    LCPS_Running,                               ///< 运行
    LCPS_Adhesion_Fault_Check_Waiting,          ///< 粘连故障检测等待
    LCPS_Open_Fault_Check_Waiting,              ///< 开路故障检测等待
    LCPS_Relay_Ctrl_Waiting,              ///< 开路故障检测等待
    LCPS_Subprocess_Waiting,                    ///< 子流程等待
    LCPS_Delay,                                 ///< 延时
    LCPS_Pending,                               ///< 挂起
    LCPS_End_Delay                              ///< 结束延时等待
}LogicCtrlProcessStatus;

/**
  * @enum   ProcessType
  * @brief  流程类型
  */
typedef enum
{
    LCNT_PROCESS_MAIN = 0,	  ///< 主流程
    LCNT_PROCESS_SUB,         ///< 子流程
}ProcessType;

 /**
  * @struct LogicCtrlProcessContext
  * @brief  逻辑控制执行管理
  */
typedef struct  _CtrlProcessMgmtContext
{
    //SDLIST_ITEM      *_FAR next;                           ///< 连接下一个流程指针

    LogicControlNodeContext     *_FAR ptr_process;           ///< 流程数组首地址
    INT8U                       process_node_num;            ///< 流程节点总数

    INT8U                       next_node_id;                ///< 下一步要执行节点的编号

    LogicCtrlProcessStatus      status;                      ///< 流程控制状态

    INT32U                      time_start;                  ///< 起始时刻（单位：tick）
    INT32U                      time_out;                    ///< 超时或延时时间（单位：tick）
    const char                  *_FAR name;                  ///< 当前流程名称
    INT8U                       process_id;
    ProcessType                 type;                        ///< 主流程子流程类型
    struct _CtrlProcessMgmtContext  *_FAR sub_process_mgmt;  ///< 子流程类型
    LogicControlNodeContext     *_FAR apply_ptr_process;     ///< 申请控制使用的流程数组首地址
}CtrlProcessMgmtContext;

/***************************************************************
宏定义
***************************************************************/
/**
  * 条件选择添加
  * @a THIS_ID          本节点编号
  * @a TRUE_NEXT_ID     条件为真需执行的节点编号
  * @a FALSE_NEXT_ID    条件为假需执行的节点编号
  * @a SELECT_COND      条件函数
  */
#define LOGIC_ADD_COND_SELECT_NODE(THIS_ID, TRUE_NEXT_ID, FALSE_NEXT_ID, SELECT_COND) {\
    NULL, 0, LCNT_CONDITION_SELECT, THIS_ID, TRUE_NEXT_ID, \
    {FALSE_NEXT_ID, SELECT_COND},\
    {LCOT_VOID, 0, STATE_INVALID, PRIO_INVALID, NULL, 0, 0, NULL}}

/**
  * 状态控制添加
  * @a THIS_ID          本节点编号
  * @a NEXT_ID          下一个要执行节点编号
  * @a OBJ_TYPE         被控对象类型
  * @a OBJ_ID           被控对象编号
  * @a STATE_SWITCH     要达到状态
  * @a PRIO             优先级
  * @a DLY_TIME         延时时间（单位：ms）
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
  * 控制释放添加
  * @a THIS_ID          本节点编号
  * @a NEXT_ID          下一个要执行节点编号
  * @a OBJ_TYPE         被控对象类型
  * @a OBJ_ID           被控对象编号
  */
#define LOGIC_ADD_CTRL_RELEASE_NODE(THIS_ID, NEXT_ID, OBJ_TYPE, OBJ_ID, STATE_SWITCH, PRIO) {\
    NULL, 0, LCNT_STATE_CONTROL, THIS_ID, NEXT_ID, \
    {NODE_ID_INVALID, NULL},\
    {OBJ_TYPE, OBJ_ID, STATE_INVALID, PRIO_INVALID, NULL, 0, 0, NULL} }

/**
  * 延时添加
  * @a THIS_ID          本节点编号
  * @a NEXT_ID          下一个要执行节点编号
  * @a DLY_TIME         延时时间（单位：ms）
  */
#define LOGIC_ADD_DELAY_NODE(THIS_ID, NEXT_ID, DLY_TIME) {\
    NULL, 0, LCNT_STATE_CONTROL, THIS_ID, NEXT_ID, \
    {NODE_ID_INVALID, NULL},\
    {LCOT_VOID, 0, STATE_INVALID, PRIO_INVALID, DLY_TIME, 0, 0, NULL}}

/**
  * 消息添加
  * @a THIS_ID          本节点编号
  * @a NEXT_ID          下一个要执行节点编号
  * @a FUNC_MSG         回调消息函数
  */
#define LOGIC_ADD_MSG_NODE(THIS_ID, NEXT_ID, FUNC_MSG) {\
    NULL, 0, LCNT_STATE_CONTROL, THIS_ID, NEXT_ID, \
    {NODE_ID_INVALID, NULL},\
    {LCOT_VOID, 0, STATE_INVALID, PRIO_INVALID, NULL, 0, 0, FUNC_MSG} }
    
/**
  * 启动添加
  * @a THIS_ID          本节点编号
  * @a NEXT_ID          下一个要执行节点编号
  * @a FUNC_MSG         回调消息函数
  */
#define LOGIC_ADD_LAUNCH_NODE(THIS_ID, NEXT_ID, PROCESS_NAME, NODE_COUNT_NUM) {\
    PROCESS_NAME, NODE_COUNT_NUM, LCNT_STATE_CONTROL, THIS_ID, NEXT_ID, \
    {NODE_ID_INVALID, NULL},\
    {LCOT_VOID, 0, STATE_INVALID, PRIO_INVALID, NULL, 0, 0, NULL} }

/**
  * 结束添加
  * @a THIS_ID          本节点编号
  */    
#define LOGIC_ADD_END_NODE(THIS_ID) {\
    NULL, 0, LCNT_END, THIS_ID, 1, \
    {0, NULL},\
    {0, 0, 0, 0, NULL, 0, 0, NULL} }


    
/// 流程创建时初始节点编号
#define     NODE_ID_INIT    1

/***************************************************************
接口
***************************************************************/
void set_process_active(LogicControlNodeContext *_FAR ptr_ctrl_process);
/**
  * @brief	启动逻辑控制流程
  *	@param[in]			ptr_logic_ctrl          控制流程节点数组首地址
  *	@param[in]			process_node_num        控制流程节点数组节点数目
  *	@return	无
 */
CtrlProcessMgmtContext  *_FAR  start_logic_ctrl_process(LogicControlNodeContext *_FAR ptr_ctrl_process, INT8U process_node_num,LogicCtrlProcessStatus init_status,ProcessType type);
CtrlProcessMgmtContext  *_FAR  start_sub_logic_ctrl_process(LogicControlNodeContext *_FAR ptr_ctrl_process, INT8U process_node_num,LogicCtrlProcessStatus init_status,ProcessType type,LogicControlNodeContext *_FAR apply_ptr_ctrl_process);
/**
  * @brief          逻辑控制流程运行
  *	@return         无
 */
void ctrl_process_mgmt_run(void);

/**
  * @brief	流程管理链表初始化
  *	@return	无
 */
void process_mgmt_init(void);

#endif
