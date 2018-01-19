
/*
 * bms_charger_gw.c
 *
 *  Created on: 2012-10-11
 *      Author: Administrator
 */

#include "includes.h"
#include "can_intermediate.h"
#include "bms_charger_gw.h"
#include "bms_charger_common.h"
#include "bms_buffer.h"
#include "bms_charger.h"
#include "bms_config.h"

#pragma MESSAGE DISABLE C1420 // Result of function-call is ignored
#pragma MESSAGE DISABLE C1860 // Pointer conversion: possible loss of data
#pragma MESSAGE DISABLE C4001 // Condition always FALSE
#pragma MESSAGE DISABLE C12056  // SP debug info incorrect because of optimization or inline assembler

#pragma DATA_SEG __RPAGE_SEG PAGED_RAM_CHARGER
GuoWangCharger g_guowang_charger = {NULL};
const ChargerDeviceInfo g_guowang_charger_info = { 0, 0, "Charger", "", "Guowang", "guowang" };
#pragma DATA_SEG DEFAULT

#define MAX_TEMP  255
#define MIN_TEMP  0

/********************************************
 * Function Area
 ********************************************/

void guowang_charger_uninit(void)
{
    can_uninit(g_guowang_charger.can_context);
    g_guowang_charger.can_context = NULL;
}

Result guowang_charger_on(void)
{
    int cpu_sr = 0;

    OS_ENTER_CRITICAL();
    ChargerParaIndex.ChgrSetCfg.ChargeOnOff = GUOWANG_CHARGER_ENABLE_CONTROL_BYTE;
    OS_EXIT_CRITICAL();

    return RES_OK;
}

Result guowang_charger_off(void)
{
    int cpu_sr = 0;

    OS_ENTER_CRITICAL();
    ChargerParaIndex.ChgrSetCfg.ChargeOnOff = GUOWANG_CHARGER_DISABLE_CONTROL_BYTE;
    OS_EXIT_CRITICAL();

    return RES_OK;
}

INT8U guowang_charger_is_connected(void)
{
    int cpu_sr = 0;
    INT8U res;

    OS_ENTER_CRITICAL();
    if(ChargerParaIndex.ChargerSts.IsCommunication)
        res = TRUE;
    else
        res = FALSE;
    OS_EXIT_CRITICAL();

    return res;
}

Result guowang_charger_is_communication()
{
    OS_CPU_SR cpu_sr = 0;
    BOOLEAN res;

    OS_ENTER_CRITICAL();
    if(ChargerParaIndex.ChargerSts.IsCommunication)
        res = TRUE;
    else
        res = FALSE;
    OS_EXIT_CRITICAL();
    return res;
}

BOOLEAN guowang_charger_is_charging(void)
{
    int cpu_sr = 0;
    BOOLEAN res;

    OS_ENTER_CRITICAL();
    if(ChargerParaIndex.ChargerSts.IsCharging && ChargerParaIndex.ChargerSts.IsCommunication)
        res = TRUE;
    else
        res = FALSE;
    OS_EXIT_CRITICAL();
    return res;
}

Result guowang_charger_get_charge_ready_status(void)
{
    return guowang_charger_is_connected();
}

Result guowang_charger_get_status(INT8U* status)
{
    int cpu_sr = 0;
    if (status == NULL) return ERR_INVALID_ARG;

    OS_ENTER_CRITICAL();
    *status = ChargerParaIndex.ChargerSts.ChgrExceptStatus[0];
    OS_EXIT_CRITICAL();

    return RES_OK;
}

INT16U guowang_charger_get_type(void* info)
{
    if(info == NULL)
        return RES_ERR;
    return g_guowang_charger_info.type;
}

Result guowang_charger_set_current(INT16U current)
{
    int cpu_sr = 0;

    OS_ENTER_CRITICAL();
    ChargerParaIndex.ChgrSetCfg.ChargeCur = current;
    OS_EXIT_CRITICAL();

    return RES_OK;
}

Result guowang_charger_get_current(INT16U* current)
{
    int cpu_sr = 0;
    if (current == NULL) return ERR_INVALID_ARG;

    OS_ENTER_CRITICAL();
    *current = ChargerParaIndex.ChgrSetCfg.ChargeCur;
    OS_EXIT_CRITICAL();

    return RES_OK;
}

