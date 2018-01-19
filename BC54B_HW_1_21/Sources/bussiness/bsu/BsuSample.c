/*******************************************************************************
 **                       ������������Դ�������޹�˾ Copyright (c)
 **                            http://www...
 **�� �� ��:BsuSample.c
 **��������:2015.10.15
 **�ļ�˵��:��ѹ�¶Ȳɼ����������
 **�޸ļ�¼:
 **��    ��:V1.0
 **��    ע:
 *******************************************************************************/
#include "bms_bmu.h"
#include "BsuSample.h"
#include "ltc_util.h"
#include "bms_bsu.h"
#include "bsu_passive_balance.h"
#include "ltc_util.h"
#if BMS_SUPPORT_HARDWARE_LTC6804 == 1
//��ض�Ӧ�Ŀ���λ�ö���
const INT8U ltc6804_cross_pack_bits[PARAM_BSU_NUM_MAX] = {0x00,0x00,0x00,0x00,0x00,0x00,
                                                          0x00,0x00,0x00,0x00,0x00,0x00,
                                                          0x00,0x00,0x00,0x00,0x00,0x00,
                                                          0x00,0x00,0x00,0x00,0x00,0x00,
                                                          0x00,0x00,0x00,0x00,0x00,0x00
                                                         };

static OS_STK BsuSampleTaskStack[AD_SAMPLE_TASK_STK_SIZE] = { 0 }; //��ѹ�¶Ȳɼ������ջ

#pragma DATA_SEG __RPAGE_SEG PAGED_RAM_BYU_SAMPLE
//�ɼ���Ϣ
static BsuSampleContext bsu_sample_context;
//�����ɼ��ĵ�ص�ѹ
static INT16U PUVoltTmpBuffer[PARAM_BSU_NUM_MAX][LTC6804_MAX_CELL_NUM];
//�����ɼ��ĵ�ص�ѹ
static INT16U PDVoltTmpBuffer[PARAM_BSU_NUM_MAX][LTC6804_MAX_CELL_NUM];
#pragma DATA_SEG DEFAULT 

#pragma DATA_SEG __RPAGE_SEG PAGED_RAM_BSU 
static INT16U VoltBuffer[PARAM_BSU_NUM_MAX]; //�¶ȵ�ѹ��ʱ����
#pragma DATA_SEG DEFAULT
//��������
static void bsu_data_init(BsuSampleContext* __far context);
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
	bsu_data_init(&bsu_sample_context); //���ݳ�ʼ��
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
 *��������:��ȡ����,��N���ӻ��ĵ����
 *��    ��:bsu_index  �ӻ���(��0��ʼ)
 *�� �� ֵ:��Ӧ�ӻ��ĵ����
 *�޶���Ϣ:
 ******************************************************************************/
INT8U get_bsu_n_cells_num(INT8U bsu_index)
{
	INT8U num = 0;
	//�������
	if (bsu_index >= PARAM_BSU_NUM_MAX)
		return 0;
    if(bsu_index < 10)
	    num = (INT8U) config_get(bsu_index + kBYUVoltCnt1Index);
    else
        num = 0;
	return num;
}
/*****************************************************************************
 *��������:is_bsu_n_cross_pack_en
 *��������:bsu�Ƿ����
 *��    ��:bsu_index  �ӻ���(��0��ʼ)
 *�� �� ֵ:TRUE��  FALSE��
 *�޶���Ϣ:
 ******************************************************************************/
BOOLEAN is_bsu_n_cross_pack_en(INT8U bsu_index)
{
	//�������
	if (bsu_index >= PARAM_BSU_NUM_MAX)
		return FALSE;
    if(ltc6804_cross_pack_bits[bsu_index]==0)
        return FALSE;
	return TRUE;
}
/*****************************************************************************
 *��������:get_bsu_num
 *��������:��ȡbsu��Ŀ
 *��    ��:��
 *�� �� ֵ:��
 *�޶���Ϣ:
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
 *��������:get_bsu_n_top_cells_num
 *��������:��ȡ��N���ӻ�оƬ���˵�о�� ,�������Ϊ����ʱ�׶˱ȶ��˶�1��,λż��ʱһ��
 *��    ��:bsu_index  �ӻ���(��0��ʼ)
 *�� �� ֵ:��Ӧ�ӻ�оƬ���˵�о��
 *�޶���Ϣ:
 ******************************************************************************/
