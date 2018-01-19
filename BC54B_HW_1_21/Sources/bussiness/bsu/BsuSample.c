/*******************************************************************************
 **                       安徽力高新能源技术有限公司 Copyright (c)
 **                            http://www...
 **文 件 名:BsuSample.c
 **创建日期:2015.10.15
 **文件说明:电压温度采集、均衡控制
 **修改记录:
 **版    本:V1.0
 **备    注:
 *******************************************************************************/
#include "bms_bmu.h"
#include "BsuSample.h"
#include "ltc_util.h"
#include "bms_bsu.h"
#include "bsu_passive_balance.h"
#include "ltc_util.h"
#if BMS_SUPPORT_HARDWARE_LTC6804 == 1
//电池对应的跨箱位置定义
const INT8U ltc6804_cross_pack_bits[PARAM_BSU_NUM_MAX] = {0x00,0x00,0x00,0x00,0x00,0x00,
                                                          0x00,0x00,0x00,0x00,0x00,0x00,
                                                          0x00,0x00,0x00,0x00,0x00,0x00,
                                                          0x00,0x00,0x00,0x00,0x00,0x00,
                                                          0x00,0x00,0x00,0x00,0x00,0x00
                                                         };

static OS_STK BsuSampleTaskStack[AD_SAMPLE_TASK_STK_SIZE] = { 0 }; //电压温度采集任务堆栈

#pragma DATA_SEG __RPAGE_SEG PAGED_RAM_BYU_SAMPLE
//采集信息
static BsuSampleContext bsu_sample_context;
//上拉采集的电池电压
static INT16U PUVoltTmpBuffer[PARAM_BSU_NUM_MAX][LTC6804_MAX_CELL_NUM];
//下拉采集的电池电压
static INT16U PDVoltTmpBuffer[PARAM_BSU_NUM_MAX][LTC6804_MAX_CELL_NUM];
#pragma DATA_SEG DEFAULT 

#pragma DATA_SEG __RPAGE_SEG PAGED_RAM_BSU 
static INT16U VoltBuffer[PARAM_BSU_NUM_MAX]; //温度电压临时缓存
#pragma DATA_SEG DEFAULT
//函数申明
static void bsu_data_init(BsuSampleContext* __far context);
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
	bsu_data_init(&bsu_sample_context); //数据初始化
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
 *函数功能:获取参数,第N个从机的电池数
 *参    数:bsu_index  从机号(从0开始)
 *返 回 值:对应从机的电池数
 *修订信息:
 ******************************************************************************/
INT8U get_bsu_n_cells_num(INT8U bsu_index)
{
	INT8U num = 0;
	//参数检查
	if (bsu_index >= PARAM_BSU_NUM_MAX)
		return 0;
    if(bsu_index < 10)
	    num = (INT8U) config_get(bsu_index + kBYUVoltCnt1Index);
    else
        num = 0;
	return num;
}
/*****************************************************************************
 *函数名称:is_bsu_n_cross_pack_en
 *函数功能:bsu是否跨箱
 *参    数:bsu_index  从机号(从0开始)
 *返 回 值:TRUE是  FALSE否
 *修订信息:
 ******************************************************************************/
BOOLEAN is_bsu_n_cross_pack_en(INT8U bsu_index)
{
	//参数检查
	if (bsu_index >= PARAM_BSU_NUM_MAX)
		return FALSE;
    if(ltc6804_cross_pack_bits[bsu_index]==0)
        return FALSE;
	return TRUE;
}
/*****************************************************************************
 *函数名称:get_bsu_num
 *函数功能:获取bsu数目
 *参    数:无
 *返 回 值:无
 *修订信息:
 ******************************************************************************/
INT8U get_bsu_num(void)
{
    INT8U i = 0;
    
    for(i = 0; i < PARAM_BSU_NUM_MAX; i++)
    {
        if((get_bsu_n_cells_num(i)==0)||(get_bsu_n_cells_num(i)>BYU_VOLT_CNT_MAX_DEF))
            return i;
    }
    return i;
}
/*****************************************************************************
 *函数名称:get_bsu_n_top_cells_num
 *函数功能:获取第N个从机芯片顶端电芯数 ,当电池数为奇数时底端比顶端多1个,位偶数时一致
 *参    数:bsu_index  从机号(从0开始)
 *返 回 值:对应从机芯片顶端电芯数
 *修订信息:
 ******************************************************************************/
