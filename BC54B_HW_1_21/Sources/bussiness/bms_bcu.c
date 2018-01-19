/**
*
* Copyright (c) 2012 Ligoo Inc.
*
* @file bms_bcu.c
* @brief
* @note
* @author
* @date 2012-5-9
*
*/

#define BCU_BMS_VAR_IMPL

#include "bms_bcu.h"
#include "bms_charger_temperature.h"
#include "bsu_sample.h"
#include "bms_bsu.h"
#include "run_mode.h"

#pragma MESSAGE DISABLE C5919 //Conversion of floating to unsigned integral
#pragma MESSAGE DISABLE C12056 // SP debug info incorrect because of optimization or inline assembler

Version g_bms_version = {
        BMS_VER_HARDWARE,
        BMS_VER_NAME,
        BMS_VER_TIME,
        BMS_VER_MAJOR_NUMBER,
        BMS_VER_MINOR_NUMBER,
        BMS_VER_REVISION_NUMBER,
        BMS_VER_BUILD_NUMBER
};

#define BCU_CHARGE_IS_STOP()    (bcu_get_full_charge_flag() != 0 || bcu_check_charge_total_time())
#define BCU_CHARGE_IS_RESTART() (g_charger_off_line_flag && !bcu_check_charge_total_time() && MV_TO_100MV(bcu_get_total_voltage()) < BMS_FULL_CHG_RELEASE_VOLT)//config_get(kChgFullChgTVReleaseIndex)

#define BCU_RESET_INSU_RES()    bcu_set_negative_insulation_resistance(0xFFFF);\
                                bcu_set_positive_insulation_resistance(0xFFFF);\
                                bcu_set_system_insulation_resistance(0xFFFF);
#define BCU_RESET_INSU_EXTEND_VOLT()  bcu_set_insulation_pre_total_voltage(0);\
		                        bcu_set_insulation_pch_total_voltage(0);\
		                        bcu_set_insulation_hv3_total_voltage(0);
#define BCU_RESET_EXT_INSU_RES()    bcu_set_ext_negative_insulation_resistance(0xFFFF);\
                                    bcu_set_ext_positive_insulation_resistance(0xFFFF);\
                                    bcu_set_ext_system_insulation_resistance(0xFFFF);
#define BCU_RESET_EXT_INSU_EXTEND_VOLT()  bcu_set_ext_insulation_pre_total_voltage(0);\
		                                      bcu_set_ext_insulation_pch_total_voltage(0);\
		                                      bcu_set_ext_insulation_hv3_total_voltage(0);
#define VERSION_STRING_MAX_LEN          60


char g_bms_version_string[VERSION_STRING_MAX_LEN] = {0};

#pragma DATA_SEG __RPAGE_SEG PAGED_RAM

static INT32U g_bcu_communication_abort_num_temp = 0;
static INT32U g_bcu_full_charge_timer = 0;
static INT8U g_charger_off_line_flag = 0; //用于满充后再次恢复充电时充电机需离线一次的判断
static FULL_CHARGE_STATE g_full_charge_state = 0;

/****************温升报警参数*******************/
#define BCU_TEMPERATURE_RISING_CHECK_INTERVAL   120000 //ms
#define BCU_TEMPERATURE_RISING_DIFF_TEMP        2 //温差

static INT8U g_temperature_rising_temp = 0;
static INT8U g_temperature_rising_delta_temp = 0;
static INT32U g_temperature_rising_last_tick = 0;

/*********************end***********************/

INT8U g_bcu_power_up_dchg_failure = 0;
INT8U g_bcu_run_time_dchg_failure = 0;
INT32U g_bcu_power_up_dchg_last_tick = 0;
INT8U g_bcu_power_up_chg_failure = 0;
INT8U g_bcu_run_time_chg_failure = 0;
INT32U g_bcu_power_up_chg_last_tick = 0;

#pragma DATA_SEG DEFAULT


#define BCU_POWER_ON_DELAY  1000
#pragma DATA_SEG __RPAGE_SEG PAGED_RAM

static INT32U g_bcu_system_state_power_on_tick = 0;

#pragma DATA_SEG DEFAULT

void bcu_check_full_charge(void);
void bcu_check_cut_off_current(void);
INT8U bcu_check_full_charge_condition(void);
INT8U bcu_check_charge_total_time(void);
void bcu_full_charge_soc_check(void);
void bcu_system_state_update(void);
void bcu_update_dchg_start_failure_status(void);
void bcu_update_chg_start_failure_status(void);
Result bcu_start_new_slave_heart_beat(ConfigIndex index, INT16U new_value);
void bcu_check_current_auto_to_zero_self(void);

BmsInfo* __far bcu_get_bms_info(void)
{
    return &g_bms_info;
}

void bcu_init(void)
{
    /** set the init value */
    BCU_RESET_INSU_RES();
    BCU_RESET_EXT_INSU_RES();
    bcu_set_insulation_work_state(kInsulationWorkStateNotConnected);
    bcu_set_ext_insulation_work_state(kInsulationWorkStateNotConnected);
    /** load the setting from config */
    g_bcu_full_charge_timer = get_tick_count();
    g_bcu_system_state_power_on_tick = get_tick_count();
    bcu_set_dchg_start_failure_state(kStartFailureOccure);
    bcu_set_chg_start_failure_state(kStartFailureOccure);
    
    g_bcu_power_up_chg_last_tick = get_tick_count();
    g_bcu_power_up_dchg_last_tick = get_tick_count();
    
    bcu_load_config();
    config_register_observer(kSlaveNumIndex, kSlaveNumIndex, bcu_start_new_slave_heart_beat);
    event_observe((EventTypeId)kInfoEventChrOutletHTState, bcu_chr_outlet_ht_alarm_event, NULL);
    
    job_schedule(MAIN_JOB_GROUP, BCU_UPDATE_JOB_PERIODIC, bcu_update, NULL);

    if(mode != RUN_MODE_READ)
    {
    #if (BMS_SUPPORT_MLOCK == 1)       
      job_schedule(MAIN_JOB_GROUP, ChgLock_JOB_STATUS_PERIODIC, bcu_ChargerLock_StateUpdate, NULL);
    #endif    
    }
}

void bcu_unint(void)
{
}

void bcu_update(void* data)
{
    UNUSED(data);
    
    if(mode != RUN_MODE_READ) 
    {
      bcu_update_config();
      bcu_update_soc();
      bcu_check_current_auto_to_zero_self();//上电400MS内检测电流进行自动校零
      bcu_check_insulation();
      bmu_update();
      bcu_update_communication();
      bcu_check_full_charge();
      bcu_system_state_update();
      //bms_ocv_soc_check();
      bcu_update_dchg_start_failure_status();
      bcu_update_chg_start_failure_status();
    }
    bcu_reset_check();
 }

