#define BMU_VAR_FUNC_IMPL
#include "bms_bmu.h"
#include "bms_bsu.h"

#pragma MESSAGE DISABLE C1420 // Result of function-call is ignored
#pragma MESSAGE DISABLE C1860 // Pointer conversion: possible loss of data
#pragma MESSAGE DISABLE C2705 // Possible loss of data
#pragma MESSAGE DISABLE C4000 // Condition always TRUE


#pragma DATA_SEG __RPAGE_SEG PAGED_RAM_BMU
BmuInfo g_bmu_infos[BMU_MAX_SLAVE_COUNT];

INT16U g_bmu_voltage_items[BMU_MAX_TOTAL_VOLTAGE_NUM] = {0};
INT8U g_bmu_temp_items[BMU_MAX_TOTAL_TEMP_NUM] = {0};
INT8U g_bmu_voltage_max_num = BMS_BMU_DEFAULT_VOLTAGE_NUM;
INT8U g_bmu_temp_max_num = BMS_BMU_DEFAULT_TEMPERATURE_NUM;

//电池最高最低统计信息
BmuTemperatureItem g_bmu_stat_high_temperatures[BMU_STAT_HIGH_TEMPERATURE_NUM];
INT8U g_bmu_stat_high_temperature_count = 0;

BmuTemperatureItem g_bmu_stat_low_temperatures[BMU_STAT_LOW_TEMPERATURE_NUM];
INT8U g_bmu_stat_low_temperature_count = 0;

BmuVoltageItem g_bmu_stat_high_voltages[BMU_STAT_HIGH_VOLTAGE_NUM];
INT8U g_bmu_stat_high_voltage_count = 0;

BmuVoltageItem g_bmu_stat_low_voltages[BMU_STAT_LOW_VOLTAGE_NUM];
INT8U g_bmu_stat_low_voltage_count = 0;
//电池最高最低统计信息临时缓存
BmuTemperatureItem g_bmu_stat_temp_high_temperatures[BMU_STAT_HIGH_TEMPERATURE_NUM];
INT8U g_bmu_stat_temp_high_temperature_count = 0;

BmuTemperatureItem g_bmu_stat_temp_low_temperatures[BMU_STAT_LOW_TEMPERATURE_NUM];
INT8U g_bmu_stat_temp_low_temperature_count = 0;

BmuVoltageItem g_bmu_stat_temp_high_voltages[BMU_STAT_HIGH_VOLTAGE_NUM];
INT8U g_bmu_stat_temp_high_voltage_count = 0;

BmuVoltageItem g_bmu_stat_temp_low_voltages[BMU_STAT_LOW_VOLTAGE_NUM];
INT8U g_bmu_stat_temp_low_voltage_count = 0;

#define MAX_TEMP    255
#define MIN_TEMP    0
#define MAX_VOLT    0xFFFF
#define MIN_VOLT    0

#define TEMP_ITEMS(SLAVE, INDEX) g_bmu_temp_items[(INT16U)SLAVE * g_bmu_temp_max_num + INDEX]
#define VOLT_ITEMS(SLAVE, INDEX) g_bmu_voltage_items[(INT16U)SLAVE * g_bmu_voltage_max_num + INDEX]
//#define GET_VOLTAGE_ITEM(SLAVE, INDEX)

#define BMU_HEAT_TEMP_NUM_MAX   1
const INT8U g_bmu_heat_temp_pos[BMU_MAX_SLAVE_COUNT][BMU_HEAT_TEMP_NUM_MAX] =  //实际位置，此处位置值必须不大于从机最大串数
{
0
};

#if BMU_CHR_OUTLET_TEMP_SUPPORT
#define BMU_CHR_OUTLET_TEMP_NUM_MAX     1
const  INT8U g_bmu_chr_outlet_temp_pos[BMU_MAX_SLAVE_COUNT][BMU_CHR_OUTLET_TEMP_NUM_MAX] =  //实际位置，此处位置值必须不大于从机最大串数
{
0
};
#endif

#pragma DATA_SEG DEFAULT

#if BCU_CONTROL_SLAVE_RELAY_INTERVAL_EN //从机继电器闭合顺序主机控制
SlaveControlContext g_slaveCtlCxt;

void bmu_relay_control_test(void);
#endif

void bmu_init(void)
{
    g_bmu_voltage_max_num = config_get(kBmuMaxVoltageNumIndex);
    if(g_bmu_voltage_max_num > BMS_BMU_BATTERY_MAX_NUM) g_bmu_voltage_max_num = BMS_BMU_BATTERY_MAX_NUM;
    g_bmu_temp_max_num = config_get(kBmuMaxTempNumIndex);
    if(g_bmu_temp_max_num > BMS_BMU_TEMPERATURE_MAX_NUM) g_bmu_temp_max_num = BMS_BMU_TEMPERATURE_MAX_NUM;
    
#if BCU_CONTROL_SLAVE_RELAY_INTERVAL_EN    
    g_slaveCtlCxt.cool_ctl_list = list_init();
    g_slaveCtlCxt.heat_ctl_list = list_init();
    job_schedule(MAIN_JOB_GROUP, SLAVE_RELAY_CONTROL_CHECK_PERIODIC, bmu_cool_control_check, NULL);
    job_schedule(MAIN_JOB_GROUP, SLAVE_RELAY_CONTROL_CHECK_PERIODIC, bmu_heat_control_check, NULL);
    
#endif
}

void bmu_update(void)
{
    bmu_stat_refersh();
}

INT8U bmu_get_max_voltage_num(void)
{
    INT8U value;
    OS_INIT_CRITICAL();
    RPAGE_INIT();
    
    RPAGE_SAVE();
    
    OS_ENTER_CRITICAL();
    value = g_bmu_voltage_max_num;
    OS_EXIT_CRITICAL();
    
    RPAGE_RESTORE();
    
    return value;
}

INT8U bmu_get_max_temperature_num(void)
{
    INT8U value;
    OS_INIT_CRITICAL();
    RPAGE_INIT();
    
    RPAGE_SAVE();
    
    OS_ENTER_CRITICAL();
    value = g_bmu_temp_max_num;
    OS_EXIT_CRITICAL();
    
    RPAGE_RESTORE();
    
    return value;
}

BmuInfo* _PAGED bmu_get_info(BmuId id)
{
    return &(g_bmu_infos[id]);
}

INT8U bmu_set_temperature_num(BmuId id, INT8U num)
{
    OS_INIT_CRITICAL();
    RPAGE_INIT();

    if (id >= BMU_MAX_SLAVE_COUNT) return RES_FALSE;

    RPAGE_SAVE();

    if (num > g_bmu_temp_max_num) num = g_bmu_temp_max_num;

    OS_ENTER_CRITICAL();
    if(g_bmu_infos[id].temperature_num != num)
    {
        g_bmu_infos[id].temperature_num = num;
    }
    OS_EXIT_CRITICAL();

    RPAGE_RESTORE();

    return RES_TRUE;
}

INT8U bmu_get_temperature_num(BmuId id)
{
    INT8U value;
    OS_INIT_CRITICAL();
    RPAGE_INIT();

    if (id >= BMU_MAX_SLAVE_COUNT) return 0;

    RPAGE_SAVE();
    OS_ENTER_CRITICAL();
    value = g_bmu_infos[id].temperature_num;
    if(value == BMU_TMEPERATURE_INVALID_VALUE)
        value = (INT8U)SLAVE_TEMPERA_NUM_DEF;//config_get(kTemperatureNumIndex);
    if(value > g_bmu_temp_max_num)
        value = g_bmu_temp_max_num;
    OS_EXIT_CRITICAL();
    RPAGE_RESTORE();

    return value;
}

INT8U bmu_set_voltage_num(BmuId id, INT16U num)
{
    OS_INIT_CRITICAL();
    RPAGE_INIT();

    if (id >= BMU_MAX_SLAVE_COUNT) return 0;

    RPAGE_SAVE();

    if (num > g_bmu_voltage_max_num) num = g_bmu_voltage_max_num;

    OS_ENTER_CRITICAL();
    g_bmu_infos[id].voltage_num = num;
    OS_EXIT_CRITICAL();

    RPAGE_RESTORE();

    return RES_TRUE;
}

INT16U bmu_get_voltage_num(BmuId id)
{
    INT16U value;
    OS_INIT_CRITICAL();
    RPAGE_INIT();

    if(id >= BMU_MAX_SLAVE_COUNT) return 0;

    RPAGE_SAVE();
    OS_ENTER_CRITICAL();
    value = g_bmu_infos[id].voltage_num;
    OS_EXIT_CRITICAL();
    RPAGE_RESTORE();

    return value;
}

