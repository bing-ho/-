/*******************************************************************************
 **                       ������������Դ�������޹�˾ Copyright (c)
 **                            http://www.ligoo.cn
 **�� �� ��:ems_adapter.c
 **��    ��:
 **��������:2017.7.12
 **�ļ�˵��:����Э�������ӿ�
 **�޸ļ�¼:
 **��    ��:V1.0
 **��    ע:
 *******************************************************************************/
#include "ems_adapter.h"


/*****************************************************************************
 *��������:get_bits_from_value
 *��������:��ȡvalue bit_posλ�õ�bit_lenλ
 *��    ��:value---������ֵ��bit_pos---��ʼλ�ã�bit_len---��ȡ����
 *�� �� ֵ:temp---��ȡ��ֵ
 *�޶���Ϣ:
 ******************************************************************************/
INT8U get_bits_from_value(INT32U value, INT8U bit_pos, INT8U bit_len)
{ 
    INT8U i, temp = 0; 
    for(i=0; i<bit_len; i++) 
        temp |= (INT8U)((value>>bit_pos) & (1<<i)); 
    return temp;
}


/*****************************************************************************
 *��������:write_data_with_bit_addr
 *��������:���ź�ֵ��������Ϣд��message������
 *��    ��:data---��Ϣ������msg_signal---�ź�������Ϣ��value---�ź�ֵ��
 *         max_len---��������󳤶�
 *�� �� ֵ:byte_pos---д���������λ��
 *�޶���Ϣ:
 ******************************************************************************/
INT8U write_data_with_bit_addr(INT8U *__far data, const SignalInMsg_t *__far msg_signal, INT32U value, INT8U max_len)
{
    INT8U i; 
    INT8U byte_pos = 0;  //��ʼ���λ��
    INT8U lt_lbit_pos = 0; //С��ģʽ���ֽ����λƫ��
    INT8U bt_lbit_pos = 0; //���ģʽ���ֽ����λƫ��
    INT8U bt_hbit_pos = 0; //���ģʽ���ֽ����λƫ��
    INT8U lb_bit_len = 0; //���ֽ�δ���λ����
    INT8U mb_byte_len = 0; //�м����ֽ���䳤��
    INT8U hb_bit_len = 0; //���ֽ����λ����
	
    if(msg_signal == NULL)
        return 0;	
	  
    byte_pos = msg_signal->start_bit >> 3;
	  
    if(byte_pos > max_len)
        return 0;	
		
    if(msg_signal->signal->byte_order == 0) //���ģʽ
    { 
        lb_bit_len = (msg_signal->start_bit & 0x07)+1;
	
        if(msg_signal->signal->signal_size > lb_bit_len) 
	  { 
	      mb_byte_len = (INT8U)(msg_signal->signal->signal_size - lb_bit_len) >> 3;
	      hb_bit_len = msg_signal->signal->signal_size - (mb_byte_len<<3) - lb_bit_len;
	  }
	  
	  if(msg_signal->signal->signal_size <= lb_bit_len) 
	  { 
	      bt_lbit_pos = lb_bit_len - msg_signal->signal->signal_size;
	      data[byte_pos] |= (get_bits_from_value(value, 0, msg_signal->signal->signal_size)<<bt_lbit_pos);
	  } 
	  else 
	  {
	      data[byte_pos] |= get_bits_from_value(value, (msg_signal->signal->signal_size - lb_bit_len), lb_bit_len); 
	  } 
		
	  for(i=1; i<=mb_byte_len; i++) 
	  { 
	      byte_pos++;
		data[byte_pos] = get_bits_from_value(value, (msg_signal->signal->signal_size - lb_bit_len - (i<<3)), 8); 
        } 
		
	  if(hb_bit_len > 0) 
	  { 
	      byte_pos++;
		bt_hbit_pos = 8 - hb_bit_len;
		data[byte_pos] |= (get_bits_from_value(value, 0, hb_bit_len)<<bt_hbit_pos);
	  }
    } 
    else //С��ģʽ
    { 	  
        lt_lbit_pos = msg_signal->start_bit & 0x07;
        lb_bit_len = 8 - lt_lbit_pos;
	
        if(msg_signal->signal->signal_size > lb_bit_len) 
	  { 
	      mb_byte_len = (INT8U)(msg_signal->signal->signal_size - lb_bit_len) >> 3;
	      hb_bit_len = msg_signal->signal->signal_size - (mb_byte_len<<3) - lb_bit_len;
	  }

	  if(msg_signal->signal->signal_size <= lb_bit_len) 
	  { 
            data[byte_pos] |= (get_bits_from_value(value, 0, msg_signal->signal->signal_size)<<lt_lbit_pos);
	  } 
	  else 
	  {
            data[byte_pos] |= (get_bits_from_value(value, 0, lb_bit_len)<<lt_lbit_pos);
	  }
		
	  for(i=0; i<mb_byte_len; i++)
	  {
	      byte_pos++;
		data[byte_pos] = get_bits_from_value(value, (lb_bit_len + (i<<3)), 8);
        }
        if(hb_bit_len > 0) 
        { 
	     byte_pos++;
	     data[byte_pos] |= get_bits_from_value(value, (lb_bit_len + (mb_byte_len<<3)), hb_bit_len);
        }
    } 
    return byte_pos;
}

