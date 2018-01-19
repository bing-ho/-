/*******************************************************************************
 **                       ������������Դ�������޹�˾ Copyright (c)
 **                            http://www.ligoo.cn
 **�� �� ��:detect_ems_message.c
 **��    ��:
 **��������:2017.11.1
 **�ļ�˵��:�ͻ�Э����Ϣ�ӿ�
 **�޸ļ�¼:
 **��    ��:V1.0
 **��    ע:
 *******************************************************************************/
#include "detect_ems_message.h"
#include "detect_ems_adapter.h"
#include "ems_message.h"
#include "ems_adapter.h"
#include "logic_ctrl.h"

#if( SWITCH_ON == UDS_SWITCH_CONFIG)
#include "Dtc_code.h"
#include "CanIf_Cbk.h"  // for bootloader
#include "CanIf.h"      // for bootloader
#include "CanTp.h"      // for uds app
#include "Dcm.h"        // for uds app
#include "can_hardware.h"  // for uds app
#endif



/**********��Ϣ�����ṹ�����鶨�壨����DBC��ȡ��*********/	                      		            



const EMS_Message_t __far detect_ems_msg_BMS_message_4 = {1, 0, 100, 0x1804FFF4,
                                              {
                                                { &sg_detect_precharge_relay_state, 34},
                                                { &sg_detect_charge_relay_state, 33},
                                                { &sg_detect_discharge_relay_state, 32},
                                                { &sg_detect_SOH, 16},
                                                { &sg_detect_negative_insulation_resistance, 0},
                                                {NULL,0},
                                                {NULL,0},
                                                {NULL,0},
                                                {NULL,0},
                                                {NULL,0},
                                                {NULL,0},
                                                {NULL,0},
                                                {NULL,0},
                                                {NULL,0},
                                                {NULL,0},
                                                {NULL,0},
                                                {NULL,0},
                                                {NULL,0},
                                                {NULL,0},

                                               }
                                             };
                                                       

const EMS_Message_t __far detect_ems_msg_BMS_message_3 = {1, 0, 100, 0x1803FFF4,
                                              {
                                                { &sg_detect_positive_insulation_resistance, 48},
                                                { &sg_detect_low_temperature_id, 32},
                                                { &sg_detect_low_temperature, 24},
                                                { &sg_detect_high_temperature_id, 8},
                                                { &sg_detect_high_temperature, 0},
                                                {NULL,0},
                                                {NULL,0},
                                                {NULL,0},
                                                {NULL,0},
                                                {NULL,0},
                                                {NULL,0},
                                                {NULL,0},
                                                {NULL,0},
                                                {NULL,0},
                                                {NULL,0},
                                                {NULL,0},
                                                {NULL,0},
                                                {NULL,0},
                                                {NULL,0},

                                               }
                                             };
                                                       

const EMS_Message_t __far detect_ems_msg_BMS_message_2 = {1, 0, 100, 0x1802FFF4,
                                              {
                                                { &sg_detect_low_voltage_id, 48},
                                                { &sg_detect_low_voltage, 32},
                                                { &sg_detect_high_voltage_id, 16},
                                                { &sg_detect_high_voltage, 0},
                                                {NULL,0},
                                                {NULL,0},
                                                {NULL,0},
                                                {NULL,0},
                                                {NULL,0},
                                                {NULL,0},
                                                {NULL,0},
                                                {NULL,0},
                                                {NULL,0},
                                                {NULL,0},
                                                {NULL,0},
                                                {NULL,0},
                                                {NULL,0},
                                                {NULL,0},
                                                {NULL,0},

                                               }
                                             };
                                                       

