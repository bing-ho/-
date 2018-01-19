/*******************************************************************************
 **                       安徽力高新能源技术有限公司 Copyright (c)
 **                            http://www...
 **文 件 名:bsu_active_balance.c
 **创建日期:2015.12.24
 **文件说明:电池主动均衡控制
 **修改记录:
 **版    本:V1.0
 **备    注:
 *******************************************************************************/

#include "bsu_active_balance.h"
#include "ltc_util.h"
#include "ltc68041_impl.h" 
#include "bsu_sample.h"
#include "bms_bsu.h"
#if BMS_SUPPORT_HARDWARE_LTC6804 == 1

static SystemPowerVoltType system_power_volt_type  = kSystemPowerVolt_Unknow; //系统供电电压类型(12V or 24V)
static SystemPowerVoltStatus system_power_volt_status = kPowerVolt_Error; //系统电压允许均衡类型
#pragma DATA_SEG __RPAGE_SEG PAGED_RAM_BSU_BALANCE 
static BsuActiveBalanceContext bsu_active_balance_context;
#pragma DATA_SEG DEFAULT

/*****************************************************************************
 *函数名称:system_power_volt_type_analyse
 *函数功能:系统供电电压类型分析
 *参    数:无
 *返 回 值:无
 *修订信息:
 ******************************************************************************/
static void system_power_volt_type_analyse(void)
{
   static INT8U flag = 0;
   static INT8U cnt = 0;
   if(system_power_volt_type != kSystemPowerVolt_Unknow)
   {
        return;
   }
   if(bat_det_voltage_is_usful() == TRUE)
   {
       flag = 1; 
   }
   if(flag == 0)
   {
        return;
   }
   if(bat_det_voltage_get()<=16000)
   {
      if(++cnt > 5)
      {
        cnt = 0;
        system_power_volt_type = kSystemPowerVolt_12V;
      }
   }
   else
   {
      if(++cnt > 5)
      {
        cnt = 0;
        system_power_volt_type = kSystemPowerVolt_24V;
      }
   }
}
/*****************************************************************************
 *函数名称:set_system_power_volt_status
 *函数功能:设置供电电压状态
 *参    数:无
 *返 回 值:无
 *修订信息:
 ******************************************************************************/
void set_system_power_volt_status(INT16U volt,INT16U min_volt, INT16U mid_volt, INT16U max_volt)
{
   static INT8U cnt = 0;
    if(volt <= min_volt)
      {
          if(++cnt > 5)
          {
            cnt = 0;
            system_power_volt_status = kPowerVolt_OverDisChg; //铅酸电池过放
          }
      }
    else if(volt > max_volt)
      {
          if(++cnt > 5)
          {
            cnt = 0;
            system_power_volt_status = kPowerVolt_OverChg;
          }
      }
    else if((volt >= mid_volt)&&(volt <= max_volt))
      {
          if(++cnt > 5)
          {
            cnt = 0;
            system_power_volt_status = kPowerVolt_Charging;
      }
   }
   else
   {
          if(++cnt > 5)
          {
            cnt = 0;
            system_power_volt_status = kPowerVolt_Normal;
          }
    }
}
/*****************************************************************************
 *函数名称:system_power_volt_status_analyse
 *函数功能:根据系统电压确定均衡类型
 *参    数:无
 *返 回 值:无
 *修订信息:
 ******************************************************************************/
static void system_power_volt_status_analyse(void)
{
   system_power_volt_type_analyse();
   if(system_power_volt_type == kSystemPowerVolt_Unknow)
   {
        system_power_volt_status = kPowerVolt_Error;
        return;
   }
   if(system_power_volt_type == kSystemPowerVolt_12V)
   {
       set_system_power_volt_status(bat_det_voltage_get(),BMS_12V_SYSTEM_VOLT_MIN,BMS_12V_SYSTEM_VOLT_MID,BMS_12V_SYSTEM_VOLT_MAX);
   }
   else
   {
       set_system_power_volt_status(bat_det_voltage_get(),BMS_24V_SYSTEM_VOLT_MIN,BMS_24V_SYSTEM_VOLT_MID,BMS_24V_SYSTEM_VOLT_MAX);
   }
}

/*****************************************************************************
 *函数名称:clr_balance_min_volt_item
 *函数功能:
 *参    数:无
 *返 回 值:无
 *修订信息:
 ******************************************************************************/
