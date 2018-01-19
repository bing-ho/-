#include "charge_off_process_create.h"
#include "bms_relay_control_impl.h"
 #include "logic_ctrl_process_start.h"
#include "logic_ctrl_lib_func.h"
#include "bms_ems_impl.h"
#include "bms_charger_common.h"
#include "bms_diagnosis_adhesion.h"

static INT8U charger_hv_on_cond(void)
{
  	if(bms_relay_diagnose_get_fault_num(kRelayTypeCharging) == 0
     &&charger_is_connected() == TRUE 
     &&charger_is_communication() == TRUE 
     &&charger_charging_is_enable() == TRUE
     &&bcu_get_discharge_relay_state() == kRelayStatusOff
#if BMS_SUPPORT_NEGATIVE_ADHESION      
     &&bms_get_relay_state(kRelayTypeNegative) == kRelayNormal
#endif
     )
  	{
  	    set_process_active(charge_off_process);
        return 1;
  	}
    return 0;
}


static void set_charge_current(void)
{
    /*INT16U current = 0,charge_current=0;
    current = ems_charger_continue_current_update();
    charger_get_current(&charge_current);
    if(current != charge_current)
        charger_set_current(current); */
}

static void set_charge_voltage(void)
{
    //charger_set_voltage(config_get(kChargerVoltIndex));
}


static void set_charge_on_state(void)
{
    bcu_set_charge_relay_state(kRelayStatusOn);
}

const LogicControlNodeContext charge_on_process[]=
{
/// 条件选择节点添加：第1个节点，条件选择节点，为真时执行节点2，为假时执行节点1，1-充电高压上电条件//charger_hv_on_cond=[charger_hv_on_cond]
LOGIC_ADD_COND_SELECT_NODE(KNodeID_1, KNodeID_2, KNodeID_1, charger_hv_on_cond),

/// 消息节点添加：第2个节点，下一步执行节点3，2-设置充电闭合状态//set_charge_on_state=[set_charge_on_state],type=[msg]
LOGIC_ADD_MSG_NODE(KNodeID_2, KNodeID_3, set_charge_on_state),

/// 状态控制节点添加：第6个节点，下一步执行节点6，6-吸合总负继电器//LCOT_RELAY,kRelayTypeNegative,ON,0,1000=[LCOT_RELAY,kRelayTypeNegative,ON,0,1000]
LOGIC_ADD_STATE_CTRL_NODE(KNodeID_3, KNodeID_4, LCOT_RELAY,kRelayTypeNegative,ON,0,relay_on_outtime),

/// 状态控制节点添加：第7个节点，下一步执行节点8，7-吸合充电继电器//LCOT_RELAY,kRelayTypeCharging,ON,0,1000=[LCOT_RELAY,kRelayTypeCharging,ON,0,1000]
LOGIC_ADD_STATE_CTRL_NODE(KNodeID_4, KNodeID_5, LCOT_RELAY,kRelayTypeCharging,ON,0,relay_on_outtime),

/// 状态控制节点添加：第30个节点，下一步执行节点31，30-吸合充电信号继电器//LCOT_RELAY,kRelayTypeChargingSignal,ON,0,1000=[LCOT_RELAY,kRelayTypeChargingSignal,ON,0,1000]
LOGIC_ADD_STATE_CTRL_NODE(KNodeID_5, KNodeID_6, LCOT_RELAY,kRelayTypeChargingSignal,ON,0,relay_on_outtime),

/// 消息节点添加：第31个节点，下一步执行节点32，31-设置充电电压//set_charge_voltage=[set_charge_voltage],type=[msg]
LOGIC_ADD_MSG_NODE(KNodeID_6, KNodeID_7, set_charge_voltage),

/// 消息节点添加：第32个节点，下一步执行节点32，32-设置充电电流//set_charge_current=[set_charge_current],type=[msg]
LOGIC_ADD_MSG_NODE(KNodeID_7, KNodeID_7, set_charge_current),

};

void start_charge_on_process(void)
{
    start_logic_ctrl_process(charge_on_process, sizeof(charge_on_process)/sizeof(LogicControlNodeContext),LCPS_Running,LCNT_PROCESS_MAIN);
}

