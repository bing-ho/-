#include "bms_relay_control_impl.h"
 #include "logic_ctrl_process_start.h"
#include "logic_ctrl_lib_func.h"
#include "precharge_process_create.h"
#include "discharge_off_process_create.h"
#include "bms_diagnosis_adhesion.h"
static INT8U discharger_hv_on_cond(void)
{
  	if((bms_relay_diagnose_get_fault_num(kRelayTypeDischarging) == 0
  	    ||bms_relay_diagnose_get_fault_num(kRelayTypeDischarging)==kRelayRunCondPrecharging 
  	    ||bms_relay_diagnose_get_fault_num(kRelayTypeDischarging)==kRelayRunCondPrechargeFailure)
  	 &&bcu_get_charge_relay_state() == kRelayStatusOff
#if BMS_SUPPORT_NEGATIVE_ADHESION 
  	 &&bms_get_relay_state(kRelayTypeNegative) == kRelayNormal
#endif
  	 )
    {  
        set_process_active(discharge_off_process);
        return 1;
    }

    return 0;
}


static INT8U precharge_is_ok(void)
{
	  if(bcu_get_precharge_with_tv_state() == kPrechargeStateFinish )
        return 1;
	  return 0;
}

static INT8U precharge_is_enabled(void)
{
    if(config_get(kRelayPreCharge))
        return 1;
    return 0;
}

static void set_precharge_on_state(void)
{
    bcu_set_discharge_relay_state(kRelayStatusPreCharging);
}

static void set_discharge_on_state(void)
{
    bcu_set_discharge_relay_state(kRelayStatusOn);
}

static void set_precharge_off_state(void)
{
    bcu_set_discharge_relay_state(kRelayStatusOff);
}

static INT32U precharge_off_delay(void)
{
    return 200;
}

const LogicControlNodeContext discharge_on_process[]=
{
/// 条件选择节点添加：第1个节点，条件选择节点，为真时执行节点2，为假时执行节点1，1-放电高压上电条件//discharger_hv_on_cond=[discharger_hv_on_cond]
LOGIC_ADD_COND_SELECT_NODE(KNodeID_1, KNodeID_2, KNodeID_1, discharger_hv_on_cond),

/// 消息节点添加：第18个节点，下一步执行节点2，18-设置放电闭合状态//set_discharge_on_state=[set_discharge_on_state],type=[msg]
LOGIC_ADD_MSG_NODE(KNodeID_2, KNodeID_3, set_discharge_on_state),

/// 状态控制节点添加：第5个节点，下一步执行节点6，5-吸合总负继电器//LCOT_RELAY,kRelayTypeNegative,ON,0,1000=[LCOT_RELAY,kRelayTypeNegative,ON,0,1000]
LOGIC_ADD_STATE_CTRL_NODE(KNodeID_3, KNodeID_4, LCOT_RELAY,kRelayTypeNegative,ON,0,relay_on_outtime),

/// 条件选择节点添加：第6个节点，条件选择节点，为真时执行节点7，为假时执行节点10，6-预充是否使能//precharge_is_enabled=[precharge_is_enabled]
LOGIC_ADD_COND_SELECT_NODE(KNodeID_4, KNodeID_5, KNodeID_9, precharge_is_enabled),

/// 消息节点添加：第7个节点，下一步执行节点8，7-设置预充状态//set_precharge_on_state=[set_precharge_on_state],type=[msg]
LOGIC_ADD_MSG_NODE(KNodeID_5, KNodeID_6, set_precharge_on_state),
           
/// 子流程添加：第8个节点，下一步执行节点9，8-启动预充流程 //precharge_process=[precharge_process],type=[add]
LOGIC_ADD_LAUNCH_NODE(KNodeID_6, KNodeID_7, precharge_process,11),

/// 条件选择节点添加：第9个节点，条件选择节点，为真时执行节点10，为假时执行节点15，9-预充是否成功//precharge_is_ok=[precharge_is_ok]
LOGIC_ADD_COND_SELECT_NODE(KNodeID_7, KNodeID_8, KNodeID_13, precharge_is_ok),

/// 消息节点添加：第10个节点，下一步执行节点11，10-设置放电闭合状态//set_discharge_on_state=[set_discharge_on_state],type=[msg]
LOGIC_ADD_MSG_NODE(KNodeID_8, KNodeID_9, set_discharge_on_state),

/// 状态控制节点添加：第11个节点，下一步执行节点12，11-吸合放电继电器//LCOT_RELAY,kRelayTypeDischarging,ON,0,1000=[LCOT_RELAY,kRelayTypeDischarging,ON,0,1000]
LOGIC_ADD_STATE_CTRL_NODE(KNodeID_9, KNodeID_10, LCOT_RELAY,kRelayTypeDischarging,ON,0,relay_on_outtime),

/// 状态控制节点添加：第12个节点，下一步执行节点13，12-吸合放电信号继电器//LCOT_RELAY,kRelayTypeDischargingSignal,ON,0,1000=[LCOT_RELAY,kRelayTypeDischargingSignal,ON,0,1000]
LOGIC_ADD_STATE_CTRL_NODE(KNodeID_10, KNodeID_11, LCOT_RELAY,kRelayTypeDischargingSignal,ON,0,relay_on_outtime),

/// 延时节点添加：第13个节点，下一步执行节点14，延时时间200,13-延时2S//2000=[2000],type=[delay]
LOGIC_ADD_DELAY_NODE(KNodeID_11, KNodeID_12, precharge_off_delay),

/// 状态控制节点添加：第14个节点，下一步执行节点17，14-断开预充继电器//LCOT_RELAY,kRelayTypePreCharging,OFF,0,1000=[LCOT_RELAY,kRelayTypePreCharging,OFF,0,1000]
LOGIC_ADD_STATE_CTRL_NODE(KNodeID_12, KNodeID_15, LCOT_RELAY,kRelayTypePreCharging,OFF,0,relay_off_outtime),

/// 消息节点添加：第15个节点，下一步执行节点16，15-设置预充失败放电断开状态//set_precharge_off_state=[set_precharge_off_state],type=[msg]
LOGIC_ADD_MSG_NODE(KNodeID_13, KNodeID_14, set_precharge_off_state),

/// 状态控制节点添加：第16个节点，下一步执行节点17，16-断开总负继电器//LCOT_RELAY,kRelayTypeNegative,OFF,0,1000=[LCOT_RELAY,kRelayTypeNegative,OFF,0,1000]
LOGIC_ADD_STATE_CTRL_NODE(KNodeID_14, KNodeID_15, LCOT_RELAY,kRelayTypeNegative,OFF,0,relay_off_outtime),

/// 结束流程添加：第17个节点
LOGIC_ADD_END_NODE(KNodeID_15),
};


void start_discharge_on_process(void)
{
    start_logic_ctrl_process(discharge_on_process, sizeof(discharge_on_process)/sizeof(LogicControlNodeContext),LCPS_Running,LCNT_PROCESS_MAIN);
}

