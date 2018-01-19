/*******************************************************************************
 **                       ��������Դ�������޹�˾ Copyright (c)
 **                            http://www...
 **�� �� ��:ltc68041_impl.c
 **��    ��:��־��
 **��������:2016.12.15
 **�ļ�˵��:LTC6804���þ���ʽ
 **�޸ļ�¼:
 **��    ��:V1.0
 **��    ע:
 *******************************************************************************/
#ifndef _LTC68041_IMPL_C_ 
#define _LTC68041_IMPL_C_

#include "ltc_util.h"
#include "ltc68041_impl.h" 
//#include "SPI1.h"
#include "app_cfg.h"
#include "bms_config.h"
#include "main_bsu_relay_ctrl.h"

#pragma DATA_SEG __RPAGE_SEG PAGED_RAM_BSU_LTC6804
//LTC6804���������
static INT16U ltc6804_balance_ctrl_word[MAX_LTC6804_IC_NUM] = {0};
//6804Gpio���ſ���
static Ltc6804Gpio ltc6804_gpio[MAX_LTC6804_IC_NUM] = {0};
//����д�Ĵ���ʱ��ʱ����.
static INT8U ltc6804_reg_buffer[MAX_LTC6804_IC_NUM][REG_WITH_PEC_LEN] = {0};
#pragma DATA_SEG DEFAULT 

/*****************************************************************************
 *��������:balance_ctrl_word_convert
 *��������:�������Ҫ�����������ת��
 *��    ��:INT8U current_ic,��ǰ��оƬ
           INT16U ctrl_word������(δ�����䴦��Ŀ�����)
 *�� �� ֵ:��
 *�޶���Ϣ:
 ******************************************************************************/
INT16U balance_ctrl_word_convert(INT8U current_ic, INT16U ctrl_word)
{
	INT16U new_ctrl_word = 0;//�µĿ�����(�����䴦���)
	INT8U i = 0,index = 0;
    volatile INT16U bits = 0;
	if (current_ic >= MAX_LTC6804_IC_NUM)
		return 0;
    ctrl_word = ctrl_word&0x0FFF;
	//���䴦��
	bits = config_get(kBSU1VoltSampleNoUseBitIndex+current_ic);
    if((bits!=0)&&(ctrl_word != 0)) 
    {  //������� ����Ҫ����
       new_ctrl_word = 0;
       index = 0;
       
       for(i = 0;i < LTC6804_MAX_CELL_NUM;i++) 
       {
           if(GET(bits,i))
           {//��Ҫ����,����  
              continue;
           }
           if(GET(ctrl_word,index)) 
           {
             SET(new_ctrl_word,i);
           }
           index++;
       }
    }
    else
    {  //������䴦��
       new_ctrl_word = ctrl_word;
    }
    return new_ctrl_word;
}


/*****************************************************************************
 *��������:set_ltc6804_balance_ctrl_word
 *��������:����LTC6804���������
 *��    ��:INT8U current_ic,��ǰ��оƬ
           INT16U ctrl_word������(δ�����䴦��Ŀ�����)
 *�� �� ֵ:��
 *�޶���Ϣ:
 ******************************************************************************/
void set_ltc6804_balance_ctrl_word(INT8U current_ic, INT16U ctrl_word)
{
	OS_CPU_SR cpu_sr = 0;
	INT16U new_ctrl_word = 0;//�µĿ�����(�����䴦���)
	//�������
	if (current_ic >= MAX_LTC6804_IC_NUM)
		return;

     new_ctrl_word = balance_ctrl_word_convert(current_ic,ctrl_word);

     
	OS_ENTER_CRITICAL();
	ltc6804_balance_ctrl_word[current_ic] = (new_ctrl_word & 0x0FFF);
	OS_EXIT_CRITICAL();

}
/*****************************************************************************
 *��������:get_ltc6804_balance_ctrl_word
 *��������:��ȡLTC6804���������
 *��    ��:INT8U current_ic,��ǰ��оƬ
 *�� �� ֵ:���������
 *�޶���Ϣ:
 ******************************************************************************/
static INT16U get_ltc6804_balance_ctrl_word(INT8U current_ic)
{
	OS_CPU_SR cpu_sr = 0;
	INT16U ctrl_word = 0;
	//�������
	if (current_ic >= MAX_LTC6804_IC_NUM)
		return 0;

	OS_ENTER_CRITICAL();
	ctrl_word = (ltc6804_balance_ctrl_word[current_ic]& 0x0FFF);
	OS_EXIT_CRITICAL();
	return ctrl_word;
}
/*****************************************************************************
 *��������:clr_all_ltc6804_balance_ctrl_word
 *��������:����������������
 *��    ��:��
 *�� �� ֵ:��
 *�޶���Ϣ:
 ******************************************************************************/
void clr_all_ltc6804_balance_ctrl_word(void)
{
	INT8U current_ic = 0;
	OS_CPU_SR cpu_sr = 0;
	for (current_ic = 0; current_ic < MAX_LTC6804_IC_NUM; current_ic++)
	{
        OS_ENTER_CRITICAL();
        ltc6804_balance_ctrl_word[current_ic] = 0;
        OS_EXIT_CRITICAL();
	}
}
/*****************************************************************************
 *��������:set_ltc6804_gpio2_pin_high
 *��������:����GPIO2λΪ��
 *��    ��:INT8U current_ic��ǰ6804���
 *�� �� ֵ:
 *�޶���Ϣ:
 ******************************************************************************/
void set_ltc6804_gpio2_pin_high(INT8U current_ic)
{
   		//�������
	if (current_ic >= MAX_LTC6804_IC_NUM)
		return ;
   	ltc6804_gpio[current_ic].Bits.GPIO2 = 1;
}
/*****************************************************************************
 *��������:SetLTCGpio2BitLow
 *��������:����GPIO2λΪ��
 *��    ��:INT8U current_ic��ǰ6804���
 *�� �� ֵ:
 *�޶���Ϣ:
 ******************************************************************************/
void set_ltc6804_gpio2_pin_low(INT8U current_ic)
{
   	//�������
	if (current_ic >= MAX_LTC6804_IC_NUM)
		return ;
   	ltc6804_gpio[current_ic].Bits.GPIO2 = 0;
}
/*****************************************************************************
 *��������:GetLTCGpio2Bit
 *��������:��ȡGPIO2λ
 *��    ��:INT8U current_ic��ǰ6804���
 *�� �� ֵ:
 *�޶���Ϣ:
 ******************************************************************************/