/*****************************************************************************
 *��������:read_data_with_bit_addr
 *��������:�����źŵ���ʼλ�ͳ��ȴ�message��������ȡ�ź�ֵ
 *��    ��:data---��Ϣ������msg_signal---�ź�������Ϣ��
 *         max_len---��������󳤶�
 *�� �� ֵ:value---�ź�ֵ
 *�޶���Ϣ:
 ******************************************************************************/
INT32U read_data_with_bit_addr(INT8U *__far data, const SignalInMsg_t *__far msg_signal, INT8U max_len)
{
    INT32U value = 0;
    INT8U i; 
    INT8U byte_pos = 0;  //��ʼ���λ��
    INT8U lt_lbit_pos = 0; //С��ģʽ���ֽ����λƫ��
    INT8U bt_lbit_pos = 0; //���ģʽ���ֽ����λƫ��
    INT8U bt_hbit_pos = 0; //���ģʽ���ֽ����λƫ��
    INT8U lb_bit_len = 0; //���ֽ�δ���λ����
    INT8U mb_byte_len = 0; //�м����ֽ���䳤��
    INT8U hb_bit_len = 0; //���ֽ����λ����
	
    if(msg_signal == NULL)
        return 0;	
	  
    byte_pos = msg_signal->start_bit >> 3;
	  
    if(byte_pos > max_len)
        return 0;
		
    if(msg_signal->signal->byte_order == 0) //���ģʽ
    { 	  
        lb_bit_len = (msg_signal->start_bit & 0x07)+1;
        if(msg_signal->signal->signal_size > lb_bit_len) 
        { 
	      mb_byte_len = (INT8U)(msg_signal->signal->signal_size - lb_bit_len) >> 3;
	      hb_bit_len = msg_signal->signal->signal_size - (mb_byte_len<<3) - lb_bit_len;
	  }
	    
	  if(msg_signal->signal->signal_size <= lb_bit_len) 
	  { 
	      bt_lbit_pos = lb_bit_len - msg_signal->signal->signal_size;
	      value = get_bits_from_value(data[byte_pos], bt_lbit_pos, msg_signal->signal->signal_size);
	  } 
	  else 
	  {
	  	value = get_bits_from_value(data[byte_pos], 0, lb_bit_len); 
	  } 
		
        for(i=0; i<mb_byte_len; i++) 
        { 
            byte_pos++;
		value <<= 8;
		value |= data[byte_pos];
        } 
		
	  if(hb_bit_len > 0) 
	  { 
	      byte_pos++;
	      bt_hbit_pos = 8 - hb_bit_len;
	      value <<= hb_bit_len;
	      value |= get_bits_from_value(data[byte_pos], bt_hbit_pos, hb_bit_len);
	  }
    } 
    else //С��ģʽ
    {  
        lt_lbit_pos = msg_signal->start_bit & 0x07;
        lb_bit_len = 8 - lt_lbit_pos;
	
        if(msg_signal->signal->signal_size > lb_bit_len) 
	  { 
	      mb_byte_len = (INT8U)(msg_signal->signal->signal_size - lb_bit_len) >> 3;
	      hb_bit_len = msg_signal->signal->signal_size - (mb_byte_len<<3) - lb_bit_len;
	  }
	
	  if(msg_signal->signal->signal_size <= lb_bit_len) 
	  { 
	  	value = get_bits_from_value(data[byte_pos], lt_lbit_pos, msg_signal->signal->signal_size);
	  } 
	  else 
	  {
	      value = get_bits_from_value(data[byte_pos], lt_lbit_pos, lb_bit_len);
	  }
		
        for(i=0; i<mb_byte_len; i++)
	  {
	      byte_pos++;
		if(i==0) 
		{
		    value |= ((INT32U)data[byte_pos] << lb_bit_len);
		} 
		else 
		{ 
		    value |= ((INT32U)data[byte_pos] << 8);
		}
        }
	  if(hb_bit_len > 0) 
	  { 
            byte_pos++;
		value |= ((INT32U)get_bits_from_value(data[byte_pos], 0, hb_bit_len) << ((mb_byte_len<<3) + hb_bit_len));
	  }
    } 
    return value;
}


