 /*******************************************************************************
 **                       ������������Դ�������޹�˾ Copyright (c)
 **                            http://www...
 **�� �� ��:bsu_sample.h
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

#define BSU_VOLT_TEMP_SAMPLE_TASK_PRODIC     300//�ɼ����� 
#define BSU_VOLT_FILTER_NUM                  10//���������ͨ���ж��˲�����
#define BSU_TEMP_FILTER_NUM                  10//���������ͨ���ж��˲�����
#define BSU_MAX_COMM_ERROR_CNT               20//ͨ���жϼ���


#define BSU_PASSIVE_BALANCE_TYPE    1  //��������
#define BSU_ACTIVE_BALANCE_TYPE 2  //��������



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
	kBoardMain = 1,//���ӻ�
	kBoardSlave = 2,//�Ӵӻ�
} BsuBoardType; 



typedef struct
{   
   BsuTempSampleItem  temp_item; //�¶���
   BsuVoltSampleItem  volt_item;//��ѹ��
   INT8U heart_beat[PARAM_BSU_NUM_MAX];//ͨ������
   BsuBoardType board_type[PARAM_BSU_NUM_MAX];//��������
   Ltc6804CommErrorByte error_byte[PARAM_BSU_NUM_MAX];//ͨ���쳣��־λ
   INT32U board_gpio_init_flag;//IO��ʼ����־
   BOOLEAN sample_once_finish_flag;
}BsuSampleContext;
//��ȡ�ɼ��������
BsuBoardType get_bsu_n_board_type(INT8U bsu_index);
//��ȡ�ɼ�����Ŀ
INT8U get_bsu_num(void);
//��ȡ�ɼ������¶�
INT8U get_bsu_cells_temp(INT8U bsu_index, INT8U pos);
//��ȡ�ɼ����ص�ѹ
INT16U get_bsu_cells_volt(INT8U bsu_index, INT8U pos);
//�ɼ���N�Ƿ�ͨ���ж�
BOOLEAN is_bsu_n_comm_error(INT8U bsu_index);
//���еĲɼ����Ƿ�ͨ���ж�
BOOLEAN is_all_bsu_comm_error(void);
//��ȡ�ɼ���ĵ����
INT8U get_bsu_n_cells_num(INT8U bsu_index);
//��ȡ�ɼ�����¶���
INT8U get_bsu_n_temp_num(INT8U bsu_index);
//���еĲɼ����Ƿ�ͨ���쳣
BOOLEAN is_bsu_comm_error(void);
/*��������:��ȡ�ɼ������������־,bit0����1�ŵ�ص��������������*/
INT16U get_bsu_n_volt_sample_cable_open_flag(INT8U bsu_index);
/*��������:�ɼ�����������*/
BOOLEAN is_volt_sample_cable_open(void);
/*��������:��ȡ��һ�βɼ���ɱ�־*/
BOOLEAN get_bsu_sample_once_finish_flag(void);
//���񴴽�
void BSUVoltTempSampleTask_Init(void);
#endif
#endif  
