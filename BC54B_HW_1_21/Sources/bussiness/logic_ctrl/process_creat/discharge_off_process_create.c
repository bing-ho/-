#include "bms_relay_control_impl.h"
#include "logic_ctrl_process_start.h"
#include "logic_ctrl_lib_func.h"
#include "discharge_on_process_create.h"

static INT8U discharger_hv_off_cond(void)
{
  	if(bms_relay_diagnose_get_fault_num(kRelayTypeDischarging)!=0  
    &&bms_relay_diagnose_get_fault_num(kRelayTypeDischarging)!=kRelayRunCondPrechargeFailure
    &&bms_relay_diagnose_get_fault_num(kRelayTypeDischarging)!=kRelayRunCondPrecharging
    &&(bcu_get_discharge_relay_state() == kRelayStatusOn || bcu_get_discharge_relay_state() == kRelayStatusPreCharging)) 
    {
  	    set_process_active(discharge_on_process);
        return 1; 
    }
    return 0;
}

static INT8U precharge_is_enabled(void)
{
    if(config_get(kRelayPreCharge))
        return 1;
    return 0;
}

static INT8U charger_relay_is_on(void)
{
    if(relay_control_is_on(kRelayTypeCharging) == TRUE)
        return 1;
    return 0;
}

static void set_discharge_off_state(void)
{
    
    bcu_set_discharge_relay_state(kRelayStatusOff);
}

const LogicControlNodeContext discharge_off_process[]=
{
/// 条件选择节点添加：第1个节点，条件选择节点，为真时执行节点2，为假时执行节点1，1-放电高压下电条件//discharger_hv_off_cond=[discharger_hv_off_cond]
LOGIC_ADD_COND_SELECT_NODE(KNodeID_1, KNodeID_3, KNodeID_1, discharger_hv_off_cond),

/// 延时节点添加：第2个节点，下一步执行节点3，延时时间2000,2-延时2S//2000=[2000],type=[delay]
//LOGIC_ADD_DELAY_NODE(KNodeID_2, KNodeID_3, 2000),

/// 条件选择节点添加：第3个节点，条件选择节点，为真时执行节点4，为假时执行节点5，3-预充是否使能//precharge_is_enabled=[precharge_is_enabled]
LOGIC_ADD_COND_SELECT_NODE(KNodeID_3, KNodeID_4, KNodeID_5, precharge_is_enabled),

/// 状态控制节点添加：第4个节点，下一步执行节点5，4-断开预充继电器//LCOT_RELAY,kRelayTypePreCharging,OFF,0,1000=[LCOT_RELAY,kRelayTypePreCharging,OFF,0,1000]
LOGIC_ADD_STATE_CTRL_NODE(KNodeID_4, KNodeID_5, LCOT_RELAY,kRelayTypePreCharging,OFF,0,relay_off_outtime),

/// 状态控制节点添加：第5个节点，下一步执行节点6，5-断开放电继电器//LCOT_RELAY,kRelayTypeDischarging,OFF,0,1000=[LCOT_RELAY,kRelayTypeDischarging,OFF,0,1000]
LOGIC_ADD_STATE_CTRL_NODE(KNodeID_5, KNodeID_6, LCOT_RELAY,kRelayTypeDischarging,OFF,0,relay_off_outtime),

/// 状态控制节点添加：第6个节点，下一步执行节点7，6-断开放电信号继电器//LCOT_RELAY,kRelayTypeDischargingSignal,OFF,0,1000=[LCOT_RELAY,kRelayTypeDischargingSignal,OFF,0,1000]
LOGIC_ADD_STATE_CTRL_NODE(KNodeID_6, KNodeID_7, LCOT_RELAY,kRelayTypeDischargingSignal,OFF,0,relay_off_outtime),

/// 条件选择节点添加：第7个节点，条件选择节点，为真时执行节点9，为假时执行节点8，7-充电继电器是否吸合//charger_relay_is_on=[charger_relay_is_on]
LOGIC_ADD_COND_SELECT_NODE(KNodeID_7, KNodeID_9, KNodeID_8, charger_relay_is_on),

/// 状态控制节点添加：第8个节点，下一步执行节点9，8-断开总负继电器//LCOT_RELAY,kRelayTypeNegative,OFF,0,1000=[LCOT_RELAY,kRelayTypeNegative,OFF,0,1000]
LOGIC_ADD_STATE_CTRL_NODE(KNodeID_8, KNodeID_9, LCOT_RELAY,kRelayTypeNegative,OFF,0,relay_off_outtime),

/// 结束流程添加：第9个节点
LOGIC_ADD_MSG_NODE(KNodeID_9, KNodeID_10, set_discharge_off_state),

LOGIC_ADD_END_NODE(KNodeID_10),
};


void start_discharge_off_process(void)
{
    start_logic_ctrl_process(discharge_off_process, sizeof(discharge_off_process)/sizeof(LogicControlNodeContext),LCPS_Running,LCNT_PROCESS_MAIN);
}