void clr_balance_min_volt_item(void)
{
	INT8U i = 0;
    BsuActiveBalanceContext *far context = &bsu_active_balance_context;
    for(i = 0;i<PARAM_BSU_NUM_MAX;i++)
    {
        context->min_volt_item[i].volt = 0xFFFF;
        context->min_volt_item[i].pos = 0;
    }
}
/*****************************************************************************
 *函数名称:bsu_statistic_min_volt
 *函数功能:获取每个BSU最小电压
 *参    数:bsu_index 从机号  pos电池位置 volt电池电压
 *返 回 值:无
 *修订信息:
 ******************************************************************************/
void bsu_statistic_min_volt(INT8U bsu_index,INT8U pos,INT16U volt)
{
    const BsuActiveBalanceContext *far context = &bsu_active_balance_context;
    //参数判断
    if(bsu_index >= get_bsu_num())
        return;
	if (pos >= get_bsu_n_cells_num(bsu_index))
		return;

	if (volt < context->min_volt_item[bsu_index].volt)
	{   
	   context->min_volt_item[bsu_index].volt=volt;
	   context->min_volt_item[bsu_index].pos=pos;
	}
}
/*****************************************************************************
 *函数名称:set_bsu_active_balance_word
 *函数功能:设置被动均衡位，用于均衡控制
 *参    数:context均衡内容  bsu_index 从机号  balance_word均衡字
 *返 回 值:无            
 *修订信息:
 ******************************************************************************/
static void set_bsu_active_balance_word(BsuActiveBalanceContext *far context,INT8U bsu_index, INT16U balance_word)
{
	OS_CPU_SR cpu_sr = 0;
	if (bsu_index >= get_bsu_num())
		return;

	OS_ENTER_CRITICAL();
	context->balance_control_word[bsu_index] = balance_word;
	OS_EXIT_CRITICAL();
}
/*****************************************************************************
 *函数名称:get_active_balance_word
 *函数功能:获取被动均衡字
 *参    数:bsu_index 从机号
 *返 回 值:INT16U 均衡位           
 *修订信息:
 ******************************************************************************/
INT16U get_bsu_active_balance_word(INT8U bsu_index)
{
	OS_CPU_SR cpu_sr = 0;
	INT16U balance_word = 0;
	const BsuActiveBalanceContext *far context = &bsu_active_balance_context;
	if (bsu_index >= get_bsu_num())
		return 0;

	OS_ENTER_CRITICAL();
	balance_word = context->balance_control_word[bsu_index]&0x0FFF;
	OS_EXIT_CRITICAL();
	return balance_word;
}
/*****************************************************************************
 *函数名称:clr_all_bsu_active_balance_word
 *函数功能:清除所有的均衡位
 *参    数:无
 *返 回 值:无    
 *修订信息:
 ******************************************************************************/
static void clr_all_bsu_active_balance_word(void)
{
	INT8U bsu_index = 0;
	for (bsu_index = 0; bsu_index < PARAM_BSU_NUM_MAX; bsu_index++)
	{
		set_bsu_active_balance_word(&bsu_active_balance_context,bsu_index, 0);
	}
}
/*****************************************************************************
 *函数名称:bsu_active_balance_volt_diff_analyze
 *函数功能:被动均衡最大电压压差分析 与所有电池平均电压比较
 *参    数:INT16U time时间间隔
 *返 回 值:无
 *修订信息:
 ******************************************************************************/
