/*******************************************************************************
 **                       安徽力高新能源技术有限公司 Copyright (c)
 **                            http://www...
 **文 件 名:bms_bsu.c
 **创建日期:2015.10.15
 **文件说明:电压温度更新到BCU中
 **修改记录:
 **版    本:V1.0
 **备    注:
 *******************************************************************************/

#include "includes.h"
#include "bsu_sample.h"
#include "bms_bsu.h"
#include "bms_bmu.h"
#include "bms_config.h"
#include "bsu_passive_balance.h"
#include "bsu_active_balance.h"
#include "ltc_util.h"
#include "main_bsu_relay_ctrl.h"
#include "ltc68041_impl.h" 
#if BMS_SUPPORT_HARDWARE_LTC6804 == 1
//主采集板板在所有的采集板中的位置
static INT8U main_bsu_pos[BMU_MAX_SLAVE_COUNT + 1] = {0xFF};
static INT8U bmu_max_temp_from_bsu[BMU_MAX_SLAVE_COUNT] = {0};
static INT8U bmu_min_temp_from_bsu[BMU_MAX_SLAVE_COUNT] = {0};
void bsu_update_bcu_info(void* data);

/*****************************************************************************
 *函数名称:bmu_set_max_temp_by_bsu
 *函数功能:设置bmu从机最大温度
 *参    数:INT8U slave_index从机号,INT8U temp温度  偏移量-50℃
 *返 回 值:无
 *修订信息:
 ******************************************************************************/
static void bmu_set_max_temp_by_bsu(INT8U slave_index,INT8U temp)
{
     OS_INIT_CRITICAL();
     
     if(slave_index >= BMU_MAX_SLAVE_COUNT)
          return;
     OS_ENTER_CRITICAL();
     if ((temp == TEMP_CABLE_SHORT_VAULE) || (temp == TEMP_CABLE_OPEN_VAULE))
        bmu_max_temp_from_bsu[slave_index]= 0;
     else
        bmu_max_temp_from_bsu[slave_index]= temp;
     OS_EXIT_CRITICAL();
}
/*****************************************************************************
 *函数名称:bmu_get_max_temp_from_bsu
 *函数功能:获取最大温度
 *参    数:INT8U slave_index从机号,
 *返 回 值:INT8U temp温度  偏移量-50℃
 *修订信息:
 ******************************************************************************/
INT8U bmu_get_max_temp_from_bsu(INT8U slave_index)
{
     OS_INIT_CRITICAL();
     INT8U temp;
     if(slave_index >= BMU_MAX_SLAVE_COUNT)
          return 0;
     OS_ENTER_CRITICAL();
     temp = bmu_max_temp_from_bsu[slave_index];
     OS_EXIT_CRITICAL();
     return temp;
}
/*****************************************************************************
 *函数名称:bmu_set_min_temp_by_bsu
 *函数功能:设置最大温度
 *参    数:INT8U slave_index从机号,INT8U temp温度  偏移量-50℃
 *返 回 值:无
 *修订信息:
 ******************************************************************************/
static void bmu_set_min_temp_by_bsu(INT8U slave_index,INT8U temp)
{
     OS_INIT_CRITICAL();
     
     if(slave_index >= BMU_MAX_SLAVE_COUNT)
          return;
     OS_ENTER_CRITICAL();
     if ((temp == TEMP_CABLE_SHORT_VAULE) || (temp == TEMP_CABLE_OPEN_VAULE))
        bmu_min_temp_from_bsu[slave_index]= 0;
     else
        bmu_min_temp_from_bsu[slave_index]= temp;
     OS_EXIT_CRITICAL();
}
/*****************************************************************************
 *函数名称:bmu_get_min_temp_from_bsu
 *函数功能:获取最大温度
 *参    数:INT8U slave_index从机号,
 *返 回 值:INT8U temp温度  偏移量-50℃
 *修订信息:
 ******************************************************************************/
INT8U bmu_get_min_temp_from_bsu(INT8U slave_index)
{
     OS_INIT_CRITICAL();
     INT8U temp;
     if(slave_index >= BMU_MAX_SLAVE_COUNT)
          return 0;
     OS_ENTER_CRITICAL();
     temp = bmu_min_temp_from_bsu[slave_index];
     OS_EXIT_CRITICAL();
     return temp;
}
/*****************************************************************************
 *函数名称:bsu_init
 *函数功能:从机bsu创建任务
 *参    数:无
 *返 回 值:无
 *修订信息:
 ******************************************************************************/