/*--------------------------------BMS_message_4-------------------------------------*/

/*****************************************************************************
 *��������:ems_get_precharge_relay_state
 *��������:��ȡԤ���̵���״̬ f_o_unit
 *��    ��:                                      
 *�� �� ֵ:Ԥ���̵���״̬
 *�޶���Ϣ:
 ******************************************************************************/
INT8U ems_get_precharge_relay_state(void)
{
  INT8U temp_value = 0;
  if (relay_control_is_on(kRelayTypePreCharging) == 1) 
  { 
    temp_value = 1;
  }
  return temp_value;
}
/*****************************************************************************
 *��������:ems_get_charge_relay_state
 *��������:��ȡ���̵���״̬ f_o_unit
 *��    ��:                                      
 *�� �� ֵ:���̵���״̬
 *�޶���Ϣ:
 ******************************************************************************/
INT8U ems_get_charge_relay_state(void)
{
 INT8U temp_value = 0;
 if ((relay_control_is_on(kRelayTypeCharging) == 1)  || (relay_control_is_on(kRelayTypeChargingSignal) == 1))
 { 
   temp_value = 1;
 }
 return temp_value;
}
/*****************************************************************************
 *��������:ems_get_discharge_relay_state
 *��������:��ȡ�ŵ�̵���״̬ f_o_unit
 *��    ��:                                      
 *�� �� ֵ:�ŵ�̵���״̬
 *�޶���Ϣ:
 ******************************************************************************/
INT8U ems_get_discharge_relay_state(void)
{
  INT8U temp_value = 0;
  if (relay_control_is_on(kRelayTypeDischarging) == 1) 
  { 
    temp_value = 1;
  }
  return temp_value;
}
/*****************************************************************************
 *��������:ems_get_SOH
 *��������:��ȡ�����SOH f_o_unit
 *��    ��:factor---�ֱ���[10.0(0.01%)/bit]��offset---ƫ����[0.0(0.01%)]                                      
 *�� �� ֵ:�����SOH
 *�޶���Ϣ:
 ******************************************************************************/