void bcu_load_config(void)
{
}

/***********************************满充状态函数*****************************************/

void bcu_check_full_charge(void)
{
    bcu_check_cut_off_current(); //更新满充状态标志
    
    switch(g_full_charge_state)
    {
        case CHARGE_NORM:
            if(BCU_CHARGE_IS_STOP()) //满充或充电总时间过长
            {
                g_charger_off_line_flag = 0;
                bcu_charger_disable(SELFCHECK_FULL_CHARGE); //停止充电
                g_full_charge_state = CHARGE_FULL;
            }
            break;
        case CHARGE_FULL:
            if(!(charger_is_charging() || charger_is_connected())) //判断充电机离线
                g_charger_off_line_flag = 1;
            
            if(BCU_CHARGE_IS_RESTART())
            {
                bcu_charger_enable(SELFCHECK_FULL_CHARGE); //重新启动充电
                bcu_set_full_charge_flag(0);
                soc_reset_charging_checked_flags();
                g_full_charge_state = CHARGE_NORM;
            }
            break;
    } 
    bcu_full_charge_soc_check();
}

void bcu_check_cut_off_current(void)
{
    INT16U max_charge_release_volt = TV_100_MV_TO_V(BMS_FULL_CHG_RELEASE_VOLT);//config_get(kChgFullChgTVReleaseIndex)
    INT32U now_tick = get_tick_count();
    
    if(bcu_get_full_charge_flag() == 0)
    {
        if(bcu_check_full_charge_condition()) 
        {
            if(get_interval_by_tick(g_bcu_full_charge_timer, now_tick) >= config_get(kChgCutOffCurDlyIndex))
            {
                bcu_set_full_charge_flag(1);
                g_bcu_full_charge_timer = now_tick;
            }
            return;
        }
    }
    g_bcu_full_charge_timer = now_tick;

}

INT8U bcu_check_full_charge_condition(void)
{
    INT16U cut_off_current = config_get(kChgCutOffCurIndex);
    INT16U max_charge_volt = TV_100_MV_TO_V(config_get(kChargerVoltIndex));
    INT16U cur_total_volt = (INT16U)MV_TO_V(bcu_get_total_voltage());
    
    if(bcu_get_current() < BMS_VALID_CHARGE_CURRENT_MIN) //电流小于截止电流
        return 0;
    
    if(cut_off_current < BMS_VALID_CHARGE_CURRENT_MIN)
        cut_off_current = BMS_VALID_CHARGE_CURRENT_MIN;
    
    if(!(cur_total_volt > max_charge_volt ||         //总压达到充电最大总压
        abs(cur_total_volt - max_charge_volt) < 5)) //允许有5V的误差
        return 0;
    
    if(bcu_get_current() <= cut_off_current) //电流小于截止电流
        return 1;
    
    return 0;
}

INT8U bcu_check_charge_total_time(void)
{
#if CHARGE_TOTAL_TIME_CHECK_EN    
    if(S_TO_HOUR(get_bms_charge_eclipse_time_s()) >= config_get(kChgTotalTimeMaxIndex))
        return 1;
#endif
    return 0;
}

void bcu_full_charge_soc_check(void)
{
    if(bcu_get_full_charge_flag() == 1)
    {
        soc_check_and_fix_as_full();
    }
}

/**********************************************************************************/

#if 0
BCU_BMS_VAR_TEST_DEF(INT16U, difference_temperature)
#else
INT16U bcu_get_difference_temperature(void)
{
    INT16U min, max;
    OS_INIT_CRITICAL();

    OS_ENTER_CRITICAL();
    min = g_bms_info.low_temperature;
    max = g_bms_info.high_temperature;
    OS_EXIT_CRITICAL();

    if (!bmu_is_valid_temperature((INT8U)min) || !bmu_is_valid_temperature((INT8U)max) || min > max)
    {
        return 0;
    }
    else
    {
        return max - min;
    }
}
#endif

INT16U bcu_get_difference_voltage(void)
{
    INT16U max, min;
    OS_INIT_CRITICAL();

    OS_ENTER_CRITICAL();
    max = bcu_get_high_voltage();
    min = bcu_get_low_voltage();
    OS_EXIT_CRITICAL();

    if (!bmu_is_valid_voltage(max) || !bmu_is_valid_voltage(min) || min > max)
    {
        return 0;
    }
    else
    {
        return max - min;
    }
}

INT16U bcu_get_charging_current(void)
{
    INT16U current = bcu_get_current();
    return ((current > 0 && current < 0x8000) ? current : 0);
}

INT16U bcu_get_discharging_current(void)
{
    INT16U current = bcu_get_current();
    return ((current >= 0x8000) ? abs((INT16S)current) : 0);
}

const Version* bcu_get_version(void)
{
    return &g_bms_version;
}

BOOLEAN bcu_is_charging(void)
{
    INT16U current = bcu_get_current();
    return ((current > 0) && (current < 0x8000));
}

BOOLEAN bcu_is_discharging(void)
{
    return (bcu_get_current() > 0x8000);
}
/*
INT32U g_test_system_failure[] = {0, 1000, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0};
*/
BOOLEAN bcu_is_system_failure(void)
{
    return (bcu_get_charge_state() >= kAlarmSecondLevel ||
            bcu_get_discharge_state() >= kAlarmSecondLevel ||
            bcu_get_high_temperature_state() >= kAlarmSecondLevel ||
            bcu_get_high_current_state()  >= kAlarmSecondLevel ||
            bcu_get_battery_insulation_state() >=kAlarmSecondLevel ||
            bcu_get_high_total_volt_state() >= kAlarmSecondLevel ||
            bcu_get_low_total_volt_state() >= kAlarmSecondLevel ||
            bcu_get_low_temperature_state() >= kAlarmSecondLevel ||
            bcu_get_delta_temperature_state() >= kAlarmSecondLevel ||
            bcu_get_delta_voltage_state() >= kAlarmSecondLevel);
}

BOOLEAN bcu_is_system_start_failure(void)
{
    INT8U i, slave_num;
    
    if(system_voltage_is_valid())
    {
        if(system_voltage_is_usful() == FALSE) return TRUE; //系统供电电压检测
    }
    
    slave_num = (INT8U)config_get(kSlaveNumIndex);
    for(i=0; i<slave_num; i++)
    {
        if(bmu_self_check(i) == 0) break;
    }
    if(i != slave_num) return TRUE;
    
    if(bcu_get_sys_exception_flags() & SYS_EXCEPTION_CONFIG_WRITE_ERROR) return TRUE;
    if(bcu_get_sys_exception_flags() & SYS_EXCEPTION_FLASH_ERROR) return TRUE;
    
    return FALSE;
}

