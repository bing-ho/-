/**
*
* Copyright (c) 2012 Ligoo Inc.
*
* @file bms_bmu.h
* @brief
* @note
* @author
* @date 2012-5-9
*
*/

#ifndef BMS_BMU_H_
#define BMS_BMU_H_
#include "bms_defs.h"
#include "bms_bcu.h"
#include "bms_util.h"
#include "bms_config.h"
#include "bms_job.h"
#include "bms_list.h"

#define BMU_TMEPERATURE_INVALID_VALUE           0xFF

#define BCU_CONTROL_SLAVE_RELAY_INTERVAL_EN     1          //�������ƴӻ��̵����պϼ��ʹ�ܿ���
#define BCU_CONTROL_SLAVE_COOL_RELAY_NUM        1          //�ӻ�����̵������ ע���������ȱ�Ų�����ͬ
#define BCU_CONTROL_SLAVE_HEAT_RELAY_NUM        2          //�ӻ����ȼ̵������ ע���������ȱ�Ų�����ͬ

#define SLAVE_COOL_RELAY_ON_INTERVAL            2000       //����̵����պϼ��
#define SLAVE_HEAT_RELAY_ON_INTERVAL            2000       //���ȼ̵����պϼ�� 
#define SLAVE_RELAY_CONTROL_CHECK_PERIODIC      300

#define BMU_SELF_CHECK_TEMPERATURE_EN           1  //�����¶��Լ쿪��
#define BMU_TMEPERATURE_INVALID_VALUE           0xFF

typedef INT8U BmuId;

typedef struct _BmuInfo
{ /*�������ģ����Ϣ*/
    INT8U voltage_num; //�����ѹ��Ŀ
    //INT16U voltages[BMS_BMU_BATTERY_MAX_NUM]; //�����ѹ
    INT16U balance_current[BMS_BMU_BALANCE_NUM]; //��·�������ֵ
    INT8U temperature_num; //�����ѹ��Ŀ
    INT8U heat_temp_num; //�����¸и���
#if BMU_CHR_OUTLET_TEMP_SUPPORT
    INT8U chr_outlet_num;
#endif
    //INT8U temperatures[BMS_BMU_TEMPERATURE_MAX_NUM]; //4�������¶�+2��������¶�
    INT8U balance_states[BMS_BMU_BALANCE_STATE_BYTE_NUM];
    //INT32U balance_state; //��ؾ���״̬
    INT8U pack_state; //�����ģ��״̬
    INT8U high_volt_state;	//������䱨��
    INT8U low_volt_state;//������ű���
    INT8U high_temp_state;//������±���
    INT8U low_temp_state; //������±���
    INT8U commabort_state;//ͨ���жϱ���
    INT8U balance_num; //����·��
    INT32U balance_except_state;//�����쳣����
    INT32U heart_beat_tick; //�ӻ���������
    INT8U relay_state;
} BmuInfo;

#define BMU_PACK_INSU_STATUS_BIT	0
#define BMU_PACK_COOL_STATUS_BIT	1
#define BMU_PACK_HEAT_STATUS_BIT	2

typedef struct
{
    INT8U bmu_id;
    INT8U index;
    INT8U temperature;
}BmuTemperatureItem;

typedef struct
{
    INT8U bmu_id;
    INT8U index;
    INT16U voltage;
}BmuVoltageItem;

#if BCU_CONTROL_SLAVE_RELAY_INTERVAL_EN //�ӻ��̵����պ�˳����������

typedef struct _LIST_ITEM LIST_ITEM;
typedef struct
{
    LIST_ITEM* next;
    union 
    {
        byte value;
        struct 
        {
            byte num         :7; //�ӻ����
            byte flag        :1; //�պϱ�־
        }bits;
    }control;
}SlaveRelayCtlItem;