INT16U bmu_get_voltage_item(BmuId id, INT16U index)
{
    INT16U value = 0;
    INT16U offset;

    OS_INIT_CRITICAL();
    RPAGE_INIT();

    RPAGE_SAVE();

    if (id < BMU_MAX_SLAVE_COUNT && index < g_bmu_voltage_max_num)
    {
        offset = (INT16U)id * g_bmu_voltage_max_num + index;
        if (offset < BMU_MAX_TOTAL_VOLTAGE_NUM)
        {
            OS_ENTER_CRITICAL();
            value = g_bmu_voltage_items[offset];
            OS_EXIT_CRITICAL();
        }
    }

    RPAGE_RESTORE();

    return value;
}

void bmu_set_voltage_item(BmuId id, INT8U index, INT16U value)
{
    INT16U offset;

    OS_INIT_CRITICAL();
    RPAGE_INIT();

    RPAGE_SAVE();

    if (id < BMU_MAX_SLAVE_COUNT && index < g_bmu_voltage_max_num)
    {
        offset = (INT16U)id * g_bmu_voltage_max_num + index;
        if (offset < BMU_MAX_TOTAL_VOLTAGE_NUM)
        {
            OS_ENTER_CRITICAL();
            /*if (!bmu_is_valid_voltage(value))
            {
                value = BATTERY_VOLTAGE_INVALID_VALUE;
            }*/
            g_bmu_voltage_items[offset] = value;
            OS_EXIT_CRITICAL();
        }
    }

    RPAGE_RESTORE();
}

INT8U bmu_get_temperature_item(BmuId id, INT8U index)
{
    INT8U value = 0;
    INT16U offset;
    OS_CPU_SR cpu_sr = 0;
    RPAGE_INIT();
    RPAGE_SAVE();

    if (id < BMU_MAX_SLAVE_COUNT && index < g_bmu_temp_max_num)
    {
        offset = (INT16U)id * g_bmu_temp_max_num + index;
        if (offset < BMU_MAX_TOTAL_TEMP_NUM)
        {
            OS_ENTER_CRITICAL();
            value = g_bmu_temp_items[offset];
            OS_EXIT_CRITICAL();
        }
    }

    RPAGE_RESTORE();

    return value;
}

void bmu_set_balance_state(BmuId id, INT8U group, INT8U value)
{
    OS_INIT_CRITICAL();
    RPAGE_INIT();

    if (id >= BMU_MAX_SLAVE_COUNT || group >= BMS_BMU_BALANCE_STATE_BYTE_NUM) return;

    RPAGE_SAVE();
    OS_ENTER_CRITICAL();

    g_bmu_infos[id].balance_states[group] = value;

    OS_EXIT_CRITICAL();
    RPAGE_RESTORE();
}

INT8U bmu_get_balance_state(BmuId id, INT8U group)
{
    INT8U value;
    OS_INIT_CRITICAL();
    RPAGE_INIT();

    if (id >= BMU_MAX_SLAVE_COUNT || group >= BMS_BMU_BALANCE_STATE_BYTE_NUM) return 0;

    RPAGE_SAVE();
    OS_ENTER_CRITICAL();

    value = g_bmu_infos[id].balance_states[group];

    OS_EXIT_CRITICAL();
    RPAGE_RESTORE();

    return value;
}

void bmu_set_heat_status(BmuId id, INT8U flag)
{
    OS_CPU_SR cpu_sr = 0;
    RPAGE_INIT();

    RPAGE_SAVE();
    OS_ENTER_CRITICAL();
    if(flag)
        bmu_set_pack_state(id, bmu_get_pack_state(id)|(1<<BMU_PACK_HEAT_STATUS_BIT));
    else
        bmu_set_pack_state(id, bmu_get_pack_state(id)&(~(1<<BMU_PACK_HEAT_STATUS_BIT)));
    OS_EXIT_CRITICAL();
    RPAGE_RESTORE();
}

INT8U bmu_get_heat_status(BmuId id)
{
    return (bmu_get_pack_state(id)>>BMU_PACK_HEAT_STATUS_BIT)&0x01;
}

void bmu_set_cool_status(BmuId id, INT8U flag)
{
    OS_CPU_SR cpu_sr = 0;
    RPAGE_INIT();

    RPAGE_SAVE();
    OS_ENTER_CRITICAL();
    if(flag)
        bmu_set_pack_state(id, bmu_get_pack_state(id)|(1<<BMU_PACK_COOL_STATUS_BIT));
    else
        bmu_set_pack_state(id, bmu_get_pack_state(id)&(~(1<<BMU_PACK_COOL_STATUS_BIT)));
    OS_EXIT_CRITICAL();
    RPAGE_RESTORE();
}

INT8U bmu_get_cool_status(BmuId id)
{
    return (bmu_get_pack_state(id)>>BMU_PACK_COOL_STATUS_BIT)&0x01;
}

void bmu_set_temperature_item(BmuId id, INT8U index, INT8U value)
{
    INT16U offset;
    OS_CPU_SR cpu_sr = 0;
    RPAGE_INIT();
    RPAGE_SAVE();

    if (id < BMU_MAX_SLAVE_COUNT && index < g_bmu_temp_max_num)
    {
        offset = (INT16U)id * g_bmu_temp_max_num + index;
        if (offset < BMU_MAX_TOTAL_TEMP_NUM)
        {
            OS_ENTER_CRITICAL();
            /*if (!bmu_is_valid_temperature(value))
            {
                value = BATTERY_TEMPERATURE_INVALID_VALUE;
            }*/
            g_bmu_temp_items[offset] = value;
            OS_EXIT_CRITICAL();
        }
    }

    RPAGE_RESTORE();
}

INT16U bmu_get_balance_current_item(BmuId id, INT8U index)
{
    INT16U value;
    OS_CPU_SR cpu_sr = 0;
    RPAGE_INIT();

    if (id >= BMU_MAX_SLAVE_COUNT || index >= BMS_BMU_BALANCE_NUM) return 0;

    RPAGE_SAVE();
    OS_ENTER_CRITICAL();
    value = g_bmu_infos[id].balance_current[index];
    OS_EXIT_CRITICAL();
    RPAGE_RESTORE();

    return value;
}

void bmu_set_balance_current_item(BmuId id, INT8U index, INT16U value)
{
    OS_CPU_SR cpu_sr = 0;
    RPAGE_INIT();

    if (id >= BMU_MAX_SLAVE_COUNT || index >= BMS_BMU_BALANCE_NUM) return;

    RPAGE_SAVE();
    OS_ENTER_CRITICAL();
    g_bmu_infos[id].balance_current[index] = value;
    OS_EXIT_CRITICAL();
    RPAGE_RESTORE();
}

INT8U bmu_balance_is_on(INT8U id)
{
    INT8U i, balance_flag = 0;
    
    if (id >= BMU_MAX_SLAVE_COUNT) return 0;
    
    for(i=0; i<BMS_BMU_BALANCE_STATE_BYTE_NUM; i++)
    {
        if (g_bmu_infos[id].balance_states[i] != 0)
        {
           balance_flag = TRUE;
           break;
        }
    }
    return balance_flag;
}

Result bmu_is_balance_state_actived(void)
{
    OS_CPU_SR cpu_sr = 0;
    INT8U index, slave_num;
    INT8U res = RES_FALSE;
    RPAGE_INIT();

    RPAGE_SAVE();
    slave_num = config_get(kSlaveNumIndex);
    for (index = 0; index < slave_num; ++index)
    {
        if(bmu_balance_is_on(index) == TRUE)
        {
            res = RES_TRUE;
            break;
        }
    }
    RPAGE_RESTORE();

    return res;
}

void bmu_stat_refersh(void)
{
    bmu_stat_refersh_voltages();
    bmu_stat_refersh_temperatures();
}

