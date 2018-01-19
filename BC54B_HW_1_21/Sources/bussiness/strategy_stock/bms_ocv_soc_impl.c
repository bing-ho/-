/**
 *
 * Copyright (c) 2012 Ligoo Inc.
 *
 * @file bms_ocv_soc_impl.c
 * @brief
 * @note
 * @author Liwei Dong
 * @date 2015-1-6
 *
 */
#include "bms_ocv_soc_impl.h"

#include "bms_ocv_soc_bm.h"

#pragma MESSAGE DISABLE C5919 // Conversion of floating to unsigned integral

#pragma DATA_SEG __RPAGE_SEG PAGED_RAM

const INT8U g_ocv_soc_temperature_buff[OCV_SOC_TEMPERATURE_BUFF_MAX] = 
{
TEMPERATURE_FROM_C(-20), TEMPERATURE_FROM_C(-10), TEMPERATURE_FROM_C(0), TEMPERATURE_FROM_C(25), TEMPERATURE_FROM_C(45), TEMPERATURE_FROM_C(55)
};

const INT8U g_ocv_soc_soc_buff[OCV_SOC_SOC_BUFF_MAX] = 
{
10, 20, 30, 40, 50, 60, 70, 80, 90
};

const INT16U g_ocv_soc_voltage_tab[OCV_SOC_TEMPERATURE_BUFF_MAX][OCV_SOC_SOC_BUFF_MAX] = 
{
2114, 2152, 2183, 2213, 2261, 2333, 2367, 2421, 2498, 
2111, 2149, 2180, 2212, 2259, 2334, 2365, 2424, 2504, 
2111, 2149, 2181, 2212, 2260, 2334, 2361, 2423, 2505, 
2114, 2153, 2184, 2216, 2264, 2332, 2356, 2420, 2503, 
2113, 2151, 2183, 2216, 2262, 2329, 2353, 2421, 2506, 
2112, 2151, 2183, 2215, 2261, 2327, 2355, 2425, 2513
};

static INT8U g_ocv_soc_bms_current_large_flag = 0; // 实时查询当前检测周期内（OCV_SOC_CHECK_TIME_INTERVAL）电池是否进行了充放电
static INT32U g_ocv_soc_check_last_tick = 0;

#pragma DATA_SEG DEFAULT

static DateTime g_ocv_soc_system_time = {0};


INT8U bms_is_ocv_soc_check_timed(void)
{
    INT8U flag = 0, check_flag = 0;
    INT32U last_second, now_second, now_tick;
    
    if(OCV_SOC_IS_CURRENT_ACTIVED()) g_ocv_soc_bms_current_large_flag = 1;
    
    last_second = ((INT32U)config_get(kOcvSocTimeHighIndex) << 16) + config_get(kOcvSocTimeLowIndex);
    
    if(last_second == 0)
    {
        goto OCV2SOCGetAndSaveTimeSeg;
    }
    
    if(g_ocv_soc_check_last_tick == 0)
    {
        if(get_interval_by_tick(0, get_tick_count()) >= OCV_SOC_POWER_UP_CHECK_DLY)
        {
            check_flag = 1;
            
            now_tick = get_tick_count();
            if(now_tick == 0) now_tick = 1;
            g_ocv_soc_check_last_tick = now_tick;
        }
    }
    else if(get_interval_by_tick(g_ocv_soc_check_last_tick, get_tick_count()) >= OCV_SOC_CHECK_TIME_INTERVAL)
    {
        g_ocv_soc_check_last_tick = get_tick_count();
        check_flag = 1;
    }
    
    if(check_flag == 1)    
    {
        bcu_get_system_time(&g_ocv_soc_system_time);
        now_second = clock_to_time_t(&g_ocv_soc_system_time);
        
        if(OCV_SOC_IS_CURRENT_ACTIVED() || 
            g_ocv_soc_bms_current_large_flag == 1 ||
            now_second < last_second)
        {
            g_ocv_soc_bms_current_large_flag = 0;
            goto OCV2SOCSaveTimeSeg;
        }
        
        if(OCV_SOC_GET_TIME_INTERVAL(now_second, last_second) >= OCV_SOC_CHECK_INTERVAL)
        {
            flag = 1;
            goto OCV2SOCSaveTimeSeg;
        }
        else
        {
            return flag;
        }
    } 
    else
        return flag;
    
OCV2SOCGetAndSaveTimeSeg:
    bcu_get_system_time(&g_ocv_soc_system_time);
    now_second = clock_to_time_t(&g_ocv_soc_system_time);
OCV2SOCSaveTimeSeg:
    config_save(kOcvSocTimeHighIndex, now_second >> 16);
    config_save(kOcvSocTimeLowIndex, (INT16U)now_second);
    
    return flag;
}

