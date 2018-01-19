/**
 *
 * Copyright (c) 2012 Ligoo Inc.
 *
 * @file bms_relay.h
 * @brief
 * @note
 * @author
 * @date 2012-5-24
 *
 */
#ifndef __BMS_RELAY_H__
#define __BMS_RELAY_H__

#include "bms_input_signal.h"
#include "bts724g_intermediate.h"

typedef enum 
{
    kRelayTypeUnknown = 0,
    kRelayTypeDischarging,
    kRelayTypeDischargingSignal,
    kRelayTypeCharging,
    kRelayTypeChargingSignal,
    kRelayTypePreCharging,
    kRelayTypeHeating,
    kRelayTypeCooling,
    kRelayTypePositive,
    kRelayTypeNegative,
    kRelayTypeSelfDefine,
    kRelayTypeSelfDefineStart = kRelayTypeSelfDefine,
    kRelayTypeRelay1 = kRelayTypeSelfDefineStart,
    kRelayTypeRelay2,
    kRelayTypeRelay3,
    kRelayTypeRelay4,
    kRelayTypeRelay5,
    kRelayTypeRelay6,
    kRelayTypeRelay7,
    kRelayTypeRelay8,
    kRelayTypeRelay9,
    kRelayTypeRelay10,
    kRelayTypeSelfDefineStop = kRelayTypeRelay10,
    kRelayTypeReserved,
    kRelayTypeMaxCount
}RelayControlType;

/*
#define RELAY_ID_DISCHARGE      1
#define RELAY_ID_DISCHARGE_S    2
#define RELAY_ID_CHARGE         3
#define RELAY_ID_CHARGE_S       4
#define RELAY_ID_PRE_CHARGE     5
#define RELAY_ID_RESERVE1       6
#define RELAY_ID_RESERVE2       7
#define RELAY_ID_RESERVE3       8
*/
#define RELAY_ID_CHARGE         1
#define RELAY_ID_PRE_CHARGE     2
#define RELAY_ID_DISCHARGE      3
#define RELAY_ID_RESERVE1       4
#define RELAY_ID_RESERVE2       5
#define RELAY_ID_RESERVE3       6
#define RELAY_ID_DISCHARGE_S    7
#define RELAY_ID_CHARGE_S       8




/* 对于继电器类型：继电器1~继电器8,均转换成自定义类型上传给上位机 */
RelayControlType relay_control_relay_type_to_computer_display(RelayControlType type);
RelayControlType relay_control_relay_type_from_computer_display(RelayControlType type, INT8U relay_num);





#endif /* BMS_RELAY_H_ */
