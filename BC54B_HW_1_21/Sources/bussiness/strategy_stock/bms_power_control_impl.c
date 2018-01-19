/**
 *
 * Copyright (c) 2012 Ligoo Inc.
 *
 * @file bms_power_control_impl.c
 * @brief
 * @note
 * @author Liwei Dong
 * @date 2015-1-6
 *
 */
#include "bms_power_control_impl.h"


/*************************宏定义***********************/

/** Max discharge continue current*/
/** 持续电流 时间较长 如(30S)*/
#define MAX_DCHG_CONTINUE_CUR_TEMPERATURE_BUFF_MAX  8 
#define MAX_DCHG_CONTINUE_CUR_SOC_BUFF_MAX          6
/** Max discharge current*/
/** 短时电流 时间较短 如(10S)*/
#define MAX_DCHG_CUR_TEMPERATURE_BUFF_MAX           8 
#define MAX_DCHG_CUR_SOC_BUFF_MAX                   6
/** Max charge continue current*/
/** 持续电流 时间较长 如(30S)*/
#define MAX_CHG_CONTINUE_CUR_TEMPERATURE_BUFF_MAX   5
#define MAX_CHG_CONTINUE_CUR_SOC_BUFF_MAX           5
/** Max charge current*/
/** 短时电流 时间较短 如(10S)*/
#define MAX_CHG_CUR_TEMPERATURE_BUFF_MAX            5
#define MAX_CHG_CUR_SOC_BUFF_MAX                    5


/************************* 变量 ***********************/


/** Max discharge continue current*/
/** 持续电流 时间较长 如(30S)*/
const INT8U g_max_dchg_continue_cur_temperature_buff[MAX_DCHG_CONTINUE_CUR_TEMPERATURE_BUFF_MAX] = 
{
TEMPERATURE_FROM_C(-50), TEMPERATURE_FROM_C(-20), TEMPERATURE_FROM_C(-5), TEMPERATURE_FROM_C(15),
TEMPERATURE_FROM_C(35),TEMPERATURE_FROM_C(45),TEMPERATURE_FROM_C(55), TEMPERATURE_FROM_C(120)
};

const INT8U g_max_dchg_continue_cur_soc_buff[MAX_DCHG_CONTINUE_CUR_SOC_BUFF_MAX] = 
{
0, 10, 20, 50, 70, 100
};

const INT16U g_max_dchg_continue_cur_current_tab[MAX_DCHG_CONTINUE_CUR_TEMPERATURE_BUFF_MAX-1][MAX_DCHG_CONTINUE_CUR_SOC_BUFF_MAX-1] = 
{
0, 0, 0, 0, 0,
TOTALCAP_DEF*3, TOTALCAP_DEF*3, TOTALCAP_DEF*3, TOTALCAP_DEF*3, TOTALCAP_DEF*5,
TOTALCAP_DEF*3, TOTALCAP_DEF*3, TOTALCAP_DEF*5, TOTALCAP_DEF*8, TOTALCAP_DEF*8,
TOTALCAP_DEF*5, TOTALCAP_DEF*5, TOTALCAP_DEF*8, TOTALCAP_DEF*8, TOTALCAP_DEF*8,
TOTALCAP_DEF*5, TOTALCAP_DEF*5, TOTALCAP_DEF*8, TOTALCAP_DEF*8, TOTALCAP_DEF*8,
TOTALCAP_DEF*3, TOTALCAP_DEF*3, TOTALCAP_DEF*5, TOTALCAP_DEF*5, TOTALCAP_DEF*5,
0, 0, 0, 0, 0
};

/** Max discharge current*/
/** 短时电流 时间较短 如(10S)*/
const INT8U g_max_dchg_cur_temperature_buff[MAX_DCHG_CUR_TEMPERATURE_BUFF_MAX] = 
{
TEMPERATURE_FROM_C(-50), TEMPERATURE_FROM_C(-20), TEMPERATURE_FROM_C(-5), TEMPERATURE_FROM_C(15),
TEMPERATURE_FROM_C(35),TEMPERATURE_FROM_C(45),TEMPERATURE_FROM_C(55), TEMPERATURE_FROM_C(120)
};

const INT8U g_max_dchg_cur_soc_buff[MAX_DCHG_CUR_SOC_BUFF_MAX] = 
{
0, 10, 20, 50, 70, 100
};

const INT16U g_max_dchg_cur_current_tab[MAX_DCHG_CUR_TEMPERATURE_BUFF_MAX-1][MAX_DCHG_CUR_SOC_BUFF_MAX-1] = 
{
0, 0, 0, 0, 0,
TOTALCAP_DEF*5, TOTALCAP_DEF*5, TOTALCAP_DEF*5, TOTALCAP_DEF*5, TOTALCAP_DEF*8,
TOTALCAP_DEF*5, TOTALCAP_DEF*5, TOTALCAP_DEF*8, TOTALCAP_DEF*10, TOTALCAP_DEF*10,
TOTALCAP_DEF*8, TOTALCAP_DEF*10, TOTALCAP_DEF*15, TOTALCAP_DEF*20, TOTALCAP_DEF*20,
TOTALCAP_DEF*8, TOTALCAP_DEF*10, TOTALCAP_DEF*10, TOTALCAP_DEF*15, TOTALCAP_DEF*15,
TOTALCAP_DEF*5, TOTALCAP_DEF*5, TOTALCAP_DEF*8, TOTALCAP_DEF*8, TOTALCAP_DEF*8,
0, 0, 0, 0, 0
};