INT8U is_ltc6804_gpio2_pin_high(INT8U current_ic)
{
   	//�������
	if (current_ic >= MAX_LTC6804_IC_NUM)
		return 0;
   	return ltc6804_gpio[current_ic].Bits.GPIO2;
}
/*****************************************************************************
 *��������:set_ltc6804_gpio1_pin_high
 *��������:����GPIO3λΪ��
 *��    ��:INT8U current_ic��ǰ6804���
 *�� �� ֵ:
 *�޶���Ϣ:
 ******************************************************************************/
void set_ltc6804_gpio1_pin_high(INT8U current_ic)
{
   //�������
	if (current_ic >= MAX_LTC6804_IC_NUM)
		return ;
   	ltc6804_gpio[current_ic].Bits.GPIO1 = 1;
}
/*****************************************************************************
 *��������:set_ltc6804_gpio3_pin_low
 *��������:����GPIO1λΪ��
 *��    ��:INT8U current_ic��ǰ6804���
 *�� �� ֵ:
 *�޶���Ϣ:
 ******************************************************************************/
void set_ltc6804_gpio1_pin_low(INT8U current_ic)
{
   	//�������
	if (current_ic >= MAX_LTC6804_IC_NUM)
		return ;
   	ltc6804_gpio[current_ic].Bits.GPIO1 = 0;
}
/*****************************************************************************
 *��������:is_ltc6804_gpio1_pin_high
 *��������:��ȡGPIO1λ
 *��    ��:INT8U current_ic��ǰ6804���
 *�� �� ֵ:
 *�޶���Ϣ:
 ******************************************************************************/
INT8U is_ltc6804_gpio1_pin_high(INT8U current_ic)
{
   	//�������
	if (current_ic >= MAX_LTC6804_IC_NUM)
		return 0;
   	return ltc6804_gpio[current_ic].Bits.GPIO1;
}
/*****************************************************************************
 *��������:bsu_board_gpio_init
 *��������:�ɼ���IO��ʼ��
 *��    ��:INT8U current_ic��ǰ6804���
 *�� �� ֵ:
 *�޶���Ϣ:
 ******************************************************************************/
void bsu_board_gpio_init(INT8U current_ic)
{
    if (current_ic >= MAX_LTC6804_IC_NUM)
		return ;
    //GPIO1 GPIO2���̵�������ʹ��,��Ҫ��0
    ltc6804_gpio[current_ic].Bits.GPIO1 = 1;
    ltc6804_gpio[current_ic].Bits.GPIO2 = 1;
    ltc6804_gpio[current_ic].Bits.GPIO3 = 1;
    ltc6804_gpio[current_ic].Bits.GPIO4 = 1;
    ltc6804_gpio[current_ic].Bits.GPIO5 = 1;
    ltc6804_gpio[current_ic].Bits.NA = 0;
}

/*****************************************************************************
 *��������:broad_cast_ltc6804_cmd
 *��������:�㲥����,�ں�CS��ƽ������
 *��    ��:cmd ����
 *�� �� ֵ:��
 *�޶���Ϣ:
 ******************************************************************************/
void broad_cast_ltc6804_cmd(INT16U cmd)
{
	INT8U tx_data[4] = { 0 }; //��������
	INT16U tmp_pec = 0; //pecУ��
    
	tx_data[0] = (INT8U) (cmd >> 8);
	tx_data[1] = (INT8U) cmd;
	tmp_pec = pec15_calc(2, tx_data);
	tx_data[2] = (INT8U) (tmp_pec >> 8);
	tx_data[3] = (INT8U) (tmp_pec);

	wakeup_iso_spi_idle();
    OSSchedLock();
	ltc6804_cs_pin_low(); //Ƭѡ
	(void)spi_write_array(&ltc6804_high_speed_spi_bus,4,tx_data);
	ltc6804_cs_pin_high(); //ȡ��Ƭѡ
	OSSchedUnlock();
}
/*****************************************************************************
 *��������:clr_ltc6804_reg
 *��������:���LTC6804�Ĵ���,������ص�ѹ��������״̬�Ĵ�����10������ִ��һ��
 *��    ��:��
 *�� �� ֵ:��
 *�޶���Ϣ:
 ******************************************************************************/
void clr_ltc6804_reg(void)
{
	static INT8U cnt = 0;
	if (cnt % 10 == 0)
	{
		broad_cast_ltc6804_cmd(CLRCELL);
		OSTimeDly(2);
		broad_cast_ltc6804_cmd(CLRAUX);
		OSTimeDly(2);
		broad_cast_ltc6804_cmd(CLRSTAT);
		OSTimeDly(5);
	}
	cnt++;
}
/*****************************************************************************
 *��������:start_ltc6804_cells_volt_adc
 *��������:������ص�ѹ�ɼ�
 *��    ��:INT8U MD ģʽ ����ģʽMD_FAST���١�MD_NORMAL������MD_FILTERED�˲�
 *         INT8U DCP �ɼ���������DCP_ENABLEDʹ�� DCP_DISABLED��ֹ
 *         INT8U CH ͨ��  CELL_CH_ALL����
 *�� �� ֵ:��
 *�޶���Ϣ:
 ******************************************************************************/
void start_ltc6804_cells_volt_adc(INT8U MD, INT8U DCP, INT8U CH)
{
	
	INT16U tmp_16u = 0;
	INT16U cmd = 0;
	MD = MD & 0x03;
	DCP = DCP & 0x01;
	CH = CH & 0x07;

	tmp_16u = ((INT16U)MD<<7);
	cmd = tmp_16u  + (DCP << 4) + CH + 0x260;
	
	broad_cast_ltc6804_cmd(cmd);
}
/*****************************************************************************
 *��������:start_ltc6804_volt_cable_open_adc
 *��������:������ѹ���߿�·�ɼ�
 *��    ��:INT8U MD ģʽ ����ģʽMD_FAST���١�MD_NORMAL������MD_FILTERED�˲�
 *         INT8U PUP������������
 *         INT8U DCP �ɼ���������DCP_ENABLEDʹ�� DCP_DISABLED��ֹ
 *         INT8U CH ͨ��  CELL_CH_ALL����
 *�� �� ֵ:��
 *�޶���Ϣ:
 ******************************************************************************/