const EMS_Message_t __far detect_ems_msg_BMS_message_1 = {1, 0, 100, 0x1801FFF4,
                                              {
                                                { &sg_detect_battery_short, 63},
                                                { &sg_detect_battery_temp_rise, 62},
                                                { &sg_detect_battery_ODV, 61},
                                                { &sg_detect_battery_ODT, 60},
                                                { &sg_detect_battery_OLT, 59},
                                                { &sg_detect_battery_OCC, 58},
                                                { &sg_detect_battery_OLTV, 57},
                                                { &sg_detect_battery_OHTV, 56},
                                                { &sg_detect_battery_comm_abort, 55},
                                                { &sg_detect_battery_Uleak, 54},
                                                { &sg_detect_battery_Leak, 53},
                                                { &sg_detect_battery_LSOC, 52},
                                                { &sg_detect_battery_ODC, 51},
                                                { &sg_detect_battery_OHT, 50},
                                                { &sg_detect_battery_OLV, 49},
                                                { &sg_detect_battery_OHV, 48},
                                                { &sg_detect_SOC, 32},
                                                { &sg_detect_total_current, 16},
                                                { &sg_detect_total_voltage, 0},

                                               }
                                             };
                                                       

                							 	                      							 
#pragma CONST_SEG DEFAULT		             





/*****************************************************************************
 *��������:detect_ems_task_ecu_tx_run
 *��������:��Ϣ����������
 *��    ��:pdata---��������
 *�� �� ֵ:none
 *�޶���Ϣ:
 ******************************************************************************/
void detect_ems_task_ecu_tx_run(void* pdata)
{
    INT16U i = 0;
    INT32U now_tick = 0;
    INT32U tick_50ms;
    now_tick = get_tick_count();
    for (i=0; i<EMS_MSG_NUM; i++)
    {
        msg_last_tick[i] = now_tick;
    }
	
	tick_50ms = now_tick;
		
    for (;;)
    { 
    
        now_tick = get_tick_count();
        if( get_interval_by_tick(msg_last_tick[0], now_tick) >= detect_ems_msg_BMS_message_4.msg_period )
        {
            msg_last_tick[0] = now_tick;
            detect_ems_task_ecu_tx_BMS_message_4();
            sleep(1);
        }    
        now_tick = get_tick_count();
        if( get_interval_by_tick(msg_last_tick[1], now_tick) >= detect_ems_msg_BMS_message_3.msg_period )
        {
            msg_last_tick[1] = now_tick;
            detect_ems_task_ecu_tx_BMS_message_3();
            sleep(1);
        }    
        now_tick = get_tick_count();
        if( get_interval_by_tick(msg_last_tick[2], now_tick) >= detect_ems_msg_BMS_message_2.msg_period )
        {
            msg_last_tick[2] = now_tick;
            detect_ems_task_ecu_tx_BMS_message_2();
            sleep(1);
        }    
        now_tick = get_tick_count();
        if( get_interval_by_tick(msg_last_tick[3], now_tick) >= detect_ems_msg_BMS_message_1.msg_period )
        {
            msg_last_tick[3] = now_tick;
            detect_ems_task_ecu_tx_BMS_message_1();
            sleep(1);
        }
        now_tick = get_tick_count();
        if( get_interval_by_tick(tick_50ms, now_tick) >= 50 )
        {
            logic_ctrl_update();
        }
        sleep(2);   
    }   
} 




