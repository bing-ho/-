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
/// ����ѡ��ڵ���ӣ���1���ڵ㣬����ѡ��ڵ㣬Ϊ��ʱִ�нڵ�2��Ϊ��ʱִ�нڵ�1��1-����ѹ�ϵ�����//charger_hv_on_cond=[charger_hv_on_cond]
LOGIC_ADD_COND_SELECT_NODE(KNodeID_1, KNodeID_2, KNodeID_1, charger_hv_on_cond),

/// ��Ϣ�ڵ���ӣ���2���ڵ㣬��һ��ִ�нڵ�3��2-���ó��պ�״̬//set_charge_on_state=[set_charge_on_state],type=[msg]
LOGIC_ADD_MSG_NODE(KNodeID_2, KNodeID_3, set_charge_on_state),

/// ״̬���ƽڵ���ӣ���6���ڵ㣬��һ��ִ�нڵ�6��6-�����ܸ��̵���//LCOT_RELAY,kRelayTypeNegative,ON,0,1000=[LCOT_RELAY,kRelayTypeNegative,ON,0,1000]
LOGIC_ADD_STATE_CTRL_NODE(KNodeID_3, KNodeID_4, LCOT_RELAY,kRelayTypeNegative,ON,0,relay_on_outtime),

/// ״̬���ƽڵ���ӣ���7���ڵ㣬��һ��ִ�нڵ�8��7-���ϳ��̵���//LCOT_RELAY,kRelayTypeCharging,ON,0,1000=[LCOT_RELAY,kRelayTypeCharging,ON,0,1000]
LOGIC_ADD_STATE_CTRL_NODE(KNodeID_4, KNodeID_5, LCOT_RELAY,kRelayTypeCharging,ON,0,relay_on_outtime),

/// ״̬���ƽڵ���ӣ���30���ڵ㣬��һ��ִ�нڵ�31��30-���ϳ���źż̵���//LCOT_RELAY,kRelayTypeChargingSignal,ON,0,1000=[LCOT_RELAY,kRelayTypeChargingSignal,ON,0,1000]
LOGIC_ADD_STATE_CTRL_NODE(KNodeID_5, KNodeID_6, LCOT_RELAY,kRelayTypeChargingSignal,ON,0,relay_on_outtime),

/// ��Ϣ�ڵ���ӣ���31���ڵ㣬��һ��ִ�нڵ�32��31-���ó���ѹ//set_charge_voltage=[set_charge_voltage],type=[msg]
LOGIC_ADD_MSG_NODE(KNodeID_6, KNodeID_7, set_charge_voltage),

/// ��Ϣ�ڵ���ӣ���32���ڵ㣬��һ��ִ�нڵ�32��32-���ó�����//set_charge_current=[set_charge_current],type=[msg]
LOGIC_ADD_MSG_NODE(KNodeID_7, KNodeID_7, set_charge_current),

};

void start_charge_on_process(void)
{
    start_logic_ctrl_process(charge_on_process, sizeof(charge_on_process)/sizeof(LogicControlNodeContext),LCPS_Running,LCNT_PROCESS_MAIN);
}