void start_ltc6804_volt_cable_open_adc(INT8U MD, INT8U DCP, INT8U PUP, INT8U CH)
{
	INT16U tmp_16u = 0;
	INT16U cmd = 0;
	MD = MD & 0x03;
	DCP = DCP & 0x01;
	PUP = PUP & 0x01;
	CH = CH & 0x07;

	tmp_16u = ((INT16U)MD<<7);
	cmd = tmp_16u + (PUP << 6) + (DCP << 4) + CH + 0x228;

	broad_cast_ltc6804_cmd(cmd);
}
/*****************************************************************************
 *��������:start_ltc6804_aux_adc
 *��������:����GPIO����׼����AD�ɼ�
 *��    ��:INT8U MD ģʽ ����ģʽMD_FAST���١�MD_NORMAL������MD_FILTERED�˲�
 *         INT8U AUX_CH ͨ�� 
 *�� �� ֵ:��
 *�޶���Ϣ:
 ******************************************************************************/
void start_ltc6804_aux_adc(INT8U MD, INT8U AUX_CH)
{
	INT16U tmp_16u = 0;
	INT16U cmd = 0;
	MD = MD & 0x03;
	AUX_CH = AUX_CH & 0x07;

	tmp_16u =  ((INT16U)MD<<7);
	cmd = tmp_16u + 0x460 + AUX_CH;

	broad_cast_ltc6804_cmd(cmd);
}
/*****************************************************************************
 *��������:start_ltc6804_stat_adc
 *��������:����״̬��ɼ�
 *��    ��:INT8U MD ģʽ ����ģʽMD_FAST���١�MD_NORMAL������MD_FILTERED�˲�
 *         INT8U CHST ͨ�� 
 *�� �� ֵ:��
 *�޶���Ϣ:
 ******************************************************************************/
void start_ltc6804_stat_adc(INT8U MD, INT8U CHST)
{
	INT16U tmp_16u = 0;
	INT16U cmd = 0;
	MD = MD & 0x03;
	CHST = CHST & 0x07;

	tmp_16u =  ((INT16U)MD<<7);
	cmd = tmp_16u + 0x468 + CHST;

	broad_cast_ltc6804_cmd(cmd);
}
/*****************************************************************************
 *��������:send_ltc6804_wrcomm_cmd
 *��������:дCOMM�Ĵ�������(�㲥��ʽ)��ע����CS��ƽ����
 *��    ��:��
 *�� �� ֵ:��
 *�޶���Ϣ:
 ******************************************************************************/
void send_ltc6804_wrcomm_cmd(void)
{
	INT8U tx_data[4] =	{ 0 };
	INT16U tmp_pec = 0;
	INT16U cmd = WRCOMM;
	tx_data[0] = (INT8U) (cmd >> 8);
	tx_data[1] = (INT8U) (cmd);
	//����PEC
	tmp_pec = pec15_calc(2, tx_data);
	tx_data[2] = (INT8U) (tmp_pec >> 8);
	tx_data[3] = (INT8U) tmp_pec;
	//SPI����
	(void)spi_write_array(&ltc6804_high_speed_spi_bus,4,tx_data);

}

/*****************************************************************************
 *��������:read_ltc6804_register
 *��������:��LTC6804�Ĵ���
 *��    ��:ltc6804_num ��ȡLTC6804оƬ��Ŀ
 *         cmd ����
 *         read_buffer �洢��������
 *�� �� ֵ:LTC_OK,�����ɹ�  ����ʧ��
 *�޶���Ϣ:
 ******************************************************************************/
static LtcResult read_ltc6804_register(INT8U ltc6804_num, INT16U cmd, INT8U*far read_buffer)
{
	INT8U tx_data[4] = { 0 };//������������
	INT16U tmp_pec = 0;//PECУ��
	INT16U read_len = 0;

	//�������
	if ((ltc6804_num > MAX_LTC6804_IC_NUM) || (ltc6804_num == 0))
	{
		ltc6804_num = MAX_LTC6804_IC_NUM;
	}

	if ((cmd != RDCFG) && //�����üĴ���
		(cmd != RDCVA) && //����ص�ѹ�Ĵ�����A
		(cmd != RDCVB) && //����ص�ѹ�Ĵ�����B
		(cmd != RDCVC) && //����ص�ѹ�Ĵ�����C
		(cmd != RDCVD) && //����ص�ѹ�Ĵ�����D
		(cmd != RDAUXA) && //�������Ĵ�����A
		(cmd != RDAUXB) && //�������Ĵ�����B
		(cmd != RDSTATA) && //��״̬�Ĵ�����A
		(cmd != RDSTATB) && //��״̬�Ĵ�����B
		(cmd != RDCOMM)) //��COMM�Ĵ�����
	{
		return LTC_CMD_ERROR;
	}
	if (read_buffer == NULL)
	{
		return LTC_PTR_NULL;
	}

	tx_data[0] = (INT8U) (cmd >> 8);
	tx_data[1] = (INT8U) cmd;
	tmp_pec = pec15_calc(2, tx_data);
	tx_data[2] = (INT8U) (tmp_pec >> 8);
	tx_data[3] = (INT8U) tmp_pec;
        read_len = ((INT16U) ltc6804_num * (REG_WITH_PEC_LEN) * sizeof(INT8U));
	OSSchedLock();
	wakeup_iso_spi_idle(); //����ISOspi����
	ltc6804_cs_pin_low(); //Ƭѡ
	(void)spi_write_read_array(&ltc6804_high_speed_spi_bus,tx_data, 4, read_buffer, read_len);
	ltc6804_cs_pin_high(); //ȡ��Ƭѡ
    OSSchedUnlock();
	return LTC_OK;
}
/*****************************************************************************
 *��������:read_ltc6804_cells_volt
 *��������:��ȡ��ص�ѹ
 *��    ��:INT8U ltc6804_num оƬ��Ŀ
 *         INT16U  (*_LTC_PAGED cell_volt)[LTC6804_MAX_CELL_NUM] �洢��ص�ѹ����  ��λmv
 *         INT8U * _LTC_PAGED comm_err_flag ͨ��ʧ�ܱ�־
 *�� �� ֵ:LTC_OK �����ɹ�,���� ʧ��
 *�޶���Ϣ:                                                    
 ******************************************************************************/
