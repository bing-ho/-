/*******************************************************************************
 **                       ������������Դ�������޹�˾ Copyright (c)
 **                            http://www...
 **�� �� ��:bsu_sample.c
 **��������:2015.10.15
 **�ļ�˵��:��ѹ�¶Ȳɼ����������
 **�޸ļ�¼:
 **��    ��:V1.0
 **��    ע:
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


static OS_STK BsuSampleTaskStack[AD_SAMPLE_TASK_STK_SIZE] = {0}; //��ѹ�¶Ȳɼ������ջ

#pragma DATA_SEG __RPAGE_SEG PAGED_RAM_BYU_SAMPLE
//�ɼ���Ϣ
static BsuSampleContext bsu_sample_context;

#pragma DATA_SEG DEFAULT 

#pragma DATA_SEG __RPAGE_SEG PAGED_RAM_BSU 
//�����ɼ��ĵ�ص�ѹ
static INT16U PUVoltTmpBuffer[PARAM_BSU_NUM_MAX][LTC6804_MAX_CELL_NUM];
//�����ɼ��ĵ�ص�ѹ
static INT16U PDVoltTmpBuffer[PARAM_BSU_NUM_MAX][LTC6804_MAX_CELL_NUM];
static INT16U AuxVoltBuffer[AUX_BUFF_LEN][PARAM_BSU_NUM_MAX]; //�¶ȵ�ѹ��ʱ����
#pragma DATA_SEG DEFAULT
//��������
static void bsu_context_init(BsuSampleContext* __far context);
void BsuVoltTempSampleTask(void *pdata);
/*****************************************************************************
 *��������:BSUVoltTempSampleTask_Init
 *��������:��������
 *��    ��:
 *�� �� ֵ:��
 *�޶���Ϣ:
 ******************************************************************************/
void BSUVoltTempSampleTask_Init(void)
{
	volatile INT8U err = 0;
	MODRR_MODRR5 = 1;     //ʹ��PH0~PH3 
	(void) spi_init(&ltc6804_high_speed_spi_bus, 1); //spi��ʼ��
	bsu_context_init(&bsu_sample_context); //���ݳ�ʼ��
	watch_dog_register(WDT_AD_ID, WDT_AD_TIME); //���Ź�ע��
	err = OSTaskCreate(BsuVoltTempSampleTask, //������
			(void *) 0, //����
			(OS_STK *) &BsuSampleTaskStack[AD_SAMPLE_TASK_STK_SIZE - 1], //��ջ
			AD_SAMPLE_TASK_PRIO); //���ȼ�
	if (err != OS_ERR_NONE)
	{
		for (;;)
		{
		}
	}
}
/*****************************************************************************
 *��������:get_bsu_n_cells_num
 *��������:��ȡ����,��N���ɼ���ĵ����
 *��    ��:bsu_index  �ɼ����(��0��ʼ)
 *�� �� ֵ:��Ӧ�ɼ���ĵ����
 *�޶���Ϣ:
 ******************************************************************************/
