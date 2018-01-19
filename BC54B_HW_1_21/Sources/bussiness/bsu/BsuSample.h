 /*******************************************************************************
 **                       安徽力高新能源技术有限公司 Copyright (c)
 **                            http://www...
 **文 件 名:BsuSample.h
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

#define BSU_VOLT_TEMP_SAMPLE_TASK_PRODIC     500//采集周期 
#define BSU_VOLT_FILTER_NUM                  10//排线脱落或通信中断滤波次数
#define BSU_TEMP_FILTER_NUM                  5//排线脱落或通信中断滤波次数
#define BSU_MAX_COMM_ERROR_CNT               10//通信中断计数



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
	kBoardPassive = 1,//被动均衡类型
	kBoardActive = 2,//主动均衡类型
} BsuBoardType; 



typedef struct
{   
   BsuTempSampleItem  temp_item; //温度项
   BsuVoltSampleItem  volt_item;//电压项
   INT8U heart_beat[PARAM_BSU_NUM_MAX];//通信心跳
   BsuBoardType board_type[PARAM_BSU_NUM_MAX];//板子类型
   Ltc6804CommErrorByte error_byte[PARAM_BSU_NUM_MAX];//通信异常标志位
}BsuSampleContext;


//设置从机1号继电器开启
void set_bsu_relay1_on(INT8U bsu_index);
//设置从机1号继电器关闭
void set_bsu_relay1_off(INT8U bsu_index);
//设置从机2号继电器开启
void set_bsu_relay2_on(INT8U bsu_index);
//设置从机2号继电器关闭
void set_bsu_relay2_off(INT8U bsu_index);
//获取从机电池温度
INT8U get_bsu_cells_temp(INT8U bsu_index, INT8U pos);
//获取从机电池电压
INT16U get_bsu_cells_volt(INT8U bsu_index, INT8U pos);
//从机N是否通信中断
BOOLEAN is_bsu_n_comm_error(INT8U bsu_index);
//所有的是否通信中断
BOOLEAN is_all_bsu_comm_error(void);
//获取从机的电池数
INT8U get_bsu_n_cells_num(INT8U bsu_index);
//获取从机底层芯片的电池数
INT8U get_bsu_n_bottom_cells_num(INT8U bsu_index);
//所有的从机是否通信异常
BOOLEAN is_bsu_comm_error(void);
/*函数功能:获取从机排线脱落标志,bit0代表1号电池的排线有脱落情况*/
INT16U get_bsu_n_volt_sample_cable_open_flag(INT8U bsu_index);
/*函数功能:从机排线脱落*/
BOOLEAN is_volt_sample_cable_open(void);
//任务创建
void BSUVoltTempSampleTask_Init(void);
#endif
#endif  