/*****************************************************************************
*��������:detect_ems_task_ecu_tx_BMS_message_4
*��������:��ϢBMS_message_4��䡢���ͺ���
*��    ��:none
*�� �� ֵ:none
*�޶���Ϣ:
******************************************************************************/
void detect_ems_task_ecu_tx_BMS_message_4(void)
{
    safe_memset(g_ems_ecu_message.data, 0x00, CAN_DATA_MAX_LEN);
    detect_ems_task_ecu_tx_BMS_message_4_fill(&g_ems_ecu_message);
    can_send(g_ems_context.can_context, &g_ems_ecu_message);
}
/*****************************************************************************
*��������:detect_ems_task_ecu_tx_BMS_message_3
*��������:��ϢBMS_message_3��䡢���ͺ���
*��    ��:none
*�� �� ֵ:none
*�޶���Ϣ:
******************************************************************************/
void detect_ems_task_ecu_tx_BMS_message_3(void)
{
    safe_memset(g_ems_ecu_message.data, 0x00, CAN_DATA_MAX_LEN);
    detect_ems_task_ecu_tx_BMS_message_3_fill(&g_ems_ecu_message);
    can_send(g_ems_context.can_context, &g_ems_ecu_message);
}
/*****************************************************************************
*��������:detect_ems_task_ecu_tx_BMS_message_2
*��������:��ϢBMS_message_2��䡢���ͺ���
*��    ��:none
*�� �� ֵ:none
*�޶���Ϣ:
******************************************************************************/
void detect_ems_task_ecu_tx_BMS_message_2(void)
{
    safe_memset(g_ems_ecu_message.data, 0x00, CAN_DATA_MAX_LEN);
    detect_ems_task_ecu_tx_BMS_message_2_fill(&g_ems_ecu_message);
    can_send(g_ems_context.can_context, &g_ems_ecu_message);
}
/*****************************************************************************
*��������:detect_ems_task_ecu_tx_BMS_message_1
*��������:��ϢBMS_message_1��䡢���ͺ���
*��    ��:none
*�� �� ֵ:none
*�޶���Ϣ:
******************************************************************************/
void detect_ems_task_ecu_tx_BMS_message_1(void)
{
    safe_memset(g_ems_ecu_message.data, 0x00, CAN_DATA_MAX_LEN);
    detect_ems_task_ecu_tx_BMS_message_1_fill(&g_ems_ecu_message);
    can_send(g_ems_context.can_context, &g_ems_ecu_message);
}

 
/*****************************************************************************
*��������:detect_ems_task_ecu_tx_BMS_message_4_fill
*��������:��ϢBMS_message_4��亯��
*��    ��:send_msg---MSCAN������Ϣ
*�� �� ֵ:none
*�޶���Ϣ:
******************************************************************************/
void detect_ems_task_ecu_tx_BMS_message_4_fill(CanMessage * _PAGED send_msg)
{
    INT32U temp_data = 0;

    if (send_msg == NULL)
    {
        return;
    }

    send_msg->id.value = can_id_from_extend_id(detect_ems_msg_BMS_message_4.msg_id);

    temp_data = detect_ems_get_precharge_relay_state();
    write_data_with_bit_addr(send_msg->data, &detect_ems_msg_BMS_message_4.msg_signal[0], temp_data, CAN_DATA_MAX_LEN);
  
    temp_data = detect_ems_get_charge_relay_state();
    write_data_with_bit_addr(send_msg->data, &detect_ems_msg_BMS_message_4.msg_signal[1], temp_data, CAN_DATA_MAX_LEN);
  
    temp_data = detect_ems_get_discharge_relay_state();
    write_data_with_bit_addr(send_msg->data, &detect_ems_msg_BMS_message_4.msg_signal[2], temp_data, CAN_DATA_MAX_LEN);
  
    temp_data = detect_ems_get_SOH(detect_ems_msg_BMS_message_4.msg_signal[3].signal->factor, detect_ems_msg_BMS_message_4.msg_signal[3].signal->offset);
    write_data_with_bit_addr(send_msg->data, &detect_ems_msg_BMS_message_4.msg_signal[3], temp_data, CAN_DATA_MAX_LEN);
  
    temp_data = detect_ems_get_negative_insulation_resistance(detect_ems_msg_BMS_message_4.msg_signal[4].signal->factor, detect_ems_msg_BMS_message_4.msg_signal[4].signal->offset);
    write_data_with_bit_addr(send_msg->data, &detect_ems_msg_BMS_message_4.msg_signal[4], temp_data, CAN_DATA_MAX_LEN);
  

    send_msg->len = CAN_DATA_MAX_LEN;
}
/*****************************************************************************
*��������:detect_ems_task_ecu_tx_BMS_message_3_fill
*��������:��ϢBMS_message_3��亯��
*��    ��:send_msg---MSCAN������Ϣ
*�� �� ֵ:none
*�޶���Ϣ:
******************************************************************************/
void detect_ems_task_ecu_tx_BMS_message_3_fill(CanMessage * _PAGED send_msg)
{
    INT32U temp_data = 0;

    if (send_msg == NULL)
    {
        return;
    }

    send_msg->id.value = can_id_from_extend_id(detect_ems_msg_BMS_message_3.msg_id);

    temp_data = detect_ems_get_positive_insulation_resistance(detect_ems_msg_BMS_message_3.msg_signal[0].signal->factor, detect_ems_msg_BMS_message_3.msg_signal[0].signal->offset);
    write_data_with_bit_addr(send_msg->data, &detect_ems_msg_BMS_message_3.msg_signal[0], temp_data, CAN_DATA_MAX_LEN);
  
    temp_data = detect_ems_get_low_temperature_id();
    write_data_with_bit_addr(send_msg->data, &detect_ems_msg_BMS_message_3.msg_signal[1], temp_data, CAN_DATA_MAX_LEN);
  
    temp_data = detect_ems_get_low_temperature(detect_ems_msg_BMS_message_3.msg_signal[2].signal->factor, detect_ems_msg_BMS_message_3.msg_signal[2].signal->offset);
    write_data_with_bit_addr(send_msg->data, &detect_ems_msg_BMS_message_3.msg_signal[2], temp_data, CAN_DATA_MAX_LEN);
  
    temp_data = detect_ems_get_high_temperature_id();
    write_data_with_bit_addr(send_msg->data, &detect_ems_msg_BMS_message_3.msg_signal[3], temp_data, CAN_DATA_MAX_LEN);
  
    temp_data = detect_ems_get_high_temperature(detect_ems_msg_BMS_message_3.msg_signal[4].signal->factor, detect_ems_msg_BMS_message_3.msg_signal[4].signal->offset);
    write_data_with_bit_addr(send_msg->data, &detect_ems_msg_BMS_message_3.msg_signal[4], temp_data, CAN_DATA_MAX_LEN);
  

    send_msg->len = CAN_DATA_MAX_LEN;
}
/*****************************************************************************
*��������:detect_ems_task_ecu_tx_BMS_message_2_fill
*��������:��ϢBMS_message_2��亯��
*��    ��:send_msg---MSCAN������Ϣ
*�� �� ֵ:none
*�޶���Ϣ:
******************************************************************************/
void detect_ems_task_ecu_tx_BMS_message_2_fill(CanMessage * _PAGED send_msg)
{
    INT32U temp_data = 0;

    if (send_msg == NULL)
    {
        return;
    }

    send_msg->id.value = can_id_from_extend_id(detect_ems_msg_BMS_message_2.msg_id);

    temp_data = detect_ems_get_low_voltage_id();
    write_data_with_bit_addr(send_msg->data, &detect_ems_msg_BMS_message_2.msg_signal[0], temp_data, CAN_DATA_MAX_LEN);
  
    temp_data = detect_ems_get_low_voltage(detect_ems_msg_BMS_message_2.msg_signal[1].signal->factor, detect_ems_msg_BMS_message_2.msg_signal[1].signal->offset);
    write_data_with_bit_addr(send_msg->data, &detect_ems_msg_BMS_message_2.msg_signal[1], temp_data, CAN_DATA_MAX_LEN);
  
    temp_data = detect_ems_get_high_voltage_id();
    write_data_with_bit_addr(send_msg->data, &detect_ems_msg_BMS_message_2.msg_signal[2], temp_data, CAN_DATA_MAX_LEN);
  
    temp_data = detect_ems_get_high_voltage(detect_ems_msg_BMS_message_2.msg_signal[3].signal->factor, detect_ems_msg_BMS_message_2.msg_signal[3].signal->offset);
    write_data_with_bit_addr(send_msg->data, &detect_ems_msg_BMS_message_2.msg_signal[3], temp_data, CAN_DATA_MAX_LEN);
  

    send_msg->len = CAN_DATA_MAX_LEN;
}
/*****************************************************************************
*��������:detect_ems_task_ecu_tx_BMS_message_1_fill
*��������:��ϢBMS_message_1��亯��
*��    ��:send_msg---MSCAN������Ϣ
*�� �� ֵ:none
*�޶���Ϣ:
******************************************************************************/
void detect_ems_task_ecu_tx_BMS_message_1_fill(CanMessage * _PAGED send_msg)
{
    INT32U temp_data = 0;

    if (send_msg == NULL)
    {
        return;
    }

    send_msg->id.value = can_id_from_extend_id(detect_ems_msg_BMS_message_1.msg_id);

    temp_data = detect_ems_get_battery_short();
    write_data_with_bit_addr(send_msg->data, &detect_ems_msg_BMS_message_1.msg_signal[0], temp_data, CAN_DATA_MAX_LEN);
  
    temp_data = detect_ems_get_battery_temp_rise();
    write_data_with_bit_addr(send_msg->data, &detect_ems_msg_BMS_message_1.msg_signal[1], temp_data, CAN_DATA_MAX_LEN);
  
    temp_data = detect_ems_get_battery_ODV();
    write_data_with_bit_addr(send_msg->data, &detect_ems_msg_BMS_message_1.msg_signal[2], temp_data, CAN_DATA_MAX_LEN);
  
    temp_data = detect_ems_get_battery_ODT();
    write_data_with_bit_addr(send_msg->data, &detect_ems_msg_BMS_message_1.msg_signal[3], temp_data, CAN_DATA_MAX_LEN);
  
    temp_data = detect_ems_get_battery_OLT();
    write_data_with_bit_addr(send_msg->data, &detect_ems_msg_BMS_message_1.msg_signal[4], temp_data, CAN_DATA_MAX_LEN);
  
    temp_data = detect_ems_get_battery_OCC();
    write_data_with_bit_addr(send_msg->data, &detect_ems_msg_BMS_message_1.msg_signal[5], temp_data, CAN_DATA_MAX_LEN);
  
    temp_data = detect_ems_get_battery_OLTV();
    write_data_with_bit_addr(send_msg->data, &detect_ems_msg_BMS_message_1.msg_signal[6], temp_data, CAN_DATA_MAX_LEN);
  
    temp_data = detect_ems_get_battery_OHTV();
    write_data_with_bit_addr(send_msg->data, &detect_ems_msg_BMS_message_1.msg_signal[7], temp_data, CAN_DATA_MAX_LEN);
  
    temp_data = detect_ems_get_battery_comm_abort();
    write_data_with_bit_addr(send_msg->data, &detect_ems_msg_BMS_message_1.msg_signal[8], temp_data, CAN_DATA_MAX_LEN);
  
    temp_data = detect_ems_get_battery_Uleak();
    write_data_with_bit_addr(send_msg->data, &detect_ems_msg_BMS_message_1.msg_signal[9], temp_data, CAN_DATA_MAX_LEN);
  
    temp_data = detect_ems_get_battery_Leak();
    write_data_with_bit_addr(send_msg->data, &detect_ems_msg_BMS_message_1.msg_signal[10], temp_data, CAN_DATA_MAX_LEN);
  
    temp_data = detect_ems_get_battery_LSOC();
    write_data_with_bit_addr(send_msg->data, &detect_ems_msg_BMS_message_1.msg_signal[11], temp_data, CAN_DATA_MAX_LEN);
  
    temp_data = detect_ems_get_battery_ODC();
    write_data_with_bit_addr(send_msg->data, &detect_ems_msg_BMS_message_1.msg_signal[12], temp_data, CAN_DATA_MAX_LEN);
  
    temp_data = detect_ems_get_battery_OHT();
    write_data_with_bit_addr(send_msg->data, &detect_ems_msg_BMS_message_1.msg_signal[13], temp_data, CAN_DATA_MAX_LEN);
  
    temp_data = detect_ems_get_battery_OLV();
    write_data_with_bit_addr(send_msg->data, &detect_ems_msg_BMS_message_1.msg_signal[14], temp_data, CAN_DATA_MAX_LEN);
  
    temp_data = detect_ems_get_battery_OHV();
    write_data_with_bit_addr(send_msg->data, &detect_ems_msg_BMS_message_1.msg_signal[15], temp_data, CAN_DATA_MAX_LEN);
  
    temp_data = detect_ems_get_SOC(detect_ems_msg_BMS_message_1.msg_signal[16].signal->factor, detect_ems_msg_BMS_message_1.msg_signal[16].signal->offset);
    write_data_with_bit_addr(send_msg->data, &detect_ems_msg_BMS_message_1.msg_signal[16], temp_data, CAN_DATA_MAX_LEN);
  
    temp_data = detect_ems_get_total_current(detect_ems_msg_BMS_message_1.msg_signal[17].signal->factor, detect_ems_msg_BMS_message_1.msg_signal[17].signal->offset);
    write_data_with_bit_addr(send_msg->data, &detect_ems_msg_BMS_message_1.msg_signal[17], temp_data, CAN_DATA_MAX_LEN);
  
    temp_data = detect_ems_get_total_voltage(detect_ems_msg_BMS_message_1.msg_signal[18].signal->factor, detect_ems_msg_BMS_message_1.msg_signal[18].signal->offset);
    write_data_with_bit_addr(send_msg->data, &detect_ems_msg_BMS_message_1.msg_signal[18], temp_data, CAN_DATA_MAX_LEN);
  

    send_msg->len = CAN_DATA_MAX_LEN;
}