INT8U get_bsu_n_top_cells_num(INT8U bsu_index)
{
	INT8U num = 0;
	INT8U bsu_n_cells_num = 0;
	//�������
	if (bsu_index >= PARAM_BSU_NUM_MAX)
		return 0;

	bsu_n_cells_num = get_bsu_n_cells_num(bsu_index);
	num = (bsu_n_cells_num >> 1);//���˳�2����

	return num;
}
/*****************************************************************************
 *��������:GetBsuNBottomCellsNum
 *��������:��ȡ��N���ӻ�оƬ�׶˵�о�� ,�������Ϊ����ʱ�׶˱ȶ��˶�1��,λż��ʱһ��
 *��    ��:INT8U bsu_index  �ӻ���:��0��ʼ
 *�� �� ֵ:��Ӧ�ӻ�оƬ�׶˵�о��
 *�޶���Ϣ:
 ******************************************************************************/
INT8U get_bsu_n_bottom_cells_num(INT8U bsu_index)
{
	INT8U num = 0;
	INT8U bsu_n_cells_num = 0;
	//�������
	if (bsu_index >= PARAM_BSU_NUM_MAX)
		return 0;

	bsu_n_cells_num = get_bsu_n_cells_num(bsu_index);
	num = (bsu_n_cells_num >> 1) + (bsu_n_cells_num & 0x01);  //�׶˳�2�������ż����0,�����������1

	return num;
}
/*****************************************************************************
 *��������:bsu_data_init
 *��������:�ӻ�BSU���ݳ�ʼ��
 *��    ��:context�ӻ��ɼ���������
 *�� �� ֵ:��
 *�޶���Ϣ:
 ******************************************************************************/
static void bsu_data_init(BsuSampleContext* __far context)
{
	INT8U index = 0;
	INT8U bsu_index = 0; //�ӻ�������
	//�������
	if(context == NULL)
	    return;
	//��ѹ����
	ltc6804_buffer_set(&context->volt_item,0,sizeof(BsuVoltSampleItem));
	//���������ֵ
	ltc6804_buffer_set(context->heart_beat,BSU_MAX_COMM_ERROR_CNT,PARAM_BSU_NUM_MAX);
	//��������
	ltc6804_buffer_set(&context->board_type,kBoardUnknow,PARAM_BSU_NUM_MAX);

	for (bsu_index = 0; bsu_index < PARAM_BSU_NUM_MAX; bsu_index++)
	{
		context->error_byte[bsu_index].value = 0;
		//�¶�����
		for (index = 0; index < LTC6804_MAX_CELL_NUM; index++)
		{
			context->temp_item.temp[bsu_index][index] = TEMP_CABLE_OPEN_VAULE;//�¶���0xFF
			context->temp_item.temp_buffer[bsu_index][index] = 0;
			context->temp_item.filter[bsu_index][index] = 0;
		}
	}
}
/*****************************************************************************
 *��������:get_bsu_cells_volt
 *��������:��ȡ��ص�ѹ
 *��    ��:bsu_index�ӻ���,pos����ڴӻ��е�λ��
 *�� �� ֵ:��ص�ѹ
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
 *��������:��ȡ����¶�
 *��    ��:INT8U bsu_index�ӻ���,INT8U pos�¸б��
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
 *��������:bsu_cells_volt_reorganize
 *��������:��ѹ��������
 *         ����ÿ���ӻ�������ͬ��Ҫ����ѹ����ʹ֮����
 *��    ��:INT16U  (*__far volt)[LTC6804_MAX_CELL_NUM] ��LTC6804�ж������ĵ�ѹ��ʱ��������
 *�� �� ֵ:��
 *�޶���Ϣ:
 ******************************************************************************/