void bmu_stat_refersh_voltages(void)
{
    BmuId slave_index;
    INT8U index;
    INT16U slave_num, items_num;
    INT16U max_voltage = MIN_VOLT, min_voltage = MAX_VOLT;
    INT32U total_voltage = 0;
    INT16U voltage_count = 0, average_voltage = 0;
    INT16U max_voltage_id = 0, min_voltage_id = 0;
    INT16U voltage;
    INT8U voltage_exception = 0;
    INT16U offset = 0;
    OS_INIT_CRITICAL();

    slave_num = config_get(kSlaveNumIndex);

    g_bmu_stat_temp_high_voltage_count = 0;
    g_bmu_stat_temp_low_voltage_count = 0;

    for (slave_index = 0; slave_index < slave_num; slave_index++, offset += g_bmu_voltage_max_num)
    {
        items_num = g_bmu_infos[slave_index].voltage_num;
        for (index = 0; index < items_num; index++) //最高、最低电压扫描,总压计算
        {
            OS_ENTER_CRITICAL();
            voltage = g_bmu_voltage_items[offset + index];
            OS_EXIT_CRITICAL();
            if (!bmu_is_valid_voltage(voltage))
            {
                voltage_exception = 1;
                continue;
            }

            ++voltage_count;
            total_voltage += voltage;

            if (max_voltage < voltage)
            {
                max_voltage = voltage;
                max_voltage_id = (((INT16U)slave_index + 1) << 8) + (index + 1);
            }

            if (min_voltage > voltage)
            {
                min_voltage = voltage;
                min_voltage_id = (((INT16U)slave_index + 1) << 8) + (index + 1);
            }

            bmu_stat_high_voltage(slave_index, index);
            bmu_stat_low_voltage(slave_index, index);
        }
    }

    for(index=g_bmu_stat_temp_high_voltage_count; index<BMU_STAT_HIGH_VOLTAGE_NUM; index++)
    {
        safe_memset(&g_bmu_stat_temp_high_voltages[index], 0, sizeof(BmuVoltageItem));
    }
    
    for(index=g_bmu_stat_temp_low_voltage_count; index<BMU_STAT_LOW_VOLTAGE_NUM; index++)
    {
        safe_memset(&g_bmu_stat_temp_low_voltages[index], 0, sizeof(BmuVoltageItem));
    }
    
    OS_ENTER_CRITICAL();
    g_bmu_stat_high_voltage_count = g_bmu_stat_temp_high_voltage_count;
    for(index = 0; index<BMU_STAT_HIGH_VOLTAGE_NUM; index++)
    {
        g_bmu_stat_high_voltages[index] = g_bmu_stat_temp_high_voltages[index];
    }
    
    g_bmu_stat_low_voltage_count = g_bmu_stat_temp_low_voltage_count;
    for(index = 0; index<BMU_STAT_LOW_VOLTAGE_NUM; index++)
    {
        g_bmu_stat_low_voltages[index] = g_bmu_stat_temp_low_voltages[index];
    }
    
    OS_EXIT_CRITICAL();
    
    if (voltage_count > 0) average_voltage = (INT16U)(total_voltage / voltage_count);

    /** update the BCU information */
    bcu_set_total_voltage_num(voltage_count);
    bcu_set_total_voltage(total_voltage); // 1mV
    bcu_set_average_voltage(average_voltage);

#if 0
    if(max_voltage == MIN_VOLT)
    {
        bcu_set_high_voltage(0);
        bcu_set_high_voltage_id(0);
    }
    else
    {
        bcu_set_high_voltage(max_voltage);
        bcu_set_high_voltage_id(max_voltage_id);
    }
    if(min_voltage == MAX_VOLT)
    {
        bcu_set_low_voltage(0);
        bcu_set_low_voltage_id(0);
    }
    else
    {
        bcu_set_low_voltage(min_voltage);
        bcu_set_low_voltage_id(min_voltage_id);
    }
#else
    bcu_set_high_voltage(max_voltage);
    bcu_set_high_voltage_id(max_voltage_id);
    bcu_set_low_voltage(min_voltage);
    bcu_set_low_voltage_id(min_voltage_id);
#endif

    bcu_set_voltage_exception(voltage_exception);
}

void bmu_stat_refersh_temperatures(void)
{
    INT8U slave_index, index;
    INT16U slave_num;
    INT8U max_temperature = MIN_TEMP, min_temperature = MAX_TEMP;
    INT16U max_temperature_id = 0, min_temperature_id = 0;
    INT8U temperature;
    INT8U temp_exception = 0;
    INT8U items_num = 0;
    INT16U temperature_num = 0;
    INT16U total_temperature = 0;
    INT16U offset = 0;
    OS_INIT_CRITICAL();

    slave_num = config_get(kSlaveNumIndex);

    g_bmu_stat_temp_high_temperature_count = 0;
    g_bmu_stat_temp_low_temperature_count = 0;

    for (slave_index = 0; slave_index < slave_num; slave_index++, offset += g_bmu_temp_max_num)
    {
        items_num = bmu_get_temperature_num(slave_index);

        for (index = 0; index < items_num; index++) //最高、最低电压扫描,总压计算
        {
            OS_ENTER_CRITICAL();
            temperature = g_bmu_temp_items[offset + index];
            OS_EXIT_CRITICAL();
            if (!bmu_is_valid_temperature(temperature))
            {
                temp_exception = 1;
                continue;
            }

            ++temperature_num;
            total_temperature += temperature;

            if (max_temperature < temperature)
            {
                max_temperature = temperature;
                max_temperature_id = (((INT16U)slave_index + 1) << 8) + (index + 1);
            }

            if (min_temperature > temperature)
            {
                min_temperature = temperature;
                min_temperature_id = (((INT16U)slave_index + 1) << 8) + (index + 1);
            }

            bmu_stat_high_temperature(slave_index, index);
            bmu_stat_low_temperature(slave_index, index);
        }
    }
    for(index=g_bmu_stat_temp_high_temperature_count; index<BMU_STAT_HIGH_TEMPERATURE_NUM; index++)
    {
        safe_memset(&g_bmu_stat_temp_high_temperatures[index], 0, sizeof(BmuTemperatureItem));
    }
    
    for(index=g_bmu_stat_temp_low_temperature_count; index<BMU_STAT_LOW_TEMPERATURE_NUM; index++)
    {
        safe_memset(&g_bmu_stat_temp_low_temperatures[index], 0, sizeof(BmuTemperatureItem));
    }
    
    OS_ENTER_CRITICAL();
    g_bmu_stat_high_temperature_count = g_bmu_stat_temp_high_temperature_count;
    for(index = 0; index<BMU_STAT_HIGH_TEMPERATURE_NUM; index++)
    {
        g_bmu_stat_high_temperatures[index] = g_bmu_stat_temp_high_temperatures[index];
    }
    
    g_bmu_stat_low_temperature_count = g_bmu_stat_temp_low_temperature_count;
    for(index = 0; index<BMU_STAT_LOW_TEMPERATURE_NUM; index++)
    {
        g_bmu_stat_low_temperatures[index] = g_bmu_stat_temp_low_temperatures[index];
    }
    
    OS_EXIT_CRITICAL();
    
    /** update the bcu information */
#if 1
    bcu_set_high_temperature(max_temperature);
    bcu_set_high_temperature_id(max_temperature_id);
    bcu_set_low_temperature(min_temperature);
    bcu_set_low_temperature_id(min_temperature_id);
#else
    if(max_temperature == MIN_TEMP)
    {
        bcu_set_high_temperature(0);
        bcu_set_high_temperature_id(0);
    }
    else
    {
        bcu_set_high_temperature(max_temperature);
        bcu_set_high_temperature_id(max_temperature_id);
    }
    if(min_temperature == MAX_TEMP)
    {
        bcu_set_low_temperature(0);
        bcu_set_low_temperature_id(0);
    }
    else
    {
        bcu_set_low_temperature(min_temperature);
        bcu_set_low_temperature_id(min_temperature_id);
    }
#endif

    bcu_set_temp_exception(temp_exception);
    bcu_set_total_temperature_num(temperature_num);
    bcu_set_average_temperature(temperature_num > 0 ? (total_temperature / temperature_num) : 0);
}


INT8U bmu_stat_get_high_temperature_count()
{
    return g_bmu_stat_high_temperature_count;
}

BmuTemperatureItem* _PAGED bmu_stat_get_high_temperature_info(int index)
{
    if(index < BMU_STAT_HIGH_TEMPERATURE_NUM)
        return &(g_bmu_stat_high_temperatures[index]);
    else
        return NULL;
}

INT8U bmu_stat_get_high_temperature(int index)
{
    if(index < BMU_STAT_HIGH_TEMPERATURE_NUM)
        return g_bmu_stat_high_temperatures[index].temperature;
    else
        return 0;
}

INT8U bmu_stat_get_high_temperature_id(int index)
{
    if(index < BMU_STAT_HIGH_TEMPERATURE_NUM)
        return g_bmu_stat_high_temperatures[index].bmu_id;
    else
        return 0;
}

INT8U bmu_stat_get_high_temperature_index(int index)
{
    if(index < BMU_STAT_HIGH_TEMPERATURE_NUM)
        return g_bmu_stat_high_temperatures[index].index;
    else
        return 0;
}

