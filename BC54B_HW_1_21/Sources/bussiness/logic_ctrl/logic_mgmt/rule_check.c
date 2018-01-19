#include "bms_rule_stock.h"
#include "rule_check.h"

void slave_communication_abort_rule(void)
{
    if(bcu_get_slave_communication_state() == kAlarmNone)
    {
        bcu_alarm_disable(SELFCHECK_COMM);
    }
    if(bcu_get_slave_communication_state() == kAlarmNone)
    {
        bcu_charger_enable(SELFCHECK_COMM);
    }
    if(bcu_get_slave_communication_state() == kAlarmFirstLevel)
    {
        bcu_alarm_enable(SELFCHECK_COMM);
    }
    if(bcu_get_slave_communication_state() == kAlarmFirstLevel)
    {
        bcu_charger_disable(SELFCHECK_COMM);
    }
}


void volt_line_exception_rule(void)
{
    if(bcu_get_voltage_exception_state() == kAlarmNone)
    {
        bcu_alarm_disable(SELFCHECK_VOLT_EXCEPTION);
    }
    if(bcu_get_voltage_exception_state() == kAlarmFirstLevel)
    {
        bcu_alarm_enable(SELFCHECK_VOLT_EXCEPTION);
    }
    if(bcu_get_voltage_exception_state() == kAlarmNone)
    {
        bcu_charger_enable(SELFCHECK_VOLT_EXCEPTION);
    }
    if(bcu_get_voltage_exception_state() == kAlarmFirstLevel)
    {
        bcu_charger_disable(SELFCHECK_VOLT_EXCEPTION);
    }
}


void temp_line_exception_rule(void)
{
    if(bcu_get_temp_exception_state() == kAlarmNone)
    {
        bcu_alarm_disable(SELFCHECK_TEMP_EXCEPTION);
    }
    if(bcu_get_temp_exception_state() == kAlarmFirstLevel)
    {
        bcu_alarm_enable(SELFCHECK_TEMP_EXCEPTION);
    }
    if(bcu_get_temp_exception_state() == kAlarmNone)
    {
        bcu_charger_enable(SELFCHECK_TEMP_EXCEPTION);
    }
    if(bcu_get_temp_exception_state() == kAlarmFirstLevel)
    {
        bcu_charger_disable(SELFCHECK_TEMP_EXCEPTION);
    }
}


void dchg_hv_alarm_rule(void)
{
    if(bcu_get_dchg_hv_state() == kAlarmNone)
    {
        bcu_alarm_disable(SELFCHECK_OHV);
    }
    if(bcu_get_dchg_hv_state() == kAlarmFirstLevel)
    {
        bcu_alarm_enable(SELFCHECK_OHV);
    }
}


void chg_hv_alarm_rule(void)
{
    if(bcu_get_chg_hv_state() == kAlarmNone)
    {
        bcu_alarm_disable(SELFCHECK_CHG_OHV);
    }
    if(bcu_get_chg_hv_state() == kAlarmFirstLevel)
    {
        bcu_alarm_enable(SELFCHECK_CHG_OHV);
    }
    if(bcu_get_chg_hv_state() == kAlarmSecondLevel)
    {
        bcu_charger_disable(SELFCHECK_CHG_OHV);
    }
}


void dchg_lv_alarm_rule(void)
{
    if(bcu_get_dchg_lv_state() == kAlarmNone)
    {
        bcu_alarm_disable(SELFCHECK_OLV);
    }
    if(bcu_get_dchg_lv_state() == kAlarmFirstLevel)
    {
        bcu_alarm_enable(SELFCHECK_OLV);
    }
}


void chg_lv_alarm_rule(void)
{
    if(bcu_get_chg_lv_state() == kAlarmNone)
    {
        bcu_alarm_disable(SELFCHECK_CHG_OLV);
    }
    if(bcu_get_chg_lv_state() == kAlarmFirstLevel)
    {
        bcu_alarm_enable(SELFCHECK_CHG_OLV);
    }
}


void dchg_hc_alarm_rule(void)
{
    if(bcu_get_dchg_oc_state() == kAlarmNone)
    {
        bcu_alarm_disable(SELFCHECK_DC);
    }
    if(bcu_get_dchg_oc_state() == kAlarmFirstLevel)
    {
        bcu_alarm_enable(SELFCHECK_DC);
    }
}


void chg_hc_alarm_rule(void)
{
    if(bcu_get_chg_oc_state() == kAlarmNone)
    {
        bcu_alarm_disable(SELFCHECK_CC);
    }
    if(bcu_get_chg_oc_state() == kAlarmFirstLevel)
    {
        bcu_alarm_enable(SELFCHECK_CC);
    }
    if(bcu_get_chg_oc_state() == kAlarmSecondLevel)
    {
        bcu_charger_enable(SELFCHECK_CC);
    }
    if(bcu_get_chg_oc_state() == kAlarmThirdLevel)
    {
        bcu_charger_disable(SELFCHECK_CC);
    }
}


