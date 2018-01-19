/*******************************************************************************
 **                       ������������Դ�������޹�˾ Copyright (c)
 **                            http://www.ligoo.cn
 **�� �� ��:logic_ctrl.c
 **��    ��:
 **��������:2017.11.1
 **�ļ�˵��:�ͻ�Э����Ϣ�ӿ�
 **�޸ļ�¼:
 **��    ��:V1.0
 **��    ע:
 *******************************************************************************/
 
#include "logic_ctrl.h"
#include "bms_relay_control.h"
#include "bms_diagnosis_relay.h"
 
void logic_ctrl_update(void)
{
    if(bms_relay_diagnose_get_fault_num(kRelayTypeDischarging) == 0)
    {
        relay_control_on(kRelayTypeDischarging);
        sleep(100);
        relay_control_on(kRelayTypeNegative);
    }
    else
    {
        relay_control_off(kRelayTypeNegative);
        relay_control_off(kRelayTypeDischarging);
    }
}