INT8U get_bsu_n_top_cells_num(INT8U bsu_index)
{
	INT8U num = 0;
	INT8U bsu_n_cells_num = 0;
	//参数检查
	if (bsu_index >= PARAM_BSU_NUM_MAX)
		return 0;

	bsu_n_cells_num = get_bsu_n_cells_num(bsu_index);
	num = (bsu_n_cells_num >> 1);//顶端除2即可

	return num;
}
/*****************************************************************************
 *函数名称:GetBsuNBottomCellsNum
 *函数功能:获取第N个从机芯片底端电芯数 ,当电池数为奇数时底端比顶端多1个,位偶数时一致
 *参    数:INT8U bsu_index  从机号:从0开始
 *返 回 值:对应从机芯片底端电芯数
 *修订信息:
 ******************************************************************************/
INT8U get_bsu_n_bottom_cells_num(INT8U bsu_index)
{
	INT8U num = 0;
	INT8U bsu_n_cells_num = 0;
	//参数检查
	if (bsu_index >= PARAM_BSU_NUM_MAX)
		return 0;

	bsu_n_cells_num = get_bsu_n_cells_num(bsu_index);
	num = (bsu_n_cells_num >> 1) + (bsu_n_cells_num & 0x01);  //底端除2后，如果是偶数加0,如果是奇数加1

	return num;
}
/*****************************************************************************
 *函数名称:bsu_data_init
 *函数功能:从机BSU数据初始化
 *参    数:context从机采集数据内容
 *返 回 值:无
 *修订信息:
 ******************************************************************************/
static void bsu_data_init(BsuSampleContext* __far context)
{
	INT8U index = 0;
	INT8U bsu_index = 0; //从机号索引
	//参数检查
	if(context == NULL)
	    return;
	//电压清零
	ltc6804_buffer_set(&context->volt_item,0,sizeof(BsuVoltSampleItem));
	//心跳置最大值
	ltc6804_buffer_set(context->heart_beat,BSU_MAX_COMM_ERROR_CNT,PARAM_BSU_NUM_MAX);
	//板子类型
	ltc6804_buffer_set(&context->board_type,kBoardUnknow,PARAM_BSU_NUM_MAX);

	for (bsu_index = 0; bsu_index < PARAM_BSU_NUM_MAX; bsu_index++)
	{
		context->error_byte[bsu_index].value = 0;
		//温度数据
		for (index = 0; index < LTC6804_MAX_CELL_NUM; index++)
		{
			context->temp_item.temp[bsu_index][index] = TEMP_CABLE_OPEN_VAULE;//温度置0xFF
			context->temp_item.temp_buffer[bsu_index][index] = 0;
			context->temp_item.filter[bsu_index][index] = 0;
		}
	}
}
/*****************************************************************************
 *函数名称:get_bsu_cells_volt
 *函数功能:获取电池电压
 *参    数:bsu_index从机号,pos电池在从机中的位置
 *返 回 值:电池电压
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
 *函数功能:获取电池温度
 *参    数:INT8U bsu_index从机号,INT8U pos温感编号
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
 *函数名称:bsu_cells_volt_reorganize
 *函数功能:电压数据重组
 *         由于每个从机串数不同需要将电压重组使之连续
 *参    数:INT16U  (*__far volt)[LTC6804_MAX_CELL_NUM] 从LTC6804中读出来的电压临时数据数组
 *返 回 值:无
 *修订信息:
 ******************************************************************************/