Result guowang_charger_set_max_voltage(INT16U voltage)
{
    int cpu_sr = 0;

    OS_ENTER_CRITICAL();
    ChargerParaIndex.ChgrSetCfg.ChargeVolt = voltage;
    OS_EXIT_CRITICAL();

    return RES_OK;
}

Result guowang_charger_get_max_voltage(INT16U* voltage)
{
    int cpu_sr = 0;
    if (voltage == NULL) return ERR_INVALID_ARG;

    OS_ENTER_CRITICAL();
    *voltage = ChargerParaIndex.ChgrSetCfg.ChargeVolt;
    OS_EXIT_CRITICAL();

    return RES_OK;
}

Result guowang_charger_get_output_voltage(INT16U* voltage)
{
    int cpu_sr = 0;
    if (voltage == NULL) return ERR_INVALID_ARG;

    OS_ENTER_CRITICAL();
    *voltage = ChargerParaIndex.ChargerSts.OutputChgVolt;
    OS_EXIT_CRITICAL();

    return RES_OK;
}

Result guowang_charger_get_output_current(INT16U* current)
{
    int cpu_sr = 0;
    if (current == NULL) return ERR_INVALID_ARG;

    OS_ENTER_CRITICAL();
    *current = ChargerParaIndex.ChargerSts.OutputChgCur;
    OS_EXIT_CRITICAL();

    return RES_OK;
}

INT8U guowang_charger_charging_is_enable(void)
{
    if(ChargerParaIndex.ChgrSetCfg.ChargeOnOff == GUOWANG_CHARGER_ENABLE_CONTROL_BYTE) return TRUE;
    
    return FALSE;
}