static void bsu_active_balance_volt_diff_analyze(INT16U time)
{
    BsuActiveBalanceContext *far context = &bsu_active_balance_context;
    
	switch (context->volt_diff_status)
	{
		case kActiveBalanceVoltDiffNormal:

			if ((bcu_get_high_voltage() >= (bcu_get_average_voltage() + config_get(kBYUBalDiffVoltMaxIndex)))//大于所有电池平均电压+均衡开始压差
			    ||((bcu_get_low_voltage()+ config_get(kBYUBalDiffVoltMaxIndex)) <= bcu_get_average_voltage() ))
			{
				context->status_dly += time;
				if (context->status_dly >= BALANCE_DLAY_2S) //延时2S
				{
					context->status_dly = 0;
					context->volt_diff_status = kActiveBalanceVoltDiffBig;
				}
			}
			else
			{
				context->status_dly = 0;
			}

		break;
		case kPassiveBalanceVoltDiffBig:
			if ((bcu_get_high_voltage() < (bcu_get_average_voltage() + config_get(kBYUBalDiffVoltMinIndex))) //小于所有电池平均电压+均衡关闭压差
			   &&((bcu_get_low_voltage()+config_get(kBYUBalDiffVoltMinIndex)) > bcu_get_average_voltage()))
			{
				context->status_dly += time;
				if (context->status_dly >= BALANCE_DLAY_30S) //延时30S
				{
					context->status_dly = 0;
					context->volt_diff_status = kActiveBalanceVoltDiffNormal;
				}
			}
			else
			{
				context->status_dly = 0;
			}
		break;
		default:
			context->status_dly = 0;
			context->volt_diff_status = kActiveBalanceVoltDiffNormal;
		break;
	}
}

 /*****************************************************************************
 *函数名称:is_active_balance_condition_ok
 *函数功能:被动均衡条件是否成立
 *参    数:无
 *返 回 值:TRUE条件成立 FALSE条件不成立
 *修订信息:
 ******************************************************************************/
static BOOLEAN is_active_balance_condition_ok(void)
{
	const BsuActiveBalanceContext *far context = &bsu_active_balance_context;

	if ((bcu_get_dchg_ht_state() == kAlarmNone)  //充电没有过温报警和保护
		&&(bcu_get_chg_ht_state() == kAlarmNone)  //放电没有过温报警和保护
		&&(config_get(kBYUBalNumIndex) > 0)  //均衡路数大于0
	    &&(context->volt_diff_status == kPassiveBalanceVoltDiffBig)  //满足 最大电池电压与平均电压压差
	    &&(is_bsu_comm_error()==FALSE) //没有通信中断
	    &&(is_volt_sample_cable_open() == FALSE) //电压采集排线脱落
	    &&(config_get(kBmuBalanceEnableIndex)) //均衡使能
	    &&(system_power_volt_status != kPowerVolt_Error)
	    &&(bcu_get_temp_exception() == 0)
	    &&(bcu_get_battery_insulation_state() == kAlarmNone)
	    &&(bcu_get_delta_temperature_state() == kAlarmNone)
	    ) 
	{
		return TRUE;
	}
	return FALSE;
}
/*****************************************************************************
 *函数名称:select_cell_only_chg_balance
 *函数功能:仅给锂电池充电均衡
 *参    数:无
 *返 回 值:无
 *修订信息:
 ******************************************************************************/
static void select_cell_only_chg_balance(BsuActiveBalanceContext *far context)
{
    INT16U cell_balance_word = 0;     //均衡位
    INT8U bsu_index = 0;       //从机编号计数
    INT8U bsu_num = 0;
    bsu_num = get_bsu_num(); //从机数目
    for (bsu_index = 0; bsu_index < bsu_num; bsu_index++)
    {
        cell_balance_word = 0;
        if(((context->min_volt_item[bsu_index].volt + config_get(kBYUBalDiffVoltMinIndex)) < bcu_get_average_voltage())
    #if BMS_DIAGNOSIS_HV_INDEPENDENT_WITH_CUR_EN      
        &&(context->min_volt_item[bsu_index].volt < config_get(kChgHVTrdAlarmIndex)))
    #else
        &&(context->min_volt_item[bsu_index].volt < config_get(kDChgHVTrdAlarmIndex)))
    #endif
        {   //充电均衡
            SET(cell_balance_word,context->min_volt_item[bsu_index].pos); 
            CLR(cell_balance_word,15); 
        }
        set_bsu_active_balance_word(context,bsu_index, cell_balance_word);
    }
    for (; bsu_index < PARAM_BSU_NUM_MAX; bsu_index++) //没有使用的从机均衡位清零
    {
        set_bsu_active_balance_word(context,bsu_index, 0);
    } 
}
 /*****************************************************************************
 *函数名称:select_cell_only_dischg_balance
 *函数功能:仅给锂电池放电均衡
 *参    数:无
 *返 回 值:无
 *修订信息:
 ******************************************************************************/