typedef struct
{
    LIST_ITEM* cool_ctl_list;
    INT8U cool_ctl_cnt;
    INT32U cool_ctl_tick; //����̵����պϼ������
    SlaveRelayCtlItem cool_ctl_item[BMU_MAX_SLAVE_COUNT];  //��list��֧��ҳ�ڴ棬���Զ����ڷǷ�ҳ��
    LIST_ITEM* heat_ctl_list;
    INT8U heat_ctl_cnt;
    INT32U heat_ctl_tick; //����̵����պϼ������
    SlaveRelayCtlItem heat_ctl_item[BMU_MAX_SLAVE_COUNT];  //��list��֧��ҳ�ڴ棬���Զ����ڷǷ�ҳ��
}SlaveControlContext;

#endif

#pragma DATA_SEG __RPAGE_SEG PAGED_RAM_BMU
extern BmuInfo g_bmu_infos[BMU_MAX_SLAVE_COUNT];
extern INT16U g_bmu_voltage_items[BMU_MAX_TOTAL_VOLTAGE_NUM];
extern INT8U g_bmu_temp_items[BMU_MAX_TOTAL_TEMP_NUM];
extern INT8U g_bmu_voltage_max_num;
extern INT8U g_bmu_temp_max_num;
#pragma DATA_SEG DEFAULT

#ifndef BMU_VAR_FUNC_IMPL
#define BMU_VAR_FUNC_DEF(TYPE, VAR) \
        TYPE bmu_get_##VAR(BmuId id); \
        void bmu_set_##VAR(BmuId id, TYPE value);
#else
#define BMU_VAR_FUNC_DEF(TYPE, VAR) \
    TYPE bmu_get_##VAR(BmuId id) \
    { \
        OS_CPU_SR cpu_sr = 0; \
        RPAGE_INIT();\
        TYPE value; \
        RPAGE_SAVE();\
        OS_ENTER_CRITICAL(); \
        value = bmu_get_info(id)->VAR; \
        OS_EXIT_CRITICAL(); \
        RPAGE_RESTORE();\
        return value; \
    } \
    void bmu_set_##VAR(BmuId id, TYPE value) \
    { \
        OS_CPU_SR cpu_sr = 0; \
        RPAGE_INIT();\
        RPAGE_SAVE();\
        OS_ENTER_CRITICAL(); \
        bmu_get_info(id)->VAR = value; \
        OS_EXIT_CRITICAL(); \
        RPAGE_RESTORE();\
    }
#endif

#ifndef BMU_VAR_FUNC_IMPL
#define BMU_FLAG_VAR_DEF(TYPE, VAR) \
        void bmu_flag_##VAR(BmuId id, TYPE flag); \
        void bmu_unflag_##VAR(BmuId id, TYPE flag);
#else
#define BMU_FLAG_VAR_DEF(TYPE, VAR) \
        void bmu_flag_##VAR(BmuId id, TYPE flag) \
        { \
            OS_CPU_SR cpu_sr = 0; \
            RPAGE_INIT();\
            RPAGE_SAVE();\
            OS_ENTER_CRITICAL(); \
            bmu_get_info(id)->VAR |= flag; \
            OS_EXIT_CRITICAL(); \
            RPAGE_RESTORE();\
        } \
        void bmu_unflag_##VAR(BmuId id, TYPE flag) \
        { \
            OS_CPU_SR cpu_sr = 0; \
            RPAGE_INIT();\
            RPAGE_SAVE();\
            OS_ENTER_CRITICAL(); \
            bmu_get_info(id)->VAR &= ~(flag); \
            OS_EXIT_CRITICAL(); \
            RPAGE_RESTORE(); \
        }
#endif

#define MOVE_RIGHT(ITEMS, NUM, START) \
do{ \
    if (NUM < 2) break; \
    pos = (NUM - 1) - 1; \
    while (pos >= START) \
    { \
        ITEMS[pos + 1] = ITEMS[pos]; \
        if (pos == 0) break;\
        --pos; \
    } \
}while(0)

BmuInfo* _PAGED bmu_get_info(BmuId id);

