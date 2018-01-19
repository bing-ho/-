#include "bms_relay_control_impl.h"
#include "logic_ctrl_lib_func.h"
#include "logic_ctrl_process_start.h"


#pragma MESSAGE DISABLE C2705


static INT8U heating_is_allowed_off_cond(void)
{
  	if(TEMP_4_DISPLAY(bcu_get_low_temperature())> config_get(kHeatOffTemperature)
  	//&&bmu_is_valid_temperature(g_bms_info.low_temperature)
  	&&relay_control_is_on(kRelayTypeHeating)==1)
    {
        return 1;
    }
    return 0;
}

static INT32U heat_relay_off_delay(void)
{
    return 5000;
}


const LogicControlNodeContext charge_heat_off_process[]=
{
/// ����ѡ��ڵ���ӣ���1���ڵ㣬����ѡ��ڵ㣬Ϊ��ʱִ�нڵ�2��Ϊ��ʱִ�нڵ�1��1-����ѹ�ϵ�����//discharger_hv_on_cond=[discharger_hv_on_cond]
LOGIC_ADD_COND_SELECT_NODE(KNodeID_1, KNodeID_2, KNodeID_1, heating_is_allowed_off_cond),

LOGIC_ADD_DELAY_NODE(KNodeID_2, KNodeID_3, heat_relay_off_delay),

/// ״̬���ƽڵ���ӣ���2���ڵ㣬��һ��ִ�нڵ�3��2-����6�ż̵���//LCOT_RELAY,6,ON,0,1000=[LCOT_RELAY,6,ON,0,1000]
LOGIC_ADD_STATE_CTRL_NODE(KNodeID_3, KNodeID_1, LCOT_RELAY,kRelayTypeHeating,OFF,0,relay_off_outtime),
};

void start_heat_off_process(void)
{
    start_logic_ctrl_process(charge_heat_off_process, sizeof(charge_heat_off_process)/sizeof(LogicControlNodeContext),LCPS_Running,LCNT_PROCESS_MAIN);
}