LtcResult read_ltc6804_cells_volt(INT8U ltc6804_num, INT16U  (*far cell_volt)[LTC6804_MAX_CELL_NUM], Ltc6804CommErrorByte * far comm_err,Ltc6804RegReadType type)
{
	INT16U tmp_16u = 0;
    INT8U current_ic = 0;
    INT16U tmp_pec = 0;
    INT16U cmd = 0;
    INT8U pos = 0;
    INT8U i = 0;
    INT16U  bits = 0;
     //�������
	if ((ltc6804_num > MAX_LTC6804_IC_NUM) || (ltc6804_num == 0))
	{
		ltc6804_num = MAX_LTC6804_IC_NUM;
	}

	if (cell_volt == NULL)
	{
		return LTC_PTR_NULL;
	}
	if (comm_err == NULL)
	{
		return LTC_PTR_NULL;
	}
	
	for(i=0;i<4;i++)
	{
        if(i==0) 
        {
            cmd =RDCVA; 
        }
        else if(i==1)
        {
            cmd =RDCVB;  
        }
        else if(i==2)
        {
            cmd =RDCVC; 
        }
        else
        {
            cmd =RDCVD; 
        }
	  pos = i*3;
      ltc6804_buffer_set(&ltc6804_reg_buffer[0][0], 0, MAX_LTC6804_IC_NUM * REG_WITH_PEC_LEN * sizeof(INT8U));
      (void)read_ltc6804_register(ltc6804_num, cmd, &ltc6804_reg_buffer[0][0]);
  	    
  	    
  	    for(current_ic = 0; current_ic < ltc6804_num; current_ic++) 
  	    {
            tmp_pec = pec15_calc(REG_LEN, &ltc6804_reg_buffer[current_ic][0]);
            tmp_16u = ((INT16U) ltc6804_reg_buffer[current_ic][6] << 8) + ltc6804_reg_buffer[current_ic][7];
            if(tmp_pec == tmp_16u) 
            {
                tmp_16u = ((INT16U) ltc6804_reg_buffer[current_ic][1] << 8) + ltc6804_reg_buffer[current_ic][0]; 
                cell_volt[current_ic][pos+0] = (tmp_16u+5) / 10;
                tmp_16u = ((INT16U) ltc6804_reg_buffer[current_ic][3] << 8) + ltc6804_reg_buffer[current_ic][2]; 
                cell_volt[current_ic][pos+1] = (tmp_16u+5) / 10;
                tmp_16u = ((INT16U) ltc6804_reg_buffer[current_ic][5] << 8) + ltc6804_reg_buffer[current_ic][4]; 
                cell_volt[current_ic][pos+2] = (tmp_16u+5) / 10;
            } 
            else 
            {            
                switch(type)
                {
                    case kReadCfg:
                         comm_err[current_ic].Bits.read_cfg_reg = 1;
                    break;
                    case kReadCV:
                         comm_err[current_ic].Bits.read_cv = 1;
                    break;
                    case kReadGpio:
                         comm_err[current_ic].Bits.read_gpio1 = 1;
                    break;
                    case kReadPUCV:
                         comm_err[current_ic].Bits.read_pucv = 1;
                    break;
                    case kReadPUDV:
                         comm_err[current_ic].Bits.read_pudv = 1;
                    break;
                    default:
                    break;
                }
            }
  	    }
  	    //OSTimeDly(2); //�ӻ�̫�࣬ռ��ʱ��ϳ�����������
	}
	//���䴦��
	for(current_ic = 0; current_ic < ltc6804_num; current_ic++) 
	{
	     INT8U index = 0;
	     bits = config_get(kBSU1VoltSampleNoUseBitIndex+current_ic);
	     if(bits!=0) 
	     {
	          for(i=0;i<LTC6804_MAX_CELL_NUM;i++)
	          {
	              if(!GET(bits,i)) 
	              {
	                cell_volt[current_ic][index++] =cell_volt[current_ic][i];      
	              }
	          
	          }
	          for(;index < LTC6804_MAX_CELL_NUM;index++)
	          {
	             cell_volt[current_ic][index] = 0;      
	          }
	     }
	}
    return LTC_OK;
 }

/*****************************************************************************
 *��������:read_ltc6804_aux_a_volt
 *��������:��ȡ����A������ѹ(gpio,��׼������ѹֵ)
 *��    ��:INT8U ltc6804_num оƬ��Ŀ
 *         INT8U AUX_CH ͨ��
 *         INT16U *volt ��ѹ����
 *         INT8U *comm_err_flag ͨ��ʧ�ܱ�־
 *�� �� ֵ:LTC_OK �����ɹ�,���� ʧ��
 *�޶���Ϣ:
 ******************************************************************************/
LtcResult read_ltc6804_aux_a_volt(INT8U ltc6804_num, INT16U  (*far volt)[PARAM_BSU_NUM_MAX], Ltc6804CommErrorByte * far comm_err)
{
	INT8U current_ic = 0;//ic���
	INT16U tmp_16u = 0; //��ʱ16λ����
	INT16U tmp_pec = 0; //PECУ��
	LtcResult result;
	//�������
	if ((ltc6804_num > MAX_LTC6804_IC_NUM) || (ltc6804_num == 0))
	{
	    ltc6804_num = MAX_LTC6804_IC_NUM;
	}

	if (volt == NULL)
	{
		return LTC_PTR_NULL;
	}
	if (comm_err == NULL)
	{
		return LTC_PTR_NULL;
	}
	//�������
	ltc6804_buffer_set(&ltc6804_reg_buffer[0][0], 0, MAX_LTC6804_IC_NUM * REG_WITH_PEC_LEN * sizeof(INT8U));
	//��ȡ����
	result = read_ltc6804_register(ltc6804_num, RDAUXA, &ltc6804_reg_buffer[0][0]);
	if (result != LTC_OK)
	{
		return result;
	}

	for (current_ic = 0; current_ic < ltc6804_num; current_ic++)
	{
		tmp_pec = pec15_calc(REG_LEN, &ltc6804_reg_buffer[current_ic][0]);
		tmp_16u = ((INT16U) ltc6804_reg_buffer[current_ic][6] << 8) + ltc6804_reg_buffer[current_ic][7];

		if (tmp_pec == tmp_16u)//У��ͨ��
		{
			comm_err[current_ic].Bits.read_gpio1 = 0;
			tmp_16u = ((INT16U) ltc6804_reg_buffer[current_ic][1] << 8) + ltc6804_reg_buffer[current_ic][0];
			volt[0][current_ic] = (tmp_16u+5) / 10;
	
			comm_err[current_ic].Bits.read_gpio2 = 0;
			tmp_16u = ((INT16U) ltc6804_reg_buffer[current_ic][3] << 8) + ltc6804_reg_buffer[current_ic][2];
			volt[1][current_ic] = (tmp_16u+5) / 10;
	
			comm_err[current_ic].Bits.read_gpio3 = 0;
			tmp_16u = ((INT16U) ltc6804_reg_buffer[current_ic][5] << 8) + ltc6804_reg_buffer[current_ic][4];
			volt[2][current_ic] = (tmp_16u+5) / 10;
		
		}
		else
		{ 
			comm_err[current_ic].Bits.read_gpio1 = 1;
			comm_err[current_ic].Bits.read_gpio2 = 1;
			comm_err[current_ic].Bits.read_gpio3 = 1;
		}
	}
	return LTC_OK;
}
/*****************************************************************************
 *��������:read_ltc6804_aux_b_volt
 *��������:��ȡ����B������ѹ(gpio,��׼������ѹֵ)
 *��    ��:INT8U ltc6804_num оƬ��Ŀ
 *         INT8U AUX_CH ͨ��
 *         INT16U *volt ��ѹ����
 *         INT8U *comm_err_flag ͨ��ʧ�ܱ�־
 *�� �� ֵ:LTC_OK �����ɹ�,���� ʧ��
 *�޶���Ϣ:
 ******************************************************************************/