INT8U bmu_stat_get_low_temperature_count()
{
    return g_bmu_stat_low_temperature_count;
}

BmuTemperatureItem* _PAGED bmu_stat_get_low_temperature_info(int index)
{
    return &(g_bmu_stat_low_temperatures[index]);
}

INT8U bmu_stat_get_low_temperature(int index)
{
    if(index < BMU_STAT_LOW_TEMPERATURE_NUM)
        return g_bmu_stat_low_temperatures[index].temperature;
    else
        return 0;
}

INT8U bmu_stat_get_low_temperature_id(int index)
{
    if(index < BMU_STAT_LOW_TEMPERATURE_NUM)
        return g_bmu_stat_low_temperatures[index].bmu_id;
    else
        return 0;
}

INT8U bmu_stat_get_low_temperature_index(int index)
{
    if(index < BMU_STAT_LOW_TEMPERATURE_NUM)
        return g_bmu_stat_low_temperatures[index].index;
    else
        return 0;
}

INT8U bmu_stat_get_high_voltage_count()
{
    return g_bmu_stat_high_voltage_count;
}

BmuVoltageItem* _PAGED bmu_stat_get_high_voltage_info(int index)
{
    return &(g_bmu_stat_high_voltages[index]);
}

INT16U bmu_stat_get_high_voltage(int index)
{
    if(index < BMU_STAT_HIGH_VOLTAGE_NUM)
        return g_bmu_stat_high_voltages[index].voltage;
    else
        return 0;
}

INT8U bmu_stat_get_high_voltage_id(int index)
{
    if(index < BMU_STAT_HIGH_VOLTAGE_NUM)
        return g_bmu_stat_high_voltages[index].bmu_id;
    else
        return 0;
}

INT8U bmu_stat_get_high_voltage_index(int index)
{
    if(index < BMU_STAT_HIGH_VOLTAGE_NUM)
        return g_bmu_stat_high_voltages[index].index;
    else
        return 0;
}

INT8U bmu_stat_get_low_voltage_count()
{
    return g_bmu_stat_low_voltage_count;
}

BmuVoltageItem* _PAGED bmu_stat_get_low_voltage_info(int index)
{
    return &(g_bmu_stat_low_voltages[index]);
}

INT16U bmu_stat_get_low_voltage(int index)
{
    if(index < BMU_STAT_LOW_VOLTAGE_NUM)
        return g_bmu_stat_low_voltages[index].voltage;
    else
        return 0;
}

INT8U bmu_stat_get_low_voltage_id(int index)
{
    if(index < BMU_STAT_LOW_VOLTAGE_NUM)
        return g_bmu_stat_low_voltages[index].bmu_id;
    else
        return 0;
}

INT8U bmu_stat_get_low_voltage_index(int index)
{
    if(index < BMU_STAT_LOW_VOLTAGE_NUM)
        return g_bmu_stat_low_voltages[index].index;
    else
        return 0;
}

// 本函数不能重复提交id, 造成index雷同数据
void bmu_stat_high_temperature(BmuId id, INT8U index)
{
    INT8U insert_pos, pos;
    INT8U temperature = TEMP_ITEMS(id, index);
    if (g_bmu_stat_temp_high_temperature_count == 0)
    {
        insert_pos = 0;
    }
    else
    {
        insert_pos = g_bmu_stat_temp_high_temperature_count;
        while ((insert_pos > 0) && (g_bmu_stat_temp_high_temperatures[insert_pos - 1].temperature < temperature))
        {
            insert_pos--;
        }
        if (insert_pos == BMU_STAT_HIGH_TEMPERATURE_NUM) return;
    }

    MOVE_RIGHT(g_bmu_stat_temp_high_temperatures, BMU_STAT_HIGH_TEMPERATURE_NUM, insert_pos);

    if (g_bmu_stat_temp_high_temperature_count < BMU_STAT_HIGH_TEMPERATURE_NUM) ++g_bmu_stat_temp_high_temperature_count;

    g_bmu_stat_temp_high_temperatures[insert_pos].bmu_id = id + 1;
    g_bmu_stat_temp_high_temperatures[insert_pos].index = index + 1;
    g_bmu_stat_temp_high_temperatures[insert_pos].temperature = temperature;
}

void bmu_stat_low_temperature(BmuId id, INT8U index)
{
    INT8U insert_pos, pos;
    INT8U temperature = TEMP_ITEMS(id, index);
    if (g_bmu_stat_temp_low_temperature_count == 0)
    {
        insert_pos = 0;
    }
    else
    {
        insert_pos = g_bmu_stat_temp_low_temperature_count;
        while (insert_pos > 0 && g_bmu_stat_temp_low_temperatures[insert_pos - 1].temperature > temperature)
        {
            insert_pos--;
        }
        if (insert_pos == BMU_STAT_LOW_TEMPERATURE_NUM) return;
    }

    MOVE_RIGHT(g_bmu_stat_temp_low_temperatures, BMU_STAT_LOW_TEMPERATURE_NUM, insert_pos);

    if (g_bmu_stat_temp_low_temperature_count < BMU_STAT_LOW_TEMPERATURE_NUM) ++g_bmu_stat_temp_low_temperature_count;

    g_bmu_stat_temp_low_temperatures[insert_pos].bmu_id = id + 1;
    g_bmu_stat_temp_low_temperatures[insert_pos].index = index + 1;
    g_bmu_stat_temp_low_temperatures[insert_pos].temperature = temperature;
}

void bmu_stat_high_voltage(BmuId id, INT8U index)
{
    INT8U insert_pos, pos;
    INT16U voltage;
    OS_INIT_CRITICAL();

    OS_ENTER_CRITICAL();
    voltage = VOLT_ITEMS(id, index);
    OS_EXIT_CRITICAL();

    if (g_bmu_stat_temp_high_voltage_count == 0)
    {
        insert_pos = 0;
    }
    else
    {
        insert_pos = g_bmu_stat_temp_high_voltage_count;
        while ((insert_pos > 0) && (g_bmu_stat_temp_high_voltages[insert_pos - 1].voltage < voltage))
        {
            insert_pos--;
        }
        if (insert_pos == BMU_STAT_HIGH_VOLTAGE_NUM) return;
    }

    MOVE_RIGHT(g_bmu_stat_temp_high_voltages, BMU_STAT_HIGH_VOLTAGE_NUM, insert_pos);

    if (g_bmu_stat_temp_high_voltage_count < BMU_STAT_HIGH_VOLTAGE_NUM) ++g_bmu_stat_temp_high_voltage_count;

    g_bmu_stat_temp_high_voltages[insert_pos].bmu_id = id + 1;
    g_bmu_stat_temp_high_voltages[insert_pos].index = index + 1;
    g_bmu_stat_temp_high_voltages[insert_pos].voltage = voltage;
}

void bmu_stat_low_voltage(BmuId id, INT8U index)
{
    INT8U insert_pos, pos;
    INT16U voltage = VOLT_ITEMS(id, index);
    if (g_bmu_stat_temp_low_voltage_count == 0)
    {
        insert_pos = 0;
    }
    else
    {
        insert_pos = g_bmu_stat_temp_low_voltage_count;
        while ((insert_pos > 0) && (g_bmu_stat_temp_low_voltages[insert_pos - 1].voltage > voltage))
        {
            insert_pos--;
        }
        if (insert_pos == BMU_STAT_LOW_VOLTAGE_NUM) return;
    }

    MOVE_RIGHT(g_bmu_stat_temp_low_voltages, BMU_STAT_LOW_VOLTAGE_NUM, insert_pos);

    if (g_bmu_stat_temp_low_voltage_count < BMU_STAT_LOW_VOLTAGE_NUM) ++g_bmu_stat_temp_low_voltage_count;

    g_bmu_stat_temp_low_voltages[insert_pos].bmu_id = id + 1;
    g_bmu_stat_temp_low_voltages[insert_pos].index = index + 1;
    g_bmu_stat_temp_low_voltages[insert_pos].voltage = voltage;
}

BOOLEAN bmu_is_valid_voltage(INT16U voltage)
{
    return (voltage >= BATTERY_VOLTAGE_MIN_VALUE && voltage <= BATTERY_VOLTAGE_MAX_VALUE);
}

