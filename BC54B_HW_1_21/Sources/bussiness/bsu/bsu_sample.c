/*******************************************************************************
 **                       安徽力高新能源技术有限公司 Copyright (c)
 **                            http://www...
 **文 件 名:bsu_sample.c
 **创建日期:2015.10.15
 **文件说明:电压温度采集、均衡控制
 **修改记录:
 **版    本:V1.0
 **备    注:
 *******************************************************************************/
#include "bms_bmu.h"
#include "bsu_sample.h"
#include "ltc_util.h"
#include "bms_bsu.h"
#include "run_mode.h"
#include "bsu_active_balance.h"
#include "bsu_passive_balance.h"
#include "main_bsu_relay_ctrl.h"
#pragma MESSAGE DISABLE C5919
#if BMS_SUPPORT_HARDWARE_LTC6804 == 1


static OS_STK BsuSampleTaskStack[AD_SAMPLE_TASK_STK_SIZE] = {0}; //电压温度采集任务堆栈

#pragma DATA_SEG __RPAGE_SEG PAGED_RAM_BYU_SAMPLE
//采集信息
static BsuSampleContext bsu_sample_context;

#pragma DATA_SEG DEFAULT 

#pragma DATA_SEG __RPAGE_SEG PAGED_RAM_BSU 
//上拉采集的电池电压
static INT16U PUVoltTmpBuffer[PARAM_BSU_NUM_MAX][LTC6804_MAX_CELL_NUM];
//下拉采集的电池电压
static INT16U PDVoltTmpBuffer[PARAM_BSU_NUM_MAX][LTC6804_MAX_CELL_NUM];
static INT16U AuxVoltBuffer[AUX_BUFF_LEN][PARAM_BSU_NUM_MAX]; //温度电压临时缓存
#pragma DATA_SEG DEFAULT
//函数申明
static void bsu_context_init(BsuSampleContext* __far context);
void BsuVoltTempSampleTask(void *pdata);
/*****************************************************************************
 *函数名称:BSUVoltTempSampleTask_Init
 *函数功能:创建任务
 *参    数:
 *返 回 值:无
 *修订信息:
 ******************************************************************************/
void BSUVoltTempSampleTask_Init(void)
{
	volatile INT8U err = 0;
	MODRR_MODRR5 = 1;     //使用PH0~PH3 
	(void) spi_init(&ltc6804_high_speed_spi_bus, 1); //spi初始化
	bsu_context_init(&bsu_sample_context); //数据初始化
	watch_dog_register(WDT_AD_ID, WDT_AD_TIME); //看门狗注册
	err = OSTaskCreate(BsuVoltTempSampleTask, //任务函数
			(void *) 0, //参数
			(OS_STK *) &BsuSampleTaskStack[AD_SAMPLE_TASK_STK_SIZE - 1], //堆栈
			AD_SAMPLE_TASK_PRIO); //优先级
	if (err != OS_ERR_NONE)
	{
		for (;;)
		{
		}
	}
}
/*****************************************************************************
 *函数名称:get_bsu_n_cells_num
 *函数功能:获取参数,第N个采集板的电池数
 *参    数:bsu_index  采集板号(从0开始)
 *返 回 值:对应采集板的电池数
 *修订信息:
 ******************************************************************************/
INT8U get_bsu_n_cells_num(INT8U bsu_index)
{
	INT8U num = 0;
	INT16U tmp_16u = 0;
	//参数检查
	if (bsu_index >= PARAM_BSU_NUM_MAX)
		return 0;
	tmp_16u =  config_get(bsu_index + kBYUVoltTempCnt1Index);
	tmp_16u = tmp_16u&0xFF00;
	num = (INT8U)(tmp_16u>>8);
    if((num < LTC6804_HALF_CELL_NUM)&&(num !=0))
       num =  LTC6804_HALF_CELL_NUM;
    if(num > LTC6804_MAX_CELL_NUM)
       num =  LTC6804_MAX_CELL_NUM;
	return num;
}
/*****************************************************************************
 *函数名称:get_bsu_n_temp_num
 *函数功能:获取参数,第N个采集板的温度数
 *参    数:bsu_index  采集板号(从0开始)
 *返 回 值:对应采集板的温度数
 *修订信息:
 ******************************************************************************/
INT8U get_bsu_n_temp_num(INT8U bsu_index)
{
	INT8U num = 0;
	INT16U tmp_16u = 0;
	//参数检查
	if (bsu_index >= PARAM_BSU_NUM_MAX)
		return 0;
	tmp_16u =  config_get(bsu_index + kBYUVoltTempCnt1Index);
	tmp_16u = tmp_16u&0x00FF;
	num = (INT8U)tmp_16u;
	if (num > LTC6804_MAX_TEMP_NUM)
	{
		num = LTC6804_MAX_TEMP_NUM;
	}
	return num;
}
/*****************************************************************************
 *函数名称:get_bsu_num
 *函数功能:获取bsu数目，不使用的BSU电池数目必须配置为0
 *参    数:无
 *返 回 值:采集板数
 *修订信息:
 ******************************************************************************/
INT8U get_bsu_num(void)
{
	INT8U i = 0;

	for (i = 0; i < PARAM_BSU_NUM_MAX; i++)
	{
		if ((get_bsu_n_cells_num(i) == 0) || (get_bsu_n_cells_num(i) > LTC6804_MAX_CELL_NUM))
			return i;
	}
	return i;
}
/*****************************************************************************
 *函数名称:bsu_context_init
 *函数功能:采集板BSU数据初始化
 *参    数:context采集板采集数据内容
 *返 回 值:无
 *修订信息:
 ******************************************************************************/
static void bsu_context_init(BsuSampleContext* __far context)
{
	INT8U index = 0;
	INT8U bsu_index = 0; //采集板号索引
	//参数检查
	if(context == NULL)
		return;
	//电压清零
	ltc6804_buffer_set(&context->volt_item,0,sizeof(BsuVoltSampleItem));
	//心跳置最大值
	ltc6804_buffer_set(context->heart_beat,BSU_MAX_COMM_ERROR_CNT,PARAM_BSU_NUM_MAX);
	//板子类型
	ltc6804_buffer_set(&context->board_type,kBoardUnknow,PARAM_BSU_NUM_MAX);
	context->board_gpio_init_flag = 0;
	context->sample_once_finish_flag = FALSE;
	for (bsu_index = 0; bsu_index < PARAM_BSU_NUM_MAX; bsu_index++)
	{
		context->error_byte[bsu_index].value = 0;
		//温度数据
		for (index = 0; index < LTC6804_MAX_TEMP_NUM; index++)
		{
			context->temp_item.temp[bsu_index][index] = TEMP_CABLE_OPEN_VAULE;	//温度置0xFF
			context->temp_item.temp_buffer[bsu_index][index] = 0;
			context->temp_item.filter[bsu_index][index] = 0;
		}
	}
}
/*****************************************************************************
 *函数名称:set_sample_once_finish_flag
 *函数功能:设置第一次采集完成标志
 *参    数:context采集板采集数据内容
 *返 回 值:无
 *修订信息:
 ******************************************************************************/