static INT8U g_bcu_power_on_exception_flag = 0;
void bcu_system_state_update(void)
{
    bms_relay_runtime_fault_update(kRelayTypeDischarging, &g_bcu_run_time_dchg_failure);
    bms_relay_runtime_fault_update(kRelayTypeCharging, &g_bcu_run_time_chg_failure);
    switch(bcu_get_system_state())
    {
        case kSystemStatePowerOn:
            if(get_interval_by_tick(g_bcu_system_state_power_on_tick, get_tick_count()) >= BCU_POWER_ON_DELAY)
            {
                if(charger_is_connected())
                {
                    if(bcu_get_dchg_start_failure_state() == kStartFailureOccure)
                    {
                        g_bcu_power_on_exception_flag |= 1;
                        bcu_set_system_state(kSystemStateFailure);
                    }
                    else
                        bcu_set_system_state(kSystemStateCharging);
                }
                else
                {
                    if(bcu_get_dchg_start_failure_state() == kStartFailureOccure)
                    {
                        g_bcu_power_on_exception_flag |= 2;
                        bcu_set_system_state(kSystemStateFailure);
                    }
                    else
                        bcu_set_system_state(kSystemStateAvailable);
                }
            }
            break;
        case kSystemStateAvailable:
            if(charger_is_connected())
            {
                if(g_bcu_run_time_chg_failure == 0) bcu_set_system_state(kSystemStateCharging);
                else bcu_set_system_state(kSystemStateFailure);
            }
            else
            {
                if((bcu_get_dchg_start_failure_state() == kStartFailureOccure) || (g_bcu_run_time_dchg_failure != 0)) bcu_set_system_state(kSystemStateFailure);
            }
            break;
        case kSystemStateCharging:
            if(!charger_is_connected())
            {
                if(g_bcu_run_time_dchg_failure == 0) bcu_set_system_state(kSystemStateAvailable);
                else bcu_set_system_state(kSystemStateFailure);
            }
            else
            {
                if((bcu_get_dchg_start_failure_state() == kStartFailureOccure) || (g_bcu_run_time_chg_failure != 0)) bcu_set_system_state(kSystemStateFailure);
            }
            break;
        case kSystemStateFailure:
            if(charger_is_connected())
            {
                if(g_bcu_power_on_exception_flag & 0x01) 
                {
                    if(bcu_get_dchg_start_failure_state() == kStartFailureRecovery)
                    {
                        g_bcu_power_on_exception_flag &= ~0x01;
                        bcu_set_system_state(kSystemStateCharging);
                    }
                }
                else
                {
                    if(g_bcu_run_time_chg_failure == 0)
                        bcu_set_system_state(kSystemStateCharging);
                }
            }
            else
            {
                if(g_bcu_power_on_exception_flag & 0x02) 
                {
                    if(bcu_get_dchg_start_failure_state() == kStartFailureRecovery)
                    {
                        g_bcu_power_on_exception_flag &= ~0x02;
                        bcu_set_system_state(kSystemStateAvailable);
                    }
                }
                else
                {
                    if(g_bcu_run_time_dchg_failure == 0)
                        bcu_set_system_state(kSystemStateAvailable);
                }
            }
            break;
        case kSystemStateDiagnose:
            
            break;
        case kSystemStatePowerOff:
            
            break;
        default:
            bcu_set_system_state(kSystemStatePowerOn);
            break;
    }
}

const char* bcu_get_version_string(void)
{
    /** if the version string is not initialized. */
    if(g_bms_version_string[0] == '\0')
    {
        //RPAGE = (INT8U)((INT32U)g_bms_version_string / 0xFFFF);
        SAFE_CALL(sprintf(g_bms_version_string, "%s_%s_%d.%d.%d.%d_", g_bms_version.name, g_bms_version.hardware,
                g_bms_version.major_number, g_bms_version.minor_number, g_bms_version.revision_number,
                g_bms_version.build_number));
        strcat(g_bms_version_string, g_bms_version.time);
    }

    return g_bms_version_string;
}

INT16U bcu_get_status_flag(void)
{
    // TODO: 可进一步优化, 将状态位改变放在相应状态改变函数中

    INT16U flag = 0;
    //OS_CPU_SR cpu_sr = 0;
    //OS_ENTER_CRITICAL();
    if (bcu_get_charge_state() >= kAlarmSecondLevel)
        BCU_ADD_FLAG(flag, BMS_OHV_STS_BIT);
    else
        BCU_REMOVE_FLAG(flag, BMS_OHV_STS_BIT);

    if (bcu_get_discharge_state() >= kAlarmSecondLevel)
        BCU_ADD_FLAG(flag,BMS_OLV_STS_BIT);
    else
        BCU_REMOVE_FLAG(flag,BMS_OLV_STS_BIT);

    if (bcu_get_high_temperature_state() >= kAlarmSecondLevel)
        BCU_ADD_FLAG(flag,BMS_OHT_STS_BIT);
    else
        BCU_REMOVE_FLAG(flag,BMS_OHT_STS_BIT);

    if (bcu_get_dchg_oc_state()  >= kAlarmSecondLevel)
        BCU_ADD_FLAG(flag,BMS_ODC_STS_BIT);
    else
        BCU_REMOVE_FLAG(flag, BMS_ODC_STS_BIT);

    if (bcu_get_low_soc_state() >= kAlarmSecondLevel)
        BCU_ADD_FLAG(flag, BMS_LSOC_STS_BIT);
    else
        BCU_REMOVE_FLAG(flag, BMS_LSOC_STS_BIT);

    if (bcu_get_battery_insulation_state() ==kAlarmFirstLevel)
        BCU_ADD_FLAG(flag,BMS_LEAK_STS_BIT);
    else
        BCU_REMOVE_FLAG(flag, BMS_LEAK_STS_BIT);

    if (bcu_get_battery_insulation_state() >=kAlarmSecondLevel)
        BCU_ADD_FLAG(flag,BMS_ULEAK_STS_BIT);
    else
        BCU_REMOVE_FLAG(flag, BMS_ULEAK_STS_BIT);

    if (bcu_get_slave_communication_state() != kAlarmNone)
        BCU_ADD_FLAG(flag,BMS_COMM_ABORT_STS_BIT);
    else
        BCU_REMOVE_FLAG(flag,BMS_COMM_ABORT_STS_BIT);

    if (bcu_get_high_total_volt_state() >= kAlarmSecondLevel)
        BCU_ADD_FLAG(flag,BMS_OHTV_STS_BIT);
    else
        BCU_REMOVE_FLAG(flag,BMS_OHTV_STS_BIT);

    if (bcu_get_low_total_volt_state() >= kAlarmSecondLevel)
        BCU_ADD_FLAG(flag,BMS_OLTV_STS_BIT);
    else
        BCU_REMOVE_FLAG(flag,BMS_OLTV_STS_BIT);

    if (bcu_get_chg_oc_state()  >= kAlarmSecondLevel)
        BCU_ADD_FLAG(flag,BMS_OCC_STS_BIT);
    else
        BCU_REMOVE_FLAG(flag, BMS_OCC_STS_BIT);

    if (bcu_get_low_temperature_state() >= kAlarmSecondLevel)
        BCU_ADD_FLAG(flag,BMS_OLT_STS_BIT);
    else
        BCU_REMOVE_FLAG(flag, BMS_OLT_STS_BIT);

    if (bcu_get_delta_temperature_state() >= kAlarmSecondLevel)
        BCU_ADD_FLAG(flag,BMS_DIFFT_STS_BIT);
    else
        BCU_REMOVE_FLAG(flag, BMS_DIFFT_STS_BIT);

    if (bcu_get_delta_voltage_state() >= kAlarmSecondLevel)
        BCU_ADD_FLAG(flag,BMS_DIFFV_STS_BIT);
    else
        BCU_REMOVE_FLAG(flag, BMS_DIFFV_STS_BIT);

    return flag;
}
void bcu_update_config(void)
{
    if (bcu_get_config_state() != kPChargeStateRunning) //检测是否需要更新配置参数
        return;

    config_save_all();
    bcu_set_left_cap_interm(CAP_TENFOLD_AH_TO_MAS(config_get(kLeftCapIndex)));//bcu_set_left_cap_interm(CAP_AH_TO_MAS(config_get(kLeftCapIndex))); /*剩余容量：1Ah==>1Ams*/

    bcu_set_config_state(kConfigStateIdle);
    bcu_set_config_changed_flag(1);
}

