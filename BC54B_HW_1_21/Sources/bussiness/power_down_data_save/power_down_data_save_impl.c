/**
 *
 * Copyright (c) 2012 Ligoo Inc.
 *
 * @file power_down_data_save_impl.c
 * @brief
 * @note
 * @author kai.lv
 * @date 2015-1-6
 *
 */
#include "power_down_data_save_impl.h"

static INT8U g_power_down_eeeprom_store_flag = 0;
static INT32U g_power_down_eeeprom_store_value = 0;

char power_down_left_cap_check_and_restore(CapAms_t cap);

void power_down_data_save_init(void)
{
    //PPSP_PPSP6 = 0;
    //PIEP_PIEP6 = 1;
}

char power_down_eeeprom_store_data(void)
{
    CapAms_t left_cap_ams;
    
    left_cap_ams = bcu_get_left_cap_interm();
    g_power_down_eeeprom_store_value = left_cap_ams;
    config_save(kCapAmsSaveLowIndex, (INT16U)left_cap_ams);
    config_save(kCapAmsSaveHighIndex, (INT16U)(left_cap_ams >> 16));
    
    return 1;
}
/*
char power_down_rtc_store_data(void)
{
    char res = 0;
    INT8U retry_cnt = 3;
    union
    {
        CapAms_t value;
        INT8U data[4];
    }left_cap_ams;
    
    left_cap_ams.value = bcu_get_left_cap_interm();
    //高低对调
    reverse_buffer(left_cap_ams.data, 4);
    
    do
    {
        res = (char)clock_store_data(left_cap_ams.data, 0, 4);
    }while(res == 0 && retry_cnt--);
    
    return res;
} */

char power_down_store_data(void)
{
    char res = 0;
    
    res |= power_down_eeeprom_store_data();
    
    //res |= power_down_rtc_store_data();
    
    return res;
}

char power_down_eeeprom_restore_left_cap(void)
{
    char res = 0;
    union
    {
        CapAms_t value;
        INT8U data[4];
    }left_cap_ams;
    OS_CPU_SR cpu_sr = 0;
    
    left_cap_ams.value = 0;
    OS_ENTER_CRITICAL();
    left_cap_ams.value = config_get(kCapAmsSaveLowIndex);
    left_cap_ams.value |= (INT32U)config_get(kCapAmsSaveHighIndex) << 16;
    OS_EXIT_CRITICAL();
    res = power_down_left_cap_check_and_restore(left_cap_ams.value);
    
    config_save(kCapAmsSaveLowIndex, 0); //清空保存值
    config_save(kCapAmsSaveHighIndex, 0);
    
    return res;
}
/*
char power_down_sd2405_restore_left_cap(void)
{
    char res = 0;
    union
    {
        CapAms_t value;
        INT8U data[4];
    }left_cap_ams;
    if (RES_OK != clock_aquire_resource(1000)) {
        return 0;
    }
    res = (char)clock_restore_data(left_cap_ams.data, 0, 4);
    clock_release_resource();
    if(res == 0) return 0;
    
    //高低对调
    reverse_buffer(left_cap_ams.data, 4);
    res = power_down_left_cap_check_and_restore(left_cap_ams.value);
    
    left_cap_ams.value = 0;
    clock_store_data(left_cap_ams.data, 0, 4); //清空保存值
    
    return res;
} */

char power_down_restore_left_cap(void)
{
    char res = 0;
    
    //res |= power_down_sd2405_restore_left_cap();
    
    //if(res == 0) 
    res |= power_down_eeeprom_restore_left_cap();
    
    return res;
}

char power_down_left_cap_check_and_restore(CapAms_t cap)
{
    char res = 0;
    INT16U temp, left_cap,m_cap;
    
    temp = CAP_MAS_TO_TENFOLD_AH(cap);//temp = CAP_MAS_TO_AH(cap);
    left_cap = config_get(kLeftCapIndex);
    m_cap = (INT16U)DIVISION(config_get(kTotalCapIndex),1000);
    if(abs(temp - left_cap) <= m_cap) //数据有效
    {
        res = 1;
        soc_update_tenfold_left_cap_by_ams(cap);
    }
    else
    {
        res = 0;
    }
    return res;
}

//保存5秒后仍未断电，置上次保存值无效
void power_down_eeeprom_store_valid_check(void)
{
    INT32U current_cap, diff_cap;
    
    current_cap = bcu_get_left_cap_interm();
    if(current_cap >= g_power_down_eeeprom_store_value)
    {
        diff_cap = current_cap - g_power_down_eeeprom_store_value;
    }
    else
    {
        diff_cap = g_power_down_eeeprom_store_value - current_cap;
    }
    
    if(diff_cap >= POWER_DOWN_EEEPROM_STORE_CAP_DIFF_MAX) //大于最大差值
    {
        g_power_down_eeeprom_store_flag = 0;
        config_save(kCapAmsSaveLowIndex, 0); //清空保存值
        config_save(kCapAmsSaveHighIndex, 0);
    }
}
/*
void port_p_isr_handler(void)
{
    if(PIFP_PIFP6 == 1)
    {
        if(g_power_down_eeeprom_store_flag == 0)  //禁止持续保存
        {
            g_power_down_eeeprom_store_flag = 1;
            power_down_eeeprom_store_data();
        }
        PIFP = 1 << 6;
    }
}*/