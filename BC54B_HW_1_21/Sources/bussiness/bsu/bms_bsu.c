/*******************************************************************************
 **                       ������������Դ�������޹�˾ Copyright (c)
 **                            http://www...
 **�� �� ��:bms_bsu.c
 **��������:2015.10.15
 **�ļ�˵��:��ѹ�¶ȸ��µ�BCU��
 **�޸ļ�¼:
 **��    ��:V1.0
 **��    ע:
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
//���ɼ���������еĲɼ����е�λ��
static INT8U main_bsu_pos[BMU_MAX_SLAVE_COUNT + 1] = {0xFF};
static INT8U bmu_max_temp_from_bsu[BMU_MAX_SLAVE_COUNT] = {0};
static INT8U bmu_min_temp_from_bsu[BMU_MAX_SLAVE_COUNT] = {0};
void bsu_update_bcu_info(void* data);

/*****************************************************************************
 *��������:bmu_set_max_temp_by_bsu
 *��������:����bmu�ӻ�����¶�
 *��    ��:INT8U slave_index�ӻ���,INT8U temp�¶�  ƫ����-50��
 *�� �� ֵ:��
 *�޶���Ϣ:
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
 *��������:bmu_get_max_temp_from_bsu
 *��������:��ȡ����¶�
 *��    ��:INT8U slave_index�ӻ���,
 *�� �� ֵ:INT8U temp�¶�  ƫ����-50��
 *�޶���Ϣ:
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
 *��������:bmu_set_min_temp_by_bsu
 *��������:��������¶�
 *��    ��:INT8U slave_index�ӻ���,INT8U temp�¶�  ƫ����-50��
 *�� �� ֵ:��
 *�޶���Ϣ:
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
 *��������:bmu_get_min_temp_from_bsu
 *��������:��ȡ����¶�
 *��    ��:INT8U slave_index�ӻ���,
 *�� �� ֵ:INT8U temp�¶�  ƫ����-50��
 *�޶���Ϣ:
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
 *��������:bsu_init
 *��������:�ӻ�bsu��������
 *��    ��:��
 *�� �� ֵ:��
 *�޶���Ϣ:
 ******************************************************************************/
void bsu_init(void)
{
	INT8U index = 0;
	if (config_get(kBCUCommModeIndex) == BMS_COMM_SLAVE_MODE) //�ж��ǲ������Ӵ���һ��ģʽ
		return;
    main_bsu_relay_control_load_config();
	BSUVoltTempSampleTask_Init(); //��������
	#if  BMS_SUPPORT_BY5248D == 0   //��������̵�������
	job_schedule(MAIN_JOB_GROUP, 500, bsu_cool_analyse, NULL);
	job_schedule(MAIN_JOB_GROUP, 500, bsu_heat_analyse, NULL);
	#endif
	job_schedule(MAIN_JOB_GROUP, 200, bsu_update_bcu_info, NULL);
}
/*****************************************************************************
 *��������:main_bsu_pos_set
 *��������:���ݰ��������������ɼ���λ��
 *��    ��:��
 *�� �� ֵ:��
 *�޶���Ϣ:
 ******************************************************************************/