static void select_cell_only_dischg_balance(BsuActiveBalanceContext *far context)
{
    INT16U cell_balance_word = 0;     //均衡位
    INT8U bsu_index = 0;       //从机编号计数
    INT8U bsu_num = 0;
    bsu_num = get_bsu_num(); //从机数目
    for (bsu_index = 0; bsu_index < bsu_num; bsu_index++)
    {
        cell_balance_word = 0;
        if ((get_bsu_max_cell_volt(bsu_index) > (bcu_get_average_voltage() + config_get(kBYUBalDiffVoltMinIndex)))
        &&(get_bsu_max_cell_volt(bsu_index) > config_get(kDChgLVTrdAlarmIndex)))
		{   //放电均衡
			SET(cell_balance_word,get_bsu_max_cell_volt_pos(bsu_index));
			SET(cell_balance_word,15); 
		}
        set_bsu_active_balance_word(context,bsu_index, cell_balance_word);
    }
    for (; bsu_index < PARAM_BSU_NUM_MAX; bsu_index++) //没有使用的从机均衡位清零
    {
        set_bsu_active_balance_word(context,bsu_index, 0);
    } 
}
 /*****************************************************************************
 *函数名称:select_cell_auto_balance
 *函数功能:自由均衡
 *参    数:无
 *返 回 值:无
 *修订信息:
 ******************************************************************************/
static void select_cell_auto_balance(BsuActiveBalanceContext *far context)
{
	INT16U cell_balance_word = 0;     //均衡位
	INT8U bsu_index = 0;       //从机编号计数
	INT8U bsu_num = 0;
	INT16U max_volt_diff=0,min_volt_diff = 0;
	bsu_num = get_bsu_num(); //从机数目
	
	for (bsu_index = 0; bsu_index < bsu_num; bsu_index++)
	{
        cell_balance_word = 0;
        max_volt_diff=0,min_volt_diff = 0;
        if(get_bsu_max_cell_volt(bsu_index) > bcu_get_average_voltage())
	    {
	        max_volt_diff =  get_bsu_max_cell_volt(bsu_index) - bcu_get_average_voltage();
	    }
	    if(bcu_get_average_voltage() > context->min_volt_item[bsu_index].volt)
	    {
	        min_volt_diff = bcu_get_average_voltage() - context->min_volt_item[bsu_index].volt;
	    }
	    if((max_volt_diff > config_get(kBYUBalDiffVoltMinIndex))&&(min_volt_diff > config_get(kBYUBalDiffVoltMinIndex)))
	    {
            if((max_volt_diff >= min_volt_diff)&&
            (get_bsu_max_cell_volt(bsu_index) > config_get(kDChgLVTrdAlarmIndex)))
            {
                SET(cell_balance_word,get_bsu_max_cell_volt_pos(bsu_index)); 
                SET(cell_balance_word,15);
            }
        #if BMS_DIAGNOSIS_HV_INDEPENDENT_WITH_CUR_EN 
            else if(context->min_volt_item[bsu_index].volt < config_get(kChgHVTrdAlarmIndex))
        #else
            else if(context->min_volt_item[bsu_index].volt < config_get(kDChgHVTrdAlarmIndex))
        #endif
		    {
				SET(cell_balance_word,context->min_volt_item[bsu_index].pos); 
				CLR(cell_balance_word,15); 
		    }
		}
	    else if(max_volt_diff > config_get(kBYUBalDiffVoltMinIndex))
	    {
	        if(get_bsu_max_cell_volt(bsu_index) > config_get(kDChgLVTrdAlarmIndex))
		{
				SET(cell_balance_word,get_bsu_max_cell_volt_pos(bsu_index));
				SET(cell_balance_word,15); 
			}
		}
	    else  if(min_volt_diff > config_get(kBYUBalDiffVoltMinIndex))
	    {
	    #if BMS_DIAGNOSIS_HV_INDEPENDENT_WITH_CUR_EN    
	        if(context->min_volt_item[bsu_index].volt < config_get(kChgHVTrdAlarmIndex))
	    #else
	        if(context->min_volt_item[bsu_index].volt < config_get(kDChgHVTrdAlarmIndex))
	    #endif
		    {
                SET(cell_balance_word,context->min_volt_item[bsu_index].pos);
	            CLR(cell_balance_word,15); 
            }
	    }
        set_bsu_active_balance_word(context,bsu_index, cell_balance_word);
    }
    for (; bsu_index < PARAM_BSU_NUM_MAX; bsu_index++) //没有使用的从机均衡位清零
    {
        set_bsu_active_balance_word(context,bsu_index, 0);
    } 
}
/*****************************************************************************
 *函数名称:select_cell_balance_with_power_charging
 *函数功能:自由均衡
 *参    数:无
 *返 回 值:无
 *修订信息:
 ******************************************************************************/