void guowang_charger_task_tx_run(void* pdata)
{
	CanMessage msg;
	INT8U i,m,n,x,V_ADD,T_ADD;
    INT16U TXDat[4] ={0},temp,macro_temp;
    INT32U last_tick=0,diff;
    void* p_arg=pdata; /*prevent 'pdata' complier has no use*/


    for(;;)
    {
    	last_tick = get_tick_count();
    	OSTimeDly(5);
    	m=0;
		msg.len = 8;
		WRITE_LT_INT16U(msg.data, m, ChargerParaIndex.ChgrSetCfg.ChargeVolt);
		WRITE_LT_INT16U(msg.data, m, ChargerParaIndex.ChgrSetCfg.ChargeCur);
		if(ChargerParaIndex.ChgrSetCfg.ChargeOnOff==GUOWANG_CHARGER_DISABLE_CONTROL_BYTE)
			x = 2;
		else
			x = 8;
		WRITE_LT_INT8U(msg.data, m, x);
		WRITE_LT_INT8U(msg.data, m, config_get(kSlaveNumIndex));
		WRITE_LT_INT16U(msg.data, m, 0xFFFF);
		msg.id.value = can_id_from_extend_id(0x13CC16B2);
		can_send(g_guowang_charger.can_context, &msg);

		OSTimeDly(5);
		m=0;
		msg.len = 8;
		WRITE_LT_INT16U_WITH_FUNC_DATA(msg.data, m, MV_TO_V(bcu_get_total_voltage()));
		WRITE_LT_INT16U_WITH_FUNC_DATA(msg.data, m, bcu_get_current());
		WRITE_LT_INT8U(msg.data, m, SOC_TO_PERCENT(bcu_get_SOC()));
		WRITE_LT_INT8U(msg.data, m, 0);
		WRITE_LT_INT16U_WITH_FUNC_DATA(msg.data, m, config_get(kDChgHVFstAlarmIndex));
		msg.id.value = can_id_from_extend_id(0x104C1991);
		can_send(g_guowang_charger.can_context, &msg);

		OSTimeDly(5);
		m=0;
		msg.len = 8;
		WRITE_LT_INT16U_WITH_FUNC_DATA(msg.data, m, config_get(kDChgHVSndAlarmIndex));
		WRITE_LT_INT16U_WITH_FUNC_DATA(msg.data, m, config_get(kDChgLVFstAlarmIndex));
		WRITE_LT_INT16U_WITH_FUNC_DATA(msg.data, m, config_get(kDChgLVSndAlarmIndex));
		if(bcu_get_high_voltage() - VOLT_4_DISPLAY(bcu_get_low_voltage()) > config_get(kDChgDVFstAlarmIndex))
			x = 1;
		else
			x = 0;
		WRITE_LT_INT8U(msg.data, m, x);
		WRITE_LT_INT8U(msg.data, m, 0);
		msg.id.value = can_id_from_extend_id(0x104C1992);
		can_send(g_guowang_charger.can_context, &msg);

		OSTimeDly(5);
		m=0;
		msg.len = 8;
		temp = config_get(kDChgHTFstAlarmIndex);
		if(temp >= 10)
		    temp = temp - 10;
		else
		    temp = 0;
		WRITE_LT_INT16U(msg.data, m, temp);
		WRITE_LT_INT16U_WITH_FUNC_DATA(msg.data, m, config_get(kChargerCurIndex));
		WRITE_LT_INT16U_WITH_FUNC_DATA(msg.data, m, config_get(kDChgOCFstAlarmIndex));
		WRITE_LT_INT16U_WITH_FUNC_DATA(msg.data, m, DIVISION(config_get(kNominalCapIndex),10)); //config_get(kNominalCapIndex)
		msg.id.value = can_id_from_extend_id(0x104C1993);
		can_send(g_guowang_charger.can_context, &msg);

		OSTimeDly(5);
		m=0;
		msg.len = 8;
		temp = config_get(kSlaveNumIndex);
		diff = 0;
		if(temp <= 32) n = (INT8U)temp;
		else n = 32;
		for(i=0; i<n; i++)
		{
		    if(bmu_get_temperature_num(i)) diff |= (1UL << i);
		}
		WRITE_LT_INT32U(msg.data, m, diff);
		
		diff = 0;
		if(temp <= 64) n = (INT8U)temp;
		else n = 64;
	    for(i=32; i<n; i++)
        {
            if(bmu_get_temperature_num(i)) diff |= (1UL << (INT8U)(i-32));
        }
	    WRITE_LT_INT32U(msg.data, m, diff);
	    
		msg.id.value = can_id_from_extend_id(0x104C1997);
		can_send(g_guowang_charger.can_context, &msg);

		OSTimeDly(5);
		if(8<config_get(kSlaveNumIndex))
			n=8;
		else
			n=(INT8U)(config_get(kSlaveNumIndex));
		m=0;
		msg.len = 8;
		for(m=0; m<n; m++)
			SET_LT_INT8U(msg.data, m, bmu_get_voltage_num(m));
		for(m=n; m<8; m++)
			SET_LT_INT8U(msg.data, m, 0);
		msg.id.value = can_id_from_extend_id(0x104C1998);
		can_send(g_guowang_charger.can_context, &msg);

		OSTimeDly(5);
		temp = config_get(kSlaveNumIndex);
		for(i=1; i< ((temp+7)>>3); i++)
		{
		    if(8*i<temp)
    		{
    			if(temp>8*(i+1))
    				n=8*(i+1);
    			else
    				n=(INT8U)temp;
        		m=0;
        		msg.len = 8;
        		for(m=8*i; m<n; m++)
        			SET_LT_INT8U(msg.data, m-8*i, bmu_get_voltage_num(m));
        		for(m=n; m<8*(i+1); m++)
        			SET_LT_INT8U(msg.data, m-8*i, 0);
        		msg.id.value = can_id_from_extend_id(0x104C1999+i-1);
        		can_send(g_guowang_charger.can_context, &msg);
        		OSTimeDly(5);
    		}
    		else
    		    break;
		}
		V_ADD=0;
		x=0;
		temp = config_get(kSlaveNumIndex);
		for(m=0;m<temp;m++)
		{
			for(n=0;n<bmu_get_voltage_num(m);n++)
			{
				TXDat[x] = bmu_get_voltage_item(m, n);
				x++;
				if(((x==4)&&(V_ADD<100))||((n==bmu_get_voltage_num(m)-1)&&(m==config_get(kSlaveNumIndex)-1)))
				{
					x=0;
					msg.len = 8;
					WRITE_LT_INT16U(msg.data, x, TXDat[0]);
					WRITE_LT_INT16U(msg.data, x, TXDat[1]);
					WRITE_LT_INT16U(msg.data, x, TXDat[2]);
					WRITE_LT_INT16U(msg.data, x, TXDat[3]);
					x=0;
					msg.id.value = can_id_from_extend_id(0x104C19A4+V_ADD);
					can_send(g_guowang_charger.can_context, &msg);
					OSTimeDly(5);
					V_ADD++;
					TXDat[0] = 0;
					TXDat[1] = 0;
					TXDat[2] = 0;
					TXDat[3] = 0;
				}
				if(x >= 4) x = 0;
			}

		}
		x=0;
		V_ADD=0;

		T_ADD=0;
		temp = config_get(kSlaveNumIndex);
		for(m=0;m<temp;)
		{
			msg.len = 8;
			do
			{
    			macro_temp = bmu_get_max_temp(m);
    			if(macro_temp == 0)
    			{SET_LT_INT16U(msg.data, 0, 0);}
    			else
    			    SET_LT_INT16U_WITH_FUNC_DATA(msg.data, 0, TEMPERATURE_TO_C(macro_temp)+GUOWANG_TEMP_OFFSET);
    			SET_LT_INT16U(msg.data, 1, 0);

                if(m+1 >= temp)
                {
                    SET_LT_INT16U(msg.data, 2, 0xFFFF);
                    SET_LT_INT16U(msg.data, 4, 0xFFFF);
                    SET_LT_INT16U(msg.data, 6, 0xFFFF);
                    break;
                }

                macro_temp = bmu_get_max_temp(m+1);
    			if(macro_temp == 0)
    			{SET_LT_INT16U(msg.data, 2, 0);}
    			else
    			    SET_LT_INT16U_WITH_FUNC_DATA(msg.data, 2, TEMPERATURE_TO_C(macro_temp)+GUOWANG_TEMP_OFFSET);
    			SET_LT_INT16U(msg.data, 3, 0);

    			if(m+2 >= temp)
                {
                    SET_LT_INT16U(msg.data, 4, 0xFFFF);
                    SET_LT_INT16U(msg.data, 6, 0xFFFF);
                    break;
                }

    			macro_temp = bmu_get_max_temp(m+2);
    			if(macro_temp == 0)
    			{SET_LT_INT16U(msg.data, 4, 0);}
    			else
    			    SET_LT_INT16U_WITH_FUNC_DATA(msg.data, 4, TEMPERATURE_TO_C(macro_temp)+GUOWANG_TEMP_OFFSET);
    			SET_LT_INT16U(msg.data, 5, 0);
    			if(m+3 >= temp)
                {
                    SET_LT_INT16U(msg.data, 6, 0xFFFF);
                    break;
                }
   				macro_temp = bmu_get_max_temp(m+3);
    			if(macro_temp == 0)
    			{SET_LT_INT16U(msg.data, 6, 0);}
    			else
    			    SET_LT_INT16U_WITH_FUNC_DATA(msg.data, 6, TEMPERATURE_TO_C(macro_temp)+GUOWANG_TEMP_OFFSET);
   				SET_LT_INT16U(msg.data, 7, 0);
			}while(0);
    	    msg.id.value = can_id_from_extend_id(0x104C1A08+T_ADD);
			can_send(g_guowang_charger.can_context, &msg);
			T_ADD++;
			m=m+4;
			OSTimeDly(5);
		}
		T_ADD=0;
		temp = config_get(kSlaveNumIndex);
		for(m=0;m<temp;)
		{
			msg.len = 8;
			do
    		{
   				macro_temp = bmu_get_min_temp(m);
    			if(macro_temp == 0)
    			{SET_LT_INT16U(msg.data, 0, 0);}
    			else
    			    SET_LT_INT16U_WITH_FUNC_DATA(msg.data, 0, TEMPERATURE_TO_C(macro_temp)+GUOWANG_TEMP_OFFSET);
   				SET_LT_INT16U(msg.data, 1, 0);

                if(m+1 >= temp)
                {
                    SET_LT_INT16U(msg.data, 2, 0xFFFF);
                    SET_LT_INT16U(msg.data, 4, 0xFFFF);
                    SET_LT_INT16U(msg.data, 6, 0xFFFF);
                    break;
                }
    			macro_temp = bmu_get_min_temp(m+1);
    			if(macro_temp == 0)
    			{SET_LT_INT16U(msg.data, 2, 0);}
    			else
    			    SET_LT_INT16U_WITH_FUNC_DATA(msg.data, 2, TEMPERATURE_TO_C(macro_temp)+GUOWANG_TEMP_OFFSET);
    			SET_LT_INT16U(msg.data, 3, 0);

    			if(m+2 >= temp)
                {
                    SET_LT_INT16U(msg.data, 4, 0xFFFF);
                    SET_LT_INT16U(msg.data, 6, 0xFFFF);
                    break;
                }
    			macro_temp = bmu_get_min_temp(m+2);
    			if(macro_temp == 0)
    			{SET_LT_INT16U(msg.data, 4, 0);}
    			else
    			    SET_LT_INT16U_WITH_FUNC_DATA(msg.data, 4, TEMPERATURE_TO_C(macro_temp)+GUOWANG_TEMP_OFFSET);
    			SET_LT_INT16U(msg.data, 5, 0);

    			if(m+3 >= temp)
                {
                    SET_LT_INT16U(msg.data, 6, 0xFFFF);
                    break;
                }
   				macro_temp = bmu_get_min_temp(m+3);
    			if(macro_temp == 0)
    			{SET_LT_INT16U(msg.data, 6, 0);}
    			else
    			    SET_LT_INT16U_WITH_FUNC_DATA(msg.data, 6, TEMPERATURE_TO_C(macro_temp)+GUOWANG_TEMP_OFFSET);
   				SET_LT_INT16U(msg.data, 7, 0);
    		}while(0);
			msg.id.value = can_id_from_extend_id(0x104C1A18+T_ADD);
			can_send(g_guowang_charger.can_context, &msg);
			T_ADD++;
			m=m+4;
			OSTimeDly(5);
		}
		diff = get_interval_by_tick(last_tick, get_tick_count());
        if(diff < GUOWANG_MESSAGE_SEND_CYCLE)
            sleep(GUOWANG_MESSAGE_SEND_CYCLE - (INT16U)diff);
        else
            sleep(5);
    }
}