/** Max charge continue current*/
/** 持续电流 时间较长 如(30S)*/
const INT8U g_max_chg_continue_cur_temperature_buff[MAX_CHG_CONTINUE_CUR_TEMPERATURE_BUFF_MAX] = 
{
TEMPERATURE_FROM_C(-50), TEMPERATURE_FROM_C(5), TEMPERATURE_FROM_C(15), TEMPERATURE_FROM_C(45), TEMPERATURE_FROM_C(125)
};

const INT8U g_max_chg_continue_cur_soc_buff[MAX_CHG_CONTINUE_CUR_SOC_BUFF_MAX] = 
{
0, 80, 90, 95, 100
};

const INT16U g_max_chg_continue_cur_current_tab[MAX_CHG_CONTINUE_CUR_TEMPERATURE_BUFF_MAX-1][MAX_CHG_CONTINUE_CUR_SOC_BUFF_MAX-1] = 
{
0, 0, 0, 0,
TOTALCAP_DEF*2, TOTALCAP_DEF*1, 0, 0,
TOTALCAP_DEF*5, TOTALCAP_DEF*5, TOTALCAP_DEF*3, 0,
0, 0, 0, 0
};

/** Max charge current*/
/** 短时电流 时间较短 如(10s)*/
const INT8U g_max_chg_cur_temperature_buff[MAX_CHG_CUR_TEMPERATURE_BUFF_MAX] = 
{
TEMPERATURE_FROM_C(-50), TEMPERATURE_FROM_C(5), TEMPERATURE_FROM_C(15), TEMPERATURE_FROM_C(45), TEMPERATURE_FROM_C(125)
};

const INT8U g_max_chg_cur_soc_buff[MAX_CHG_CUR_SOC_BUFF_MAX] = 
{
0, 80, 90, 95, 100
};

const INT16U g_max_chg_cur_current_tab[MAX_CHG_CUR_TEMPERATURE_BUFF_MAX-1][MAX_CHG_CUR_SOC_BUFF_MAX-1] = 
{
0, 0, 0, 0,
TOTALCAP_DEF*5, TOTALCAP_DEF*3, 0, 0,
TOTALCAP_DEF*10, TOTALCAP_DEF*10, TOTALCAP_DEF*8, 0,
0, 0, 0, 0
};
#pragma DATA_SEG __RPAGE_SEG PAGED_RAM_BMU

#pragma DATA_SEG DEFAULT

/** 获取短时放电电流 时间较短 如(10s)*/
INT16U bms_get_discharge_current_max(void)
{
    INT8U soc, temperature, low_temp, soc_index, temperature_index, low_temp_index;
    
    temperature = (INT8U)bcu_get_high_temperature();
    low_temp = TEMP_4_DISPLAY((INT8U)bcu_get_low_temperature());
    soc = SOC_TO_PERCENT(bcu_get_SOC());
    
    temperature_index = (INT8U)bms_get_byte_range_index(g_max_dchg_cur_temperature_buff, MAX_DCHG_CUR_TEMPERATURE_BUFF_MAX, temperature);
    if(temperature_index >= MAX_DCHG_CUR_TEMPERATURE_BUFF_MAX) return 0;
    low_temp_index = (INT8U)bms_get_byte_range_index(g_max_dchg_cur_temperature_buff, MAX_DCHG_CUR_TEMPERATURE_BUFF_MAX, low_temp);
    if(low_temp_index >= MAX_DCHG_CUR_TEMPERATURE_BUFF_MAX) return 0;
    
    soc_index = (INT8U)bms_get_byte_range_index(g_max_dchg_cur_soc_buff, MAX_DCHG_CUR_SOC_BUFF_MAX, soc);
    
    if(soc_index >= MAX_DCHG_CUR_SOC_BUFF_MAX) return 0;
    if(g_max_dchg_cur_current_tab[temperature_index][soc_index] < g_max_dchg_cur_current_tab[low_temp_index][soc_index])
        return g_max_dchg_cur_current_tab[temperature_index][soc_index];
    else
        return g_max_dchg_cur_current_tab[low_temp_index][soc_index];
}
/** 获取持续放电电流 时间较长 如(30s)*/
INT16U bms_get_discharge_continue_current_max(void)
{
    INT8U soc, temperature, low_temp, soc_index, temperature_index, low_temp_index;
    
    temperature = (INT8U)bcu_get_high_temperature();
    low_temp = TEMP_4_DISPLAY((INT8U)bcu_get_low_temperature());
    soc = SOC_TO_PERCENT(bcu_get_SOC());
    
    temperature_index = (INT8U)bms_get_byte_range_index(g_max_dchg_continue_cur_temperature_buff, MAX_DCHG_CONTINUE_CUR_TEMPERATURE_BUFF_MAX, temperature);
    if(temperature_index >= MAX_DCHG_CONTINUE_CUR_TEMPERATURE_BUFF_MAX) return 0;
    low_temp_index = (INT8U)bms_get_byte_range_index(g_max_dchg_continue_cur_temperature_buff, MAX_DCHG_CONTINUE_CUR_TEMPERATURE_BUFF_MAX, low_temp);
    if(low_temp_index >= MAX_DCHG_CONTINUE_CUR_TEMPERATURE_BUFF_MAX) return 0;
    
    soc_index = (INT8U)bms_get_byte_range_index(g_max_dchg_continue_cur_soc_buff, MAX_DCHG_CONTINUE_CUR_SOC_BUFF_MAX, soc);
    
    if(soc_index >= MAX_DCHG_CONTINUE_CUR_SOC_BUFF_MAX) return 0;
    
    if(g_max_dchg_continue_cur_current_tab[temperature_index][soc_index] < g_max_dchg_continue_cur_current_tab[low_temp_index][soc_index])
        return g_max_dchg_continue_cur_current_tab[temperature_index][soc_index];
    else
        return g_max_dchg_continue_cur_current_tab[low_temp_index][soc_index];
}