static void set_sample_once_finish_flag(BsuSampleContext* __far context)
{
	if(context == NULL)
			return;
	context->sample_once_finish_flag = TRUE;
}
/*****************************************************************************
 *函数名称:get_bsu_sample_once_finish_flag
 *函数功能:获取第一次采集完成标志
 *参    数:无
 *返 回 值:第一次采集完成标志 0未完成，1采集完成
 *修订信息:
 ******************************************************************************/
BOOLEAN get_bsu_sample_once_finish_flag(void)
{
	BsuSampleContext* __far context = &bsu_sample_context;

	return context->sample_once_finish_flag;
}
/*****************************************************************************
 *函数名称:get_bsu_cells_volt
 *函数功能:获取电池电压
 *参    数:bsu_index采集板号,pos电池在采集板中的位置
 *返 回 值:电池电压1mv/bit
 *修订信息:
 ******************************************************************************/
INT16U get_bsu_cells_volt(INT8U bsu_index, INT8U pos)
{
	OS_CPU_SR cpu_sr = 0;
	INT16U volt = 0;
	const BsuVoltSampleItem *__far item = &bsu_sample_context.volt_item;
	//参数检查
	if ((bsu_index >= PARAM_BSU_NUM_MAX) || (pos >= LTC6804_MAX_CELL_NUM))
		return 0;

	OS_ENTER_CRITICAL();
	volt = item->volt[bsu_index][pos];
	OS_EXIT_CRITICAL();

	return volt;
}
/*****************************************************************************
 *函数名称:get_bsu_cells_temp
 *函数功能:获取采集板电池温度
 *参    数:INT8U bsu_index采集板号,INT8U pos温感编号
 *返 回 值:电池温度  
 *修订信息:
 ******************************************************************************/
INT8U get_bsu_cells_temp(INT8U bsu_index, INT8U pos)
{
	OS_CPU_SR cpu_sr = 0;
	INT8U temp = 0;
	const BsuTempSampleItem *__far item = &bsu_sample_context.temp_item;
	//参数检查
	if ((bsu_index >= PARAM_BSU_NUM_MAX) || (pos >= LTC6804_MAX_TEMP_NUM))
		return 0;

	OS_ENTER_CRITICAL();
	temp = item->temp[bsu_index][pos];
	OS_EXIT_CRITICAL();

	return temp;
}

/*****************************************************************************
 *函数名称:clr_all_bsu_comm_error_byte
 *函数功能:清除所有通信异常位
 *参    数:无
 *返 回 值:无
 *修订信息:
 ******************************************************************************/
static void clr_all_bsu_comm_error_byte(void)
{
	INT8U bsu_index = 0;
	const BsuSampleContext* __far context = &bsu_sample_context;
	for (bsu_index = 0; bsu_index < PARAM_BSU_NUM_MAX; bsu_index++)
	{
		context->error_byte[bsu_index].value = 0;
	}
}
/*****************************************************************************
 *函数名称:bsu_comm_heart_beat_run
 *函数功能:采集板通信中断心跳
 *参    数:context采集板电压温度信息
 *返 回 值:无
 *修订信息:
 ******************************************************************************/
static void bsu_comm_heart_beat_run(BsuSampleContext *__far context)
{
	INT8U bsu_index = 0;
	INT8U bsu_num = 0;
	OS_CPU_SR cpu_sr = 0;

	if(context == NULL)
	return;

	bsu_num = get_bsu_num(); //采集板数
	for (bsu_index = 0; bsu_index < bsu_num; bsu_index++)
	{
		OS_ENTER_CRITICAL();
		if (context->error_byte[bsu_index].Bits.read_cv) //读电压通信异常
		{
			context->heart_beat[bsu_index]++;  //计数增加
			if (context->heart_beat[bsu_index] >= BSU_MAX_COMM_ERROR_CNT)//达到最大值
			{
				context->heart_beat[bsu_index] = BSU_MAX_COMM_ERROR_CNT; //采集板通信中断
				context->volt_item.cable_open_flag[bsu_index] = 0;
				clear_bsu_active_balance_current(bsu_index);
			}
		}
		else
		{
			context->heart_beat[bsu_index] = 0;
		}
		OS_EXIT_CRITICAL();
	}
	//未使用的采集板全部标记为通信中断
	for (; bsu_index < PARAM_BSU_NUM_MAX; bsu_index++)
	{
		OS_ENTER_CRITICAL();
		context->heart_beat[bsu_index] = BSU_MAX_COMM_ERROR_CNT;
		OS_EXIT_CRITICAL();
		clear_bsu_active_balance_current(bsu_index);
	}
	clr_all_bsu_comm_error_byte();
}
/*****************************************************************************
 *函数名称:is_bsu_n_comm_error
 *函数功能:判断第N个采集板是否通信失败
 *参    数:bsu_index采集板编号
 *返 回 值:FALSE通信正常 TRUE通信失败
 *修订信息:
 ******************************************************************************/
BOOLEAN is_bsu_n_comm_error(INT8U bsu_index)
{
	OS_CPU_SR cpu_sr = 0;
	const INT8U *__far heartbeat = bsu_sample_context.heart_beat;

	if (bsu_index >= get_bsu_num())
		return FALSE;

	OS_ENTER_CRITICAL();
	if (heartbeat[bsu_index] >= BSU_MAX_COMM_ERROR_CNT)
	{   //通信中断
		OS_EXIT_CRITICAL();
		return TRUE;
	}
	OS_EXIT_CRITICAL();

	return FALSE; //正常
}

/*****************************************************************************
 *函数名称:is_all_bsu_comm_error
 *函数功能:判断所有的采集板是否通信失败
 *参    数:无
 *返 回 值:FALSE通信正常 TRUE通信失败
 *修订信息:
 ******************************************************************************/
BOOLEAN is_all_bsu_comm_error(void)
{
	INT8U bsu_index = 0;
	INT8U bsu_num = 0;

	bsu_num = get_bsu_num();
	for (bsu_index = 0; bsu_index < bsu_num; bsu_index++)
	{
		if (is_bsu_n_comm_error(bsu_index) == FALSE)
		{
			return FALSE;
		}
	}
	return TRUE;
}