INT16U ems_get_SOH(float factor, float offset)
{
    INT32S temp_value = 0;
 /* INT32S raw_factor = 1; //�˴���дBMS�ź�ԭʼ���ȡ�   
    INT32S raw_offset = 0; //�˴���дBMS�ź�ԭʼƫ������*/

    temp_value = bcu_get_SOH(); 
    /*temp_value *= raw_factor;
    temp_value += raw_offset;*/
    temp_value = (INT32U)OFF_SET(temp_value, (INT32S)offset);
    temp_value = (INT32U)DIVISION(temp_value, (INT32S)factor); 

    return (INT16U)temp_value;
}
/*****************************************************************************
 *��������:ems_get_negative_insulation_resistance
 *��������:��ȡ������Ե��ֵ f_o_unit
 *��    ��:factor---�ֱ���[100.0(��)/bit]��offset---ƫ����[0.0(��)]                                      
 *�� �� ֵ:������Ե��ֵ
 *�޶���Ϣ:
 ******************************************************************************/
INT16U ems_get_negative_insulation_resistance(float factor, float offset)
{
    INT32S temp_value = 0;
    INT32S raw_factor = 100; //�˴���дBMS�ź�ԭʼ���ȡ�
   /* INT32S raw_offset = 0; //�˴���дBMS�ź�ԭʼƫ����*/

    temp_value = bcu_get_negative_insulation_resistance();
    temp_value *= raw_factor;
   /* temp_value += raw_offset;*/
    temp_value = (INT32U)OFF_SET(temp_value, (INT32S)offset);
    temp_value = (INT32U)DIVISION(temp_value, (INT32S)factor); 

    return (INT16U)temp_value;
}
/*--------------------------------BMS_message_3-------------------------------------*/

/*****************************************************************************
 *��������:ems_get_positive_insulation_resistance
 *��������:��ȡ������Ե��ֵ f_o_unit
 *��    ��:factor---�ֱ���[100.0(��)/bit]��offset---ƫ����[0.0(��)]                                      
 *�� �� ֵ:������Ե��ֵ
 *�޶���Ϣ:
 ******************************************************************************/
INT16U ems_get_positive_insulation_resistance(float factor, float offset)
{
    INT32S temp_value = 0;
    INT32S raw_factor = 100; //�˴���дBMS�ź�ԭʼ���ȡ�
   /* INT32S raw_offset = 0; //�˴���дBMS�ź�ԭʼƫ����*/

    temp_value = bcu_get_positive_insulation_resistance();
    temp_value *= raw_factor;
   /* temp_value += raw_offset;*/
    temp_value = (INT32U)OFF_SET(temp_value, (INT32S)offset);
    temp_value = (INT32U)DIVISION(temp_value, (INT32S)factor); 

    return (INT16U)temp_value;
}
/*****************************************************************************
 *��������:ems_get_low_temperature_id
 *��������:��ȡ��͵����¶ȴ��� f_o_unit
 *��    ��:                                      
 *�� �� ֵ:��͵����¶ȴ���
 *�޶���Ϣ:
 ******************************************************************************/
INT16U ems_get_low_temperature_id(void)
{
    INT16U temp_value = 0;
    temp_value = bcu_get_temperature_num_in_pack(bcu_get_low_temperature_id());
    return temp_value;
}
/*****************************************************************************
 *��������:ems_get_low_temperature
 *��������:��ȡ��͵����¶� f_o_unit
 *��    ��:factor---�ֱ���[1.0(��)/bit]��offset---ƫ����[-40.0(��)]                                      
 *�� �� ֵ:��͵����¶�
 *�޶���Ϣ:
 ******************************************************************************/
INT8U ems_get_low_temperature(float factor, float offset)
{
    INT32S temp_value = 0;
 /* INT32S raw_factor = 1; //�˴���дBMS�ź�ԭʼ���ȡ�*/
    INT32S raw_offset = -50; //�˴���дBMS�ź�ԭʼƫ����*�ֱ��ʡ�

    temp_value = bcu_get_low_temperature();
    /* temp_value *= raw_factor;*/
    temp_value += raw_offset;
    temp_value = (INT32U)OFF_SET(temp_value, (INT32S)offset);
    temp_value = (INT32U)DIVISION(temp_value, (INT32S)factor); 

    return (INT8U)temp_value;
}
/*****************************************************************************
 *��������:ems_get_high_temperature_id
 *��������:��ȡ��ߵ����¶ȴ��� f_o_unit
 *��    ��:                                      
 *�� �� ֵ:��ߵ����¶ȴ���
 *�޶���Ϣ:
 ******************************************************************************/