INT8U get_bsu_n_cells_num(INT8U bsu_index)
{
	INT8U num = 0;
	INT16U tmp_16u = 0;
	//�������
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
 *��������:get_bsu_n_temp_num
 *��������:��ȡ����,��N���ɼ�����¶���
 *��    ��:bsu_index  �ɼ����(��0��ʼ)
 *�� �� ֵ:��Ӧ�ɼ�����¶���
 *�޶���Ϣ:
 ******************************************************************************/
INT8U get_bsu_n_temp_num(INT8U bsu_index)
{
	INT8U num = 0;
	INT16U tmp_16u = 0;
	//�������
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
 *��������:get_bsu_num
 *��������:��ȡbsu��Ŀ����ʹ�õ�BSU�����Ŀ��������Ϊ0
 *��    ��:��
 *�� �� ֵ:�ɼ�����
 *�޶���Ϣ:
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
 *��������:bsu_context_init
 *��������:�ɼ���BSU���ݳ�ʼ��
 *��    ��:context�ɼ���ɼ���������
 *�� �� ֵ:��
 *�޶���Ϣ:
 ******************************************************************************/
static void bsu_context_init(BsuSampleContext* __far context)
{
	INT8U index = 0;
	INT8U bsu_index = 0; //�ɼ��������
	//�������
	if(context == NULL)
		return;
	//��ѹ����
	ltc6804_buffer_set(&context->volt_item,0,sizeof(BsuVoltSampleItem));
	//���������ֵ
	ltc6804_buffer_set(context->heart_beat,BSU_MAX_COMM_ERROR_CNT,PARAM_BSU_NUM_MAX);
	//��������
	ltc6804_buffer_set(&context->board_type,kBoardUnknow,PARAM_BSU_NUM_MAX);
	context->board_gpio_init_flag = 0;
	context->sample_once_finish_flag = FALSE;
	for (bsu_index = 0; bsu_index < PARAM_BSU_NUM_MAX; bsu_index++)
	{
		context->error_byte[bsu_index].value = 0;
		//�¶�����
		for (index = 0; index < LTC6804_MAX_TEMP_NUM; index++)
		{
			context->temp_item.temp[bsu_index][index] = TEMP_CABLE_OPEN_VAULE;	//�¶���0xFF
			context->temp_item.temp_buffer[bsu_index][index] = 0;
			context->temp_item.filter[bsu_index][index] = 0;
		}
	}
}
/*****************************************************************************
 *��������:set_sample_once_finish_flag
 *��������:���õ�һ�βɼ���ɱ�־
 *��    ��:context�ɼ���ɼ���������
 *�� �� ֵ:��
 *�޶���Ϣ:
 ******************************************************************************/
static void set_sample_once_finish_flag(BsuSampleContext* __far context)
{
	if(context == NULL)
			return;
	context->sample_once_finish_flag = TRUE;
}
/*****************************************************************************
 *��������:get_bsu_sample_once_finish_flag
 *��������:��ȡ��һ�βɼ���ɱ�־
 *��    ��:��
 *�� �� ֵ:��һ�βɼ���ɱ�־ 0δ��ɣ�1�ɼ����
 *�޶���Ϣ:
 ******************************************************************************/
BOOLEAN get_bsu_sample_once_finish_flag(void)
{
	BsuSampleContext* __far context = &bsu_sample_context;

	return context->sample_once_finish_flag;
}
/*****************************************************************************
 *��������:get_bsu_cells_volt
 *��������:��ȡ��ص�ѹ
 *��    ��:bsu_index�ɼ����,pos����ڲɼ����е�λ��
 *�� �� ֵ:��ص�ѹ1mv/bit
 *�޶���Ϣ:
 ******************************************************************************/
INT16U get_bsu_cells_volt(INT8U bsu_index, INT8U pos)
{
	OS_CPU_SR cpu_sr = 0;
	INT16U volt = 0;
	const BsuVoltSampleItem *__far item = &bsu_sample_context.volt_item;
	//�������
	if ((bsu_index >= PARAM_BSU_NUM_MAX) || (pos >= LTC6804_MAX_CELL_NUM))
		return 0;

	OS_ENTER_CRITICAL();
	volt = item->volt[bsu_index][pos];
	OS_EXIT_CRITICAL();

	return volt;
}
/*****************************************************************************
 *��������:get_bsu_cells_temp
 *��������:��ȡ�ɼ������¶�
 *��    ��:INT8U bsu_index�ɼ����,INT8U pos�¸б��
 *�� �� ֵ:����¶�  
 *�޶���Ϣ:
 ******************************************************************************/
INT8U get_bsu_cells_temp(INT8U bsu_index, INT8U pos)
{
	OS_CPU_SR cpu_sr = 0;
	INT8U temp = 0;
	const BsuTempSampleItem *__far item = &bsu_sample_context.temp_item;
	//�������
	if ((bsu_index >= PARAM_BSU_NUM_MAX) || (pos >= LTC6804_MAX_TEMP_NUM))
		return 0;

	OS_ENTER_CRITICAL();
	temp = item->temp[bsu_index][pos];
	OS_EXIT_CRITICAL();

	return temp;
}

/*****************************************************************************
 *��������:clr_all_bsu_comm_error_byte
 *��������:�������ͨ���쳣λ
 *��    ��:��
 *�� �� ֵ:��
 *�޶���Ϣ:
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
 *��������:bsu_comm_heart_beat_run
 *��������:�ɼ���ͨ���ж�����
 *��    ��:context�ɼ����ѹ�¶���Ϣ
 *�� �� ֵ:��
 *�޶���Ϣ:
 ******************************************************************************/
static void bsu_comm_heart_beat_run(BsuSampleContext *__far context)
{
	INT8U bsu_index = 0;
	INT8U bsu_num = 0;
	OS_CPU_SR cpu_sr = 0;

	if(context == NULL)
	return;

	bsu_num = get_bsu_num(); //�ɼ�����
	for (bsu_index = 0; bsu_index < bsu_num; bsu_index++)
	{
		OS_ENTER_CRITICAL();
		if (context->error_byte[bsu_index].Bits.read_cv) //����ѹͨ���쳣
		{
			context->heart_beat[bsu_index]++;  //��������
			if (context->heart_beat[bsu_index] >= BSU_MAX_COMM_ERROR_CNT)//�ﵽ���ֵ
			{
				context->heart_beat[bsu_index] = BSU_MAX_COMM_ERROR_CNT; //�ɼ���ͨ���ж�
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
	//δʹ�õĲɼ���ȫ�����Ϊͨ���ж�
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
 *��������:is_bsu_n_comm_error
 *��������:�жϵ�N���ɼ����Ƿ�ͨ��ʧ��
 *��    ��:bsu_index�ɼ�����
 *�� �� ֵ:FALSEͨ������ TRUEͨ��ʧ��
 *�޶���Ϣ:
 ******************************************************************************/
BOOLEAN is_bsu_n_comm_error(INT8U bsu_index)
{
	OS_CPU_SR cpu_sr = 0;
	const INT8U *__far heartbeat = bsu_sample_context.heart_beat;

	if (bsu_index >= get_bsu_num())
		return FALSE;

	OS_ENTER_CRITICAL();
	if (heartbeat[bsu_index] >= BSU_MAX_COMM_ERROR_CNT)
	{   //ͨ���ж�
		OS_EXIT_CRITICAL();
		return TRUE;
	}
	OS_EXIT_CRITICAL();

	return FALSE; //����
}

/*****************************************************************************
 *��������:is_all_bsu_comm_error
 *��������:�ж����еĲɼ����Ƿ�ͨ��ʧ��
 *��    ��:��
 *�� �� ֵ:FALSEͨ������ TRUEͨ��ʧ��
 *�޶���Ϣ:
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
 *��������:is_bsu_comm_error
 *��������:�ж��Ƿ��вɼ���ͨ��ʧ��
 *��    ��:��
 *�� �� ֵ:FALSEͨ������ TRUEͨ��ʧ��
 *�޶���Ϣ:
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
 *��������:is_cell_volt_sample_cable_open
 *��������:��ѹ������߿�·
 *��    ��:INT8U bsu_index�ɼ�����, INT8U cell_index���λ��
 *�� �� ֵ:FALSE������,TRUE����
 *�޶���Ϣ:
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
 *��������:get_bsu_n_volt_sample_cable_open_flag
 *��������:��ȡ�ɼ������������־,bit0����1�ŵ�ص��������������
 *��    ��:INT8U bsu_index�ɼ�����
 *�� �� ֵ:���������־
 *�޶���Ϣ:
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
 *��������:is_volt_sample_cable_open
 *��������:�ɼ�����������
 *��    ��:��
 *�� �� ֵ:TRUE������,FALSE������
 *�޶���Ϣ:
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
 *��������:get_bsu_n_board_type
 *��������:��ȡ��������
 *��    ��:INT8U bsu_index �ɼ�����
 *�� �� ֵ:�������� ���ɼ��� �Ӳɼ��� δ֪����
 *�޶���Ϣ:
 ******************************************************************************/
BsuBoardType get_bsu_n_board_type(INT8U bsu_index)
{
    const BsuBoardType *__far board_type = bsu_sample_context.board_type;

	if (bsu_index >= get_bsu_num())
		return kBoardUnknow;

	return board_type[bsu_index];
}
/*****************************************************************************
 *��������:get_temp_filter_cnt
 *��������:��ȡ�¶��˲�����
 *��    ��:bsu_index�ɼ�����, index�¶Ⱥ�
 *�� �� ֵ:��
 *�޶���Ϣ:
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
 *��������:get_bsu_avg_volt
 *��������:��ȡ�ɼ���ƽ����ѹ
 *��    ��:volt��ѹ����,num�����
 *�� �� ֵ:ƽ��ֵ
 *�޶���Ϣ:
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
 *��������:is_cells_volt_abnormal
 *��������:��ѹ�Ƿ����쳣
 *��    ��:context
 *�� �� ֵ:FALSE���쳣 TRUE���쳣
 *�޶���Ϣ:
 ******************************************************************************/
static INT8U is_cells_volt_abnormal(BsuSampleContext *__far context)
{
	INT8U bsu_index = 0; //��ǰ�ӻ���
	INT8U pos = 0;
	INT8U bsu_num = 0;  //�ɼ�����
	INT8U  bsu_n_cells_num = 0; //��N���ɼ�������
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
			if ((context->volt_item.volt_buffer[bsu_index][pos] > LTC6804_CELL_MAX_VOLT)||//�쳣
			(context->volt_item.volt_buffer[bsu_index][pos] < 500))
			{
				return TRUE;
			}
			if (context->volt_item.cable_open_flag[bsu_index] != 0)//�Ѿ�����������
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
			//��ѹͻ��100mv
			if ((abs(context->volt_item.volt_buffer[bsu_index][pos] - context->volt_item.volt[bsu_index][pos]) >= 100)//ǰһ�����һ��
			   &&(abs(avg_volt - context->volt_item.volt_buffer[bsu_index][pos]) > 50)) //������ƽ����ѹ�Ƚ�
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
 *��������:cells_volt_sample_cable_open_diagnose
 *��������:��ѹ���߿�·���
 *��    ��: INT16U (*__far PU_volt)[LTC6804_MAX_CELL_NUM] //������ѹ����
         INT16U (*__far PD_volt)[LTC6804_MAX_CELL_NUM] //������ѹ����
 *�� �� ֵ:��
 *�޶���Ϣ:
 ******************************************************************************/
static void cells_volt_sample_cable_open_diagnose(BsuSampleContext *__far context,const INT16U (*__far PU_volt)[LTC6804_MAX_CELL_NUM], const INT16U (*__far PD_volt)[LTC6804_MAX_CELL_NUM])
{
	INT16U volt_diff = 0;
	INT8U bsu_index = 0; //�ӻ����
	INT8U pos = 0;
	INT16U open_flag = 0;
	INT8U tmp_8u = 0;
	OS_CPU_SR cpu_sr = 0;
	INT8U  bsu_num = 0;       //�ӻ���
	INT8U  bsu_n_cells_num = 0; //��N���ӻ������

    if((context == NULL)||(PU_volt == NULL)||(PD_volt == NULL))
          return;

	bsu_num = get_bsu_num();
	for (bsu_index = 0; bsu_index < bsu_num; bsu_index++)
	{
		if ((context->error_byte[bsu_index].Bits.read_pucv != 0) || //�����ɼ�ͨ������
			(context->error_byte[bsu_index].Bits.read_pudv != 0)) //�����ɼ�ͨ������
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
			//���������䣬���ݲ��ɿ�����
			context->volt_item.volt_buffer[bsu_index][(bsu_n_cells_num - 1)] = 0;
			context->volt_item.volt_buffer[bsu_index][(bsu_n_cells_num - 2)] = 0;
		}
		for (pos = 1; pos < bsu_n_cells_num; pos++)
		{
			volt_diff = PD_volt[bsu_index][pos] - PU_volt[bsu_index][pos];
			if ((volt_diff > 400) && (volt_diff < 0x8000))
			{ //���߸���2����ص�ѹ��Ч
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
 *��������:copy_cells_volt_with_filter
 *��������:����ص�ѹ����buffer�����ݿ��������յ�ѹ����volt��,��Ч��ѹ�����˲�10����0
 *��    ��:context
 *�� �� ֵ:��
 *�޶���Ϣ:
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
		if (context->heart_beat[bsu_index] > 0) //ͨ���쳣 û�ж�������
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
		else //ͨ������
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
                else //���쳣ֱ�Ӹ���
                {
                    OS_ENTER_CRITICAL();
                    if(context->volt_item.volt[bsu_index][pos] != 0)//���ӵ�ѹһ���˲� 
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
 *��������:temp_volt_adjust
 *��������:��ѹУ׼,Ӳ��ԭ��-7��������Ҫ���������������ѹ+31mv
 *��    ��:
 *�� �� ֵ:
 *�޶���Ϣ:
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
 *��������:get_ntc_resistance_vaule
 *��������:��ȡNTC�¸еĵ�ǰ��ֵ,���ݲɼ��ĵ�ѹ�������ǰNTC����ֵ
 *��    ��:INT16U r_volt�ɼ��������ϵĵ�ѹ,INT16U ref_volt�ο���ѹ
 *�� �� ֵ:����ֵ(10��/bit)
 *�޶���Ϣ:
 ******************************************************************************/
static INT16U get_ntc_resistance_vaule(INT16U r_volt, INT16U ref_volt,INT8U temp_index)
{
	INT16U R = 0xFFFF;
	INT32U tmp_32u = 0;
	
	r_volt=temp_volt_adjust(r_volt,temp_index);  //Ӳ��ԭ�����¶ȵ���-7��ʱ������ѹ
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
 *��������:convert_volt_to_temp
 *��������:����ѹ������¶�
 *��    ��:context,temp_index
 *�� �� ֵ:��
 *�޶���Ϣ:
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
        
        if (flag == 0)  //���쳣
        {
            if(temp_index<3)
                tmp_16u = get_ntc_resistance_vaule(AuxVoltBuffer[temp_index][bsu_index], 3000,temp_index);
            else 
                tmp_16u = get_ntc_resistance_vaule(AuxVoltBuffer[temp_index-3][bsu_index], 3000,temp_index);
            context->temp_item.temp_buffer[bsu_index][temp_index] = ltc6804_temp_query(tmp_16u); //���ó��¶�
        }
        else
        {
            context->temp_item.temp_buffer[bsu_index][temp_index] = TEMP_CABLE_OPEN_VAULE;
        }
    }
}
/*****************************************************************************
 *��������:copy_cells_temp_with_filter
 *��������:���¶Ȼ�������ݿ����������¶���,��Ч�¶��˲�n�κ��ٿ���
 *��    ��:BsuVoltTempContext *__far context
 *�� �� ֵ:��
 *�޶���Ϣ:
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
        if (context->heart_beat[bsu_index] > 0) //ͨ���쳣 û�ж�������
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
        else //ͨ������
        {
            INT8U start_index = 0;
            INT8U end_index = 0;
            temp_num = get_bsu_n_temp_num(bsu_index);
        #if  BMS_SUPPORT_BY5248D == 0 
            if((config_get(kBYUBalTypeIndex)== BSU_PASSIVE_BALANCE_TYPE)//�����ӻ���һ���ɼ����¶�ƫ�Ʋ�һ��
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
       #else //����6804
		    if(config_get(kBYUBalTypeIndex)== BSU_PASSIVE_BALANCE_TYPE)//�����ӻ���һ���ɼ����¶�ƫ�Ʋ�һ��
            {
                if(bsu_index < BMS_SUPPORT_LTC6804_ON_BOARD_CHIP_NUM)
                {
		    if (temp_num > 4)
					temp_num = 4;
				start_index = 0; //��ʼ��ַΪ0
				end_index = temp_num; //������ַ
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
                {    //�����쳣
                	if (context->temp_item.filter[bsu_index][pos]++ > BSU_TEMP_FILTER_NUM)
                	{
                		OS_ENTER_CRITICAL();
                		context->temp_item.temp[bsu_index][pos] = context->temp_item.temp_buffer[bsu_index][pos];
                		OS_EXIT_CRITICAL();
                		context->temp_item.filter[bsu_index][pos] = BSU_TEMP_FILTER_NUM;
                	}
                }
                else //���쳣ֱ�Ӹ���
                {
                	OS_ENTER_CRITICAL();  //�¶�����һ���˲�
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
 *��������:active_board_temp_index_convert
 *��������:�¶�����ת����Ӳ��������
 *��    ��:
 *�� �� ֵ:��
 *�޶���Ϣ:
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
 *��������:bsu_active_board_temp_sample
 *��������:
 *��    ��:BsuSampleContext *__far context    
 *�� �� ֵ:��
 *�޶���Ϣ:
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
    {  //6804����ԭ��ǰ2�βɼ�����������
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
			OSTimeDly(FILTERED_MODE_AD_TIME);//�ȴ��ɼ����
			if (read_ltc6804_aux_a_volt(bsu_num, AuxVoltBuffer, context->error_byte) == LTC_OK)
			{
				for (bsu_index = 0; bsu_index < bsu_num; bsu_index++)
				{
    				if (context->error_byte[bsu_index].Bits.read_gpio3 == 0)  //���쳣
                    {
                        tmp_16u = get_ntc_resistance_vaule(AuxVoltBuffer[2][bsu_index], 3000,2);
                        context->temp_item.temp_buffer[bsu_index][i] = ltc6804_temp_query(tmp_16u); //���ó��¶�
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
 *��������:bsu_passive_board_temp_sample
 *��������:�¶Ȳɼ�ÿ�βɼ�3·
 *��    ��:BsuSampleContext *__far context    
 *�� �� ֵ:��
 *�޶���Ϣ:
 ******************************************************************************/
static void bsu_passive_board_temp_sample(BsuSampleContext *__far context)
{
    INT8U bsu_num = 0;
    INT8U i = 0;

    if(context == NULL)
        return;
    for (i = 0;i < 2;i++)
    {  //6804����ԭ��ǰ2�βɼ�����������
       start_ltc6804_aux_adc(MD_FILTERED, AUX_CH_ALL); 
       OSTimeDly(FILTERED_MODE_AD_TIME);
    }
    bsu_num = get_bsu_num();
    start_ltc6804_aux_adc(MD_FILTERED, AUX_CH_ALL);
    OSTimeDly(FILTERED_MODE_AD_TIME);//ʱ�䲻����
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
 *��������:bsu_passive_board_temp_sample
 *��������:�¶Ȳɼ�ÿ�βɼ�3·
 *��    ��:BsuSampleContext *__far context    
 *�� �� ֵ:��
 *�޶���Ϣ:
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
 *��������:ReadBsuBoardType
 *��������:��ȡ����BSU������
 *��    ��:BsuBoardType *__far board_type
 *�� �� ֵ:��
 *�޶���Ϣ:
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
            	if (AuxVoltBuffer[1][bsu_index] > 3500)//����3.5VΪ���ɼ��壬С��3.5VΪ�Ӳɼ���
            	{
            		context->board_type[bsu_index] = kBoardMain;
            	}
            	else 
            	{
            		context->board_type[bsu_index] = kBoardSlave;
            	}
            }
            else //ͨ�Ŵ���  δ֪
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
 *��������:is_all_bsu_board_type_read
 *��������:����BSU�����Ƿ��ȡOK
 *��    ��: ��
 *�� �� ֵ: FALSE--��; TRUE--��
 *�޶���Ϣ:
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
        return TRUE;//��������ÿ���ɼ��嶼�����ɼ���
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
 *��������:bsu_board_type_io_set
 *��������:����BSU��������IO����
 *��    ��:��
 *�� �� ֵ:
 *�޶���Ϣ:
 ******************************************************************************/
void bsu_board_type_io_set(void)
{
    INT8U bsu_num = 0;
    INT8U bsu_index = 0;
    BsuSampleContext* __far context = &bsu_sample_context;
    //if(config_get(kBYUBalTypeIndex) == BSU_ACTIVE_BALANCE_TYPE)
    //{
    //    return;//��������ɼ����ڳ�ʼ��ʱ�Ѿ����ù�
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
           bsu_board_gpio_init(bsu_index);//δ֪���ͳ�ʼ����������IO����
           CLR(context->board_gpio_init_flag,bsu_index); 
       }
       
       if((get_bsu_n_board_type(bsu_index) == kBoardSlave)&&(!GET(context->board_gpio_init_flag,bsu_index)))
       {
            bsu_board_gpio_init(bsu_index);//��ȡ�������ͺ󣬵�һ����Ҫ��ʼ��IO����
            SET(context->board_gpio_init_flag,bsu_index); 
       }
       if((get_bsu_n_board_type(bsu_index) == kBoardMain)&&(!GET(context->board_gpio_init_flag,bsu_index)))
       {
            bsu_board_gpio_init(bsu_index);//��ȡ�������ͺ󣬵�һ����Ҫ��ʼ��IO����
            SET(context->board_gpio_init_flag,bsu_index); 
       }
    }
}
/*****************************************************************************
 *��������:set_ltc6804_on_board_bsu_type
 *��������:����6804��BSU��������
 *��    ��:��
 *�� �� ֵ:
 *�޶���Ϣ:
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
 *��������:bsu_board_diagnose
 *��������:����BSU�������
 *��    ��:��
 *�� �� ֵ:
 *�޶���Ϣ:
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
    main_bsu_pos_set();//���ɼ���λ������
}
/*****************************************************************************
 *��������:BalanceControl
 *��������:�������,���ݰ�������������ͬ�ľ������
 *��    ��:
 *�� �� ֵ:
 *�޶���Ϣ:
 ******************************************************************************/
static void bsu_balance_control(void)
{
    static INT8U flag = 0;
    if(config_get(kBYUBalTypeIndex) == BSU_PASSIVE_BALANCE_TYPE)
    {
        bsu_passive_balance_ctrl();
        (void)send_balance_cmd_to_ltc6804(get_bsu_num()); //����6804����
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
 *��������:task_sleep
 *��������:�������ߣ������������ڿ��ƣ���������ĩ��
 *��    ��:INT32U start_tick������ʼʱ��,INT16U task_prodic��������
 *�� �� ֵ:��
 *�޶���Ϣ:
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
 *��������:VoltTempSampleTask
 *��������:��ѹ�¶Ȳɼ�����
 *��    ��:void *pdata
 *�� �� ֵ:��
 *�޶���Ϣ:
 ******************************************************************************/
void BsuVoltTempSampleTask(void *pdata)
{
    void* p_arg = pdata;

    INT32U start_tick = 0;	    //������ʼʱ��
    INT8U bsu_num = 0;
    pdata = pdata;
    //LTC6804��ʼ��
    (void) ltc6804_init(get_bsu_num());

    for (;;)
    {
        bsu_num = get_bsu_num();
        start_tick = OSTimeGet();
        watch_dog_feed(WDT_AD_ID);        
        //���LTC6804��������
        clr_ltc6804_reg();
        //����6804����ȡ��������
        if(runmode_get() == 2)wakeup_ltc6804_with_write_cfg_reg(bsu_num,TRUE);
        else wakeup_ltc6804_with_write_cfg_reg(bsu_num,FALSE);//�رձ�������
        bsu_board_diagnose(&bsu_sample_context);
        /*************************************��ѹ�ɼ�************************************************************/
        //������زɼ�,  ģʽ�����е��
        start_ltc6804_cells_volt_adc(MD_FILTERED, DCP_DISABLED, CELL_CH_ALL);
        //�ȴ��ɼ����
        OSTimeDly(FILTERED_MODE_AD_TIME);//ʱ�䲻����
        //�����ʱ��ѹ��������
        ltc6804_buffer_set(&bsu_sample_context.volt_item.volt_buffer[0][0], 0, PARAM_BSU_NUM_MAX * LTC6804_MAX_CELL_NUM * sizeof(INT16U));
        //��ȡ��ص�ѹ
        (void) read_ltc6804_cells_volt(bsu_num, bsu_sample_context.volt_item.volt_buffer, bsu_sample_context.error_byte, kReadCV);
        //�ӻ���ѹͨ���ж�����
        bsu_comm_heart_beat_run(&bsu_sample_context);

        if (is_cells_volt_abnormal(&bsu_sample_context) == TRUE)	    //��ѹ�쳣
        {
            INT8U cnt = 0;
            //������·AD�ɼ�,  ģʽ�����е�� ��������10��
            for (cnt = 0; cnt < 10; cnt++)
            {
                start_ltc6804_volt_cable_open_adc(MD_FILTERED, DCP_DISABLED, PUP_UP, CELL_CH_ALL);
                OSTimeDly(FILTERED_MODE_AD_TIME);
            }
            ltc6804_buffer_set(&PUVoltTmpBuffer[0][0], 0, MAX_LTC6804_IC_NUM * LTC6804_MAX_CELL_NUM * sizeof(INT16U));
            (void) read_ltc6804_cells_volt(bsu_num, PUVoltTmpBuffer, bsu_sample_context.error_byte, kReadPUCV);

            //������·AD�ɼ�,��׼ģʽ�����е��  ��������10��
            for (cnt = 0; cnt < 10; cnt++)
            {
                start_ltc6804_volt_cable_open_adc(MD_FILTERED, DCP_DISABLED, PUP_DOWN, CELL_CH_ALL);
                OSTimeDly(FILTERED_MODE_AD_TIME);
            }
            ltc6804_buffer_set(&PDVoltTmpBuffer[0][0], 0, MAX_LTC6804_IC_NUM * LTC6804_MAX_CELL_NUM * sizeof(INT16U));
            (void) read_ltc6804_cells_volt(bsu_num, PDVoltTmpBuffer, bsu_sample_context.error_byte, kReadPUDV);
            //��ѹ������߿�·���
            cells_volt_sample_cable_open_diagnose(&bsu_sample_context, PUVoltTmpBuffer, PDVoltTmpBuffer);
            start_ltc6804_cells_volt_adc(MD_FILTERED, DCP_DISABLED, CELL_CH_ALL);//���Ӵ˲�����Ŀ���Ƿ�ֹ��·���ɼ��������ɼ��ճ�-40mv����
        }
        //������ѹ�����ջ���
        copy_cells_volt_with_filter(&bsu_sample_context);
        bsu_update_bcu_volt_info();
        task_sleep(start_tick, (BSU_VOLT_TEMP_SAMPLE_TASK_PRODIC/2));
        start_tick = OSTimeGet();
        /*************************************�¶Ȳɼ�************************************************************/
        wakeup_ltc6804_with_write_cfg_reg(bsu_num,TRUE); //���´򿪱������� 
        bsu_board_diagnose(&bsu_sample_context);
        bsu_temp_sample(&bsu_sample_context); 
        copy_cells_temp_with_filter(&bsu_sample_context);
        bsu_update_bcu_temp_info();
        set_sample_once_finish_flag(&bsu_sample_context);//��һ�ɼ����
        /*************************************�������************************************************************/
        if(runmode_get() != 2) bsu_balance_control();
        task_sleep(start_tick, (BSU_VOLT_TEMP_SAMPLE_TASK_PRODIC/2));
    }
}
#endif