void bcu_update_soc(void)
{
    soc_refresh();
    soc_empty_calib_check();
}

void bcu_check_third_generation_insulation(void)
{
    if ((bcu_get_insulation_work_state() == kInsulationWorkStateNormal)
			|| (bcu_get_insulation_work_state() == kInsulationWorkStateResOverLow)) //数据有效   2010-7-28 dlw修改
	{
		if (bcu_get_positive_insulation_resistance() > bcu_get_negative_insulation_resistance())
		    bcu_set_system_insulation_resistance(bcu_get_negative_insulation_resistance()); //单位：0.1Kohm
		else
			bcu_set_system_insulation_resistance(bcu_get_positive_insulation_resistance());
	}
	else
	{
		if(bcu_get_insulation_work_state() != kInsulationWorkStateVoltExcept) 
		{
		    bcu_set_insulation_total_voltage(0);
		    BCU_RESET_INSU_EXTEND_VOLT();
		}
		BCU_RESET_INSU_RES();
	  }
	  if(insu_type_is_both())
	  {
	      if ((bcu_get_ext_insulation_work_state() == kInsulationWorkStateNormal)
            || (bcu_get_ext_insulation_work_state() == kInsulationWorkStateResOverLow)) //数据有效   2010-7-28 dlw修改
    	  {
            if (bcu_get_ext_positive_insulation_resistance() > bcu_get_ext_negative_insulation_resistance())
    		        bcu_set_ext_system_insulation_resistance(bcu_get_ext_negative_insulation_resistance()); //单位：0.1Kohm
    		    else
                bcu_set_ext_system_insulation_resistance(bcu_get_ext_positive_insulation_resistance());
        }
    	  else
    	  {
            if(bcu_get_ext_insulation_work_state() != kInsulationWorkStateVoltExcept) 
    		    {
                bcu_set_ext_insulation_total_voltage(0);
                BCU_RESET_EXT_INSU_EXTEND_VOLT();
    		    }
    	      BCU_RESET_EXT_INSU_RES();
    	  }
	}
}

void bcu_check_insulation(void)
{
    if(insu_type_is_none())
    {
        bcu_set_insulation_total_voltage(0);
    	BCU_RESET_INSU_EXTEND_VOLT();
    	BCU_RESET_INSU_RES();
        BCU_RESET_EXT_INSU_EXTEND_VOLT();
        BCU_RESET_EXT_INSU_RES();
    	return;
    }

	bcu_check_third_generation_insulation();
}

void bcu_communication_check_slave(void)
{
    INT8U index;
    INT32U temp = 1;

    if( config_get(kBCUCommModeIndex) == BMS_COMM_SLAVE_AND_BYU_MODE || 
        config_get(kBCUCommModeIndex) == BMS_COMM_BYU_MODE || 
        config_get(kBCUCommModeIndex) == BMS_SELF_CHECK_MODE) return;
    
    for (index = 0; index < config_get(kSlaveNumIndex); index++)
    {
        if (bmu_is_online(index))
        {
            //diagnose_communication_on_slave_online(index);
            g_bcu_communication_abort_num_temp &= (~(temp << index));
        }
        else
        {
            //diagnose_communication_on_slave_offline(index);
            /*从机通信中断   ,初始化所有相关的数据*/
            bmu_reset_offline(index);
            g_bcu_communication_abort_num_temp |= (temp << index);
        }
    }
}

BOOLEAN insu_is_online(void)
{
    if(insu_type_is_on_board()) 
        return TRUE;
    if(config_get(kCommFstAlarmDlyIndex) == 0xFFFF)
        return TRUE;
    if(bcu_get_insulation_tick() == 0)
        return TRUE;
    if(get_interval_by_tick(bcu_get_insulation_tick(), get_tick_count()) <= (INT32U)config_get(kCommFstAlarmDlyIndex)*1000)
        return TRUE;
    else
        return FALSE;
}

InsulationType bcu_get_insulation_type(void)
{
	return (InsulationType)config_get(kInsulationType);
}

BOOLEAN insu_type_is_rs485(void)
{
	if(kInsulationTypeRS485 == bcu_get_insulation_type())
		return TRUE;
	else
		return FALSE;
}

BOOLEAN insu_type_is_can(void)
{
	if(kInsulationTypeCan == bcu_get_insulation_type())
		return TRUE;
	else
		return FALSE;
}

BOOLEAN insu_type_is_on_board(void)
{
	if(kInsulationTypeBoard == bcu_get_insulation_type())
		return TRUE;
	else
		return FALSE;
}

BOOLEAN insu_type_is_both(void)
{
	if(kInsulationTypeBoth == bcu_get_insulation_type())
		return TRUE;
	else
		return FALSE;
}

BOOLEAN insu_type_is_none(void)
{
    if(kInsulationNone == bcu_get_insulation_type())
        return TRUE;
    else
        return FALSE;
}