INT16U ems_get_high_temperature_id(void)
{
 INT16U temp_value = 0;
 temp_value = bcu_get_temperature_num_in_pack(bcu_get_high_temperature_id());
 return temp_value;
}
/*****************************************************************************
 *��������:ems_get_high_temperature
 *��������:��ȡ��ߵ����¶� f_o_unit
 *��    ��:factor---�ֱ���[1.0(��)/bit]��offset---ƫ����[-40.0(��)]                                      
 *�� �� ֵ:��ߵ����¶�
 *�޶���Ϣ:
 ******************************************************************************/
INT8U ems_get_high_temperature(float factor, float offset)
{
    INT32S temp_value = 0;
 /* INT32S raw_factor = 1; //�˴���дBMS�ź�ԭʼ���ȡ�*/
    INT32S raw_offset = -50; //�˴���дBMS�ź�ԭʼƫ����*�ֱ��ʡ�

    temp_value = bcu_get_high_temperature();
    /* temp_value *= raw_factor;*/
    temp_value += raw_offset;
    temp_value = (INT32U)OFF_SET(temp_value, (INT32S)offset);
    temp_value = (INT32U)DIVISION(temp_value, (INT32S)factor); 

    return (INT8U)temp_value;
}
/*--------------------------------BMS_message_2-------------------------------------*/

/*****************************************************************************
 *��������:ems_get_low_voltage_id
 *��������:��ȡ��͵����ѹ���� f_o_unit
 *��    ��:                                      
 *�� �� ֵ:��͵����ѹ����
 *�޶���Ϣ:
 ******************************************************************************/
INT16U ems_get_low_voltage_id(void)
{
    INT16U temp_value = 0;
    temp_value = bcu_get_bat_num_in_pack(bcu_get_low_voltage_id());
    return temp_value;
}
/*****************************************************************************
 *��������:ems_get_low_voltage
 *��������:��ȡ��͵����ѹ f_o_unit
 *��    ��:factor---�ֱ���[1.0(mV)/bit]��offset---ƫ����[0.0(mV)]                                      
 *�� �� ֵ:��͵����ѹ
 *�޶���Ϣ:
 ******************************************************************************/
INT16U ems_get_low_voltage(float factor, float offset)
{
    INT32S temp_value = 0;
 /* INT32S raw_factor = 1; //�˴���дBMS�ź�ԭʼ���ȡ�   
    INT32S raw_offset = 0; //�˴���дBMS�ź�ԭʼƫ������*/

    temp_value = VOLT_4_DISPLAY(bcu_get_low_voltage()); 
    /* temp_value *= raw_factor;
    temp_value += raw_offset;*/
    temp_value = (INT32U)OFF_SET(temp_value, (INT32S)offset);
    temp_value = (INT32U)DIVISION(temp_value, (INT32S)factor); 

    return (INT16U)temp_value;
}
/*****************************************************************************
 *��������:ems_get_high_voltage_id
 *��������:��ȡ��ߵ����ѹ���� f_o_unit
 *��    ��:                                      
 *�� �� ֵ:��ߵ����ѹ����
 *�޶���Ϣ:
 ******************************************************************************/
INT16U ems_get_high_voltage_id(void)
{
    INT16U temp_value = 0;
    temp_value = bcu_get_bat_num_in_pack(bcu_get_high_voltage_id());
    return temp_value;
}
/*****************************************************************************
 *��������:ems_get_high_voltage
 *��������:��ȡ��ߵ����ѹ f_o_unit
 *��    ��:factor---�ֱ���[1.0(mV)/bit]��offset---ƫ����[0.0(mV)]                                      
 *�� �� ֵ:��ߵ����ѹ
 *�޶���Ϣ:
 ******************************************************************************/