BOOLEAN bmu_is_valid_temperature(INT8U temperature)
{
    return (temperature >= BATTERY_TEMPERATURE_MIN_VALUE && temperature <= BATTERY_TEMPERATURE_MAX_VALUE);
}
//////////////////////////////////////////////////////////////////////
BOOLEAN charger_is_valid_temperature(INT8U temperature)
{
    return (temperature >= CHARGER_TEMPERATURE_MIN_VALUE && temperature <= CHARGER_TEMPERATURE_MAX_VALUE);
}
///////////////////////////////////////////////////////////////////////
BOOLEAN bmu_is_online(BmuId id)
{
    INT8U mode;
    
    mode = (INT8U)config_get(kBCUCommModeIndex);
    if(mode != BMS_COMM_SLAVE_MODE)
    {
        #if BMS_SUPPORT_HARDWARE_LTC6803 == 1
        if(id == BYU_USE_SLAVE_INDEX) return byu_all_battery_sample_board_is_online();
        #endif
        #if BMS_SUPPORT_HARDWARE_LTC6804 == 1
         return (!is_bmu_comm_error_use_bsu(id));
        #endif
    }
    
    if(config_get(kCommFstAlarmDlyIndex) == 0xFFFF)
        return TRUE;
    if(bmu_get_heart_beat_tick(id) == 0)
        return TRUE;
    if(get_interval_by_tick(bmu_get_heart_beat_tick(id), get_tick_count()) <= (INT32U)config_get(kCommFstAlarmDlyIndex)*1000)
        return TRUE;
    else
        return FALSE;
}

void bmu_reset_offline(BmuId id)
{
    INT8U index;
    INT8U mode;
    
    mode = (INT8U)config_get(kBCUCommModeIndex);
    if(mode != BMS_COMM_SLAVE_MODE)
    {//byu信息由采集板驱动控制清零
        #if BMS_SUPPORT_HARDWARE_LTC6803 == 1
        if(id == BYU_USE_SLAVE_INDEX) return;
        #endif
        #if BMS_SUPPORT_HARDWARE_LTC6803 == 1
         return ;
        #endif
    }
    
    /*从机通信中断   ,初始化所有相关的数据*/
    //bmu_set_temperature_num(id, 0);
    //bmu_set_voltage_num(id, 0);

    for (index = 0; index < g_bmu_voltage_max_num; index++)
        bmu_set_voltage_item(id, index, 0);

    for (index = 0; index < g_bmu_temp_max_num; index++)
        bmu_set_temperature_item(id, index, 0);

    bmu_set_pack_state(id, 0);
#if BCU_CONTROL_SLAVE_RELAY_INTERVAL_EN
    bmu_slave_cool_control_update(id, 0);
    bmu_slave_heat_control_update(id, 0);
#endif

    for (index = 0; index < BMS_BMU_BALANCE_STATE_BYTE_NUM; index++)
        bmu_set_balance_state(id, index, 0);

    bmu_set_balance_except_state(id, 0);
}

void start_bmu_heart_beat_tick(INT8U id)
{
    if(0 == bmu_get_heart_beat_tick(id))
        bmu_set_heart_beat_tick(id, get_tick_count());
}

void bmu_heart_beat_tick_update(INT8U num)
{
    INT32U tick;

    tick = get_tick_count();//tick=0做为特殊数据，不做计数值
    if(tick != 0)
        bmu_set_heart_beat_tick(num, tick);
    else
        bmu_set_heart_beat_tick(num, 1);
}


INT8U bmu_get_max_temp(INT8U id)
{
    INT8U items_num,temperature,index,max_temperature=MIN_TEMP;
    INT16U offset;

    if (id >= BMU_MAX_SLAVE_COUNT) return 0;

    offset = (INT16U)id * g_bmu_temp_max_num;
    items_num = bmu_get_temperature_num(id);

    for (index = 0; index < items_num; index++) //最高、最低电压扫描,总压计算
    {
        temperature = g_bmu_temp_items[offset++];
        if (!bmu_is_valid_temperature(temperature))
        {
            continue;
        }

        if (max_temperature < temperature)
        {
            max_temperature = temperature;
            //max_temperature_id = (((INT16U)slave_index + 1) << 8) + (index + 1);
        }
    }
    if(max_temperature == MIN_TEMP)
        return 0;
    else
        return max_temperature;
}

INT8U bmu_get_min_temp(INT8U id)
{
    INT8U items_num,temperature,index,min_temperature=MAX_TEMP;
    INT16U offset;

    if (id >= BMU_MAX_SLAVE_COUNT) return 0;

    offset = (INT16U)id * g_bmu_temp_max_num;
    items_num = bmu_get_temperature_num(id);

    for (index = 0; index < items_num; index++) //最高、最低电压扫描,总压计算
    {
        temperature = g_bmu_temp_items[offset++];
        if (!bmu_is_valid_temperature(temperature))
        {
            continue;
        }
        if (min_temperature > temperature)
        {
            min_temperature = temperature;
            //min_temperature_id = (((INT16U)slave_index + 1) << 8) + (index + 1);
        }
    }
    if(min_temperature == MAX_TEMP)
        return 0;
    else
        return min_temperature;
}

INT32U bmu_get_total_voltage(INT8U id)
{
    INT8U index, items_num;
    INT16U voltage;
    INT32U total_voltage = 0;
    
    if (id >= BMU_MAX_SLAVE_COUNT) return 0;
    
    items_num = bmu_get_voltage_num(id);
    for(index = 0; index < items_num; index++)
    {
        voltage = bmu_get_voltage_item(id, index);
        if (!bmu_is_valid_voltage(voltage))
        {
            continue;
        }
        total_voltage += bmu_get_voltage_item(id, index);
    }
    
    return total_voltage;
}

void bmu_get_low_voltage(INT8U id, INT16U* voltage, INT8U* pos)
{
    INT8U index, items_num;
    INT8U voltage_min_pos = 0;
    INT16U voltage_item;
    INT16U voltage_min = MAX_VOLT;
    
    if (id >= BMU_MAX_SLAVE_COUNT ||
        (voltage == NULL && pos == NULL) ||
        bmu_get_voltage_num(id) == 0)
    {
        if(voltage) *voltage = 0;
        if(pos) *pos = 0;
        return;
    }
    
    items_num = bmu_get_voltage_num(id);
    for(index = 0; index < items_num; index++)
    {
        voltage_item = bmu_get_voltage_item(id, index);
        if (!bmu_is_valid_voltage(voltage_item))
        {
            continue;
        }
        if(voltage_min > voltage_item)
        {
            voltage_min = voltage_item;
            voltage_min_pos = index;
        }
    }
    if(voltage) *voltage = voltage_min;
    if(pos) *pos = voltage_min_pos + 1;
}

void bmu_get_high_voltage(INT8U id, INT16U* voltage, INT8U* pos)
{
    INT8U index, items_num;
    INT8U voltage_max_pos = 0;
    INT16U voltage_item;
    INT16U voltage_max = MIN_VOLT;
    
    if (id >= BMU_MAX_SLAVE_COUNT ||
        (voltage == NULL && pos == NULL) ||
        bmu_get_voltage_num(id) == 0) 
    {
        if(voltage) *voltage = 0;
        if(pos) *pos = 0;
        return;
    }
    
    items_num = bmu_get_voltage_num(id);
    for(index = 0; index < items_num; index++)
    {
        voltage_item = bmu_get_voltage_item(id, index);
        if (!bmu_is_valid_voltage(voltage_item))
        {
            continue;
        }
        if(voltage_max < voltage_item)
        {
            voltage_max = voltage_item;
            voltage_max_pos = index;
        }
    }
    if(voltage) *voltage = voltage_max;
    if(pos) *pos = voltage_max_pos + 1;
}

void bmu_get_low_temperature(INT8U id, INT8U* temperature, INT8U* pos)
{
    INT8U index, items_num;
    INT8U temperature_min_pos = 0;
    INT16U temperature_item;
    INT16U temperature_min = MAX_TEMP;
    
    if (id >= BMU_MAX_SLAVE_COUNT ||
        (temperature == NULL && pos == NULL) ||
        bmu_get_temperature_num(id) == 0)
    {
        if(temperature) *temperature = 0;
        if(pos) *pos = 0;
        return;
    }
    
    items_num = bmu_get_temperature_num(id);
    for(index = 0; index < items_num; index++)
    {
        temperature_item = bmu_get_temperature_item(id, index);
        if (!bmu_is_valid_temperature(temperature_item))
        {
            continue;
        }
        if(temperature_min > temperature_item)
        {
            temperature_min = temperature_item;
            temperature_min_pos = index;
        }
    }
    if(temperature) *temperature = temperature_min;
    if(pos) *pos = temperature_min_pos + 1;
}