static void bsu_cells_volt_reorganize(INT16U (*__far volt)[LTC6804_MAX_CELL_NUM])
{
	INT8U index = 0;
	INT8U bsu_index = 0; //�ӻ�������
	INT8U bottom_not_use_num = 0;//�°�Ƭδʹ�õ����
	INT8U bsu_num = 0;//�ӻ���
	INT8U bsu_n_cells_num = 0;//�ӻ������
	//�������
	if(volt == NULL)
	    return;

	bsu_num = get_bsu_num();
	for(bsu_index = 0; bsu_index < bsu_num; bsu_index++)
	{
		bsu_n_cells_num = get_bsu_n_cells_num(bsu_index);
		if(bsu_n_cells_num == LTC6804_MAX_CELL_NUM) //Ϊ���ֵ,��������
		{
			continue;
		}
		bottom_not_use_num = LTC6804_HALF_CELL_NUM - get_bsu_n_bottom_cells_num(bsu_index);

		if(bottom_not_use_num > 0)//�ײ�δ��6��,������Ҫ����
		{
			for(index = get_bsu_n_bottom_cells_num(bsu_index);index < bsu_n_cells_num;index++)
			{
				volt[bsu_index][index] = volt[bsu_index][index+ bottom_not_use_num];
			}
		}

		//ʣ�µ�����
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
 *��������:�ӻ�ͨ���ж�����
 *��    ��:context�ӻ���ѹ�¶���Ϣ
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

	bsu_num = get_bsu_num();//�ӻ���
	for (bsu_index = 0; bsu_index < bsu_num; bsu_index++)
	{
		OS_ENTER_CRITICAL();
		if (context->error_byte[bsu_index].Bits.read_cv)//����ѹͨ���쳣
		{
			context->heart_beat[bsu_index]++;  //��������
			if (context->heart_beat[bsu_index] >= BSU_MAX_COMM_ERROR_CNT)//�ﵽ���ֵ
			{
				context->heart_beat[bsu_index] = BSU_MAX_COMM_ERROR_CNT; //�ӻ�ͨ���ж�
				context->volt_item.cable_open_flag[bsu_index] = 0;
			}
		}
		else
		{
			context->heart_beat[bsu_index] = 0;
		}
		OS_EXIT_CRITICAL();
	}
	//δʹ�õĴӻ�ȫ�����Ϊͨ���ж�
	for (; bsu_index < PARAM_BSU_NUM_MAX; bsu_index++)
	{
		OS_ENTER_CRITICAL();
		context->heart_beat[bsu_index] = BSU_MAX_COMM_ERROR_CNT;
		OS_EXIT_CRITICAL();
	}
	clr_all_bsu_comm_error_byte();
}
/*****************************************************************************
 *��������:is_bsu_n_comm_error
 *��������:�жϵ�N���ӻ��Ƿ�ͨ��ʧ��
 *��    ��:bsu_index�ӻ����
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
 *��������:�ж����еĴӻ��Ƿ�ͨ��ʧ��
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
 *��������:�ж��Ƿ��дӻ�ͨ��ʧ��
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
 *��    ��:INT8U bsu_index�ӻ����, INT8U cell_index���λ��                                 
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
 *��������:��ȡ�ӻ����������־,bit0����1�ŵ�ص��������������
 *��    ��:INT8U bsu_index�ӻ����
 *�� �� ֵ:��
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
 *��������:�ӻ���������
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
 *��������:get_temp_filter_cnt
 *��������:��ȡ�¶��˲�����
 *��    ��:bsu_index�ӻ����, index�¶Ⱥ�                                     
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
 *��������:is_cells_volt_abnormal
 *��������:��ѹ�Ƿ����쳣
 *��    ��:item
 *�� �� ֵ:FALSE���쳣 TRUE���쳣
 *�޶���Ϣ:
 ******************************************************************************/
static INT8U is_cells_volt_abnormal(BsuSampleContext *__far context)
{
	INT8U bsu_index = 0; //��ǰ�ӻ���
	INT8U pos = 0;
	INT8U bsu_num = 0;  //�ӻ���
	INT8U  bsu_n_cells_num = 0; //��N���ӻ������
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
			if (context->volt_item.volt_buffer[bsu_index][pos] > LTC6804_CELL_MAX_VOLT)//�쳣
			{
				return TRUE;
			}
			if (context->volt_item.cable_open_flag[bsu_index] != 0)//�Ѿ�����������
			{
				return TRUE;
			}
			if (abs(context->volt_item.volt_buffer[bsu_index][pos] - context->volt_item.volt[bsu_index][pos]) >= 100)//��ѹͻ��100mv
			{
			    return TRUE;  
			}
		}
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
			SET(open_flag, 0);
		}
		if (PD_volt[bsu_index][bsu_n_cells_num - 1] == 0)
		{ //TODO  ������ⲿ�������B12+���伴��
			for (pos = 0; pos < bsu_n_cells_num; pos++)
			{
				SET(open_flag, pos);
                //���������䣬���ݲ��ɿ�����
                context->volt_item.volt_buffer[bsu_index][pos] = 0;
			}
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
 *��������:get_ntc_resistance_vaule
 *��������:��ȡNTC�¸еĵ�ǰ��ֵ,���ݲɼ��ĵ�ѹ�������ǰNTC����ֵ
 *��    ��:INT16U r_volt�ɼ��������ϵĵ�ѹ,INT16U ref_volt�ο���ѹ
 *�� �� ֵ:����ֵ(10��/bit)
 *�޶���Ϣ:
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
    if (temp_index >= LTC6804_MAX_TEMP_NUM)
        return;
    if(context == NULL)
        return;
    bsu_num = get_bsu_num();
    for (bsu_index = 0; bsu_index < bsu_num; bsu_index++)
    {
        if (context->error_byte[bsu_index].Bits.read_gpio1 == 0)//���쳣
        {
            tmp_16u = get_ntc_resistance_vaule(VoltBuffer[bsu_index], 3000);
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
            temp_num = (INT8U)config_get(kBmuMaxTempNumIndex);
            if(temp_num > LTC6804_MAX_TEMP_NUM)
                temp_num = LTC6804_MAX_TEMP_NUM;
            for (pos = 0; pos < temp_num; pos++)
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
 *��������:bsu_temp_sample
 *��������:�¶Ȳɼ�ÿ�βɼ�3·
 *��    ��:BsuSampleContext *__far context    
 *�� �� ֵ:��
 *�޶���Ϣ:
 ******************************************************************************/
static void bsu_temp_sample(BsuSampleContext *__far context)
{
    INT8U index = 0;
    INT8U channel = 0; //ͨ��
    INT8U bsu_index = 0;
    static INT8U flag = 0;//�л���־
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
        if(flag == 0) //ÿ�ν��ɼ�3·
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

        result = pcf8574_write_channel(bsu_num, channel); //ѡ���¶�

        if (result == LTC_OK)  //40ms
        {
            OSTimeDly(2);
            start_ltc6804_aux_adc(MD_NORMAL, AUX_CH_GPIO1);
            OSTimeDly(4);  //�ȴ��ɼ����
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
 *��������:ReadBsuBoardType
 *��������:��ȡ����BSU������
 *��    ��:BsuBoardType *__far board_type
 *�� �� ֵ:��
 *�޶���Ϣ:
 ******************************************************************************/
static void read_bsu_board_type(BsuSampleContext *__far context)
{
    INT8U index = 0;
    INT8U channel = 0; //ͨ��
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
 *��������:get_bsu_n_board_type
 *��������:��ȡ���Ӿ�������,���ڷ�����λ����HMI
 *��    ��:INT8U bsu_index �ӻ����
 *�� �� ֵ:��������
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
 *��������:is_all_bsu_passive_board
 *��������:���дӻ��Ƿ�Ϊ�����������
 *��    ��: ��
 *�� �� ֵ: FALSE--��; TRUE--��
 *�޶���Ϣ:
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
 *��������:IsAllBsuActiveBoard
 *��������:���дӻ��Ƿ�Ϊ�����������
 *��    ��:��
 *�� �� ֵ:FALSE--��; TRUE--��
 *�޶���Ϣ:
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
 *��������:BalanceControl
 *��������:�������,���ݰ�������������ͬ�ľ������
 *��    ��:
 *�� �� ֵ:
 *�޶���Ϣ:
 ******************************************************************************/
static void bsu_balance_control(void)
{
    if (is_all_bsu_passive_board() == TRUE)
    {
        bsu_passive_balance_ctrl(); //��������
    }
    else if (is_all_bsu_active_board() == TRUE)
    {
     //ActiveBalanceControl();//��������  TODO
    }
    else
    {
        clr_all_ltc6804_balance_ctrl_word(); //�������ͨ�����е�����
        //ClrAllBsuActiveBalanceWord();    TODO
        clr_all_bsu_passive_balance_word();
        read_bsu_board_type(&bsu_sample_context);	    //��ȡ�����ͺ�
    }
}

/*****************************************************************************
 *��������:set_bsu_relay1_on
 *��������:bsu�ӻ��̵�������
 *��    ��:
 *�� �� ֵ:
 *�޶���Ϣ:
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
 *��������:set_bsu_relay1_off
 *��������:bsu�ӻ��̵�������
 *��    ��:
 *�� �� ֵ:
 *�޶���Ϣ:
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
 *��������:set_bsu_relay2_on
 *��������:bsu�ӻ��̵�������
 *��    ��:
 *�� �� ֵ:
 *�޶���Ϣ:
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
 *��������:set_bsu_relay2_off
 *��������:bsu�ӻ��̵�������
 *��    ��:
 *�� �� ֵ:
 *�޶���Ϣ:
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
    INT32U tick_diff = 0;	    //ʱ���ֵ
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

        /*************************************��ѹ�ɼ�************************************************************/
        wakeup_ltc6804_with_write_cfg_reg(bsu_num);
        //������زɼ�,  ģʽ�����е��
        start_ltc6804_cells_volt_adc(MD_FILTERED, DCP_DISABLED, CELL_CH_ALL);
        //�ȴ��ɼ����
        OSTimeDly(220);
        //�����ʱ��ѹ��������
        ltc6804_buffer_set(&bsu_sample_context.volt_item.volt_buffer[0][0], 0, PARAM_BSU_NUM_MAX * LTC6804_MAX_CELL_NUM * sizeof(INT16U));
        //��ȡ��ص�ѹ
        (void) read_ltc6804_cells_volt(bsu_num, bsu_sample_context.volt_item.volt_buffer, bsu_sample_context.error_byte, kReadCV);
        //�������õĴ�����ͬ,����ѹ��������
        bsu_cells_volt_reorganize(bsu_sample_context.volt_item.volt_buffer);
        //�ӻ���ѹͨ���ж�����
        bsu_comm_heart_beat_run(&bsu_sample_context);

        if (is_cells_volt_abnormal(&bsu_sample_context) == TRUE)	    //��ѹ�쳣
        {
            INT8U cnt = 0;
            //������·AD�ɼ�,  ģʽ�����е�� ��������10��
            for (cnt = 0; cnt < 10; cnt++)
            {
                start_ltc6804_volt_cable_open_adc(MD_NORMAL, DCP_DISABLED, PUP_UP, CELL_CH_ALL);
                OSTimeDly(3);
            }
            ltc6804_buffer_set(&PUVoltTmpBuffer[0][0], 0, MAX_LTC6804_IC_NUM * LTC6804_MAX_CELL_NUM * sizeof(INT16U));
            (void) read_ltc6804_cells_volt(bsu_num, PUVoltTmpBuffer, bsu_sample_context.error_byte, kReadPUCV);
            bsu_cells_volt_reorganize(PUVoltTmpBuffer);

            //������·AD�ɼ�,��׼ģʽ�����е��  ��������10��
            for (cnt = 0; cnt < 10; cnt++)
            {
                start_ltc6804_volt_cable_open_adc(MD_NORMAL, DCP_DISABLED, PUP_DOWN, CELL_CH_ALL);
                OSTimeDly(3);
            }
            ltc6804_buffer_set(&PDVoltTmpBuffer[0][0], 0, MAX_LTC6804_IC_NUM * LTC6804_MAX_CELL_NUM * sizeof(INT16U));
            (void) read_ltc6804_cells_volt(bsu_num, PDVoltTmpBuffer, bsu_sample_context.error_byte, kReadPUDV);
            bsu_cells_volt_reorganize(PDVoltTmpBuffer);
            //��ѹ������߿�·���
            cells_volt_sample_cable_open_diagnose(&bsu_sample_context, PUVoltTmpBuffer, PDVoltTmpBuffer);
        }
        	    //������ѹ�����ջ���
        copy_cells_volt_with_filter(&bsu_sample_context);
        bsu_update_bcu_volt_info();

        /*************************************�¶Ȳɼ�************************************************************/
        wakeup_ltc6804_with_write_cfg_reg(bsu_num);  

        bsu_temp_sample(&bsu_sample_context); 
        copy_cells_temp_with_filter(&bsu_sample_context);
        bsu_update_bcu_temp_info();
        /*************************************�������************************************************************/
        bsu_balance_control();

        task_sleep(start_tick, BSU_VOLT_TEMP_SAMPLE_TASK_PRODIC);
    }
}
#endif