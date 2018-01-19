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
/// ����ѡ��ڵ���ӣ���1���ڵ㣬����ѡ��ڵ㣬Ϊ��ʱִ�нڵ�2��Ϊ��ʱִ�нڵ�1��1-���³������ϵ�����//charger_hv_on_cond=[charger_hv_on_cond]
LOGIC_ADD_COND_SELECT_NODE(KNodeID_1, KNodeID_2, KNodeID_1, heating_is_allowed_on_cond),

/// ����ѡ��ڵ���ӣ���2���ڵ㣬����ѡ��ڵ㣬Ϊ��ʱִ�нڵ�3��Ϊ��ʱִ�нڵ�5��2-�����Ƿ�����//bms_slow_charger_is_connected=[bms_slow_charger_is_connected]
LOGIC_ADD_DELAY_NODE(KNodeID_2, KNodeID_3, heat_relay_on_delay),

/// ״̬���ƽڵ���ӣ���3���ڵ㣬��һ��ִ�нڵ�4��3-����2�ż̵���//LCOT_RELAY��1��ON��0,1000=[LCOT_RELAY��1��ON��0,relay_on_outtime]
LOGIC_ADD_STATE_CTRL_NODE(KNodeID_3, KNodeID_1, LCOT_RELAY,kRelayTypeHeating,ON,0,relay_on_outtime),
};

void start_heat_on_process(void)
{
    start_logic_ctrl_process(charge_heat_on_process, sizeof(charge_heat_on_process)/sizeof(LogicControlNodeContext),LCPS_Running,LCNT_PROCESS_MAIN);
}

