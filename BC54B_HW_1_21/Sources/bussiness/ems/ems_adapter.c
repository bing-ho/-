/*******************************************************************************
 **                       安徽力高新能源技术有限公司 Copyright (c)
 **                            http://www.ligoo.cn
 **文 件 名:ems_adapter.c
 **作    者:
 **创建日期:2017.7.12
 **文件说明:整车协议适配层接口
 **修改记录:
 **版    本:V1.0
 **备    注:
 *******************************************************************************/
#include "ems_adapter.h"


/*****************************************************************************
 *函数名称:get_bits_from_value
 *函数功能:获取value bit_pos位置的bit_len位
 *参    数:value---输入数值，bit_pos---起始位置，bit_len---获取长度
 *返 回 值:temp---获取数值
 *修订信息:
 ******************************************************************************/
INT8U get_bits_from_value(INT32U value, INT8U bit_pos, INT8U bit_len)
{ 
    INT8U i, temp = 0; 
    for(i=0; i<bit_len; i++) 
        temp |= (INT8U)((value>>bit_pos) & (1<<i)); 
    return temp;
}


/*****************************************************************************
 *函数名称:write_data_with_bit_addr
 *函数功能:将信号值按描述信息写入message数据域
 *参    数:data---消息数据域，msg_signal---信号描述信息，value---信号值，
 *         max_len---数据域最大长度
 *返 回 值:byte_pos---写入数据域的位置
 *修订信息:
 ******************************************************************************/