INT16U ems_get_high_voltage(float factor, float offset)
{
    INT32S temp_value = 0;
 /* INT32S raw_factor = 1; //�˴���дBMS�ź�ԭʼ���ȡ�   
    INT32S raw_offset = 0; //�˴���дBMS�ź�ԭʼƫ������*/

    temp_value = bcu_get_high_voltage(); 
    /* temp_value *= raw_factor;
    temp_value += raw_offset;*/
    temp_value = (INT32U)OFF_SET(temp_value, (INT32S)offset);
    temp_value = (INT32U)DIVISION(temp_value, (INT32S)factor); 

    return (INT16U)temp_value;
}
/*--------------------------------BMS_message_1-------------------------------------*/

/*****************************************************************************
 *��������:ems_get_battery_short
 *��������:��ȡ��·���� f_o_unit
 *��    ��:                                      
 *�� �� ֵ:��·����
 *�޶���Ϣ:
 ******************************************************************************/
INT8U ems_get_battery_short(void)
{ 
    INT8U temp_value = 0;
    return temp_value;
}
/*****************************************************************************
 *��������:ems_get_battery_temp_rise
 *��������:��ȡ�������� f_o_unit
 *��    ��:                                      
 *�� �� ֵ:��������
 *�޶���Ϣ:
 ******************************************************************************/
INT8U ems_get_battery_temp_rise(void)
{ 
    AlarmLevel temp_rise_state;
    temp_rise_state = bcu_get_temp_rising_state();
    return (temp_rise_state >= kAlarmSecondLevel) ? 1 : 0;
}
/*****************************************************************************
 *��������:ems_get_battery_ODV
 *��������:��ȡѹ����� f_o_unit
 *��    ��:                                      
 *�� �� ֵ:ѹ�����
 *�޶���Ϣ:
 ******************************************************************************/
INT8U ems_get_battery_ODV(void)
{ 
    AlarmLevel odv_state;
    odv_state = bcu_get_delta_voltage_state();
    return (odv_state >= kAlarmSecondLevel) ? 1 : 0;
}
/*****************************************************************************
 *��������:ems_get_battery_ODT
 *��������:��ȡ�²���� f_o_unit
 *��    ��:                                      
 *�� �� ֵ:�²����
 *�޶���Ϣ:
 ******************************************************************************/
INT8U ems_get_battery_ODT(void)
{ 
    AlarmLevel odt_state;
    odt_state = bcu_get_delta_temperature_state();
    return (odt_state >= kAlarmSecondLevel) ? 1 : 0;
}
/*****************************************************************************
 *��������:ems_get_battery_OLT
 *��������:��ȡ�¶ȹ��� f_o_unit
 *��    ��:                                      
 *�� �� ֵ:�¶ȹ���
 *�޶���Ϣ:
 ******************************************************************************/
INT8U ems_get_battery_OLT(void)
{ 
    AlarmLevel olt_state;
    olt_state = bcu_get_low_temperature_state();
    return (olt_state >= kAlarmSecondLevel) ? 1 : 0;
}
/*****************************************************************************
 *��������:ems_get_battery_OCC
 *��������:��ȡ������ f_o_unit
 *��    ��:                                      
 *�� �� ֵ:������
 *�޶���Ϣ:
 ******************************************************************************/
INT8U ems_get_battery_OCC(void)
{ 
    AlarmLevel occ_state;
    occ_state = bcu_get_chg_oc_state();
    return (occ_state >= kAlarmSecondLevel) ? 1 : 0;
}
/*****************************************************************************
 *��������:ems_get_battery_OLTV
 *��������:��ȡ��ѹ���� f_o_unit
 *��    ��:                                      
 *�� �� ֵ:��ѹ����
 *�޶���Ϣ:
 ******************************************************************************/