INT16U bcu_get_bat_num_in_pack(INT16U pos)//pos:高8位：从机号  低8位：从机内电池号
{
    INT8U slave_num, i;
    INT16U bat_num=0;

    if(pos == 0x00)
        return 0;
    slave_num = pos>>8;
    if((slave_num == 0)||
        (slave_num > config_get(kSlaveNumIndex)))
        return 0;

    slave_num -= 1;
    for(i=0; i<slave_num; i++)
        bat_num += bmu_get_voltage_num(i);
    bat_num += pos&0xff;

    return bat_num;
}

INT16U bcu_get_temperature_num_in_pack(INT16U pos)//pos:高8位：从机号  低8位：从机内温度号
{
    INT8U slave_num, i;
    INT16U temp_num=0;

    if(pos == 0x00)
        return 0;
    slave_num = pos>>8;
    if((slave_num == 0)||
        (slave_num > config_get(kSlaveNumIndex)))
        return 0;

    slave_num -= 1;
    for(i=0; i<slave_num; i++)
        temp_num += bmu_get_temperature_num(i);
    temp_num += pos&0xff;

    return temp_num;
}

INT16U bcu_get_voltage_with_num_in_pack(INT16U bat_num)//根据电池在pack中的编号获取对应电池电压
{
    INT8U slave_num, i, temp;
    INT16U bat_cnt=0;

    slave_num = (INT8U)config_get(kSlaveNumIndex);
    for(i=0; i<slave_num; i++)
    {
        temp = (INT8U)bmu_get_voltage_num(i);
        if(bat_cnt + temp >= bat_num)
        {
            temp = bat_num - bat_cnt - 1;
            return bmu_get_voltage_item(i, temp);
        }
        bat_cnt += temp;
    }
    return 0;
}

INT8U bcu_get_temperature_with_num_in_pack(INT16U bat_num)//根据电池在pack中的编号获取对应电池温度
{
    INT8U slave_num, i, temp;
    INT16U bat_cnt=0;

    slave_num = (INT8U)config_get(kSlaveNumIndex);
    for(i=0; i<slave_num; i++)
    {
        temp = (INT8U)bmu_get_temperature_num(i);
        if(bat_cnt + temp >= bat_num)
        {
            temp = bat_num - bat_cnt - 1;
            return bmu_get_temperature_item(i, temp);
        }
        bat_cnt += temp;
    }
    return 0;
}

void bcu_communication_check_insulation(void)
{
    if (!insu_is_online())//绝缘模块在线，判断通信状态
    {
        bcu_set_insulation_online(0);
        BCU_RESET_EXT_INSU_RES();
        BCU_RESET_EXT_INSU_EXTEND_VOLT();
        if(insu_type_is_none())
        {
            bcu_set_insulation_work_state(kInsulationWorkStateNormal);
            bcu_set_ext_insulation_work_state(kInsulationWorkStateNormal);
        }
        else
        {
            if(insu_type_is_can())
            {
                BCU_RESET_INSU_RES();
                BCU_RESET_INSU_EXTEND_VOLT();
                bcu_set_insulation_work_state(kInsulationWorkStateNotConnected);
                bcu_set_insulation_total_voltage(0x00);
                bcu_set_insulation_error(0x00);
            }
            bcu_set_ext_insulation_work_state(kInsulationWorkStateNotConnected);
            bcu_set_ext_insulation_total_voltage(0x00);
            bcu_set_ext_insulation_error(0x00);
        }
    }
}

BOOLEAN pcm_is_online(void)
{
    if(config_get(kCommFstAlarmDlyIndex) == 0xFFFF)
        return TRUE;
    if(bcu_get_power_ctl_mode_tick() == 0)
        return FALSE;
    if(get_interval_by_tick(bcu_get_power_ctl_mode_tick(), get_tick_count()) <= (INT32U)config_get(kCommFstAlarmDlyIndex)*1000)
        return TRUE;
    else
        return FALSE;
}

void bcu_communication_check_pcm(void)
{
    if(!pcm_is_online())
    {
        bcu_set_power_ctl_mode_tick(0);
        bcu_set_pcm_total_voltage(0);
        bcu_set_pcm_tv_rate_of_change(0);
        bcu_set_pcm_communication_state(kAlarmFirstLevel);
    }
    else
        bcu_set_pcm_communication_state(kAlarmNone);
}
#if BMS_SUPPORT_HARDWARE_LTC6803 == 1
void bcu_communication_check_byu_battery_sample_board(void)
{
    if(config_get(kBCUCommModeIndex) == BMS_COMM_SLAVE_MODE) return;
        
    if(byu_all_battery_sample_board_is_online() == FALSE) 
        g_bcu_communication_abort_num_temp |= (1 <<BYU_USE_SLAVE_INDEX);
}
#endif
#if BMS_SUPPORT_HARDWARE_LTC6804 == 1
void bcu_communication_check_bsu_battery_sample_board(void)
{
    INT8U  bmu_num=0,bmu_index=0;
    if(config_get(kBCUCommModeIndex) == BMS_COMM_SLAVE_MODE) return;
    if(get_bsu_sample_once_finish_flag() == FALSE)
    	return;    
    bmu_num = (INT8U) config_get(kSlaveNumIndex); //获取从机数
	for (bmu_index = 0; bmu_index < bmu_num; bmu_index++)
	{ 
	   if(is_bmu_comm_error_use_bsu( bmu_index) == TRUE) 
	   {
	      g_bcu_communication_abort_num_temp |= (1 <<bmu_index);
	   }
	}
    
}
#endif
void bcu_update_communication(void)
{
    g_bcu_communication_abort_num_temp = 0;

    bcu_communication_check_slave();
    
    bcu_communication_check_insulation();
    
    bcu_communication_check_pcm();
#if BMS_SUPPORT_HARDWARE_LTC6803 == 1    
    bcu_communication_check_byu_battery_sample_board();
#endif
#if BMS_SUPPORT_HARDWARE_LTC6804 == 1
    bcu_communication_check_bsu_battery_sample_board();
#endif    
    if(g_bcu_communication_abort_num_temp) bcu_flag_pack_state(1 << PACK_COMM_ALERT_BIT);
    else bcu_unflag_pack_state(1 << PACK_COMM_ALERT_BIT);
    
    bcu_set_com_abort_num(g_bcu_communication_abort_num_temp);
}
////////////////////////////////////////////////////////////////////////
void bcu_ChargerLock_StateUpdate(void* data)
{

    UNUSED(data);
    chargerlock_stateupdate();
} 
/////////////////////////////////////////////////////////////////////////
void bcu_alarm_enable(INT32U flag)
{
    bcu_flag_alarm_check_flag(flag);
    alert_beep_on(flag);
}

