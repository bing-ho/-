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
/// ����ѡ��ڵ���ӣ���1���ڵ㣬����ѡ��ڵ㣬Ϊ��ʱִ�нڵ�2��Ϊ��ʱִ�нڵ�1��1-�ŵ��ѹ�ϵ�����//discharger_hv_on_cond=[discharger_hv_on_cond]
LOGIC_ADD_COND_SELECT_NODE(KNodeID_1, KNodeID_2, KNodeID_1, discharger_hv_on_cond),

/// ��Ϣ�ڵ���ӣ���18���ڵ㣬��һ��ִ�нڵ�2��18-���÷ŵ�պ�״̬//set_discharge_on_state=[set_discharge_on_state],type=[msg]
LOGIC_ADD_MSG_NODE(KNodeID_2, KNodeID_3, set_discharge_on_state),

/// ״̬���ƽڵ���ӣ���5���ڵ㣬��һ��ִ�нڵ�6��5-�����ܸ��̵���//LCOT_RELAY,kRelayTypeNegative,ON,0,1000=[LCOT_RELAY,kRelayTypeNegative,ON,0,1000]
LOGIC_ADD_STATE_CTRL_NODE(KNodeID_3, KNodeID_4, LCOT_RELAY,kRelayTypeNegative,ON,0,relay_on_outtime),

/// ����ѡ��ڵ���ӣ���6���ڵ㣬����ѡ��ڵ㣬Ϊ��ʱִ�нڵ�7��Ϊ��ʱִ�нڵ�10��6-Ԥ���Ƿ�ʹ��//precharge_is_enabled=[precharge_is_enabled]
LOGIC_ADD_COND_SELECT_NODE(KNodeID_4, KNodeID_5, KNodeID_9, precharge_is_enabled),

/// ��Ϣ�ڵ���ӣ���7���ڵ㣬��һ��ִ�нڵ�8��7-����Ԥ��״̬//set_precharge_on_state=[set_precharge_on_state],type=[msg]
LOGIC_ADD_MSG_NODE(KNodeID_5, KNodeID_6, set_precharge_on_state),
           
/// ��������ӣ���8���ڵ㣬��һ��ִ�нڵ�9��8-����Ԥ������ //precharge_process=[precharge_process],type=[add]
LOGIC_ADD_LAUNCH_NODE(KNodeID_6, KNodeID_7, precharge_process,11),

/// ����ѡ��ڵ���ӣ���9���ڵ㣬����ѡ��ڵ㣬Ϊ��ʱִ�нڵ�10��Ϊ��ʱִ�нڵ�15��9-Ԥ���Ƿ�ɹ�//precharge_is_ok=[precharge_is_ok]
LOGIC_ADD_COND_SELECT_NODE(KNodeID_7, KNodeID_8, KNodeID_13, precharge_is_ok),

/// ��Ϣ�ڵ���ӣ���10���ڵ㣬��һ��ִ�нڵ�11��10-���÷ŵ�պ�״̬//set_discharge_on_state=[set_discharge_on_state],type=[msg]
LOGIC_ADD_MSG_NODE(KNodeID_8, KNodeID_9, set_discharge_on_state),

/// ״̬���ƽڵ���ӣ���11���ڵ㣬��һ��ִ�нڵ�12��11-���Ϸŵ�̵���//LCOT_RELAY,kRelayTypeDischarging,ON,0,1000=[LCOT_RELAY,kRelayTypeDischarging,ON,0,1000]
LOGIC_ADD_STATE_CTRL_NODE(KNodeID_9, KNodeID_10, LCOT_RELAY,kRelayTypeDischarging,ON,0,relay_on_outtime),

/// ״̬���ƽڵ���ӣ���12���ڵ㣬��һ��ִ�нڵ�13��12-���Ϸŵ��źż̵���//LCOT_RELAY,kRelayTypeDischargingSignal,ON,0,1000=[LCOT_RELAY,kRelayTypeDischargingSignal,ON,0,1000]
LOGIC_ADD_STATE_CTRL_NODE(KNodeID_10, KNodeID_11, LCOT_RELAY,kRelayTypeDischargingSignal,ON,0,relay_on_outtime),

/// ��ʱ�ڵ���ӣ���13���ڵ㣬��һ��ִ�нڵ�14����ʱʱ��200,13-��ʱ2S//2000=[2000],type=[delay]
LOGIC_ADD_DELAY_NODE(KNodeID_11, KNodeID_12, precharge_off_delay),

/// ״̬���ƽڵ���ӣ���14���ڵ㣬��һ��ִ�нڵ�17��14-�Ͽ�Ԥ��̵���//LCOT_RELAY,kRelayTypePreCharging,OFF,0,1000=[LCOT_RELAY,kRelayTypePreCharging,OFF,0,1000]
LOGIC_ADD_STATE_CTRL_NODE(KNodeID_12, KNodeID_15, LCOT_RELAY,kRelayTypePreCharging,OFF,0,relay_off_outtime),

/// ��Ϣ�ڵ���ӣ���15���ڵ㣬��һ��ִ�нڵ�16��15-����Ԥ��ʧ�ܷŵ�Ͽ�״̬//set_precharge_off_state=[set_precharge_off_state],type=[msg]
LOGIC_ADD_MSG_NODE(KNodeID_13, KNodeID_14, set_precharge_off_state),

/// ״̬���ƽڵ���ӣ���16���ڵ㣬��һ��ִ�нڵ�17��16-�Ͽ��ܸ��̵���//LCOT_RELAY,kRelayTypeNegative,OFF,0,1000=[LCOT_RELAY,kRelayTypeNegative,OFF,0,1000]
LOGIC_ADD_STATE_CTRL_NODE(KNodeID_14, KNodeID_15, LCOT_RELAY,kRelayTypeNegative,OFF,0,relay_off_outtime),

/// ����������ӣ���17���ڵ�
LOGIC_ADD_END_NODE(KNodeID_15),
};


void start_discharge_on_process(void)
{
    start_logic_ctrl_process(discharge_on_process, sizeof(discharge_on_process)/sizeof(LogicControlNodeContext),LCPS_Running,LCNT_PROCESS_MAIN);
}

