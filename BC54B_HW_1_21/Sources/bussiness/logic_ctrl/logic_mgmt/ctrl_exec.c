#include "ctrl_exec.h"              // define LogicControlObjectType
#include "bms_charger_common.h"     // 电压电流设置头文件
#include "bms_relay.h"              // 枚举类型头文件
#include "bms_relay_control_impl.h" // use relay_control_get_id
#include "bts724g_hardware.h"          // 继电器控制头文件
#include <string.h>
#include <stdio.h>


INT16U atom_ctrl_operate(LogicControlObjectType object_type, INT8U object_id, INT16U ctrl_state)
{
    switch (object_type)
    {
    // 继电器控制，正常开合与强制开合
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

        // 设置电流
    case LCOT_CURRENT:
        return charger_set_current(ctrl_state);

        // 设置电压
    case LCOT_VOLTAGE:
        return charger_set_voltage(ctrl_state);
    }

    return RES_ERR;
}