/*****************************************************************************
 *��������:detect_ems_task_ecu_rx_run
 *��������:��Ϣ����������
 *��    ��:pdata---��������
 *�� �� ֵ:none
 *�޶���Ϣ:
 ******************************************************************************/
#if( SWITCH_ON == UDS_SWITCH_CONFIG)  //UDS����
INT8U g_MCU_cnt;
INT8U g_VCU_cnt;
INT8U g_ACU_cnt;
INT8U g_OBC_cnt;
#endif

void detect_ems_task_ecu_rx_run(void* pdata)
{
    #if( SWITCH_ON == UDS_SWITCH_CONFIG)
    INT32U std_can_id ;
    CanMessage msg;
    INT32U tick_to_check_dtc      = 0;
    INT32U tick_to_handle_uds_msg = 0;
    INT32U tick_to_check_busoff   = 0;
    INT8U temp = 0;
    INT32U now_tick = get_tick_count();
    INT32U vcu_last = now_tick;
    INT32U mcu_last = now_tick;
    INT32U acu_last = now_tick;
    INT32U obc_last = now_tick;
    #endif
    Result res;

    for (;;)
    {
        res = can_receive(g_ems_context.can_context, &g_ems_ecu_rx_message, 10);
        if (res == RES_OK)
        {
            INT32U id = 0;
            id = can_id_to_extend_id(g_ems_ecu_rx_message.id.value);
            switch(id)
            {

                default:
                    break;
            }

            #if( SWITCH_ON == UDS_SWITCH_CONFIG)  //UDS���մ���
            std_can_id   = can_id_to_std_id(g_ems_ecu_rx_message.id.value);
            /* CAN ���յ�����-> �жϸñ���Ϊ��ϱ���-> ���øýӿڣ��������ݸ�CanIf -> CanIf ���ݸ�CanTp -> CanTP ���ݸ�Dcm ���� */
            if((std_can_id == CanIfRxPduConfig[0].CanId) || (std_can_id ==  CanIfRxPduConfig[1].CanId))
            {
                CanIf_RxIndication((std_can_id == CanIfRxPduConfig[1].CanId),  //Can_HwHandleType Hrh, 
                            std_can_id,                   //Can_IdType CanId,
                            g_ems_ecu_rx_message.len,     //uint8 CanDlc, 
                            g_ems_ecu_rx_message.data);   //const uint8 * CanSduPtr
            }
            #endif
        }
        #if( SWITCH_ON == UDS_SWITCH_CONFIG)  //UDS���ʹ���
        now_tick = get_tick_count();
    
        if(getBusOffstate() == TRUE )
        {
            vcu_last = now_tick;
            mcu_last = now_tick;
            acu_last = now_tick;
            obc_last = now_tick;
        }
        
        if(get_interval_by_tick(tick_to_handle_uds_msg, get_tick_count()) >= EMS_CHECK_UDS_MSG_INTERVAL)
        {
            Dcm_MainFunction();
            CanTp_MainFunction();            
            tick_to_handle_uds_msg = get_tick_count();      
        }
        
        if(get_interval_by_tick(tick_to_check_busoff, get_tick_count()) >= EMS_CHECK_BUSOFF_INTERVAL)
        {
            Canbusoff_MainFunction();   ///zmx
            Canbusoff_process();      ///zmx            
            CanPower_MainFunction();   ///zmx           
            tick_to_check_busoff = get_tick_count();
        }
        
        if(get_interval_by_tick(tick_to_check_dtc, get_tick_count()) > EMS_CHECK_DTC_STATUS_INTERVAL)
        {
            //CanVoltagecheck_MainFunction(); ///zmx
            DtcTimeOutMainfunction(); ///zmx
             
            CheckDTC();  //DTC���麯��    Ҫ���Ժ���ʹ�õ�ʱ��
            tick_to_check_dtc = get_tick_count();
        }
        #endif
    }
}