static void bsu_cells_volt_reorganize(INT16U (*__far volt)[LTC6804_MAX_CELL_NUM])
{
	INT8U index = 0;
	INT8U bsu_index = 0; //从机号索引
	INT8U bottom_not_use_num = 0;//下半片未使用电池数
	INT8U bsu_num = 0;//从机数
	INT8U bsu_n_cells_num = 0;//从机电池数
	//参数检查
	if(volt == NULL)
	    return;

	bsu_num = get_bsu_num();
	for(bsu_index = 0; bsu_index < bsu_num; bsu_index++)
	{
		bsu_n_cells_num = get_bsu_n_cells_num(bsu_index);
		if(bsu_n_cells_num == LTC6804_MAX_CELL_NUM) //为最大值,不用重组
		{
			continue;
		}
		bottom_not_use_num = LTC6804_HALF_CELL_NUM - get_bsu_n_bottom_cells_num(bsu_index);

		if(bottom_not_use_num > 0)//底部未满6串,数据需要重组
		{
			for(index = get_bsu_n_bottom_cells_num(bsu_index);index < bsu_n_cells_num;index++)
			{
				volt[bsu_index][index] = volt[bsu_index][index+ bottom_not_use_num];
			}
		}

		//剩下的清零
		for(index = bsu_n_cells_num;index < LTC6804_MAX_CELL_NUM; index++)
		{
			volt[bsu_index][index] = 0;
		}
		if( is_bsu_n_cross_pack_en(bsu_index) == TRUE ) 
		{
		
		
		}
	}
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
 *函数功能:从机通信中断心跳
 *参    数:context从机电压温度信息
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

	bsu_num = get_bsu_num();//从机数
	for (bsu_index = 0; bsu_index < bsu_num; bsu_index++)
	{
		OS_ENTER_CRITICAL();
		if (context->error_byte[bsu_index].Bits.read_cv)//读电压通信异常
		{
			context->heart_beat[bsu_index]++;  //计数增加
			if (context->heart_beat[bsu_index] >= BSU_MAX_COMM_ERROR_CNT)//达到最大值
			{
				context->heart_beat[bsu_index] = BSU_MAX_COMM_ERROR_CNT; //从机通信中断
				context->volt_item.cable_open_flag[bsu_index] = 0;
			}
		}
		else
		{
			context->heart_beat[bsu_index] = 0;
		}
		OS_EXIT_CRITICAL();
	}
	//未使用的从机全部标记为通信中断
	for (; bsu_index < PARAM_BSU_NUM_MAX; bsu_index++)
	{
		OS_ENTER_CRITICAL();
		context->heart_beat[bsu_index] = BSU_MAX_COMM_ERROR_CNT;
		OS_EXIT_CRITICAL();
	}
	clr_all_bsu_comm_error_byte();
}
/*****************************************************************************
 *函数名称:is_bsu_n_comm_error
 *函数功能:判断第N个从机是否通信失败
 *参    数:bsu_index从机编号
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
 *函数功能:判断所有的从机是否通信失败
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
 *函数功能:判断是否有从机通信失败
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
 *参    数:INT8U bsu_index从机编号, INT8U cell_index电池位置                                 
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
 *函数功能:获取从机排线脱落标志,bit0代表1号电池的排线有脱落情况
 *参    数:INT8U bsu_index从机编号
 *返 回 值:无
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
 *函数功能:从机排线脱落
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
 *函数名称:get_temp_filter_cnt
 *函数功能:获取温度滤波计数
 *参    数:bsu_index从机编号, index温度号                                     
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
 *函数名称:is_cells_volt_abnormal
 *函数功能:电压是否有异常
 *参    数:item
 *返 回 值:FALSE无异常 TRUE有异常
 *修订信息:
 ******************************************************************************/