static void get_cell_need_balance_num_by_auto(BsuActiveBalanceContext *far context,INT8U *chg_num,INT8U *dischg_num)
{
    INT8U bsu_index = 0;       //从机编号计数
    INT8U bsu_num = 0;
    INT16U max_volt_diff=0,min_volt_diff = 0;
    bsu_num = get_bsu_num(); //从机数目
    
    for (bsu_index = 0; bsu_index < bsu_num; bsu_index++)
    {
            max_volt_diff=0,min_volt_diff = 0;
		    if(get_bsu_max_cell_volt(bsu_index) > bcu_get_average_voltage())
		    {
		        max_volt_diff =  get_bsu_max_cell_volt(bsu_index) - bcu_get_average_voltage();
		    }
		    if(bcu_get_average_voltage() > context->min_volt_item[bsu_index].volt)
		    {
		        min_volt_diff = bcu_get_average_voltage() - context->min_volt_item[bsu_index].volt;
		    }
		    if((max_volt_diff > config_get(kBYUBalDiffVoltMinIndex))&&(min_volt_diff > config_get(kBYUBalDiffVoltMinIndex)))
		    {
               if((max_volt_diff >= min_volt_diff)&&
               (get_bsu_max_cell_volt(bsu_index) > config_get(kDChgLVTrdAlarmIndex)))
		       {
                  (*dischg_num)++;
		       }
		    #if BMS_DIAGNOSIS_HV_INDEPENDENT_WITH_CUR_EN 
               else if(context->min_volt_item[bsu_index].volt < config_get(kChgHVTrdAlarmIndex))
            #else
               else if(context->min_volt_item[bsu_index].volt < config_get(kDChgHVTrdAlarmIndex))
            #endif
		       {
	              (*chg_num)++; 
		       }
		    }
		    else if(max_volt_diff > config_get(kBYUBalDiffVoltMinIndex))
		    {
    	        if(get_bsu_max_cell_volt(bsu_index) > config_get(kDChgLVTrdAlarmIndex))
                {
                    (*dischg_num)++;
                }
		    }
		    else if(min_volt_diff > config_get(kBYUBalDiffVoltMinIndex))
		    {
    	    #if BMS_DIAGNOSIS_HV_INDEPENDENT_WITH_CUR_EN 
    	        if(context->min_volt_item[bsu_index].volt < config_get(kChgHVTrdAlarmIndex))
    	    #else
    	        if(context->min_volt_item[bsu_index].volt < config_get(kDChgHVTrdAlarmIndex))
    	    #endif
                {
    	            (*chg_num)++;
                }
	        }
    }
}
/*****************************************************************************
 *函数名称:volt_diff_sort
 *函数功能:压差排序
 *参    数:无
 *返 回 值:无
 *修订信息:
 ******************************************************************************/
static void volt_diff_sort(BsuActiveBalanceContext *far context,INT8U bsu_index,INT16U volt_diff,INT8U pos,INT8U is_max_volt_diff)
{
   INT8U i = 0, move_num = 0, move_i = 0, satrt_i = 0; 
   for (i = 0; i < VOLT_DIFF_SORT_NUM; i++)
    {
    	if (volt_diff > context->volt_diff_item[i].volt_diff)
    	{   //第[0]个为最高电压,如果加入的电压大于第[0]个电压,那么所有电压后移
    		move_num = (INT8U) VOLT_DIFF_SORT_NUM - i - 1;
    		satrt_i = (INT8U) VOLT_DIFF_SORT_NUM - 1;
    		for (move_i = move_num; move_i > 0; move_i--)
    		{
    			context->volt_diff_item[satrt_i].volt_diff = context->volt_diff_item[satrt_i - 1].volt_diff;
    			context->volt_diff_item[satrt_i].bsu_index = context->volt_diff_item[satrt_i - 1].bsu_index;
    			context->volt_diff_item[satrt_i].pos.pos_byte = context->volt_diff_item[satrt_i - 1].pos.pos_byte;
    			satrt_i--;
    		}
    		context->volt_diff_item[satrt_i].volt_diff = volt_diff;
    		context->volt_diff_item[satrt_i].pos.pos_bits.pos = pos;
    		context->volt_diff_item[satrt_i].bsu_index = bsu_index;
    		context->volt_diff_item[satrt_i].pos.pos_bits.is_max_volt_diff = is_max_volt_diff;
    		break;
    	}
    }
}
/*****************************************************************************
 *函数名称:select_cell_balance_with_power_charging
 *函数功能:铅酸浮充下选择电池均衡
 *参    数:无
 *返 回 值:无
 *修订信息:
 ******************************************************************************/