void bcu_alarm_disable(INT32U flag)
{
    bcu_unflag_alarm_check_flag(flag);
    alert_beep_off(flag);
}

void bcu_charger_enable(INT32U flag)
{
    charger_control_enable_charger(flag);
}

void bcu_charger_disable(INT32U flag)
{
    charger_control_disable_charger(flag);
}

INT8U temp_to_40_offset(INT8U temp)
{
    if (temp <= 10) return 0;
    if (temp >= 255) return 255;
    temp -= 10;
    return temp;
}

INT8U low_temp_for_display(INT8U value)
{
    return value == 0xFF ? 0 : value;
}

INT16U low_voltage_for_display(INT16U value)
{
    return value == 0xFFFF ? 0 : value;
}


void start_bcu_insu_heart_beat_tick(void)
{
    if(0 == bcu_get_insulation_tick())
        bcu_set_insulation_tick(get_tick_count());
}

void bcu_insu_heart_beat_tick_update(void)
{
    INT32U tick;

    tick = get_tick_count();//tick=0做为特殊数据，不做计数值
    if(tick != 0)
        bcu_set_insulation_tick(tick);
    else
        bcu_set_insulation_tick(1);
}

void start_bcu_pcm_heart_beat_tick(void)
{
    if(0 == bcu_get_power_ctl_mode_tick())
        bcu_set_power_ctl_mode_tick(get_tick_count());
}

void bcu_pcm_heart_beat_tick_update(void)
{
    INT32U tick = get_tick_count();
    
    if(tick != 0)
        bcu_set_power_ctl_mode_tick(tick);
    else
        bcu_set_power_ctl_mode_tick(1);
}

INT32U bcu_get_running_time(void)
{
    return get_tick_count() / 1000;
}
/*
CurrentCheck g_current_check={0};
void bcu_update_current_check(void* data)
{
    INT16U* save_data = (INT16U *)data;
    
    if(((config_get(kCurSenRangeTypeIndex) == kCurrentRangeTypeDouble)||(config_get(kCurSenRangeTypeIndex) == kCurrentRangeTypeTwoSingle)) && 
        abs(save_data[0]) <= CCHK_CUR_OUT_VOLT_MAX_DEF)
        g_current_check.diff0=save_data[0];//config_save(kCchkIndex, save_data[0]);
    if(abs(save_data[1]) <= CCHK_CUR_OUT_VOLT_MAX_DEF)
        g_current_check.diff1=save_data[1];//config_save(kHighCurChkIndex, save_data[1]);
}
CurrentCheck bcu_get_current_check_diff(void)
{
  return g_current_check; 
}

void bcu_check_current_auto_check(void* data)
{
    Result res;
    UNUSED(data);
    
    if(bcu_get_current_auto_check_flag() == 0) return;
    
    res = current_sample_auto_check(bcu_update_current_check);
    if (res != RES_OK)
        return;
    
    bcu_set_current_auto_check_flag(0);
}

static INT8U g_current_check_count=0,g_current_auto_check_flag=0;
void bcu_check_current_auto_to_zero_self(void)
{
   INT16S current = bcu_get_current(); 
   if(g_current_auto_check_flag == 1) return;
   if((get_tick_count() < BMS_CURRENT_AUTO_CHECK_DELAY) && (abs(current) < CURRENT_START_CHECK_TO_ZERO_CURRENT))
    {
      g_current_check_count++;
      if(g_current_check_count > 2) 
      {
        current_sample_auto_check(bcu_update_current_check);
        g_current_auto_check_flag = 1; 
        g_current_check_count = 0; 
      }
    }
} 
void bcu_check_current_auto_to_zero(void* data)
{
    
    UNUSED(data);
    current_sample_auto_to_zero(bcu_update_current_check);
}*/

INT8U bcu_set_custom_information(INT16U index, INT16U value)
{
    OS_CPU_SR cpu_sr = 0;
    
    if(index >= HMI_CUSTOM_INFORMATION_NUM) return RES_FALSE;
    
    OS_ENTER_CRITICAL();
    g_bms_info.custom_information[index] = value;
    OS_EXIT_CRITICAL();
    
    return RES_OK;
}

INT16U bcu_get_custom_information(INT16U index)
{
    INT16U data;
    OS_CPU_SR cpu_sr = 0;
    
    if(index >= HMI_CUSTOM_INFORMATION_NUM) return 0;
    
    OS_ENTER_CRITICAL();
    data = g_bms_info.custom_information[index];
    OS_EXIT_CRITICAL();
    
    return data;
}