/*****************************************************************************
 *��������:detect_ems_task_dbd_tx_run
 *��������:������Ϣ����������
 *��    ��:pdata---��������
 *�� �� ֵ:none
 *�޶���Ϣ:
 ******************************************************************************/
void detect_ems_task_dbd_tx_run(void* pdata)
{
    INT32U now_tick = 0;
		
    for (;;)
    { 
        now_tick = get_tick_count();
        if( get_interval_by_tick(volt_msg_last_tick, now_tick) >= EMS_VOLT_MSG_SEND_PERIOD )
        {
            volt_msg_last_tick = now_tick;
    		detect_ems_task_dbd_tx_voltage();
    		sleep(1);
     	  } 
        now_tick = get_tick_count();
        if( get_interval_by_tick(temp_msg_last_tick, now_tick) >= EMS_TEMP_MSG_SEND_PERIOD )
        {
            temp_msg_last_tick = now_tick;
    		detect_ems_task_dbd_tx_temperature();
    		sleep(1);
     	  } 
     		
        sleep(5);
    }
    
}

/*****************************************************************************
 *��������:detect_ems_task_dbd_tx_voltage
 *��������:�����ѹ��Ϣ���ͺ���
 *��    ��:none
 *�� �� ֵ:none
 *�޶���Ϣ:
 ******************************************************************************/