static INT8U is_cells_volt_abnormal(BsuSampleContext *__far context)
{
	INT8U bsu_index = 0; //当前从机号
	INT8U pos = 0;
	INT8U bsu_num = 0;  //从机数
	INT8U  bsu_n_cells_num = 0; //第N个从机电池数
	static INT8U first_flag = 0;
	
	if(context == NULL)
          return FALSE;
	
	if(first_flag == 0)
	{
	   first_flag = 1;
	   return FALSE;
	}
	bsu_num = get_bsu_num();
	for (bsu_index = 0; bsu_index < bsu_num; bsu_index++)
	{

		if (context->error_byte[bsu_index].Bits.read_cv)
		{
			continue;
		}
		bsu_n_cells_num = get_bsu_n_cells_num(bsu_index);
		for (pos = 0; pos < bsu_n_cells_num; pos++)
		{
			if (context->volt_item.volt_buffer[bsu_index][pos] > LTC6804_CELL_MAX_VOLT)//异常
			{
				return TRUE;
			}
			if (context->volt_item.cable_open_flag[bsu_index] != 0)//已经有排线脱落
			{
				return TRUE;
			}
			if (abs(context->volt_item.volt_buffer[bsu_index][pos] - context->volt_item.volt[bsu_index][pos]) >= 100)//电压突变100mv
			{
			    return TRUE;  
			}
		}
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
			SET(open_flag, 0);
		}
		if (PD_volt[bsu_index][bsu_n_cells_num - 1] == 0)
		{ //TODO  如果是外部供电则仅B12+脱落即可
			for (pos = 0; pos < bsu_n_cells_num; pos++)
			{
				SET(open_flag, pos);
                //供电线脱落，数据不可靠清零
                context->volt_item.volt_buffer[bsu_index][pos] = 0;
			}
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
                    context->volt_item.volt[bsu_index][pos] = context->volt_item.volt_buffer[bsu_index][pos];
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
 *函数名称:get_ntc_resistance_vaule
 *函数功能:获取NTC温感的当前阻值,根据采集的电压计算出当前NTC的阻值
 *参    数:INT16U r_volt采集到电阻上的电压,INT16U ref_volt参考电压
 *返 回 值:电阻值(10Ω/bit)
 *修订信息:
 ******************************************************************************/
static INT16U get_ntc_resistance_vaule(INT16U r_volt, INT16U ref_volt)
{
    INT16U R = 0xFFFF;
    INT32U tmp_32u = 0;
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
    if (temp_index >= LTC6804_MAX_TEMP_NUM)
        return;
    if(context == NULL)
        return;
    bsu_num = get_bsu_num();
    for (bsu_index = 0; bsu_index < bsu_num; bsu_index++)
    {
        if (context->error_byte[bsu_index].Bits.read_gpio1 == 0)//无异常
        {
            tmp_16u = get_ntc_resistance_vaule(VoltBuffer[bsu_index], 3000);
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
            temp_num = (INT8U)config_get(kBmuMaxTempNumIndex);
            if(temp_num > LTC6804_MAX_TEMP_NUM)
                temp_num = LTC6804_MAX_TEMP_NUM;
            for (pos = 0; pos < temp_num; pos++)
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
                	OS_ENTER_CRITICAL();
                	context->temp_item.temp[bsu_index][pos] = context->temp_item.temp_buffer[bsu_index][pos];
                	OS_EXIT_CRITICAL();
                	context->temp_item.filter[bsu_index][pos] = 0;
                }
            }
        }
    }
}
/*****************************************************************************
 *函数名称:bsu_temp_sample
 *函数功能:温度采集每次采集3路
 *参    数:BsuSampleContext *__far context    
 *返 回 值:无
 *修订信息:
 ******************************************************************************/
static void bsu_temp_sample(BsuSampleContext *__far context)
{
    INT8U index = 0;
    INT8U channel = 0; //通道
    INT8U bsu_index = 0;
    static INT8U flag = 0;//切换标志
    LtcResult result = LTC_ERROR;
    INT8U bsu_num = 0;
    INT8U temp_num = 0;

    if(context == NULL)
        return;

    bsu_num = get_bsu_num();
    temp_num = (INT8U)config_get(kBmuMaxTempNumIndex);
    if(temp_num > LTC6804_MAX_TEMP_NUM)
        temp_num = LTC6804_MAX_TEMP_NUM;
    start_ltc6804_aux_adc(MD_NORMAL, AUX_CH_GPIO1);
    OSTimeDly(2);

    for (index = 0; index < temp_num; index++)
    {
        if(flag == 0) //每次仅采集3路
        {
            if(index >= 3)
            {
                continue;
            }
        }
        else
        {
            if(index < 3)
            {
                continue;
            }
        }

        channel = index + T1_CHANNEL;

        result = pcf8574_write_channel(bsu_num, channel); //选择温度

        if (result == LTC_OK)  //40ms
        {
            OSTimeDly(2);
            start_ltc6804_aux_adc(MD_NORMAL, AUX_CH_GPIO1);
            OSTimeDly(4);  //等待采集完成
            if (read_ltc6804_aux_volt(bsu_num, AUX_CH_GPIO1, VoltBuffer, context->error_byte) == LTC_OK)
            {
                convert_volt_to_temp(context,index);
            }
        }
        else
        {
            for (bsu_index = 0; bsu_index < bsu_num; bsu_index++)
            {
                context->temp_item.temp_buffer[bsu_index][index] = TEMP_CABLE_OPEN_VAULE;
            }
        }
    }
    if(flag == 0)
    {
        flag = 1;
    }
    else
    {
        flag = 0;
    }
}
/*****************************************************************************
 *函数名称:ReadBsuBoardType
 *函数功能:读取所有BSU板类型
 *参    数:BsuBoardType *__far board_type
 *返 回 值:无
 *修订信息:
 ******************************************************************************/