void bsu_init(void)
{
	INT8U index = 0;
	if (config_get(kBCUCommModeIndex) == BMS_COMM_SLAVE_MODE) //判断是不是主从代码一体模式
		return;
    main_bsu_relay_control_load_config();
	BSUVoltTempSampleTask_Init(); //创建任务
	#if  BMS_SUPPORT_BY5248D == 0   //板载无需继电器控制
	job_schedule(MAIN_JOB_GROUP, 500, bsu_cool_analyse, NULL);
	job_schedule(MAIN_JOB_GROUP, 500, bsu_heat_analyse, NULL);
	#endif
	job_schedule(MAIN_JOB_GROUP, 200, bsu_update_bcu_info, NULL);
}
/*****************************************************************************
 *函数名称:main_bsu_pos_set
 *函数功能:根据板子类型设置主采集板位置
 *参    数:无
 *返 回 值:无
 *修订信息:
 ******************************************************************************/
void main_bsu_pos_set(void)
{
	INT8U bsu_num = 0; //从机数
	INT8U bsu_index = 0;
	INT8U slave_index = 0;
	bsu_num = get_bsu_num(); //获取从机数
	switch(config_get(kBYUBalTypeIndex) )
	{
    	case BSU_PASSIVE_BALANCE_TYPE:   
	for (bsu_index = 0; bsu_index < bsu_num; bsu_index++)
	{//第一个一定为主从机
		if ((bsu_index == 0) || (get_bsu_n_board_type(bsu_index) == kBoardMain) || //是主从机
        	      ((slave_index > 0) && ((main_bsu_pos[slave_index - 1] + BMU_SUPPORT_BSU_MAX_NUM) <= bsu_index))) //从机最大采集板为5个
		{
			if (slave_index < BMU_MAX_SLAVE_COUNT) //从机最大25个
			{
				main_bsu_pos[slave_index] = bsu_index;
				slave_index++;
			}
		}
	}
    	break;
    	case BSU_ACTIVE_BALANCE_TYPE: //主动均衡每个采集板独立为一个从机
        	for (bsu_index = 0; bsu_index < bsu_num; bsu_index++)
        	{
            	if (slave_index < BMU_MAX_SLAVE_COUNT) //从机最大25个
    			{
    				main_bsu_pos[slave_index] = bsu_index;
    				slave_index++;
    			}
        		
        	}
    	break;
    	default:
    	break;
	}
	for (; slave_index <= BMU_MAX_SLAVE_COUNT; slave_index++)
	{
	   main_bsu_pos[slave_index] = 0xFF;
	}
	
}
/*****************************************************************************
 *函数名称:get_main_bsu_pos
 *函数功能:获取主从机的位置
 *参    数:INT8U slave_index从机号
 *返 回 值:主机采集板在所有采集板中的位置
 *修订信息:
 ******************************************************************************/
INT8U get_main_bsu_pos(INT8U slave_index)
{
	if (slave_index >= BMU_MAX_SLAVE_COUNT)
	{
		return 0xFF;
	}
	return main_bsu_pos[slave_index];
}
/*****************************************************************************
 *函数名称:bsu_update_bcu_volt_info
 *函数功能:采集板BSU更新电池电压信息到BCU
 *参    数:无
 *返 回 值:无
 *修订信息:
 ******************************************************************************/
