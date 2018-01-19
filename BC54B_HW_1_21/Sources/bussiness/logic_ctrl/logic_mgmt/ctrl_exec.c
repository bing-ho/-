#include "ctrl_exec.h"              // define LogicControlObjectType
#include "bms_charger_common.h"     // ��ѹ��������ͷ�ļ�
#include "bms_relay.h"              // ö������ͷ�ļ�
#include "bms_relay_control_impl.h" // use relay_control_get_id
#include "bts724g_hardware.h"          // �̵�������ͷ�ļ�
#include <string.h>
#include <stdio.h>


INT16U atom_ctrl_operate(LogicControlObjectType object_type, INT8U object_id, INT16U ctrl_state)
{
    switch (object_type)
    {
    // �̵������ƣ�����������ǿ�ƿ���
    case LCOT_RELAY:
        switch (ctrl_state)
        {
        case kRelayOn:
            return relay_on(object_id);

        case kRelayOff:
            return relay_off(object_id);

        case kRelayForceOn:
            return relay_force_on(object_id);

        case kRelayForceOff:
            return relay_force_off(object_id);
        }
        break;

        // ���õ���
    case LCOT_CURRENT:
        return charger_set_current(ctrl_state);

        // ���õ�ѹ
    case LCOT_VOLTAGE:
        return charger_set_voltage(ctrl_state);
    }

    return RES_ERR;
}
