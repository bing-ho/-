/*
 * bms_upper_computer_j1939.h
 *
 *  Created on: 2012-9-19
 *      Author: Administrator
 */

#ifndef BMS_UPPER_COMPUTER_J1939_H_
#define BMS_UPPER_COMPUTER_J1939_H_

#include "bms_upper_computer.h"

// 这样定义方便测试模式下使用这部分内存,
// 测试模式的任务需要任务较大.
union upper_computer_task_stack {
    struct slave_task_stack {
    OS_STK tx[SLAVE_TX_STK_SIZE];
    OS_STK rx[SLAVE_RX_STK_SIZE];
    } stack;
    OS_STK stack_all[SLAVE_TX_STK_SIZE + SLAVE_RX_STK_SIZE];  
};

extern union upper_computer_task_stack g_slave_task_stack;

#define BCU_J1939_PROTOCOL_MAJOR_VERSION    1
#define BCU_J1939_PROTOCOL_MINOR_VERSION    0

#define SLAVE_ADDR_OFFSET                   180
#define SLAVE_START_NUM                     1
#define INSU_ADDRESS                        0xD0
#define PCM_ADDRESS                         0xD7

#define UPPER_REC_PDU_BUFF                  5           //接收PDU单帧数据缓存大小
#define UPPER_SENT_FRAME_BUFF               5            //发送帧缓存个数
#define UPPER_REC_FRAME_BUFF                5            //接收帧缓存个数
#define UPPER_SENT_MESSAGE_LENGTH_MAX       500            //待发送消息的最大长度
#define UPPER_REC_MESSAGE_LENGTH_MAX        500          //接收消息的最大长度
#define UPPER_REC_MESSAGE_BUFF_MAX          2           //接收消息的最大缓存个数
#define UPPER_TP_REC_MAX                    2           //同时可以接收的TP的最大个数

#define BCU_REQ_BMU_FIRST_FRAME_TIMEOUT     50

#define BCU_CAN_MESSAGE_PRI_DEFAULT         6

#define DTU_INFO_MESSAGE_PGN                0x9600
#define DTU_CONFIG_MESSAGE_PGN              0xA900

#define BMS_DEVICE_COMM_CTL_PGN             0x6000
#define BMS_DEVICE_COMM_INFO_PGN            0x6200
#define BMS_DEVICE_PING_PGN                 0x6500
#define BMS_DEVICE_RELAY_PGN                0x7100

#define BCU_RULE_READ_REQ_PGN               0x7500
#define BCU_RULE_READ_RSP_PGN               0x7600
#define BCU_RULE_WRITE_REQ_PGN              0x7700
#define BCU_EXPRESS_REQ_PGN                 0x7800
#define BCU_COMM_RSP_PGN                    0x7900
#define BMS_DEBUG_REQ_PGN                   0x7C00
#define BMS_DEBUG_RSP_PGN                   0x7D00
#define BMS_PACK_TEST_PGN                   0x7F00

#define BCU_BASE_CONFIG_PGN                 0x8000
#define BCU_COMMON_ALARM_CONFIG_PGN         0x8100
#define BCU_VOLT_ALARM_CONFIG_PGN           0x8200
#define BCU_CURRENT_ALARM_CONFIG_PGN        0x8300
#define BCU_TEMPERATURE_ALARM_CONFIG_PGN    0x8400
#define BCU_SOC_ALARM_CONFIG_PGN            0x8500
#define BCU_INSU_ALARM_CONFIG_PGN           0x8600
#define BCU_INSU2_ALARM_CONFIG_PGN          0xBB00
#define BCU_RELAY_CONFIG_PGN                0x8700
#define BCU_HEAT_COOL_CONFIG_PGN            0x8800
#define BCU_CHARGER_CONFIG_PGN              0x8900
#define BCU_CHG_VOLT_ALARM_CONFIG_PGN       0x8A00
#define BCU_CHG_TEMP_ALARM_CONFIG_PGN       0x8B00
#define BCU_BATTERY_CODING_PGN              0x8C00

#define BCU_NAND_READ_PGN                   0x8D00 
#define BCU_NAND_SCAN_PGN                   0x8E00

#define BCU_BASE_MESSAGE_PGN                0x9000
#define BCU_STATISTIC_MESSAGE_PGN           0x9100
#define BCU_ALARM_MESSAGE_PGN               0x9200
#define BCU_RELAY_MESSAGE_PGN               0x9300
#define BCU_INSU_MESSAGE_PGN                0x9400
#define BCU_CHARGER_INFO_MESSAGE_PGN        0x9500
#define BCU_MLOCK_CONTROL_PGN               0x9700
#define BCU_CHGR_OUTLET_TEMP_CONFIG_PGN     0x9800        
#define BCU_DEBUG_MESSAGE_PGN               0x9A00

#define BCU_PGN_END                         BCU_DEBUG_MESSAGE_PGN

#define BMU_BASE_CONFIG_PGN                 0xA000
#define BMU_RELAY_CONFIG_PGN                0xA100
#define BMU_ALARM_CONFIG_PGN                0xA200
#define BMU_BALANCE_CONFIG_PGN              0xA300
#define BMU_5160_CONFIG_PGN                 0xA800
#define BMU_BASE_STATUS_PGN                 0xB000
#define BMU_VOLT_STATUS_PGN                 0xB100
#define BMU_TEMP_STATUS_PGN                 0xB200
#define BMU_RELAY_STATUS_PGN                0xB300
#define BMU_ALARM_STATUS_PGN                0xB400