void bsu_update_bcu_volt_info(void)
{
	INT8U bsu_index = 0; //采集板索引
	INT8U bsu_n_cells_num = 0;  //第N个采集板电池数
	INT8U index = 0;
	INT8U cell_index = 0;
	INT8U bsu_num = 0; //采集板数
	INT8U slave_index = 0;
	INT8U slave_num = 0; //从机个数
	INT8U slave_cell_num = 0; //从机电池数
	INT16U volt = 0; //电压
	BOOLEAN comm_err_flag = 0; //通信标志

	clr_bsu_passive_balance_multi_volt_item(); //清除均衡需要的多路最高电压信息
	clr_balance_min_volt_item();
	slave_num = (INT8U) config_get(kSlaveNumIndex); //获取从机数
	bsu_num = get_bsu_num(); //采集板数目
	for (slave_index = 0; slave_index < slave_num; slave_index++)
	{
		cell_index = 0;
		slave_cell_num = 0;
		for (bsu_index = main_bsu_pos[slave_index]; bsu_index < main_bsu_pos[slave_index + 1]; bsu_index++)
		{
			if (bsu_index >= bsu_num) //异常
			{
				break;
			}
			comm_err_flag = is_bsu_n_comm_error(bsu_index);
			bsu_n_cells_num = get_bsu_n_cells_num(bsu_index);
			slave_cell_num += bsu_n_cells_num;
			for (index = 0; index < bsu_n_cells_num; index++) //最高、最低电压扫描,总压计算
			{
				volt = get_bsu_cells_volt(bsu_index, index);

				if (comm_err_flag == TRUE) //通信中断
				{
					bmu_set_voltage_item(slave_index, cell_index, 0);
				}
				else
				{
					bmu_set_voltage_item(slave_index, cell_index, volt);
				}
				cell_index++;
				bsu_statistic_multi_max_volt(bsu_index, index, volt); //更新均衡需要的多路最高电压信息
				bsu_statistic_min_volt(bsu_index, index, volt);
			}
		}
		bmu_set_voltage_num(slave_index, slave_cell_num); //设置电池数
	}
}
/*****************************************************************************
 *函数名称:bsu_update_bcu_temp_info
 *函数功能:从机BSU更新电池温度信息到BCU
 *参    数:无
 *返 回 值:无
 *修订信息:
 ******************************************************************************/
void bsu_update_bcu_temp_info(void)
{
	INT8U bsu_index = 0, index = 0;
	INT8U bsu_num = 0;  //采集板数
	INT8U temp = 0;  //温度
	INT8U offset = 0;
	INT8U temp_index = 0;
	INT8U bsu_n_temp_num = 0; //采集板温感数
	INT8U slave_index = 0;
	INT8U slave_num = 0;
	INT8U slave_temp_num = 0;
	INT8U HT = 0;
    INT8U LT = TEMP_CABLE_SHORT_VAULE;
	BOOLEAN comm_err_flag = 0; //通信标志

	slave_num = (INT8U) config_get(kSlaveNumIndex); //获取从机数
	bsu_num = get_bsu_num();
	for (slave_index = 0; slave_index < slave_num; slave_index++)
	{
		temp_index = 0;
		slave_temp_num = 0;
		HT = 0;
        LT = TEMP_CABLE_SHORT_VAULE;
		for (bsu_index = main_bsu_pos[slave_index]; bsu_index < main_bsu_pos[slave_index + 1]; bsu_index++)
		{
			INT8U start_index = 0;
			INT8U end_index = 0;
			if (bsu_index >= bsu_num)
			{
				break;
			}
			comm_err_flag = is_bsu_n_comm_error(bsu_index);
			bsu_n_temp_num = get_bsu_n_temp_num(bsu_index);
			slave_temp_num += bsu_n_temp_num;
		#if  BMS_SUPPORT_BY5248D == 0 
			if ((config_get(kBYUBalTypeIndex) == BSU_PASSIVE_BALANCE_TYPE)//
			 &&(get_bsu_n_board_type(bsu_index) == kBoardMain)) //主采集板为2个温度
			{
				if (bsu_n_temp_num > 2)
					bsu_n_temp_num = 2;
				start_index = 2; //起始地址为2
				end_index = 2 + bsu_n_temp_num; //结束地址
			}
			else
			{
				start_index = 0; //起始地址为0
				end_index = bsu_n_temp_num; //结束地址
			}
		#else //板载6804
		    if(config_get(kBYUBalTypeIndex)== BSU_PASSIVE_BALANCE_TYPE)//被动从机第一个采集板温度偏移不一样
            {
                if(bsu_index < BMS_SUPPORT_LTC6804_ON_BOARD_CHIP_NUM)
                {
		    if (bsu_n_temp_num > 4)
					bsu_n_temp_num = 4;
				start_index = 0; //起始地址为0
				end_index = bsu_n_temp_num; //结束地址
                }
                else if(get_bsu_n_board_type(bsu_index) == kBoardMain)
                {
                    if(bsu_n_temp_num > 2)
                        bsu_n_temp_num = 2;
                    start_index = 2;
                    end_index = 2+bsu_n_temp_num;
                }
                else
                {
                    start_index = 0;
                    end_index = bsu_n_temp_num;
                }
            } 
            else 
            {
                start_index = 0;
                end_index = bsu_n_temp_num;
            }
		#endif
		
			for (index = start_index; index < end_index; index++) //最高、最低电压扫描,总压计算
			{
				temp = get_bsu_cells_temp(bsu_index, index);
				if (comm_err_flag == TRUE)  //通信异常
				{
					temp = 0;
				}
				else
				{
					if ((temp == TEMP_CABLE_SHORT_VAULE) || (temp == TEMP_CABLE_OPEN_VAULE)) //温度为开路或者短路情况
					{
					    temp = 0;
					}
					else
					{
					    temp =  temp + 10;
						if(temp < LT)
						   LT = temp;
						if(temp > HT)
						   HT = temp;
					}
				}
				if(bmu_temperature_is_heat_temperature(slave_index, temp_index+1) == TRUE)
                {
                    offset = bmu_get_temperature_num(slave_index) + bmu_heat_temp_pos_2_num(slave_index, temp_index+1);
                }
        #if BMU_CHR_OUTLET_TEMP_SUPPORT
                else if(bmu_temperature_is_chr_outlet_temperature(slave_index, temp_index+1) == TRUE)
                {
                    offset = bmu_get_temperature_num(slave_index) + bmu_get_heat_temperature_num(slave_index) + bmu_outlet_temp_pos_2_num(slave_index, temp_index+1);
                }
        #endif
                else
                {
                    offset = bmu_temperature_pos_2_num(slave_index, temp_index+1);
                }
                if(offset != 0xFF)
        		    bmu_set_temperature_item(slave_index, offset, temp);
                
				temp_index++;
			}
		}
		bmu_set_max_temp_by_bsu(slave_index,HT);
		bmu_set_min_temp_by_bsu(slave_index,LT);
		bmu_set_total_temperature_num(slave_index, slave_temp_num);
	}
}
/*****************************************************************************
 *函数名称:bsu_update_bcu_balance_state
 *函数功能:将均衡信息更新到BCU中
 *参    数:无
 *返 回 值:无
 *修订信息:
 ******************************************************************************/