static void select_cell_balance_with_power_charging(BsuActiveBalanceContext *far context,INT8U chg_num,INT8U dischg_num)
{
    INT8U bsu_index = 0,i = 0;       //从机编号计数
    INT8U bsu_num = 0;
    INT16U volt_diff = 0;
    INT8U num = 0;
    INT8U cnt = 0;
    INT16U cell_balance_word = 0; 
    
    num = chg_num + dischg_num;
    if(num == 0)
        return;
    
    bsu_num = get_bsu_num(); //从机数目
    //清除排序内容
    for(i = 0;i < VOLT_DIFF_SORT_NUM;i++)
    {
        context->volt_diff_item[i].volt_diff = 0;
        context->volt_diff_item[i].pos.pos_byte = 0;
    }
    //将需要均衡的压差进行排序
    for (bsu_index = 0; bsu_index < bsu_num; bsu_index++)
    {
        if((get_bsu_max_cell_volt(bsu_index) > (bcu_get_average_voltage()+config_get(kBYUBalDiffVoltMinIndex)))
        &&(get_bsu_max_cell_volt(bsu_index) > config_get(kDChgLVTrdAlarmIndex)))
	    {
	        volt_diff =  get_bsu_max_cell_volt(bsu_index) - bcu_get_average_voltage();
	        volt_diff_sort(context,bsu_index,volt_diff,get_bsu_max_cell_volt_pos(bsu_index),1);
	    }
	    if((bcu_get_average_voltage() > (context->min_volt_item[bsu_index].volt + config_get(kBYUBalDiffVoltMinIndex)))
	 #if BMS_DIAGNOSIS_HV_INDEPENDENT_WITH_CUR_EN 
	    &&(context->min_volt_item[bsu_index].volt < config_get(kChgHVTrdAlarmIndex)))
	 #else
	    &&(context->min_volt_item[bsu_index].volt < config_get(kDChgHVTrdAlarmIndex)))
	 #endif
	    {
	        volt_diff =  bcu_get_average_voltage() - context->min_volt_item[bsu_index].volt;
	        volt_diff_sort(context,bsu_index,volt_diff,context->min_volt_item[bsu_index].pos,0);
	    }
    }
    //第一个压差最大必须均衡
    cell_balance_word = 0; 
    if(context->volt_diff_item[0].volt_diff > config_get(kBYUBalDiffVoltMinIndex))
    {
        SET(cell_balance_word,context->volt_diff_item[0].pos.pos_bits.pos); 
        if(context->volt_diff_item[0].pos.pos_bits.is_max_volt_diff)
        { //放电均衡类型
            SET(cell_balance_word,15);
            cnt++;
        }
        else
        {
            CLR(cell_balance_word,15);
        }
        set_bsu_active_balance_word(context,context->volt_diff_item[0].bsu_index, cell_balance_word);
	}
    //选择 (num - 1)/2个放电均衡
    num = (INT8U)((num - 1)>>1);
    for(i = 0;i < VOLT_DIFF_SORT_NUM;i++)
    {
       if((cnt+1) >= num)
       {
            break;
       }
       cell_balance_word = 0; 
       if(context->volt_diff_item[i].pos.pos_bits.is_max_volt_diff)//放电均衡类型
       {
          if(context->volt_diff_item[i].volt_diff > config_get(kBYUBalDiffVoltMinIndex))
          {
               SET(cell_balance_word,context->volt_diff_item[i].pos.pos_bits.pos); 
               SET(cell_balance_word,15);
               set_bsu_active_balance_word(context,context->volt_diff_item[i].bsu_index, cell_balance_word);
               cnt++;
          }
       }
    }
    //剩余的全部为充电均衡
    for (bsu_index = 0; bsu_index < bsu_num; bsu_index++)
    {
        cell_balance_word = 0;
        if(get_bsu_active_balance_word(bsu_index) != 0)//已经有均衡位
        {
            continue;
        }
        if(((context->min_volt_item[bsu_index].volt + config_get(kBYUBalDiffVoltMinIndex)) < bcu_get_average_voltage())
     #if BMS_DIAGNOSIS_HV_INDEPENDENT_WITH_CUR_EN  
        &&(context->min_volt_item[bsu_index].volt < config_get(kChgHVTrdAlarmIndex)))
     #else
        &&(context->min_volt_item[bsu_index].volt < config_get(kDChgHVTrdAlarmIndex)))
     #endif
        {   //充电均衡
            SET(cell_balance_word,context->min_volt_item[bsu_index].pos); 
            CLR(cell_balance_word,15); 
		}
		set_bsu_active_balance_word(context,bsu_index, cell_balance_word);
	}
}
/*****************************************************************************
 *函数名称:select_active_balance_cell
 *函数功能:选择需要被动均衡的电池
 *参    数:无
 *返 回 值:无
 *修订信息:
 ******************************************************************************/