#define INSU_BASE_STATUS_PGN                0xBA00

#define PCM_BASE_STATUS_PGN                 0xBD00

#define BCU_TO_BMU_MESSAGE_PGN              0xC000
#define BCU_TO_LDM_MESSAGE_PGN              0xC500

#define MESSAGE_REQUIRE_PGN                 0xEA00        //请求数据PGN
#define MESSAGE_CONFIRM_PGN                 0xE800        //消息确认PGN
#define DATE_TIME_PGN                       0xD500      // J1939 对时函数

#define BMU_STATUS_START_PGN                BMU_BASE_STATUS_PGN
#define BMU_STATUS_STOP_PGN                    BMU_ALARM_STATUS_PGN

#define BCU_RULE_READ_REQ_PF                0x75
#define BCU_RULE_READ_RSP_PF                0x76
#define BCU_RULE_WRITE_REQ_PF               0x77
#define BCU_EXPRESS_REQ_PF                  0x78
#define BCU_COMM_RSP_PF                     0x79

#define BCU_RULE_WRITER_OPEN_CMD    90
#define BCU_RULE_WRITER_CLOSE_CMD   91
#define BCU_RULE_WRITER_WRITE_CMD   92

#define BCU_RULE_READER_READ_CMD    82
#define BCU_RULE_READER_COUNT_CMD   83

#define BCU_RULE_VM_COMPILE_CMD     70
#define BCU_RULE_VM_EXECUTE_CMD     71
#define BCU_RULE_VM_QUERY_NUM_CMD   72
#define BCU_RULE_VM_QUERY_CMD       73

#define RESPONSE_ACK                0
#define RESPONSE_NACK               1
#define RESPONSE_REFUSE             2
#define RESPONSE_BUSY               3

#define BCU_COMM_CMD_STOP           1
#define BCU_COMM_CMD_RESTART        2
#define BCU_COMM_CMD_ENTER_TEST     5
#define BCU_COMM_CMD_ENTER_READ     8
#define BCU_COMM_CMD_ENTER_NORMAL   7
#define BCU_COMM_CMD_ENTER_DETECT   6

#define BCU_DEBUG_CMD_REQ_INFO      1


#define BCU_DEBUG_INFO_CPU_USAGE_INDEX 1


//#define BCU_RECEIVE_BMU_MESSAGE_TIMEOUT           1500
//#define BCU_SEND_MESSAGE_TIMEOUT                  1500 //BCU发送超时时间 分辨率：1ms/bit
#define CAN_FRAME_TIMEOUT_DEFINE                    100    //CAN一帧数据的超时时间定义 分辨率：1ms/bit
#define SLAVE_CAN_RE_INIT_TIME_WITH_COMM_ABORT      600000//10min 主从机CAN通信中断后重启延时时间 单位：ms

#define PCM_TV_RATE_OF_CHANGE_OFFSET_DEF            32000
#define BCU_REQ_SLAVE_ALL_VALID_INFO_TIMEOUT        5000 //所有从机信息有效的超时时间
#define BCU_REQ_INSU_WITH_PRECHARGE_TIMEOUT         2000 //预充超时时间，保证从机数据可更新


#pragma DATA_SEG __RPAGE_SEG PAGED_RAM_UPPER
extern J1939SendMessageBuff g_slaveSendMessage;
extern J1939CanContext* _PAGED g_slaveCanContextPtr;
extern J1939CanContext g_upperCanContext;
extern J1939Event g_upperSendEventSem;
extern J1939Event g_upperRecEventSem;
extern J1939CanInfo g_upperCanInfo;
extern J1939CanTranPro g_upperSendTpIndex;
extern J1939CanTPBuff g_upperRecTpBuffIndex;
extern J1939TPChain g_upperRecTpBuffs[UPPER_TP_REC_MAX];
extern J1939CanFrameBuff g_upperSendFrameBuffIndex;
extern J1939CanFrame g_upperSendFrameBuffs[UPPER_SENT_FRAME_BUFF];
extern J1939CanFrameBuff g_upperRecPduBuffIndex;
extern J1939CanFrame g_upperRecPduBuffs[UPPER_REC_PDU_BUFF];
extern J1939CanFrameBuff g_upperRecFrameBuffIndex;
extern J1939CanFrame g_upperRecFrameBuffs[UPPER_REC_FRAME_BUFF];
extern J1939SendMessageBuff g_upperSendMessageBuff;
extern J1939RecTempMessageBuff g_upperRecTempMessageBuffIndex;
extern J1939RecMessage g_upperRecTempMessageBuffs[UPPER_TP_REC_MAX];
extern J1939RecMessageBuff g_upperRecMessageBuffIndex;
extern J1939RecMessage g_upperRecMessageBuffs[UPPER_REC_MESSAGE_BUFF_MAX];
#pragma DATA_SEG DEFAULT

extern INT16U upper_computer_callback(can_t context, CanMessage* msg, void* userdata);
void upper_computer_init_ext(char is_test_mode);

#endif