void bmu_get_high_temperature(INT8U id, INT8U* temperature, INT8U* pos)
{
    INT8U index, items_num;
    INT8U temperature_max_pos = 0;
    INT16U temperature_item;
    INT16U temperature_max = MIN_TEMP;
    
    if (id >= BMU_MAX_SLAVE_COUNT ||
        (temperature == NULL && pos == NULL) ||
        bmu_get_temperature_num(id) == 0) 
    {
        if(temperature) *temperature = 0;
        if(pos) *pos = 0;
        return;
    }
    
    items_num = bmu_get_temperature_num(id);
    for(index = 0; index < items_num; index++)
    {
        temperature_item = bmu_get_temperature_item(id, index);
        if (!bmu_is_valid_temperature(temperature_item))
        {
            continue;
        }
        if(temperature_max < temperature_item)
        {
            temperature_max = temperature_item;
            temperature_max_pos = index;
        }
    }
    if(temperature) *temperature = temperature_max;
    if(pos) *pos = temperature_max_pos + 1;
}

INT8U bmu_self_check(INT8U id)
{
    INT8U i, num;
    
    if (id >= BMU_MAX_SLAVE_COUNT) return 1;
    
    num = bmu_get_voltage_num(id);
    if (num == 0) return 0;
    for (i=0; i<num; i++)
    {
        if (!bmu_is_valid_voltage(bmu_get_voltage_item(id, i))) return 0;
    }
    
    num = bmu_get_temperature_num(id);
#if BMU_SELF_CHECK_TEMPERATURE_EN  //是否有温度
    if (num == 0) return 0;
#endif
    for (i=0; i<num; i++)
    {
        if (!bmu_is_valid_temperature(bmu_get_temperature_item(id, i))) return 0;
    }
    
    return 1;    
}

INT16U bmu_get_delta_voltage_max(INT8U id)
{
    INT16U high_value, low_value;
    INT8U pos;
    
    if (id >= BMU_MAX_SLAVE_COUNT) return 0;
    
    bmu_get_high_voltage(id, &high_value, &pos);
    bmu_get_low_voltage(id, &low_value, &pos);
    
    return (high_value >= low_value) ? (high_value-low_value) : 0;
}

INT8U bmu_get_delta_temperature_max(INT8U id)
{
    INT8U high_value, low_value;
    INT8U pos;
    
    if (id >= BMU_MAX_SLAVE_COUNT) return 0;
    
    bmu_get_high_temperature(id, &high_value, &pos);
    bmu_get_low_temperature(id, &low_value, &pos);
    
    return (high_value >= low_value) ? (high_value-low_value) : 0;
}


#if BCU_CONTROL_SLAVE_RELAY_INTERVAL_EN
//主机控制从机制冷继电器，保证每个从机制冷不同时闭合，且间隔设定时间
void bmu_slave_cool_control_update(INT8U slave, INT8U relay_flag)
{
    INT8U cool_flag, slave_num;
    OS_CPU_SR cpu_sr = 0;
    
    slave_num = (INT8U)config_get(kSlaveNumIndex);
    if(slave >= slave_num) return;
    cool_flag = bmu_get_cool_status(slave);
    
    OS_ENTER_CRITICAL();
    if(cool_flag)
    {
        if(relay_flag == 0)
        {
            if(g_slaveCtlCxt.cool_ctl_item[slave].control.bits.num == 0)
            {
                g_slaveCtlCxt.cool_ctl_item[slave].control.bits.num = slave+1;
                g_slaveCtlCxt.cool_ctl_item[slave].control.bits.flag = 0;
                g_slaveCtlCxt.cool_ctl_cnt++;
                g_slaveCtlCxt.cool_ctl_item[slave].next = NULL;
                list_add(g_slaveCtlCxt.cool_ctl_list, (LIST_ITEM*)(&g_slaveCtlCxt.cool_ctl_item[slave]));
            }
        }
    }
    else
    {
        if(g_slaveCtlCxt.cool_ctl_item[slave].control.bits.num) 
        {
            list_remove(g_slaveCtlCxt.cool_ctl_list, (LIST_ITEM*)(&g_slaveCtlCxt.cool_ctl_item[slave]));
            bmu_cool_control_set_off(slave);
        }
    }
    OS_EXIT_CRITICAL();
}

void bmu_cool_control_check(void* pdata)
{
    SlaveRelayCtlItem* item;
    OS_CPU_SR cpu_sr = 0;
    UNUSED(pdata);
    
    if(g_slaveCtlCxt.cool_ctl_list == NULL) return;
    if(g_slaveCtlCxt.cool_ctl_cnt == 0) return;
    
    OS_ENTER_CRITICAL();
    item = (SlaveRelayCtlItem*) g_slaveCtlCxt.cool_ctl_list->next;
    if(g_slaveCtlCxt.cool_ctl_tick == 0)
    {
        if(item != NULL)
        {
            bmu_cool_control_set_on(item->control.bits.num-1);
        }
    }
    else
    {
        if(get_elapsed_tick_count(g_slaveCtlCxt.cool_ctl_tick) >= SLAVE_COOL_RELAY_ON_INTERVAL)
        {
            while(item != NULL)
            {
                if(item->control.bits.flag == 0)
                {
                    bmu_cool_control_set_on(item->control.bits.num-1);
                    break;
                }
                item = (SlaveRelayCtlItem*)list_next((LIST_ITEM*)item);
            }
        }
    }
    OS_EXIT_CRITICAL();
}

void bmu_cool_control_set_on(INT8U slave)
{
    if(slave >= config_get(kSlaveNumIndex)) return;
    
    if(bmu_get_cool_status(slave))
    {
        g_slaveCtlCxt.cool_ctl_item[slave].control.bits.flag = 1;
        if(g_slaveCtlCxt.cool_ctl_cnt)
            g_slaveCtlCxt.cool_ctl_cnt--;
        g_slaveCtlCxt.cool_ctl_tick = get_tick_count();
    }
    else
    {
        bmu_slave_cool_control_update(slave, 0);
    }
}

void bmu_cool_control_set_off(INT8U slave)
{
    if(slave >= config_get(kSlaveNumIndex)) return;
    
    g_slaveCtlCxt.cool_ctl_item[slave].next = NULL;
    if(g_slaveCtlCxt.cool_ctl_item[slave].control.bits.flag == 0)
    {
        if(g_slaveCtlCxt.cool_ctl_cnt)
            g_slaveCtlCxt.cool_ctl_cnt--;
    }
    g_slaveCtlCxt.cool_ctl_item[slave].control.value = 0;
}

INT8U bmu_cool_control_is_on(INT8U slave)
{
    SlaveRelayCtlItem* item;
    
    if(slave >= config_get(kSlaveNumIndex)) return 0;
    
    item = &g_slaveCtlCxt.cool_ctl_item[slave];
    if(list_find(g_slaveCtlCxt.cool_ctl_list, (LIST_ITEM*)item) && item->control.bits.flag)
        return 1;
    return 0;
}

//主机控制从机制热继电器，保证每个从机制热不同时闭合，且间隔设定时间
void bmu_slave_heat_control_update(INT8U slave, INT8U relay_flag)
{
    INT8U heat_flag, slave_num;
    OS_CPU_SR cpu_sr = 0;
    
    slave_num = (INT8U)config_get(kSlaveNumIndex);
    if(slave >= slave_num) return;
    heat_flag = bmu_get_heat_status(slave);
    
    OS_ENTER_CRITICAL();
    if(heat_flag)
    {
        if(relay_flag == 0)
        {
            if(g_slaveCtlCxt.heat_ctl_item[slave].control.bits.num == 0)
            {
                g_slaveCtlCxt.heat_ctl_item[slave].control.bits.num = slave+1;
                g_slaveCtlCxt.heat_ctl_item[slave].control.bits.flag = 0;
                g_slaveCtlCxt.heat_ctl_cnt++;
                g_slaveCtlCxt.heat_ctl_item[slave].next = NULL;
                list_add(g_slaveCtlCxt.heat_ctl_list, (LIST_ITEM*)(&g_slaveCtlCxt.heat_ctl_item[slave]));
            }
        }
    }
    else
    {
        if(g_slaveCtlCxt.heat_ctl_item[slave].control.bits.num) 
        {
            list_remove(g_slaveCtlCxt.heat_ctl_list, (LIST_ITEM*)(&g_slaveCtlCxt.heat_ctl_item[slave]));
            bmu_heat_control_set_off(slave);
        }
    }
    OS_EXIT_CRITICAL();
}