/** 获取短时充电电流 时间较短 如(10s)*/
INT16U bms_get_charge_current_max(void)
{
    INT8U soc, temperature, low_temp, soc_index, temperature_index, low_temp_index;
    
    temperature = (INT8U)bcu_get_high_temperature();
    low_temp = TEMP_4_DISPLAY((INT8U)bcu_get_low_temperature());
    soc = SOC_TO_PERCENT(bcu_get_SOC());
    
    temperature_index = (INT8U)bms_get_byte_range_index(g_max_chg_cur_temperature_buff, MAX_CHG_CUR_TEMPERATURE_BUFF_MAX, temperature);
    if(temperature_index >= MAX_CHG_CUR_TEMPERATURE_BUFF_MAX) return 0;
    low_temp_index = (INT8U)bms_get_byte_range_index(g_max_chg_cur_temperature_buff, MAX_CHG_CUR_TEMPERATURE_BUFF_MAX, low_temp);
    if(low_temp_index >= MAX_CHG_CUR_TEMPERATURE_BUFF_MAX) return 0;
    
    soc_index = (INT8U)bms_get_byte_range_index(g_max_chg_cur_soc_buff, MAX_CHG_CUR_SOC_BUFF_MAX, soc);
    
    if(soc_index >= MAX_CHG_CUR_SOC_BUFF_MAX) return 0;
    
    if(g_max_chg_cur_current_tab[temperature_index][soc_index] < g_max_chg_cur_current_tab[low_temp_index][soc_index])
        return g_max_chg_cur_current_tab[temperature_index][soc_index];
    else
        return g_max_chg_cur_current_tab[low_temp_index][soc_index];
}

/** 获取持续充电电流 时间较长 如(30S)*/
INT16U bms_get_charge_continue_current_max(void)
{
    INT8U soc, temperature, low_temp, soc_index, temperature_index, low_temp_index;
    
    temperature = (INT8U)bcu_get_high_temperature();
    low_temp = TEMP_4_DISPLAY((INT8U)bcu_get_low_temperature());
    soc = SOC_TO_PERCENT(bcu_get_SOC());
    
    temperature_index = (INT8U)bms_get_byte_range_index(g_max_chg_continue_cur_temperature_buff, MAX_CHG_CONTINUE_CUR_TEMPERATURE_BUFF_MAX, temperature);
    if(temperature_index >= MAX_CHG_CONTINUE_CUR_TEMPERATURE_BUFF_MAX) return 0;
    low_temp_index = (INT8U)bms_get_byte_range_index(g_max_chg_continue_cur_temperature_buff, MAX_CHG_CONTINUE_CUR_TEMPERATURE_BUFF_MAX, low_temp);
    if(low_temp_index >= MAX_CHG_CONTINUE_CUR_TEMPERATURE_BUFF_MAX) return 0;
    
    soc_index = (INT8U)bms_get_byte_range_index(g_max_chg_continue_cur_soc_buff, MAX_CHG_CONTINUE_CUR_SOC_BUFF_MAX, soc);
    
    if(soc_index >= MAX_CHG_CONTINUE_CUR_SOC_BUFF_MAX) return 0;
    
    if(g_max_chg_continue_cur_current_tab[temperature_index][soc_index] < g_max_chg_continue_cur_current_tab[low_temp_index][soc_index])
        return g_max_chg_continue_cur_current_tab[temperature_index][soc_index];
    else
        return g_max_chg_continue_cur_current_tab[low_temp_index][soc_index];
}