void guowang_charger_task_rx_run(void* pdata)
{
    OS_CPU_SR cpu_sr = 0;
    INT8U err = 0, flag;
    INT16U temp;
    CanMessage g_charger_message;
    void* p_arg=pdata; /*prevent 'pdata' complier has no use*/

    for(;;)
    {
        if(can_receive(g_guowang_charger.can_context, &g_charger_message, 5000) == ERR_TIMEOUT)
        {
            ChargerParaIndex.ChargerSts.OutputChgVolt = 0;
            ChargerParaIndex.ChargerSts.OutputChgCur = 0;
            ChargerParaIndex.ChargerSts.IsCommunication = 0;
            ChargerParaIndex.ChargerSts.IsCharging = FALSE;
            //charger_clear_all_except_status();
            continue;
        }

        switch(can_id_to_extend_id(g_charger_message.id.value))
        { //判断报文ID,并根据不同ID做处理
        case GUOWANG_CHARGER_BMS_RECEIVE_ID:
        	ChargerParaIndex.ChargerSts.OutputChgVolt=((INT16U)g_charger_message.data[1]<<8)
					  +g_charger_message.data[0];
        	ChargerParaIndex.ChargerSts.OutputChgCur=((INT16U)g_charger_message.data[3]<<8)
						+g_charger_message.data[2];
        	temp = ((INT16U)g_charger_message.data[5]<<8) + g_charger_message.data[4];

        	flag = temp & 0x01;
            if(flag)
                charger_clear_charge_except_status_with_num(CHARGER_RUN_STATUS_NUM);
            else
                charger_set_charge_except_status_with_num(CHARGER_RUN_STATUS_NUM, 0x01);

            flag = (temp & 0x02) >> 1;
            if(flag)
                charger_set_charge_except_status_with_num(CHARGER_HIGH_VOLT_ALARM_NUM, flag);
            else
                charger_clear_charge_except_status_with_num(CHARGER_HIGH_VOLT_ALARM_NUM);

            flag = (temp & 0x04) >> 2;
            if(flag)
                charger_set_charge_except_status_with_num(CHARGER_HIGH_CURRENT_ALARM_NUM, flag);
            else
                charger_clear_charge_except_status_with_num(CHARGER_HIGH_CURRENT_ALARM_NUM);

            flag = (temp & 0x08) >> 3;
            if(flag)
                charger_set_charge_except_status_with_num(CHARGER_OVER_TEMPERATURE_EXCEPT_NUM, flag);
            else
                charger_clear_charge_except_status_with_num(CHARGER_OVER_TEMPERATURE_EXCEPT_NUM);

        	flag = (temp & 0x10) >> 4;
            if(flag)
                charger_set_charge_except_status_with_num(CHARGER_CHARGE_SPOT_ENERGENCY_STOP_NUM, flag);
            else
                charger_clear_charge_except_status_with_num(CHARGER_CHARGE_SPOT_ENERGENCY_STOP_NUM);

            flag = (temp & 0x20) >> 5;
            if(flag)
                charger_set_charge_except_status_with_num(CHARGER_EMERGENCY_STOP_NUM, flag);
            else
                charger_clear_charge_except_status_with_num(CHARGER_EMERGENCY_STOP_NUM);

            flag = (temp & 0x40) >> 6;
            if(flag)
                charger_set_charge_except_status_with_num(CHARGER_BATTERY_REVERSE_CONNECT_NUM, flag);
            else
                charger_clear_charge_except_status_with_num(CHARGER_BATTERY_REVERSE_CONNECT_NUM);

            flag = (temp & 0x80) >> 7;
            if(flag)
                charger_set_charge_except_status_with_num(CHARGER_PHASE_LACK_EXCEPT_NUM, flag);
            else
                charger_clear_charge_except_status_with_num(CHARGER_PHASE_LACK_EXCEPT_NUM);

            flag = (temp & 0x100) >> 8;
            if(flag)
                charger_set_charge_except_status_with_num(CHARGER_BMS_STOP_CHARGE_EXCEPT_NUM, flag);
            else
                charger_clear_charge_except_status_with_num(CHARGER_BMS_STOP_CHARGE_EXCEPT_NUM);

            flag = (temp & 0x200) >> 9;
            if(flag)
                charger_set_charge_except_status_with_num(CHARGER_OVER_PROTECT_VOLT_NUM, flag);
            else
                charger_clear_charge_except_status_with_num(CHARGER_OVER_PROTECT_VOLT_NUM);

            flag = (temp & 0x400) >> 10;
            if(flag)
                charger_set_charge_except_status_with_num(CHARGER_OVER_PROTECT_CURR_NUM, flag);
            else
                charger_clear_charge_except_status_with_num(CHARGER_OVER_PROTECT_CURR_NUM);

            flag = (temp & 0x800) >> 11;
            if(flag)
                charger_set_charge_except_status_with_num(CHARGER_OVER_NOMINAL_VOLT_NUM, flag);
            else
                charger_clear_charge_except_status_with_num(CHARGER_OVER_NOMINAL_VOLT_NUM);

            flag = (temp & 0x1000) >> 12;
            if(flag)
                charger_set_charge_except_status_with_num(CHARGER_OVER_NOMINAL_CURR_NUM, flag);
            else
                charger_clear_charge_except_status_with_num(CHARGER_OVER_NOMINAL_CURR_NUM);

            flag = (temp & 0x2000) >> 13;
            if(flag)
                charger_set_charge_except_status_with_num(CHARGER_COMM_ABORT_WITH_BMS_NUM, flag);
            else
                charger_clear_charge_except_status_with_num(CHARGER_COMM_ABORT_WITH_BMS_NUM);

            flag = (temp & 0x4000) >> 14;
            if(flag)
                charger_set_charge_except_status_with_num(CHARGER_COMM_ABORT_WITH_CHARGE_SPOT_NUM, flag);
            else
                charger_clear_charge_except_status_with_num(CHARGER_COMM_ABORT_WITH_CHARGE_SPOT_NUM);

            flag = !((temp & 0x8000) >> 15);
            if(flag)
                charger_set_charge_except_status_with_num(CHARGER_CHAGER_CONNECTOR_EXCEPT_NUM, flag);
            else
                charger_clear_charge_except_status_with_num(CHARGER_CHAGER_CONNECTOR_EXCEPT_NUM);

        	ChargerParaIndex.ChargerSts.IsCommunication = TRUE;
        	break;
        default:
            break;

        }
        if(ChargerParaIndex.ChargerSts.OutputChgCur)
            ChargerParaIndex.ChargerSts.IsCharging = TRUE;
        else
            ChargerParaIndex.ChargerSts.IsCharging = FALSE;
    }
}

