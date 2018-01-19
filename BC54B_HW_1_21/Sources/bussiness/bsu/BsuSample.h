 /*******************************************************************************
 **                       ������������Դ�������޹�˾ Copyright (c)
 **                            http://www...
 **�� �� ��:BsuSample.h
 **��������:201612.15
 **�ļ�˵��:6804�ӻ��ɼ�
 **�޸ļ�¼:
 **��    ��:V1.0
 **��    ע:
 *******************************************************************************/
 
#ifndef   _BSU_SAMPLE_H_ 
#define   _BSU_SAMPLE_H_

#include "includes.h"
#include "ltc68041_impl.h" 
#if BMS_SUPPORT_HARDWARE_LTC6804 == 1

#define BSU_VOLT_TEMP_SAMPLE_TASK_PRODIC     500//�ɼ����� 
#define BSU_VOLT_FILTER_NUM                  10//���������ͨ���ж��˲�����
#define BSU_TEMP_FILTER_NUM                  5//���������ͨ���ж��˲�����
#define BSU_MAX_COMM_ERROR_CNT               10//ͨ���жϼ���



#define  LTC6804_CELL_MAX_VOLT     4500
typedef struct
{   
   INT16U volt[PARAM_BSU_NUM_MAX][LTC6804_MAX_CELL_NUM]; //���յ�ص�ѹ����
   INT16U volt_buffer[PARAM_BSU_NUM_MAX][LTC6804_MAX_CELL_NUM];//���βɼ�����ʱ��ص�ѹ����
   INT16U cable_open_flag[PARAM_BSU_NUM_MAX]; //���������־
   INT8U filter[PARAM_BSU_NUM_MAX][LTC6804_MAX_CELL_NUM]; //�˲�����  
}BsuVoltSampleItem;

typedef struct
{   
   INT8U temp[PARAM_BSU_NUM_MAX][LTC6804_MAX_TEMP_NUM]; //�����¶�
   INT8U temp_buffer[PARAM_BSU_NUM_MAX][LTC6804_MAX_TEMP_NUM];//��ʱ�¶�
   INT8U filter[PARAM_BSU_NUM_MAX][LTC6804_MAX_TEMP_NUM]; //�˲�����  
}BsuTempSampleItem;

typedef enum
{
	kBoardUnknow = 0,//δ֪����
	kBoardPassive = 1,//������������
	kBoardActive = 2,//������������
} BsuBoardType; 



typedef struct
{   
   BsuTempSampleItem  temp_item; //�¶���
   BsuVoltSampleItem  volt_item;//��ѹ��
   INT8U heart_beat[PARAM_BSU_NUM_MAX];//ͨ������
   BsuBoardType board_type[PARAM_BSU_NUM_MAX];//��������
   Ltc6804CommErrorByte error_byte[PARAM_BSU_NUM_MAX];//ͨ���쳣��־λ
}BsuSampleContext;


//���ôӻ�1�ż̵�������
void set_bsu_relay1_on(INT8U bsu_index);
//���ôӻ�1�ż̵����ر�
void set_bsu_relay1_off(INT8U bsu_index);
//���ôӻ�2�ż̵�������
void set_bsu_relay2_on(INT8U bsu_index);
//���ôӻ�2�ż̵����ر�
void set_bsu_relay2_off(INT8U bsu_index);
//��ȡ�ӻ�����¶�
INT8U get_bsu_cells_temp(INT8U bsu_index, INT8U pos);
//��ȡ�ӻ���ص�ѹ
INT16U get_bsu_cells_volt(INT8U bsu_index, INT8U pos);
//�ӻ�N�Ƿ�ͨ���ж�
BOOLEAN is_bsu_n_comm_error(INT8U bsu_index);
//���е��Ƿ�ͨ���ж�
BOOLEAN is_all_bsu_comm_error(void);
//��ȡ�ӻ��ĵ����
INT8U get_bsu_n_cells_num(INT8U bsu_index);
//��ȡ�ӻ��ײ�оƬ�ĵ����
INT8U get_bsu_n_bottom_cells_num(INT8U bsu_index);
//���еĴӻ��Ƿ�ͨ���쳣
BOOLEAN is_bsu_comm_error(void);
/*��������:��ȡ�ӻ����������־,bit0����1�ŵ�ص��������������*/
INT16U get_bsu_n_volt_sample_cable_open_flag(INT8U bsu_index);
/*��������:�ӻ���������*/
BOOLEAN is_volt_sample_cable_open(void);
//���񴴽�
void BSUVoltTempSampleTask_Init(void);
#endif
#endif  