static void select_active_balance_cell(void)
{
    BsuActiveBalanceContext *far context = &bsu_active_balance_context;
    INT8U chg_num = 0,dischg_num = 0; 
	switch(system_power_volt_status)
	{
    	case kPowerVolt_Error://故障
    	     clr_all_bsu_active_balance_word();
    	break;
        case kPowerVolt_Normal://正常
             select_cell_auto_balance(context);
        break;
        case kPowerVolt_OverChg://过充
             select_cell_only_chg_balance(context);//锂电充电均衡，给铅酸放电 
        break;
        case kPowerVolt_OverDisChg://过放
             select_cell_only_dischg_balance(context);//锂电放电均衡，给铅酸充电 
        break;
        case kPowerVolt_Charging://浮充
             get_cell_need_balance_num_by_auto(context,&chg_num,&dischg_num);
             if((chg_num+dischg_num) == 0)//没有满足条件的均衡
                return;
             if((chg_num >= dischg_num)||(dischg_num == 1)||((chg_num+dischg_num)==1))
             { //自由均衡
                select_cell_auto_balance(context);
             }
             else
             { //放电均衡路数最大只能是 (chg_num + dischg_num -1)/2个
                select_cell_balance_with_power_charging(context,chg_num,dischg_num);
             }
             
        break;
    	default:
    	break;
	} 
}
/*****************************************************************************
 *函数名称:is_bsu_active_balancing
 *函数功能:判断是否在被动均衡中
 *参    数:无
 *返 回 值:TRUE 是  FALSE否 
 *修订信息:
 ******************************************************************************/
BOOLEAN is_bsu_active_balancing(void)
{
	INT8U bsu_index = 0;
	INT8U bsu_num = 0;
	bsu_num = get_bsu_num();
	for (bsu_index = 0; bsu_index < bsu_num; bsu_index++)
	{
		if (get_bsu_active_balance_word(bsu_index) != 0)
		{
			return TRUE;
		}
	}
	return FALSE;
}
/*****************************************************************************
 *函数名称:bsu_active_balance_ctrl
 *函数功能:主动均衡控制 
 *参    数:无
 *返 回 值:无
 *修订信息:
 ******************************************************************************/
void send_active_balance_cmd_to_ltc6804(INT8U bsu_num)
{
    BsuActiveBalanceContext *far context = &bsu_active_balance_context;
    (void)write_balance_info_to_dsp(bsu_num,DSP_IIC_ADDR,context->balance_control_word,ACTIVE_BALANCE_CURRENT);
}
/*****************************************************************************
 *函数名称:read_active_balance_current
 *函数功能:获取均衡电流
 *参    数:无
 *返 回 值:无
 *修订信息:
 ******************************************************************************/
void read_active_balance_current(INT8U bsu_num)
{
    BsuActiveBalanceContext *far context = &bsu_active_balance_context;
    (void)read_balance_current_from_dsp(bsu_num,DSP_IIC_ADDR,context->balance_current);
}
/*****************************************************************************
 *函数名称:get_bsu_active_balance_current
 *函数功能:获取均衡电流
 *参    数:无
 *返 回 值:无
 *修订信息:
 ******************************************************************************/
