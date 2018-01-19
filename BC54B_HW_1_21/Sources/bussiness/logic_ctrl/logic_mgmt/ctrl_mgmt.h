/**
  * @file			  ctrl_mgmt.h   
  * @brief			  逻辑控制流程处理：控制管理头文件
  *	@copyright	      Ligoo Inc.
  *	@date			  2017-03-03
  *	@author
  */
#ifndef	CTRL_MGMT_H
#define	CTRL_MGMT_H

#include "includes.h"
#include "ctrl_exec.h"
#include "single_direct_list.h"

/***************************************************************
数据结构定义
***************************************************************/

/// 控制抢占处理函数
typedef	void	(*_FAR PreemptFuncHandler)(void *_FAR pdata);

/// 基本控制函数
typedef	INT16U	(*_FAR ObjectStateCtrlFuncHandler)(LogicControlObjectType object_type, INT8U object_id, INT16U state);

#define	MAX_CTRL_ON_OBJ         10
#define	MAX_OBJ_COUNT           11

#define STATE_OPEN              0               ///< 继电器断开状态
#define STATE_CLOSE             1               ///< 继电器闭合状态
#define STATE_INVALID           0xFFFF          ///< 无效状态
#define STATE_RELEASE           0xFFFE          ///< 状态释放
#define STATE_FAULT_ADHESION    0x11            ///< 继电器粘连故障
#define STATE_FAULT_OPEN        0x22            ///< 继电器开路故障

#define PRIO_INVALID            0xFF            ///< 无效优先级
#define CONTROL_LOCK            1               ///< 控制锁定
#define CONTROL_UNLOCK          0               ///< 控制释放


/**
  * @struct     ObjectStateControl
  * @brief      对单个被控对象的状态控制，对应struct StateControl
  */
typedef struct
{
    SDLIST_ITEM                     *_FAR next;                      ///< 链表数据结构

    void                            *_FAR ptr_ctrl_process;          ///< 状态控制发起者

    PreemptFuncHandler              func_ctrl_preempt;          ///< 控制抢占处理函数

    ObjectStateCtrlFuncHandler  	func_state_ctrl;            ///< 基本控制函数

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
    INT16U                          state_to_switch;

    INT8U                           ctrl_prio;                  ///< 优先级，数值越小优先级越高，可相同
}ObjectStateControl;

/**
  * @struct     ControlManagement
  * @brief      控制管理
  */
typedef struct
{
    SDLIST_ITEM               *_FAR next;                      ///< 链表数据结构

    //OS_EVENT                *pSem;                      ///< 读写控制

    LogicControlObjectType  object_type;                ///< 被控对象类型：消息、延时等使用LCOT_VOID
    INT8U                   object_id;                  ///< 被控对象编号

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
    INT16U                  cur_state;

    SDLIST_ITEM             ctrl_list;             ///< 来自不同控制流程的状态控制
}ControlManagement;


/***************************************************************
接口
***************************************************************/
/**
  * @brief				申请单个流程对单个受控对象的状态控制
  *	@param[in]			ptr_ctrl_process		流程的指针
  *	@param[in]			func_ctrl_preempt       抢占回调函数
  *	@param[in]			func_state_ctrl			控制函数
  *	@param[in]			ctrl_prio				控制的优先级
  *	@param[in]			object_type				受控对象的类型
  *	@param[in]			object_id				受控对象的ID
  *	@param[in]			state_to_switch			受控对象要达到的状态
  *	@return	无
 */
INT8U apply_state_ctrl(void *_FAR ptr_ctrl_process,
                      PreemptFuncHandler func_ctrl_preempt,
                      ObjectStateCtrlFuncHandler func_state_ctrl,
                      INT8U ctrl_prio,
                      LogicControlObjectType object_type,
                      INT8U object_id,
                      INT16U state_to_switch);

/**
  * @brief				释放单个流程对单个受控对象的状态控制
  *	@param[in]			ptr_ctrl_process		流程的指针
  *	@param[in]			object_type				受控对象的类型
  *	@param[in]			object_id				受控对象的ID
  *	@return	无
 */
void release_state_ctrl(void *_FAR ptr_ctrl_process,
                        LogicControlObjectType object_type,
                        INT8U object_id);

/**
  * @brief				释放单个流程对所有受控对象的状态控制
  *	@param[in]			ptr_ctrl_process		流程的指针
  *	@return	无
 */
void release_state_ctrl_all(void *_FAR ptr_ctrl_process);

/**
  * @brief        控制管理任务
  *	@return         无
 */
void basic_logic_control_run(void);

/**
  * @brief	控制链表初始化
  *	@return	无
 */
void ctrl_mgmt_init(void);

#endif
