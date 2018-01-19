/*******************************************************************************
 **                       安徽力高新能源技术有限公司 Copyright (c)
 **                            http://www...
 **文 件 名:bsu_passive_balance.c
 **创建日期:2015.12.24
 **文件说明:电池被动均衡控制
 **修改记录:
 **版    本:V1.0
 **备    注:
 *******************************************************************************/
#include "bsu_passive_balance.h"
#include "ltc_util.h"
#include "ltc68041_impl.h" 
#include "bsu_sample.h"
#include "bms_bsu.h"
#if BMS_SUPPORT_HARDWARE_LTC6804 == 1

#pragma DATA_SEG __RPAGE_SEG PAGED_RAM_BSU_BALANCE 
static BsuPassiveBalanceContext bsu_passive_balance_context;
#pragma DATA_SEG DEFAULT
/*****************************************************************************
 *函数名称:clr_passive_balance_multi_volt_item
 *函数功能:将被动均衡临时缓存数据清除
 *参    数:无
 *返 回 值:无
 *修订信息:
 ******************************************************************************/
void clr_bsu_passive_balance_multi_volt_item(void)
{
	ltc6804_buffer_set(&bsu_passive_balance_context.volt_item[0][0], 0, PARAM_BSU_NUM_MAX * MULTI_MAX_VOLT_NUM * sizeof(BalanceItem));
}
/*****************************************************************************
 *函数名称:bsu_statistic_multi_max_volt
 *函数功能:获取每个从机的前n个最大电池电压
 *参    数:bsu_index 从机号  pos电池位置 volt电池电压
 *返 回 值:无
 *修订信息:
 ******************************************************************************/
void bsu_statistic_multi_max_volt(INT8U bsu_index,INT8U pos, INT16U volt)
{
	INT8U i = 0, move_num = 0, move_i = 0, satrt_i = 0;
    const BsuPassiveBalanceContext *far context = &bsu_passive_balance_context;
    //参数判断
    if(bsu_index >= get_bsu_num())
        return;
	if (pos >= get_bsu_n_cells_num(bsu_index))
		return;

	for (i = 0; i < (INT8U) MULTI_MAX_VOLT_NUM; i++)
	{
		if (volt > context->volt_item[bsu_index][i].volt)
		{   //第[0]个为最高电压,如果加入的电压大于第[0]个电压,那么所有电压后移
			move_num = (INT8U) MULTI_MAX_VOLT_NUM - i - 1;
			satrt_i = (INT8U) MULTI_MAX_VOLT_NUM - 1;
			for (move_i = move_num; move_i > 0; move_i--)
			{
				context->volt_item[bsu_index][satrt_i].volt = context->volt_item[bsu_index][satrt_i - 1].volt;
				context->volt_item[bsu_index][satrt_i].pos = context->volt_item[bsu_index][satrt_i - 1].pos;
				satrt_i--;
			}
			context->volt_item[bsu_index][satrt_i].volt = volt;
			context->volt_item[bsu_index][satrt_i].pos = pos;
			break;
		}
	}
}
/*****************************************************************************
 *函数名称:set_bsu_passive_balance_word
 *函数功能:设置被动均衡位，用于均衡控制
 *参    数:context均衡内容  bsu_index 从机号  balance_word均衡字
 *返 回 值:无            
 *修订信息:
 ******************************************************************************/
static void set_bsu_passive_balance_word(BsuPassiveBalanceContext *far context,INT8U bsu_index, INT16U balance_word)
{
	OS_CPU_SR cpu_sr = 0;
	if (bsu_index >= get_bsu_num())
		return;

	OS_ENTER_CRITICAL();
	context->balance_control_word[bsu_index] = balance_word;
	OS_EXIT_CRITICAL();
    set_ltc6804_balance_ctrl_word(bsu_index,balance_word);
}
/*****************************************************************************
 *函数名称:get_passive_balance_word
 *函数功能:获取被动均衡字
 *参    数:bsu_index 从机号
 *返 回 值:INT16U 均衡位           
 *修订信息:
 ******************************************************************************/
