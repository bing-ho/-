#include "bms_relay_control_impl.h"
#include "logic_ctrl_lib_func.h"
#include "logic_ctrl_process_start.h"

#pragma MESSAGE DISABLE C2705 // Possible loss of data
#pragma MESSAGE DISABLE C12056 // SP debug info incorrect because of optimization or inline assembler

#pragma DATA_SEG __RPAGE_SEG PAGED_RAM
INT8U g_ems_precharge_cnt=0;
#pragma DATA_SEG DEFAULT

INT8U bms_get_precharge_cnt(void) 
{
  return  g_ems_precharge_cnt;
}

static void set_prechar_state(void)
{
    bcu_set_precharge_with_tv_state(kPrechargeStatePrecharging);
}

static void set_prechar_succ_flag(void)
{
    bcu_set_precharge_with_tv_state(kPrechargeStateFinish); //预充成功
}

static INT32U bms_precharge_timeout(void)
{
    return 3000; //ms/bit 预充电超时时间
}

/**
  * @brief  预充流程结构体数组
  * \note
  * \li 为节省内存，数组定义为const类型，放在ROM中；
  * \li 流程初始执行的节点编号应为NODE_ID_INIT：宏定义为1，下层按此约定设计；
  * \li 被控类型为继电器时，被控对象编号指enum RelayControlType定义的继电器类型；
  */
const LogicControlNodeContext precharge_process[]=
{
/// 条件选择节点添加：第1个节点，条件选择节点，为真时执行节点2，为假时执行节点1，1-条件为真// return_ture=[return_ture]
LOGIC_ADD_MSG_NODE(KNodeID_1, KNodeID_2, set_prechar_state),

/// 状态控制节点添加：第5个节点，下一步执行节点6，5-闭合预充继电器//[LCOT_RELAY, kRelayTypePreCharging, ON, 0, 0=[LCOT_RELAY, kRelayTypePreCharging, ON, 0, 0]
LOGIC_ADD_STATE_CTRL_NODE(KNodeID_2, KNodeID_3, LCOT_RELAY, kRelayTypePreCharging, ON, 0, relay_on_outtime),

LOGIC_ADD_DELAY_NODE(KNodeID_3, KNodeID_4, bms_precharge_timeout),

/// 消息节点添加：第8个节点，下一步执行节点12，8-置预充成功标志//set_prechar_flag=[set_prechar_flag],type=[msg]
LOGIC_ADD_MSG_NODE(KNodeID_4, KNodeID_5, set_prechar_succ_flag),

/// 结束流程添加：第11个节点
LOGIC_ADD_END_NODE(KNodeID_5),
};