INT8U get_bsu_active_balance_current(INT8U bsu_num)
{
    INT8U current = 0;
    OS_CPU_SR cpu_sr = 0;
    BsuActiveBalanceContext *far context = &bsu_active_balance_context;
    if(bsu_num >= PARAM_BSU_NUM_MAX)
        return 0;
    OS_ENTER_CRITICAL();
    current = context->balance_current[bsu_num];
    OS_EXIT_CRITICAL();
    return current;
}
/*****************************************************************************
 *函数名称:get_bsu_active_balance_current
 *函数功能:清除均衡电流
 *参    数:无
 *返 回 值:无
 *修订信息:
 ******************************************************************************/
void clear_bsu_active_balance_current(INT8U bsu_num)
{
    INT16U current = 0;
    OS_CPU_SR cpu_sr = 0;
    BsuActiveBalanceContext *far context = &bsu_active_balance_context;
    if(bsu_num >= PARAM_BSU_NUM_MAX)
        return ;
    OS_ENTER_CRITICAL();
    context->balance_current[bsu_num] = 0;
    OS_EXIT_CRITICAL();
}
/*****************************************************************************
 *函数名称:is_active_balance_error
 *函数功能:均衡故障
 *参    数:无
 *返 回 值:无
 *修订信息:
 ******************************************************************************/
INT8U is_active_balance_error(INT8U bsu_index)
{
   INT32U flag = 0;
   OS_CPU_SR cpu_sr = 0;
   BsuActiveBalanceContext *far context = &bsu_active_balance_context;
   OS_ENTER_CRITICAL();
   flag = context->balance_error;
   OS_EXIT_CRITICAL();
   if(GET(flag,bsu_index))
   {
        return TRUE;
   }
   return FALSE;
}
/*****************************************************************************
 *函数名称:balance_error_analyze
 *函数功能:主动均衡故障
 *参    数:无
 *返 回 值:无
 *修订信息:
 ******************************************************************************/
 void balance_error_analyze(void)
 {
	INT8U bsu_index = 0;
	INT8U bsu_num = 0;
	OS_CPU_SR cpu_sr = 0;
	BsuActiveBalanceContext *far context = &bsu_active_balance_context;
	bsu_num = get_bsu_num();
	for (bsu_index = 0; bsu_index < bsu_num; bsu_index++)
	{
		if ((get_bsu_active_balance_word(bsu_index) != 0)&&(get_bsu_active_balance_current(bsu_index) == 0))
		{
            OS_ENTER_CRITICAL();
            SET(context->balance_error,bsu_index);
            OS_EXIT_CRITICAL();	
		}
		else
		{
		    OS_ENTER_CRITICAL();
            CLR(context->balance_error,bsu_index);
            OS_EXIT_CRITICAL();
		}
	}
}
 
 /*****************************************************************************
 *函数名称:bsu_active_balance_ctrl
 *函数功能:主动均衡控制 
 *参    数:无
 *返 回 值:无
 *修订信息:
 ******************************************************************************/
void bsu_active_balance_ctrl(void)
{
	static INT32U last_tick = 0;//上一个时刻
	INT32U tick_diff = 0;       //时间差
	static INT32U balance_time = 0; //均衡时间

	tick_diff = get_elapsed_tick_count(last_tick);
	last_tick = get_tick_count();
    system_power_volt_status_analyse();
    bsu_active_balance_volt_diff_analyze((INT16U) tick_diff); //被动均衡最大电压压差分析

	if (!is_active_balance_condition_ok()) //均衡条件不满足,立即停止均衡
	{
		balance_time = 0;  //均衡时间清零
		clr_all_bsu_active_balance_word();//清除均衡字
	}
	else
	{
		if(is_bsu_active_balancing() == TRUE)
		{
    		balance_time += tick_diff;
    		if(balance_time >= BALANCE_DLAY_5S)
    		{
    		   balance_error_analyze();
    		}
    		if (balance_time >= BALANCE_DLAY_1MIN) 
    		{
    			clr_all_bsu_active_balance_word();
    			balance_time = 0;
    		}
		}
		else
		{
		    balance_time += tick_diff;
    		if (balance_time >= BALANCE_DLAY_3S) 
    		{
    			select_active_balance_cell();
    			balance_time = 0;
    		}
    	}
    }
}


#endif