/*****************************************************************************
 *函数名称:is_bsu_comm_error
 *函数功能:判断是否有采集板通信失败
 *参    数:无
 *返 回 值:FALSE通信正常 TRUE通信失败
 *修订信息:
 ******************************************************************************/
BOOLEAN is_bsu_comm_error(void)
{
	INT8U bsu_index = 0;
	INT8U bsu_num = 0;

	bsu_num = get_bsu_num();
	for (bsu_index = 0; bsu_index < bsu_num; bsu_index++)
	{
		if (is_bsu_n_comm_error(bsu_index))
		{
			return TRUE;
		}
	}
	return FALSE;
}

/*****************************************************************************
 *函数名称:is_cell_volt_sample_cable_open
 *函数功能:电压检测排线开路
 *参    数:INT8U bsu_index采集板编号, INT8U cell_index电池位置
 *返 回 值:FALSE无脱落,TRUE脱落
 *修订信息:
 ******************************************************************************/
BOOLEAN is_cell_volt_sample_cable_open(INT8U bsu_index, INT8U cell_index)
{
	OS_CPU_SR cpu_sr = 0;
	const BsuVoltSampleItem *__far item = &bsu_sample_context.volt_item;

	if ((bsu_index >= get_bsu_num()) || (cell_index >= LTC6804_MAX_CELL_NUM))
		return FALSE;

	OS_ENTER_CRITICAL();
	if (GET(item->cable_open_flag[bsu_index], cell_index))
	{
		OS_EXIT_CRITICAL();
		return TRUE;
	}
	OS_EXIT_CRITICAL();

	return FALSE;
}
/*****************************************************************************
 *函数名称:get_bsu_n_volt_sample_cable_open_flag
 *函数功能:获取采集板排线脱落标志,bit0代表1号电池的排线有脱落情况
 *参    数:INT8U bsu_index采集板编号
 *返 回 值:排线脱落标志
 *修订信息:
 ******************************************************************************/
INT16U get_bsu_n_volt_sample_cable_open_flag(INT8U bsu_index)
{
	OS_CPU_SR cpu_sr = 0;
	INT16U flag = 0;
	const BsuVoltSampleItem *__far item = &bsu_sample_context.volt_item;

	if (bsu_index >= get_bsu_num())
		return 0;

	OS_ENTER_CRITICAL();
	flag = item->cable_open_flag[bsu_index];
	OS_EXIT_CRITICAL();

	return flag;
}
/*****************************************************************************
 *函数名称:is_volt_sample_cable_open
 *函数功能:采集板排线脱落
 *参    数:无
 *返 回 值:TRUE有脱落,FALSE无脱落
 *修订信息:
 ******************************************************************************/
BOOLEAN is_volt_sample_cable_open(void)
{
	INT8U bsu_index = 0;
	INT8U bsu_num = 0;

	bsu_num = get_bsu_num();
	for (bsu_index = 0; bsu_index < bsu_num; bsu_index++)
	{
		if (get_bsu_n_volt_sample_cable_open_flag(bsu_index))
		{
			return TRUE;
		}
	}
	return FALSE;
}
/*****************************************************************************
 *函数名称:get_bsu_n_board_type
 *函数功能:获取板子类型
 *参    数:INT8U bsu_index 采集板编号
 *返 回 值:板子类型 主采集板 从采集板 未知类型
 *修订信息:
 ******************************************************************************/
BsuBoardType get_bsu_n_board_type(INT8U bsu_index)
{
    const BsuBoardType *__far board_type = bsu_sample_context.board_type;

	if (bsu_index >= get_bsu_num())
		return kBoardUnknow;

	return board_type[bsu_index];
}
/*****************************************************************************
 *函数名称:get_temp_filter_cnt
 *函数功能:获取温度滤波计数
 *参    数:bsu_index采集板编号, index温度号
 *返 回 值:无
 *修订信息:
 ******************************************************************************/
INT8U get_temp_filter_cnt(INT8U bsu_index, INT8U index)
{
	OS_CPU_SR cpu_sr = 0;
	INT8U tmp_8u = 0;
	const BsuTempSampleItem *__far item = &bsu_sample_context.temp_item;

	if ((bsu_index >= get_bsu_num()) || (index >= LTC6804_MAX_TEMP_NUM))
		return BSU_TEMP_FILTER_NUM;

	OS_ENTER_CRITICAL();
	tmp_8u = item->filter[bsu_index][index];
	OS_EXIT_CRITICAL();

	return tmp_8u;
}
/*****************************************************************************
 *函数名称:get_bsu_avg_volt
 *函数功能:获取采集板平均电压
 *参    数:volt电压数组,num电池数
 *返 回 值:平均值
 *修订信息:
 ******************************************************************************/
static INT16U get_bsu_avg_volt(const INT16U *__far volt,INT8U num)
{
   INT32U sum = 0;
   INT16U avg_volt = 0;
   INT8U i = 0;
   if((num == 0)||(num > LTC6804_MAX_CELL_NUM)||(volt ==NULL))
      return 0 ;
   for(i==0;i<num;i++)
   {
      sum += volt[i];  
   }
   avg_volt =  (INT16U)(sum/num);
   return avg_volt;
}
/*****************************************************************************
 *函数名称:is_cells_volt_abnormal
 *函数功能:电压是否有异常
 *参    数:context
 *返 回 值:FALSE无异常 TRUE有异常
 *修订信息:
 ******************************************************************************/