INT8U bms_ocv_soc_get_soc(INT8U temperature, INT16U voltage)
{
    INT16U temp_index = 0xFFFF, soc_index = 0xFFFF;
#if OCV_SOC_LINEAR_DIFFERENTIAL_EN > 0
    INT8U soc_low, soc_high;
    INT16U volt_low, volt_high;
#endif 
    
    if(!bmu_is_valid_temperature(temperature) || !bmu_is_valid_voltage(voltage)) return 0xFF;
    
    // get temperature nearby index
    temp_index = bms_get_byte_nearby_index(g_ocv_soc_temperature_buff, OCV_SOC_TEMPERATURE_BUFF_MAX, temperature);

    if(temp_index == 0xFFFF) return 0xFF;
    
    // get voltage bearby index
    soc_index = bms_get_word_nearby_index(g_ocv_soc_voltage_tab[temp_index], OCV_SOC_SOC_BUFF_MAX, voltage);
    if(soc_index == 0xFFFF) return 0xFF;

    if(soc_index == 0)
    {
        return g_ocv_soc_soc_buff[0];
    }
    else if(soc_index+1 >= OCV_SOC_SOC_BUFF_MAX)
    {
        return g_ocv_soc_soc_buff[OCV_SOC_SOC_BUFF_MAX-1];
    }
#if OCV_SOC_LINEAR_DIFFERENTIAL_EN > 0
    else
    {
        if(voltage <= g_ocv_soc_voltage_tab[temp_index][soc_index] ||
            soc_index >= OCV_SOC_SOC_BUFF_MAX)
        {
            soc_low = g_ocv_soc_soc_buff[soc_index - 1];
            soc_high = g_ocv_soc_soc_buff[soc_index];
            volt_low = g_ocv_soc_voltage_tab[temp_index][soc_index - 1];
            volt_high = g_ocv_soc_voltage_tab[temp_index][soc_index];
        }
        else
        {
            soc_high = g_ocv_soc_soc_buff[soc_index + 1];
            soc_low = g_ocv_soc_soc_buff[soc_index];
            volt_high = g_ocv_soc_voltage_tab[temp_index][soc_index + 1];
            volt_low = g_ocv_soc_voltage_tab[temp_index][soc_index];
        }
    }
    soc_low = (INT8U)(math_linear_differential_fun(volt_low, soc_low, volt_high, soc_high, voltage) + 0.5);
    
    return soc_low;
#else
    return g_ocv_soc_soc_buff[soc_index];
#endif
}

float bms_ocv_soc_get_soc_float(INT8U temperature, INT16U voltage)
{
    INT16U temp_index = 0xFFFF, soc_index = 0xFFFF;
    INT8U soc_low, soc_high;
    INT16U volt_low, volt_high;
    float soc_linear;
    
    if(!bmu_is_valid_temperature(temperature) || !bmu_is_valid_voltage(voltage)) return -1;
    
    // get temperature nearby index
    temp_index = bms_get_byte_range_index(g_ocv_soc_temperature_buff, OCV_SOC_TEMPERATURE_BUFF_MAX, temperature);

    if(temp_index == 0xFFFF) return -1;
    
    // get voltage bearby index
    soc_index = bms_get_word_nearby_index(g_ocv_soc_voltage_tab[temp_index], OCV_SOC_SOC_BUFF_MAX, voltage);
    if(soc_index == 0xFFFF) return -1;

    if(soc_index == 0)
    {
        return g_ocv_soc_soc_buff[0];
    }
    else if(soc_index+1 >= OCV_SOC_SOC_BUFF_MAX)
    {
        return g_ocv_soc_soc_buff[OCV_SOC_SOC_BUFF_MAX-1];
    }
    else
    {
        if(voltage <= g_ocv_soc_voltage_tab[temp_index][soc_index] ||
            soc_index >= OCV_SOC_SOC_BUFF_MAX)
        {
            soc_low = g_ocv_soc_soc_buff[soc_index - 1];
            soc_high = g_ocv_soc_soc_buff[soc_index];
            volt_low = g_ocv_soc_voltage_tab[temp_index][soc_index - 1];
            volt_high = g_ocv_soc_voltage_tab[temp_index][soc_index];
        }
        else
        {
            soc_high = g_ocv_soc_soc_buff[soc_index + 1];
            soc_low = g_ocv_soc_soc_buff[soc_index];
            volt_high = g_ocv_soc_voltage_tab[temp_index][soc_index + 1];
            volt_low = g_ocv_soc_voltage_tab[temp_index][soc_index];
        }
    }
    
    
    soc_linear = math_linear_differential_fun(volt_low, soc_low, volt_high, soc_high, voltage);
    
    return soc_linear;
}

