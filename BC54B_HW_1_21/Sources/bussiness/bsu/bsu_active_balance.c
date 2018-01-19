/*******************************************************************************
 **                       ������������Դ�������޹�˾ Copyright (c)
 **                            http://www...
 **�� �� ��:bsu_active_balance.c
 **��������:2015.12.24
 **�ļ�˵��:��������������
 **�޸ļ�¼:
 **��    ��:V1.0
 **��    ע:
 *******************************************************************************/

#include "bsu_active_balance.h"
#include "ltc_util.h"
#include "ltc68041_impl.h" 
#include "bsu_sample.h"
#include "bms_bsu.h"
#if BMS_SUPPORT_HARDWARE_LTC6804 == 1

static SystemPowerVoltType system_power_volt_type  = kSystemPowerVolt_Unknow; //ϵͳ�����ѹ����(12V or 24V)
static SystemPowerVoltStatus system_power_volt_status = kPowerVolt_Error; //ϵͳ��ѹ�����������
#pragma DATA_SEG __RPAGE_SEG PAGED_RAM_BSU_BALANCE 
static BsuActiveBalanceContext bsu_active_balance_context;
#pragma DATA_SEG DEFAULT

/*****************************************************************************
 *��������:system_power_volt_type_analyse
 *��������:ϵͳ�����ѹ���ͷ���
 *��    ��:��
 *�� �� ֵ:��
 *�޶���Ϣ:
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
 *��������:set_system_power_volt_status
 *��������:���ù����ѹ״̬
 *��    ��:��
 *�� �� ֵ:��
 *�޶���Ϣ:
 ******************************************************************************/