static INT8U is_cells_volt_abnormal(BsuSampleContext *__far context)
{
	INT8U bsu_index = 0; //当前从机号
	INT8U pos = 0;
	INT8U bsu_num = 0;  //采集板数
	INT8U  bsu_n_cells_num = 0; //第N个采集板电池数
	static INT8U first_flag = 0;
	INT16U avg_volt = 0;
	
	if(context == NULL)
          return FALSE;
	bsu_num = get_bsu_num();
	for (bsu_index = 0; bsu_index < bsu_num; bsu_index++)
	{
		if (context->error_byte[bsu_index].Bits.read_cv)
		{
			continue;
		}
		bsu_n_cells_num = get_bsu_n_cells_num(bsu_index);
		avg_volt = get_bsu_avg_volt(context->volt_item.volt_buffer[bsu_index],bsu_n_cells_num);
		for (pos = 0; pos < bsu_n_cells_num; pos++)
		{
			if ((context->volt_item.volt_buffer[bsu_index][pos] > LTC6804_CELL_MAX_VOLT)||//异常
			(context->volt_item.volt_buffer[bsu_index][pos] < 500))
			{
				return TRUE;
			}
			if (context->volt_item.cable_open_flag[bsu_index] != 0)//已经有排线脱落
			{
				return TRUE;
			}
			if((pos+1) < bsu_n_cells_num)
			{
			    if(abs(context->volt_item.volt_buffer[bsu_index][pos] - context->volt_item.volt_buffer[bsu_index][pos+1]) >= 400)
			    {
			       return TRUE; 
			    }
			}
			if(first_flag == 0)
			{
			    continue;
			}
			//电压突变100mv
			if ((abs(context->volt_item.volt_buffer[bsu_index][pos] - context->volt_item.volt[bsu_index][pos]) >= 100)//前一次与后一次
			   &&(abs(avg_volt - context->volt_item.volt_buffer[bsu_index][pos]) > 50)) //本次与平均电压比较
			{
			    return TRUE;  
			}
		}
	}
	if(first_flag == 0)
	{
	   first_flag = 1;
	}
	return FALSE;
}
/*****************************************************************************
 *函数名称:cells_volt_sample_cable_open_diagnose
 *函数功能:电压排线开路诊断
 *参    数: INT16U (*__far PU_volt)[LTC6804_MAX_CELL_NUM] //上拉电压数组
         INT16U (*__far PD_volt)[LTC6804_MAX_CELL_NUM] //下拉电压数组
 *返 回 值:无
 *修订信息:
 ******************************************************************************/
static void cells_volt_sample_cable_open_diagnose(BsuSampleContext *__far context,const INT16U (*__far PU_volt)[LTC6804_MAX_CELL_NUM], const INT16U (*__far PD_volt)[LTC6804_MAX_CELL_NUM])
{
	INT16U volt_diff = 0;
	INT8U bsu_index = 0; //从机编号
	INT8U pos = 0;
	INT16U open_flag = 0;
	INT8U tmp_8u = 0;
	OS_CPU_SR cpu_sr = 0;
	INT8U  bsu_num = 0;       //从机数
	INT8U  bsu_n_cells_num = 0; //第N个从机电池数

    if((context == NULL)||(PU_volt == NULL)||(PD_volt == NULL))
          return;

	bsu_num = get_bsu_num();
	for (bsu_index = 0; bsu_index < bsu_num; bsu_index++)
	{
		if ((context->error_byte[bsu_index].Bits.read_pucv != 0) || //上拉采集通信正常
			(context->error_byte[bsu_index].Bits.read_pudv != 0)) //下拉采集通信正常
		{
			continue;
		}
        bsu_n_cells_num = get_bsu_n_cells_num(bsu_index);
		open_flag = 0;
		if (PU_volt[bsu_index][0] == 0)
		{
			for (pos = 0; pos < bsu_n_cells_num; pos++)
			{
			    SET(open_flag, 0); 
				context->volt_item.volt_buffer[bsu_index][pos] = 0;
			}
		}
		if (PD_volt[bsu_index][bsu_n_cells_num - 1] == 0)
		{ //
			SET(open_flag, (bsu_n_cells_num - 1));
			SET(open_flag, (bsu_n_cells_num - 2));
			//供电线脱落，数据不可靠清零
			context->volt_item.volt_buffer[bsu_index][(bsu_n_cells_num - 1)] = 0;
			context->volt_item.volt_buffer[bsu_index][(bsu_n_cells_num - 2)] = 0;
		}
		for (pos = 1; pos < bsu_n_cells_num; pos++)
		{
			volt_diff = PD_volt[bsu_index][pos] - PU_volt[bsu_index][pos];
			if ((volt_diff > 400) && (volt_diff < 0x8000))
			{ //排线附近2个电池电压无效
				SET(open_flag, pos);tmp_8u = pos - 1;
				SET(open_flag, tmp_8u);
			}
		}
		OS_ENTER_CRITICAL();
		context->volt_item.cable_open_flag[bsu_index] = open_flag;
		OS_EXIT_CRITICAL();
	}
}
/*****************************************************************************
 *函数名称:copy_cells_volt_with_filter
 *函数功能:将电池电压缓存buffer的数据拷贝到最终电压缓存volt中,无效电压均衡滤波10次清0
 *参    数:context
 *返 回 值:无
 *修订信息:
 ******************************************************************************/
static void copy_cells_volt_with_filter(BsuSampleContext *__far context)
{
	INT8U bsu_index = 0;
	INT8U pos = 0;
	OS_CPU_SR cpu_sr = 0;
    INT8U  bsu_num = 0;
    
    if(context == NULL)
          return;
    
	bsu_num = get_bsu_num();
	for (bsu_index = 0; bsu_index < bsu_num; bsu_index++)
	{
		if (context->heart_beat[bsu_index] > 0) //通信异常 没有读到数据
		{
			if (context->heart_beat[bsu_index] >= BSU_MAX_COMM_ERROR_CNT)
			{
				for (pos = 0; pos < LTC6804_MAX_CELL_NUM; pos++)
				{
					OS_ENTER_CRITICAL();
					context->volt_item.volt[bsu_index][pos] = 0;
					OS_EXIT_CRITICAL();
				}
			}
		}
		else //通信正常
		{
            for (pos = 0; pos < get_bsu_n_cells_num(bsu_index); pos++)
            {
                if ((GET(context->volt_item.cable_open_flag[bsu_index], pos))||
                (context->volt_item.volt_buffer[bsu_index][pos] > LTC6804_CELL_MAX_VOLT)||
                (context->volt_item.volt_buffer[bsu_index][pos] == 0))
                {
                    if (context->volt_item.filter[bsu_index][pos]++ > BSU_VOLT_FILTER_NUM)
                    {
                        OS_ENTER_CRITICAL();
                        context->volt_item.volt[bsu_index][pos] = 0;
                        OS_EXIT_CRITICAL();
                        context->volt_item.filter[bsu_index][pos] = BSU_VOLT_FILTER_NUM;
                    }
                }
                else //无异常直接复制
                {
                    OS_ENTER_CRITICAL();
                    if(context->volt_item.volt[bsu_index][pos] != 0)//增加电压一阶滤波 
                    {
                        context->volt_item.volt[bsu_index][pos] = (context->volt_item.volt[bsu_index][pos]+context->volt_item.volt_buffer[bsu_index][pos])>>1; 
                    }
                    else
                    {
                        context->volt_item.volt[bsu_index][pos] = context->volt_item.volt_buffer[bsu_index][pos]; 
                    }
                    if((pos == 0)||((pos+1) == get_bsu_n_cells_num(bsu_index)))
                    {
                       context->volt_item.volt[bsu_index][pos] +=3;  
                    }
                    OS_EXIT_CRITICAL();
                    context->volt_item.filter[bsu_index][pos] = 0;
                }
            }
            for (; pos < LTC6804_MAX_CELL_NUM; pos++)
            {
                OS_ENTER_CRITICAL();
                context->volt_item.volt[bsu_index][pos] = 0;
                OS_EXIT_CRITICAL();
            }
       }
    }
}
/*****************************************************************************
 *函数名称:temp_volt_adjust
 *函数功能:电压校准,硬件原因-7℃以下需要软件补偿，补偿电压+31mv
 *参    数:
 *返 回 值:
 *修订信息:
 ******************************************************************************/