void detect_ems_task_dbd_tx_voltage(void)
{
    INT8U slave_index = 0;
    INT8U cell_index = 0;
    INT8U slave_num = 0;
    INT8U voltage_num = 0;
    INT8U VoltageIDOffset = 0;
    INT16U temp = 0;
    
    g_ems_dbd_message.len = 0;
    safe_memset(g_ems_dbd_message.data, 0xFF, CAN_DATA_MAX_LEN);
    
    slave_num = (INT8U)config_get(kSlaveNumIndex);

    for (slave_index = 0; slave_index < slave_num; slave_index++)
    {
        voltage_num = (INT8U)bmu_get_voltage_num((BmuId)slave_index);
        for (cell_index = 0; cell_index < voltage_num; cell_index++)
        {
            temp = (INT16U)bmu_get_voltage_item(slave_index, cell_index);
            WRITE_LT_INT16U_WITH_BUFF_SIZE(g_ems_dbd_message.data, g_ems_dbd_message.len, temp, CAN_DATA_MAX_LEN);
            
            if(g_ems_dbd_message.len >= CAN_DATA_MAX_LEN)
            {
                g_ems_dbd_message.len = CAN_DATA_MAX_LEN;
                g_ems_dbd_message.id.value = can_id_from_extend_id(EMS_ECU_VOLT_BASE_CAN_ID + ((INT32U)VoltageIDOffset << 16));
                can_send(g_ems_context.can_context, &g_ems_dbd_message);

                safe_memset(g_ems_dbd_message.data, 0xFF, CAN_DATA_MAX_LEN);
                g_ems_dbd_message.len = 0;
                VoltageIDOffset++;
                sleep(EMS_ECU_TX_INTERVAL);
            }
            
        }
    }
    
    if(g_ems_dbd_message.len > 0)
    {
       g_ems_dbd_message.len = CAN_DATA_MAX_LEN;
       g_ems_dbd_message.id.value = can_id_from_extend_id(EMS_ECU_VOLT_BASE_CAN_ID + ((INT32U)VoltageIDOffset << 16));
       can_send(g_ems_context.can_context, &g_ems_dbd_message);

       safe_memset(g_ems_dbd_message.data, 0xFF, CAN_DATA_MAX_LEN);
       g_ems_dbd_message.len = 0;
       VoltageIDOffset++;
       sleep(EMS_ECU_TX_INTERVAL);
    }
}
/*****************************************************************************
 *��������:detect_ems_task_dbd_tx_temperature
 *��������:�¶���Ϣ���ͺ���
 *��    ��:none
 *�� �� ֵ:none
 *�޶���Ϣ:
 ******************************************************************************/