//BMU_VAR_FUNC_DEF(INT8U, voltage_num)
//BMU_VAR_FUNC_DEF(INT8U, temperature_num)
//BMU_VAR_FUNC_DEF(INT32U, balance_state)
BMU_VAR_FUNC_DEF(INT8U, pack_state)
BMU_VAR_FUNC_DEF(INT32U, heart_beat_tick)
BMU_VAR_FUNC_DEF(INT8U, high_volt_state)
BMU_VAR_FUNC_DEF(INT8U, low_volt_state)
BMU_VAR_FUNC_DEF(INT8U, high_temp_state)
BMU_VAR_FUNC_DEF(INT8U, low_temp_state)
BMU_VAR_FUNC_DEF(INT8U, commabort_state)
BMU_VAR_FUNC_DEF(INT8U, balance_num)
BMU_VAR_FUNC_DEF(INT32U, balance_except_state)

//BMU_FLAG_VAR_DEF(INT8U, pack_state)
BMU_FLAG_VAR_DEF(INT8U, relay_state)
BMU_VAR_FUNC_DEF(INT8U, relay_state)

void bmu_init(void);
void bmu_update(void);

INT8U bmu_get_max_voltage_num(void);
INT8U bmu_get_max_temperature_num(void);

void bmu_set_balance_state(BmuId id, INT8U group, INT8U value);
INT8U bmu_get_balance_state(BmuId id, INT8U group);

void bmu_set_heat_status(BmuId id, INT8U flag);
INT8U bmu_get_heat_status(BmuId id);
void bmu_set_cool_status(BmuId id, INT8U flag);
INT8U bmu_get_cool_status(BmuId id);

INT8U bmu_set_voltage_num(BmuId id, INT16U num);
INT16U bmu_get_voltage_num(BmuId id);

INT8U bmu_set_temperature_num(BmuId id, INT8U num);
INT8U bmu_get_temperature_num(BmuId id);

INT16U bmu_get_voltage_item(BmuId id, INT16U index);
void bmu_set_voltage_item(BmuId id, INT8U index, INT16U value);

INT8U bmu_get_temperature_item(BmuId id, INT8U index);
void bmu_set_temperature_item(BmuId id, INT8U index, INT8U value);

Result bmu_is_balance_state_actived(void);

INT16U bmu_get_balance_current_item(BmuId id, INT8U index);
void bmu_set_balance_current_item(BmuId id, INT8U index, INT16U value);

void bmu_stat_refersh(void);

INT8U bmu_stat_get_high_temperature_count();
BmuTemperatureItem* _PAGED bmu_stat_get_high_temperature_info(int index);
INT8U bmu_stat_get_high_temperature(int index);
INT8U bmu_stat_get_high_temperature_id(int index);
INT8U bmu_stat_get_high_temperature_index(int index);

INT8U bmu_stat_get_low_temperature_count();
BmuTemperatureItem* _PAGED bmu_stat_get_low_temperature_info(int index);
INT8U bmu_stat_get_low_temperature(int index);
INT8U bmu_stat_get_low_temperature_id(int index);
INT8U bmu_stat_get_low_temperature_index(int index);

INT8U bmu_stat_get_high_voltage_count();
BmuVoltageItem* _PAGED bmu_stat_get_high_voltage_info(int index);
INT16U bmu_stat_get_high_voltage(int index);
INT8U bmu_stat_get_high_voltage_id(int index);
INT8U bmu_stat_get_high_voltage_index(int index);

INT8U bmu_stat_get_low_voltage_count();
BmuVoltageItem* _PAGED bmu_stat_get_low_voltage_info(int index);
INT16U bmu_stat_get_low_voltage(int index);
INT8U bmu_stat_get_low_voltage_id(int index);
INT8U bmu_stat_get_low_voltage_index(int index);

/**
 *  Inside Functions
 *  Don't call these functions in other places
 */
void bmu_stat_high_temperature(BmuId id, INT8U index);
void bmu_stat_low_temperature(BmuId id, INT8U index);
void bmu_stat_high_voltage(BmuId id, INT8U index);
void bmu_stat_low_voltage(BmuId id, INT8U index);