static INT16U temp_volt_adjust(INT16U temp_volt, INT8U temp_index)
{
    const  FP32 RA1 = 1.880301E-04;
    const  FP32 RA2 = 5.045898E-02;
    const  FP32 RA3 = 1.191073E+03;
    
    const  FP32 RB1 = 1.551607E-04;
    const  FP32 RB2 = 2.389185E-01;
    const  FP32 RB3 = 9.312020E+02;
    if((temp_volt <= 260)&&(temp_volt > 0)) 
    {
         temp_volt =  temp_volt+4;
    } 
    else 
    {
        if(temp_index == 0) 
        {
          if (temp_volt > 2470)
                temp_volt =(INT16U) (RB1*temp_volt*temp_volt +  RB2*temp_volt + RB3);
        
        } 
        else 
        {
          if (temp_volt > 2482)
              temp_volt =(INT16U) (RA1*temp_volt*temp_volt +  RA2*temp_volt + RA3);
        }
    }
    return  temp_volt;
}
/*****************************************************************************
 *函数名称:get_ntc_resistance_vaule
 *函数功能:获取NTC温感的当前阻值,根据采集的电压计算出当前NTC的阻值
 *参    数:INT16U r_volt采集到电阻上的电压,INT16U ref_volt参考电压
 *返 回 值:电阻值(10Ω/bit)
 *修订信息:
 ******************************************************************************/
static INT16U get_ntc_resistance_vaule(INT16U r_volt, INT16U ref_volt,INT8U temp_index)
{
	INT16U R = 0xFFFF;
	INT32U tmp_32u = 0;
	
	r_volt=temp_volt_adjust(r_volt,temp_index);  //硬件原因，在温度低于-7度时补偿电压
	if (r_volt >= ref_volt)
	{
		R = 0xFFFF;
	}
	else
	{
		tmp_32u = ((INT32U) r_volt * 1000) / (ref_volt - r_volt);
		if (tmp_32u > 0xFFFF)
			R = 0xFFFF;
		else
			R = (INT16U) tmp_32u;
	}
	return R;
}

/*****************************************************************************
 *函数名称:convert_volt_to_temp
 *函数功能:将电压计算成温度
 *参    数:context,temp_index
 *返 回 值:无
 *修订信息:
 ******************************************************************************/
static void convert_volt_to_temp(BsuSampleContext *__far context,INT8U temp_index)
{
    INT8U bsu_index = 0;
    INT16U tmp_16u = 0;
    INT8U bsu_num = 0;
    INT8U flag = 0;
    if (temp_index >= LTC6804_MAX_TEMP_NUM)
        return;
    if(context == NULL)
        return;
    bsu_num = get_bsu_num();
    for (bsu_index = 0; bsu_index < bsu_num; bsu_index++)
    {
        switch(temp_index)
        {
            case 0:
                flag = context->error_byte[bsu_index].Bits.read_gpio1;
            break;
            case 1:
                flag = context->error_byte[bsu_index].Bits.read_gpio2;
            break;
            case 2:
                flag = context->error_byte[bsu_index].Bits.read_gpio3;
            break;
            case 3:
                flag = context->error_byte[bsu_index].Bits.read_gpio4;
            break;
            case 4:
                flag = context->error_byte[bsu_index].Bits.read_gpio5;
            break;
            default:
                return;
            break;
        }
        
        if (flag == 0)  //无异常
        {
            if(temp_index<3)
                tmp_16u = get_ntc_resistance_vaule(AuxVoltBuffer[temp_index][bsu_index], 3000,temp_index);
            else 
                tmp_16u = get_ntc_resistance_vaule(AuxVoltBuffer[temp_index-3][bsu_index], 3000,temp_index);
            context->temp_item.temp_buffer[bsu_index][temp_index] = ltc6804_temp_query(tmp_16u); //查表得出温度
        }
        else
        {
            context->temp_item.temp_buffer[bsu_index][temp_index] = TEMP_CABLE_OPEN_VAULE;
        }
    }
}
/*****************************************************************************
 *函数名称:copy_cells_temp_with_filter
 *函数功能:将温度缓存的数据拷贝到最终温度中,无效温度滤波n次后再拷贝
 *参    数:BsuVoltTempContext *__far context
 *返 回 值:无
 *修订信息:
 ******************************************************************************/