void guowang_charger_task_create(void)
{
    OSTaskCreate(guowang_charger_task_tx_run, (void *) NULL,
            (OS_STK *) &g_charger_tx_task_stack[CHARGER_TX_STK_SIZE - 1], CHARGER_TX_TASK_PRIO);
    OSTaskCreate(guowang_charger_task_rx_run, (void *) NULL,
            (OS_STK *) &g_charger_rx_task_stack[CHARGER_RX_STK_SIZE - 1], CHARGER_RX_TASK_PRIO);
}

Result guowang_charger_on_config_changing(ConfigIndex index, INT16U new_value)
{
    switch(index)
    {
    case kChargerCurIndex:
        return charger_set_current(new_value);
        break;
    case kChargerVoltIndex:
        return charger_set_voltage(new_value);
        break;
    }
    return RES_OK;
}

void guowang_charger_init(void)
{
    CanInfo charger_can_info;

    ChargerFuction[GUO_WANG_PROTOCOL].ChargerInit = &guowang_charger_init;
    ChargerFuction[GUO_WANG_PROTOCOL].ChargerUninit = &guowang_charger_uninit;
    ChargerFuction[GUO_WANG_PROTOCOL].ChargeOn = &guowang_charger_on;
    ChargerFuction[GUO_WANG_PROTOCOL].ChargeOff = &guowang_charger_off;
    ChargerFuction[GUO_WANG_PROTOCOL].ChargeIsOn = &guowang_charger_is_charging;
    ChargerFuction[GUO_WANG_PROTOCOL].ChargerIsConnected = &guowang_charger_is_connected;
    ChargerFuction[GUO_WANG_PROTOCOL].GetChargerStatus = &guowang_charger_get_status;
    ChargerFuction[GUO_WANG_PROTOCOL].GetChargerType = &guowang_charger_get_type;
    ChargerFuction[GUO_WANG_PROTOCOL].SetChargeVolt = &guowang_charger_set_max_voltage;
    ChargerFuction[GUO_WANG_PROTOCOL].SetChargeCur = &guowang_charger_set_current;
    ChargerFuction[GUO_WANG_PROTOCOL].GetChargeVolt = &guowang_charger_get_max_voltage;
    ChargerFuction[GUO_WANG_PROTOCOL].GetChargeCur = &guowang_charger_get_current;
    ChargerFuction[GUO_WANG_PROTOCOL].ChargerGetOutputVolt = &guowang_charger_get_output_voltage;
    ChargerFuction[GUO_WANG_PROTOCOL].ChargerGetOutputCur = &guowang_charger_get_output_current;
    ChargerFuction[GUO_WANG_PROTOCOL].ChargerGetChargeReadyStatus = &guowang_charger_get_charge_ready_status;
    ChargerFuction[GUO_WANG_PROTOCOL].ChargerChargingIsEnable = &guowang_charger_charging_is_enable;
    ChargerFuction[GUO_WANG_PROTOCOL].ChargerIsCommunication = &guowang_charger_is_communication;
    
    guowang_charger_set_max_voltage(config_get(kChargerVoltIndex));
    guowang_charger_set_current(config_get(kChargerCurIndex));

    BMS_CHARGER_ENABLE_DEFAULT ? guowang_charger_on() : guowang_charger_off();

    memset(&charger_can_info, 0, sizeof(CanInfo));
    charger_can_info.dev = (INT8U)config_get(kChargerCanChannelIndex);
    if(can_channel_is_valid(charger_can_info.dev) == FALSE)
	{
	    charger_can_info.dev = BMS_CHARGER_CAN_DEV;
	    bcu_reset_all_can_channel();
	}
    if(charger_can_info.dev == kCanDev3) charger_can_info.bps = can_get_bps((INT8U)config_get(kCAN5BpsIndex));
    else charger_can_info.bps = can_get_bps((INT8U)config_get(kCAN1BpsIndex + charger_can_info.dev));
	
    charger_can_info.receive_id = GUOWANG_CHARGER_BMS_RECEIVE_ID;
    charger_can_info.mask_id = GUOWANG_CHARGER_BMS_RECEIVE_ID_MASK;
    charger_can_info.mode = GUOWANG_CHARGER_CAN_MODE;
    charger_can_info.buffers = g_charger_can_buffers;
    charger_can_info.buffer_count = CHARGER_CAN_BUFFER_COUNT;
    charger_can_info.filtemode=CanFilte32;
    g_guowang_charger.can_context = can_init(&charger_can_info);

    config_register_observer(kChargerCurIndex, kChargerVoltIndex,guowang_charger_on_config_changing);
    relay_control_set_enable_flag(kRelayTypeCharging, TRUE);
    guowang_charger_task_create();
}


