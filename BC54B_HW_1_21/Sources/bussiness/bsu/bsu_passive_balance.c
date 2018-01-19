/*******************************************************************************
 **                       ������������Դ�������޹�˾ Copyright (c)
 **                            http://www...
 **�� �� ��:bsu_passive_balance.c
 **��������:2015.12.24
 **�ļ�˵��:��ر����������
 **�޸ļ�¼:
 **��    ��:V1.0
 **��    ע:
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
 *��������:clr_passive_balance_multi_volt_item
 *��������:������������ʱ�����������
 *��    ��:��
 *�� �� ֵ:��
 *�޶���Ϣ:
 ******************************************************************************/
void clr_bsu_passive_balance_multi_volt_item(void)
{
	ltc6804_buffer_set(&bsu_passive_balance_context.volt_item[0][0], 0, PARAM_BSU_NUM_MAX * MULTI_MAX_VOLT_NUM * sizeof(BalanceItem));
}
/*****************************************************************************
 *��������:bsu_statistic_multi_max_volt
 *��������:��ȡÿ���ӻ���ǰn������ص�ѹ
 *��    ��:bsu_index �ӻ���  pos���λ�� volt��ص�ѹ
 *�� �� ֵ:��
 *�޶���Ϣ:
 ******************************************************************************/
