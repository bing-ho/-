/**
*
* Copyright (c) 2012 Ligoo Inc.
*
* @file bms_script_engine_impl.c
* @brief
* @note
* @author Xiangyong Zhang
* @date 2012-8-10
*
*/
#include "bms_script_engine_impl.h"


#pragma MESSAGE DISABLE C1420  //Result of function-call is ignored
#pragma MESSAGE DISABLE C4001  //Condition always FALSE
#pragma MESSAGE DISABLE C1825 // Indirection to different types
#pragma MESSAGE DISABLE C12056

#define VM_REG_CFG(NAME, INDEX) \
    vm_register_variable_ex(0, NAME, kVmInt32U, (VmFunc)script_engine_set_config, (VmFunc)script_engine_get_config, (void*)INDEX);

#define VM_REG_BCU_STATUS(NAME) \
    vm_register_variable(0, #NAME, kVmInt16U, (VmFunc)bcu_set_##NAME, (VmFunc)bcu_get_##NAME)

#define VM_REG_BCU_INT32_STATUS(NAME) \
    vm_register_variable(0, #NAME, kVmInt32U, (VmFunc)bcu_set_##NAME, (VmFunc)bcu_get_##NAME)

#define VM_REG_BCU_STATUS_WITH_NAME(NAME, STATUS) \
    vm_register_variable(0, #NAME, kVmInt16U, (VmFunc)bcu_set_##STATUS, (VmFunc)bcu_get_##STATUS)

#define VM_REG_BCU_INT32U_STATUS_WITH_NAME(NAME, STATUS) \
    vm_register_variable(0, #NAME, kVmInt32U, (VmFunc)bcu_set_##STATUS, (VmFunc)bcu_get_##STATUS)


void script_engine_init(void)
{
    vm_init();

    script_engine_register_constants();
    script_engine_register_variables();
    script_engine_register_functions();
}

void script_engine_uninit(void)
{
    vm_uninit();
}

Result script_engine_run_expr(PCSTR expr, VmValue* value)
{
    return vm_interpret(expr, value);
}

void script_engine_set_config(INT32U value, void* user_data)
{
    config_save((ConfigIndex)user_data, (INT16U)value);
}

void script_engine_get_config(INT32U* value, void* user_data)
{
    if (value) *value = config_get((ConfigIndex)user_data);
}

Result script_engine_register_constants(void)
{
    /** 电池组状态 */
    //vm_register_constant(0, "NORMAL", vm_int_to_value_ptr(kPackStateNormal));
    //vm_register_constant(0, "ULV", vm_int_to_value_ptr(kPackStateULV));
    //vm_register_constant(0, "OLV", vm_int_to_value_ptr(kPackStateOLV));
    //vm_register_constant(0, "LV", vm_int_to_value_ptr(kPackStateLV));
    //vm_register_constant(0, "HV", vm_int_to_value_ptr(kPackStateHV));
    //vm_register_constant(0, "OHV", vm_int_to_value_ptr(kPackStateOHV));
    //vm_register_constant(0, "UHV", vm_int_to_value_ptr(kPackStateUHV));

    //vm_register_constant(0, "SOC_LOW", vm_int_to_value_ptr(kSocStateLow));
    //vm_register_constant(0, "SOC_OL", vm_int_to_value_ptr(kSocStateOverLow));
    //vm_register_constant(0, "SOC_NORMAL", vm_int_to_value_ptr(kSocStateNormal));

    vm_register_constant(0, "BAT_STATE_OHV", vm_int_to_value_ptr(1 << CELL_OHV_ALERT_BIT));
    vm_register_constant(0, "BAT_STATE_LV", vm_int_to_value_ptr(1 << CELL_LV_ALERT_BIT));
    vm_register_constant(0, "BAT_STATE_OLV", vm_int_to_value_ptr(1 << CELL_ODV_STOP_BIT));
    vm_register_constant(0, "BAT_STATE_OHT", vm_int_to_value_ptr(1 << TEMP_OHT_ALERT_BIT));
    vm_register_constant(0, "BAT_STATE_OCC", vm_int_to_value_ptr(1 << CELL_OCC_ALERT_BIT));
    vm_register_constant(0, "BAT_STATE_ODC", vm_int_to_value_ptr(1 << CELL_ODC_ALERT_BIT));
    vm_register_constant(0, "BAT_STATE_COMM", vm_int_to_value_ptr(1 << PACK_COMM_ALERT_BIT));

    /* 温度状态*/
    /*vm_register_constant(0, "TEMP_NORMAL", vm_int_to_value_ptr(kTempStateNormal));
    vm_register_constant(0, "TEMP_HT", vm_int_to_value_ptr(kTempStateHT));
    vm_register_constant(0, "TEMP_OHT", vm_int_to_value_ptr(kTempStateOHT));
    vm_register_constant(0, "TEMP_UHT", vm_int_to_value_ptr(kTempStateUHT));
    vm_register_constant(0, "TEMP_LT", vm_int_to_value_ptr(kTempStateLT));
    vm_register_constant(0, "TEMP_OLT", vm_int_to_value_ptr(kTempStateOLT));
    */
    //vm_register_constant(0, "COMM_NORMAL", vm_int_to_value_ptr(kCommStateNormal));
    //vm_register_constant(0, "COMM_ABORT", vm_int_to_value_ptr(kCommStateAbort));

    /*vm_register_constant(0, "SYS_VOLT_HIGH", vm_int_to_value_ptr(kSystemVoltageStateHigh));
    vm_register_constant(0, "SYS_VOLT_LOW", vm_int_to_value_ptr(kSystemVoltageStateLow));
    vm_register_constant(0, "SYS_VOLT_NORMAL", vm_int_to_value_ptr(kSystemVoltageStateNormal));
    */
    vm_register_constant(0, "SYS_POWER_ON", vm_int_to_value_ptr(kSystemStatePowerOn));
    vm_register_constant(0, "SYS_FAILURE", vm_int_to_value_ptr(kSystemStateFailure));
    vm_register_constant(0, "SYS_CHARGING", vm_int_to_value_ptr(kSystemStateCharging));
    vm_register_constant(0, "SYS_AVAILABLE", vm_int_to_value_ptr(kSystemStateAvailable));

    /* 充电电流状态*/
    /*vm_register_constant(0, "CC_NORMAL", vm_int_to_value_ptr(kChargeCurrentStateNormal));
    vm_register_constant(0, "CC_HCC", vm_int_to_value_ptr(kChargeCurrentStateHCC));
    vm_register_constant(0, "CC_OCC", vm_int_to_value_ptr(kChargeCurrentStateOCC));

    vm_register_constant(0, "DCC_NORMAL", vm_int_to_value_ptr(kDischargeCurrentNormal));
    vm_register_constant(0, "DCC_HDC", vm_int_to_value_ptr(kDischargeCurrentHDC));
    vm_register_constant(0, "DCC_ODC", vm_int_to_value_ptr(kDischargeCurrentODC));
    */
    vm_register_constant(0, "MCU_WORK", vm_int_to_value_ptr(kMCUWorkStateWork));
    vm_register_constant(0, "MCU_STOP", vm_int_to_value_ptr(kMCUWorkStateStop));

    /*绝缘状态*/
    vm_register_constant(0, "INSU_NORMAL", vm_int_to_value_ptr(kPInsuStateNormal));
    vm_register_constant(0, "INSU_TLEAK", vm_int_to_value_ptr(kPInsuStateTLeak));
    vm_register_constant(0, "INSU_LEAK", vm_int_to_value_ptr(kPInsuStateLeak));
    vm_register_constant(0, "INSU_ULEAK", vm_int_to_value_ptr(kPInsuStateULeak));

    /* 报警标志 */
    vm_register_constant(0, "OHV_ALARM", vm_int_to_value_ptr(SELFCHECK_OHV));
    vm_register_constant(0, "OLV_ALARM", vm_int_to_value_ptr(SELFCHECK_OLV));
    vm_register_constant(0, "LSOC_ALARM", vm_int_to_value_ptr(SELFCHECK_LOW_SOC));
    vm_register_constant(0, "OHT_ALARM", vm_int_to_value_ptr(SELFCHECK_OHT));
    vm_register_constant(0, "OLT_ALARM", vm_int_to_value_ptr(SELFCHECK_OLT));
    vm_register_constant(0, "DT_ALARM", vm_int_to_value_ptr(SELFCHECK_DT));
    vm_register_constant(0, "INSU_ALARM", vm_int_to_value_ptr(SELFCHECK_INSU));
    vm_register_constant(0, "COMM_ALARM", vm_int_to_value_ptr(SELFCHECK_COMM));
    vm_register_constant(0, "CHG_OHTV_ALARM", vm_int_to_value_ptr(SELFCHECK_CHG_TV_OHV));
    vm_register_constant(0, "CHG_OLTV_ALARM", vm_int_to_value_ptr(SELFCHECK_CHG_TV_OLV));
    vm_register_constant(0, "DV_ALARM", vm_int_to_value_ptr(SELFCHECK_DV));
    vm_register_constant(0, "VOLT_EXCEPTION_ALARM", vm_int_to_value_ptr(SELFCHECK_VOLT_EXCEPTION));
    vm_register_constant(0, "TEMP_EXCEPTION_ALARM", vm_int_to_value_ptr(SELFCHECK_TEMP_EXCEPTION));
    vm_register_constant(0, "OLTV_ALARM", vm_int_to_value_ptr(SELFCHECK_TV_OLV));
    vm_register_constant(0, "OHTV_ALARM", vm_int_to_value_ptr(SELFCHECK_TV_OHV));
    vm_register_constant(0, "CC_ALARM", vm_int_to_value_ptr(SELFCHECK_CC));
    vm_register_constant(0, "DC_ALARM", vm_int_to_value_ptr(SELFCHECK_DC));

    vm_register_constant(0, "RELAY_STATE_ON", vm_int_to_value_ptr(kRelayStatusOn));
    vm_register_constant(0, "RELAY_STATE_OFF", vm_int_to_value_ptr(kRelayStatusOff));
    vm_register_constant(0, "RELAY_STATE_PRECHG", vm_int_to_value_ptr(kRelayStatusPreCharging));

    vm_register_constant(0, "RELAY_DCHARGE", vm_int_to_value_ptr(kRelayTypeDischarging));
    vm_register_constant(0, "RELAY_DCHARGE_S", vm_int_to_value_ptr(kRelayTypeDischargingSignal));
    vm_register_constant(0, "RELAY_CHARGE", vm_int_to_value_ptr(kRelayTypeCharging));
    vm_register_constant(0, "RELAY_CHARGE_S", vm_int_to_value_ptr(kRelayTypeChargingSignal));
    vm_register_constant(0, "RELAY_HEAT", vm_int_to_value_ptr(kRelayTypeHeating));
    vm_register_constant(0, "RELAY_COOL", vm_int_to_value_ptr(kRelayTypeCooling));
    vm_register_constant(0, "RELAY_PRECHG", vm_int_to_value_ptr(kRelayTypePreCharging));

    vm_register_constant(0, "DEFAULT_DELAY", vm_int_to_value_ptr(2000));

    vm_register_constant(0, "MAX_TEMP", vm_int_to_value_ptr(BATTERY_TEMPERATURE_MAX_VALUE));
    vm_register_constant(0, "MIN_TEMP", vm_int_to_value_ptr(BATTERY_TEMPERATURE_MIN_VALUE));

    vm_register_constant(0, "ALARM_NORMAL", vm_int_to_value_ptr(kAlarmNone));
    vm_register_constant(0, "FIRST_ALARM", vm_int_to_value_ptr(kAlarmFirstLevel));
    vm_register_constant(0, "SECOND_ALARM", vm_int_to_value_ptr(kAlarmSecondLevel));
    vm_register_constant(0, "THIRD_ALARM", vm_int_to_value_ptr(kAlarmThirdLevel));
    
    vm_register_constant(0, "CHG_OHV_ALARM", vm_int_to_value_ptr(SELFCHECK_CHG_OHV));
    vm_register_constant(0, "CHG_OLV_ALARM", vm_int_to_value_ptr(SELFCHECK_CHG_OLV));
    vm_register_constant(0, "CHG_OHT_ALARM", vm_int_to_value_ptr(SELFCHECK_CHG_OHT));
    vm_register_constant(0, "CHG_DT_ALARM", vm_int_to_value_ptr(SELFCHECK_CHG_DT));
    vm_register_constant(0, "CHG_OLT_ALARM", vm_int_to_value_ptr(SELFCHECK_CHG_OLT));
    vm_register_constant(0, "CHG_DV_ALARM", vm_int_to_value_ptr(SELFCHECK_CHG_DV));
    vm_register_constant(0, "HSOC_ALARM", vm_int_to_value_ptr(SELFCHECK_HIGH_SOC));
    
    vm_register_constant(0, "START_FAIL_NONE", vm_int_to_value_ptr(kStartFailureNone));
    vm_register_constant(0, "START_FAIL_OCCURE", vm_int_to_value_ptr(kStartFailureOccure));
    vm_register_constant(0, "START_FAIL_RECOVERY", vm_int_to_value_ptr(kStartFailureRecovery));
    
    vm_register_constant(0, "RELAY_POSITIVE", vm_int_to_value_ptr(kRelayTypePositive));
    vm_register_constant(0, "RELAY_NEGATIVE", vm_int_to_value_ptr(kRelayTypeNegative));
    vm_register_constant(0, "RELAY_ONE", vm_int_to_value_ptr(kRelayTypeRelay1));
    vm_register_constant(0, "RELAY_TWO", vm_int_to_value_ptr(kRelayTypeRelay2));
    vm_register_constant(0, "RELAY_THREE", vm_int_to_value_ptr(kRelayTypeRelay3));
    vm_register_constant(0, "RELAY_FOUR", vm_int_to_value_ptr(kRelayTypeRelay4));
    vm_register_constant(0, "RELAY_FIVE", vm_int_to_value_ptr(kRelayTypeRelay5));
    vm_register_constant(0, "RELAY_SIX", vm_int_to_value_ptr(kRelayTypeRelay6));
    vm_register_constant(0, "RELAY_SEVEN", vm_int_to_value_ptr(kRelayTypeRelay7));
    vm_register_constant(0, "RELAY_EIGHT", vm_int_to_value_ptr(kRelayTypeRelay8));
    
    vm_register_constant(0, "RELAY_FAULT_PRECHG", vm_int_to_value_ptr(kRelayRunCondPrecharging));
    
    vm_register_constant(0, "PRECHG_FINISH", vm_int_to_value_ptr(kPrechargeStateFinish));
    vm_register_constant(0, "PRECHG_FAIL", vm_int_to_value_ptr(kPrechargeStateFailure));
    
    return RES_OK;
}

Result script_engine_register_variables(void)
{
    /* register the configuration */

    // 容量相关
    VM_REG_CFG("cfg_left_cap", kLeftCapIndex);
    VM_REG_CFG("cfg_nominal_cap", kNominalCapIndex);
    VM_REG_CFG("cfg_total_cap", kTotalCapIndex);

    //VM_REG_CFG("cfg_sys_state_power_timeout", kSysStateAvailableDlyIndex);

    // 杂项
    VM_REG_CFG("cfg_cur_check", kCchkIndex);
    //VM_REG_CFG("cfg_slave_num", kSlaveNumIndex);
    //VM_REG_CFG("cfg_temp_num", kTemperatureNumIndex);

    // 充放电压
    VM_REG_CFG("cfg_hv_snd", kDChgHVSndAlarmIndex);
    VM_REG_CFG("cfg_hv_snd_dly", kDChgHVSndAlarmDlyIndex);
    //VM_REG_CFG("cfg_hv_snd_rel", kDChgHVSndAlarmRelIndex);
    //VM_REG_CFG("cfg_hv_snd_rel_dly", kDChgHVSndAlarmRelDlyIndex);

    VM_REG_CFG("cfg_hv_fst", kDChgHVFstAlarmIndex);
    //VM_REG_CFG("cfg_hv_fst_rel", kDChgHVFstAlarmRelIndex);
    VM_REG_CFG("cfg_hv_fst_dly", kDChgHVFstAlarmDlyIndex);
    //VM_REG_CFG("cfg_hv_fst_rel_dly", kDChgHVFstAlarmRelDlyIndex);

    VM_REG_CFG("cfg_lv_fst", kDChgLVFstAlarmIndex);
    //VM_REG_CFG("cfg_lv_fst_rel", kDChgLVFstAlarmRelIndex);
    VM_REG_CFG("cfg_lv_fst_dly", kDChgLVFstAlarmDlyIndex);
    //VM_REG_CFG("cfg_lv_fst_rel_dly", kDChgLVFstAlarmRelDlyIndex);

    VM_REG_CFG("cfg_lv_snd", kDChgLVSndAlarmIndex);
    VM_REG_CFG("cfg_lv_snd_dly", kDChgLVSndAlarmDlyIndex);
    //VM_REG_CFG("cfg_lv_snd_rel", kDChgLVSndAlarmRelIndex);
    //VM_REG_CFG("cfg_lv_snd_rel_dly", kDChgLVSndAlarmRelDlyIndex);

    VM_REG_CFG("cfg_htv_snd", kDChgHTVSndAlarmIndex);
    VM_REG_CFG("cfg_htv_snd_dly", kDChgHTVSndAlarmDlyIndex);
    //VM_REG_CFG("cfg_htv_snd_rel", kDChgHTVSndAlarmRelIndex);
    //VM_REG_CFG("cfg_htv_snd_rel_dly", kDChgHTVSndAlarmRelDlyIndex);

    VM_REG_CFG("cfg_htv_fst", kDChgHTVFstAlarmIndex);
    //VM_REG_CFG("cfg_htv_fst_rel", kDChgHTVFstAlarmRelIndex);
    VM_REG_CFG("cfg_htv_fst_dly", kDChgHTVFstAlarmDlyIndex);
    //VM_REG_CFG("cfg_htv_fst_rel_dly", kDChgHTVFstAlarmRelDlyIndex);

    VM_REG_CFG("cfg_ltv_fst", kDChgLTVFstAlarmIndex);
    //VM_REG_CFG("cfg_ltv_fst_rel", kDChgLTVFstAlarmRelIndex);
    VM_REG_CFG("cfg_ltv_fst_dly", kDChgLTVFstAlarmDlyIndex);
    //VM_REG_CFG("cfg_ltv_fst_rel_dly", kDChgLTVFstAlarmRelDlyIndex);

    VM_REG_CFG("cfg_ltv_snd", kDChgLTVSndAlarmIndex);
    VM_REG_CFG("cfg_ltv_snd_dly", kDChgLTVSndAlarmDlyIndex);
    //VM_REG_CFG("cfg_ltv_snd_rel", kDChgLTVSndAlarmRelIndex);
    //VM_REG_CFG("cfg_ltv_snd_rel_dly", kDChgLTVSndAlarmRelDlyIndex);

    VM_REG_CFG("cfg_dv_fst", kDChgDVFstAlarmIndex);
    VM_REG_CFG("cfg_dv_fst_dly", kDChgDVFstAlarmDlyIndex);
    //VM_REG_CFG("cfg_dv_fst_rel", kDChgDVFstAlarmRelIndex);
    //VM_REG_CFG("cfg_dv_fst_rel_dly", kDChgDVFstAlarmRelDlyIndex);

    VM_REG_CFG("cfg_dv_snd", kDChgDVSndAlarmIndex);
    VM_REG_CFG("cfg_dv_snd_dly", kDChgDVSndAlarmDlyIndex);
    //VM_REG_CFG("cfg_dv_snd_rel", kDChgDVSndAlarmRelIndex);
    //VM_REG_CFG("cfg_dv_snd_rel_dly", kDChgDVSndAlarmRelDlyIndex);

    // 充放电流
    VM_REG_CFG("cfg_chg_oc_snd", kChgOCSndAlarmIndex);
    VM_REG_CFG("cfg_chg_oc_snd_dly", kChgOCSndAlarmDlyIndex);
    //VM_REG_CFG("cfg_chg_oc_snd_rel", kChgOCSndAlarmRelIndex);
    //VM_REG_CFG("cfg_chg_oc_snd_rel_dly", kChgOCSndAlarmRelDlyIndex);

    VM_REG_CFG("cfg_oc_snd", kDChgOCSndAlarmIndex);
    VM_REG_CFG("cfg_oc_snd_dly", kDChgOCSndAlarmDlyIndex);
    //VM_REG_CFG("cfg_oc_snd_rel", kDChgOCSndAlarmRelIndex);
    //VM_REG_CFG("cfg_oc_snd_rel_dly", kDChgOCSndAlarmRelDlyIndex);

    // 充电器
    VM_REG_CFG("cfg_chg_cur", kChargerCurIndex);
    VM_REG_CFG("cfg_full_chg", kChargerVoltIndex);//TODO:此值无效
    VM_REG_CFG("cfg_chg_volt", kChargerVoltIndex);

    // 温度
    VM_REG_CFG("cfg_ht_fst", kDChgHTFstAlarmIndex);
    VM_REG_CFG("cfg_ht_fst_dly", kDChgHTFstAlarmDlyIndex);
    //VM_REG_CFG("cfg_ht_fst_rel", kDChgHTFstAlarmRelIndex);
    //VM_REG_CFG("cfg_ht_fst_rel_dly", kDChgHTFstAlarmRelDlyIndex);

    VM_REG_CFG("cfg_lt_fst", kDChgLTFstAlarmIndex);
    VM_REG_CFG("cfg_lt_fst_dly", kDChgLTFstAlarmDlyIndex);
    //VM_REG_CFG("cfg_lt_fst_rel", kDChgLTFstAlarmRelIndex);
    //VM_REG_CFG("cfg_lt_fst_rel_dly", kDChgLTFstAlarmRelDlyIndex);

    VM_REG_CFG("cfg_ht_snd", kDChgHTSndAlarmIndex);
    VM_REG_CFG("cfg_ht_snd_dly", kDChgHTSndAlarmDlyIndex);
    //VM_REG_CFG("cfg_ht_snd_rel", kDChgHTSndAlarmRelIndex);
    //VM_REG_CFG("cfg_ht_snd_rel_dly", kDChgHTSndAlarmRelDlyIndex);

    VM_REG_CFG("cfg_lt_snd", kDChgLTSndAlarmIndex);
    VM_REG_CFG("cfg_lt_snd_dly", kDChgLTSndAlarmDlyIndex);
    //VM_REG_CFG("cfg_lt_snd_rel", kDChgLTSndAlarmRelIndex);
    //VM_REG_CFG("cfg_lt_snd_rel_dly", kDChgLTSndAlarmRelDlyIndex);

    VM_REG_CFG("cfg_dt_fst", kDChgDTFstAlarmIndex);
    VM_REG_CFG("cfg_dt_fst_dly", kDChgDTFstAlarmDlyIndex);
    //VM_REG_CFG("cfg_dt_fst_rel", kDChgDTFstAlarmRelIndex);
    //VM_REG_CFG("cfg_dt_fst_rel_dly", kDChgDTFstAlarmRelDlyIndex);

    VM_REG_CFG("cfg_dt_snd", kDChgDTSndAlarmIndex);
    VM_REG_CFG("cfg_dt_snd_dly", kDChgDTSndAlarmDlyIndex);
    //VM_REG_CFG("cfg_dt_snd_rel", kDChgDTSndAlarmRelIndex);
    //VM_REG_CFG("cfg_dt_snd_rel_dly", kDChgDTSndAlarmRelDlyIndex);

    // SOC相关
    VM_REG_CFG("cfg_calib_lsoc", kCapCalibLowSoc);
    VM_REG_CFG("cfg_calib_lsoc_dly", kCapCalibLowSocDly);

    VM_REG_CFG("cfg_lsoc_fst", kLSOCFstAlarmIndex);
    //VM_REG_CFG("cfg_lsoc_fst_rel", kLSOCFstAlarmRelIndex);
    VM_REG_CFG("cfg_lsoc_fst_dly", kLSOCFstAlarmDlyIndex);
    //VM_REG_CFG("cfg_lsoc_fst_rel_dly", kLSOCFstAlarmRelDlyIndex);
    VM_REG_CFG("cfg_lsoc_snd", kLSOCSndAlarmIndex);
    VM_REG_CFG("cfg_lsoc_snd_dly", kLSOCSndAlarmDlyIndex);
    //VM_REG_CFG("cfg_lsoc_snd_rel", kLSOCSndAlarmRelIndex);
    //VM_REG_CFG("cfg_lsoc_snd_rel_dly", kLSOCSndAlarmRelDlyIndex);

    // 漏电保护
    VM_REG_CFG("cfg_insu_trd", kInsuTrdAlarmIndex);
    //VM_REG_CFG("cfg_insu_trd_rel", kInsuTrdAlarmRelIndex);
    VM_REG_CFG("cfg_insu_trd_dly", kInsuTrdAlarmDlyIndex);
    //VM_REG_CFG("cfg_insu_trd_rel_dly", kInsuTrdAlarmRelDlyIndex);

    VM_REG_CFG("cfg_insu_snd", kInsuSndAlarmIndex);
    //VM_REG_CFG("cfg_insu_snd_rel", kInsuSndAlarmRelIndex);
    VM_REG_CFG("cfg_insu_snd_dly", kInsuSndAlarmDlyIndex);
    //VM_REG_CFG("cfg_insu_snd_rel_dly", kInsuSndAlarmRelDlyIndex);

    VM_REG_CFG("cfg_insu_fst", kInsuFstAlarmIndex);
    //VM_REG_CFG("cfg_insu_fst_rel", kInsuFstAlarmRelIndex);
    VM_REG_CFG("cfg_insu_fst_dly", kInsuFstAlarmDlyIndex);
    //VM_REG_CFG("cfg_insu_fst_rel_dly", kInsuFstAlarmRelDlyIndex);

#if BMS_SUPPORT_SYSTEM_VOLTAGE
    // 系统电压
    //VM_REG_CFG("cfg_sys_hv", kSystemVoltHVIndex);
    //VM_REG_CFG("cfg_sys_lv", kSystemVoltLVIndex);
    //VM_REG_CFG("cfg_sys_hv_delay", kSystemVoltHVDlyIndex);
    //VM_REG_CFG("cfg_sys_lv_delay", kSystemVoltLVDlyIndex);
    //VM_REG_CFG("cfg_sys_hv_release", kSystemVoltHVReleaseIndex);
    //VM_REG_CFG("cfg_sys_lv_release", kSystemVoltLVReleaseIndex);
    //VM_REG_CFG("cfg_sys_hv_release_delay", kSystemVoltHVReleaseDlyIndex);
    //VM_REG_CFG("cfg_sys_lv_release_delay", kSystemVoltLVReleaseDlyIndex);
#endif

    // 继电器
    VM_REG_CFG("cfg_chg_relay_off_delay", kChgRelayOffDlyIndex);
    VM_REG_CFG("cfg_chg_relay_on_delay", kChgRelayOnDlyIndex);
    VM_REG_CFG("cfg_dchg_relay_off_delay", kDisChgRelayOffDlyIndex);
    VM_REG_CFG("cfg_dchg_relay_on_delay", kDisChgRelayOnDlyIndex);
    VM_REG_CFG("cfg_prechg_relay_off_delay", kPreChgRelayOffDlyIndex);
    VM_REG_CFG("cfg_prechg_relay_on_delay", kPreChgRelayOnDlyIndex);

    VM_REG_CFG("cfg_comm_fst_dly", kCommFstAlarmDlyIndex);

#if BMS_SUPPORT_SYSTEM_VOLTAGE
    VM_REG_CFG("cfg_sys_volt_calib", kSystemVoltageCalibration);
#endif

    VM_REG_CFG("cfg_heat_on_temp", kHeatOnTemperature);
    VM_REG_CFG("cfg_heat_off_temp", kHeatOffTemperature);
    VM_REG_CFG("cfg_cool_on_temp", kCoolOnTemperature);
    VM_REG_CFG("cfg_cool_off_temp", kCoolOffTemperature);
    //VM_REG_CFG("cfg_heat_on_temp_delay", kHeatOnTempDlyIndex);
    //VM_REG_CFG("cfg_heat_off_temp_delay", kHeatOffTempDlyIndex);
    //VM_REG_CFG("cfg_cool_on_temp_delay", kCoolOnTempDlyIndex);
    //VM_REG_CFG("cfg_cool_off_temp_delay", kCoolOffTempDlyIndex);

    VM_REG_CFG("cfg_relay_dchg_same_port", kRelayDischargeSamePort);
    VM_REG_CFG("cfg_relay_chg_mutex", kRelayChargeMutex);
    VM_REG_CFG("cfg_relay_prechg", kRelayPreCharge);

    VM_REG_CFG("cfg_volt_exception_off_delay", kVLineFstAlarmDlyIndex);
    //VM_REG_CFG("cfg_volt_exception_off_release_delay", kVLineFstAlarmRelDlyIndex);
    VM_REG_CFG("cfg_temp_exception_off_delay", kTLineFstAlarmDlyIndex);
    //VM_REG_CFG("cfg_temp_exception_off_release_delay", kTLineFstAlarmRelDlyIndex);

    /* temp */
    // 工作状态延时
#if BMS_SUPPORT_LOAD_NODE_DETCTION
    //VM_REG_CFG("cfg_work_state_dly", kWorkStateDlyIndex);
    //VM_REG_CFG("cfg_stop_state_dly", kWorkStateDlyIndex);
#endif

    /* register the status */
    //VM_REG_BCU_STATUS(mcu_work_state);
    VM_REG_BCU_INT32_STATUS(total_voltage);
    VM_REG_BCU_STATUS(current);
    VM_REG_BCU_STATUS(SOC);
    VM_REG_BCU_STATUS(SOH);
    VM_REG_BCU_STATUS(average_voltage);
    VM_REG_BCU_STATUS(high_voltage);
    VM_REG_BCU_STATUS(high_voltage_id);
    VM_REG_BCU_STATUS(low_voltage);
    VM_REG_BCU_STATUS(low_voltage_id);
    VM_REG_BCU_STATUS(high_temperature);
    VM_REG_BCU_STATUS(high_temperature_id);
    VM_REG_BCU_STATUS(low_temperature);
    VM_REG_BCU_STATUS(low_temperature_id);
    VM_REG_BCU_INT32_STATUS(pack_state);
    VM_REG_BCU_STATUS(positive_insulation_resistance);
    VM_REG_BCU_STATUS(negative_insulation_resistance);
    VM_REG_BCU_STATUS_WITH_NAME(insulation_resistance, system_insulation_resistance);
    VM_REG_BCU_STATUS(insulation_work_state);
    VM_REG_BCU_STATUS(system_state);
    VM_REG_BCU_STATUS(dchg_hv_state);
    VM_REG_BCU_STATUS(dchg_lv_state);
    VM_REG_BCU_INT32_STATUS(alarm_check_flag);
    VM_REG_BCU_STATUS(low_soc_state);
    VM_REG_BCU_STATUS(dchg_ht_state);
    VM_REG_BCU_STATUS(chg_ht_state);
    VM_REG_BCU_STATUS(dchg_lt_state);
    VM_REG_BCU_STATUS(dchg_delta_temp_state);
    VM_REG_BCU_STATUS(dchg_delta_volt_state);
    VM_REG_BCU_STATUS(chg_oc_state);
    VM_REG_BCU_STATUS(dchg_oc_state);
    VM_REG_BCU_STATUS_WITH_NAME(insulation_state, battery_insulation_state);
    VM_REG_BCU_INT32_STATUS(insulation_total_voltage);
    VM_REG_BCU_STATUS_WITH_NAME(comm_state, slave_communication_state);
    VM_REG_BCU_STATUS(system_voltage);
    VM_REG_BCU_INT32_STATUS(com_abort_num);

    VM_REG_BCU_STATUS(charge_relay_state);
    VM_REG_BCU_STATUS(discharge_relay_state);
    VM_REG_BCU_STATUS(charge_exception_flag);
    VM_REG_BCU_STATUS(discharge_exception_flag);

    VM_REG_BCU_STATUS(voltage_exception);
    VM_REG_BCU_STATUS(temp_exception);
    VM_REG_BCU_STATUS(voltage_exception_state);
    VM_REG_BCU_STATUS(temp_exception_state);

    VM_REG_BCU_STATUS(dchg_htv_state);

    VM_REG_BCU_STATUS(discharge_relay_check);
    VM_REG_BCU_STATUS(full_charge_flag);
    
    // 充电温度  TODO
    VM_REG_CFG("cfg_chg_ht_fst", kChgHTFstAlarmIndex);
    VM_REG_CFG("cfg_chg_ht_fst_dly", kChgHTFstAlarmDlyIndex);
    //VM_REG_CFG("cfg_chg_ht_fst_rel", kChgHTFstAlarmRelIndex);
    //VM_REG_CFG("cfg_chg_ht_fst_rel_dly", kChgHTFstAlarmRelDlyIndex);

    VM_REG_CFG("cfg_chg_ht_snd", kChgHTSndAlarmIndex);
    VM_REG_CFG("cfg_chg_ht_snd_dly", kChgHTSndAlarmDlyIndex);
    //VM_REG_CFG("cfg_chg_ht_snd_rel", kChgHTSndAlarmRelIndex);
    //VM_REG_CFG("cfg_chg_ht_snd_rel_dly", kChgHTSndAlarmRelDlyIndex);
    
    VM_REG_BCU_STATUS(chg_ht_state);
    
    // 充放电流
    VM_REG_CFG("cfg_chg_oc_fst", kChgOCFstAlarmIndex);
    VM_REG_CFG("cfg_chg_oc_fst_dly", kChgOCFstAlarmDlyIndex);
    //VM_REG_CFG("cfg_chg_oc_fst_rel", kChgOCFstAlarmRelIndex);
    //VM_REG_CFG("cfg_chg_oc_fst_rel_dly", kChgOCFstAlarmRelDlyIndex);

    VM_REG_CFG("cfg_oc_fst", kDChgOCFstAlarmIndex);
    VM_REG_CFG("cfg_oc_fst_dly", kDChgOCFstAlarmDlyIndex);
    //VM_REG_CFG("cfg_oc_fst_rel", kDChgOCFstAlarmRelIndex);
    //VM_REG_CFG("cfg_oc_fst_rel_dly", kDChgOCFstAlarmRelDlyIndex);
    
    VM_REG_BCU_STATUS_WITH_NAME(pcm_comm_state, pcm_communication_state);
    VM_REG_BCU_STATUS_WITH_NAME(pcm_tv, pcm_total_voltage);
    VM_REG_BCU_STATUS_WITH_NAME(pcm_tv_rate, pcm_tv_rate_of_change);
    
    VM_REG_BCU_STATUS(chg_hv_state);
    VM_REG_BCU_STATUS(chg_lv_state);
    VM_REG_BCU_STATUS(chg_lt_state);
    VM_REG_BCU_STATUS(chg_delta_temp_state);
    VM_REG_BCU_STATUS(high_soc_state);
    VM_REG_BCU_STATUS(chg_delta_volt_state);
    VM_REG_BCU_STATUS(dchg_ltv_state);
    VM_REG_BCU_STATUS(chg_htv_state);
    VM_REG_BCU_STATUS(chg_ltv_state);
    
    VM_REG_CFG("cfg_custom_para_one", kCustomEeParaOneIndex);
    VM_REG_CFG("cfg_custom_para_two", kCustomEeParaTwoIndex);
    
    VM_REG_CFG("cfg_chg_hv_fst", kChgHVFstAlarmIndex);
    //VM_REG_CFG("cfg_chg_hv_fst_rel", kChgHVFstAlarmRelIndex);
    VM_REG_CFG("cfg_chg_hv_snd", kChgHVSndAlarmIndex);
    //VM_REG_CFG("cfg_chg_hv_snd_rel", kChgHVSndAlarmRelIndex);
    VM_REG_CFG("cfg_chg_hv_trd", kChgHVTrdAlarmIndex);
    //VM_REG_CFG("cfg_chg_hv_trd_rel", kChgHVTrdAlarmRelIndex);
    VM_REG_CFG("cfg_chg_hv_fst_dly", kChgHVFstAlarmDlyIndex);
    //VM_REG_CFG("cfg_chg_hv_fst_rel_dly", kChgHVFstAlarmRelDlyIndex);
    VM_REG_CFG("cfg_chg_hv_snd_dly", kChgHVSndAlarmDlyIndex);
    //VM_REG_CFG("cfg_chg_hv_snd_rel_dly", kChgHVSndAlarmRelDlyIndex);
    VM_REG_CFG("cfg_chg_hv_trd_dly", kChgHVTrdAlarmDlyIndex);
    //VM_REG_CFG("cfg_chg_hv_trd_rel_dly", kChgHVTrdAlarmRelDlyIndex);
    VM_REG_CFG("cfg_hv_trd", kDChgHVTrdAlarmIndex);
    //VM_REG_CFG("cfg_hv_trd_rel", kDChgHVTrdAlarmRelIndex);
    VM_REG_CFG("cfg_hv_trd_dly", kDChgHVTrdAlarmDlyIndex);
    //VM_REG_CFG("cfg_hv_trd_rel_dly", kDChgHVTrdAlarmRelDlyIndex);
    
    VM_REG_CFG("cfg_chg_lv_fst", kChgLVFstAlarmIndex);
    //VM_REG_CFG("cfg_chg_lv_fst_rel", kChgLVFstAlarmRelIndex);
    VM_REG_CFG("cfg_chg_lv_snd", kChgLVSndAlarmIndex);
    //VM_REG_CFG("cfg_chg_lv_snd_rel", kChgLVSndAlarmRelIndex);
    VM_REG_CFG("cfg_chg_lv_trd", kChgLVTrdAlarmIndex);
    //VM_REG_CFG("cfg_chg_lv_trd_rel", kChgLVTrdAlarmRelIndex);
    VM_REG_CFG("cfg_chg_lv_fst_dly", kChgLVFstAlarmDlyIndex);
    //VM_REG_CFG("cfg_chg_lv_fst_rel_dly", kChgLVFstAlarmRelDlyIndex);
    VM_REG_CFG("cfg_chg_lv_snd_dly", kChgLVSndAlarmDlyIndex);
    //VM_REG_CFG("cfg_chg_lv_snd_rel_dly", kChgLVSndAlarmRelDlyIndex);
    VM_REG_CFG("cfg_chg_lv_trd_dly", kChgLVTrdAlarmDlyIndex);
    //VM_REG_CFG("cfg_chg_lv_trd_rel_dly", kChgLVTrdAlarmRelDlyIndex);
    VM_REG_CFG("cfg_lv_trd", kDChgLVTrdAlarmIndex);
    //VM_REG_CFG("cfg_lv_trd_rel", kDChgLVTrdAlarmRelIndex);
    VM_REG_CFG("cfg_lv_trd_dly", kDChgLVTrdAlarmDlyIndex);
    //VM_REG_CFG("cfg_lv_trd_rel_dly", kDChgLVTrdAlarmRelDlyIndex);
    
    VM_REG_CFG("cfg_chg_hv_trd", kChgOCTrdAlarmIndex);
    //VM_REG_CFG("cfg_chg_hv_trd_rel", kChgOCTrdAlarmRelIndex);
    VM_REG_CFG("cfg_chg_hv_trd_dly", kChgOCTrdAlarmDlyIndex);
    //VM_REG_CFG("cfg_chg_hv_trd_rel_dly", kChgOCTrdAlarmRelDlyIndex);
    
    VM_REG_CFG("cfg_hv_trd", kDChgOCTrdAlarmIndex);
    //VM_REG_CFG("cfg_hv_trd_rel", kDChgOCTrdAlarmRelIndex);
    VM_REG_CFG("cfg_hv_trd_dly", kDChgOCTrdAlarmDlyIndex);
    //VM_REG_CFG("cfg_hv_trd_rel_dly", kDChgOCTrdAlarmRelDlyIndex);
    
    VM_REG_CFG("cfg_chg_ht_trd", kChgHTTrdAlarmIndex);
    //VM_REG_CFG("cfg_chg_ht_trd_rel", kChgHTTrdAlarmRelIndex);
    VM_REG_CFG("cfg_chg_ht_trd_dly", kChgHTTrdAlarmDlyIndex);
    //VM_REG_CFG("cfg_chg_ht_trd_rel_dly", kChgHTTrdAlarmRelDlyIndex);
    
    VM_REG_CFG("cfg_ht_trd", kDChgHTTrdAlarmIndex);
    //VM_REG_CFG("cfg_ht_trd_rel", kDChgHTTrdAlarmRelIndex);
    VM_REG_CFG("cfg_ht_trd_dly", kDChgHTTrdAlarmDlyIndex);
    //VM_REG_CFG("cfg_ht_trd_rel_dly", kDChgHTTrdAlarmRelDlyIndex);
    
    VM_REG_CFG("cfg_chg_lt_fst", kChgLTFstAlarmIndex);
    //VM_REG_CFG("cfg_chg_lt_fst_rel", kChgLTFstAlarmRelIndex);
    VM_REG_CFG("cfg_chg_lt_snd", kChgLTSndAlarmIndex);
    //VM_REG_CFG("cfg_chg_lt_snd_rel", kChgLTSndAlarmRelIndex);
    VM_REG_CFG("cfg_chg_lt_trd", kChgLTTrdAlarmIndex);
    //VM_REG_CFG("cfg_chg_lt_trd_rel", kChgLTTrdAlarmRelIndex);
    VM_REG_CFG("cfg_chg_lt_fst_dly", kChgLTFstAlarmDlyIndex);
    //VM_REG_CFG("cfg_chg_lt_fst_rel_dly", kChgLTFstAlarmRelDlyIndex);
    VM_REG_CFG("cfg_chg_lt_snd_dly", kChgLTSndAlarmDlyIndex);
    //VM_REG_CFG("cfg_chg_lt_snd_rel_dly", kChgLTSndAlarmRelDlyIndex);
    VM_REG_CFG("cfg_chg_lt_trd_dly", kChgLTTrdAlarmDlyIndex);
    //VM_REG_CFG("cfg_chg_lt_trd_rel_dly", kChgLTTrdAlarmRelDlyIndex);
    VM_REG_CFG("cfg_lt_trd", kDChgLTTrdAlarmIndex);
    //VM_REG_CFG("cfg_lt_trd_rel", kDChgLTTrdAlarmRelIndex);
    VM_REG_CFG("cfg_lt_trd_dly", kDChgLTTrdAlarmDlyIndex);
    //VM_REG_CFG("cfg_lt_trd_rel_dly", kDChgLTTrdAlarmRelDlyIndex);
    
    VM_REG_CFG("cfg_chg_dt_fst", kChgDTFstAlarmIndex);
    //VM_REG_CFG("cfg_chg_dt_fst_rel", kChgDTFstAlarmRelIndex);
    VM_REG_CFG("cfg_chg_dt_snd", kChgDTSndAlarmIndex);
    //VM_REG_CFG("cfg_chg_dt_snd_rel", kChgDTSndAlarmRelIndex);
    VM_REG_CFG("cfg_chg_dt_trd", kChgDTTrdAlarmIndex);
    //VM_REG_CFG("cfg_chg_dt_trd_rel", kChgDTTrdAlarmRelIndex);
    VM_REG_CFG("cfg_chg_dt_fst_dly", kChgDTFstAlarmDlyIndex);
    //VM_REG_CFG("cfg_chg_dt_fst_rel_dly", kChgDTFstAlarmRelDlyIndex);
    VM_REG_CFG("cfg_chg_dt_snd_dly", kChgDTSndAlarmDlyIndex);
    //VM_REG_CFG("cfg_chg_dt_snd_rel_dly", kChgDTSndAlarmRelDlyIndex);
    VM_REG_CFG("cfg_chg_dt_trd_dly", kChgDTTrdAlarmDlyIndex);
    //VM_REG_CFG("cfg_chg_dt_trd_rel_dly", kChgDTTrdAlarmRelDlyIndex);
    VM_REG_CFG("cfg_dt_trd", kDChgDTTrdAlarmIndex);
    //VM_REG_CFG("cfg_dt_trd_rel", kDChgDTTrdAlarmRelIndex);
    VM_REG_CFG("cfg_dt_trd_dly", kDChgDTTrdAlarmDlyIndex);
    //VM_REG_CFG("cfg_dt_trd_rel_dly", kDChgDTTrdAlarmRelDlyIndex);
    
    //VM_REG_CFG("cfg_comm_snd_dly", kCommSndAlarmDlyIndex);
    //VM_REG_CFG("cfg_comm_snd_rel_dly", kCommSndAlarmRelDlyIndex);
    //VM_REG_CFG("cfg_comm_trd_dly", kCommTrdAlarmDlyIndex);
    //VM_REG_CFG("cfg_comm_trd_rel_dly", kCommTrdAlarmRelDlyIndex);
    
    //VM_REG_CFG("cfg_tline_snd_dly", kTempLineSndAlarmDlyIndex);
    //VM_REG_CFG("cfg_tline_snd_rel_dly", kTempLineSndAlarmRelDlyIndex);
    //VM_REG_CFG("cfg_tline_trd_dly", kTempLineTrdAlarmDlyIndex);
    //VM_REG_CFG("cfg_tline_trd_rel_dly", kTempLineTrdAlarmRelDlyIndex);
    
    //VM_REG_CFG("cfg_vline_snd_dly", kVoltLineSndAlarmDlyIndex);
    //VM_REG_CFG("cfg_vline_snd_rel_dly", kVoltLineSndAlarmRelDlyIndex);
    //VM_REG_CFG("cfg_vline_trd_dly", kVoltLineTrdAlarmDlyIndex);
    //VM_REG_CFG("cfg_vline_trd_rel_dly", kVoltLineTrdAlarmRelDlyIndex);
    
    //VM_REG_CFG("cfg_hsoc_trd", kSOCHighTrdAlarmIndex);
    //VM_REG_CFG("cfg_hsoc_trd_rel", kSOCHighTrdAlarmRelIndex);
    //VM_REG_CFG("cfg_hsoc_trd_dly", kSOCHighTrdAlarmDlyIndex);
    //VM_REG_CFG("cfg_hsoc_trd_rel_dly", kSOCHighTrdAlarmRelDlyIndex);
    
    VM_REG_CFG("cfg_lsoc_trd", kSOCLowTrdAlarmIndex);
    //VM_REG_CFG("cfg_lsoc_trd_rel", kSOCLowTrdAlarmRelIndex);
    VM_REG_CFG("cfg_lsoc_trd_dly", kSOCLowTrdAlarmDlyIndex);
    //VM_REG_CFG("cfg_lsoc_trd_rel_dly", kSOCLowTrdAlarmRelDlyIndex);
    
    VM_REG_CFG("cfg_chg_dv_fst", kChgDVFstAlarmIndex);
    //VM_REG_CFG("cfg_chg_dv_fst_rel", kChgDVFstAlarmRelIndex);
    VM_REG_CFG("cfg_chg_dv_snd", kChgDVSndAlarmIndex);
    //VM_REG_CFG("cfg_chg_dv_snd_rel", kChgDVSndAlarmRelIndex);
    VM_REG_CFG("cfg_chg_dv_trd", kChgDVTrdAlarmIndex);
    //VM_REG_CFG("cfg_chg_dv_trd_rel", kChgDVTrdAlarmRelIndex);
    VM_REG_CFG("cfg_chg_dv_fst_dly", kChgDVFstAlarmDlyIndex);
    //VM_REG_CFG("cfg_chg_dv_fst_rel_dly", kChgDVFstAlarmRelDlyIndex);
    VM_REG_CFG("cfg_chg_dv_snd_dly", kChgDVSndAlarmDlyIndex);
    //VM_REG_CFG("cfg_chg_dv_snd_rel_dly", kChgDVSndAlarmRelDlyIndex);
    VM_REG_CFG("cfg_chg_dv_trd_dly", kChgDVTrdAlarmDlyIndex);
    //VM_REG_CFG("cfg_chg_dv_trd_rel_dly", kChgDVTrdAlarmRelDlyIndex);
    VM_REG_CFG("cfg_dv_trd", kDChgDVTrdAlarmIndex);
    //VM_REG_CFG("cfg_dv_trd_rel", kDChgDVTrdAlarmRelIndex);
    VM_REG_CFG("cfg_dv_trd_dly", kDChgDVTrdAlarmDlyIndex);
    //VM_REG_CFG("cfg_dv_trd_rel_dly", kDChgDVTrdAlarmRelDlyIndex);
    
    VM_REG_CFG("cfg_chg_htv_fst", kChgHTVFstAlarmIndex);
    //VM_REG_CFG("cfg_chg_htv_fst_rel", kChgHTVFstAlarmRelIndex);
    VM_REG_CFG("cfg_chg_htv_snd", kChgHTVSndAlarmIndex);
    //VM_REG_CFG("cfg_chg_htv_snd_rel", kChgHTVSndAlarmRelIndex);
    VM_REG_CFG("cfg_chg_htv_trd", kChgHTVTrdAlarmIndex);
    //VM_REG_CFG("cfg_chg_htv_trd_rel", kChgHTVTrdAlarmRelIndex);
    VM_REG_CFG("cfg_chg_htv_fst_dly", kChgHTVFstAlarmDlyIndex);
    //VM_REG_CFG("cfg_chg_htv_fst_rel_dly", kChgHTVFstAlarmRelDlyIndex);
    VM_REG_CFG("cfg_chg_htv_snd_dly", kChgHTVSndAlarmDlyIndex);
    //VM_REG_CFG("cfg_chg_htv_snd_rel_dly", kChgHTVSndAlarmRelDlyIndex);
    VM_REG_CFG("cfg_chg_htv_trd_dly", kChgHTVTrdAlarmDlyIndex);
    //VM_REG_CFG("cfg_chg_htv_trd_rel_dly", kChgHTVTrdAlarmRelDlyIndex);
    VM_REG_CFG("cfg_htv_trd", kDChgHTVTrdAlarmIndex);
    //VM_REG_CFG("cfg_htv_trd_rel", kDChgHTVTrdAlarmRelIndex);
    VM_REG_CFG("cfg_htv_trd_dly", kDChgHTVTrdAlarmDlyIndex);
    //VM_REG_CFG("cfg_htv_trd_rel_dly", kDChgHTVTrdAlarmRelDlyIndex);
    
    VM_REG_CFG("cfg_chg_ltv_fst", kChgLTVFstAlarmIndex);
    //VM_REG_CFG("cfg_chg_ltv_fst_rel", kChgLTVFstAlarmRelIndex);
    VM_REG_CFG("cfg_chg_ltv_snd", kChgLTVSndAlarmIndex);
    //VM_REG_CFG("cfg_chg_ltv_snd_rel", kChgLTVSndAlarmRelIndex);
    VM_REG_CFG("cfg_chg_ltv_trd", kChgLTVTrdAlarmIndex);
    //VM_REG_CFG("cfg_chg_ltv_trd_rel", kChgLTVTrdAlarmRelIndex);
    VM_REG_CFG("cfg_chg_ltv_fst_dly", kChgLTVFstAlarmDlyIndex);
    //VM_REG_CFG("cfg_chg_ltv_fst_rel_dly", kChgLTVFstAlarmRelDlyIndex);
    VM_REG_CFG("cfg_chg_ltv_snd_dly", kChgLTVSndAlarmDlyIndex);
    //VM_REG_CFG("cfg_chg_ltv_snd_rel_dly", kChgLTVSndAlarmRelDlyIndex);
    VM_REG_CFG("cfg_chg_ltv_trd_dly", kChgLTVTrdAlarmDlyIndex);
    //VM_REG_CFG("cfg_chg_ltv_trd_rel_dly", kChgLTVTrdAlarmRelDlyIndex);
    VM_REG_CFG("cfg_ltv_trd", kDChgLTVTrdAlarmIndex);
    //VM_REG_CFG("cfg_ltv_trd_rel", kDChgLTVTrdAlarmRelIndex);
    VM_REG_CFG("cfg_ltv_trd_dly", kDChgLTVTrdAlarmDlyIndex);
    //VM_REG_CFG("cfg_ltv_trd_rel_dly", kDChgLTVTrdAlarmRelDlyIndex);
    
    VM_REG_BCU_STATUS(dchg_start_failure_state);
    VM_REG_BCU_STATUS(chg_start_failure_state);
    VM_REG_BCU_STATUS(precharge_with_tv_state);
    
    return RES_OK;
}

extern Result rule_engine_checker_post_rule(INT16U event);

//#pragma STRING_SEG STRING_ROM
Result script_engine_register_functions(void)
{
    /* register the actions */
    //vm_register_function(0, "test_beep", rule_stock_test_beep, kVmPrototypeVW);
    //vm_register_function(0, "show_cpu", rule_stock_dump_cpu_usage, kVmPrototypeVV);
    //vm_register_function(0, "test", rule_stock_test, kVmPrototypeVW);

    vm_register_function(0, "get", rule_stock_get_var, kVmPrototypeWW);
    vm_register_function(0, "set", rule_stock_set_var, kVmPrototypeWWW);

    // 数学函数
    vm_register_function(0, "abs", abs, kVmPrototypeWW);

    // 蜂鸣器
    vm_register_function(0, "beep_on", beep_on, kVmPrototypeVV);
    vm_register_function(0, "beep_off", beep_off, kVmPrototypeVV);

    // 系统函数
    vm_register_function(0, "sleep", sleep, kVmPrototypeVI);

    vm_register_function(0, "check_and_fix_full_cap", soc_check_and_fix_as_full, kVmPrototypeVV);
    vm_register_function(0, "check_and_fix_empty_cap", soc_check_and_fix_as_empty, kVmPrototypeVV);
    vm_register_function(0, "fix_as_empty_cap", soc_fix_as_empty, kVmPrototypeVV);
    vm_register_function(0, "fix_as_full_cap", soc_fix_as_full, kVmPrototypeVV);

    vm_register_function(0, "is_keep", rule_stock_rule_check_keep, kVmPrototypeWWI);

    // 报警相关
    vm_register_function(0, "flag_alarm", bcu_flag_alarm_check_flag, kVmPrototypeVI);
    vm_register_function(0, "unflag_alarm", bcu_unflag_alarm_check_flag, kVmPrototypeVI);
    vm_register_function(0, "beep_alarm", alert_beep_on, kVmPrototypeVI);
    vm_register_function(0, "unbeep_alarm", alert_beep_off, kVmPrototypeVI);
    vm_register_function(0, "enable_alarm", bcu_alarm_enable, kVmPrototypeVI);
    vm_register_function(0, "disable_alarm", bcu_alarm_disable, kVmPrototypeVI);

    // 充电机
    vm_register_function(0, "enable_charger", bcu_charger_enable, kVmPrototypeVI);
    vm_register_function(0, "disable_charger", bcu_charger_disable, kVmPrototypeVI);

    vm_register_function(0, "charger_off", charger_off, kVmPrototypeWV);
    vm_register_function(0, "charger_on", charger_on, kVmPrototypeWV);
    vm_register_function(0, "charger_is_charging", charger_is_charging, kVmPrototypeBV);
    vm_register_function(0, "charger_is_connected", charger_is_connected, kVmPrototypeBV);
    vm_register_function(0, "charger_set_current", charger_set_current, kVmPrototypeWW);
    vm_register_function(0, "charger_set_exp_current", rule_stock_charger_set_exp_current, kVmPrototypeVV);

    // 标志位
    vm_register_function(0, "flag_pack_state", bcu_flag_pack_state, kVmPrototypeVI);
    vm_register_function(0, "unflag_pack_state", bcu_unflag_pack_state, kVmPrototypeVI);

    vm_register_function(0, "get_diff_temp", bcu_get_difference_temperature, kVmPrototypeWV);
    vm_register_function(0, "get_diff_volt", bcu_get_difference_voltage, kVmPrototypeWV);

    vm_register_function(0, "is_charging", bcu_is_charging, kVmPrototypeBV);
    vm_register_function(0, "is_discharging", bcu_is_discharging, kVmPrototypeBV);
    vm_register_function(0, "is_sys_failure", bcu_is_system_failure, kVmPrototypeBV);

    vm_register_function(0, "relay_on", relay_control_on, kVmPrototypeVW);
    vm_register_function(0, "relay_off", relay_control_off, kVmPrototypeVW);
    vm_register_function(0, "relay_hw_on", relay_on, kVmPrototypeWB);
    vm_register_function(0, "relay_hw_off", relay_off, kVmPrototypeWB);

    vm_register_function(0, "post_event", event_deliver, kVmPrototypeWB);

    //vm_register_function(0, "load_node_detect", hardware_load_node_detect, kVmPrototypeBV);
    vm_register_function(0, "mcu_reset", mcu_reset, kVmPrototypeVV);
    //vm_register_function(0, "mcu_to_work", mcu_switch_to_work_mode, kVmPrototypeVV);

    vm_register_function(0, "is_valid_temp", bmu_is_valid_temperature, kVmPrototypeBB);
    vm_register_function(0, "is_valid_voltage", bmu_is_valid_voltage, kVmPrototypeBW);

    vm_register_function(0, "set_left_cap", soc_update_left_cap_by_tenfold_ah, kVmPrototypeVW);//vm_register_function(0, "set_left_cap", soc_update_left_cap_by_ah, kVmPrototypeVW);
    
    vm_register_function(0, "is_sys_start_failure", bcu_is_system_start_failure, kVmPrototypeBV);
    vm_register_function(0, "charger_get_ready_status", charger_get_charge_ready_status, kVmPrototypeWV); 

    vm_register_function(0, "higher_alarm_priority_run", rule_stock_higher_level_priority_alarm_run, kVmPrototypeVW);
    vm_register_function(0, "relay_is_on", relay_control_is_on, kVmPrototypeBW);
    
    vm_register_function(0, "input_is_high", input_signal_is_high, kVmPrototypeBB);
    vm_register_function(0, "input_is_low", input_signal_is_low, kVmPrototypeBB);
    
    vm_register_function(0, "relay_ctl_is_enable", relay_control_enable_is_on, kVmPrototypeBW);
    
    vm_register_function(0, "hv_state", bcu_get_charge_state, kVmPrototypeWV);
    vm_register_function(0, "lv_state", bcu_get_discharge_state, kVmPrototypeWV);
    vm_register_function(0, "ht_state", bcu_get_high_temperature_state, kVmPrototypeWV);
    vm_register_function(0, "lt_state", bcu_get_low_temperature_state, kVmPrototypeWV);
    vm_register_function(0, "delta_volt_state", bcu_get_delta_voltage_state, kVmPrototypeWV);
    vm_register_function(0, "delta_temp_state", bcu_get_delta_temperature_state, kVmPrototypeWV);
    vm_register_function(0, "htv_state", bcu_get_high_total_volt_state, kVmPrototypeWV);
    vm_register_function(0, "ltv_state", bcu_get_low_total_volt_state, kVmPrototypeWV);
    vm_register_function(0, "charger_is_communication", charger_is_communication, kVmPrototypeBV);
    vm_register_function(0, "get_relay_fault", bms_relay_diagnose_get_fault_num, kVmPrototypeBW);
    
    vm_register_function(0, "cc_is_connect", guobiao_charger_cc_is_connected, kVmPrototypeBV);
    vm_register_function(0, "cc2_is_connect", guobiao_charger_cc2_is_connected, kVmPrototypeBV);
    vm_register_function(0, "relay_delay_on", relay_control_delay_on, kVmPrototypeWWW);
    vm_register_function(0, "relay_delay_off", relay_control_delay_off, kVmPrototypeWWW);
    
    return RES_OK;
}
//#pragma STRING_SEG DEFAULT


