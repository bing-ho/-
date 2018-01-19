/*******************************************************************************
 **                       安徽力高新能源技术有限公司 Copyright (c)
 **                            http://www.ligoo.cn
 **文 件 名:logic_ctrl.c
 **作    者:
 **创建日期:2017.11.1
 **文件说明:客户协议消息接口
 **修改记录:
 **版    本:V1.0
 **备    注:
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