void set_system_power_volt_status(INT16U volt,INT16U min_volt, INT16U mid_volt, INT16U max_volt)
{
   static INT8U cnt = 0;
    if(volt <= min_volt)
      {
          if(++cnt > 5)
          {
            cnt = 0;
            system_power_volt_status = kPowerVolt_OverDisChg; //Ǧ���ع���
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
 *��������:system_power_volt_status_analyse
 *��������:����ϵͳ��ѹȷ����������
 *��    ��:��
 *�� �� ֵ:��
 *�޶���Ϣ:
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
 *��������:clr_balance_min_volt_item
 *��������:
 *��    ��:��
 *�� �� ֵ:��
 *�޶���Ϣ:
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
 *��������:bsu_statistic_min_volt
 *��������:��ȡÿ��BSU��С��ѹ
 *��    ��:bsu_index �ӻ���  pos���λ�� volt��ص�ѹ
 *�� �� ֵ:��
 *�޶���Ϣ:
 ******************************************************************************/
void bsu_statistic_min_volt(INT8U bsu_index,INT8U pos,INT16U volt)
{
    const BsuActiveBalanceContext *far context = &bsu_active_balance_context;
    //�����ж�
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
 *��������:set_bsu_active_balance_word
 *��������:���ñ�������λ�����ھ������
 *��    ��:context��������  bsu_index �ӻ���  balance_word������
 *�� �� ֵ:��            
 *�޶���Ϣ:
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
 *��������:get_active_balance_word
 *��������:��ȡ����������
 *��    ��:bsu_index �ӻ���
 *�� �� ֵ:INT16U ����λ           
 *�޶���Ϣ:
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
 *��������:clr_all_bsu_active_balance_word
 *��������:������еľ���λ
 *��    ��:��
 *�� �� ֵ:��    
 *�޶���Ϣ:
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
 *��������:bsu_active_balance_volt_diff_analyze
 *��������:������������ѹѹ����� �����е��ƽ����ѹ�Ƚ�
 *��    ��:INT16U timeʱ����
 *�� �� ֵ:��
 *�޶���Ϣ:
 ******************************************************************************/
static void bsu_active_balance_volt_diff_analyze(INT16U time)
{
    BsuActiveBalanceContext *far context = &bsu_active_balance_context;
    
	switch (context->volt_diff_status)
	{
		case kActiveBalanceVoltDiffNormal:

			if ((bcu_get_high_voltage() >= (bcu_get_average_voltage() + config_get(kBYUBalDiffVoltMaxIndex)))//�������е��ƽ����ѹ+���⿪ʼѹ��
			    ||((bcu_get_low_voltage()+ config_get(kBYUBalDiffVoltMaxIndex)) <= bcu_get_average_voltage() ))
			{
				context->status_dly += time;
				if (context->status_dly >= BALANCE_DLAY_2S) //��ʱ2S
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
			if ((bcu_get_high_voltage() < (bcu_get_average_voltage() + config_get(kBYUBalDiffVoltMinIndex))) //С�����е��ƽ����ѹ+����ر�ѹ��
			   &&((bcu_get_low_voltage()+config_get(kBYUBalDiffVoltMinIndex)) > bcu_get_average_voltage()))
			{
				context->status_dly += time;
				if (context->status_dly >= BALANCE_DLAY_30S) //��ʱ30S
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
 *��������:is_active_balance_condition_ok
 *��������:�������������Ƿ����
 *��    ��:��
 *�� �� ֵ:TRUE�������� FALSE����������
 *�޶���Ϣ:
 ******************************************************************************/
static BOOLEAN is_active_balance_condition_ok(void)
{
	const BsuActiveBalanceContext *far context = &bsu_active_balance_context;

	if ((bcu_get_dchg_ht_state() == kAlarmNone)  //���û�й��±����ͱ���
		&&(bcu_get_chg_ht_state() == kAlarmNone)  //�ŵ�û�й��±����ͱ���
		&&(config_get(kBYUBalNumIndex) > 0)  //����·������0
	    &&(context->volt_diff_status == kPassiveBalanceVoltDiffBig)  //���� ����ص�ѹ��ƽ����ѹѹ��
	    &&(is_bsu_comm_error()==FALSE) //û��ͨ���ж�
	    &&(is_volt_sample_cable_open() == FALSE) //��ѹ�ɼ���������
	    &&(config_get(kBmuBalanceEnableIndex)) //����ʹ��
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
 *��������:select_cell_only_chg_balance
 *��������:����﮵�س�����
 *��    ��:��
 *�� �� ֵ:��
 *�޶���Ϣ:
 ******************************************************************************/
static void select_cell_only_chg_balance(BsuActiveBalanceContext *far context)
{
    INT16U cell_balance_word = 0;     //����λ
    INT8U bsu_index = 0;       //�ӻ���ż���
    INT8U bsu_num = 0;
    bsu_num = get_bsu_num(); //�ӻ���Ŀ
    for (bsu_index = 0; bsu_index < bsu_num; bsu_index++)
    {
        cell_balance_word = 0;
        if(((context->min_volt_item[bsu_index].volt + config_get(kBYUBalDiffVoltMinIndex)) < bcu_get_average_voltage())
    #if BMS_DIAGNOSIS_HV_INDEPENDENT_WITH_CUR_EN      
        &&(context->min_volt_item[bsu_index].volt < config_get(kChgHVTrdAlarmIndex)))
    #else
        &&(context->min_volt_item[bsu_index].volt < config_get(kDChgHVTrdAlarmIndex)))
    #endif
        {   //������
            SET(cell_balance_word,context->min_volt_item[bsu_index].pos); 
            CLR(cell_balance_word,15); 
        }
        set_bsu_active_balance_word(context,bsu_index, cell_balance_word);
    }
    for (; bsu_index < PARAM_BSU_NUM_MAX; bsu_index++) //û��ʹ�õĴӻ�����λ����
    {
        set_bsu_active_balance_word(context,bsu_index, 0);
    } 
}
 /*****************************************************************************
 *��������:select_cell_only_dischg_balance
 *��������:����﮵�طŵ����
 *��    ��:��
 *�� �� ֵ:��
 *�޶���Ϣ:
 ******************************************************************************/
static void select_cell_only_dischg_balance(BsuActiveBalanceContext *far context)
{
    INT16U cell_balance_word = 0;     //����λ
    INT8U bsu_index = 0;       //�ӻ���ż���
    INT8U bsu_num = 0;
    bsu_num = get_bsu_num(); //�ӻ���Ŀ
    for (bsu_index = 0; bsu_index < bsu_num; bsu_index++)
    {
        cell_balance_word = 0;
        if ((get_bsu_max_cell_volt(bsu_index) > (bcu_get_average_voltage() + config_get(kBYUBalDiffVoltMinIndex)))
        &&(get_bsu_max_cell_volt(bsu_index) > config_get(kDChgLVTrdAlarmIndex)))
		{   //�ŵ����
			SET(cell_balance_word,get_bsu_max_cell_volt_pos(bsu_index));
			SET(cell_balance_word,15); 
		}
        set_bsu_active_balance_word(context,bsu_index, cell_balance_word);
    }
    for (; bsu_index < PARAM_BSU_NUM_MAX; bsu_index++) //û��ʹ�õĴӻ�����λ����
    {
        set_bsu_active_balance_word(context,bsu_index, 0);
    } 
}
 /*****************************************************************************
 *��������:select_cell_auto_balance
 *��������:���ɾ���
 *��    ��:��
 *�� �� ֵ:��
 *�޶���Ϣ:
 ******************************************************************************/
static void select_cell_auto_balance(BsuActiveBalanceContext *far context)
{
	INT16U cell_balance_word = 0;     //����λ
	INT8U bsu_index = 0;       //�ӻ���ż���
	INT8U bsu_num = 0;
	INT16U max_volt_diff=0,min_volt_diff = 0;
	bsu_num = get_bsu_num(); //�ӻ���Ŀ
	
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
    for (; bsu_index < PARAM_BSU_NUM_MAX; bsu_index++) //û��ʹ�õĴӻ�����λ����
    {
        set_bsu_active_balance_word(context,bsu_index, 0);
    } 
}
/*****************************************************************************
 *��������:select_cell_balance_with_power_charging
 *��������:���ɾ���
 *��    ��:��
 *�� �� ֵ:��
 *�޶���Ϣ:
 ******************************************************************************/
static void get_cell_need_balance_num_by_auto(BsuActiveBalanceContext *far context,INT8U *chg_num,INT8U *dischg_num)
{
    INT8U bsu_index = 0;       //�ӻ���ż���
    INT8U bsu_num = 0;
    INT16U max_volt_diff=0,min_volt_diff = 0;
    bsu_num = get_bsu_num(); //�ӻ���Ŀ
    
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
 *��������:volt_diff_sort
 *��������:ѹ������
 *��    ��:��
 *�� �� ֵ:��
 *�޶���Ϣ:
 ******************************************************************************/
static void volt_diff_sort(BsuActiveBalanceContext *far context,INT8U bsu_index,INT16U volt_diff,INT8U pos,INT8U is_max_volt_diff)
{
   INT8U i = 0, move_num = 0, move_i = 0, satrt_i = 0; 
   for (i = 0; i < VOLT_DIFF_SORT_NUM; i++)
    {
    	if (volt_diff > context->volt_diff_item[i].volt_diff)
    	{   //��[0]��Ϊ��ߵ�ѹ,�������ĵ�ѹ���ڵ�[0]����ѹ,��ô���е�ѹ����
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
 *��������:select_cell_balance_with_power_charging
 *��������:Ǧ�ḡ����ѡ���ؾ���
 *��    ��:��
 *�� �� ֵ:��
 *�޶���Ϣ:
 ******************************************************************************/
static void select_cell_balance_with_power_charging(BsuActiveBalanceContext *far context,INT8U chg_num,INT8U dischg_num)
{
    INT8U bsu_index = 0,i = 0;       //�ӻ���ż���
    INT8U bsu_num = 0;
    INT16U volt_diff = 0;
    INT8U num = 0;
    INT8U cnt = 0;
    INT16U cell_balance_word = 0; 
    
    num = chg_num + dischg_num;
    if(num == 0)
        return;
    
    bsu_num = get_bsu_num(); //�ӻ���Ŀ
    //�����������
    for(i = 0;i < VOLT_DIFF_SORT_NUM;i++)
    {
        context->volt_diff_item[i].volt_diff = 0;
        context->volt_diff_item[i].pos.pos_byte = 0;
    }
    //����Ҫ�����ѹ���������
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
    //��һ��ѹ�����������
    cell_balance_word = 0; 
    if(context->volt_diff_item[0].volt_diff > config_get(kBYUBalDiffVoltMinIndex))
    {
        SET(cell_balance_word,context->volt_diff_item[0].pos.pos_bits.pos); 
        if(context->volt_diff_item[0].pos.pos_bits.is_max_volt_diff)
        { //�ŵ��������
            SET(cell_balance_word,15);
            cnt++;
        }
        else
        {
            CLR(cell_balance_word,15);
        }
        set_bsu_active_balance_word(context,context->volt_diff_item[0].bsu_index, cell_balance_word);
	}
    //ѡ�� (num - 1)/2���ŵ����
    num = (INT8U)((num - 1)>>1);
    for(i = 0;i < VOLT_DIFF_SORT_NUM;i++)
    {
       if((cnt+1) >= num)
       {
            break;
       }
       cell_balance_word = 0; 
       if(context->volt_diff_item[i].pos.pos_bits.is_max_volt_diff)//�ŵ��������
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
    //ʣ���ȫ��Ϊ������
    for (bsu_index = 0; bsu_index < bsu_num; bsu_index++)
    {
        cell_balance_word = 0;
        if(get_bsu_active_balance_word(bsu_index) != 0)//�Ѿ��о���λ
        {
            continue;
        }
        if(((context->min_volt_item[bsu_index].volt + config_get(kBYUBalDiffVoltMinIndex)) < bcu_get_average_voltage())
     #if BMS_DIAGNOSIS_HV_INDEPENDENT_WITH_CUR_EN  
        &&(context->min_volt_item[bsu_index].volt < config_get(kChgHVTrdAlarmIndex)))
     #else
        &&(context->min_volt_item[bsu_index].volt < config_get(kDChgHVTrdAlarmIndex)))
     #endif
        {   //������
            SET(cell_balance_word,context->min_volt_item[bsu_index].pos); 
            CLR(cell_balance_word,15); 
		}
		set_bsu_active_balance_word(context,bsu_index, cell_balance_word);
	}
}
/*****************************************************************************
 *��������:select_active_balance_cell
 *��������:ѡ����Ҫ��������ĵ��
 *��    ��:��
 *�� �� ֵ:��
 *�޶���Ϣ:
 ******************************************************************************/
static void select_active_balance_cell(void)
{
    BsuActiveBalanceContext *far context = &bsu_active_balance_context;
    INT8U chg_num = 0,dischg_num = 0; 
	switch(system_power_volt_status)
	{
    	case kPowerVolt_Error://����
    	     clr_all_bsu_active_balance_word();
    	break;
        case kPowerVolt_Normal://����
             select_cell_auto_balance(context);
        break;
        case kPowerVolt_OverChg://����
             select_cell_only_chg_balance(context);//﮵�����⣬��Ǧ��ŵ� 
        break;
        case kPowerVolt_OverDisChg://����
             select_cell_only_dischg_balance(context);//﮵�ŵ���⣬��Ǧ���� 
        break;
        case kPowerVolt_Charging://����
             get_cell_need_balance_num_by_auto(context,&chg_num,&dischg_num);
             if((chg_num+dischg_num) == 0)//û�����������ľ���
                return;
             if((chg_num >= dischg_num)||(dischg_num == 1)||((chg_num+dischg_num)==1))
             { //���ɾ���
                select_cell_auto_balance(context);
             }
             else
             { //�ŵ����·�����ֻ���� (chg_num + dischg_num -1)/2��
                select_cell_balance_with_power_charging(context,chg_num,dischg_num);
             }
             
        break;
    	default:
    	break;
	} 
}
/*****************************************************************************
 *��������:is_bsu_active_balancing
 *��������:�ж��Ƿ��ڱ���������
 *��    ��:��
 *�� �� ֵ:TRUE ��  FALSE�� 
 *�޶���Ϣ:
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
 *��������:bsu_active_balance_ctrl
 *��������:����������� 
 *��    ��:��
 *�� �� ֵ:��
 *�޶���Ϣ:
 ******************************************************************************/
void send_active_balance_cmd_to_ltc6804(INT8U bsu_num)
{
    BsuActiveBalanceContext *far context = &bsu_active_balance_context;
    (void)write_balance_info_to_dsp(bsu_num,DSP_IIC_ADDR,context->balance_control_word,ACTIVE_BALANCE_CURRENT);
}
/*****************************************************************************
 *��������:read_active_balance_current
 *��������:��ȡ�������
 *��    ��:��
 *�� �� ֵ:��
 *�޶���Ϣ:
 ******************************************************************************/
void read_active_balance_current(INT8U bsu_num)
{
    BsuActiveBalanceContext *far context = &bsu_active_balance_context;
    (void)read_balance_current_from_dsp(bsu_num,DSP_IIC_ADDR,context->balance_current);
}
/*****************************************************************************
 *��������:get_bsu_active_balance_current
 *��������:��ȡ�������
 *��    ��:��
 *�� �� ֵ:��
 *�޶���Ϣ:
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
 *��������:get_bsu_active_balance_current
 *��������:����������
 *��    ��:��
 *�� �� ֵ:��
 *�޶���Ϣ:
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
 *��������:is_active_balance_error
 *��������:�������
 *��    ��:��
 *�� �� ֵ:��
 *�޶���Ϣ:
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
 *��������:balance_error_analyze
 *��������:�����������
 *��    ��:��
 *�� �� ֵ:��
 *�޶���Ϣ:
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
 *��������:bsu_active_balance_ctrl
 *��������:����������� 
 *��    ��:��
 *�� �� ֵ:��
 *�޶���Ϣ:
 ******************************************************************************/
void bsu_active_balance_ctrl(void)
{
	static INT32U last_tick = 0;//��һ��ʱ��
	INT32U tick_diff = 0;       //ʱ���
	static INT32U balance_time = 0; //����ʱ��

	tick_diff = get_elapsed_tick_count(last_tick);
	last_tick = get_tick_count();
    system_power_volt_status_analyse();
    bsu_active_balance_volt_diff_analyze((INT16U) tick_diff); //������������ѹѹ�����

	if (!is_active_balance_condition_ok()) //��������������,����ֹͣ����
	{
		balance_time = 0;  //����ʱ������
		clr_all_bsu_active_balance_word();//���������
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