void main_bsu_pos_set(void)
{
	INT8U bsu_num = 0; //�ӻ���
	INT8U bsu_index = 0;
	INT8U slave_index = 0;
	bsu_num = get_bsu_num(); //��ȡ�ӻ���
	switch(config_get(kBYUBalTypeIndex) )
	{
    	case BSU_PASSIVE_BALANCE_TYPE:   
	for (bsu_index = 0; bsu_index < bsu_num; bsu_index++)
	{//��һ��һ��Ϊ���ӻ�
		if ((bsu_index == 0) || (get_bsu_n_board_type(bsu_index) == kBoardMain) || //�����ӻ�
        	      ((slave_index > 0) && ((main_bsu_pos[slave_index - 1] + BMU_SUPPORT_BSU_MAX_NUM) <= bsu_index))) //�ӻ����ɼ���Ϊ5��
		{
			if (slave_index < BMU_MAX_SLAVE_COUNT) //�ӻ����25��
			{
				main_bsu_pos[slave_index] = bsu_index;
				slave_index++;
			}
		}
	}
    	break;
    	case BSU_ACTIVE_BALANCE_TYPE: //��������ÿ���ɼ������Ϊһ���ӻ�
        	for (bsu_index = 0; bsu_index < bsu_num; bsu_index++)
        	{
            	if (slave_index < BMU_MAX_SLAVE_COUNT) //�ӻ����25��
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
 *��������:get_main_bsu_pos
 *��������:��ȡ���ӻ���λ��
 *��    ��:INT8U slave_index�ӻ���
 *�� �� ֵ:�����ɼ��������вɼ����е�λ��
 *�޶���Ϣ:
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
 *��������:bsu_update_bcu_volt_info
 *��������:�ɼ���BSU���µ�ص�ѹ��Ϣ��BCU
 *��    ��:��
 *�� �� ֵ:��
 *�޶���Ϣ:
 ******************************************************************************/
void bsu_update_bcu_volt_info(void)
{
	INT8U bsu_index = 0; //�ɼ�������
	INT8U bsu_n_cells_num = 0;  //��N���ɼ�������
	INT8U index = 0;
	INT8U cell_index = 0;
	INT8U bsu_num = 0; //�ɼ�����
	INT8U slave_index = 0;
	INT8U slave_num = 0; //�ӻ�����
	INT8U slave_cell_num = 0; //�ӻ������
	INT16U volt = 0; //��ѹ
	BOOLEAN comm_err_flag = 0; //ͨ�ű�־

	clr_bsu_passive_balance_multi_volt_item(); //���������Ҫ�Ķ�·��ߵ�ѹ��Ϣ
	clr_balance_min_volt_item();
	slave_num = (INT8U) config_get(kSlaveNumIndex); //��ȡ�ӻ���
	bsu_num = get_bsu_num(); //�ɼ�����Ŀ
	for (slave_index = 0; slave_index < slave_num; slave_index++)
	{
		cell_index = 0;
		slave_cell_num = 0;
		for (bsu_index = main_bsu_pos[slave_index]; bsu_index < main_bsu_pos[slave_index + 1]; bsu_index++)
		{
			if (bsu_index >= bsu_num) //�쳣
			{
				break;
			}
			comm_err_flag = is_bsu_n_comm_error(bsu_index);
			bsu_n_cells_num = get_bsu_n_cells_num(bsu_index);
			slave_cell_num += bsu_n_cells_num;
			for (index = 0; index < bsu_n_cells_num; index++) //��ߡ���͵�ѹɨ��,��ѹ����
			{
				volt = get_bsu_cells_volt(bsu_index, index);

				if (comm_err_flag == TRUE) //ͨ���ж�
				{
					bmu_set_voltage_item(slave_index, cell_index, 0);
				}
				else
				{
					bmu_set_voltage_item(slave_index, cell_index, volt);
				}
				cell_index++;
				bsu_statistic_multi_max_volt(bsu_index, index, volt); //���¾�����Ҫ�Ķ�·��ߵ�ѹ��Ϣ
				bsu_statistic_min_volt(bsu_index, index, volt);
			}
		}
		bmu_set_voltage_num(slave_index, slave_cell_num); //���õ����
	}
}
/*****************************************************************************
 *��������:bsu_update_bcu_temp_info
 *��������:�ӻ�BSU���µ���¶���Ϣ��BCU
 *��    ��:��
 *�� �� ֵ:��
 *�޶���Ϣ:
 ******************************************************************************/
void bsu_update_bcu_temp_info(void)
{
	INT8U bsu_index = 0, index = 0;
	INT8U bsu_num = 0;  //�ɼ�����
	INT8U temp = 0;  //�¶�
	INT8U offset = 0;
	INT8U temp_index = 0;
	INT8U bsu_n_temp_num = 0; //�ɼ����¸���
	INT8U slave_index = 0;
	INT8U slave_num = 0;
	INT8U slave_temp_num = 0;
	INT8U HT = 0;
    INT8U LT = TEMP_CABLE_SHORT_VAULE;
	BOOLEAN comm_err_flag = 0; //ͨ�ű�־

	slave_num = (INT8U) config_get(kSlaveNumIndex); //��ȡ�ӻ���
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
			 &&(get_bsu_n_board_type(bsu_index) == kBoardMain)) //���ɼ���Ϊ2���¶�
			{
				if (bsu_n_temp_num > 2)
					bsu_n_temp_num = 2;
				start_index = 2; //��ʼ��ַΪ2
				end_index = 2 + bsu_n_temp_num; //������ַ
			}
			else
			{
				start_index = 0; //��ʼ��ַΪ0
				end_index = bsu_n_temp_num; //������ַ
			}
		#else //����6804
		    if(config_get(kBYUBalTypeIndex)== BSU_PASSIVE_BALANCE_TYPE)//�����ӻ���һ���ɼ����¶�ƫ�Ʋ�һ��
            {
                if(bsu_index < BMS_SUPPORT_LTC6804_ON_BOARD_CHIP_NUM)
                {
		    if (bsu_n_temp_num > 4)
					bsu_n_temp_num = 4;
				start_index = 0; //��ʼ��ַΪ0
				end_index = bsu_n_temp_num; //������ַ
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
		
			for (index = start_index; index < end_index; index++) //��ߡ���͵�ѹɨ��,��ѹ����
			{
				temp = get_bsu_cells_temp(bsu_index, index);
				if (comm_err_flag == TRUE)  //ͨ���쳣
				{
					temp = 0;
				}
				else
				{
					if ((temp == TEMP_CABLE_SHORT_VAULE) || (temp == TEMP_CABLE_OPEN_VAULE)) //�¶�Ϊ��·���߶�·���
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
 *��������:bsu_update_bcu_balance_state
 *��������:��������Ϣ���µ�BCU��
 *��    ��:��
 *�� �� ֵ:��
 *�޶���Ϣ:
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
	slave_num = (INT8U) config_get(kSlaveNumIndex); //��ȡ�ӻ���
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
					SET(new_value, (slave_bit_index & 0x07));//8λ��λΪ0-7 ��ռ3λ0x07
				}
				slave_bit_index++;
				if ((slave_bit_index & 0x07) == 0)//�Ѿ���8λ����new_value���㲢���������ݸ��µ�bcu��
				{   //
				    tmp_8U = (INT8U)((slave_bit_index - 1) >>3);//����8 
					bmu_set_balance_state(slave_index, tmp_8U, new_value);
					new_value = 0;
				}
			}
		}
		//δ��8λ��Ҫ����
		if ((slave_bit_index & 0x07) != 0)
		{
			tmp_8U = (slave_bit_index / 8);
			bmu_set_balance_state(slave_index, tmp_8U, new_value);
			new_value = 0;
		}
	}

}
/*****************************************************************************
 *��������:is_bmu_comm_error_use_bsu
 *��������:BMU�ӻ��Ƿ�ͨѶ�ж�
 *��    ��:��
 *�� �� ֵ:��
 *�޶���Ϣ:
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
 *��������:bsu_update_bmu_relay_state
 *��������:���̵�����Ϣ���µ�BMU��
 *��    ��:��
 *�� �� ֵ:��
 *�޶���Ϣ:
 ******************************************************************************/
void bsu_update_bmu_relay_state(void)
{
	INT8U slave_index = 0;
    INT8U slave_num = 0;
    INT8U tmp_8u = 0;
    INT8U bsu_pos = 0;
    
	slave_num = (INT8U) config_get(kSlaveNumIndex); //��ȡ�ӻ���

        for (slave_index = 0; slave_index < slave_num; slave_index++)
    	{
    	     bsu_pos = get_main_bsu_pos(slave_index);//��ȡ���ɼ���ı��
	     
            if (bsu_pos == 0xFF)
        		continue;
        
            if(get_bsu_relay1_status(bsu_pos) == kBsuRelayOff)
            {
                set_ltc6804_gpio2_pin_high(bsu_pos); //��Ӳ��
            }
            else
            {
                set_ltc6804_gpio2_pin_low(bsu_pos); //��Ӳ��
            }
            if(get_bsu_relay2_status(bsu_pos) == kBsuRelayOff)
            {
                set_ltc6804_gpio1_pin_high(bsu_pos); //��Ӳ��
            }
            else
            {
                set_ltc6804_gpio1_pin_low(bsu_pos); //��Ӳ��
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
 *��������:bsu_update_bmu_relay_state
 *��������:���̵�����Ϣ���µ�BMU��
 *��    ��:��
 *�� �� ֵ:��
 *�޶���Ϣ:
 ******************************************************************************/
void bsu_update_bmu_balance_current(void)
{
	INT8U slave_index = 0;
    INT8U slave_num = 0;
    INT8U tmp_8u = 0;
    INT8U bsu_pos = 0;
    
	slave_num = (INT8U) config_get(kSlaveNumIndex); //��ȡ�ӻ���

    for (slave_index = 0; slave_index < slave_num; slave_index++)
	{
	    bsu_pos = get_main_bsu_pos(slave_index);//��ȡ���ɼ���ı��
     
        if (bsu_pos == 0xFF)
    		continue;
        tmp_8u = get_bsu_active_balance_current(bsu_pos);
        bmu_set_balance_current_item(slave_index,0,tmp_8u);
	}
}
/*****************************************************************************
 *��������:bsu_update_bcu_info
 *��������:��BSU��Ϣ���µ�BCU��
 *��    ��:��
 *�� �� ֵ:��
 *�޶���Ϣ:
 ******************************************************************************/
void bsu_update_bcu_info(void* data)
{
    (void)data;
    if(get_bsu_sample_once_finish_flag() == TRUE)
    {
        bsu_update_bcu_balance_state();
    #if  BMS_SUPPORT_BY5248D == 0 //��������̵�������
        bsu_update_bmu_relay_state();  
    #endif
        if(config_get(kBYUBalTypeIndex) == BSU_ACTIVE_BALANCE_TYPE)
        {
            bsu_update_bmu_balance_current();
        }
    }
}

#endif
