/**
*
* Copyright (c) 2012 Ligoo Inc.
*
* @file bms_byu.c
* @brief
* @note
* @author
* @date 2014-3-31
*
*/

#include "includes.h"
#include "ADSample.h"
#include "BatBalance.h"
#if BMS_SUPPORT_HARDWARE_LTC6803 == 1 
#pragma MESSAGE DISABLE C12056 // SP debug info incorrect because of optimization or inline assembler

void byu_init(void)
{
    if(config_get(kBCUCommModeIndex) == BMS_COMM_SLAVE_MODE) return;
    
    ADSampleInit();
    BatBalanceInit();
}

INT8U byu_all_battery_sample_board_is_online(void)
{
    INT8U max_addr, ltc_addr = 0;
    
    max_addr = get_ltc_num();
    if(max_addr>MAX_LTC6803_NUM)
        max_addr = MAX_LTC6803_NUM;
    
    for(ltc_addr=0; ltc_addr<max_addr; ltc_addr++)
    {
        if(g_bmsInfo.LtcVHeartBeat[ltc_addr] > LTC_COM_ERR_CNT)
            return FALSE;
    }
    return TRUE;
}

INT8U byu_battery_sample_board_is_online(INT8U ltc_addr)
{
    INT8U max_addr;
    
    max_addr = get_ltc_num();
    if(max_addr>MAX_LTC6803_NUM)
        max_addr = MAX_LTC6803_NUM;
    if(ltc_addr >= max_addr) return FALSE;
    
    if(g_bmsInfo.LtcVHeartBeat[ltc_addr] > LTC_COM_ERR_CNT)
            return FALSE;
    
    return TRUE;
}

void byu_update_bcu_info(void)
{
    INT8U i=0, j, count=0, max_addr=0;
    INT8U bmu_volt_index=0, comm_flag;
    
    // 电压串数更新
    max_addr = get_ltc_num();
    for(i=0; i<max_addr; i++ )
	    count +=g_bmsInfo.batCnt[i];
	bmu_set_voltage_num(BYU_USE_SLAVE_INDEX, count);
	
	// 电压值更新
	bmu_volt_index = 0;
	for(i = 0; i < max_addr; i++)
	{
	    count = g_bmsInfo.batCnt[i];
	    if(count > LTC_CELLV_NUM) count = LTC_CELLV_NUM;
	    
	    comm_flag = byu_battery_sample_board_is_online(i);
	    for(j=0; j<count; j++)
	    {
	        if(comm_flag == TRUE)bmu_set_voltage_item(BYU_USE_SLAVE_INDEX, bmu_volt_index++, g_bmsInfo.volt[i][j]);
	        else bmu_set_voltage_item(BYU_USE_SLAVE_INDEX, bmu_volt_index++, 0);
	    }
	}
	
	// 温度串数更新
	count = 0;
	for(i=0;i<max_addr; i++ )
	    count +=g_bmsInfo.tempCnt[i];
	bmu_set_temperature_num(BYU_USE_SLAVE_INDEX, count);
	
	// 温度值更新
	bmu_volt_index = 0;
	for(i = 0; i < max_addr; i++)
	{
	    count = g_bmsInfo.tempCnt[i];
	    if(count > LTC_CELLT_NUM) count = LTC_CELLT_NUM;
	    
	    comm_flag = byu_battery_sample_board_is_online(i);
	    for(j=0; j<count; j++)
		{
		    if(comm_flag == TRUE)
		    {
  		        if(g_bmsInfo.temp[i][j] == 0)
    		        bmu_set_temperature_item(BYU_USE_SLAVE_INDEX, bmu_volt_index++, 0);
    		    else
    		        bmu_set_temperature_item(BYU_USE_SLAVE_INDEX, bmu_volt_index++, g_bmsInfo.temp[i][j] + 10);
		    }
		    else
		    {
		        bmu_set_temperature_item(BYU_USE_SLAVE_INDEX, bmu_volt_index++, 0);
		    }
		}
	}
}
#endif