INT8U bms_ocv_soc_update_soc(void)
{
    INT8U soc_clib, soc_cur;
    INT16U left_cap;
    
    soc_clib = bms_ocv_soc_get_soc(bcu_get_average_temperature(), bcu_get_average_voltage());
    if(soc_clib == 0xFF) return 0;
    soc_cur = SOC_TO_PERCENT(bcu_get_SOC());
    if(abs(soc_clib - soc_cur) <= OCV_SOC_CALIB_DIFF_SOC_MIN) return 0;
    
    if(soc_clib >= soc_cur)
    {
        if(soc_clib - soc_cur > OCV_SOC_SOC_CHANGE_MAX) soc_clib = soc_cur + OCV_SOC_SOC_CHANGE_MAX;
    }
    else
    {
        if(soc_cur - soc_clib > OCV_SOC_SOC_CHANGE_MAX) soc_clib = soc_cur - OCV_SOC_SOC_CHANGE_MAX;
    }
    if(soc_clib > 100) soc_clib = 100;
    
    left_cap = (INT16U)((GAIN(config_get(kTotalCapIndex),soc_clib) + 50) / 100);
    if(left_cap != config_get(kLeftCapIndex)) soc_update_left_cap_by_tenfold_ah(left_cap);//soc_update_left_cap_by_ah(left_cap);
    
    return 1;
}

INT8U bms_ocv_soc_check(void)
{
    if(bms_is_ocv_soc_check_timed())
    {
        update_soc_bm_by_ocv_soc_table();
        // return bms_ocv_soc_update_soc();
    }
    return 0;
}

//total cap check using ocv-soc tab

INT8U g_sohCalibState = 0;
INT32U g_sohCalibTick = 0;
INT32U g_sohCalibRelTick = 0;
INT16U g_sohClibStartSOC = 0;
INT16U g_sohClibStartLeftCap = 0;
INT16U g_sohClibStartLv = 0;
INT16U g_sohClibStopSOC = 0;
INT16U g_sohClibStopLeftCap = 0;
INT16U g_sohClibStopLv = 0;
INT16U g_sohClibTotalCap = 0;
INT8U g_sohClibStartLvID = 0, g_sohClibStartLvIndex = 0; //最低电压编号
INT8U g_sohClibLog = 0;

