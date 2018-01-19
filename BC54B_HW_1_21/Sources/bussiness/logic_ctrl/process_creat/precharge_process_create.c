#include "bms_relay_control_impl.h"
#include "logic_ctrl_lib_func.h"
#include "logic_ctrl_process_start.h"

#pragma MESSAGE DISABLE C2705 // Possible loss of data
#pragma MESSAGE DISABLE C12056 // SP debug info incorrect because of optimization or inline assembler

#pragma DATA_SEG __RPAGE_SEG PAGED_RAM
INT8U g_ems_precharge_cnt=0;
#pragma DATA_SEG DEFAULT

INT8U bms_get_precharge_cnt(void) 
{
  return  g_ems_precharge_cnt;
}

static void set_prechar_state(void)
{
    bcu_set_precharge_with_tv_state(kPrechargeStatePrecharging);
}

static void set_prechar_succ_flag(void)
{
    bcu_set_precharge_with_tv_state(kPrechargeStateFinish); //Ԥ��ɹ�
}

static INT32U bms_precharge_timeout(void)
{
    return 3000; //ms/bit Ԥ��糬ʱʱ��
}

/**
  * @brief  Ԥ�����̽ṹ������
  * \note
  * \li Ϊ��ʡ�ڴ棬���鶨��Ϊconst���ͣ�����ROM�У�
  * \li ���̳�ʼִ�еĽڵ���ӦΪNODE_ID_INIT���궨��Ϊ1���²㰴��Լ����ƣ�
  * \li ��������Ϊ�̵���ʱ�����ض�����ָenum RelayControlType����ļ̵������ͣ�
  */
const LogicControlNodeContext precharge_process[]=
{
/// ����ѡ��ڵ���ӣ���1���ڵ㣬����ѡ��ڵ㣬Ϊ��ʱִ�нڵ�2��Ϊ��ʱִ�нڵ�1��1-����Ϊ��// return_ture=[return_ture]
LOGIC_ADD_MSG_NODE(KNodeID_1, KNodeID_2, set_prechar_state),

/// ״̬���ƽڵ���ӣ���5���ڵ㣬��һ��ִ�нڵ�6��5-�պ�Ԥ��̵���//[LCOT_RELAY, kRelayTypePreCharging, ON, 0, 0=[LCOT_RELAY, kRelayTypePreCharging, ON, 0, 0]
LOGIC_ADD_STATE_CTRL_NODE(KNodeID_2, KNodeID_3, LCOT_RELAY, kRelayTypePreCharging, ON, 0, relay_on_outtime),

LOGIC_ADD_DELAY_NODE(KNodeID_3, KNodeID_4, bms_precharge_timeout),

/// ��Ϣ�ڵ���ӣ���8���ڵ㣬��һ��ִ�нڵ�12��8-��Ԥ��ɹ���־//set_prechar_flag=[set_prechar_flag],type=[msg]
LOGIC_ADD_MSG_NODE(KNodeID_4, KNodeID_5, set_prechar_succ_flag),

/// ����������ӣ���11���ڵ�
LOGIC_ADD_END_NODE(KNodeID_5),
};