static void copy_cells_temp_with_filter(BsuSampleContext *__far context)
{
    INT8U bsu_index = 0;
    INT8U pos = 0;
    OS_CPU_SR cpu_sr = 0;
    INT8U bsu_num = 0;
    INT8U temp_num = 0;

    if(context == NULL)
        return;

    bsu_num = get_bsu_num();
    for (bsu_index = 0; bsu_index < bsu_num; bsu_index++)
    {
        if (context->heart_beat[bsu_index] > 0) //通信异常 没有读到数据
        {
            if (context->heart_beat[bsu_index] >= BSU_MAX_COMM_ERROR_CNT)
            {
                for (pos = 0; pos < LTC6804_MAX_TEMP_NUM; pos++)
                {
                	OS_ENTER_CRITICAL();
                	context->temp_item.temp[bsu_index][pos] = TEMP_CABLE_OPEN_VAULE;
                	OS_EXIT_CRITICAL();
                }
            }
        }
        else //通信正常
        {
            INT8U start_index = 0;
            INT8U end_index = 0;
            temp_num = get_bsu_n_temp_num(bsu_index);
        #if  BMS_SUPPORT_BY5248D == 0 
            if((config_get(kBYUBalTypeIndex)== BSU_PASSIVE_BALANCE_TYPE)//被动从机第一个采集板温度偏移不一样
               &&(get_bsu_n_board_type(bsu_index) == kBoardMain)) 
            {
                if(temp_num > 2)
                    temp_num = 2;
                start_index = 2;
                end_index = 2+temp_num;
            } 
            else 
            {
                start_index = 0;
                end_index = temp_num;
            }
       #else //板载6804
		    if(config_get(kBYUBalTypeIndex)== BSU_PASSIVE_BALANCE_TYPE)//被动从机第一个采集板温度偏移不一样
            {
                if(bsu_index < BMS_SUPPORT_LTC6804_ON_BOARD_CHIP_NUM)
                {
		    if (temp_num > 4)
					temp_num = 4;
				start_index = 0; //起始地址为0
				end_index = temp_num; //结束地址
                }
                else if(get_bsu_n_board_type(bsu_index) == kBoardMain)
                {
                    if(temp_num > 2)
                        temp_num = 2;
                    start_index = 2;
                    end_index = 2+temp_num;
                }
                else
                {
                    start_index = 0;
                    end_index = temp_num;
                }
            } 
            else 
            {
                start_index = 0;
                end_index = temp_num;
            }
      #endif
            for (pos = start_index; pos < end_index; pos++)
            {
                if ((context->temp_item.temp_buffer[bsu_index][pos] == TEMP_CABLE_OPEN_VAULE) || ( context->temp_item.temp_buffer[bsu_index][pos] == TEMP_CABLE_SHORT_VAULE))
                {    //数据异常
                	if (context->temp_item.filter[bsu_index][pos]++ > BSU_TEMP_FILTER_NUM)
                	{
                		OS_ENTER_CRITICAL();
                		context->temp_item.temp[bsu_index][pos] = context->temp_item.temp_buffer[bsu_index][pos];
                		OS_EXIT_CRITICAL();
                		context->temp_item.filter[bsu_index][pos] = BSU_TEMP_FILTER_NUM;
                	}
                }
                else //无异常直接复制
                {
                	OS_ENTER_CRITICAL();  //温度增加一阶滤波
                	if((context->temp_item.temp[bsu_index][pos] == TEMP_CABLE_OPEN_VAULE) || ( context->temp_item.temp[bsu_index][pos] == TEMP_CABLE_SHORT_VAULE))
                	{
                	    context->temp_item.temp[bsu_index][pos] = context->temp_item.temp_buffer[bsu_index][pos];
                	}
                	else
                	{
                	    context->temp_item.temp[bsu_index][pos] = (INT8U)(((INT16U)context->temp_item.temp[bsu_index][pos] + (INT16U)context->temp_item.temp_buffer[bsu_index][pos]) >> 1);
                	}                	
                	OS_EXIT_CRITICAL();
                	context->temp_item.filter[bsu_index][pos] = 0;
                }
            }
        }
    }
}
/*****************************************************************************
 *函数名称:active_board_temp_index_convert
 *函数功能:温度索引转换（硬件决定）
 *参    数:
 *返 回 值:无
 *修订信息:
 ******************************************************************************/
 INT8U active_board_temp_index_convert(INT8U i)
 {
      INT8U  t_index = 0;
      switch(i)
      {
          case 1:
          t_index = 0;
          break;
          case 2:
          t_index = 1;
          break;
          case 3:
          t_index = 2;
          break;
          case 0:
          t_index = 3;
          break;
          case 4:
          t_index = 4;
          break;
          case 7:
          t_index = 5;
          break;
          case 5:
          t_index = 6;
          break;
          case 6:
          t_index = 7;
          break;
          default:
          break;
      }
      return  t_index;
 }
/*****************************************************************************
 *函数名称:bsu_active_board_temp_sample
 *函数功能:
 *参    数:BsuSampleContext *__far context    
 *返 回 值:无
 *修订信息:
 ******************************************************************************/
static void bsu_active_board_temp_sample(BsuSampleContext *__far context)
{
    INT8U bsu_index = 0,bsu_num = 0;
    INT8U i = 0,t_index = 0;
    INT16U tmp_16u = 0;
    LtcResult result;

    if(context == NULL)
        return;
    for (i = 0;i < 2;i++)
    {  //6804本身原因，前2次采集的数据抛弃
       start_ltc6804_aux_adc(MD_FILTERED, AUX_CH_ALL); 
       OSTimeDly(FILTERED_MODE_AD_TIME);
    }
    bsu_num = get_bsu_num();
    for(i = 0;i < LTC6804_MAX_TEMP_NUM;i++)
    {
        t_index = active_board_temp_index_convert(i);
        result = pcf8574_write_channel(bsu_num, t_index);
        if (result == LTC_OK)  
		{
			OSTimeDly(2);
			start_ltc6804_aux_adc(MD_FILTERED, AUX_CH_GPIO3);
			OSTimeDly(FILTERED_MODE_AD_TIME);//等待采集完成
			if (read_ltc6804_aux_a_volt(bsu_num, AuxVoltBuffer, context->error_byte) == LTC_OK)
			{
				for (bsu_index = 0; bsu_index < bsu_num; bsu_index++)
				{
    				if (context->error_byte[bsu_index].Bits.read_gpio3 == 0)  //无异常
                    {
                        tmp_16u = get_ntc_resistance_vaule(AuxVoltBuffer[2][bsu_index], 3000,2);
                        context->temp_item.temp_buffer[bsu_index][i] = ltc6804_temp_query(tmp_16u); //查表得出温度
                    }
                    else
                    {
                        context->temp_item.temp_buffer[bsu_index][i] = TEMP_CABLE_OPEN_VAULE;
                    }
                }
			}
		}
		else
		{
			for (bsu_index = 0; bsu_index < bsu_num; bsu_index++)
			{
				context->temp_item.temp_buffer[bsu_index][i] = TEMP_CABLE_OPEN_VAULE;
			}
		}
    }
}

/*****************************************************************************
 *函数名称:bsu_passive_board_temp_sample
 *函数功能:温度采集每次采集3路
 *参    数:BsuSampleContext *__far context    
 *返 回 值:无
 *修订信息:
 ******************************************************************************/
static void bsu_passive_board_temp_sample(BsuSampleContext *__far context)
{
    INT8U bsu_num = 0;
    INT8U i = 0;

    if(context == NULL)
        return;
    for (i = 0;i < 2;i++)
    {  //6804本身原因，前2次采集的数据抛弃
       start_ltc6804_aux_adc(MD_FILTERED, AUX_CH_ALL); 
       OSTimeDly(FILTERED_MODE_AD_TIME);
    }
    bsu_num = get_bsu_num();
    start_ltc6804_aux_adc(MD_FILTERED, AUX_CH_ALL);
    OSTimeDly(FILTERED_MODE_AD_TIME);//时间不能少
    if (read_ltc6804_aux_a_volt(bsu_num, AuxVoltBuffer, context->error_byte) == LTC_OK)
    {
        convert_volt_to_temp(context,0);
        convert_volt_to_temp(context,1);
        convert_volt_to_temp(context,2);
    }
    if (read_ltc6804_aux_b_volt(bsu_num, AuxVoltBuffer, context->error_byte) == LTC_OK)
    {
        convert_volt_to_temp(context,3);
        convert_volt_to_temp(context,4);
    }
}
/*****************************************************************************
 *函数名称:bsu_passive_board_temp_sample
 *函数功能:温度采集每次采集3路
 *参    数:BsuSampleContext *__far context    
 *返 回 值:无
 *修订信息:
 ******************************************************************************/