INT16U get_bsu_passive_balance_word(INT8U bsu_index)
{
	OS_CPU_SR cpu_sr = 0;
	INT16U balance_word = 0;
	const BsuPassiveBalanceContext *far context = &bsu_passive_balance_context;
	if (bsu_index >= get_bsu_num())
		return 0;

	OS_ENTER_CRITICAL();
	balance_word = context->balance_control_word[bsu_index];
	OS_EXIT_CRITICAL();
	return balance_word;
}
/*****************************************************************************
 *函数名称:clr_all_bsu_passive_balance_word
 *函数功能:清除所有的均衡位
 *参    数:无
 *返 回 值:无    
 *修订信息:
 ******************************************************************************/
static void clr_all_bsu_passive_balance_word(void)
{
	INT8U bsu_index = 0;
	for (bsu_index = 0; bsu_index < PARAM_BSU_NUM_MAX; bsu_index++)
	{
		set_bsu_passive_balance_word(&bsu_passive_balance_context,bsu_index, 0);
	}
}
/*****************************************************************************
 *函数名称:bsu_passive_balance_max_volt_diff_analyze
 *函数功能:被动均衡最大电压压差分析 与所有电池平均电压比较
 *参    数:INT16U time时间间隔
 *返 回 值:无
 *修订信息:
 ******************************************************************************/
static void bsu_passive_balance_max_volt_diff_analyze(INT16U time)
{
    BsuPassiveBalanceContext *far context = &bsu_passive_balance_context;
    
	switch (context->max_volt_diff_status)
	{
		case kPassiveBalanceVoltDiffNormal:

			if (bcu_get_high_voltage() >= (bcu_get_average_voltage() + config_get(kBYUBalDiffVoltMaxIndex)))//大于所有电池平均电压+均衡开始压差
			{
				context->status_dly += time;
				if (context->status_dly >= BALANCE_DLAY_2S) //延时2S
				{
					context->status_dly = 0;
					context->max_volt_diff_status = kPassiveBalanceVoltDiffBig;
				}
			}
			else
			{
				context->status_dly = 0;
			}

		break;
		case kPassiveBalanceVoltDiffBig:
			if (bcu_get_high_voltage() < (bcu_get_average_voltage() + config_get(kBYUBalDiffVoltMinIndex))) //小于所有电池平均电压+均衡关闭压差
			{
				context->status_dly += time;
				if (context->status_dly >= BALANCE_DLAY_2S) //延时2S
				{
					context->status_dly = 0;
					context->max_volt_diff_status = kPassiveBalanceVoltDiffNormal;
				}
			}
			else
			{
				context->status_dly = 0;
			}
		break;
		default:
			context->status_dly = 0;
			context->max_volt_diff_status = kPassiveBalanceVoltDiffNormal;
		break;
	}
}

 /*****************************************************************************
 *函数名称:is_passive_balance_condition_ok
 *函数功能:被动均衡条件是否成立
 *参    数:无
 *返 回 值:TRUE条件成立 FALSE条件不成立
 *修订信息:
 ******************************************************************************/
static BOOLEAN is_passive_balance_condition_ok(void)
{
	const BsuPassiveBalanceContext *far context = &bsu_passive_balance_context;
	
	if ((bcu_get_dchg_ht_state() == kAlarmNone)  //充电没有过温报警和保护
		&&(bcu_get_chg_ht_state() == kAlarmNone)  //放电没有过温报警和保护
		&&(config_get(kBYUBalNumIndex) > 0)  //均衡路数大于0
	    &&(context->max_volt_diff_status == kPassiveBalanceVoltDiffBig)  //满足 最大电池电压与平均电压压差
	    &&(is_bsu_comm_error()==FALSE) //没有通信中断
	    &&(is_volt_sample_cable_open() == FALSE) //电压采集排线脱落
	    &&(bcu_get_temp_exception() == 0)
	    &&(config_get(kBmuBalanceEnableIndex)) //均衡使能
	    &&(bcu_get_high_voltage() >= config_get(kBYUBalStartVoltIndex)) //均衡启动电压满足
	    &&(bcu_get_low_voltage() >= config_get(kChgLVTrdAlarmIndex))
	    &&(bcu_is_charging() == TRUE)&&(charger_is_connected())//在充电过充中
	    ) 
	{
		return TRUE;
	}
	return FALSE;
}
/*****************************************************************************
 *函数名称:get_bsu_max_cell_volt
 *函数功能:
 *参    数:无
 *返 回 值:无
 *修订信息:
 ******************************************************************************/
