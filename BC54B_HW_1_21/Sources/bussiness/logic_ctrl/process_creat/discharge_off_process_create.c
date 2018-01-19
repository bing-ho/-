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
/// ����ѡ��ڵ���ӣ���1���ڵ㣬����ѡ��ڵ㣬Ϊ��ʱִ�нڵ�2��Ϊ��ʱִ�нڵ�1��1-�ŵ��ѹ�µ�����//discharger_hv_off_cond=[discharger_hv_off_cond]
LOGIC_ADD_COND_SELECT_NODE(KNodeID_1, KNodeID_3, KNodeID_1, discharger_hv_off_cond),

/// ��ʱ�ڵ���ӣ���2���ڵ㣬��һ��ִ�нڵ�3����ʱʱ��2000,2-��ʱ2S//2000=[2000],type=[delay]
//LOGIC_ADD_DELAY_NODE(KNodeID_2, KNodeID_3, 2000),

/// ����ѡ��ڵ���ӣ���3���ڵ㣬����ѡ��ڵ㣬Ϊ��ʱִ�нڵ�4��Ϊ��ʱִ�нڵ�5��3-Ԥ���Ƿ�ʹ��//precharge_is_enabled=[precharge_is_enabled]
LOGIC_ADD_COND_SELECT_NODE(KNodeID_3, KNodeID_4, KNodeID_5, precharge_is_enabled),

/// ״̬���ƽڵ���ӣ���4���ڵ㣬��һ��ִ�нڵ�5��4-�Ͽ�Ԥ��̵���//LCOT_RELAY,kRelayTypePreCharging,OFF,0,1000=[LCOT_RELAY,kRelayTypePreCharging,OFF,0,1000]
LOGIC_ADD_STATE_CTRL_NODE(KNodeID_4, KNodeID_5, LCOT_RELAY,kRelayTypePreCharging,OFF,0,relay_off_outtime),

/// ״̬���ƽڵ���ӣ���5���ڵ㣬��һ��ִ�нڵ�6��5-�Ͽ��ŵ�̵���//LCOT_RELAY,kRelayTypeDischarging,OFF,0,1000=[LCOT_RELAY,kRelayTypeDischarging,OFF,0,1000]
LOGIC_ADD_STATE_CTRL_NODE(KNodeID_5, KNodeID_6, LCOT_RELAY,kRelayTypeDischarging,OFF,0,relay_off_outtime),

/// ״̬���ƽڵ���ӣ���6���ڵ㣬��һ��ִ�нڵ�7��6-�Ͽ��ŵ��źż̵���//LCOT_RELAY,kRelayTypeDischargingSignal,OFF,0,1000=[LCOT_RELAY,kRelayTypeDischargingSignal,OFF,0,1000]
LOGIC_ADD_STATE_CTRL_NODE(KNodeID_6, KNodeID_7, LCOT_RELAY,kRelayTypeDischargingSignal,OFF,0,relay_off_outtime),

/// ����ѡ��ڵ���ӣ���7���ڵ㣬����ѡ��ڵ㣬Ϊ��ʱִ�нڵ�9��Ϊ��ʱִ�нڵ�8��7-���̵����Ƿ�����//charger_relay_is_on=[charger_relay_is_on]
LOGIC_ADD_COND_SELECT_NODE(KNodeID_7, KNodeID_9, KNodeID_8, charger_relay_is_on),

/// ״̬���ƽڵ���ӣ���8���ڵ㣬��һ��ִ�нڵ�9��8-�Ͽ��ܸ��̵���//LCOT_RELAY,kRelayTypeNegative,OFF,0,1000=[LCOT_RELAY,kRelayTypeNegative,OFF,0,1000]
LOGIC_ADD_STATE_CTRL_NODE(KNodeID_8, KNodeID_9, LCOT_RELAY,kRelayTypeNegative,OFF,0,relay_off_outtime),

/// ����������ӣ���9���ڵ�
LOGIC_ADD_MSG_NODE(KNodeID_9, KNodeID_10, set_discharge_off_state),

LOGIC_ADD_END_NODE(KNodeID_10),
};


void start_discharge_off_process(void)
{
    start_logic_ctrl_process(discharge_off_process, sizeof(discharge_off_process)/sizeof(LogicControlNodeContext),LCPS_Running,LCNT_PROCESS_MAIN);
}