LtcResult read_ltc6804_aux_b_volt(INT8U ltc6804_num, INT16U  (*far volt)[PARAM_BSU_NUM_MAX], Ltc6804CommErrorByte * far comm_err)
{
	INT8U current_ic = 0;//ic���
	INT16U tmp_16u = 0; //��ʱ16λ����
	INT16U tmp_pec = 0; //PECУ��
	LtcResult result;
	//�������
	if ((ltc6804_num > MAX_LTC6804_IC_NUM) || (ltc6804_num == 0))
	{
	    ltc6804_num = MAX_LTC6804_IC_NUM;
	}

	if (volt == NULL)
	{
		return LTC_PTR_NULL;
	}
	if (comm_err == NULL)
	{
		return LTC_PTR_NULL;
	}
	//�������
	ltc6804_buffer_set(&ltc6804_reg_buffer[0][0], 0, MAX_LTC6804_IC_NUM * REG_WITH_PEC_LEN * sizeof(INT8U));
	//��ȡ����
	result = read_ltc6804_register(ltc6804_num, RDAUXB, &ltc6804_reg_buffer[0][0]);
	if (result != LTC_OK)
	{
		return result;
	}

	for (current_ic = 0; current_ic < ltc6804_num; current_ic++)
	{
		tmp_pec = pec15_calc(REG_LEN, &ltc6804_reg_buffer[current_ic][0]);
		tmp_16u = ((INT16U) ltc6804_reg_buffer[current_ic][6] << 8) + ltc6804_reg_buffer[current_ic][7];

		if (tmp_pec == tmp_16u)//У��ͨ��
		{
			comm_err[current_ic].Bits.read_gpio4 = 0;
			tmp_16u = ((INT16U) ltc6804_reg_buffer[current_ic][1] << 8) + ltc6804_reg_buffer[current_ic][0];
			volt[0][current_ic] = (tmp_16u+5) / 10;
	
			comm_err[current_ic].Bits.read_gpio5 = 0;
			tmp_16u = ((INT16U) ltc6804_reg_buffer[current_ic][3] << 8) + ltc6804_reg_buffer[current_ic][2];
			volt[1][current_ic] = (tmp_16u+5) / 10;
		}
		else
		{ 
			comm_err[current_ic].Bits.read_gpio4 = 1;
			comm_err[current_ic].Bits.read_gpio5 = 1;
		}
	}
	return LTC_OK;
}

/*****************************************************************************
 *��������:write_cfg_reg_to_ltc6804
 *��������:д���üĴ���
 *��    ��:INT8U ltc6804_numоƬ��
 *�� �� ֵ:LTC_OK �����ɹ�,���� ʧ��
 *�޶���Ϣ:
 ******************************************************************************/
LtcResult write_cfg_reg_to_ltc6804(INT8U ltc6804_num,BOOLEAN balance_en)
{
	INT8U current_ic = 0;
	INT8U i = 0;
	INT8U index = 0;
	INT16U cfg_pec = 0;
	INT8U tx_data[4] = { 0 };//������������
	INT16U tmp_pec = 0;//PECУ��
	INT8U tmp_8u = 0;
	
	//�������
	if ((ltc6804_num > MAX_LTC6804_IC_NUM) || (ltc6804_num == 0))
	{
	    ltc6804_num = MAX_LTC6804_IC_NUM;
	}
	tx_data[0] = (INT8U) (WRCFG >> 8);
	tx_data[1] = (INT8U) WRCFG;
	tmp_pec = pec15_calc(2, tx_data);
	tx_data[2] = (INT8U) (tmp_pec >> 8);
	tx_data[3] = (INT8U) tmp_pec;	
	OSSchedLock();
	wakeup_iso_spi_idle();
	ltc6804_cs_pin_low(); //Ƭѡ
	(void)spi_write_array(&ltc6804_high_speed_spi_bus,4,tx_data);
	
	//�����һ����ʼд
	for (current_ic = ltc6804_num; current_ic > 0; current_ic--)
	{
		index = 0;
		if (index < REG_WITH_PEC_LEN)
			ltc6804_reg_buffer[current_ic - 1][index++] = (ltc6804_gpio[current_ic-1].value << 3) + (REFON << 2) + ADCOPT;
		if (index < REG_WITH_PEC_LEN)
			ltc6804_reg_buffer[current_ic - 1][index++] = 0x00;
		if (index < REG_WITH_PEC_LEN)
			ltc6804_reg_buffer[current_ic - 1][index++] = 0x00;
		if (index < REG_WITH_PEC_LEN)
			ltc6804_reg_buffer[current_ic - 1][index++] = 0x00;
		if (index < REG_WITH_PEC_LEN)
		{
		    if(balance_en)
		        tmp_8u = (INT8U)get_ltc6804_balance_ctrl_word(current_ic - 1);
		    else
		        tmp_8u = 0;
		    ltc6804_reg_buffer[current_ic - 1][index++] = tmp_8u;
		}
		if (index < REG_WITH_PEC_LEN)
		{
		    if(balance_en)
		        tmp_8u = (INT8U)((get_ltc6804_balance_ctrl_word(current_ic - 1)>>8)&0x0F);
		    else
		        tmp_8u = 0;
		    #if DCTO != 0
		    tmp_8u += (DCTO << 4);
		    #endif
		    ltc6804_reg_buffer[current_ic - 1][index++] = tmp_8u ;
		}
		cfg_pec = pec15_calc(REG_LEN, &ltc6804_reg_buffer[current_ic - 1][0]);

		if (index < REG_WITH_PEC_LEN)
			ltc6804_reg_buffer[current_ic - 1][index++] = (INT8U) (cfg_pec >> 8);
		if (index < REG_WITH_PEC_LEN)
			ltc6804_reg_buffer[current_ic - 1][index++] = (INT8U) cfg_pec;
		(void)spi_write_array(&ltc6804_high_speed_spi_bus,REG_WITH_PEC_LEN, &ltc6804_reg_buffer[current_ic - 1][0]);
	}
	ltc6804_cs_pin_high(); //ȡ��Ƭѡ
        OSSchedUnlock();
	return LTC_OK;
}