INT16U get_bsu_max_cell_volt(INT8U bsu_index)
{
	INT8U bsu_num = 0;
    BsuPassiveBalanceContext *far context = &bsu_passive_balance_context;
			
	bsu_num = get_bsu_num(); //从机数目
	if(bsu_index >= bsu_num)
	    return 0;
	
	return context->volt_item[bsu_index][0].volt; 
}
/*****************************************************************************
 *函数名称:get_bsu_max_cell_volt_pos
 *函数功能:
 *参    数:无
 *返 回 值:无
 *修订信息:
 ******************************************************************************/
INT8U get_bsu_max_cell_volt_pos(INT8U bsu_index)
{
	INT8U bsu_num = 0;
    BsuPassiveBalanceContext *far context = &bsu_passive_balance_context;
			
	bsu_num = get_bsu_num(); //从机数目
	if(bsu_index >= bsu_num)
	    return 0;
	
	return context->volt_item[bsu_index][0].pos; 
}
/*****************************************************************************
 *函数名称:select_passive_balance_cell
 *函数功能:选择需要被动均衡的电池
 *参    数:无
 *返 回 值:无
 *修订信息:
 ******************************************************************************/
static void select_passive_balance_cell(void)
{
	INT16U cell_balance_word = 0;     //均衡位
	INT8U i = 0;
	INT8U bsu_index = 0;       //从机编号计数
	INT8U bsu_num = 0;
	INT8U max_balance_num = 0; //最大均衡路数
    BsuPassiveBalanceContext *far context = &bsu_passive_balance_context;
			
	bsu_num = get_bsu_num(); //从机数目
	max_balance_num = (INT8U) config_get(kBYUBalNumIndex);//均衡路数
	if (max_balance_num > MULTI_MAX_VOLT_NUM)
		max_balance_num = MULTI_MAX_VOLT_NUM;
	
	for (bsu_index = 0; bsu_index < bsu_num; bsu_index++)
	{
        cell_balance_word = 0;
		for (i = 0; i < max_balance_num; i++)
		{
			if (context->volt_item[bsu_index][i].volt > (bcu_get_average_voltage() + config_get(kBYUBalDiffVoltMinIndex)))
			{
				//电池均衡位
				SET(cell_balance_word,context->volt_item[bsu_index][i].pos);
			}
			else
			{
				break;
			}
		}
		set_bsu_passive_balance_word(context,bsu_index, cell_balance_word);
	}
	for (; bsu_index < PARAM_BSU_NUM_MAX; bsu_index++) //没有使用的从机均衡位清零
	{
		set_bsu_passive_balance_word(context,bsu_index, 0);
	} 
}
/*****************************************************************************
 *函数名称:is_bsu_passive_balancing
 *函数功能:判断是否在被动均衡中
 *参    数:无
 *返 回 值:TRUE 是  FALSE否 
 *修订信息:
 ******************************************************************************/
BOOLEAN is_bsu_passive_balancing(void)
{
	INT8U bsu_index = 0;
	INT8U bsu_num = 0;
	bsu_num = get_bsu_num();
	for (bsu_index = 0; bsu_index < bsu_num; bsu_index++)
	{
		if (get_bsu_passive_balance_word(bsu_index) != 0)
		{
			return TRUE;
		}
	}
	return FALSE;
}
/*****************************************************************************
 *函数名称:bsu_passive_balance_ctrl
 *函数功能:被动均衡控制 
 *参    数:无
 *返 回 值:无
 *修订信息:
 ******************************************************************************/
void bsu_passive_balance_ctrl(void)
{
	static INT32U last_tick = 0;//上一个时刻
	INT32U tick_diff = 0;       //时间差
	static INT32U balance_time = 0; //均衡时间

	tick_diff = get_elapsed_tick_count(last_tick);
	last_tick = get_tick_count();

    bsu_passive_balance_max_volt_diff_analyze((INT16U) tick_diff); //被动均衡最大电压压差分析

	if (!is_passive_balance_condition_ok()) //均衡条件不满足,立即停止均衡
	{
		balance_time = 0;  //均衡时间清零
		clr_all_bsu_passive_balance_word();//清除均衡字
	}
	else
	{
		balance_time += tick_diff;
		if (balance_time >= BALANCE_DLAY_5S) 
		{
			select_passive_balance_cell();
			balance_time = 0;
		}
	}
}
#endif