void dchg_ht_alarm_rule(void)
{
    if(bcu_get_dchg_ht_state() == kAlarmNone)
    {
        bcu_alarm_disable(SELFCHECK_OHT);
    }
    if(bcu_get_dchg_ht_state() == kAlarmFirstLevel)
    {
        bcu_alarm_enable(SELFCHECK_OHT);
    }
}


void chg_ht_alarm_rule(void)
{
    if(bcu_get_chg_ht_state() == kAlarmNone)
    {
        bcu_alarm_disable(SELFCHECK_CHG_OHT);
    }
    if(bcu_get_chg_ht_state() == kAlarmFirstLevel)
    {
        bcu_alarm_enable(SELFCHECK_CHG_OHT);
    }
    if(bcu_get_chg_ht_state() == kAlarmSecondLevel)
    {
        bcu_charger_enable(SELFCHECK_CHG_OHT);
    }
    if(bcu_get_chg_ht_state() == kAlarmThirdLevel)
    {
        bcu_charger_disable(SELFCHECK_CHG_OHT);
    }
}


void dchg_lt_alarm_rule(void)
{
    if(bcu_get_dchg_lt_state() == kAlarmNone)
    {
        bcu_alarm_disable(SELFCHECK_OLT);
    }
    if(bcu_get_dchg_lt_state() == kAlarmFirstLevel)
    {
        bcu_alarm_enable(SELFCHECK_OLT);
    }
}


void chg_lt_alarm_rule(void)
{
    if(bcu_get_chg_lt_state() == kAlarmNone)
    {
        bcu_alarm_disable(SELFCHECK_CHG_OLT);
    }
    if(bcu_get_chg_lt_state() == kAlarmFirstLevel)
    {
        bcu_alarm_enable(SELFCHECK_CHG_OLT);
    }
    if(bcu_get_chg_lt_state() == kAlarmSecondLevel)
    {
        bcu_charger_enable(SELFCHECK_CHG_OLT);
    }
    if(bcu_get_chg_lt_state() == kAlarmThirdLevel)
    {
        bcu_charger_disable(SELFCHECK_CHG_OLT);
    }
}


void dchg_dt_alarm_rule(void)
{
    if(bcu_get_dchg_delta_temp_state() == kAlarmNone)
    {
        bcu_alarm_disable(SELFCHECK_DT);
    }
    if(bcu_get_dchg_delta_temp_state() == kAlarmFirstLevel)
    {
        bcu_alarm_enable(SELFCHECK_DT);
    }
}


void chg_dt_alarm_rule(void)
{
    if(bcu_get_chg_delta_temp_state() == kAlarmNone)
    {
        bcu_alarm_disable(SELFCHECK_CHG_DT);
    }
    if(bcu_get_chg_delta_temp_state() == kAlarmFirstLevel)
    {
        bcu_alarm_enable(SELFCHECK_CHG_DT);
    }
    if(bcu_get_chg_delta_temp_state() == kAlarmSecondLevel)
    {
        bcu_charger_enable(SELFCHECK_CHG_DT);
    }
    if(bcu_get_chg_delta_temp_state() == kAlarmThirdLevel)
    {
        bcu_charger_disable(SELFCHECK_CHG_DT);
    }
}


void insu_alarm_rule(void)
{
    if(bcu_get_battery_insulation_state() == kAlarmNone)
    {
        bcu_alarm_disable(SELFCHECK_INSU);
    }
    if(bcu_get_battery_insulation_state() == kAlarmFirstLevel)
    {
        bcu_alarm_enable(SELFCHECK_INSU);
    }
    if(bcu_get_battery_insulation_state() == kAlarmSecondLevel)
    {
        bcu_charger_enable(SELFCHECK_INSU);
    }
    if(bcu_get_battery_insulation_state() == kAlarmThirdLevel)
    {
        bcu_charger_disable(SELFCHECK_INSU);
    }
}


void high_soc_alarm_rule(void)
{
    if(bcu_get_high_soc_state() == kAlarmNone)
    {
        bcu_alarm_disable(SELFCHECK_HIGH_SOC);
    }
    if(bcu_get_high_soc_state() == kAlarmFirstLevel)
    {
        bcu_alarm_enable(SELFCHECK_HIGH_SOC);
    }
}


