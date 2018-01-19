 /*******************************************************************************
 **                       安徽力高新能源技术有限公司 Copyright (c)
 **                            http://www...
 **文 件 名:bsu_sample.h
 **创建日期:201612.15
 **文件说明:6804从机采集
 **修改记录:
 **版    本:V1.0
 **备    注:
 *******************************************************************************/
 
#ifndef   _BSU_SAMPLE_H_ 
#define   _BSU_SAMPLE_H_

#include "includes.h"
#include "ltc68041_impl.h" 
#if BMS_SUPPORT_HARDWARE_LTC6804 == 1

#define BSU_VOLT_TEMP_SAMPLE_TASK_PRODIC     300//采集周期 
#define BSU_VOLT_FILTER_NUM                  10//排线脱落或通信中断滤波次数
#define BSU_TEMP_FILTER_NUM                  10//排线脱落或通信中断滤波次数
#define BSU_MAX_COMM_ERROR_CNT               20//通信中断计数


#define BSU_PASSIVE_BALANCE_TYPE    1  //被动均衡
#define BSU_ACTIVE_BALANCE_TYPE 2  //主动均衡



#define  LTC6804_CELL_MAX_VOLT     4500
typedef struct
{   
   INT16U volt[PARAM_BSU_NUM_MAX][LTC6804_MAX_CELL_NUM]; //最终电池电压数组
   INT16U volt_buffer[PARAM_BSU_NUM_MAX][LTC6804_MAX_CELL_NUM];//本次采集的临时电池电压数组
   INT16U cable_open_flag[PARAM_BSU_NUM_MAX]; //排线脱落标志
   INT8U filter[PARAM_BSU_NUM_MAX][LTC6804_MAX_CELL_NUM]; //滤波计数  
}BsuVoltSampleItem;

typedef struct
{   
   INT8U temp[PARAM_BSU_NUM_MAX][LTC6804_MAX_TEMP_NUM]; //最终温度
   INT8U temp_buffer[PARAM_BSU_NUM_MAX][LTC6804_MAX_TEMP_NUM];//临时温度
   INT8U filter[PARAM_BSU_NUM_MAX][LTC6804_MAX_TEMP_NUM]; //滤波计数  
}BsuTempSampleItem;

typedef enum
{
	kBoardUnknow = 0,//未知类型
	kBoardMain = 1,//主从机
	kBoardSlave = 2,//从从机
} BsuBoardType; 



typedef struct
{   
   BsuTempSampleItem  temp_item; //温度项
   BsuVoltSampleItem  volt_item;//电压项
   INT8U heart_beat[PARAM_BSU_NUM_MAX];//通信心跳
   BsuBoardType board_type[PARAM_BSU_NUM_MAX];//板子类型
   Ltc6804CommErrorByte error_byte[PARAM_BSU_NUM_MAX];//通信异常标志位
   INT32U board_gpio_init_flag;//IO初始化标志
   BOOLEAN sample_once_finish_flag;
}BsuSampleContext;
//获取采集板的类型
BsuBoardType get_bsu_n_board_type(INT8U bsu_index);
//获取采集的数目
INT8U get_bsu_num(void);
//获取采集板电池温度
INT8U get_bsu_cells_temp(INT8U bsu_index, INT8U pos);
//获取采集板电池电压
INT16U get_bsu_cells_volt(INT8U bsu_index, INT8U pos);
//采集板N是否通信中断
BOOLEAN is_bsu_n_comm_error(INT8U bsu_index);
//所有的采集板是否通信中断
BOOLEAN is_all_bsu_comm_error(void);
//获取采集板的电池数
INT8U get_bsu_n_cells_num(INT8U bsu_index);
//获取采集板的温度数
INT8U get_bsu_n_temp_num(INT8U bsu_index);
//所有的采集板是否通信异常
BOOLEAN is_bsu_comm_error(void);
/*函数功能:获取采集板排线脱落标志,bit0代表1号电池的排线有脱落情况*/
INT16U get_bsu_n_volt_sample_cable_open_flag(INT8U bsu_index);
/*函数功能:采集板排线脱落*/
BOOLEAN is_volt_sample_cable_open(void);
/*函数功能:获取第一次采集完成标志*/
BOOLEAN get_bsu_sample_once_finish_flag(void);
//任务创建
void BSUVoltTempSampleTask_Init(void);
#endif
#endif  