void bsu_update_bcu_balance_state(void)
{
	INT8U bsu_index = 0;
	INT16U value = 0;
	INT8U bsu_bit_index = 0;
	INT8U slave_bit_index = 0;
	INT8U slave_index = 0;
	INT8U slave_num = 0;
	INT8U new_value = 0;
	INT8U bsu_num = 0;
	INT8U bsu_n_cell_num = 0;
	INT8U tmp_8U = 0;
	slave_num = (INT8U) config_get(kSlaveNumIndex); //获取从机数
	bsu_num = get_bsu_num();
	for (slave_index = 0; slave_index < slave_num; slave_index++)
	{
		slave_bit_index = 0;
		for (bsu_index = main_bsu_pos[slave_index]; bsu_index < main_bsu_pos[slave_index + 1]; bsu_index++)
		{
			if (bsu_index >= bsu_num)
			{
				break;
			}
			if(config_get(kBYUBalTypeIndex) == BSU_PASSIVE_BALANCE_TYPE)
			value = get_bsu_passive_balance_word(bsu_index);
			else
			    value = get_bsu_active_balance_word(bsu_index);
			bsu_n_cell_num = get_bsu_n_cells_num(bsu_index);

			for (bsu_bit_index = 0; bsu_bit_index < bsu_n_cell_num; bsu_bit_index++)
			{
				if (GET(value, bsu_bit_index))
				{
					SET(new_value, (slave_bit_index & 0x07));//8位数位为0-7 ，占3位0x07
				}
				slave_bit_index++;
				if ((slave_bit_index & 0x07) == 0)//已经满8位，将new_value清零并将均衡数据更新到bcu中
				{   //
				    tmp_8U = (INT8U)((slave_bit_index - 1) >>3);//除以8 
					bmu_set_balance_state(slave_index, tmp_8U, new_value);
					new_value = 0;
				}
			}
		}
		//未满8位需要补上
		if ((slave_bit_index & 0x07) != 0)
		{
			tmp_8U = (slave_bit_index / 8);
			bmu_set_balance_state(slave_index, tmp_8U, new_value);
			new_value = 0;
		}
	}

}
/*****************************************************************************
 *函数名称:is_bmu_comm_error_use_bsu
 *函数功能:BMU从机是否通讯中断
 *参    数:无
 *返 回 值:无
 *修订信息:
 ******************************************************************************/