void low_soc_alarm_rule(void)
{
    if(bcu_get_low_soc_state() == kAlarmNone)
    {
        bcu_alarm_disable(SELFCHECK_LOW_SOC);
    }
    if(bcu_get_low_soc_state() == kAlarmFirstLevel)
    {
        bcu_alarm_enable(SELFCHECK_LOW_SOC);
    }
}


void dchg_dv_alarm_rule(void)
{
    if(bcu_get_dchg_delta_volt_state() == kAlarmNone)
    {
        bcu_alarm_disable(SELFCHECK_DV);
    }
    if(bcu_get_dchg_delta_volt_state() == kAlarmFirstLevel)
    {
        bcu_alarm_enable(SELFCHECK_DV);
    }
}


void chg_dv_alarm_rule(void)
{
    if(bcu_get_chg_delta_volt_state() == kAlarmNone)
    {
        bcu_alarm_disable(SELFCHECK_CHG_DV);
    }
    if(bcu_get_chg_delta_volt_state() == kAlarmFirstLevel)
    {
        bcu_alarm_enable(SELFCHECK_CHG_DV);
    }
    if(bcu_get_chg_delta_volt_state() == kAlarmSecondLevel)
    {
        bcu_charger_enable(SELFCHECK_CHG_DV);
    }
    if(bcu_get_chg_delta_volt_state() == kAlarmThirdLevel)
    {
        bcu_charger_disable(SELFCHECK_CHG_DV);
    }
}


void dchg_htv_alarm_rule(void)
{
    if(bcu_get_dchg_htv_state() == kAlarmNone)
    {
        bcu_alarm_disable(SELFCHECK_TV_OHV);
    }
    if(bcu_get_dchg_htv_state() == kAlarmFirstLevel)
    {
        bcu_alarm_enable(SELFCHECK_TV_OHV);
    }
}


void chg_htv_alarm_rule(void)
{
    if(bcu_get_chg_htv_state() == kAlarmNone)
    {
        bcu_alarm_disable(SELFCHECK_CHG_TV_OHV);
    }
    if(bcu_get_chg_htv_state() == kAlarmFirstLevel)
    {
        bcu_alarm_enable(SELFCHECK_CHG_TV_OHV);
    }
    if(bcu_get_chg_htv_state() == kAlarmThirdLevel)
    {
        bcu_charger_disable(SELFCHECK_CHG_TV_OHV);
    }
}


void dchg_ltv_alarm_rule(void)
{
    if(bcu_get_dchg_ltv_state() == kAlarmNone)
    {
        bcu_alarm_disable(SELFCHECK_TV_OLV);
    }
    if(bcu_get_dchg_ltv_state() == kAlarmFirstLevel)
    {
        bcu_alarm_enable(SELFCHECK_TV_OLV);
    }
}


void chg_ltv_alarm_rule(void)
{
    if(bcu_get_chg_ltv_state() == kAlarmNone)
    {
        bcu_alarm_disable(SELFCHECK_CHG_TV_OLV);
    }
    if(bcu_get_chg_ltv_state() == kAlarmFirstLevel)
    {
        bcu_alarm_enable(SELFCHECK_CHG_TV_OLV);
    }
}

void chg_relay_rule(void)
{
    if(relay_control_is_on(kRelayTypeCharging) == 1)
    {
       bcu_set_charge_relay_state(kRelayStatusOn);        
    }
    else
    {
       bcu_set_charge_relay_state(kRelayStatusOff);    
    }
}

void dischg_relay_rule(void)
{
    if(relay_control_is_on(kRelayTypeDischarging) == 1)
    {
       bcu_set_discharge_relay_state(kRelayStatusOn);        
    }
    else
    {
       bcu_set_discharge_relay_state(kRelayStatusOff);    
    }
}


void rule_check(void)
{
    slave_communication_abort_rule();
    volt_line_exception_rule();
    temp_line_exception_rule();
    dchg_hv_alarm_rule();
    chg_hv_alarm_rule();
    dchg_lv_alarm_rule();
    chg_lv_alarm_rule();
    dchg_hc_alarm_rule();
    chg_hc_alarm_rule();
    dchg_ht_alarm_rule();
    chg_ht_alarm_rule();
    dchg_lt_alarm_rule();
    chg_lt_alarm_rule();
    dchg_dt_alarm_rule();
    chg_dt_alarm_rule();
    insu_alarm_rule();
    high_soc_alarm_rule();
    low_soc_alarm_rule();
    dchg_dv_alarm_rule();
    chg_dv_alarm_rule();
    dchg_htv_alarm_rule();
    chg_htv_alarm_rule();
    dchg_ltv_alarm_rule();
    chg_ltv_alarm_rule(); 
    //chg_relay_rule();
    //dischg_relay_rule();	
}