INT16U bcu_get_voltage_num(void)
{
    INT16U i, slave_num, volt_num = 0;
    
    slave_num = config_get(kSlaveNumIndex);
    for(i=0; i<slave_num; i++)
        volt_num += bmu_get_voltage_num((BmuId)i);
    
    return volt_num;
}
//获取当前总的电池温感数
INT16U bcu_get_temperature_num(void)
{
    INT16U i, slave_num, temp_num = 0;
    
    slave_num = config_get(kSlaveNumIndex);
    for(i=0; i<slave_num; i++)
        temp_num += bmu_get_temperature_num((BmuId)i);
    
    return temp_num;
}
//获取当前总的加热片温感数
INT16U bcu_get_heat_temperature_num(void)
{
    INT16U i, slave_num, temp_num = 0;
    
    slave_num = config_get(kSlaveNumIndex);
    for(i=0; i<slave_num; i++)
        temp_num += bmu_get_heat_temperature_num((BmuId)i);
    
    return temp_num;
}
//获得放电状态自检故障码
INT8U bcu_get_dchg_selfcheck_fault_num(void)
{
    return g_bcu_power_up_dchg_failure;
}
//获得充电状态自检故障码
INT8U bcu_get_chg_selfcheck_fault_num(void)
{
    return g_bcu_power_up_chg_failure;
}
//放电状态自检检测
void bcu_update_dchg_start_failure_status(void)
{
    if(config_get(kBCUCommModeIndex) == BMS_COMM_SLAVE_MODE)   //连接CAN通信从机
    {
        if(get_tick_count() < BMS_SLEF_CHECK_DELAY)          //延时等待从机第一包有效数据到来
        {
            g_bcu_power_up_dchg_last_tick = get_tick_count(); //保存时间
            return;
        } 
    }
    else                                                    //连接S型从机
    {
        if(get_tick_count() < BMS_SLEF_CHECK_TIMEOUT_DELAY) 
        {
            if(get_bsu_sample_once_finish_flag() == FALSE)  //接收到第一包从机数据无效
            { 
                g_bcu_power_up_dchg_last_tick = get_tick_count(); //保存时间
                return;
            }      
        }
    }
    if(bcu_get_dchg_start_failure_state() == kStartFailureNone) return; //默认为上电错误状态成立，在无错误状态不做任何处理，也不进行上电错误检测
    if(bcu_get_dchg_start_failure_state() == kStartFailureRecovery) return;
    
    if(bcu_get_dchg_start_failure_state() == kStartFailureOccure)
    {
        bms_relay_selfcheck_fault_update(kRelayTypeDischarging, &g_bcu_power_up_dchg_failure);
        if(g_bcu_power_up_dchg_failure == 0) 
        {
            if(get_interval_by_tick(g_bcu_power_up_dchg_last_tick, get_tick_count()) >= BMS_POWER_UP_FAULT_RELEASE_DELAY)
                bcu_set_dchg_start_failure_state(kStartFailureRecovery);
        }
        else
            g_bcu_power_up_dchg_last_tick = get_tick_count();
    }
}
//充电状态自检检测
void bcu_update_chg_start_failure_status(void)
{
   if(config_get(kBCUCommModeIndex) == BMS_COMM_SLAVE_MODE)   //连接CAN通信从机
    {
        if(get_tick_count() < BMS_SLEF_CHECK_DELAY)          //延时等待从机第一包有效数据到来
        {
            g_bcu_power_up_chg_last_tick = get_tick_count(); //保存时间
            return;
        } 
    }
    else                                                    //连接S型从机
    {
        if(get_tick_count() < BMS_SLEF_CHECK_TIMEOUT_DELAY) 
        {
            if(get_bsu_sample_once_finish_flag() == FALSE)  //接收到第一包从机数据无效
            { 
                g_bcu_power_up_chg_last_tick = get_tick_count(); //保存时间
                return;
            }      
        }
    }
    if(bcu_get_chg_start_failure_state() == kStartFailureNone) return; //默认为上电错误状态成立，在无错误状态不做任何处理，也不进行上电错误检测
    if(bcu_get_chg_start_failure_state() == kStartFailureRecovery) return;
    
    if(bcu_get_chg_start_failure_state() == kStartFailureOccure)
    {
        bms_relay_selfcheck_fault_update(kRelayTypeCharging, &g_bcu_power_up_chg_failure);
        if(g_bcu_power_up_chg_failure == 0) 
        {
            if(get_interval_by_tick(g_bcu_power_up_chg_last_tick, get_tick_count()) >= BMS_POWER_UP_FAULT_RELEASE_DELAY)
                bcu_set_chg_start_failure_state(kStartFailureRecovery);
        }
        else
            g_bcu_power_up_chg_last_tick = get_tick_count();
    }
}

INT16U bcu_get_current_total_voltage_num(void) //获取当前实际监控电压串数,受通信中断影响
{
    INT16U total_num = 0;
    INT8U i, slave_num;
    
    slave_num = (INT8U)config_get(kSlaveNumIndex);
    for (i = 0; i < slave_num; i++)
    {
        total_num += bmu_get_voltage_num(i);
    }
    
    return total_num;
}

INT16U bcu_get_current_total_temperature_num(void) //获取当前实际监控温感串数,受通信中断影响
{
    INT16U total_num = 0;
    INT8U i, slave_num;
    
    slave_num = (INT8U)config_get(kSlaveNumIndex);
    for (i = 0; i < slave_num; i++)
    {
        total_num += bmu_get_temperature_num(i);
    }
    
    return total_num;
}

INT8U bcu_set_system_time(DateTime* _PAGED time)
{
    OS_CPU_SR cpu_sr = 0;
    
    if(time == NULL) return 0;
    
    OS_ENTER_CRITICAL();
    bcu_get_bms_info()->system_time = *time;
    OS_EXIT_CRITICAL();
    
    return 1;
}

INT8U bcu_get_system_time(DateTime* _PAGED time)
{
    OS_CPU_SR cpu_sr = 0;
    
    if(time == NULL) return 0;
    
    OS_ENTER_CRITICAL();
    *time = bcu_get_bms_info()->system_time;
    OS_EXIT_CRITICAL();
    
    return 1;
}

INT8U bcu_config_para_cannot_restart_check(void)
{
    INT8U index, relay_num;
    
    if(bcu_get_current_auto_check_flag()) return 1;
    
    relay_num = relay_count();
    for(index = 0; index < relay_num; ++index)
    {
        if(relay_is_force_status(index)) return 1;
    }
    
    return 0;
}

void bcu_temperature_rising_check(INT8U temp, INT32U tick)
{
    INT8U delta_temp;
    
    if(bmu_is_valid_temperature(g_temperature_rising_temp) == 0)
    {
        if(bmu_is_valid_temperature(temp)) 
        {
            g_temperature_rising_temp = temp;
        }
        return;
    }
    
    delta_temp = (INT8U)abs(g_temperature_rising_temp - temp);
    if(delta_temp < BCU_TEMPERATURE_RISING_DIFF_TEMP)
    {
        if(get_interval_by_tick(g_temperature_rising_last_tick, tick) < BCU_TEMPERATURE_RISING_CHECK_INTERVAL)
            return;
    }
    
    g_temperature_rising_delta_temp = delta_temp;
    g_temperature_rising_temp = temp;
    g_temperature_rising_last_tick = tick;
    
    if(delta_temp >= BCU_TEMPERATURE_RISING_DIFF_TEMP) bcu_set_temp_rising_state(kAlarmFirstLevel);
    else bcu_set_temp_rising_state(kAlarmNone);
}

INT16U bcu_get_slave_voltage_num_max(void)
{
    INT16U i, slave_num, volt_num = 0;
    
    slave_num = config_get(kSlaveNumIndex);
    for(i=0; i<slave_num; i++)
    {
        if(volt_num < bmu_get_voltage_num((BmuId)i))
        {
            volt_num = bmu_get_voltage_num((BmuId)i);
        }
    }
    if(volt_num == 0)
    {
        volt_num = bmu_get_max_voltage_num();
    }
    
    return volt_num;
}

INT16U bcu_get_slave_temperature_num_max(void)
{
    INT16U i, slave_num, temp_num = 0;
    
    slave_num = config_get(kSlaveNumIndex);
    for(i=0; i<slave_num; i++)
    {
        if(temp_num < bmu_get_temperature_num((BmuId)i))
        {
            temp_num = bmu_get_temperature_num((BmuId)i);
        }
    }
    if(temp_num == 0)
    {
        temp_num = bmu_get_max_temperature_num();
    }
    
    return temp_num;
}

INT8U bcu_get_comm_exception_slave_num(void) //无通信中断延时
{
    INT8U i, slave_num;
    
    slave_num = (INT8U)config_get(kSlaveNumIndex);
    for(i=0; i<slave_num; i++)
    {
        if(bmu_get_voltage_num((BmuId)i) == 0) return i+1;
    }
    
    return 0;
}