INT8U ems_get_battery_OLTV(void)
{ 
    AlarmLevel oltv_state;
    oltv_state = bcu_get_low_total_volt_state();
    return (oltv_state >= kAlarmSecondLevel) ? 1 : 0;
}
/*****************************************************************************
 *��������:ems_get_battery_OHTV
 *��������:��ȡ��ѹ���� f_o_unit
 *��    ��:                                      
 *�� �� ֵ:��ѹ����
 *�޶���Ϣ:
 ******************************************************************************/
INT8U ems_get_battery_OHTV(void)
{ 
    AlarmLevel ohtv_state;
    ohtv_state = bcu_get_high_total_volt_state();
    return (ohtv_state >= kAlarmSecondLevel) ? 1 : 0;
}
/*****************************************************************************
 *��������:ems_get_battery_comm_abort
 *��������:��ȡͨѶ�ж� f_o_unit
 *��    ��:                                      
 *�� �� ֵ:ͨѶ�ж�
 *�޶���Ϣ:
 ******************************************************************************/
INT8U ems_get_battery_comm_abort(void)
{
    AlarmLevel abort_state;
    abort_state = bcu_get_slave_communication_state();
    return (abort_state != kAlarmNone) ? 1 : 0;
}
/*****************************************************************************
 *��������:ems_get_battery_Uleak
 *��������:��ȡ��Ե���� f_o_unit
 *��    ��:                                      
 *�� �� ֵ:��Ե����
 *�޶���Ϣ:
 ******************************************************************************/
INT8U ems_get_battery_Uleak(void)
{ 
    AlarmLevel uleak_state;
    uleak_state = bcu_get_battery_insulation_state();
    return (uleak_state >= kAlarmSecondLevel) ? 1 : 0;
}
/*****************************************************************************
 *��������:ems_get_battery_Leak
 *��������:��ȡ��Ե�� f_o_unit
 *��    ��:                                      
 *�� �� ֵ:��Ե��
 *�޶���Ϣ:
 ******************************************************************************/
INT8U ems_get_battery_Leak(void)
{ 
    AlarmLevel leak_state;
    leak_state = bcu_get_battery_insulation_state();
    return (leak_state == kAlarmFirstLevel) ? 1 : 0;
}
/*****************************************************************************
 *��������:ems_get_battery_LSOC
 *��������:��ȡSOC���� f_o_unit
 *��    ��:                                      
 *�� �� ֵ:SOC����
 *�޶���Ϣ:
 ******************************************************************************/
INT8U ems_get_battery_LSOC(void)
{ 
    AlarmLevel lsoc_state;
    lsoc_state = bcu_get_low_soc_state();
    return (lsoc_state >= kAlarmSecondLevel) ? 1 : 0;
}
/*****************************************************************************
 *��������:ems_get_battery_ODC
 *��������:��ȡ�ŵ���� f_o_unit
 *��    ��:                                      
 *�� �� ֵ:�ŵ����
 *�޶���Ϣ:
 ******************************************************************************/
INT8U ems_get_battery_ODC(void)
{ 
    AlarmLevel odc_state;
    odc_state = bcu_get_dchg_oc_state();
    return (odc_state >= kAlarmSecondLevel) ? 1 : 0;
}
/*****************************************************************************
 *��������:ems_get_battery_OHT
 *��������:��ȡ������� f_o_unit
 *��    ��:                                      
 *�� �� ֵ:�������
 *�޶���Ϣ:
 ******************************************************************************/
INT8U ems_get_battery_OHT(void)
{ 
    AlarmLevel oht_state;
    oht_state = bcu_get_high_temperature_state();
    return (oht_state >= kAlarmSecondLevel) ? 1 : 0;
}
/*****************************************************************************
 *��������:ems_get_battery_OLV
 *��������:��ȡ������� f_o_unit
 *��    ��:                                      
 *�� �� ֵ:�������
 *�޶���Ϣ:
 ******************************************************************************/