static void bsu_temp_sample(BsuSampleContext *__far context)
{
     if(config_get(kBYUBalTypeIndex) == BSU_PASSIVE_BALANCE_TYPE)
     {
        bsu_passive_board_temp_sample(context);
     }
     else
     {
        bsu_active_board_temp_sample(context);
     
     }
}
/*****************************************************************************
 *函数名称:ReadBsuBoardType
 *函数功能:读取所有BSU板类型
 *参    数:BsuBoardType *__far board_type
 *返 回 值:无
 *修订信息:
 ******************************************************************************/
void read_bsu_board_type(BsuSampleContext *__far context)
{
    INT8U bsu_index = 0;
    INT8U bsu_num = 0;

    bsu_num = get_bsu_num();

    start_ltc6804_aux_adc(MD_FILTERED, AUX_CH_GPIO5);
    OSTimeDly(FILTERED_MODE_AD_TIME);
    if (read_ltc6804_aux_b_volt(bsu_num, AuxVoltBuffer, context->error_byte) == LTC_OK)
    {
        for (bsu_index = 0; bsu_index < bsu_num; bsu_index++)
        {
        #if  BMS_SUPPORT_BY5248D == 1 
            if(bsu_index < BMS_SUPPORT_LTC6804_ON_BOARD_CHIP_NUM) 
            {
                if(bsu_index == 0)
                {
                    context->board_type[bsu_index] = kBoardMain;
                }
                else
                {
                    context->board_type[bsu_index] = kBoardSlave;  
                }
                continue;
            }
        #endif
            if(context->board_type[bsu_index] != kBoardUnknow)
            {
                continue;
            }
            if (context->error_byte[bsu_index].Bits.read_gpio5 == 0)
            {
            	if (AuxVoltBuffer[1][bsu_index] > 3500)//大于3.5V为主采集板，小于3.5V为从采集板
            	{
            		context->board_type[bsu_index] = kBoardMain;
            	}
            	else 
            	{
            		context->board_type[bsu_index] = kBoardSlave;
            	}
            }
            else //通信错误  未知
            {
            	context->board_type[bsu_index] = kBoardUnknow;
            }
        }
    }
    else
    {
        for (bsu_index = 0; bsu_index < bsu_num; bsu_index++)
        {
            context->board_type[bsu_index] = kBoardUnknow;
        }
    }
}

/*****************************************************************************
 *函数名称:is_all_bsu_board_type_read
 *函数功能:所有BSU类型是否读取OK
 *参    数: 无
 *返 回 值: FALSE--否; TRUE--是
 *修订信息:
 ******************************************************************************/
BOOLEAN is_all_bsu_board_type_read(void)
{
    INT8U bsu_index = 0;
    INT8U bsu_num = 0;
    BsuBoardType *__far board_type = bsu_sample_context.board_type;
    bsu_num = get_bsu_num();
    if(config_get(kBYUBalTypeIndex) == BSU_ACTIVE_BALANCE_TYPE)
    {
        for (bsu_index = 0; bsu_index < bsu_num; bsu_index++)
        {
           board_type[bsu_index] = kBoardMain;
        }
        return TRUE;//主动均衡每个采集板都是主采集板
    }
    
    for (bsu_index = 0; bsu_index < bsu_num; bsu_index++)
    {
        if (board_type[bsu_index] == kBoardUnknow)
        {
            return FALSE;
        }
    }
    return TRUE;
}
/*****************************************************************************
 *函数名称:bsu_board_type_io_set
 *函数功能:根据BSU类型设置IO引脚
 *参    数:无
 *返 回 值:
 *修订信息:
 ******************************************************************************/
void bsu_board_type_io_set(void)
{
    INT8U bsu_num = 0;
    INT8U bsu_index = 0;
    BsuSampleContext* __far context = &bsu_sample_context;
    //if(config_get(kBYUBalTypeIndex) == BSU_ACTIVE_BALANCE_TYPE)
    //{
    //    return;//主动均衡采集板在初始化时已经设置过
    //}
    bsu_num = get_bsu_num();
    for(bsu_index = 0; bsu_index < bsu_num; bsu_index++)
    {
       #if  BMS_SUPPORT_BY5248D == 1   
        if(bsu_index < BMS_SUPPORT_LTC6804_ON_BOARD_CHIP_NUM) 
        {
            continue;
        }
    #endif
       
       if(get_bsu_n_board_type(bsu_index) == kBoardUnknow)
       {
           bsu_board_gpio_init(bsu_index);//未知类型初始化主板类型IO引脚
           CLR(context->board_gpio_init_flag,bsu_index); 
       }
       
       if((get_bsu_n_board_type(bsu_index) == kBoardSlave)&&(!GET(context->board_gpio_init_flag,bsu_index)))
       {
            bsu_board_gpio_init(bsu_index);//读取板子类型后，第一次需要初始化IO引脚
            SET(context->board_gpio_init_flag,bsu_index); 
       }
       if((get_bsu_n_board_type(bsu_index) == kBoardMain)&&(!GET(context->board_gpio_init_flag,bsu_index)))
       {
            bsu_board_gpio_init(bsu_index);//读取板子类型后，第一次需要初始化IO引脚
            SET(context->board_gpio_init_flag,bsu_index); 
       }
    }
}
/*****************************************************************************
 *函数名称:set_ltc6804_on_board_bsu_type
 *函数功能:板载6804的BSU类型设置
 *参    数:无
 *返 回 值:
 *修订信息:
 ******************************************************************************/
void set_ltc6804_on_board_bsu_type(BsuSampleContext *__far context)
{
    INT8U bsu_index = 0;
    INT8U bsu_num = 0;
    if(context == NULL)
        return;
    bsu_num = get_bsu_num();

    for (bsu_index = 0; bsu_index < bsu_num; bsu_index++)
    {
        if((bsu_index%BMU_SUPPORT_BSU_MAX_NUM)==0)
        {
            context->board_type[bsu_index] = kBoardMain;
        }
    	else 
    	{
    		context->board_type[bsu_index] = kBoardSlave;
    	}
    }
}
/*****************************************************************************
 *函数名称:bsu_board_diagnose
 *函数功能:根据BSU类型诊断
 *参    数:无
 *返 回 值:
 *修订信息:
 ******************************************************************************/