/*****************************************************************************
 *��������:send_balance_cmd_to_ltc6804
 *��������:������������
 *��    ��:ltc6804_numоƬ��
 *�� �� ֵ:LTC_OK �����ɹ�,���� ʧ��
 *�޶���Ϣ:
 ******************************************************************************/
LtcResult send_balance_cmd_to_ltc6804(INT8U ltc6804_num) 
{
    LtcResult result = LTC_OK;
	//�������
	if ((ltc6804_num > MAX_LTC6804_IC_NUM) || (ltc6804_num == 0))
	{
		ltc6804_num = MAX_LTC6804_IC_NUM;
	}
	result = write_cfg_reg_to_ltc6804(ltc6804_num,TRUE);//��������оƬ
    return result;
}
/*****************************************************************************
 *��������:send_ltc6804_stcomm_cmd
 *��������:����I2C/SPIͨ������(�㲥��ʽ)��ע����CS��ƽ����
 *��    ��:INT8U byte_cnt  �����ֽ���
 *�� �� ֵ:��
 *�޶���Ϣ:
 ******************************************************************************/
void send_ltc6804_stcomm_cmd(INT8U byte_cnt)
{
	INT8U tx_data[4] = { 0 };
	INT16U tmp_pec = 0;
	INT16U cmd = STCOMM;
	INT8U i = 0;
	INT8U data = 0;

    if(byte_cnt > 3)
       byte_cnt = 3;
    
	tx_data[0] = (INT8U) (cmd >> 8);
	tx_data[1] = (INT8U) (cmd);
	//����PEC
	tmp_pec = pec15_calc(2, tx_data);
	tx_data[2] = (INT8U) (tmp_pec >> 8);
	tx_data[3] = (INT8U) tmp_pec;
	//Ҫ���ǽ�Ƶ����,PCF8574��֧��100K,��ô6804���ͨ��Ƶ�ʲ�����200K
	(void)spi_reinit(&ltc6804_low_speed_spi_bus, 1);
	//SPI����
	 (void)spi_write_array(&ltc6804_low_speed_spi_bus,4,tx_data);;
	//��SCK�Ϸ���N��ʱ��
	for (i = 0; i < (byte_cnt*3); i++)
	{
		data = 0xFF;
        (void)spi_transmit_byte(&ltc6804_low_speed_spi_bus,&data);
	}
    (void)spi_reinit(&ltc6804_high_speed_spi_bus, 1);;
	//�ָ�Ƶ��
}
/*****************************************************************************
 *��������:ltc68041_iic_spi_read_write
 *��������:6804IIC��SPI��д
 *��    ��:ltc6804_num IC���� 
 *        bufferд����
 *        byte_cnt �����ֽ������3��
 *�� �� ֵ:LTC_OK �����ɹ�,���� ʧ��
 *�޶���Ϣ:
 ******************************************************************************/
LtcResult ltc68041_iic_spi_read_write(INT8U ltc6804_num, INT8U (*far buffer)[REG_WITH_PEC_LEN], Ltc6804IICSPIWRFlag w_r_flag,INT8U byte_cnt)
{
	LtcResult result;
	INT8U current_ic = 0;
	if ((ltc6804_num > MAX_LTC6804_IC_NUM) || (ltc6804_num == 0))
	{
    	ltc6804_num = MAX_LTC6804_IC_NUM;
	}
	if (buffer == NULL)
	{
		return LTC_PTR_NULL;
	}
    if(byte_cnt > 3)
    {
       return LTC_ERROR;
    }
    OSSchedLock();
	//��������
	wakeup_iso_spi_idle();

	ltc6804_cs_pin_low(); //Ƭѡ
	//����WRCOMM����
	send_ltc6804_wrcomm_cmd();
	//����COMM�Ĵ�������,�Ӷ���--->���׶�
	for (current_ic = ltc6804_num; current_ic > 0; current_ic--)
	{
		(void)spi_write_array(&ltc6804_high_speed_spi_bus,REG_WITH_PEC_LEN, &buffer[current_ic - 1][0]);
	}
	ltc6804_cs_pin_high(); //Ƭѡ

	//����STCOMM����
	ltc6804_cs_pin_low(); //Ƭѡ
	send_ltc6804_stcomm_cmd(byte_cnt);
	ltc6804_cs_pin_high(); //ȡ��Ƭѡ
    OSSchedUnlock();
	if (w_r_flag == kIICSPIWrite)
	{
		return LTC_OK;
	}
	ltc6804_buffer_set(&buffer[0][0], 0, MAX_LTC6804_IC_NUM * REG_WITH_PEC_LEN * sizeof(INT8U));
	result = read_ltc6804_register(ltc6804_num, RDCOMM, &buffer[0][0]);
	if (result != LTC_OK)
	{
		return result;
	}
	return LTC_OK;
}
/*****************************************************************************
 *��������:ltc6804_init
 *��������:LTC��ʼ��,��Ҫ��Ϊ��,ȷ������Ƶ�����,ͨ�������øߵ�ƽ.
 *��    ��:ltc6804_num 6804����
 *�� �� ֵ:LTC_OK �����ɹ�,���� ʧ��
 *�޶���Ϣ:
 ******************************************************************************/