void bsu_statistic_multi_max_volt(INT8U bsu_index,INT8U pos, INT16U volt)
{
	INT8U i = 0, move_num = 0, move_i = 0, satrt_i = 0;
    const BsuPassiveBalanceContext *far context = &bsu_passive_balance_context;
    //�����ж�
    if(bsu_index >= get_bsu_num())
        return;
	if (pos >= get_bsu_n_cells_num(bsu_index))
		return;

	for (i = 0; i < (INT8U) MULTI_MAX_VOLT_NUM; i++)
	{
		if (volt > context->volt_item[bsu_index][i].volt)
		{   //��[0]��Ϊ��ߵ�ѹ,�������ĵ�ѹ���ڵ�[0]����ѹ,��ô���е�ѹ����
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
 *��������:set_bsu_passive_balance_word
 *��������:���ñ�������λ�����ھ������
 *��    ��:context��������  bsu_index �ӻ���  balance_word������
 *�� �� ֵ:��            
 *�޶���Ϣ:
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
 *��������:get_passive_balance_word
 *��������:��ȡ����������
 *��    ��:bsu_index �ӻ���
 *�� �� ֵ:INT16U ����λ           
 *�޶���Ϣ:
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
 *��������:clr_all_bsu_passive_balance_word
 *��������:������еľ���λ
 *��    ��:��
 *�� �� ֵ:��    
 *�޶���Ϣ:
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
 *��������:bsu_passive_balance_max_volt_diff_analyze
 *��������:������������ѹѹ����� �����е��ƽ����ѹ�Ƚ�
 *��    ��:INT16U timeʱ����
 *�� �� ֵ:��
 *�޶���Ϣ:
 ******************************************************************************/
static void bsu_passive_balance_max_volt_diff_analyze(INT16U time)
{
    BsuPassiveBalanceContext *far context = &bsu_passive_balance_context;
    
	switch (context->max_volt_diff_status)
	{
		case kPassiveBalanceVoltDiffNormal:

			if (bcu_get_high_voltage() >= (bcu_get_average_voltage() + config_get(kBYUBalDiffVoltMaxIndex)))//�������е��ƽ����ѹ+���⿪ʼѹ��
			{
				context->status_dly += time;
				if (context->status_dly >= BALANCE_DLAY_2S) //��ʱ2S
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
			if (bcu_get_high_voltage() < (bcu_get_average_voltage() + config_get(kBYUBalDiffVoltMinIndex))) //С�����е��ƽ����ѹ+����ر�ѹ��
			{
				context->status_dly += time;
				if (context->status_dly >= BALANCE_DLAY_2S) //��ʱ2S
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
 *��������:is_passive_balance_condition_ok
 *��������:�������������Ƿ����
 *��    ��:��
 *�� �� ֵ:TRUE�������� FALSE����������
 *�޶���Ϣ:
 ******************************************************************************/
static BOOLEAN is_passive_balance_condition_ok(void)
{
	const BsuPassiveBalanceContext *far context = &bsu_passive_balance_context;
	
	if ((bcu_get_dchg_ht_state() == kAlarmNone)  //���û�й��±����ͱ���
		&&(bcu_get_chg_ht_state() == kAlarmNone)  //�ŵ�û�й��±����ͱ���
		&&(config_get(kBYUBalNumIndex) > 0)  //����·������0
	    &&(context->max_volt_diff_status == kPassiveBalanceVoltDiffBig)  //���� ����ص�ѹ��ƽ����ѹѹ��
	    &&(is_bsu_comm_error()==FALSE) //û��ͨ���ж�
	    &&(is_volt_sample_cable_open() == FALSE) //��ѹ�ɼ���������
	    &&(bcu_get_temp_exception() == 0)
	    &&(config_get(kBmuBalanceEnableIndex)) //����ʹ��
	    &&(bcu_get_high_voltage() >= config_get(kBYUBalStartVoltIndex)) //����������ѹ����
	    &&(bcu_get_low_voltage() >= config_get(kChgLVTrdAlarmIndex))
	    &&(bcu_is_charging() == TRUE)&&(charger_is_connected())//�ڳ�������
	    ) 
	{
		return TRUE;
	}
	return FALSE;
}
/*****************************************************************************
 *��������:get_bsu_max_cell_volt
 *��������:
 *��    ��:��
 *�� �� ֵ:��
 *�޶���Ϣ:
 ******************************************************************************/
INT16U get_bsu_max_cell_volt(INT8U bsu_index)
{
	INT8U bsu_num = 0;
    BsuPassiveBalanceContext *far context = &bsu_passive_balance_context;
			
	bsu_num = get_bsu_num(); //�ӻ���Ŀ
	if(bsu_index >= bsu_num)
	    return 0;
	
	return context->volt_item[bsu_index][0].volt; 
}
/*****************************************************************************
 *��������:get_bsu_max_cell_volt_pos
 *��������:
 *��    ��:��
 *�� �� ֵ:��
 *�޶���Ϣ:
 ******************************************************************************/
INT8U get_bsu_max_cell_volt_pos(INT8U bsu_index)
{
	INT8U bsu_num = 0;
    BsuPassiveBalanceContext *far context = &bsu_passive_balance_context;
			
	bsu_num = get_bsu_num(); //�ӻ���Ŀ
	if(bsu_index >= bsu_num)
	    return 0;
	
	return context->volt_item[bsu_index][0].pos; 
}
/*****************************************************************************
 *��������:select_passive_balance_cell
 *��������:ѡ����Ҫ��������ĵ��
 *��    ��:��
 *�� �� ֵ:��
 *�޶���Ϣ:
 ******************************************************************************/
static void select_passive_balance_cell(void)
{
	INT16U cell_balance_word = 0;     //����λ
	INT8U i = 0;
	INT8U bsu_index = 0;       //�ӻ���ż���
	INT8U bsu_num = 0;
	INT8U max_balance_num = 0; //������·��
    BsuPassiveBalanceContext *far context = &bsu_passive_balance_context;
			
	bsu_num = get_bsu_num(); //�ӻ���Ŀ
	max_balance_num = (INT8U) config_get(kBYUBalNumIndex);//����·��
	if (max_balance_num > MULTI_MAX_VOLT_NUM)
		max_balance_num = MULTI_MAX_VOLT_NUM;
	
	for (bsu_index = 0; bsu_index < bsu_num; bsu_index++)
	{
        cell_balance_word = 0;
		for (i = 0; i < max_balance_num; i++)
		{
			if (context->volt_item[bsu_index][i].volt > (bcu_get_average_voltage() + config_get(kBYUBalDiffVoltMinIndex)))
			{
				//��ؾ���λ
				SET(cell_balance_word,context->volt_item[bsu_index][i].pos);
			}
			else
			{
				break;
			}
		}
		set_bsu_passive_balance_word(context,bsu_index, cell_balance_word);
	}
	for (; bsu_index < PARAM_BSU_NUM_MAX; bsu_index++) //û��ʹ�õĴӻ�����λ����
	{
		set_bsu_passive_balance_word(context,bsu_index, 0);
	} 
}
/*****************************************************************************
 *��������:is_bsu_passive_balancing
 *��������:�ж��Ƿ��ڱ���������
 *��    ��:��
 *�� �� ֵ:TRUE ��  FALSE�� 
 *�޶���Ϣ:
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
 *��������:bsu_passive_balance_ctrl
 *��������:����������� 
 *��    ��:��
 *�� �� ֵ:��
 *�޶���Ϣ:
 ******************************************************************************/
void bsu_passive_balance_ctrl(void)
{
	static INT32U last_tick = 0;//��һ��ʱ��
	INT32U tick_diff = 0;       //ʱ���
	static INT32U balance_time = 0; //����ʱ��

	tick_diff = get_elapsed_tick_count(last_tick);
	last_tick = get_tick_count();

    bsu_passive_balance_max_volt_diff_analyze((INT16U) tick_diff); //������������ѹѹ�����

	if (!is_passive_balance_condition_ok()) //��������������,����ֹͣ����
	{
		balance_time = 0;  //����ʱ������
		clr_all_bsu_passive_balance_word();//���������
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