void detect_ems_task_dbd_tx_temperature(void)
{
    INT8U slave_index = 0;
    INT8U temp_index = 0;
    INT8U slave_num = 0;
    INT8U temp_num = 0;
    INT8U TempIDOffset = 0;
    INT16U temp = 0;
    
    g_ems_dbd_message.len = 0;
    safe_memset(g_ems_dbd_message.data, 0xFF, CAN_DATA_MAX_LEN);
    
    slave_num = (INT8U)config_get(kSlaveNumIndex);
    
    for(slave_index=0; slave_index < slave_num; slave_index++)
    {
        temp_num = (INT8U)bmu_get_temperature_num((BmuId)slave_index);
      
        for(temp_index=0; temp_index < temp_num; temp_index++)
        {
           temp = TEMP_TO_40_OFFSET(bmu_get_temperature_item(slave_index, temp_index));
           WRITE_LT_INT8U(g_ems_dbd_message.data, g_ems_dbd_message.len, temp);
           
           if(g_ems_dbd_message.len >= CAN_DATA_MAX_LEN)
           {
              g_ems_dbd_message.len = CAN_DATA_MAX_LEN;
              g_ems_dbd_message.id.value = can_id_from_extend_id(EMS_ECU_TEMP_BASE_CAN_ID + ((INT32U)TempIDOffset << 16));
              can_send(g_ems_context.can_context, &g_ems_dbd_message);

              safe_memset(g_ems_dbd_message.data, 0xFF, CAN_DATA_MAX_LEN);
              g_ems_dbd_message.len = 0;
              TempIDOffset++;
              sleep(EMS_ECU_TX_INTERVAL);
           }  
        }  
    }
    
    if(g_ems_dbd_message.len > 0)
    {
       g_ems_dbd_message.len = CAN_DATA_MAX_LEN;
       g_ems_dbd_message.id.value = can_id_from_extend_id(EMS_ECU_TEMP_BASE_CAN_ID + ((INT32U)TempIDOffset << 16));
       can_send(g_ems_context.can_context, &g_ems_dbd_message);

       safe_memset(g_ems_dbd_message.data, 0xFF, CAN_DATA_MAX_LEN);
       g_ems_dbd_message.len = 0;
       TempIDOffset++;
       sleep(EMS_ECU_TX_INTERVAL);
    }
}