BOOLEAN is_bmu_comm_error_use_bsu(INT8U slave_index) 
{
   INT8U  bsu_num = 0;
   INT8U  bsu_index = 0;
   if(slave_index >= BMU_MAX_SLAVE_COUNT) 
   {
        return TRUE;
   }
   bsu_num = get_bsu_num();
   for (bsu_index = main_bsu_pos[slave_index]; bsu_index < main_bsu_pos[slave_index + 1]; bsu_index++)
	{
		if(is_bsu_n_comm_error(bsu_index) == TRUE) 
		{
		 	return TRUE;
		}
	}
	return FALSE;
}
/*****************************************************************************
 *函数名称:bsu_update_bmu_relay_state
 *函数功能:将继电器信息更新到BMU中
 *参    数:无
 *返 回 值:无
 *修订信息:
 ******************************************************************************/
void bsu_update_bmu_relay_state(void)
{
	INT8U slave_index = 0;
    INT8U slave_num = 0;
    INT8U tmp_8u = 0;
    INT8U bsu_pos = 0;
    
	slave_num = (INT8U) config_get(kSlaveNumIndex); //获取从机数

        for (slave_index = 0; slave_index < slave_num; slave_index++)
    	{
    	     bsu_pos = get_main_bsu_pos(slave_index);//获取主采集板的编号
	     
            if (bsu_pos == 0xFF)
        		continue;
        
            if(get_bsu_relay1_status(bsu_pos) == kBsuRelayOff)
            {
                set_ltc6804_gpio2_pin_high(bsu_pos); //见硬件
            }
            else
            {
                set_ltc6804_gpio2_pin_low(bsu_pos); //见硬件
            }
            if(get_bsu_relay2_status(bsu_pos) == kBsuRelayOff)
            {
                set_ltc6804_gpio1_pin_high(bsu_pos); //见硬件
            }
            else
            {
                set_ltc6804_gpio1_pin_low(bsu_pos); //见硬件
            }
    	}
    	    
	
	for (slave_index = 0; slave_index < slave_num; slave_index++)
	{
	    tmp_8u = bmu_get_relay_state(slave_index);
	    if(bmu_get_bsu_relay_status(slave_index,1))
	    {
	       SET(tmp_8u,0);
	    }
	    else
	    {
	       CLR(tmp_8u,0);
	    }
	    if(bmu_get_bsu_relay_status(slave_index,2))
	    {
	      SET(tmp_8u,1);
	    }
	    else
	    {
	      CLR(tmp_8u,1);
	    }
	    bmu_set_relay_state(slave_index,tmp_8u);
	}
}
/*****************************************************************************
 *函数名称:bsu_update_bmu_relay_state
 *函数功能:将继电器信息更新到BMU中
 *参    数:无
 *返 回 值:无
 *修订信息:
 ******************************************************************************/
void bsu_update_bmu_balance_current(void)
{
	INT8U slave_index = 0;
    INT8U slave_num = 0;
    INT8U tmp_8u = 0;
    INT8U bsu_pos = 0;
    
	slave_num = (INT8U) config_get(kSlaveNumIndex); //获取从机数

    for (slave_index = 0; slave_index < slave_num; slave_index++)
	{
	    bsu_pos = get_main_bsu_pos(slave_index);//获取主采集板的编号
     
        if (bsu_pos == 0xFF)
    		continue;
        tmp_8u = get_bsu_active_balance_current(bsu_pos);
        bmu_set_balance_current_item(slave_index,0,tmp_8u);
	}
}
/*****************************************************************************
 *函数名称:bsu_update_bcu_info
 *函数功能:将BSU信息更新到BCU中
 *参    数:无
 *返 回 值:无
 *修订信息:
 ******************************************************************************/
void bsu_update_bcu_info(void* data)
{
    (void)data;
    if(get_bsu_sample_once_finish_flag() == TRUE)
    {
        bsu_update_bcu_balance_state();
    #if  BMS_SUPPORT_BY5248D == 0 //板载无需继电器控制
        bsu_update_bmu_relay_state();  
    #endif
        if(config_get(kBYUBalTypeIndex) == BSU_ACTIVE_BALANCE_TYPE)
        {
            bsu_update_bmu_balance_current();
        }
    }
}

#endif