static void read_bsu_board_type(BsuSampleContext *__far context)
{
    INT8U index = 0;
    INT8U channel = 0; //通道
    INT8U bsu_index = 0;
    INT8U bsu_num = 0;
    LtcResult result = LTC_ERROR;

    bsu_num = get_bsu_num();

    channel = 6;
    result = pcf8574_write_channel(bsu_num, channel);
    if (result == LTC_OK)
    {
        OSTimeDly(2);
        start_ltc6804_aux_adc(MD_NORMAL, AUX_CH_GPIO1);
        OSTimeDly(4);
        if (read_ltc6804_aux_volt(bsu_num, AUX_CH_GPIO1, VoltBuffer, context->error_byte) == LTC_OK)
        {
            for (bsu_index = 0; bsu_index < bsu_num; bsu_index++)
            {
                if (context->error_byte[bsu_index].Bits.read_gpio1 == 0)
                {
                	if (VoltBuffer[bsu_index] > 2000)
                	{
                		context->board_type[bsu_index] = kBoardActive;
                	}
                	else if (VoltBuffer[bsu_index] < 1000)
                	{
                		context->board_type[bsu_index] = kBoardPassive;
                	}
                }
                else
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
    else
    {
        for (bsu_index = 0; bsu_index < bsu_num; bsu_index++)
        {
            context->board_type[bsu_index] = kBoardUnknow;
        }
    }

}
/*****************************************************************************
 *函数名称:get_bsu_n_board_type
 *函数功能:获取板子均衡类型,用于发给上位机或HMI
 *参    数:INT8U bsu_index 从机编号
 *返 回 值:均衡类型
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
 *函数名称:is_all_bsu_passive_board
 *函数功能:所有从机是否为被动均衡板子
 *参    数: 无
 *返 回 值: FALSE--否; TRUE--是
 *修订信息:
 ******************************************************************************/
static BOOLEAN is_all_bsu_passive_board(void)
{
    INT8U bsu_index = 0;
    INT8U bsu_num = 0;
    const BsuBoardType *__far board_type = bsu_sample_context.board_type;

    bsu_num = get_bsu_num();
    for (bsu_index = 0; bsu_index < bsu_num; bsu_index++)
    {
        if (board_type[bsu_index] != kBoardPassive)
        {
            return FALSE;
        }
    }
    return TRUE;
}

/*****************************************************************************
 *函数名称:IsAllBsuActiveBoard
 *函数功能:所有从机是否为主动均衡板子
 *参    数:无
 *返 回 值:FALSE--否; TRUE--是
 *修订信息:
 ******************************************************************************/
static BOOLEAN is_all_bsu_active_board(void)
{
    INT8U bsu_index = 0;
    INT8U bsu_num = 0;
    const BsuBoardType *__far board_type = bsu_sample_context.board_type;

    bsu_num = get_bsu_num();
    for (bsu_index = 0; bsu_index < bsu_num; bsu_index++)
    {
        if (board_type[bsu_index] != kBoardActive)
        {
            return FALSE;
        }
    }
    return TRUE;
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
    if (is_all_bsu_passive_board() == TRUE)
    {
        bsu_passive_balance_ctrl(); //被动均衡
    }
    else if (is_all_bsu_active_board() == TRUE)
    {
     //ActiveBalanceControl();//主动均衡  TODO
    }
    else
    {
        clr_all_ltc6804_balance_ctrl_word(); //清除均衡通道所有的数据
        //ClrAllBsuActiveBalanceWord();    TODO
        clr_all_bsu_passive_balance_word();
        read_bsu_board_type(&bsu_sample_context);	    //读取板子型号
    }
}

/*****************************************************************************
 *函数名称:set_bsu_relay1_on
 *函数功能:bsu从机继电器控制
 *参    数:
 *返 回 值:
 *修订信息:
 ******************************************************************************/
void set_bsu_relay1_on(INT8U bsu_index)
{
    if (bsu_index >= get_bsu_num())
    {
        return;
    }
    set_ltc6804_gpio2_pin_high(bsu_index);
}
/*****************************************************************************
 *函数名称:set_bsu_relay1_off
 *函数功能:bsu从机继电器控制
 *参    数:
 *返 回 值:
 *修订信息:
 ******************************************************************************/
void set_bsu_relay1_off(INT8U bsu_index)
{
    if (bsu_index >= get_bsu_num())
    {
        return;
    }
    set_ltc6804_gpio2_pin_low(bsu_index);
}
/*****************************************************************************
 *函数名称:set_bsu_relay2_on
 *函数功能:bsu从机继电器控制
 *参    数:
 *返 回 值:
 *修订信息:
 ******************************************************************************/
void set_bsu_relay2_on(INT8U bsu_index)
{
    if (bsu_index >= get_bsu_num())
    {
        return;
    }
    set_ltc6804_gpio2_pin_high(bsu_index);
}
/*****************************************************************************
 *函数名称:set_bsu_relay2_off
 *函数功能:bsu从机继电器控制
 *参    数:
 *返 回 值:
 *修订信息:
 ******************************************************************************/
void set_bsu_relay2_off(INT8U bsu_index)
{
    if (bsu_index >= get_bsu_num())
    {
        return;
    }
    set_ltc6804_gpio3_pin_low(bsu_index);
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
    INT32U tick_diff = 0;	    //时间差值
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

        /*************************************电压采集************************************************************/
        wakeup_ltc6804_with_write_cfg_reg(bsu_num);
        //启动电池采集,  模式、所有电池
        start_ltc6804_cells_volt_adc(MD_FILTERED, DCP_DISABLED, CELL_CH_ALL);
        //等待采集完成
        OSTimeDly(220);
        //清除临时电压数组数据
        ltc6804_buffer_set(&bsu_sample_context.volt_item.volt_buffer[0][0], 0, PARAM_BSU_NUM_MAX * LTC6804_MAX_CELL_NUM * sizeof(INT16U));
        //读取电池电压
        (void) read_ltc6804_cells_volt(bsu_num, bsu_sample_context.volt_item.volt_buffer, bsu_sample_context.error_byte, kReadCV);
        //根据配置的串数不同,将电压数据重组
        bsu_cells_volt_reorganize(bsu_sample_context.volt_item.volt_buffer);
        //从机电压通信中断心跳
        bsu_comm_heart_beat_run(&bsu_sample_context);

        if (is_cells_volt_abnormal(&bsu_sample_context) == TRUE)	    //电压异常
        {
            INT8U cnt = 0;
            //上拉开路AD采集,  模式、所有电池 连续发送10次
            for (cnt = 0; cnt < 10; cnt++)
            {
                start_ltc6804_volt_cable_open_adc(MD_NORMAL, DCP_DISABLED, PUP_UP, CELL_CH_ALL);
                OSTimeDly(3);
            }
            ltc6804_buffer_set(&PUVoltTmpBuffer[0][0], 0, MAX_LTC6804_IC_NUM * LTC6804_MAX_CELL_NUM * sizeof(INT16U));
            (void) read_ltc6804_cells_volt(bsu_num, PUVoltTmpBuffer, bsu_sample_context.error_byte, kReadPUCV);
            bsu_cells_volt_reorganize(PUVoltTmpBuffer);

            //下拉开路AD采集,标准模式、所有电池  连续发送10次
            for (cnt = 0; cnt < 10; cnt++)
            {
                start_ltc6804_volt_cable_open_adc(MD_NORMAL, DCP_DISABLED, PUP_DOWN, CELL_CH_ALL);
                OSTimeDly(3);
            }
            ltc6804_buffer_set(&PDVoltTmpBuffer[0][0], 0, MAX_LTC6804_IC_NUM * LTC6804_MAX_CELL_NUM * sizeof(INT16U));
            (void) read_ltc6804_cells_volt(bsu_num, PDVoltTmpBuffer, bsu_sample_context.error_byte, kReadPUDV);
            bsu_cells_volt_reorganize(PDVoltTmpBuffer);
            //电压检测排线开路诊断
            cells_volt_sample_cable_open_diagnose(&bsu_sample_context, PUVoltTmpBuffer, PDVoltTmpBuffer);
        }
        	    //拷贝电压到最终缓存
        copy_cells_volt_with_filter(&bsu_sample_context);
        bsu_update_bcu_volt_info();

        /*************************************温度采集************************************************************/
        wakeup_ltc6804_with_write_cfg_reg(bsu_num);  

        bsu_temp_sample(&bsu_sample_context); 
        copy_cells_temp_with_filter(&bsu_sample_context);
        bsu_update_bcu_temp_info();
        /*************************************均衡控制************************************************************/
        bsu_balance_control();

        task_sleep(start_tick, BSU_VOLT_TEMP_SAMPLE_TASK_PRODIC);
    }
}
#endif