void bsu_board_diagnose(BsuSampleContext *__far context)
{
//#if  BMS_SUPPORT_BY5248D == 0    
    if(is_all_bsu_board_type_read()==FALSE) 
    {
        read_bsu_board_type(context);
    }
    bsu_board_type_io_set();
//#else
//    set_ltc6804_on_board_bsu_type(context);
//#endif
    main_bsu_pos_set();//主采集板位置设置
}
/*****************************************************************************
 *函数名称:BalanceControl
 *函数功能:均衡控制,根据板子类型启动不同的均衡控制
 *参    数:
 *返 回 值:
 *修订信息:
 ******************************************************************************/
static void bsu_balance_control(void)
{
    static INT8U flag = 0;
    if(config_get(kBYUBalTypeIndex) == BSU_PASSIVE_BALANCE_TYPE)
    {
        bsu_passive_balance_ctrl();
        (void)send_balance_cmd_to_ltc6804(get_bsu_num()); //控制6804均衡
    }
    else
    {
        if(flag == 0)
        {
            flag = 1;
            bsu_active_balance_ctrl();
            send_active_balance_cmd_to_ltc6804(get_bsu_num()); 
        }
        else
        {
            flag = 0;
            read_active_balance_current(get_bsu_num());
        }
    }
    
}
/*****************************************************************************
 *函数名称:task_sleep
 *函数功能:任务休眠，用于任务周期控制，放在任务末端
 *参    数:INT32U start_tick任务起始时刻,INT16U task_prodic任务周期
 *返 回 值:无
 *修订信息:
 ******************************************************************************/
void task_sleep(INT32U start_tick, INT16U task_prodic)
{
	INT32U tick_diff = 0;
	tick_diff = get_elapsed_tick_count(start_tick);
	if (tick_diff < task_prodic)
	{
		sleep(task_prodic - tick_diff);
	}
	else
	{
		sleep(5);
	}
}
/*****************************************************************************
 *函数名称:VoltTempSampleTask
 *函数功能:电压温度采集任务
 *参    数:void *pdata
 *返 回 值:无
 *修订信息:
 ******************************************************************************/
void BsuVoltTempSampleTask(void *pdata)
{
    void* p_arg = pdata;

    INT32U start_tick = 0;	    //任务起始时刻
    INT8U bsu_num = 0;
    pdata = pdata;
    //LTC6804初始化
    (void) ltc6804_init(get_bsu_num());

    for (;;)
    {
        bsu_num = get_bsu_num();
        start_tick = OSTimeGet();
        watch_dog_feed(WDT_AD_ID);        
        //清寄LTC6804存器数据
        clr_ltc6804_reg();
        //唤醒6804并读取板子类型
        if(runmode_get() == 2)wakeup_ltc6804_with_write_cfg_reg(bsu_num,TRUE);
        else wakeup_ltc6804_with_write_cfg_reg(bsu_num,FALSE);//关闭被动均衡
        bsu_board_diagnose(&bsu_sample_context);
        /*************************************电压采集************************************************************/
        //启动电池采集,  模式、所有电池
        start_ltc6804_cells_volt_adc(MD_FILTERED, DCP_DISABLED, CELL_CH_ALL);
        //等待采集完成
        OSTimeDly(FILTERED_MODE_AD_TIME);//时间不能少
        //清除临时电压数组数据
        ltc6804_buffer_set(&bsu_sample_context.volt_item.volt_buffer[0][0], 0, PARAM_BSU_NUM_MAX * LTC6804_MAX_CELL_NUM * sizeof(INT16U));
        //读取电池电压
        (void) read_ltc6804_cells_volt(bsu_num, bsu_sample_context.volt_item.volt_buffer, bsu_sample_context.error_byte, kReadCV);
        //从机电压通信中断心跳
        bsu_comm_heart_beat_run(&bsu_sample_context);

        if (is_cells_volt_abnormal(&bsu_sample_context) == TRUE)	    //电压异常
        {
            INT8U cnt = 0;
            //上拉开路AD采集,  模式、所有电池 连续发送10次
            for (cnt = 0; cnt < 10; cnt++)
            {
                start_ltc6804_volt_cable_open_adc(MD_FILTERED, DCP_DISABLED, PUP_UP, CELL_CH_ALL);
                OSTimeDly(FILTERED_MODE_AD_TIME);
            }
            ltc6804_buffer_set(&PUVoltTmpBuffer[0][0], 0, MAX_LTC6804_IC_NUM * LTC6804_MAX_CELL_NUM * sizeof(INT16U));
            (void) read_ltc6804_cells_volt(bsu_num, PUVoltTmpBuffer, bsu_sample_context.error_byte, kReadPUCV);

            //下拉开路AD采集,标准模式、所有电池  连续发送10次
            for (cnt = 0; cnt < 10; cnt++)
            {
                start_ltc6804_volt_cable_open_adc(MD_FILTERED, DCP_DISABLED, PUP_DOWN, CELL_CH_ALL);
                OSTimeDly(FILTERED_MODE_AD_TIME);
            }
            ltc6804_buffer_set(&PDVoltTmpBuffer[0][0], 0, MAX_LTC6804_IC_NUM * LTC6804_MAX_CELL_NUM * sizeof(INT16U));
            (void) read_ltc6804_cells_volt(bsu_num, PDVoltTmpBuffer, bsu_sample_context.error_byte, kReadPUDV);
            //电压检测排线开路诊断
            cells_volt_sample_cable_open_diagnose(&bsu_sample_context, PUVoltTmpBuffer, PDVoltTmpBuffer);
            start_ltc6804_cells_volt_adc(MD_FILTERED, DCP_DISABLED, CELL_CH_ALL);//增加此操作的目的是防止开路检测采集对正常采集照成-40mv抖动
        }
        //拷贝电压到最终缓存
        copy_cells_volt_with_filter(&bsu_sample_context);
        bsu_update_bcu_volt_info();
        task_sleep(start_tick, (BSU_VOLT_TEMP_SAMPLE_TASK_PRODIC/2));
        start_tick = OSTimeGet();
        /*************************************温度采集************************************************************/
        wakeup_ltc6804_with_write_cfg_reg(bsu_num,TRUE); //重新打开被动均衡 
        bsu_board_diagnose(&bsu_sample_context);
        bsu_temp_sample(&bsu_sample_context); 
        copy_cells_temp_with_filter(&bsu_sample_context);
        bsu_update_bcu_temp_info();
        set_sample_once_finish_flag(&bsu_sample_context);//第一采集完成
        /*************************************均衡控制************************************************************/
        if(runmode_get() != 2) bsu_balance_control();
        task_sleep(start_tick, (BSU_VOLT_TEMP_SAMPLE_TASK_PRODIC/2));
    }
}
#endif