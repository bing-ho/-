#include "bms_relay_control_impl.h"
#include "logic_ctrl_lib_func.h"
#include "logic_ctrl_process_start.h"

#pragma MESSAGE DISABLE C2705



static INT8U heating_is_allowed_on_cond(void)
{
  	if(TEMP_4_DISPLAY(bcu_get_low_temperature())< config_get(kHeatOnTemperature)
  	&&bmu_is_valid_temperature(g_bms_info.low_temperature)
  	&&relay_control_is_on(kRelayTypeHeating)==0)
    {
        return 1;
    }
    return 0;
}

static INT32U heat_relay_on_delay(void)
{
    return 5000;
}

const LogicControlNodeContext charge_heat_on_process[]=
{
/// 条件选择节点添加：第1个节点，条件选择节点，为真时执行节点2，为假时执行节点1，1-低温充电加热上电条件//charger_hv_on_cond=[charger_hv_on_cond]
LOGIC_ADD_COND_SELECT_NODE(KNodeID_1, KNodeID_2, KNodeID_1, heating_is_allowed_on_cond),

/// 条件选择节点添加：第2个节点，条件选择节点，为真时执行节点3，为假时执行节点5，2-慢充是否连接//bms_slow_charger_is_connected=[bms_slow_charger_is_connected]
LOGIC_ADD_DELAY_NODE(KNodeID_2, KNodeID_3, heat_relay_on_delay),

/// 状态控制节点添加：第3个节点，下一步执行节点4，3-吸合2号继电器//LCOT_RELAY，1，ON，0,1000=[LCOT_RELAY，1，ON，0,relay_on_outtime]
LOGIC_ADD_STATE_CTRL_NODE(KNodeID_3, KNodeID_1, LCOT_RELAY,kRelayTypeHeating,ON,0,relay_on_outtime),
};

void start_heat_on_process(void)
{
    start_logic_ctrl_process(charge_heat_on_process, sizeof(charge_heat_on_process)/sizeof(LogicControlNodeContext),LCPS_Running,LCNT_PROCESS_MAIN);
}