INT8U ems_get_battery_OLV(void)
{ 
    AlarmLevel olv_state;
    olv_state = bcu_get_discharge_state();
    return (olv_state >= kAlarmSecondLevel) ? 1 : 0;
}
/*****************************************************************************
 *��������:ems_get_battery_OHV
 *��������:��ȡ������� f_o_unit
 *��    ��:                                      
 *�� �� ֵ:�������
 *�޶���Ϣ:
 ******************************************************************************/
INT8U ems_get_battery_OHV(void)
{ 
    AlarmLevel ohv_state;
    ohv_state = bcu_get_charge_state();
    return (ohv_state >= kAlarmSecondLevel) ? 1 : 0;
}
/*****************************************************************************
 *��������:ems_get_SOC
 *��������:��ȡ�����SOC f_o_unit
 *��    ��:factor---�ֱ���[10.0(0.01%)/bit]��offset---ƫ����[0.0(0.01%)]                                      
 *�� �� ֵ:�����SOC
 *�޶���Ϣ:
 ******************************************************************************/
INT16U ems_get_SOC(float factor, float offset)
{ 
    INT32S temp_value = 0;
 /* INT32S raw_factor = 1; //�˴���дBMS�ź�ԭʼ���ȡ�   
    INT32S raw_offset = 0; //�˴���дBMS�ź�ԭʼƫ������*/

    temp_value = bcu_get_SOC(); 
    /* temp_value *= raw_factor;
    temp_value += raw_offset;*/
    temp_value = (INT32U)OFF_SET(temp_value, (INT32S)offset);
    temp_value = (INT32U)DIVISION(temp_value, (INT32S)factor); 

    return (INT16U)temp_value;
}
/*****************************************************************************
 *��������:ems_get_total_current
 *��������:��ȡ������ܵ��������Ϊ�����ŵ�Ϊ���� f_o_unit
 *��    ��:factor---�ֱ���[1.0(0.1A)/bit]��offset---ƫ����[-32000.0(0.1A)]                                      
 *�� �� ֵ:������ܵ��������Ϊ�����ŵ�Ϊ����
 *�޶���Ϣ:
 ******************************************************************************/
INT16U ems_get_total_current(float factor, float offset)
{
    INT32S temp_value = 0;
 /* INT32S raw_factor = 1; //�˴���дBMS�ź�ԭʼ���ȡ�   
    INT32S raw_offset = 0; //�˴���дBMS�ź�ԭʼƫ������*/

    temp_value = -1*bcu_get_current(); 
    /* temp_value *= raw_factor;
    temp_value += raw_offset;*/
    temp_value = (INT32U)OFF_SET(temp_value, (INT32S)offset);
    temp_value = (INT32U)DIVISION(temp_value, (INT32S)factor); 

    return (INT16U)temp_value;  
}
/*****************************************************************************
 *��������:ems_get_total_voltage
 *��������:��ȡ�������ѹ f_o_unit
 *��    ��:factor---�ֱ���[100.0(mV)/bit]��offset---ƫ����[0.0(mV)]                                      
 *�� �� ֵ:�������ѹ
 *�޶���Ϣ:
 ******************************************************************************/
INT16U ems_get_total_voltage(float factor, float offset)
{
    INT32S temp_value = 0;
   /*INT32S raw_factor = 1; //�˴���дBMS�ź�ԭʼ���ȡ� 
    INT32S raw_offset = 0; //�˴���дBMS�ź�ԭʼƫ������*/

    temp_value = bcu_get_total_voltage(); 
    /* temp_value *= raw_factor;
    temp_value += raw_offset;*/
    temp_value = (INT32U)OFF_SET(temp_value, (INT32S)offset);
    temp_value = (INT32U)DIVISION(temp_value, (INT32S)factor); 
    return (INT16U)temp_value;  
}