INT16U bcu_get_delta_volt_max_in_slave(void) // 获取所有从机内的压差最大值
{
    INT16U delta = 0, temp;
    INT8U i, slave_num;
    
    slave_num = (INT8U)config_get(kSlaveNumIndex);
    for(i=0; i<slave_num; i++)
    {
        temp = bmu_get_delta_voltage_max(i);
        if(temp > delta) delta = temp;
    }
    
    return delta;
}

INT8U bcu_get_delta_temp_max_in_slave(void) // 获取所有从机内的温差最大值
{
    INT8U delta = 0, temp;
    INT8U i, slave_num;
    
    slave_num = (INT8U)config_get(kSlaveNumIndex);
    for(i=0; i<slave_num; i++)
    {
        temp = bmu_get_delta_temperature_max(i);
        if(temp > delta) delta = temp;
    }
    
    return delta;
}

INT8U bcu_slave_self_check(void) //所有从机信息全部有效性检查
{
    INT8U i;
    INT16U slave_num = 0;
    
    slave_num = config_get(kSlaveNumIndex);
    for(i=0; i<slave_num; i++)
    {
        if(bmu_self_check(i) == 0) return 0;
    }
    return 1;
}

PCSTR bcu_get_device_name_4_display(void)
{
    if(config_get(kBCUCommModeIndex) == BMS_COMM_SLAVE_MODE)
    {
        if(input_signal_is_high_by_name("GSM_ID"))
        {
            return DEVICE_NAME;
        }
        else
        {
            return DEVICE_NAME_WITH_DTU;
        }
    }
    else
    {
        if(input_signal_is_high_by_name("GSM_ID"))
        {
            return DEVICE_BYU_NAME;
        }
        else
        {
            return DEVICE_BYU_NAME_WITH_DTU;
        }
    }
}

void bcu_reset_all_can_channel(void)
{
    config_save(kEmsCanChannelIndex, EMS_CAN_DEV);
    config_save(kSlaveCanChannelIndex, UPPER_COMPUTER_CAN_DEV);
    config_save(kChargerCanChannelIndex, BMS_CHARGER_CAN_DEV);
    //config_save(kDtuCanChannelIndex, DTU_CAN_DEV);
}

HW_VER_NUM hardware_io_revision_get(void)
{
    volatile INT8U flag = 0;
#if BMS_SUPPORT_BY5248D == 0    
    DDRD_DDRD0 = 0; 
    DDRD_DDRD1 = 0; 
    
    flag |= (!PORTD_PD0 << 0);
    flag |= (!PORTD_PD1 << 1);
    
#else 
    DDRD_DDRD0 = 0; 
    DDRD_DDRD1 = 0; 
    DDRE_DDRE3 = 0;
    
    flag |= (!PORTD_PD0 << 0);
    flag |= (!PORTD_PD1 << 1);
    flag |= (!PORTE_PE3 << 2);
    
#endif
    
    return (HW_VER_NUM)flag;   // ver<1.15, A1A0= 11; ver=1.15 A1A0=10; 1.16: A1A0=01, flag = 2
}

void hardware_revision_check(void)
{
/*
    if (hardware_io_revision_get() > DEVICE_SW_SUPPORT_HW_VER)  //硬件版本过高
    {
        bcu_set_io_revision_alarm(1);
        for(;;) 
        {
            wdt_feed_dog();
            alert_long_beep(HARDWARE_CHECK_FAIL_BEEP_TIME);
            sleep(HARDWARE_CHECK_FAIL_BEEP_TIME);
            alert_long_beep(HARDWARE_CHECK_FAIL_BEEP_TIME);
            sleep(HARDWARE_CHECK_FAIL_BEEP_TIME);
            alert_long_beep(HARDWARE_CHECK_FAIL_BEEP_TIME);
            sleep(500);
        }
    }
    else */
    {
        bcu_set_io_revision_alarm(0);
    }
}

Result bcu_start_new_slave_heart_beat(ConfigIndex index, INT16U new_value)
{
    if (index == kSlaveNumIndex)
    {
        INT8U i, old_value;
        
        old_value = (INT8U)config_get(kSlaveNumIndex);
        for (i=old_value; i<new_value; i++)
        {
            bmu_heart_beat_tick_update(i);
        }
    }

    return RES_OK;
}

INT8U bcu_get_high_heat_temperature(void) //获取最高加热温度
{
    INT8U i=0, high_temp=0, slave_num=0, temp=0;
    
    slave_num = (INT8U)config_get(kSlaveNumIndex);
    for(i=0; i<slave_num; i++)
    {
        temp = bmu_get_high_heat_temperature(i);
        if (high_temp < temp) high_temp = temp;
    }
    return high_temp;
}

INT8U bcu_get_low_heat_temperature(void) //获取最低加热温度
{
    INT8U i=0, low_temp=0xFF, slave_num=0, temp=0;
    
    slave_num = (INT8U)config_get(kSlaveNumIndex);
    for(i=0; i<slave_num; i++)
    {
        temp = bmu_get_low_heat_temperature(i);
        if (low_temp > temp) low_temp = temp;
    }
    return low_temp;
}

AlarmLevel bcu_get_chg_oc_state(void) //包含充电过流和回馈过流
{
    AlarmLevel chr_oc, fd_oc;
    
    chr_oc = bcu_get_chgr_oc_state();
    fd_oc = bcu_get_feedback_oc_state();
    
    return chr_oc > fd_oc ? chr_oc : fd_oc;
}

void bcu_set_chg_oc_state(AlarmLevel alarm)
{
    UNUSED(alarm);
    //do nothing
}

#if BMU_CHR_OUTLET_TEMP_SUPPORT

//获取当前总的充电插座温感数
INT16U bcu_get_chr_outlet_temperature_num(void)
{
    INT16U i, slave_num, temp_num = 0;
    
    slave_num = config_get(kSlaveNumIndex);
    for(i=0; i<slave_num; i++)
        temp_num += bmu_get_chr_outlet_temperature_num((BmuId)i);
    
    return temp_num;
}

INT8U bcu_get_high_chr_outlet_temperature(void) //获取最高充电插座温度
{
    INT8U i=0, high_temp=0, slave_num=0, temp=0;
    
    slave_num = (INT8U)config_get(kSlaveNumIndex);
    for(i=0; i<slave_num; i++)
    {
        temp = bmu_get_high_chr_outlet_temperature(i);
        if (high_temp < temp) high_temp = temp;
    }
    return high_temp;
}


#endif

Result bcu_chr_outlet_ht_alarm_event(EventTypeId event_id, void* event_data, void* user_data)
{
    UNUSED(event_id);
    UNUSED(event_data);
    UNUSED(user_data);
    
    return RES_OK;
}