LtcResult ltc6804_init(INT8U ltc6804_num)
{
	INT8U current_ic = 0;
	INT8U refon = 0;
	INT8U adcopt = 0;
	INT8U dcto = 0;
	INT16U tmp_pec = 0, tmp_16u = 0;
	volatile LtcResult result;

	//�������
	if ((ltc6804_num > MAX_LTC6804_IC_NUM) || (ltc6804_num == 0))
	{
		ltc6804_num = MAX_LTC6804_IC_NUM;
	}
	clr_all_ltc6804_balance_ctrl_word();
	//GPIO��ʼ��
	for (current_ic = 0; current_ic < MAX_LTC6804_IC_NUM; current_ic++)
	{   //��һ��IO��ȫ����ʼ��
	    bsu_board_gpio_init(current_ic);
	}
    wakeup_ltc6804_sleep(); //����ǰ��Ҫ��6804���л���
	result = write_cfg_reg_to_ltc6804(ltc6804_num,FALSE);//��������оƬ
	if (result != LTC_OK)
	{
		return result;
	}
	ltc6804_buffer_set(&ltc6804_reg_buffer[0][0], 0, MAX_LTC6804_IC_NUM * REG_WITH_PEC_LEN * sizeof(INT8U));

	result = read_ltc6804_register(ltc6804_num, RDCFG, &ltc6804_reg_buffer[0][0]);//�ض�ȡ���üĴ���
	if (result != LTC_OK)
	{
		return result;
	}

	for (current_ic = 0; current_ic < ltc6804_num; current_ic++)
	{
		tmp_pec = pec15_calc(REG_LEN, &ltc6804_reg_buffer[current_ic][0]);
		tmp_16u = ((INT16U) ltc6804_reg_buffer[current_ic][6] << 8) + ltc6804_reg_buffer[current_ic][7];

		if (tmp_pec == tmp_16u)//PECУ��
		{ //�Լ����ؼ���������ȷ��
			refon = (ltc6804_reg_buffer[current_ic][0] >> 2) & 0x01;
			adcopt = ltc6804_reg_buffer[current_ic][0] & 0x01;

			if ((refon != CFGR_REFON) || (adcopt != CFGR_ADCOPT))
			{
				return LTC_INIT_ERROR;
			}
		}
		else
		{ //У������ͨ�Ź���
			return LTC_INIT_ERROR;
		}
	}
	return LTC_OK;
}
/*****************************************************************************
 *��������:wakeup_ltc6804_with_write_cfg_reg
 *��������:ͨ���жϺ����»���6804
 *��    ��:ltc6804_num 6804����     27ms +0.32*n
 *�� �� ֵ:��
 *�޶���Ϣ:
 ******************************************************************************/
void wakeup_ltc6804_with_write_cfg_reg(INT8U ltc6804_num,BOOLEAN balance_en)
{
	//�������
	if ((ltc6804_num > MAX_LTC6804_IC_NUM) || (ltc6804_num == 0))
	{
		ltc6804_num = MAX_LTC6804_IC_NUM;
	}
	wakeup_ltc6804_sleep();
    (void)write_cfg_reg_to_ltc6804(ltc6804_num,balance_en);//��������оƬ
    OSTimeDly(2); //6804��׼�ϵ���Ҫʱ��
}
/*****************************************************************************
 *��������:read_balance_current_from_dsp
 *��������:ͨ��LTC6804��IIC��������
 *��    ��:ltc6804_num �����, addr DSP��ַ
 *�� �� ֵ:LTC_OK�ɹ� ,����ʧ�ܡ�
 *�޶���Ϣ:
 ******************************************************************************/
LtcResult read_balance_current_from_dsp(INT8U ltc6804_num,INT8U addr,INT8U *FAR balance_current)
{
	LtcResult result;
	INT8U current_ic = 0,cur = 0; //��ǰIC���
	INT16U tmp_pec = 0,tmp_16u = 0; //PECУ��

	if ((ltc6804_num > MAX_LTC6804_IC_NUM) || (ltc6804_num == 0))
		return LTC6804_NUM_OUT_OF_RANGE;
        addr += IIC_ADDR_READ_BIT;
	ltc6804_buffer_set(&ltc6804_reg_buffer[0][0], 0, MAX_LTC6804_IC_NUM * REG_WITH_PEC_LEN * sizeof(INT8U));
	for (current_ic = 0; current_ic < ltc6804_num; current_ic++)
	{
		ltc6804_reg_buffer[current_ic][0] = ((IIC_ICOM_READ_START << 4) & 0xF0) + ((addr >> 4) & 0x0F);
		ltc6804_reg_buffer[current_ic][1] = ((addr << 4) & 0xF0) + IIC_FCOM_READ_ACK_FROM_MIAN;
		ltc6804_reg_buffer[current_ic][2] = ((IIC_ICOM_READ_SDA_LOW << 4) & 0xF0) + 0x0F;
		ltc6804_reg_buffer[current_ic][3] = 0xF0+IIC_FCOM_WRITE_NACK_STOP;
		ltc6804_reg_buffer[current_ic][4] = ((IIC_ICOM_WRITE_NO_TRANSMIT << 4) & 0xF0);
		ltc6804_reg_buffer[current_ic][5] = IIC_FCOM_WRITE_NACK_STOP;
		tmp_pec = pec15_calc(6, &ltc6804_reg_buffer[current_ic][0]);
		ltc6804_reg_buffer[current_ic][6] = (INT8U) (tmp_pec >> 8);
		ltc6804_reg_buffer[current_ic][7] = (INT8U) (tmp_pec);
	}
	result = ltc68041_iic_spi_read_write(ltc6804_num, ltc6804_reg_buffer, kIICSPIRead, 2);
	if(result != LTC_OK) 
	{
	    return result;  
	}
	for (current_ic = 0; current_ic < ltc6804_num; current_ic++)
	{
		tmp_pec = pec15_calc(REG_LEN, &ltc6804_reg_buffer[current_ic][0]);
		tmp_16u = ((INT16U) ltc6804_reg_buffer[current_ic][6] << 8) + ltc6804_reg_buffer[current_ic][7];
		if (tmp_pec == tmp_16u)//PECУ��
		{
			cur = ((ltc6804_reg_buffer[current_ic][2] << 4) & 0xF0) + ((ltc6804_reg_buffer[current_ic][3] >> 4) & 0x0F);
			if(cur == 0xFF)
			   cur = 0;
			balance_current[current_ic] = cur;
		}
	}
	return LTC_OK;
}
/*****************************************************************************
 *��������:write_balance_info_to_dsp
 *��������:ͨ��LTC6804��IIC��дDSP
 *��    ��:ltc6804_num �����, addr DSP��ַ,active_balance_word��������� current�������
 *�� �� ֵ:LTC_OK�ɹ� ,����ʧ�ܡ�
 *�޶���Ϣ:
 ******************************************************************************/