void bmu_heat_control_check(void* pdata)
{
    SlaveRelayCtlItem* item;
    OS_CPU_SR cpu_sr = 0;
    UNUSED(pdata);
    
    if(g_slaveCtlCxt.heat_ctl_list == NULL) return;
    if(g_slaveCtlCxt.heat_ctl_cnt == 0) return;
    
    OS_ENTER_CRITICAL();
    item = (SlaveRelayCtlItem*) g_slaveCtlCxt.heat_ctl_list->next;
    if(g_slaveCtlCxt.heat_ctl_tick == 0)
    {
        if(item != NULL)
        {
            bmu_heat_control_set_on(item->control.bits.num-1);
        }
    }
    else
    {
        if(get_elapsed_tick_count(g_slaveCtlCxt.heat_ctl_tick) >= SLAVE_HEAT_RELAY_ON_INTERVAL)
        {
            while(item != NULL)
            {
                if(item->control.bits.flag == 0)
                {
                    bmu_heat_control_set_on(item->control.bits.num-1);
                    break;
                }
                item = (SlaveRelayCtlItem*)list_next((LIST_ITEM*)item);
            }
        }
    }
    OS_EXIT_CRITICAL();
}

void bmu_heat_control_set_on(INT8U slave)
{
    if(slave >= config_get(kSlaveNumIndex)) return;
    
    if(bmu_get_heat_status(slave))
    {
        g_slaveCtlCxt.heat_ctl_item[slave].control.bits.flag = 1;
        if(g_slaveCtlCxt.heat_ctl_cnt)
            g_slaveCtlCxt.heat_ctl_cnt--;
        g_slaveCtlCxt.heat_ctl_tick = get_tick_count();
    }
    else
    {
        bmu_slave_heat_control_update(slave, 0);
    }
}

void bmu_heat_control_set_off(INT8U slave)
{
    if(slave >= config_get(kSlaveNumIndex)) return;
    
    g_slaveCtlCxt.heat_ctl_item[slave].next = NULL;
    if(g_slaveCtlCxt.heat_ctl_item[slave].control.bits.flag == 0)
    {
        if(g_slaveCtlCxt.heat_ctl_cnt)
            g_slaveCtlCxt.heat_ctl_cnt--;
    }
    g_slaveCtlCxt.heat_ctl_item[slave].control.value = 0;
}

INT8U bmu_heat_control_is_on(INT8U slave)
{
    SlaveRelayCtlItem* item;
    
    if(slave >= config_get(kSlaveNumIndex)) return 0;
    
    item = &g_slaveCtlCxt.heat_ctl_item[slave];
    if(list_find(g_slaveCtlCxt.heat_ctl_list, (LIST_ITEM*)item) && item->control.bits.flag)
        return 1;
    return 0;
}

INT8U bmu_set_total_temperature_num(BmuId id, INT8U num) //包含电池温度个数和加热器温度个数
{
    INT8U i, heat_temp_num = 0, outlet_num = 0;
    RPAGE_INIT();

    if (id >= BMU_MAX_SLAVE_COUNT) return RES_FALSE;

    RPAGE_SAVE();

    if (num > g_bmu_temp_max_num) num = g_bmu_temp_max_num;
    for (i=0; i<num; i++)
    {
        if (bmu_temperature_is_heat_temperature(id, i+1)) heat_temp_num++;
#if BMU_CHR_OUTLET_TEMP_SUPPORT
        else if (bmu_temperature_is_chr_outlet_temperature(id, i+1)) outlet_num++;
#endif
    }
    if (heat_temp_num > BMU_HEAT_TEMP_NUM_MAX) heat_temp_num = BMU_HEAT_TEMP_NUM_MAX;
    
    num = num - heat_temp_num;
    num = num - outlet_num;
    if(num > config_get(kBmuMaxTempNumIndex)) num = config_get(kBmuMaxTempNumIndex);
    
    bmu_set_heat_temperature_num(id, heat_temp_num);
    bmu_set_temperature_num(id, num);
#if BMU_CHR_OUTLET_TEMP_SUPPORT
    bmu_set_chr_outlet_temperature_num(id, outlet_num);
#endif
    RPAGE_RESTORE();

    return RES_TRUE;
}

INT8U bmu_get_total_temperature_num(BmuId id) //包含电池温度个数和加热器温度个数
{
#if BMU_CHR_OUTLET_TEMP_SUPPORT
    return bmu_get_heat_temperature_num(id) + bmu_get_temperature_num(id) + bmu_get_chr_outlet_temperature_num(id);
#else
    return bmu_get_heat_temperature_num(id) + bmu_get_temperature_num(id);
#endif
}

INT8U bmu_set_heat_temperature_num(BmuId id, INT8U num)
{
    OS_INIT_CRITICAL();
    RPAGE_INIT();

    if (id >= BMU_MAX_SLAVE_COUNT) return RES_FALSE;

    RPAGE_SAVE();

    if (num > g_bmu_temp_max_num) num = g_bmu_temp_max_num;
    if (num > BMU_HEAT_TEMP_NUM_MAX) num = BMU_HEAT_TEMP_NUM_MAX;
    
    OS_ENTER_CRITICAL();
    g_bmu_infos[id].heat_temp_num = num;
    OS_EXIT_CRITICAL();

    RPAGE_RESTORE();

    return RES_TRUE;
}

INT8U bmu_get_heat_temperature_num(BmuId id)
{
    INT8U value;
    OS_INIT_CRITICAL();
    RPAGE_INIT();

    if (id >= BMU_MAX_SLAVE_COUNT) return 0;

    RPAGE_SAVE();
    OS_ENTER_CRITICAL();
    value = g_bmu_infos[id].heat_temp_num;
    if(value == BMU_TMEPERATURE_INVALID_VALUE)
        value = 0;
    if(value > BMU_HEAT_TEMP_NUM_MAX)
        value = BMU_HEAT_TEMP_NUM_MAX;
    OS_EXIT_CRITICAL();
    RPAGE_RESTORE();

    return value;
}

void bmu_set_heat_temperature_item(BmuId id, INT8U index, INT8U value)
{
    INT16U offset;
    OS_CPU_SR cpu_sr = 0;
    RPAGE_INIT();
    RPAGE_SAVE();

    if (id < BMU_MAX_SLAVE_COUNT && index < g_bmu_temp_max_num)
    {
        offset = (INT16U)id * g_bmu_temp_max_num + bmu_get_temperature_num(id) + index;
        if (offset < BMU_MAX_TOTAL_TEMP_NUM)
        {
            OS_ENTER_CRITICAL();
            g_bmu_temp_items[offset] = value;
            OS_EXIT_CRITICAL();
        }
    }

    RPAGE_RESTORE();
}

INT8U bmu_get_heat_temperature_item(BmuId id, INT8U index)
{
    INT8U value = 0;
    INT16U offset;
    OS_CPU_SR cpu_sr = 0;
    RPAGE_INIT();
    RPAGE_SAVE();

    if (id < BMU_MAX_SLAVE_COUNT && index < g_bmu_temp_max_num)
    {
        offset = (INT16U)id * g_bmu_temp_max_num + bmu_get_temperature_num(id) + index;
        if (offset < BMU_MAX_TOTAL_TEMP_NUM)
        {
            OS_ENTER_CRITICAL();
            value = g_bmu_temp_items[offset];
            OS_EXIT_CRITICAL();
        }
    }

    RPAGE_RESTORE();

    return value;
}

INT8U bmu_temperature_is_heat_temperature(BmuId id, INT16U pos)
{
    INT8U flag = FALSE, i = 0;
    RPAGE_INIT();
    
    if (id >= BMU_MAX_SLAVE_COUNT) return flag;
    RPAGE_SAVE();
    for (i = 0; i < BMU_HEAT_TEMP_NUM_MAX; i++)
    {
        if (g_bmu_heat_temp_pos[i] == 0 || g_bmu_heat_temp_pos[id][i] > pos) break; //无效地址或超过当前地址
        if (pos == g_bmu_heat_temp_pos[id][i])
        {
            flag = TRUE;
            break;
        }
    }
    RPAGE_RESTORE();
    return flag;
}

INT8U bmu_heat_temp_pos_2_num(BmuId id, INT8U pos) //位置转索引号 pos:1,2...  num:0,1...
{
    INT8U i = 0, index = 0xFF;
    RPAGE_INIT();
    
    if (id >= config_get(kSlaveNumIndex)) return index;
    RPAGE_SAVE();
    for (i=0; i<BMU_HEAT_TEMP_NUM_MAX; i++)
    {
        if (g_bmu_heat_temp_pos[id][i] == 0)
        {
            break;
        }
        if (g_bmu_heat_temp_pos[id][i] == pos)
        {
            index = i;
            break;
        }
    }
    RPAGE_RESTORE();
    return index;
}