void bms_soh_check(void)
{
    INT8U log = 0;
    INT16U soc_clib = 0;
    OS_CPU_SR cpu_sr = 0;
    
    if(CHARGER_IS_CONNECTED() == 0) log = 1; //充电机未连接，停止SOC策略
    else if(bcu_get_low_temperature() < g_ocv_soc_temperature_buff[0]) log = 2; //电池最低温度过低，停止SOC策略
    else if(bcu_get_high_temperature() > g_ocv_soc_temperature_buff[OCV_SOC_TEMPERATURE_BUFF_MAX-1]) log = 3; //电池最高温度过高，停止SOC策略
    
    if(log) 
    {
        g_sohCalibState = 0;
        if(g_sohClibLog < 2) g_sohClibLog = log;
        bcu_set_soh_calib_cur_limit_flag(0); //取消小电流充电
        return;
    }
    switch(g_sohCalibState)
    {
        case 0://初始状态
            log = 0;
            if(bcu_is_discharging() != 0) log = 10; //电池处于放点状态，停止SOC策略
            //else if(bcu_get_SOC() >= PERCENT_TO_SOC(SOH_CALIB_START_SOC_MAX)) log = 11; //当前SOC大于起始SOC限制(25%)，停止SOC策略
            else if(bcu_get_low_voltage() == 0 || bcu_get_low_voltage() >= SOH_CALIB_START_LV_MAX) log = 12; //当前最低电压高于起始低压限制，停止SOC策略
            if(log == 0)
            {
                bcu_set_soh_calib_cur_limit_flag(1); //启动小电流充电
                g_sohCalibTick = get_tick_count();
                g_sohCalibState = 1;
            }
            else
            {
                bcu_set_soh_calib_cur_limit_flag(0); //取消小电流充电
                if(g_sohClibLog < 2) g_sohClibLog = log;
            }
        break;
        case 1: //小电流充电
            log = 0;
            if(bcu_is_discharging()) log = 20; //未检测到充电电流(3A)，停止SOC策略
            else if(abs(bcu_get_current()) >= SOH_CALIB_LIMIT_CUR_MAX) log = 21; //小电流充电阶段电流过大(超过5A)，停止SOC策略
            if(log == 0)
            {
                if(get_elapsed_tick_count(g_sohCalibTick) >= SOH_CALIB_LIMIT_CUR_TIME)
                {
                    g_sohCalibTick += SOH_CALIB_LIMIT_CUR_TIME;
                    OS_ENTER_CRITICAL();
                    g_sohClibStartLv = bcu_get_low_voltage();
                    g_sohClibStartLvID = bmu_stat_get_low_voltage_id(0);
                    g_sohClibStartLvIndex = bmu_stat_get_low_voltage_index(0);
                    OS_EXIT_CRITICAL();
                    soc_clib = bms_ocv_soc_get_soc(bcu_get_average_temperature(), g_sohClibStartLv);
                    log = 0;
                    if(g_sohClibStartLvID == 0 || g_sohClibStartLvIndex == 0) log = 23; //起始最低电压编号异常，停止SOC策略
                    else if(soc_clib > SOH_CALIB_START_SOC_MAX) log = 24; //根据当前开路电压查表起始SOC过高，停止SOC策略
                    if(log) 
                    {
                        g_sohCalibState = 0;
                        g_sohClibLog = log;
                    }
                    else
                    {
                        g_sohClibStartSOC = soc_clib;
                        g_sohClibStartLeftCap = CAP_MAS_TO_TENFOLD_AH(bcu_get_left_cap_interm());//CAP_MAS_TO_AH(bcu_get_left_cap_interm());
                        bcu_set_soh_calib_cur_limit_flag(0); //取消小电流充电
                        g_sohCalibTick = get_tick_count();
                        g_sohCalibRelTick = g_sohCalibTick;
                        g_sohClibLog = 30;
                        g_sohCalibState = 2;
                    }
                }
                else
                {
                    g_sohClibLog = 22; //等待小电流持续1min
                }
            }
            else
            {
                g_sohCalibState = 0;
                g_sohClibLog = log;
            }
        break;
        case 2: //正常充电并检查是否进入充电末端
            log = 0;
            if(bcu_get_high_voltage() < config_get(kDChgHVTrdAlarmIndex)-10) log = 31; //出现停止充电，最高单体电压未达到停止正常充电阀值，停止SOC策略
            else if(get_elapsed_tick_count(g_sohCalibTick) < SOH_CALIB_CHG_TIME_MIN) log = 32; //出现停止充电，正常充电时间未达到要求时间，停止SOC策略
            //if(bcu_is_charging() && abs(bcu_get_current()) < SOH_CALIB_LIMIT_CUR_MAX) log = 100;
            //else if(bcu_get_SOC() < PERCENT_TO_SOC(SOH_CALIB_STOP_SOC_MIN)) log = 33; //出现停止充电，但当前SOC过低，停止SOC策略
            else if(bmu_get_voltage_item(g_sohClibStartLvID-1, g_sohClibStartLvIndex-1) < SOH_CALIB_STOP_LV_MIN) log = 34; //出现停止充电，但最低单体电压过低，停止SOC策略
            
            if(bcu_is_charging() == 0) 
            {
                if(log) g_sohClibLog = log; 
                else g_sohClibLog = 35; //正常充电阶段，无充电电流，停止SOC策略
                if(get_elapsed_tick_count(g_sohCalibRelTick) >= 20000)
                {
                    g_sohCalibState = 0;
                }
            }
            else
            {
                g_sohCalibRelTick = get_tick_count();
                if(log == 0)
                {
                    bcu_set_soh_calib_cur_limit_flag(1); //启动小电流充电
                    g_sohCalibTick = get_tick_count();
                    g_sohCalibRelTick = g_sohCalibTick;
                    g_sohCalibState = 3;
                }
            }
        break;
        case 3: //小电流充电
            if(bcu_is_discharging()==0 && abs(bcu_get_current()) < SOH_CALIB_LIMIT_CUR_MAX)
            {
                if(get_elapsed_tick_count(g_sohCalibTick) >= SOH_CALIB_LIMIT_CUR_TIME)
                {
                    g_sohCalibTick += SOH_CALIB_LIMIT_CUR_TIME;
                    g_sohClibStopLv = bmu_get_voltage_item(g_sohClibStartLvID-1, g_sohClibStartLvIndex-1);
                    soc_clib = bms_ocv_soc_get_soc(bcu_get_average_temperature(), g_sohClibStopLv);
                    if(soc_clib < SOH_CALIB_STOP_SOC_MIN) 
                    {
                        g_sohClibLog = 43; //根据当前开路电压查表结束SOC过低，停止SOC策略
                        g_sohCalibState = 0;
                    }
                    else
                    {
                        g_sohClibStopSOC = soc_clib;
                        g_sohClibStopLeftCap = CAP_MAS_TO_TENFOLD_AH(bcu_get_left_cap_interm());//CAP_MAS_TO_AH(bcu_get_left_cap_interm());
                        g_sohCalibTick = get_tick_count();
                        g_sohClibLog = 50;
                        g_sohCalibState = 4;
                    }
                }
                else
                {
                    g_sohClibLog = 42; //等待小电流持续1min
                }
                g_sohCalibRelTick = get_tick_count();
            }
            else
            {
                if(get_elapsed_tick_count(g_sohCalibRelTick) >= 20000)
                {
                    g_sohClibLog = 41; //等待电流降至小电流充电值超时(20S)，停止SOC策略
                    g_sohCalibState = 0;
                }
                else
                {
                    g_sohClibLog = 40; //等待电流降至小电流充电值
                }
                g_sohCalibTick = get_tick_count();
            }
        break;
        case 4: //计算总容量修正值
            if(g_sohClibStopLeftCap > g_sohClibStartLeftCap + SOH_CALIB_VALID_CHG_AH_MIN) //真实充电容量判断
            {
                INT16U diff, total_cap_min, total_cap = config_get(kTotalCapIndex);
                if(g_sohClibStopLeftCap < g_sohClibStartLeftCap + config_get(kNominalCapIndex))
                {
                    soc_clib = (INT16U)DIVISION((g_sohClibStopLeftCap - g_sohClibStartLeftCap)*g_sohClibStopSOC*SOH_CALIB_CHG_FACTOR, g_sohClibStopSOC - g_sohClibStartSOC);
                }
                else
                {
                    soc_clib = config_get(kNominalCapIndex);
                }
                g_sohClibTotalCap = soc_clib;

                if(total_cap>soc_clib)
                {
                    diff = total_cap - soc_clib;
                    total_cap_min = (INT16U)((INT32U)config_get(kNominalCapIndex) * SOC_CALIB_MIN_SOH / 100);
                    if(diff >  TOTAL_CAP_DECREASE_MAX) diff = TOTAL_CAP_DECREASE_MAX;
                    if(total_cap >= total_cap_min + diff) 
                    {
                        total_cap -= diff;
                    }
                    else 
                    {
                        total_cap = total_cap_min;
                        g_sohClibLog = 52; //当前总容量过低，不能再向下减少
                    }
                }
                else if(total_cap < soc_clib)
                {
                    diff = soc_clib - total_cap;
                    if(diff >  TOTAL_CAP_INCREASE_MAX) diff = TOTAL_CAP_INCREASE_MAX;
                    total_cap += diff;
                    if(total_cap > config_get(kNominalCapIndex)) 
                    {
                        total_cap = config_get(kNominalCapIndex);
                        g_sohClibLog = 53; //当前总容量达到额定容量，不能再向上增加
                    }
                }
                else
                {
                    g_sohClibLog = 54; //当前总容量与计算值相同，不变化
                }
                config_save(kTotalCapIndex, total_cap);
                soc_update_left_cap_by_tenfold_ah(total_cap);//soc_update_left_cap_by_ah(total_cap);
                bcu_charger_disable(SELFCHECK_FULL_CHARGE); //停止充电
                g_sohCalibState = 5; 
            }
            else
            {
                g_sohClibLog = 51; //真实充电容量过少(要求至少增加总容量一半)，停止SOC策略
                g_sohCalibState = 0;
            }
        break;
        case 5:
            if(bcu_get_high_voltage() < CHG_HV_FST_ALARM_REL)//config_get(kChgHVFstAlarmRelIndex)
            {
                bcu_set_soh_calib_cur_limit_flag(0); //取消小电流充电
                bcu_charger_enable(SELFCHECK_FULL_CHARGE); 
                g_sohCalibState = 0xFF; 
            }
            else
            {
                g_sohClibLog = 60; //SOC计算完成
            }
        break;
        case 0xFF:
            g_sohClibLog = 70; //本次上电SOC策略更新完成，等待重新上电再次启动次策略
        break;
    }
}