void bmu_stat_refersh_voltages(void);
void bmu_stat_refersh_temperatures(void);

INT8U bmu_get_max_temp(INT8U id);
INT8U bmu_get_min_temp(INT8U id);

BOOLEAN bmu_is_valid_voltage(INT16U voltage);
BOOLEAN bmu_is_valid_temperature(INT8U temperature);
BOOLEAN charger_is_valid_temperature(INT8U temperature);

BOOLEAN bmu_is_online(BmuId id);
void bmu_reset_offline(BmuId id);

void start_bmu_heart_beat_tick(INT8U id);
void bmu_heart_beat_tick_update(INT8U num);
INT8U bmu_balance_is_on(INT8U id);

INT32U bmu_get_total_voltage(INT8U id);
void bmu_get_low_voltage(INT8U id, INT16U* voltage, INT8U* addr);
void bmu_get_high_voltage(INT8U id, INT16U* voltage, INT8U* addr);
void bmu_get_low_temperature(INT8U id, INT8U* temperature, INT8U* pos);
void bmu_get_high_temperature(INT8U id, INT8U* temperature, INT8U* pos);
INT8U bmu_self_check(INT8U id);

INT16U bmu_get_delta_voltage_max(INT8U id); //��ȡ�ӻ�ѹ�����ֵ
INT8U bmu_get_delta_temperature_max(INT8U id); //��ȡ�ӻ��²����ֵ

INT8U bmu_set_total_temperature_num(BmuId id, INT8U num);//��������¶ȸ����ͼ������¶ȸ���
INT8U bmu_get_total_temperature_num(BmuId id);//��������¶ȸ����ͼ������¶ȸ���
INT8U bmu_set_heat_temperature_num(BmuId id, INT8U num);
INT8U bmu_get_heat_temperature_num(BmuId id);
void bmu_set_heat_temperature_item(BmuId id, INT8U index, INT8U value);
INT8U bmu_get_heat_temperature_item(BmuId id, INT8U index);
INT8U bmu_temperature_is_heat_temperature(BmuId id, INT16U pos);
INT8U bmu_heat_temp_pos_2_num(BmuId id, INT8U pos); //λ��ת������ pos:1,2...  num:0,1...
INT8U bmu_heat_temp_num_2_pos(BmuId id, INT8U num); //������תλ�� pos:1,2...  num:0,1...
INT8U bmu_temperature_pos_2_num(BmuId id, INT8U pos); //λ��ת������ pos:1,2...  num:0,1...
INT8U bmu_get_high_heat_temperature(BmuId id);
INT8U bmu_get_low_heat_temperature(BmuId id);

#if BMU_CHR_OUTLET_TEMP_SUPPORT
INT8U bmu_temperature_is_chr_outlet_temperature(BmuId id, INT16U pos);
void bmu_set_chr_outlet_temperature_item(BmuId id, INT8U index, INT8U value);
INT8U bmu_get_chr_outlet_temperature_item(BmuId id, INT8U index);
INT8U bmu_outlet_temp_pos_2_num(BmuId id, INT8U pos);
INT8U bmu_set_chr_outlet_temperature_num(BmuId id, INT8U num);
INT8U bmu_get_chr_outlet_temperature_num(BmuId id);
INT8U bmu_get_high_chr_outlet_temperature(BmuId id);
#endif

#if BCU_CONTROL_SLAVE_RELAY_INTERVAL_EN
void bmu_slave_cool_control_update(INT8U slave, INT8U relay_flag);
void bmu_cool_control_check(void* pdata);
void bmu_cool_control_set_on(INT8U slave);
void bmu_cool_control_set_off(INT8U slave);
INT8U bmu_cool_control_is_on(INT8U slave);

void bmu_slave_heat_control_update(INT8U slave, INT8U relay_flag);
void bmu_heat_control_check(void* pdata);
void bmu_heat_control_set_on(INT8U slave);
void bmu_heat_control_set_off(INT8U slave);
INT8U bmu_heat_control_is_on(INT8U slave);
#endif

#endif /* BMS_BMU_H_ */