INT8U write_data_with_bit_addr(INT8U *__far data, const SignalInMsg_t *__far msg_signal, INT32U value, INT8U max_len)
{
    INT8U i; 
    INT8U byte_pos = 0;  //起始填充位置
    INT8U lt_lbit_pos = 0; //小端模式低字节填充位偏移
    INT8U bt_lbit_pos = 0; //大端模式低字节填充位偏移
    INT8U bt_hbit_pos = 0; //大端模式高字节填充位偏移
    INT8U lb_bit_len = 0; //低字节未填充位长度
    INT8U mb_byte_len = 0; //中间整字节填充长度
    INT8U hb_bit_len = 0; //高字节填充位长度
	
    if(msg_signal == NULL)
        return 0;	
	  
    byte_pos = msg_signal->start_bit >> 3;
	  
    if(byte_pos > max_len)
        return 0;	
		
    if(msg_signal->signal->byte_order == 0) //大端模式
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
    else //小端模式
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
 *函数名称:read_data_with_bit_addr
 *函数功能:根据信号的起始位和长度从message数据域提取信号值
 *参    数:data---消息数据域，msg_signal---信号描述信息，
 *         max_len---数据域最大长度
 *返 回 值:value---信号值
 *修订信息:
 ******************************************************************************/
INT32U read_data_with_bit_addr(INT8U *__far data, const SignalInMsg_t *__far msg_signal, INT8U max_len)
{
    INT32U value = 0;
    INT8U i; 
    INT8U byte_pos = 0;  //起始填充位置
    INT8U lt_lbit_pos = 0; //小端模式低字节填充位偏移
    INT8U bt_lbit_pos = 0; //大端模式低字节填充位偏移
    INT8U bt_hbit_pos = 0; //大端模式高字节填充位偏移
    INT8U lb_bit_len = 0; //低字节未填充位长度
    INT8U mb_byte_len = 0; //中间整字节填充长度
    INT8U hb_bit_len = 0; //高字节填充位长度
	
    if(msg_signal == NULL)
        return 0;	
	  
    byte_pos = msg_signal->start_bit >> 3;
	  
    if(byte_pos > max_len)
        return 0;
		
    if(msg_signal->signal->byte_order == 0) //大端模式
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
    else //小端模式
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
 *函数名称:ems_get_precharge_relay_state
 *函数功能:获取预充电继电器状态 f_o_unit
 *参    数:                                      
 *返 回 值:预充电继电器状态
 *修订信息:
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
 *函数名称:ems_get_charge_relay_state
 *函数功能:获取充电继电器状态 f_o_unit
 *参    数:                                      
 *返 回 值:充电继电器状态
 *修订信息:
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
 *函数名称:ems_get_discharge_relay_state
 *函数功能:获取放电继电器状态 f_o_unit
 *参    数:                                      
 *返 回 值:放电继电器状态
 *修订信息:
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
 *函数名称:ems_get_SOH
 *函数功能:获取电池组SOH f_o_unit
 *参    数:factor---分辨率[10.0(0.01%)/bit]，offset---偏移量[0.0(0.01%)]                                      
 *返 回 值:电池组SOH
 *修订信息:
 ******************************************************************************/
INT16U ems_get_SOH(float factor, float offset)
{
    INT32S temp_value = 0;
 /* INT32S raw_factor = 1; //此处填写BMS信号原始精度。   
    INT32S raw_offset = 0; //此处填写BMS信号原始偏移量。*/

    temp_value = bcu_get_SOH(); 
    /*temp_value *= raw_factor;
    temp_value += raw_offset;*/
    temp_value = (INT32U)OFF_SET(temp_value, (INT32S)offset);
    temp_value = (INT32U)DIVISION(temp_value, (INT32S)factor); 

    return (INT16U)temp_value;
}
/*****************************************************************************
 *函数名称:ems_get_negative_insulation_resistance
 *函数功能:获取负极绝缘阻值 f_o_unit
 *参    数:factor---分辨率[100.0(Ω)/bit]，offset---偏移量[0.0(Ω)]                                      
 *返 回 值:负极绝缘阻值
 *修订信息:
 ******************************************************************************/
INT16U ems_get_negative_insulation_resistance(float factor, float offset)
{
    INT32S temp_value = 0;
    INT32S raw_factor = 100; //此处填写BMS信号原始精度。
   /* INT32S raw_offset = 0; //此处填写BMS信号原始偏移量*/

    temp_value = bcu_get_negative_insulation_resistance();
    temp_value *= raw_factor;
   /* temp_value += raw_offset;*/
    temp_value = (INT32U)OFF_SET(temp_value, (INT32S)offset);
    temp_value = (INT32U)DIVISION(temp_value, (INT32S)factor); 

    return (INT16U)temp_value;
}
/*--------------------------------BMS_message_3-------------------------------------*/

/*****************************************************************************
 *函数名称:ems_get_positive_insulation_resistance
 *函数功能:获取正极绝缘阻值 f_o_unit
 *参    数:factor---分辨率[100.0(Ω)/bit]，offset---偏移量[0.0(Ω)]                                      
 *返 回 值:正极绝缘阻值
 *修订信息:
 ******************************************************************************/
INT16U ems_get_positive_insulation_resistance(float factor, float offset)
{
    INT32S temp_value = 0;
    INT32S raw_factor = 100; //此处填写BMS信号原始精度。
   /* INT32S raw_offset = 0; //此处填写BMS信号原始偏移量*/

    temp_value = bcu_get_positive_insulation_resistance();
    temp_value *= raw_factor;
   /* temp_value += raw_offset;*/
    temp_value = (INT32U)OFF_SET(temp_value, (INT32S)offset);
    temp_value = (INT32U)DIVISION(temp_value, (INT32S)factor); 

    return (INT16U)temp_value;
}
/*****************************************************************************
 *函数名称:ems_get_low_temperature_id
 *函数功能:获取最低单体温度串号 f_o_unit
 *参    数:                                      
 *返 回 值:最低单体温度串号
 *修订信息:
 ******************************************************************************/
INT16U ems_get_low_temperature_id(void)
{
    INT16U temp_value = 0;
    temp_value = bcu_get_temperature_num_in_pack(bcu_get_low_temperature_id());
    return temp_value;
}
/*****************************************************************************
 *函数名称:ems_get_low_temperature
 *函数功能:获取最低单体温度 f_o_unit
 *参    数:factor---分辨率[1.0(℃)/bit]，offset---偏移量[-40.0(℃)]                                      
 *返 回 值:最低单体温度
 *修订信息:
 ******************************************************************************/
INT8U ems_get_low_temperature(float factor, float offset)
{
    INT32S temp_value = 0;
 /* INT32S raw_factor = 1; //此处填写BMS信号原始精度。*/
    INT32S raw_offset = -50; //此处填写BMS信号原始偏移量*分辨率。

    temp_value = bcu_get_low_temperature();
    /* temp_value *= raw_factor;*/
    temp_value += raw_offset;
    temp_value = (INT32U)OFF_SET(temp_value, (INT32S)offset);
    temp_value = (INT32U)DIVISION(temp_value, (INT32S)factor); 

    return (INT8U)temp_value;
}
/*****************************************************************************
 *函数名称:ems_get_high_temperature_id
 *函数功能:获取最高单体温度串号 f_o_unit
 *参    数:                                      
 *返 回 值:最高单体温度串号
 *修订信息:
 ******************************************************************************/
INT16U ems_get_high_temperature_id(void)
{
 INT16U temp_value = 0;
 temp_value = bcu_get_temperature_num_in_pack(bcu_get_high_temperature_id());
 return temp_value;
}
/*****************************************************************************
 *函数名称:ems_get_high_temperature
 *函数功能:获取最高单体温度 f_o_unit
 *参    数:factor---分辨率[1.0(℃)/bit]，offset---偏移量[-40.0(℃)]                                      
 *返 回 值:最高单体温度
 *修订信息:
 ******************************************************************************/
INT8U ems_get_high_temperature(float factor, float offset)
{
    INT32S temp_value = 0;
 /* INT32S raw_factor = 1; //此处填写BMS信号原始精度。*/
    INT32S raw_offset = -50; //此处填写BMS信号原始偏移量*分辨率。

    temp_value = bcu_get_high_temperature();
    /* temp_value *= raw_factor;*/
    temp_value += raw_offset;
    temp_value = (INT32U)OFF_SET(temp_value, (INT32S)offset);
    temp_value = (INT32U)DIVISION(temp_value, (INT32S)factor); 

    return (INT8U)temp_value;
}
/*--------------------------------BMS_message_2-------------------------------------*/

/*****************************************************************************
 *函数名称:ems_get_low_voltage_id
 *函数功能:获取最低单体电压串号 f_o_unit
 *参    数:                                      
 *返 回 值:最低单体电压串号
 *修订信息:
 ******************************************************************************/
INT16U ems_get_low_voltage_id(void)
{
    INT16U temp_value = 0;
    temp_value = bcu_get_bat_num_in_pack(bcu_get_low_voltage_id());
    return temp_value;
}
/*****************************************************************************
 *函数名称:ems_get_low_voltage
 *函数功能:获取最低单体电压 f_o_unit
 *参    数:factor---分辨率[1.0(mV)/bit]，offset---偏移量[0.0(mV)]                                      
 *返 回 值:最低单体电压
 *修订信息:
 ******************************************************************************/
INT16U ems_get_low_voltage(float factor, float offset)
{
    INT32S temp_value = 0;
 /* INT32S raw_factor = 1; //此处填写BMS信号原始精度。   
    INT32S raw_offset = 0; //此处填写BMS信号原始偏移量。*/

    temp_value = VOLT_4_DISPLAY(bcu_get_low_voltage()); 
    /* temp_value *= raw_factor;
    temp_value += raw_offset;*/
    temp_value = (INT32U)OFF_SET(temp_value, (INT32S)offset);
    temp_value = (INT32U)DIVISION(temp_value, (INT32S)factor); 

    return (INT16U)temp_value;
}
/*****************************************************************************
 *函数名称:ems_get_high_voltage_id
 *函数功能:获取最高单体电压串号 f_o_unit
 *参    数:                                      
 *返 回 值:最高单体电压串号
 *修订信息:
 ******************************************************************************/
INT16U ems_get_high_voltage_id(void)
{
    INT16U temp_value = 0;
    temp_value = bcu_get_bat_num_in_pack(bcu_get_high_voltage_id());
    return temp_value;
}
/*****************************************************************************
 *函数名称:ems_get_high_voltage
 *函数功能:获取最高单体电压 f_o_unit
 *参    数:factor---分辨率[1.0(mV)/bit]，offset---偏移量[0.0(mV)]                                      
 *返 回 值:最高单体电压
 *修订信息:
 ******************************************************************************/
INT16U ems_get_high_voltage(float factor, float offset)
{
    INT32S temp_value = 0;
 /* INT32S raw_factor = 1; //此处填写BMS信号原始精度。   
    INT32S raw_offset = 0; //此处填写BMS信号原始偏移量。*/

    temp_value = bcu_get_high_voltage(); 
    /* temp_value *= raw_factor;
    temp_value += raw_offset;*/
    temp_value = (INT32U)OFF_SET(temp_value, (INT32S)offset);
    temp_value = (INT32U)DIVISION(temp_value, (INT32S)factor); 

    return (INT16U)temp_value;
}
/*--------------------------------BMS_message_1-------------------------------------*/

/*****************************************************************************
 *函数名称:ems_get_battery_short
 *函数功能:获取短路保护 f_o_unit
 *参    数:                                      
 *返 回 值:短路保护
 *修订信息:
 ******************************************************************************/
INT8U ems_get_battery_short(void)
{ 
    INT8U temp_value = 0;
    return temp_value;
}
/*****************************************************************************
 *函数名称:ems_get_battery_temp_rise
 *函数功能:获取温升过快 f_o_unit
 *参    数:                                      
 *返 回 值:温升过快
 *修订信息:
 ******************************************************************************/
INT8U ems_get_battery_temp_rise(void)
{ 
    AlarmLevel temp_rise_state;
    temp_rise_state = bcu_get_temp_rising_state();
    return (temp_rise_state >= kAlarmSecondLevel) ? 1 : 0;
}
/*****************************************************************************
 *函数名称:ems_get_battery_ODV
 *函数功能:获取压差过大 f_o_unit
 *参    数:                                      
 *返 回 值:压差过大
 *修订信息:
 ******************************************************************************/
INT8U ems_get_battery_ODV(void)
{ 
    AlarmLevel odv_state;
    odv_state = bcu_get_delta_voltage_state();
    return (odv_state >= kAlarmSecondLevel) ? 1 : 0;
}
/*****************************************************************************
 *函数名称:ems_get_battery_ODT
 *函数功能:获取温差过大 f_o_unit
 *参    数:                                      
 *返 回 值:温差过大
 *修订信息:
 ******************************************************************************/
INT8U ems_get_battery_ODT(void)
{ 
    AlarmLevel odt_state;
    odt_state = bcu_get_delta_temperature_state();
    return (odt_state >= kAlarmSecondLevel) ? 1 : 0;
}
/*****************************************************************************
 *函数名称:ems_get_battery_OLT
 *函数功能:获取温度过低 f_o_unit
 *参    数:                                      
 *返 回 值:温度过低
 *修订信息:
 ******************************************************************************/
INT8U ems_get_battery_OLT(void)
{ 
    AlarmLevel olt_state;
    olt_state = bcu_get_low_temperature_state();
    return (olt_state >= kAlarmSecondLevel) ? 1 : 0;
}
/*****************************************************************************
 *函数名称:ems_get_battery_OCC
 *函数功能:获取充电过流 f_o_unit
 *参    数:                                      
 *返 回 值:充电过流
 *修订信息:
 ******************************************************************************/
INT8U ems_get_battery_OCC(void)
{ 
    AlarmLevel occ_state;
    occ_state = bcu_get_chg_oc_state();
    return (occ_state >= kAlarmSecondLevel) ? 1 : 0;
}
/*****************************************************************************
 *函数名称:ems_get_battery_OLTV
 *函数功能:获取总压过低 f_o_unit
 *参    数:                                      
 *返 回 值:总压过低
 *修订信息:
 ******************************************************************************/
INT8U ems_get_battery_OLTV(void)
{ 
    AlarmLevel oltv_state;
    oltv_state = bcu_get_low_total_volt_state();
    return (oltv_state >= kAlarmSecondLevel) ? 1 : 0;
}
/*****************************************************************************
 *函数名称:ems_get_battery_OHTV
 *函数功能:获取总压过高 f_o_unit
 *参    数:                                      
 *返 回 值:总压过高
 *修订信息:
 ******************************************************************************/
INT8U ems_get_battery_OHTV(void)
{ 
    AlarmLevel ohtv_state;
    ohtv_state = bcu_get_high_total_volt_state();
    return (ohtv_state >= kAlarmSecondLevel) ? 1 : 0;
}
/*****************************************************************************
 *函数名称:ems_get_battery_comm_abort
 *函数功能:获取通讯中断 f_o_unit
 *参    数:                                      
 *返 回 值:通讯中断
 *修订信息:
 ******************************************************************************/
INT8U ems_get_battery_comm_abort(void)
{
    AlarmLevel abort_state;
    abort_state = bcu_get_slave_communication_state();
    return (abort_state != kAlarmNone) ? 1 : 0;
}
/*****************************************************************************
 *函数名称:ems_get_battery_Uleak
 *函数功能:获取绝缘过低 f_o_unit
 *参    数:                                      
 *返 回 值:绝缘过低
 *修订信息:
 ******************************************************************************/
INT8U ems_get_battery_Uleak(void)
{ 
    AlarmLevel uleak_state;
    uleak_state = bcu_get_battery_insulation_state();
    return (uleak_state >= kAlarmSecondLevel) ? 1 : 0;
}
/*****************************************************************************
 *函数名称:ems_get_battery_Leak
 *函数功能:获取绝缘低 f_o_unit
 *参    数:                                      
 *返 回 值:绝缘低
 *修订信息:
 ******************************************************************************/
INT8U ems_get_battery_Leak(void)
{ 
    AlarmLevel leak_state;
    leak_state = bcu_get_battery_insulation_state();
    return (leak_state == kAlarmFirstLevel) ? 1 : 0;
}
/*****************************************************************************
 *函数名称:ems_get_battery_LSOC
 *函数功能:获取SOC过低 f_o_unit
 *参    数:                                      
 *返 回 值:SOC过低
 *修订信息:
 ******************************************************************************/
INT8U ems_get_battery_LSOC(void)
{ 
    AlarmLevel lsoc_state;
    lsoc_state = bcu_get_low_soc_state();
    return (lsoc_state >= kAlarmSecondLevel) ? 1 : 0;
}
/*****************************************************************************
 *函数名称:ems_get_battery_ODC
 *函数功能:获取放电过流 f_o_unit
 *参    数:                                      
 *返 回 值:放电过流
 *修订信息:
 ******************************************************************************/
INT8U ems_get_battery_ODC(void)
{ 
    AlarmLevel odc_state;
    odc_state = bcu_get_dchg_oc_state();
    return (odc_state >= kAlarmSecondLevel) ? 1 : 0;
}
/*****************************************************************************
 *函数名称:ems_get_battery_OHT
 *函数功能:获取单体过温 f_o_unit
 *参    数:                                      
 *返 回 值:单体过温
 *修订信息:
 ******************************************************************************/
INT8U ems_get_battery_OHT(void)
{ 
    AlarmLevel oht_state;
    oht_state = bcu_get_high_temperature_state();
    return (oht_state >= kAlarmSecondLevel) ? 1 : 0;
}
/*****************************************************************************
 *函数名称:ems_get_battery_OLV
 *函数功能:获取单体过放 f_o_unit
 *参    数:                                      
 *返 回 值:单体过放
 *修订信息:
 ******************************************************************************/
INT8U ems_get_battery_OLV(void)
{ 
    AlarmLevel olv_state;
    olv_state = bcu_get_discharge_state();
    return (olv_state >= kAlarmSecondLevel) ? 1 : 0;
}
/*****************************************************************************
 *函数名称:ems_get_battery_OHV
 *函数功能:获取单体过充 f_o_unit
 *参    数:                                      
 *返 回 值:单体过充
 *修订信息:
 ******************************************************************************/
INT8U ems_get_battery_OHV(void)
{ 
    AlarmLevel ohv_state;
    ohv_state = bcu_get_charge_state();
    return (ohv_state >= kAlarmSecondLevel) ? 1 : 0;
}
/*****************************************************************************
 *函数名称:ems_get_SOC
 *函数功能:获取电池组SOC f_o_unit
 *参    数:factor---分辨率[10.0(0.01%)/bit]，offset---偏移量[0.0(0.01%)]                                      
 *返 回 值:电池组SOC
 *修订信息:
 ******************************************************************************/
INT16U ems_get_SOC(float factor, float offset)
{ 
    INT32S temp_value = 0;
 /* INT32S raw_factor = 1; //此处填写BMS信号原始精度。   
    INT32S raw_offset = 0; //此处填写BMS信号原始偏移量。*/

    temp_value = bcu_get_SOC(); 
    /* temp_value *= raw_factor;
    temp_value += raw_offset;*/
    temp_value = (INT32U)OFF_SET(temp_value, (INT32S)offset);
    temp_value = (INT32U)DIVISION(temp_value, (INT32S)factor); 

    return (INT16U)temp_value;
}
/*****************************************************************************
 *函数名称:ems_get_total_current
 *函数功能:获取电池组总电流（充电为负，放电为正） f_o_unit
 *参    数:factor---分辨率[1.0(0.1A)/bit]，offset---偏移量[-32000.0(0.1A)]                                      
 *返 回 值:电池组总电流（充电为负，放电为正）
 *修订信息:
 ******************************************************************************/
INT16U ems_get_total_current(float factor, float offset)
{
    INT32S temp_value = 0;
 /* INT32S raw_factor = 1; //此处填写BMS信号原始精度。   
    INT32S raw_offset = 0; //此处填写BMS信号原始偏移量。*/

    temp_value = -1*bcu_get_current(); 
    /* temp_value *= raw_factor;
    temp_value += raw_offset;*/
    temp_value = (INT32U)OFF_SET(temp_value, (INT32S)offset);
    temp_value = (INT32U)DIVISION(temp_value, (INT32S)factor); 

    return (INT16U)temp_value;  
}
/*****************************************************************************
 *函数名称:ems_get_total_voltage
 *函数功能:获取电池组总压 f_o_unit
 *参    数:factor---分辨率[100.0(mV)/bit]，offset---偏移量[0.0(mV)]                                      
 *返 回 值:电池组总压
 *修订信息:
 ******************************************************************************/
INT16U ems_get_total_voltage(float factor, float offset)
{
    INT32S temp_value = 0;
   /*INT32S raw_factor = 1; //此处填写BMS信号原始精度。 
    INT32S raw_offset = 0; //此处填写BMS信号原始偏移量。*/

    temp_value = bcu_get_total_voltage(); 
    /* temp_value *= raw_factor;
    temp_value += raw_offset;*/
    temp_value = (INT32U)OFF_SET(temp_value, (INT32S)offset);
    temp_value = (INT32U)DIVISION(temp_value, (INT32S)factor); 
    return (INT16U)temp_value;  
}