LtcResult write_balance_info_to_dsp(INT8U ltc6804_num,INT8U addr,INT16U*far active_balance_word,INT8U current)
{
	LtcResult result;
	INT8U current_ic = 0; //��ǰIC���
	INT16U tmp_pec = 0; //PECУ��
	INT8U data = 0;
	INT8U i = 0;
	INT16U balance_word = 0;

	if ((ltc6804_num > MAX_LTC6804_IC_NUM) || (ltc6804_num == 0))
		return LTC6804_NUM_OUT_OF_RANGE;

	ltc6804_buffer_set(&ltc6804_reg_buffer[0][0], 0, MAX_LTC6804_IC_NUM * REG_WITH_PEC_LEN * sizeof(INT8U));
	for (current_ic = 0; current_ic < ltc6804_num; current_ic++)
	{
		data = 0;
		balance_word = balance_ctrl_word_convert(current_ic,active_balance_word[current_ic]);
		for(i = 0;i < LTC6804_MAX_CELL_NUM;i++)
		{
		    if(GET(balance_word,i))
		    {
		        data = i+1;
		        break;
		    }
		}
		if(GET(active_balance_word[current_ic],15))
		{
		    SET(data,4);
		}
		
		data |= ((current &0x07)<<5);
		ltc6804_reg_buffer[current_ic][0] = ((IIC_ICOM_WRITE_START << 4) & 0xF0) + ((addr >> 4) & 0x0F);
		ltc6804_reg_buffer[current_ic][1] = ((addr << 4) & 0xF0) + IIC_FCOM_WRITE_NACK;
		ltc6804_reg_buffer[current_ic][2] = ((IIC_ICOM_WRITE_BLANK << 4) & 0xF0) + ((data >> 4) & 0x0F);
		ltc6804_reg_buffer[current_ic][3] = ((data << 4) & 0xF0) + IIC_FCOM_WRITE_NACK_STOP;
		ltc6804_reg_buffer[current_ic][4] = ((IIC_ICOM_WRITE_NO_TRANSMIT << 4) & 0xF0);
		ltc6804_reg_buffer[current_ic][5] = IIC_FCOM_WRITE_NACK_STOP;
		tmp_pec = pec15_calc(6, &ltc6804_reg_buffer[current_ic][0]);
		ltc6804_reg_buffer[current_ic][6] = (INT8U) (tmp_pec >> 8);
		ltc6804_reg_buffer[current_ic][7] = (INT8U) (tmp_pec);
	}

	result = ltc68041_iic_spi_read_write(ltc6804_num, ltc6804_reg_buffer, kIICSPIWrite, 2);
	return result;

}
/*****************************************************************************
 *��������:pcf8574_write_channel
 *��������:ͨ��LTC6804��IIC��дPCF8574
 *��    ��:ltc6804_num 6804��Ŀ, channelͨ��
 *�� �� ֵ:LTC_OK�ɹ� ,����ʧ�ܡ�
 *�޶���Ϣ:
 ******************************************************************************/
LtcResult pcf8574_write_channel(INT8U ltc6804_num, INT8U channel)
{
	LtcResult result;
	INT8U current_ic = 0; //��ǰIC���
	INT16U tmp_pec = 0; //PECУ��
	INT8U addr_byte = 0; //��ַ
	INT8U cmd_byte = 0; //������
	INT8U pcf8574_addr = 0; //A0A1A2Ƭѡ
	
	if ((ltc6804_num > MAX_LTC6804_IC_NUM) || (ltc6804_num == 0))
		return LTC6804_NUM_OUT_OF_RANGE;
	if (channel > PCF8574_CHANNEL_MAX_NUM)
		return PCF8574_CHANNEL_ERROR;
	
	addr_byte = PCF8574_WRITE_ADDR | ((pcf8574_addr << 1) & 0x0E); //д��ַ
	cmd_byte = channel&0x07; //ͨ��ʹ��

	ltc6804_buffer_set(&ltc6804_reg_buffer[0][0], 0, MAX_LTC6804_IC_NUM * REG_WITH_PEC_LEN * sizeof(INT8U));
	for (current_ic = 0; current_ic < ltc6804_num; current_ic++)
	{
		ltc6804_reg_buffer[current_ic][0] = ((IIC_ICOM_WRITE_START << 4) & 0xF0) + ((addr_byte >> 4) & 0x0F);
		ltc6804_reg_buffer[current_ic][1] = ((addr_byte << 4) & 0xF0) + IIC_FCOM_WRITE_NACK;
		ltc6804_reg_buffer[current_ic][2] = ((IIC_ICOM_WRITE_BLANK << 4) & 0xF0) + ((cmd_byte >> 4) & 0x0F);
		ltc6804_reg_buffer[current_ic][3] = ((cmd_byte << 4) & 0xF0) + IIC_FCOM_WRITE_NACK_STOP;
		ltc6804_reg_buffer[current_ic][4] = ((IIC_ICOM_WRITE_NO_TRANSMIT << 4) & 0xF0);
		ltc6804_reg_buffer[current_ic][5] = IIC_FCOM_WRITE_NACK_STOP;
		tmp_pec = pec15_calc(6, &ltc6804_reg_buffer[current_ic][0]);
		ltc6804_reg_buffer[current_ic][6] = (INT8U) (tmp_pec >> 8);
		ltc6804_reg_buffer[current_ic][7] = (INT8U) (tmp_pec);
	}

	result = ltc68041_iic_spi_read_write(ltc6804_num, ltc6804_reg_buffer, kIICSPIWrite, 2);
	return result;

}

#endif