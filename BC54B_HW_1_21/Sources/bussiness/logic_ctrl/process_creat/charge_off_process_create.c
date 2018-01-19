#include "bms_relay_control_impl.h"
 #include "logic_ctrl_process_start.h"
#include "logic_ctrl_lib_func.h"

static INT8U charger_hv_off_cond(void)
{
  	if((bms_relay_diagnose_get_fault_num(kRelayTypeCharging) != 0
  	  ||charger_is_connected() == FALSE 
      ||charger_is_communication() == FALSE )
     // ||charger_charging_is_enable() == FALSE)
     &&bcu_get_charge_relay_state() == kRelayStatusOn
       ) 

    {
        return 1;
    }
    return 0;
}

static INT8U discharger_relay_is_on(void)
{
    if(relay_control_is_on(kRelayTypeDischarging) == TRUE)
        return 1;
    return 0;
}

static void set_charge_off_state(void)
{
   // set_process_active(discharge_on_process);
    bcu_set_charge_relay_state(kRelayStatusOff);
}

const LogicControlNodeContext charge_off_process[]=
{
/// ����ѡ��ڵ���ӣ���1���ڵ㣬����ѡ��ڵ㣬Ϊ��ʱִ�нڵ�2��Ϊ��ʱִ�нڵ�1��1-����ѹ�µ�����//charger_hv_off_cond=[charger_hv_off_cond]
LOGIC_ADD_COND_SELECT_NODE(KNodeID_1, KNodeID_3, KNodeID_1, charger_hv_off_cond),

/// ��ʱ�ڵ���ӣ���2���ڵ㣬��һ��ִ�нڵ�3����ʱʱ��2000,2-��ʱ2S//2000=[2000],type=[delay]
//LOGIC_ADD_DELAY_NODE(KNodeID_2, KNodeID_3, 2000),

/// ״̬���ƽڵ���ӣ���4���ڵ㣬��һ��ִ�нڵ�5��4-�Ͽ����̵���//LCOT_RELAY,kRelayTypeCharging,OFF,0,1000=[LCOT_RELAY,kRelayTypeCharging,OFF,0,1000]
LOGIC_ADD_STATE_CTRL_NODE(KNodeID_3, KNodeID_4, LCOT_RELAY,kRelayTypeCharging,OFF,0,relay_off_outtime),

/// ״̬���ƽڵ���ӣ���5���ڵ㣬��һ��ִ�нڵ�6��5-�Ͽ�����źż̵���//LCOT_RELAY,kRelayTypeChargingSignal,OFF,0,1000=[LCOT_RELAY,kRelayTypeChargingSignal,OFF,0,1000]
LOGIC_ADD_STATE_CTRL_NODE(KNodeID_4, KNodeID_5, LCOT_RELAY,kRelayTypeChargingSignal,OFF,0,relay_off_outtime),

/// ����ѡ��ڵ���ӣ���6���ڵ㣬����ѡ��ڵ㣬Ϊ��ʱִ�нڵ�8��Ϊ��ʱִ�нڵ�7��7-�ŵ�̵����Ƿ�����//discharger_relay_is_on=[discharger_relay_is_on]
LOGIC_ADD_COND_SELECT_NODE(KNodeID_5, KNodeID_7, KNodeID_6, discharger_relay_is_on),

/// ״̬���ƽڵ���ӣ���7���ڵ㣬��һ��ִ�нڵ�8��7-�Ͽ��ܸ��̵���//LCOT_RELAY,kRelayTypeNegative,OFF,0,1000=[LCOT_RELAY,kRelayTypeNegative,OFF,0,1000]
LOGIC_ADD_STATE_CTRL_NODE(KNodeID_6, KNodeID_7, LCOT_RELAY,kRelayTypeNegative,OFF,0,relay_off_outtime),

LOGIC_ADD_MSG_NODE(KNodeID_7, KNodeID_8, set_charge_off_state),

LOGIC_ADD_END_NODE(KNodeID_8),
};



void start_charge_off_process(void)
{
    start_logic_ctrl_process(charge_off_process, sizeof(charge_off_process)/sizeof(LogicControlNodeContext),LCPS_Running,LCNT_PROCESS_MAIN);
}