INT8U bmu_heat_temp_num_2_pos(BmuId id, INT8U num) //索引号转位置 pos:1,2...  num:0,1...
{
    INT8U pos = 0xFF;
    RPAGE_INIT();
    
    RPAGE_SAVE();
    if (id < config_get(kSlaveNumIndex) && num < BMU_HEAT_TEMP_NUM_MAX)
        pos = g_bmu_heat_temp_pos[id][num];
    
    RPAGE_RESTORE();
    return pos;
}

INT8U bmu_temperature_pos_2_num(BmuId id, INT8U pos) //位置转索引号 pos:1,2...  num:0,1...
{
    INT8U i = 0, cnt = 0;
    
    if (id >= config_get(kSlaveNumIndex)) return 0xFF;
    if (bmu_temperature_is_heat_temperature(id, pos)) return 0xFF;
    for (i=1; i<=pos; i++)
    {
        if (bmu_temperature_is_heat_temperature(id, i)) cnt++;
#if BMU_CHR_OUTLET_TEMP_SUPPORT
        if (bmu_temperature_is_chr_outlet_temperature(id, i)) cnt++;
#endif
    }
    if (pos < cnt+1) return 0xFF;
    pos -= (cnt+1);
    
    return pos;    
}

INT8U bmu_get_high_heat_temperature(BmuId id)
{
    INT8U i, high_temp = 0, heat_temp_num = 0, temp = 0;
    
    if (id >= config_get(kSlaveNumIndex)) return 0;
    heat_temp_num = bmu_get_heat_temperature_num(id);
    for (i=0; i<heat_temp_num; i++)
    {
        temp = bmu_get_heat_temperature_item(id, i);
        if(temp > high_temp) high_temp = temp;
    }
    return high_temp;
}

INT8U bmu_get_low_heat_temperature(BmuId id)
{
    INT8U i, low_temp = 0xFF, heat_temp_num = 0, temp = 0;
    
    if (id >= config_get(kSlaveNumIndex)) return 0;
    heat_temp_num = bmu_get_heat_temperature_num(id);
    for (i=0; i<heat_temp_num; i++)
    {
        temp = bmu_get_heat_temperature_item(id, i);
        if(temp < low_temp) low_temp = temp;
    }
    return low_temp;
}

#if BMU_CHR_OUTLET_TEMP_SUPPORT
INT8U bmu_temperature_is_chr_outlet_temperature(BmuId id, INT16U pos)
{
    INT8U flag = FALSE, i = 0;
    RPAGE_INIT();
    
    if (id >= BMU_MAX_SLAVE_COUNT) return flag;
    RPAGE_SAVE();
    for (i = 0; i < BMU_CHR_OUTLET_TEMP_NUM_MAX; i++)
    {
        if (g_bmu_chr_outlet_temp_pos[i] == 0 || g_bmu_chr_outlet_temp_pos[id][i] > pos) break; //无效地址或超过当前地址
        if (pos == g_bmu_chr_outlet_temp_pos[id][i])
        {
            flag = TRUE;
            break;
        }
    }
    RPAGE_RESTORE();
    return flag;
}

void bmu_set_chr_outlet_temperature_item(BmuId id, INT8U index, INT8U value)
{
    INT16U offset;
    OS_CPU_SR cpu_sr = 0;
    RPAGE_INIT();
    RPAGE_SAVE();

    if (id < BMU_MAX_SLAVE_COUNT && index < g_bmu_temp_max_num)
    {
        offset = (INT16U)id * g_bmu_temp_max_num + bmu_get_temperature_num(id) + bmu_get_heat_temperature_num(id) + index;
        if (offset < BMU_MAX_TOTAL_TEMP_NUM)
        {
            OS_ENTER_CRITICAL();
            g_bmu_temp_items[offset] = value;
            OS_EXIT_CRITICAL();
        }
    }

    RPAGE_RESTORE();
}

INT8U bmu_get_chr_outlet_temperature_item(BmuId id, INT8U index)
{
    INT8U value = 0;
    INT16U offset;
    OS_CPU_SR cpu_sr = 0;
    RPAGE_INIT();
    RPAGE_SAVE();

    if (id < BMU_MAX_SLAVE_COUNT && index < g_bmu_temp_max_num)
    {
        offset = (INT16U)id * g_bmu_temp_max_num + bmu_get_temperature_num(id) + bmu_get_heat_temperature_num(id) + index;
        if (offset < BMU_MAX_TOTAL_TEMP_NUM)
        {
            OS_ENTER_CRITICAL();
            value = g_bmu_temp_items[offset];
            OS_EXIT_CRITICAL();
        }
    }

    RPAGE_RESTORE();

    return value;
}

INT8U bmu_outlet_temp_pos_2_num(BmuId id, INT8U pos) //位置转索引号 pos:1,2...  num:0,1...
{
    INT8U i = 0, index = 0xFF;
    RPAGE_INIT();
    
    if (id >= config_get(kSlaveNumIndex)) return index;
    RPAGE_SAVE();
    for (i=0; i<BMU_CHR_OUTLET_TEMP_NUM_MAX; i++)
    {
        if (g_bmu_chr_outlet_temp_pos[id][i] == 0)
        {
            break;
        }
        if (g_bmu_chr_outlet_temp_pos[id][i] == pos)
        {
            index = i;
            break;
        }
    }
    RPAGE_RESTORE();
    return index;
}

INT8U bmu_chr_outlet_temp_num_2_pos(BmuId id, INT8U num) //索引号转位置 pos:1,2...  num:0,1...
{
    INT8U pos = 0xFF;
    RPAGE_INIT();
    
    RPAGE_SAVE();
    if (id < config_get(kSlaveNumIndex) && num < BMU_CHR_OUTLET_TEMP_NUM_MAX)
        pos = g_bmu_chr_outlet_temp_pos[id][num];
    
    RPAGE_RESTORE();
    return pos;
}

INT8U bmu_set_chr_outlet_temperature_num(BmuId id, INT8U num)
{
    OS_INIT_CRITICAL();
    RPAGE_INIT();

    if (id >= BMU_MAX_SLAVE_COUNT) return RES_FALSE;

    RPAGE_SAVE();

    if (num > g_bmu_temp_max_num) num = g_bmu_temp_max_num;
    if (num > BMU_CHR_OUTLET_TEMP_NUM_MAX) num = BMU_CHR_OUTLET_TEMP_NUM_MAX;
    
    OS_ENTER_CRITICAL();
    g_bmu_infos[id].chr_outlet_num = num;
    OS_EXIT_CRITICAL();

    RPAGE_RESTORE();

    return RES_TRUE;
}

INT8U bmu_get_chr_outlet_temperature_num(BmuId id)
{
    INT8U value;
    OS_INIT_CRITICAL();
    RPAGE_INIT();

    if (id >= BMU_MAX_SLAVE_COUNT) return 0;

    RPAGE_SAVE();
    OS_ENTER_CRITICAL();
    value = g_bmu_infos[id].chr_outlet_num;
    if(value == BMU_TMEPERATURE_INVALID_VALUE)
        value = 0;
    if(value > BMU_CHR_OUTLET_TEMP_NUM_MAX)
        value = BMU_CHR_OUTLET_TEMP_NUM_MAX;
    OS_EXIT_CRITICAL();
    RPAGE_RESTORE();

    return value;
}

INT8U bmu_get_high_chr_outlet_temperature(BmuId id)
{
    INT8U i, high_temp = 0, outlet_temp_num = 0, temp = 0;
    
    if (id >= config_get(kSlaveNumIndex)) return 0;
    outlet_temp_num = bmu_get_chr_outlet_temperature_num(id);
    for (i=0; i<outlet_temp_num; i++)
    {
        temp = bmu_get_chr_outlet_temperature_item(id, i);
        if(temp > high_temp) high_temp = temp;
    }
    return high_temp;
}

#endif

//test
#if 0
void bmu_relay_control_test(void)
{
    INT8U i;
    
    i = (INT8U)config_get(kSlaveNumIndex);
    for(; i!=0; i--)
    {
        bmu_set_cool_status(i-1, 1);
        bmu_slave_cool_control_update(i-1, 0);
        bmu_set_heat_status(i-1, 1);
        bmu_slave_heat_control_update(i-1, 0);
    }
}
#endif //#if 0
#endif //#if BCU_CONTROL_SLAVE_RELAY_INTERVAL_EN
