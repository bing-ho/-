/// \file dtu_m35.c
/// \brief M35驱动.
/// \author Xi Qingping, qingping.xi@ligoo.cn
/// \version
/// \date 2015-09-01
#include <string.h>
#include "dtu_m35.h"

#include "dtu_atcmd.h"
#include "ring_buffer.h"
#include "dtu_send_sim.h"
#include "ex_string.h"
#include "os_cpu.h"
#include "Types.h"
#include "PFlash.h"
#include "bms_config.h"
#include "bms_upper_computer_j1939.h"
#include "main_bsu_relay_ctrl.h"
#include "bms_mlock.h"
#include "bms_rule_engine_persistence.h"

///////////////////////////////////// for data safe ///////////////////////////
#include "bms_util.h"
#define memset safe_memset
#define memcmp safe_memcmp
#define memcpy safe_memcpy
#define strcpy safe_strcpy
#define strlen safe_strlen
#define strcmp safe_strcmp


//#define dprintf printf
#define dprintf (void)

#if DTUM35_CHECK != 0
#define DTUM35_INTERFACE_IS_VALID(iface) ((0 != (iface)) && ((iface)->magic == DTUM35_CHECK_MAGIC))
#else
#define DTUM35_INTERFACE_IS_VALID(iface) (0 != (iface))
#endif

#define SMS_SERVER_NUM "18655194565"
#define SELF_UNKNOW_NUM "18898765432"

/// 计算数字元素个数的宏.
#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))
/// 计算结构体中成员变量的便宜量的宏.
#define STRUCT_OFFSET(__struct, __menmber) ((int)(&(((__struct *)0)->__menmber)))

extern INT8U bcu_base_config_update(J1939RecMessage* _PAGED msg);
extern INT8U bms_config_device_relay_info(J1939RecMessage* _PAGED rec_msg);
extern INT8U bcu_common_alarm_config_update(J1939RecMessage* _PAGED msg);
extern INT8U bcu_volt_alarm_config_update(J1939RecMessage* _PAGED msg);
extern INT8U bcu_current_alarm_config_update(J1939RecMessage* _PAGED msg);
extern INT8U bcu_temperature_alarm_config_update(J1939RecMessage* _PAGED msg);
extern INT8U bcu_soc_alarm_config_update(J1939RecMessage* _PAGED msg);
extern INT8U bcu_insu_alarm_config_update(J1939RecMessage* _PAGED msg);
extern INT8U bcu_insu2_alarm_config_update(J1939RecMessage* _PAGED msg);
extern INT8U bcu_relay_config_update(J1939RecMessage* _PAGED msg);
extern INT8U bcu_heat_cool_config_update(J1939RecMessage* _PAGED msg);
extern INT8U bcu_charger_config_update(J1939RecMessage* _PAGED msg);
extern INT8U bcu_chg_volt_alarm_config_update(J1939RecMessage* _PAGED msg);
extern INT8U bcu_chg_temp_alarm_config_update(J1939RecMessage* _PAGED msg);
extern INT8U bcu_mlock_config_update(J1939RecMessage* _PAGED msg);
extern INT8U bmu_base_config_update(J1939RecMessage* _PAGED msg);
extern INT8U bmu_balance_config_update(J1939RecMessage* _PAGED msg);
extern INT8U bmu_5160_config_update(J1939RecMessage* _PAGED msg);
extern INT8U dtu_config_update(J1939RecMessage *_PAGED msg);
extern INT8U bcu_chgr_outlet_config_update(J1939RecMessage* _PAGED msg);

#pragma push
#pragma DATA_SEG __PPAGE_SEG CONST_TABLES

/*******CRC相关定义***********/
const uchar crc16_tab_h[] = 
{ 
  (byte) 0x00, (byte) 0xC1, (byte) 0x81, (byte) 0x40, (byte) 0x01, (byte) 0xC0, (byte) 0x80, (byte) 0x41, (byte) 0x01, (byte) 0xC0, (byte) 0x80, (byte) 0x41, (byte) 0x00, 
  (byte) 0xC1, (byte) 0x81, (byte) 0x40, (byte) 0x01, (byte) 0xC0, (byte) 0x80, (byte) 0x41, (byte) 0x00, (byte) 0xC1, (byte) 0x81, (byte) 0x40, (byte) 0x00, (byte) 0xC1, 
  (byte) 0x81, (byte) 0x40, (byte) 0x01, (byte) 0xC0, (byte) 0x80, (byte) 0x41, (byte) 0x01, (byte) 0xC0, (byte) 0x80, (byte) 0x41, (byte) 0x00, (byte) 0xC1, (byte) 0x81, 
  (byte) 0x40, (byte) 0x00, (byte) 0xC1, (byte) 0x81, (byte) 0x40, (byte) 0x01, (byte) 0xC0, (byte) 0x80, (byte) 0x41, (byte) 0x00, (byte) 0xC1, (byte) 0x81, (byte) 0x40, 
  (byte) 0x01, (byte) 0xC0, (byte) 0x80, (byte) 0x41, (byte) 0x01, (byte) 0xC0, (byte) 0x80, (byte) 0x41, (byte) 0x00, (byte) 0xC1, (byte) 0x81, (byte) 0x40, (byte) 0x01, 
  (byte) 0xC0, (byte) 0x80, (byte) 0x41, (byte) 0x00, (byte) 0xC1, (byte) 0x81, (byte) 0x40, (byte) 0x00, (byte) 0xC1, (byte) 0x81, (byte) 0x40, (byte) 0x01, (byte) 0xC0, 
  (byte) 0x80, (byte) 0x41, (byte) 0x00, (byte) 0xC1, (byte) 0x81, (byte) 0x40, (byte) 0x01, (byte) 0xC0, (byte) 0x80, (byte) 0x41, (byte) 0x01, (byte) 0xC0, (byte) 0x80, 
  (byte) 0x41, (byte) 0x00, (byte) 0xC1, (byte) 0x81, (byte) 0x40, (byte) 0x00, (byte) 0xC1, (byte) 0x81, (byte) 0x40, (byte) 0x01, (byte) 0xC0, (byte) 0x80, (byte) 0x41, 
  (byte) 0x01, (byte) 0xC0, (byte) 0x80, (byte) 0x41, (byte) 0x00, (byte) 0xC1, (byte) 0x81, (byte) 0x40, (byte) 0x01, (byte) 0xC0, (byte) 0x80, (byte) 0x41, (byte) 0x00, 
  (byte) 0xC1, (byte) 0x81, (byte) 0x40, (byte) 0x00, (byte) 0xC1, (byte) 0x81, (byte) 0x40, (byte) 0x01, (byte) 0xC0, (byte) 0x80, (byte) 0x41, (byte) 0x01, (byte) 0xC0, 
  (byte) 0x80, (byte) 0x41, (byte) 0x00, (byte) 0xC1, (byte) 0x81, (byte) 0x40, (byte) 0x00, (byte) 0xC1, (byte) 0x81, (byte) 0x40, (byte) 0x01, (byte) 0xC0, (byte) 0x80, 
  (byte) 0x41, (byte) 0x00, (byte) 0xC1, (byte) 0x81, (byte) 0x40, (byte) 0x01, (byte) 0xC0, (byte) 0x80, (byte) 0x41, (byte) 0x01, (byte) 0xC0, (byte) 0x80, (byte) 0x41, 
  (byte) 0x00, (byte) 0xC1, (byte) 0x81, (byte) 0x40, (byte) 0x00, (byte) 0xC1, (byte) 0x81, (byte) 0x40, (byte) 0x01, (byte) 0xC0, (byte) 0x80, (byte) 0x41, (byte) 0x01, 
  (byte) 0xC0, (byte) 0x80, (byte) 0x41, (byte) 0x00, (byte) 0xC1, (byte) 0x81, (byte) 0x40, (byte) 0x01, (byte) 0xC0, (byte) 0x80, (byte) 0x41, (byte) 0x00, (byte) 0xC1, 
  (byte) 0x81, (byte) 0x40, (byte) 0x00, (byte) 0xC1, (byte) 0x81, (byte) 0x40, (byte) 0x01, (byte) 0xC0, (byte) 0x80, (byte) 0x41, (byte) 0x00, (byte) 0xC1, (byte) 0x81,
  (byte) 0x40, (byte) 0x01, (byte) 0xC0, (byte) 0x80, (byte) 0x41, (byte) 0x01, (byte) 0xC0, (byte) 0x80, (byte) 0x41, (byte) 0x00, (byte) 0xC1, (byte) 0x81, (byte) 0x40, 
  (byte) 0x01, (byte) 0xC0, (byte) 0x80, (byte) 0x41, (byte) 0x00, (byte) 0xC1, (byte) 0x81, (byte) 0x40, (byte) 0x00, (byte) 0xC1, (byte) 0x81, (byte) 0x40, (byte) 0x01, 
  (byte) 0xC0, (byte) 0x80, (byte) 0x41, (byte) 0x01, (byte) 0xC0, (byte) 0x80, (byte) 0x41, (byte) 0x00, (byte) 0xC1, (byte) 0x81, (byte) 0x40, (byte) 0x00, (byte) 0xC1, 
  (byte) 0x81, (byte) 0x40, (byte) 0x01, (byte) 0xC0, (byte) 0x80, (byte) 0x41, (byte) 0x00, (byte) 0xC1, (byte) 0x81, (byte) 0x40, (byte) 0x01, (byte) 0xC0, (byte) 0x80, 
  (byte) 0x41, (byte) 0x01, (byte) 0xC0, (byte) 0x80, (byte) 0x41, (byte) 0x00, (byte) 0xC1, (byte) 0x81, (byte) 0x40 
 };

 const uchar crc16_tab_l[] = 
 { 
   (byte) 0x00, (byte) 0xC0, (byte) 0xC1, (byte) 0x01, (byte) 0xC3, (byte) 0x03, (byte) 0x02, (byte) 0xC2, (byte) 0xC6, (byte) 0x06, (byte) 0x07, (byte) 0xC7, (byte) 0x05, 
   (byte) 0xC5, (byte) 0xC4, (byte) 0x04, (byte) 0xCC, (byte) 0x0C, (byte) 0x0D, (byte) 0xCD, (byte) 0x0F, (byte) 0xCF, (byte) 0xCE, (byte) 0x0E, (byte) 0x0A, (byte) 0xCA, 
   (byte) 0xCB, (byte) 0x0B, (byte) 0xC9, (byte) 0x09, (byte) 0x08, (byte) 0xC8, (byte) 0xD8, (byte) 0x18, (byte) 0x19, (byte) 0xD9, (byte) 0x1B, (byte) 0xDB, (byte) 0xDA, 
   (byte) 0x1A, (byte) 0x1E, (byte) 0xDE, (byte) 0xDF, (byte) 0x1F, (byte) 0xDD, (byte) 0x1D, (byte) 0x1C, (byte) 0xDC, (byte) 0x14, (byte) 0xD4, (byte) 0xD5, (byte) 0x15, 
   (byte) 0xD7, (byte) 0x17, (byte) 0x16, (byte) 0xD6, (byte) 0xD2, (byte) 0x12, (byte) 0x13, (byte) 0xD3, (byte) 0x11, (byte) 0xD1, (byte) 0xD0, (byte) 0x10, (byte) 0xF0, 
   (byte) 0x30, (byte) 0x31, (byte) 0xF1, (byte) 0x33, (byte) 0xF3, (byte) 0xF2, (byte) 0x32, (byte) 0x36, (byte) 0xF6, (byte) 0xF7, (byte) 0x37, (byte) 0xF5, (byte) 0x35, 
   (byte) 0x34, (byte) 0xF4, (byte) 0x3C, (byte) 0xFC, (byte) 0xFD, (byte) 0x3D, (byte) 0xFF, (byte) 0x3F, (byte) 0x3E, (byte) 0xFE, (byte) 0xFA, (byte) 0x3A, (byte) 0x3B, 
   (byte) 0xFB, (byte) 0x39, (byte) 0xF9, (byte) 0xF8, (byte) 0x38, (byte) 0x28, (byte) 0xE8, (byte) 0xE9, (byte) 0x29, (byte) 0xEB, (byte) 0x2B, (byte) 0x2A, (byte) 0xEA, 
   (byte) 0xEE, (byte) 0x2E, (byte) 0x2F, (byte) 0xEF, (byte) 0x2D, (byte) 0xED, (byte) 0xEC, (byte) 0x2C, (byte) 0xE4, (byte) 0x24, (byte) 0x25, (byte) 0xE5, (byte) 0x27, 
   (byte) 0xE7, (byte) 0xE6, (byte) 0x26, (byte) 0x22, (byte) 0xE2, (byte) 0xE3, (byte) 0x23, (byte) 0xE1, (byte) 0x21, (byte) 0x20, (byte) 0xE0, (byte) 0xA0, (byte) 0x60, 
   (byte) 0x61, (byte) 0xA1, (byte) 0x63, (byte) 0xA3, (byte) 0xA2, (byte) 0x62, (byte) 0x66, (byte) 0xA6, (byte) 0xA7, (byte) 0x67, (byte) 0xA5, (byte) 0x65, (byte) 0x64, 
   (byte) 0xA4, (byte) 0x6C, (byte) 0xAC, (byte) 0xAD, (byte) 0x6D, (byte) 0xAF, (byte) 0x6F, (byte) 0x6E, (byte) 0xAE, (byte) 0xAA, (byte) 0x6A, (byte) 0x6B, (byte) 0xAB, 
   (byte) 0x69, (byte) 0xA9, (byte) 0xA8, (byte) 0x68, (byte) 0x78, (byte) 0xB8, (byte) 0xB9, (byte) 0x79, (byte) 0xBB, (byte) 0x7B, (byte) 0x7A, (byte) 0xBA, (byte) 0xBE, 
   (byte) 0x7E, (byte) 0x7F, (byte) 0xBF, (byte) 0x7D, (byte) 0xBD, (byte) 0xBC, (byte) 0x7C, (byte) 0xB4, (byte) 0x74, (byte) 0x75, (byte) 0xB5, (byte) 0x77, (byte) 0xB7, 
   (byte) 0xB6, (byte) 0x76, (byte) 0x72, (byte) 0xB2, (byte) 0xB3, (byte) 0x73, (byte) 0xB1, (byte) 0x71, (byte) 0x70, (byte) 0xB0, (byte) 0x50, (byte) 0x90, (byte) 0x91, 
   (byte) 0x51, (byte) 0x93, (byte) 0x53, (byte) 0x52, (byte) 0x92, (byte) 0x96, (byte) 0x56, (byte) 0x57, (byte) 0x97, (byte) 0x55, (byte) 0x95, (byte) 0x94, (byte) 0x54, 
   (byte) 0x9C, (byte) 0x5C, (byte) 0x5D, (byte) 0x9D, (byte) 0x5F, (byte) 0x9F, (byte) 0x9E, (byte) 0x5E, (byte) 0x5A, (byte) 0x9A, (byte) 0x9B, (byte) 0x5B, (byte) 0x99,
   (byte) 0x59, (byte) 0x58, (byte) 0x98, (byte) 0x88, (byte) 0x48, (byte) 0x49, (byte) 0x89, (byte) 0x4B, (byte) 0x8B, (byte) 0x8A, (byte) 0x4A, (byte) 0x4E, (byte) 0x8E, 
   (byte) 0x8F, (byte) 0x4F, (byte) 0x8D, (byte) 0x4D, (byte) 0x4C, (byte) 0x8C, (byte) 0x44, (byte) 0x84, (byte) 0x85, (byte) 0x45, (byte) 0x87, (byte) 0x47, (byte) 0x46, 
   (byte) 0x86, (byte) 0x82, (byte) 0x42, (byte) 0x43, (byte) 0x83, (byte) 0x41, (byte) 0x81, (byte) 0x80, (byte) 0x40 
  };
  
#pragma pop 

enum {
    RECVED_IPD,
    RECVED_CMT,
    DATA_FOR_MASTER_SEND,
    DATA_FOR_SLAVE_SEND,
    DATA_FOR_SEND_WITH_CALLBACK,
};


#pragma DATA_SEG DEFAULT


static INT8U mTaskStack[286];


#define READ_DATA(ID, TYPE, OFFSET) READ_DATA_EX(ID, TYPE, OFFSET, 0)

#define READ_DATA_EX(ID, TYPE, OFFSET, DEFAULT) \
        if (OFFSET + sizeof(TYPE) <= rec_msg->byte_cnt) \
        {\
          ID = READ_LT_##TYPE(rec_msg->data, OFFSET); \
        }\
        else     \
        {\
          ID = DEFAULT;\
          OFFSET += sizeof(TYPE);\
        }

#define READ_STRING(ID, OFFSET) \
        READ_DATA(len, INT16U, OFFSET); \
        if (OFFSET + len + 2 <= rec_msg->byte_cnt && len > 0 && *(rec_msg->data + OFFSET + len - 1) == '\0') \
            ID = (PCSTR)rec_msg->data + OFFSET; \
        OFFSET += len;

#pragma DATA_SEG __RPAGE_SEG PAGED_RAM_DTU2

static char mIsForTest;

/// 底层接口指针, 这个在初始化的时候应该被赋值.
static const DTUM35_BspInterface *__FAR bspInterface;

/// \name SignalStrength 信号强度相关变量与定义
/// @{
/// 更新信号强度的时间间隔.
#define UPDATE_SIGNAL_STRENGTH_INTERVAL (OS_TICKS_PER_SEC * 11UL)
/// 下一次更新信号强度的时间.
static INT32U mTickToUpdateSignalStrength = 0;
/// 信号强度.
static INT8S mSignalStrength;
/// 误码率.
static INT8S mBitErrorRate;
///  @}
//

static char mIMEI[20];

static INT8U mOps;
static INT32U mTickToUpdateLacCi;
#define UPDATE_LAC_CI_INTERVAL (OS_TICKS_PER_SEC * 10UL)
static unsigned short mLacCi[2] = { 0xffff, 0xffff };

INT32U mPacketCounter = 0;

static INT8U mRuntimeStatus[5];

#pragma DATA_SEG __RPAGE_SEG PAGED_RAM_DTU

/// \name  GPSUpdate GPS更新相关.
/// @{
/// GPS更新的周期
#define UPDATE_GPS_INTERVAL (OS_TICKS_PER_SEC * 6UL)
static INT32U mTickToUpdateGPS = 0;
///  @}

/// \name NetStatusCheck 网络检查相关.
/// @{
#define CHECK_UNACKED_INTERVAL (OS_TICKS_PER_SEC * 9UL)
static INT32U mTickCheckUnackedSize = 0;
#define CHECK_UNACKED_ALLOW_MAX_LT0 9
static INT8U mTimesUnacked = 0;
#define MAX_TIMES_DATA_SEND_FAIL 4
static INT8U mTimesSendFail = 0;
/// @}

/// \name SIMCard SIM卡相关.
/// @{
/// SIM卡相关信息.

typedef struct {
    char from[15];
    char content[72];
} sms_t;
static simcard_data_t mSimCardData;
///  @}

static unsigned char ota_connect_buf[50] ={0x10,0x30,0x0,0x04,'M','Q','T','T',0x04,0xc2,0x0,0x3c,0x0,
                                     0x11,'d','8','4','4','9','8','e','e','8','d','a','2','7','b','a',
                                     '5','4',0x0,0x06,'m','a','s','t','e','r',
                                     0x0,0x09,'m','a','s','t','e','r','1','2','3'};
                                                                                               
static unsigned char program_subscribe_buf[32] ={0x82,0x1e,0x0,0x01,0x00,0x19,'o','t','a','/','s','1',
                                         '9','/','d','8','4','4','9','8','e','e','8','d','a','2','7','b','a',
                                     '5','4',0x00};
                                     
static unsigned char config_subscribe_buf[32] ={0x82,0x1e,0x0,0x01,0x00,0x19,'o','t','a','/','c','f',
                                         'g','/','d','8','4','4','9','8','e','e','8','d','a','2','7','b','a',
                                     '5','4',0x00};
                                     
static unsigned char csf_subscribe_buf[32] ={0x82,0x1e,0x0,0x01,0x00,0x19,'o','t','a','/','c','s',
                                         'f','/','d','8','4','4','9','8','e','e','8','d','a','2','7','b','a',
                                     '5','4',0x00};                                     
                                     
static unsigned char rule_subscribe_buf[33] ={0x82,0x1f,0x0,0x01,0x00,0x1a,'o','t','a','/','r','u',
                                         'l','e','/','d','8','4','4','9','8','e','e','8','d','a','2','7','b','a',
                                     '5','4',0x00};
                                     
static unsigned char ota_client_id[17] ={0};                                                                         
                                         
static unsigned char ping_buf[2] ={0xc0,0x00};
                                                                         
static INT8U mqtt_data_type = 0;
static INT8U mqtt_data_ready = 1;
static INT8U upgrade_data_ready = 0;
static INT8U cipsend_mode = 0;
static INT8U upgrade_data_over = 0;

J1939RecMessage g_otaconfigdata;
RuleItem g_ota_upper_computer_rule_item;
INT16U g_ota_rule_writer_last_pack_index;

//static INT8U mqtt_data_wait_handle = 0;

#pragma DATA_SEG __RPAGE_SEG PAGED_RAM_DTU3             
static unsigned char mSerialRecvLine[500];
static unsigned char mUpgradeData[500];
#pragma DATA_SEG __RPAGE_SEG PAGED_RAM_DTU

static INT16U mSerialRecvLineIndex;
static INT16U mUpgradeDataIndex;

static OS_EVENT *mEvent;

/// \name FrameHead 发送数据帧头
/// @{

/// 数据帧头格式.
typedef struct {
    char header[5]; // "DTUPF"
    unsigned char version;
    unsigned char length[2];
    unsigned char phone[11];
    unsigned char gps_s;
    unsigned char gps_longitude[4];
    unsigned char gps_latitude[4];
    unsigned char gps_hight[2];
    unsigned char gps_speed[2];
    unsigned char gps_bearing[2];
    unsigned char pgn[3];
} frame_header_t;


/// 这个变量保存数据发送时, 已经发送的数据量.
//static INT16U mGetDataOffset;

/// 数据帧头.
static frame_header_t mFrameHead;
///  @}

#define DATA_BUFFER_SIZE 128
RINGBUFFER_DEFINE_STATIC(mReceivedMessage, DATA_BUFFER_SIZE);
#define RECV_BUFFER_SIZE 256
RINGBUFFER_DEFINE_STATIC(mSerialRecvBuffer, RECV_BUFFER_SIZE);



static void runtimeStatusInit(void) {
    INT8U i;
    OS_CPU_SR cpu_sr = 0;
    OS_ENTER_CRITICAL();
    for (i = 0; i < sizeof(mRuntimeStatus); ++i) {
        mRuntimeStatus[i] = DTUM35_RUNTIME_STATUS_UNKNOWN;
    }
    OS_EXIT_CRITICAL();
}

static void runtimeStatusSetCurrent(INT8U status) {
    OS_CPU_SR cpu_sr = 0;
    OS_ENTER_CRITICAL();
    mRuntimeStatus[0] = (mRuntimeStatus[0]  & 0xE0) | status;
    OS_EXIT_CRITICAL();
}

static void runTimeStatusSetCurrentREGWarn(void) {
    mRuntimeStatus[0] |= 0x80;
}


static void runtimeStatusPush(INT8U status) {
    INT8U i;
    OS_CPU_SR cpu_sr = 0;
    OS_ENTER_CRITICAL();
    for (i = sizeof(mRuntimeStatus) - 1; i != 0; --i) {
        mRuntimeStatus[i] = mRuntimeStatus[i - 1];
    }
    mRuntimeStatus[0] = status;
    OS_EXIT_CRITICAL();
}

/// \name FrameHead 发送数据帧头
/// @{

/// \brief frameHeaderPutU16 以传输字节序保存16位数据到指定地址.
///
/// \param buf 保存的地址.
/// \param val 需要保存的16位数据.
static void frameHeaderPutU16(unsigned char *__FAR buf, INT16U val) {
    *buf++ = (unsigned char)(val >> 8);
    *buf = (unsigned char)(val);
}

static void frameHeaderUpdateLACCI(void) {
    frameHeaderPutU16(mFrameHead.gps_hight, mLacCi[0]);
    frameHeaderPutU16(mFrameHead.gps_speed, mLacCi[1]);
}

/// \brief frameHeaderInit 初始化数据帧头.
static void frameHeaderInit(void) {
    //(void)memset(&mFrameHead, 0, sizeof(mFrameHead));
    (void)memcpy((unsigned char *__FAR)mFrameHead.header, (unsigned char *__FAR)"DTUPF", sizeof(mFrameHead.header));
    mFrameHead.version = 0x03;
    (void)memcpy((unsigned char *__FAR)mFrameHead.phone, (unsigned char *__FAR)SELF_UNKNOW_NUM, sizeof(mFrameHead.phone));
    frameHeaderUpdateLACCI();
}


#if 0
/// \brief frameHeaderPutU32 以传输字节序保存32位数据到指定地址.
///
/// \param buf 保存的地址.
/// \param val 需要保存的32位数据.
static void frameHeaderPutU32(unsigned char *__FAR buf, INT32U val) {
    *buf++ = (unsigned char)(val >> 24);
    *buf++ = (unsigned char)(val >> 16);
    *buf++ = (unsigned char)(val >> 8);
    *buf = (unsigned char)(val);
}
#endif

/// \brief frameHeaderSetPhoneNum 设置帧头中手机卡号的字段.
///
/// \param phone 把帧头中的手机卡号设置成这个值.
static void frameHeaderSetPhoneNum(const char *__FAR phone) {
    if (strlen(phone) != sizeof(mFrameHead.phone)) {
        return;
    }
    memcpy((unsigned char *__FAR)mFrameHead.phone, (unsigned char *__FAR)phone, sizeof(mFrameHead.phone));
}

/// \brief frameHeaderSetPGN 设置帧头中PGN的字段.
///
/// \param isMaster 是否把帧头中的PGN字段设置成主机.
//static void frameHeaderSetPGN(INT8U isMaster) {
//    mFrameHead.pgn[1] = isMaster ? 0x90 : 0xB0;
//}

/// \brief frameHeaderSetDataLength 设置帧头中数据长度的字段.
///
/// \param len 把帧头中数据长度自动设置成这个值 + 部分帧头的长度.
static void frameHeaderSetDataLength(INT16U len) {
    len += sizeof(frame_header_t) - STRUCT_OFFSET(frame_header_t, phone[0]);
    frameHeaderPutU16(mFrameHead.length, len);
}

#if 0
/// \brief frameHeaderUpdateGPS 设置帧头中位置信息部分.
///
/// \param pos 位置信息.
static void frameHeaderUpdateGPS(const DTUGPS_Position_t *pos) {
    if (0 == pos) {
        return;
    }
    frameHeaderPutU32(mFrameHead.gps_longitude, pos->longitude);
    frameHeaderPutU32(mFrameHead.gps_latitude, pos->latitude);
    frameHeaderPutU16(mFrameHead.gps_hight, pos->high);
    frameHeaderPutU16(mFrameHead.gps_speed, pos->speed);
    frameHeaderPutU16(mFrameHead.gps_bearing, pos->bearing);
}
#endif


INT16U DTUM35_ProtocolGetBCUHeader(unsigned char *__FAR dat, INT16U packed_dat_length) {
    mFrameHead.pgn[1] = 0x90;
    frameHeaderSetDataLength(packed_dat_length);
    memcpy(dat, (const unsigned char *__FAR)&mFrameHead, sizeof(mFrameHead));
    return sizeof(mFrameHead);
}
INT16U DTUM35_ProtocolGetBMUHeader(unsigned char *__FAR dat, INT16U packed_dat_length) {
    mFrameHead.pgn[1] = 0xB0;
    frameHeaderSetDataLength(packed_dat_length);
    memcpy(dat, (const unsigned char *__FAR)&mFrameHead, sizeof(mFrameHead));
    return sizeof(mFrameHead);
}

INT16U DTUM35_ProtocolGetHeaderLength(void) {
    return sizeof(mFrameHead);
}


///  @}


/// \brief stopModem 开启关闭800模块，流程都是key拉高1秒
static void setModem(void) {
    if (!DTUM35_INTERFACE_IS_VALID(bspInterface)) {
        return;
    }
    
    bspInterface->m35_set_pwrkey(FALSE);
    OSTimeDly(OS_TICKS_PER_SEC * 4 / 5);
    bspInterface->m35_set_pwrkey(TRUE);
    OSTimeDly(OS_TICKS_PER_SEC * 8 / 5);
    bspInterface->m35_set_pwrkey(FALSE);
    OSTimeDly(OS_TICKS_PER_SEC * 4 / 5);
    return;
}

/// \brief startAndConfigModem 开启M35模块.
///这里面先开启电源, 再测试AT命令, 然后做一些配置, 等待网络注册.
/// \return TRUE 启动M35模块成功; FALSE 启动M35模块失败.
static Bool startAndConfigModem() {
    dprintf("[DTUM35]: startAndConfigModem\n");
    if (!DTUM35_INTERFACE_IS_VALID(bspInterface)) {
        return FALSE;
    }
    runtimeStatusSetCurrent(DTUM35_RUNTIME_STATUS_POWERUP);
    
    /*初始化之前如果800在上电状态，需要先进行掉电处理*/
    while(bspInterface->m35_is_poweron()){ 
        setModem();
        if (!bspInterface->m35_is_poweron()) {
            OSTimeDly(OS_TICKS_PER_SEC * 5);
            break;
        }
        dprintf("[DTUM35]: Power has not been detected, will retry\n");
        OSTimeDly(OS_TICKS_PER_SEC * 3);
    }
    
    
    while (!bspInterface->m35_is_poweron()) {
        /*SIM800工作状态时需要保持enable低电平*/
        bspInterface->m35_power_enable(TRUE);
        OSTimeDly(OS_TICKS_PER_SEC);
        setModem();
        if (bspInterface->m35_is_poweron()) {
            OSTimeDly(OS_TICKS_PER_SEC * 5);
            break;
        }
        dprintf("[DTUM35]: Power has not been detected, will retry\n");
        OSTimeDly(OS_TICKS_PER_SEC * 3);
    } 
    runtimeStatusSetCurrent(DTUM35_RUNTIME_STATUS_CONFIGURE);
    dprintf("[DTUM35]: Power is on, test AT\n");
    
    /*上电后等待30s，防止可能出现的OK响应影响后面的流程*/
   // if (!ATCMD_RetryUntilExpect(NULL, "OK", 10, 3UL * OS_TICKS_PER_SEC)) {
      //  return FALSE;
   // }
    
    /*发送AT，接收到OK表示主机与800通信正常*/
    //(void)ATCMD_ExecCommand("AT", NULL, OS_TICKS_PER_SEC / 2);
    if (!ATCMD_RetryUntilExpect("AT", "OK", 10, 2UL * OS_TICKS_PER_SEC)){
         return FALSE;
    }

    // 关闭回显.
    if (!ATCMD_RetryUntilExpect("ATE0", "OK", 5, OS_TICKS_PER_SEC * 5 / 4)) {
    //    return FALSE;
    }

    // 设置AT+CREG?的输出格式.
    if (!ATCMD_RetryUntilExpect("AT+CREG=2", "OK", 5, OS_TICKS_PER_SEC * 5 / 4)) {
    //    return FALSE;
    }

    // 设置AT+CGREG?的输出格式.
    if (!ATCMD_RetryUntilExpect("AT+CGREG=0", "OK", 5, OS_TICKS_PER_SEC * 5 / 4)) {
    //    return FALSE;
    }

    // 设置短信为TEXT格式.
    if (!ATCMD_RetryUntilExpect("AT+CMGF=1", "OK", 5, OS_TICKS_PER_SEC * 5 / 4)) {
     //   return FALSE;
    }

    // 设置文本格式.
    if (!ATCMD_RetryUntilExpect("AT+CSDH=0", "OK", 5, OS_TICKS_PER_SEC * 5 / 4)) {
    //    return FALSE;
    }
    
    //设置新消息提示
    if (!ATCMD_RetryUntilExpect("AT+CNMI=2,2,0,0,0", "OK", 5, OS_TICKS_PER_SEC)) {
    //    return FALSE;
    }
    
    //ME不需要提供密码
    runtimeStatusSetCurrent(DTUM35_RUNTIME_STATUS_SIM_INIT);
    if (!ATCMD_RetryUntilExpect("AT+CPIN?", "+CPIN: READY", 10, OS_TICKS_PER_SEC)) {
    //    return FALSE;
    }

    runtimeStatusSetCurrent(DTUM35_RUNTIME_STATUS_WAIT_REGISTER);
    {   //GSM网络注册信息
        static const char *__FAR const creg_replys[] = {"+CREG: 2,1", "550B", "5A65"};
        if (!ATCMD_RetryUntilExpects("AT+CREG?", creg_replys, 30, OS_TICKS_PER_SEC)) {
            return FALSE;
        }
    }

    {     //GPRS网络注册
        static const char *__FAR const cgreg_replys[] = {"+CGREG: 0,1"};
        if (!ATCMD_RetryUntilExpects("AT+CGREG?", cgreg_replys, 30, OS_TICKS_PER_SEC)) {
            runTimeStatusSetCurrentREGWarn();
            dprintf("[M35]: Query CGREG failture\n");
        }
    }

    //配置短消息模式为文本模式
    if (!ATCMD_RetryUntilExpect("AT+CMGF=1", "OK", 5, OS_TICKS_PER_SEC)) {
   //     return FALSE;
    }

    //设置TE输入字符集为GSM格式
    if (!ATCMD_RetryUntilExpect("AT+CSCS=\"GSM\"", "OK", 5, OS_TICKS_PER_SEC)) {
   //     return FALSE;
    }

    //设置发送成功时显示">"，返回SEND OK
    if (!ATCMD_RetryUntilExpect("AT+CIPSPRT=0", "OK", 5, OS_TICKS_PER_SEC / 2)){
        return FALSE;
    }
    return TRUE;
}


static void getOpsData(void) {
    char ops[32];
    if (!ATCMD_GetOPS(ops, sizeof(ops))) {
        return;
    }

    if (0 == strcmp(ops, "CHINA MOBILE")) {
        mOps = DTU_OPS_CHINA_MOBILE;
    } else if (0 == strcmp(ops, "CHINA UNIONCOM")) {
        mOps = DTU_OPS_CHINA_UNIONCOM;
    } else if (0 == strcmp(ops, "CHINA TELECOM")) {
        mOps = DTU_OPS_CHINA_TELECOM;
    } else {
        mOps = DTU_OPS_UNKNOWN;
    }
}

static void getIMEI(void) {
    unsigned char i;
    char buf[sizeof(mIMEI)];
    memset(buf, 0, sizeof(buf));
    for (i = 0; i < 10; ++i) {
        if (ATCMD_GetIMEI(buf, sizeof(buf) - 1)) {
            (void)strcpy(mIMEI, buf);
            return;
        }
    }
}


void dtum35_SetSIMInfo(const char *__FAR oa, const char *__FAR args) {
    // args: 13867678787,[cimi]
    if (memcmp((const unsigned char *__FAR)oa, (const unsigned char *__FAR)"+86", 3) == 0) {
        oa = oa + 3;
    }
    if (memcmp((const unsigned char *__FAR)oa, (const unsigned char *__FAR)SMS_SERVER_NUM, 11) != 0) {
        return;
    }
    if (strcmp(&args[12], mSimCardData.cimi) != 0) {
        return;
    }
    if (!ExString_NIsDigitString(args, 11)) {
        return;
    }
    memcpy((unsigned char *__FAR)mSimCardData.phone, (const unsigned char *__FAR)args, 11);
    mSimCardData.phone[11] = 0;
    frameHeaderSetPhoneNum(mSimCardData.phone);

    if (!DTUM35_INTERFACE_IS_VALID(bspInterface)) {
        return;
    }
    (void)bspInterface->nvm_store_data(&mSimCardData);
}                               

/// \brief getsimcard_data_t 获取SIM卡相关信息.
///
/// \return TRUE 获取成功; FALSE 获取失败.
/*M35有使用，800没有使用，防止编译报警先注释掉*/

static Bool getSimCardData(void) {
    char buf[64];
    simcard_data_t *sim = (simcard_data_t *)buf;

    if (!ATCMD_GetCIMI(buf, sizeof(mSimCardData.cimi))) {
        (void)strcpy(mSimCardData.phone, SELF_UNKNOW_NUM); // default phone number.
        return FALSE;
    } else {
        buf[20] = 0;
        (void)strcpy(mSimCardData.cimi, buf);
        if (strlen(mSimCardData.cimi) > (sizeof(mSimCardData.phone) - 1)) {
            (void)strcpy(mSimCardData.phone, &mSimCardData.cimi[strlen(mSimCardData.cimi) - (sizeof(mSimCardData.phone) - 1)]);
        } else {
            (void)strcpy(mSimCardData.phone, &mSimCardData.cimi[0]);
        }
        return TRUE;
        //dprintf("[M35]: CIMI = %s\n", mSimCardData.cimi);
    }

#if 0
    if (!DTUM35_INTERFACE_IS_VALID(bspInterface)) {
        return FALSE;
    }

    memset(buf, 0, sizeof(buf));
    (void)bspInterface->nvm_restore_data((INT8U *)sim, sizeof(simcard_data_t));
    if (strcmp(mSimCardData.cimi, sim->cimi) == 0) {
        (void)strcpy(mSimCardData.phone, sim->phone); // phone number store on NVM
        return TRUE;
    }

    (void)strcpy(buf, "WHOAMI:");
    if (ATCMD_GetIMEI(&buf[7], sizeof(buf) - 7)) {
        (void)strcpy(&buf[7], ",");
    } else {
        (void)strcpy(&buf[7], "NONE,");
    }

    (void)strcpy(buf + strlen(buf), mSimCardData.cimi);
#if 1
    return TRUE;
#else
    runtimeStatusSetCurrent(DTUM35_RUNTIME_STATUS_SEND_SMS);
    if (!ATCMD_SendSMS(SMS_SERVER_NUM, buf)) {
        dprintf("[M35]: Send sms error.\n");
        //return FALSE;
    }
    return TRUE; // if debug return TRUE;
#endif
#endif
}




static long mDataSentAckedSize = 0;

/// \brief connectionIsOK 检测连接是否畅通
/// 如果有更多的数据得到服务器确认 ==> 链接保持畅通;
/// 如果有未得到服务器确认的数据, 并没有更多的数据发送出去 ==> 链接可能已经断开.
///
/// \return FALSE 链接保持畅通; FALSE 链接保持畅通.
/// \note 如果连续90秒返回值都FALSE, 那么网络可能出现问题, 需要重启.
/*M35有使用，800没有使用，防止编译报警先注释掉*/
#if 0
static Bool connectionIsOK(void) {
    unsigned long acked;
    unsigned long unacked;

    if (!ATCMD_GetDataSentInfo(NULL, &acked, &unacked)) {
        // 如果应答错误, 认为连接可能不正常.
        return FALSE;
    }

    if (acked > mDataSentAckedSize) {
        mDataSentAckedSize = acked;
        return TRUE;
    }

    if (unacked > 0) {
        return FALSE;
    }

    return TRUE;
}
#endif


/// \name RestartWaitTime 发生错误时, 延时重启M35的延时函数.
/// @{
/// 连续重启到不成功的计数.

static INT8U mWaitTimeIndexForRestart;
/// \brief resetCounters 清零一些计数器
/// \note 一旦连接服务器成功, 需要调用这个函数.
static void resetCounters(void) {
    mWaitTimeIndexForRestart = 0;
    mDataSentAckedSize = 0;
    mTimesUnacked = 0;
    mTimesSendFail = 0;
}

static void reinitDtuInfo(void) {
    mOps = DTU_OPS_UNKNOWN;
    mSignalStrength = 99;
    mBitErrorRate = 7;
    mLacCi[0] = 0xffff;
    mLacCi[1] = 0xffff;
    (void)strcpy(mSimCardData.cimi, "UNKNOWN CIMI");
    (void)strcpy(mSimCardData.phone, "18812345678");
    (void)strcpy(mIMEI, "UNKNOWN IMEI");
}

/// \brief waitSomeTimeForRestart 等待一段时间再, 等待时间与连续重启不成功计数器相关.
static void waitSomeTimeForRestart(void) {
    static const INT32U timeWaitTableForRestart[] = {
        OS_TICKS_PER_SEC * 10UL,
        OS_TICKS_PER_SEC * 30UL,
        OS_TICKS_PER_SEC * 100UL,
        OS_TICKS_PER_SEC * 300UL,
        OS_TICKS_PER_SEC * 900UL,
        OS_TICKS_PER_SEC * 1800UL,
        OS_TICKS_PER_SEC * 3600UL
    };

    volatile INT32U now;
    INT32U tickend = OSTimeGet() + timeWaitTableForRestart[mWaitTimeIndexForRestart];

    for (now = OSTimeGet(); now < tickend; now = OSTimeGet()) {
        INT16U thisWait = tickend - now > 32767 ? 32767 : (INT16U)((INT32U)(tickend - now));
        //dprintf("[M35]: Wait %d tick for restart ...\n", thisWait);
        OSTimeDly(thisWait);
    }

    if (mWaitTimeIndexForRestart < (ARRAY_SIZE(timeWaitTableForRestart) - 1)) {
        ++mWaitTimeIndexForRestart;
    }
}
///  @}

   /*M35有使用，800没有使用，防止编译报警先注释掉*/
//#if 0
static char taskUpdateSignalStrength(void) {
    unsigned char strength;
    unsigned char biterror_rate;
    if (OSTimeGet() < mTickToUpdateSignalStrength) {
        return 1;
    }
    runtimeStatusSetCurrent(DTUM35_RUNTIME_STATUS_GET_SIGNAL_QUALITY);
    // 更新信号强度和误码率.
    if (ATCMD_GetSignalQuality(&strength, &biterror_rate)) {
        // 设置下次更新的时间.
        OS_CPU_SR cpu_sr = 0;
        mTickToUpdateSignalStrength = OSTimeGet() + UPDATE_SIGNAL_STRENGTH_INTERVAL;
        OS_ENTER_CRITICAL();
        mSignalStrength = strength;
        mBitErrorRate = biterror_rate;
        OS_EXIT_CRITICAL();
    }
    return 1;
}

#if 0
static char taskCheckConnection(void) {
    if (OSTimeGet() < mTickCheckUnackedSize) {
        return 1;
    }
    // 检测链接是否畅通, 如果多次检测到连接不畅通, 则需要重启模块.
    runtimeStatusSetCurrent(DTUM35_RUNTIME_STATUS_CHECK_CONNECTION);
    if (connectionIsOK()) {
        mTimesUnacked = 0;
    } else if (++mTimesUnacked > CHECK_UNACKED_ALLOW_MAX_LT0) {
        return 0;
    }

    // 设置下次检测时间.
    mTickCheckUnackedSize = OSTimeGet() + CHECK_UNACKED_INTERVAL;
    return 1;
}

#endif


INT32U DTUM35_GetPacketCounter(void) {
    INT32U ret;
    OS_CPU_SR cpu_sr = 0;
    OS_ENTER_CRITICAL();
    ret = mPacketCounter;
    OS_EXIT_CRITICAL();
    return ret;
}


/********************************************************************************
* Function Name: calcCrc16校验算法
*                
* Description:    CRC16校验
*                             
* Inputs:        data[]表示传入的数组offset表示需要校验数组元素偏移量
                 Len是以offset为起始位的偏移量preval表示其实参数 
*                
* Outputs:       None
* 
* Limitations:   None
********************************************************************************/

static UINT16 calcCrc16(unsigned char *__FAR data, UINT16 offset, UINT16 len, UINT16 preval)  /*preval默认值为0xffff*/
{  
    UINT16 ucCRCHi = (preval & 0xff00) >> 8;  
    UINT16 ucCRCLo = preval & 0x00ff;  
    UINT16 iIndex; 
    UINT16 i=0; 
    const uchar *__FAR table_h = NULL;
    const uchar *__FAR table_l = NULL;
    
    table_h = (uchar *__FAR)crc16_tab_h;
    table_l = (uchar *__FAR)crc16_tab_l;
    
    for (i = 0; i < len; ++i) 
    {  
        iIndex = (ucCRCLo ^ data[offset + i]) & 0x00ff;  
        ucCRCLo = ucCRCHi ^ table_h[iIndex];  
        ucCRCHi = table_l[iIndex];  
    }  
    return ((ucCRCHi & 0x00ff) << 8) | (ucCRCLo & 0x00ff) & 0xffff;  
}  



static UINT32 upgrade_change_address(UINT32 address)  /*preval默认值为0xffff*/
{ 
    UINT32 cur_change_address = 0;
     
    if((address >= 0x780000) && (address <= 0x78ffff)) 
    {         
        cur_change_address = address + 0x5c000;
        return cur_change_address; 
    }
    
    if((address >= 0x7f0000) && (address <= 0x7f3fff)) 
    {         
        cur_change_address = address - 0x4000;
        return cur_change_address; 
    }
    
    if((address >= 0x7f4000) && (address <= 0x7f7fff)) 
    {         
        cur_change_address = address - 0x1c000;
        return cur_change_address; 
    }
    
    if((address >= 0x7fc000) && (address <= 0x7fffff)) 
    {         
        cur_change_address = address - 0x28000;
        return cur_change_address; 
    }
     
    return address;  
} 

                                                          
static char upgrade_write_data(UINT32 address, unsigned char *__FAR ptr,UINT16 length)
{ 
    UINT8  err=0;
    UINT16 data_index = 0;
    UINT8  upgrade_temp_buffer[8] = {0};
    OS_INIT_CRITICAL();
    
    while(length > data_index) 
    {
        if(length-data_index >= 8) 
        {
            safe_memcpy((PINT8U)upgrade_temp_buffer, &(ptr[data_index]), 8);
            OS_ENTER_CRITICAL();
            while(PFlash_Program(address+data_index, (UINT16 *)upgrade_temp_buffer, 8)!=RES_OK)
            {
                err++;
                if(err >= 5)
                {
                    OS_EXIT_CRITICAL();
                    return RES_ERR;
                }
            } 
            data_index = data_index + 8;
            OS_EXIT_CRITICAL(); 
        } 
        else 
        {
            return RES_ERR;
        }
    }
            
    return RES_OK; 
}  

/********************************************************************************
* Function Name:upgrade_erase_data
*               
* Description: 根据头文件定义的宏UPGRADEDATA_ERASE_PART选择擦除A或者B的扇区和公共部分 
*               
* Inputs:       
                
*               
* Outputs:      
* 
* Limitations:  
********************************************************************************/

static char upgrade_erase_s19_data(void)
{ 
    UINT32 Address;
    UINT8 Error;
    
    OS_INIT_CRITICAL();
    //擦除扇区F5-FA，作为公共代码的临时存放扇区  
    for(Address = 0x7D4000; Address < 0x7EBFFF; Address += 0x400)
    {
        OS_ENTER_CRITICAL();
        Error = PFlash_EraseSector(Address);
        OS_EXIT_CRITICAL();
        if(Error != 0) 
        {
            return(Error);
        }
        OSTimeDly(100);
    }
        
    if(0xa == OTA_S19_PART) 
    { 
        //擦除扇区D0-DF      
        for(Address = 0x740000; Address < 0x77FFFF; Address += 0x400)
        {
            OS_ENTER_CRITICAL();
            Error = PFlash_EraseSector(Address);
            OS_EXIT_CRITICAL();
            if(Error != 0) 
            {
                return(Error);
            }
            OSTimeDly(100);
        }    
         //擦除扇区EB-ED
        for(Address = 0x7AC000; Address < 0x7B7FFF; Address += 0x400)
        {
            OS_ENTER_CRITICAL();
            Error = PFlash_EraseSector(Address);
            OS_EXIT_CRITICAL();
            if(Error != 0) 
            {
                return(Error);
            }
            OSTimeDly(100);
        }        
        
    }      
    else if(0xb == OTA_S19_PART) 
    {
       //擦除扇区C0-CF 
        for(Address = 0x700000; Address < 0x73FFFF; Address += 0x400)
        {
            OS_ENTER_CRITICAL();
            Error = PFlash_EraseSector(Address);
            OS_EXIT_CRITICAL();
            if(Error != 0) 
            {
                return(Error);
            }
            OSTimeDly(100);
        }
        //擦除扇区E8-EA
        
        for(Address = 0x7A0000; Address < 0x7ABFFF; Address += 0x400)
        {
            OS_ENTER_CRITICAL();
            Error = PFlash_EraseSector(Address);
            OS_EXIT_CRITICAL();
            if(Error != 0) 
            {
                return(Error);
            }
            OSTimeDly(100);
        }
             
        
    }
    
    
    //擦除扇区EF-F4，作为公共代码的临时存放扇区  
    for(Address = 0x7BC000; Address < 0x7D3FFF; Address += 0x400)
    {
        OS_ENTER_CRITICAL();
        Error = PFlash_EraseSector(Address);
        OS_EXIT_CRITICAL();
        if(Error != 0) 
        {
            return(Error);
        }
        OSTimeDly(100);
    }
    
    if(upgrade_write_data((INT32U)0x7BC000, (unsigned char *__FAR)0x780000,0x4000)) return RES_ERR;
    if(upgrade_write_data((INT32U)0x7C0000, (unsigned char *__FAR)0x784000,0x4000)) return RES_ERR;
    if(upgrade_write_data((INT32U)0x7C4000, (unsigned char *__FAR)0x788000,0x4000)) return RES_ERR;
    if(upgrade_write_data((INT32U)0x7C8000, (unsigned char *__FAR)0x78C000,0x4000)) return RES_ERR;
    if(upgrade_write_data((INT32U)0x7CC000, (unsigned char *__FAR)0x7F4000,0x4000)) return RES_ERR;
    if(upgrade_write_data((INT32U)0x7D0000, (unsigned char *__FAR)0x7FC000,0x3000)) return RES_ERR;    
    
    return RES_OK;
}



/********************************************************************************
* Function Name:upgrade_erase_data
*               
* Description: 根据头文件定义的宏UPGRADEDATA_ERASE_PART选择擦除A或者B的扇区和公共部分 
*               
* Inputs:       
                
*               
* Outputs:      
* 
* Limitations:  
********************************************************************************/

static char upgrade_check_address(UINT32 address)
{     

    if(0xb == OTA_S19_PART) 
    {    
        if((address >= 0x740000) && (address <= 0x77FFFF))
        {
            return RES_ERR;
        }    

        if((address >= 0x7AC000) && (address <= 0x7B7FFF))
        {
            return RES_ERR;
        }        
        
    } 
    else if(0xa == OTA_S19_PART) 
    {

        if((address >= 0x700000) && (address <= 0x73FFFF))
        {
            return RES_ERR;
        }

        if((address >= 0x7A0000) && (address <= 0x7ABFFF))
        {
            return RES_ERR;
        }
        
        
    }   
    
    return RES_OK;
}

/*s19数据处理接口*/
static UINT8 taskOtaProgramCmd(void) 
{
    UINT8  err=0;
    UINT8  end_flag=0;
    UINT8  start_flag=0;
    UINT16 length = 0;
    UINT8  long_flag = 0;
    UINT16 cmpVal = 0;
    UINT16 data_over = 0;
    UINT32 cur_upgrade_address = 0;
   // UINT32 pre_upgrade_address= (UINT32)config_get(kEraseSerialNumber1)<<16 |(UINT32)config_get(kEraseSerialNumber2); 
    OS_INIT_CRITICAL();
      
    /*判断接收的字节长度，如果字节数最高位置位1，则表示长度大于256，否则小于256*/
    if(0x80 == (mUpgradeData[1]&0x80)) 
    {
        /*mUpgradeData[4]为topic长度*/
        /*mUpgradeData[1]和mUpgradeData[2]为报文可变头加数据和校验位长度*/
        long_flag = TRUE;
        length = (UINT16)((mUpgradeData[1]&0x7f)+mUpgradeData[2]*128 - mUpgradeData[4] - 10);/*表示写入的长度，除去校验长度、Topic长度以及长度数目*/
        cmpVal = ((UINT16)(mUpgradeData[(mUpgradeData[1]&0x7f)+mUpgradeData[2]*128+1]<<8)) | ((UINT16)mUpgradeData[(mUpgradeData[1]&0x7f)+mUpgradeData[2]*128+2]); /*提取校验值*/
        cur_upgrade_address=(UINT32)((UINT32)mUpgradeData[6+mUpgradeData[4]]<<16|(UINT32)mUpgradeData[7+mUpgradeData[4]]<<8|(UINT32)mUpgradeData[8+mUpgradeData[4]]);
    }
    else
    {
        /*mUpgradeData[3]为topic长度*/
        /*mUpgradeData[1]为报文可变头加数据和校验位长度*/
        long_flag = FALSE;
        length = (UINT16)(mUpgradeData[1] - mUpgradeData [3] - 10);
        cmpVal = ((UINT16)(mUpgradeData[mUpgradeData[1]] << 8)) | ((UINT16)mUpgradeData[mUpgradeData[1]+1]);
        cur_upgrade_address=(UINT32)((UINT32)mUpgradeData[5+mUpgradeData[3]]<<16|(UINT32)mUpgradeData[6+mUpgradeData[3]]<<8|(UINT32)mUpgradeData[7+mUpgradeData[3]]);
    } 
    
    /*检测地址为当前软件使用的地址时，返回错误*/ 
    if(RES_OK != upgrade_check_address(cur_upgrade_address)) 
    {   
        return RES_ERR;     
    }
        

    if(0 == length) 
    {
        return RES_ERR;
    }
     
    /*srartup的特殊报文，需要存放在特殊位置并且注意按8字节补位的情况*/   
    if((0x7ffffe == cur_upgrade_address) &&(8 == length)) 
    {
        cur_upgrade_address = cur_upgrade_address - 6;    
    }
    
    /*地址为公共部分的数据，放到临时区域存放，需要做地址转换*/    
    cur_upgrade_address = upgrade_change_address(cur_upgrade_address); 
         
    
    if(TRUE == long_flag) /*接收到的长度大于128*/ 
    {   
        /***对接收到的写数据进行CRC16验算***/    
        if(cmpVal != calcCrc16(mUpgradeData,(UINT16)(5+mUpgradeData[4]),length+6,0xffff))
        {
            return RES_ERR; 
        }
               
        return upgrade_write_data(cur_upgrade_address, mUpgradeData+11+mUpgradeData[4], length); 
    }  
    else                  /*接收到的字节数小于128*/
    {
        /***对接收到的写数据进行CRC16验算***/
        if(cmpVal!=calcCrc16(mUpgradeData,(UINT16)(4+mUpgradeData[3]),length+6,0xffff))
        {
            return RES_ERR;
        }
        
        /***接收到16个字节的0xff为结束标志***/    
        while(0xFF == mUpgradeData[end_flag+10+mUpgradeData[3]]) 
        {
            end_flag++;
            if(end_flag >= 8) 
            {
                data_over = config_get(kOtaOverFlag); 
                config_save(kOtaOverFlag,data_over|0x1);
                upgrade_data_over = 1;    
                return RES_OK;
            }
            
        }
        
         /***接收到16个字节的0x0为开始标志***/
        while(0x0 == mUpgradeData[start_flag+10+mUpgradeData[3]]) 
        {
            start_flag++;
            if(start_flag >= 8) 
            {
                upgrade_data_over = 0;
                return upgrade_erase_s19_data();
            }
            
        }
            
        return upgrade_write_data(cur_upgrade_address, mUpgradeData+10+mUpgradeData[3], length); 
    }  
}      

/*mqtt反馈数据填充，数据格式：消息类型+报文长度+topic长度+topic+消息体*/
static void Ota_Program_Data_Fill(UINT8 err_flag) 
{

    UINT16 cmpVal = 0;
          
    mUpgradeData[0] = 0x30;
    mUpgradeData[1] = 0x22;
    mUpgradeData[2] = 0x0;
    mUpgradeData[3] = 0x1c;
    safe_memcpy((PINT8U)(mUpgradeData+4), (PINT8U)"ota/s19ack/", 11);
    safe_memcpy((PINT8U)(mUpgradeData+15), (PINT8U)ota_client_id, 17);
    mUpgradeData[32] = 0x0;
    mUpgradeData[33] = err_flag;
    cmpVal = calcCrc16(mUpgradeData,32,2,0xffff);
    mUpgradeData[34] = (UINT8)(cmpVal >> 8);
    mUpgradeData[35] = (UINT8)(cmpVal & 0xff);
    
    mUpgradeDataIndex = 36; 
    
    return;   

}





             /*
static INT16U Ota_Config_Pgn_Num_Get(INT8U pgn_index) 
{
    switch(pgn_index)
    {
        case 0 :
            return BMS_DEVICE_RELAY_PGN;
            break;
        case 1:
            return BCU_BASE_CONFIG_PGN;
            break;
        case 2: 
            return BCU_COMMON_ALARM_CONFIG_PGN;
            break;
        case 3:
            return BCU_VOLT_ALARM_CONFIG_PGN;
            break;
        case 4 :
            return BCU_CURRENT_ALARM_CONFIG_PGN;
            break;
        case 5 :
            return BCU_TEMPERATURE_ALARM_CONFIG_PGN;
            break;
        case 6:
            return BCU_SOC_ALARM_CONFIG_PGN;
            break;
        case 7: 
            return BCU_INSU_ALARM_CONFIG_PGN;
            break;
        case 8:
            return BCU_INSU2_ALARM_CONFIG_PGN;
            break;
        case 9 :
            return BCU_RELAY_CONFIG_PGN;
            break;
        case 10 :
            return BCU_HEAT_COOL_CONFIG_PGN;
            break;
        case 11:
            return BCU_CHARGER_CONFIG_PGN;
            break; 
        case 12 :
            return BCU_CHG_VOLT_ALARM_CONFIG_PGN;
            break;
        case 13 :
            return BCU_CHG_TEMP_ALARM_CONFIG_PGN;
            break;
        case 14:
            return BCU_MLOCK_CONTROL_PGN;
            break;
        case 15: 
            return BCU_CHGR_OUTLET_TEMP_CONFIG_PGN;
            break;
        case 16:
            return BMU_BASE_CONFIG_PGN;
            break;
        case 17 :
            return BMU_RELAY_CONFIG_PGN;
            break;
        case 18:
            return BMU_ALARM_CONFIG_PGN;
            break;
        case 19 :
            return BMU_BALANCE_CONFIG_PGN;
            break;
        case 20:
            return DTU_CONFIG_MESSAGE_PGN;
            break;
        case 21 :
            return BMU_5160_CONFIG_PGN;
            break;
        default:
            return 0xffff;
        }
}       */

static INT8U update_config_data(INT16U pgn_num) 
{                    
    switch(pgn_num)        
    {
        case BMS_DEVICE_RELAY_PGN :
            return bms_config_device_relay_info(&g_otaconfigdata);
            break;
        case BCU_BASE_CONFIG_PGN:
            return bcu_base_config_update(&g_otaconfigdata);
            break;
        case BCU_COMMON_ALARM_CONFIG_PGN: 
            return bcu_common_alarm_config_update(&g_otaconfigdata);
            break;
        case BCU_VOLT_ALARM_CONFIG_PGN:
            return bcu_volt_alarm_config_update(&g_otaconfigdata);
            break;
        case BCU_CURRENT_ALARM_CONFIG_PGN :
            return bcu_current_alarm_config_update(&g_otaconfigdata);
            break;
        case BCU_TEMPERATURE_ALARM_CONFIG_PGN :
            return bcu_temperature_alarm_config_update(&g_otaconfigdata);
            break;
        case BCU_SOC_ALARM_CONFIG_PGN:
            return bcu_soc_alarm_config_update(&g_otaconfigdata);
            break;
        case BCU_INSU_ALARM_CONFIG_PGN: 
            return bcu_insu_alarm_config_update(&g_otaconfigdata);
            break;
        case BCU_INSU2_ALARM_CONFIG_PGN:
            return bcu_insu2_alarm_config_update(&g_otaconfigdata);
            break;
        case BCU_RELAY_CONFIG_PGN :
            return bcu_relay_config_update(&g_otaconfigdata);
            break;
        case BCU_HEAT_COOL_CONFIG_PGN :
            return bcu_heat_cool_config_update(&g_otaconfigdata);
            break;
        case BCU_CHARGER_CONFIG_PGN:
            return bcu_charger_config_update(&g_otaconfigdata);
            break; 
        case BCU_CHG_VOLT_ALARM_CONFIG_PGN :
            return bcu_chg_volt_alarm_config_update(&g_otaconfigdata);
            break;
        case BCU_CHG_TEMP_ALARM_CONFIG_PGN :
            return bcu_chg_temp_alarm_config_update(&g_otaconfigdata);
            break;
        case BCU_MLOCK_CONTROL_PGN:
            return bcu_mlock_config_update(&g_otaconfigdata);
            break;
        case BCU_CHGR_OUTLET_TEMP_CONFIG_PGN: 
            return bcu_chgr_outlet_config_update(&g_otaconfigdata);
            break;
        case BMU_BASE_CONFIG_PGN:
            return bmu_base_config_update(&g_otaconfigdata);
            break;
        case BMU_RELAY_CONFIG_PGN :
            return RES_OK;
            break;
        case BMU_ALARM_CONFIG_PGN:
            return RES_OK;
            break;
        case BMU_BALANCE_CONFIG_PGN :
            return bmu_balance_config_update(&g_otaconfigdata);
            break;
        case DTU_CONFIG_MESSAGE_PGN:
            return dtu_config_update(&g_otaconfigdata);
            break;
        case BMU_5160_CONFIG_PGN :
            return bmu_5160_config_update(&g_otaconfigdata);
            break;
        default:
            return RES_ERR;
        }
}

static UINT8 taskOtaConfigCmd(UINT16* upgrade_pgn_id, UINT8* upgrade_pgn_fill) 
{
    UINT8  err=0;
    UINT8  end_flag=0;
    UINT8  start_flag=0;
    UINT16 length = 0;
    UINT8  long_flag = 0;
    UINT8  cmd_type = 0;
    UINT16 cmpVal = 0;
      
    /*判断接收的字节长度，如果字节数最高位置位1，则表示长度大于256，否则小于256*/
    if(0x80 == (mUpgradeData[1]&0x80)) 
    {
        /*mUpgradeData[4]为topic长度*/
        /*mUpgradeData[1]和mUpgradeData[2]为报文可变头加数据和校验位长度*/
        long_flag = TRUE;
        length = (UINT16)((mUpgradeData[1]&0x7f)+mUpgradeData[2]*128 - mUpgradeData[4] - 4);/*表示写入的长度，除去校验长度、Topic长度以及长度数目*/
        cmpVal = ((UINT16)(mUpgradeData[(mUpgradeData[1]&0x7f)+mUpgradeData[2]*128+1]<<8)) | ((UINT16)mUpgradeData[(mUpgradeData[1]&0x7f)+mUpgradeData[2]*128+2]); /*提取校验值*/
        (*upgrade_pgn_id) = (UINT16)(mUpgradeData[5+mUpgradeData[4]]*256 + mUpgradeData[6+mUpgradeData[4]]);
    }
    else
    {
        /*mUpgradeData[3]为topic长度*/
        /*mUpgradeData[1]为报文可变头加数据和校验位长度*/
        long_flag = FALSE;
        length = (UINT16)(mUpgradeData[1] - mUpgradeData[3] - 4);
        cmpVal = ((UINT16)(mUpgradeData[mUpgradeData[1]] << 8)) | ((UINT16)mUpgradeData[mUpgradeData[1]+1]);
        (*upgrade_pgn_id) = (UINT16)(mUpgradeData[4+mUpgradeData[3]]*256 + mUpgradeData[5+mUpgradeData[3]]);
    } 
    
    /*消息体只含有pgn id，且pfn if为0，为开始标志*/
    if((2 == length) &&  (*upgrade_pgn_id == 0x0)) 
    {
        (*upgrade_pgn_fill) = 0;
        upgrade_data_over = 0;
        return RES_OK;
    } /*消息体只含有pgn id，且pfn if为0xff，为开始标志*/
    else if((2 == length) &&  (*upgrade_pgn_id == 0xffff)) 
    {
        (*upgrade_pgn_fill) = 0;
        upgrade_data_over = 1;
        return RES_OK;
    }/*消息体只含有pgn id，需要上传该pgn配置*/
    else if(2 == length)
    {
        (*upgrade_pgn_fill) = 1;
        return RES_OK; 
    }
    else if(0 == length)
    {
        return RES_ERR; 
    }
        
       
    if(TRUE == long_flag) /*接收到的长度大于128*/ 
    {    
         /***对接收到的写数据进行CRC16验算***/   
        if(cmpVal != calcCrc16(mUpgradeData,(UINT16)(5+mUpgradeData[4]),length,0xffff))
        {
            return RES_ERR; 
        }
        (*upgrade_pgn_fill) = 0;
        
        
        g_otaconfigdata.byte_max = UPPER_REC_MESSAGE_LENGTH_MAX;
        g_otaconfigdata.byte_cnt = length-2;
        g_otaconfigdata.data = &(mUpgradeData[7+mUpgradeData[4]]);    
        return update_config_data(*upgrade_pgn_id); 
    }  
    else                  /*接收到的字节数小于128*/
    {
        /***对接收到的写数据进行CRC16验算***/   
        if(cmpVal!=calcCrc16(mUpgradeData,(UINT16)(4+mUpgradeData[3]),length,0xffff))
        {
            return RES_ERR;
        }
        (*upgrade_pgn_fill) = 0;
        /*解析配置文件并去配置到相关E2P*/
        g_otaconfigdata.byte_max = UPPER_REC_MESSAGE_LENGTH_MAX;
        g_otaconfigdata.byte_cnt = length-2;
        g_otaconfigdata.data = &(mUpgradeData[6+mUpgradeData[3]]);      
        return update_config_data(*upgrade_pgn_id); 
    }
     
}      



INT16U ota_bms_query_relay_info_fill(unsigned char *__FAR buf)
{
    RelayCtlStatus status;
    INT16U offset = 0;
    INT16U byte_max = 0xffff;
    INT8U index, relay_num = relay_count();

    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, offset, 0, byte_max);

    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, offset, relay_num, byte_max);
    for (index = 1; index <= relay_num; ++index)
    {
        WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, offset, relay_is_on(index), byte_max);
        WRITE_LT_INT8U_WITH_BUFF_SIZE(
                                    buf,
                                    offset,
                                    relay_control_relay_type_to_computer_display(relay_control_get_type(index)),
                                    byte_max);
    }
    for (index = 1; index <= relay_num; ++index)
    {
        status = relay_control_get_force_command(relay_control_get_type(index));
        if(status == kRelayForceOn)
            WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, offset, 0x01, byte_max)
        else if(status == kRelayForceOff)
            WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, offset, 0x02, byte_max)
        else
            WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, offset, 0x00, byte_max)
    }
    relay_num = input_signal_get_count(); 
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, offset, relay_num, byte_max);
    for (index = 1; index <=relay_num; ++index)
    {
        WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, offset, input_signal_is_low(index), byte_max);
        
        WRITE_LT_INT8U_WITH_BUFF_SIZE(
                                    buf,
                                    offset,
                                    input_signal_control_type_to_computer_display(input_signal_control_get_type(index)),
                                    byte_max);
    }

    return offset;
}


INT16U ota_bcu_base_config_fill(unsigned char *__FAR buf)
{
    INT8U index = 0;
    INT16U macro_temp;
    INT16U byte_max = 0xffff;

    buf[index++] = 0x00;

    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, config_get(kSlaveNumIndex), byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, 0, byte_max);//TODO:BMU监测电池数目  bmu_get_voltage_num(0)
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, SLAVE_TEMPERA_NUM_DEF, byte_max);//config_get(kTemperatureNumIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, bcu_get_current_auto_check_flag(), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kNominalCapIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kTotalCapIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kLeftCapIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kSystemVoltageCalibration), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, ALARM_BEEP_INTERVAL_DEF, byte_max);//config_get(kAlarmBeepInterval)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, ALARM_BEEP_MASK_LOW_DEF, byte_max);//config_get(kAlarmBeepMaskLow)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, ALARM_BEEP_MASK_HIGH_DEF, byte_max);//config_get(kAlarmBeepMaskHigh)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kBmuBalanceEnableIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kCurSenTypeIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kCapCalibHighAverageVoltage), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kCapCalibLowAverageVoltage), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kCapCalibLowSoc), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kCapCalibLowSocDly), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kReqBmuMessageIntervalIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kBmuResponseTimeoutIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kCurZeroFilterIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, SOC_CURRENT_SAMPLE_PERIOD, byte_max);//config_get(kCurSampleintervalIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, SOC_CURRENT_SAMPLE_COUNT, byte_max);//config_get(kCurSampleCntIndex)
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, config_get(kInsulationType), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, BATTERY_TOTAL_NUM_DEF, byte_max);//config_get(kBatteryTotalNum)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kNominalTotalVolt), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, CHARGE_EFFICIENCY_FACTOR_DEF, byte_max);//config_get(kChargeEfficiencyFactor)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, DISCHARGE_EFFICIENCY_FACTOR_DEF, byte_max);//config_get(kDisChargeEfficiencyFactor)
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, config_get(kBmuMaxVoltageNumIndex), byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, config_get(kBmuMaxTempNumIndex), byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, config_get(kCAN1BpsIndex), byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, config_get(kCAN2BpsIndex), byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, config_get(kCAN3BpsIndex), byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, RS485_DTU_BPS_DEF, byte_max);//config_get(kRS4851BpsIndex)
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, RS485_SLAVE_BPS_DEF, byte_max);//config_get(kRS4852BpsIndex)
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, config_get(kBCUCommModeIndex), byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, config_get(kGBChargerConnectModeIndex), byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, config_get(kSlaveCanChannelIndex), byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, config_get(kChargerCanChannelIndex), byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, config_get(kEmsCanChannelIndex), byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, HMI_DEV_NAME, byte_max);//config_get(kHmiRS485ChannelIndex)
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, DTU_DEV_NAME, byte_max);//config_get(kDtuRS485ChannelIndex)
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, config_get(kCurSenRangeTypeIndex), byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, config_get(kCAN5BpsIndex), byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, DTU_CAN_DEV, byte_max);//config_get(kDtuCanChannelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kSysStatusSaveIntervalIndex), byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, config_get(kChgTotalTimeMaxIndex), byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, BMS_VIN_LEN, byte_max); //VIN码长度
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kVIN1Index), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kVIN2Index), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kVIN3Index), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kVIN4Index), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kVIN5Index), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kVIN6Index), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kVIN7Index), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kVIN8Index), byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, config_get(kVIN9Index), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kWakeupIntervalMinutes), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kWorkMaxTimeAfterWakeupSeconds), byte_max);    
    
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kCurSenType1Index), byte_max);    
    return index;
}

INT16U ota_bcu_common_alarm_fill(unsigned char *__FAR buf)
{
    INT8U index = 0;
    INT16U macro_temp;
    INT16U byte_max = 0xffff;

    buf[index++] = 0x00;

    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, CFG_SYS_VOLT_HIGH_DEF, byte_max);//config_get(kSystemVoltHVIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, CFG_SYS_VOLT_HIGH_DLY_DEF, byte_max);//config_get(kSystemVoltHVDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, CFG_SYS_VOLT_HIGH_DEF, byte_max);//config_get(kSystemVoltHVReleaseIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, CFG_SYS_VOLT_HIGH_DLY_DEF, byte_max);//config_get(kSystemVoltHVReleaseDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, CFG_SYS_VOLT_LOW_DEF, byte_max);//config_get(kSystemVoltLVIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, CFG_SYS_VOLT_LOW_DLY_DEF, byte_max);//config_get(kSystemVoltLVDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, CFG_SYS_VOLT_LOW_DEF, byte_max);//config_get(kSystemVoltLVReleaseIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, CFG_SYS_VOLT_LOW_DLY_DEF, byte_max);//config_get(kSystemVoltLVReleaseDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kCommFstAlarmDlyIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, COMM_FST_ALARM_REL_DLY, byte_max);//config_get(kCommFstAlarmRelDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, COMM_SND_ALARM_DLY_DEF, byte_max);//config_get(kCommSndAlarmDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, COMM_SND_ALARM_REL_DLY_DEF, byte_max);//config_get(kCommSndAlarmRelDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, COMM_TRD_ALARM_DLY_DEF, byte_max);//config_get(kCommTrdAlarmDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, COMM_TRD_ALARM_REL_DLY_DEF, byte_max);//config_get(kCommTrdAlarmRelDlyIndex)

    return index;
}

INT16U ota_bcu_volt_alarm_fill(unsigned char *__FAR buf)
{
    INT8U index = 0;
    INT16U macro_temp;
    INT16U byte_max = 0xffff;

    buf[index++] = 0x00;

    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kDChgHTVFstAlarmIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kDChgHTVFstAlarmDlyIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, DCHG_HTV_FST_ALARM_REL, byte_max);//config_get(kDChgHTVFstAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, DCHG_HTV_FST_ALARM_REL_DLY, byte_max);//config_get(kDChgHTVFstAlarmRelDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kDChgHTVSndAlarmIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kDChgHTVSndAlarmDlyIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, DCHG_HTV_SND_ALARM_REL, byte_max);//config_get(kDChgHTVSndAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, DCHG_HTV_SND_ALARM_REL_DLY, byte_max);//config_get(kDChgHTVSndAlarmRelDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kDChgLTVFstAlarmIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kDChgLTVFstAlarmDlyIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, DCHG_LTV_FST_ALARM_REL, byte_max);//config_get(kDChgLTVFstAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, DCHG_LTV_FST_ALARN_REL_DLY, byte_max);//config_get(kDChgLTVFstAlarmRelDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kDChgLTVSndAlarmIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kDChgLTVSndAlarmDlyIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, DCHG_LTV_SND_ALARM_REL, byte_max);//config_get(kDChgLTVSndAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, DCHG_LTV_SND_ALARM_REL_DLY, byte_max);//config_get(kDChgLTVSndAlarmRelDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kDChgHVFstAlarmIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kDChgHVFstAlarmDlyIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, DCHG_HV_FST_ALARM_REL, byte_max);//config_get(kDChgHVFstAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, DCHG_HV_FST_AlARM_REL_DLY, byte_max);//config_get(kDChgHVFstAlarmRelDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kDChgHVSndAlarmIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kDChgHVSndAlarmDlyIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, DCHG_HV_SND_AlARM_REL, byte_max);//config_get(kDChgHVSndAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, DCHG_HV_SND_AlARM_REL_DLY, byte_max);//config_get(kDChgHVSndAlarmRelDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kDChgLVFstAlarmIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kDChgLVFstAlarmDlyIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, DCHG_LV_FST_ALARM_REL, byte_max);//config_get(kDChgLVFstAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, DCHG_LV_FST_AlARM_REL_DLY, byte_max);//config_get(kDChgLVFstAlarmRelDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kDChgLVSndAlarmIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kDChgLVSndAlarmDlyIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, DCHG_LV_SND_ALARM_REL, byte_max);//config_get(kDChgLVSndAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, DCHG_LV_SND_AlARM_REL_DLY, byte_max);//config_get(kDChgLVSndAlarmRelDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kDChgDVFstAlarmIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kDChgDVFstAlarmDlyIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, DCHG_DV_FST_ALARM_REL, byte_max);//config_get(kDChgDVFstAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, DCHG_DV_FST_AlARM_REL_DLY, byte_max);//config_get(kDChgDVFstAlarmRelDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kDChgDVSndAlarmIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kDChgDVSndAlarmDlyIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, DCHG_DV_SND_AlARM_REL, byte_max);//config_get(kDChgDVSndAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, DCHG_DV_SND_AlARM_REL_DLY, byte_max);//config_get(kDChgDVSndAlarmRelDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kVLineFstAlarmDlyIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, VLINE_FST_ALARM_REL_DLY, byte_max);//config_get(kVLineFstAlarmRelDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kDChgHTVTrdAlarmIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kDChgHTVTrdAlarmDlyIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, DCHG_HTV_TRD_ALARM_REL, byte_max);//config_get(kDChgHTVTrdAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, DCHG_HTV_TRD_ALARM_REL_DLY, byte_max);//config_get(kDChgHTVTrdAlarmRelDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kDChgLTVTrdAlarmIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kDChgLTVTrdAlarmDlyIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, DCHG_LTV_TRD_ALARM_REL, byte_max);//config_get(kDChgLTVTrdAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, DCHG_LTV_TRD_ALARM_REL_DLY, byte_max);//config_get(kDChgLTVTrdAlarmRelDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kDChgHVTrdAlarmIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kDChgHVTrdAlarmDlyIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, DCHG_HV_TRD_ALARM_REL, byte_max);//config_get(kDChgHVTrdAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, DCHG_HV_TRD_ALARM_REL_DLY, byte_max);//config_get(kDChgHVTrdAlarmRelDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kDChgLVTrdAlarmIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kDChgLVTrdAlarmDlyIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, DCHG_LV_TRD_ALARM_REL, byte_max);//config_get(kDChgLVTrdAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, DCHG_LV_TRD_ALARM_REL_DLY, byte_max);//config_get(kDChgLVTrdAlarmRelDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kDChgDVTrdAlarmIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kDChgDVTrdAlarmDlyIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, DCHG_DV_TRD_ALARM_REL, byte_max);//config_get(kDChgDVTrdAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, DCHG_DV_TRD_ALARM_REL_DLY, byte_max);//config_get(kDChgDVTrdAlarmRelDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, VOLT_LINE_SND_ALARM_DLY_DEF, byte_max);//config_get(kVoltLineSndAlarmDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, VOLT_LINE_SND_ALARM_REL_DLY_DEF, byte_max);//config_get(kVoltLineSndAlarmRelDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, VOLT_LINE_TRD_ALARM_DLY_DEF, byte_max);//config_get(kVoltLineTrdAlarmDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, VOLT_LINE_TRD_ALARM_REL_DLY_DEF, byte_max);//config_get(kVoltLineTrdAlarmRelDlyIndex)
    
    return index;
}

INT16U ota_bcu_current_alarm_fill(unsigned char *__FAR buf)
{
    INT8U index = 0;
    INT16U macro_temp;
    INT16U byte_max = 0xffff;

    buf[index++] = 0x00;

    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kChgOCFstAlarmIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kChgOCFstAlarmDlyIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, CHG_OC_FST_ALARM_REL, byte_max);//config_get(kChgOCFstAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, CHG_OC_FST_ALARM_REL_DLY, byte_max);//config_get(kChgOCFstAlarmRelDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kChgOCSndAlarmIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kChgOCSndAlarmDlyIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, CHG_OC_SND_ALARM_REL, byte_max);//config_get(kChgOCSndAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, CHG_OC_SND_ALARM_REL_DLY, byte_max);//config_get(kChgOCSndAlarmRelDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kDChgOCFstAlarmIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kDChgOCFstAlarmDlyIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, DCHG_OC_FST_ALARM_REL, byte_max);//config_get(kDChgOCFstAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, DCHG_OC_FST_ALARM_REL_DLY, byte_max);//config_get(kDChgOCFstAlarmRelDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kDChgOCSndAlarmIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kDChgOCSndAlarmDlyIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, DCHG_OC_SND_ALARM_REL, byte_max);//config_get(kDChgOCSndAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, DCHG_OC_SND_ALARM_REL_DLY, byte_max);//config_get(kDChgOCSndAlarmRelDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kChgOCTrdAlarmIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kChgOCTrdAlarmDlyIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, CHG_OC_TRD_ALARM_REL, byte_max);//config_get(kChgOCTrdAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, CHG_OC_TRD_ALARM_REL_DLY, byte_max);//config_get(kChgOCTrdAlarmRelDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kDChgOCTrdAlarmIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kDChgOCTrdAlarmDlyIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, DCHG_OC_TRD_ALARM_REL, byte_max);//config_get(kDChgOCTrdAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, DCHG_OC_TRD_ALARM_REL_DLY, byte_max);//config_get(kDChgOCTrdAlarmRelDlyIndex)
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, config_get(kCurSensorReverseIndex), byte_max);

    return index;
}

INT16U ota_bcu_temperature_message_fill(unsigned char *__FAR buf)
{
    INT8U index = 0;
    INT16U macro_temp;
    INT16U byte_max = 0xffff;

    buf[index++] = 0x00;

    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, config_get(kDChgHTFstAlarmIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kDChgHTFstAlarmDlyIndex), byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, DCHG_HT_FST_ALARM_REL, byte_max);//config_get(kDChgHTFstAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, DCHG_HT_FST_ALARM_REL_DLY, byte_max);//config_get(kDChgHTFstAlarmRelDlyIndex)
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, config_get(kDChgHTSndAlarmIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kDChgHTSndAlarmDlyIndex), byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, DCHG_HT_SND_ALARM_REL, byte_max);//config_get(kDChgHTSndAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, DCHG_HT_SND_ALARM_REL_DLY, byte_max);//config_get(kDChgHTSndAlarmRelDlyIndex)
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, config_get(kDChgLTFstAlarmIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kDChgLTFstAlarmDlyIndex), byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, DCHG_LT_FST_ALARM_REL, byte_max);//config_get(kDChgLTFstAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, DCHG_LT_FST_ALARM_REL_DLY, byte_max);//config_get(kDChgLTFstAlarmRelDlyIndex)
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, config_get(kDChgLTSndAlarmIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kDChgLTSndAlarmDlyIndex), byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, DCHG_LT_SND_ALARM_REL, byte_max);//config_get(kDChgLTSndAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, DCHG_LT_SND_ALARM_REL_DLY, byte_max);//config_get(kDChgLTSndAlarmRelDlyIndex)
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, config_get(kDChgDTFstAlarmIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kDChgDTFstAlarmDlyIndex), byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, DCHG_DT_FST_ALARM_REL, byte_max);//config_get(kDChgDTFstAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index,DCHG_DT_FST_ALARM_REL_DLY, byte_max); //config_get(kDChgDTFstAlarmRelDlyIndex)
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, config_get(kDChgDTSndAlarmIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kDChgDTSndAlarmDlyIndex), byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, DCHG_DT_SND_ALARM_REL, byte_max);//config_get(kDChgDTSndAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, DCHG_DT_SND_ALARM_REL_DLY, byte_max);//config_get(kDChgDTSndAlarmRelDlyIndex)
#if !REMOVE_NO_USED_IDX_TO_SAVE_EEPROM
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, config_get(kRiseTempFstAlarmIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kRiseTempFstAlarmTimeIndex), byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, config_get(kRiseTempFstAlarmRelIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kRiseTempFstAlarmRelTimeIndex), byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, config_get(kRiseTempSndAlarmIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kRiseTempSndAlarmTimeIndex), byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, config_get(kRiseTempSndAlarmRelIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kRiseTempSndAlarmRelTimeIndex), byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, config_get(kFallTempFstAlarmIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kFallTempFstAlarmTimeIndex), byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, config_get(kFallTempFstAlarmRelIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kFallTempFstAlarmTimeRelIndex), byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, config_get(kFallTempSndAlarmIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kFallTempSndAlarmTimeIndex), byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, config_get(kFallTempSndAlarmRelIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kFallTempSndAlarmTimeRelIndex), byte_max);
#else
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, 0, byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, 0, byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, 0, byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, 0, byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, 0, byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, 0, byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, 0, byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, 0, byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, 0, byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, 0, byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, 0, byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, 0, byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, 0, byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, 0, byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, 0, byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, 0, byte_max);
#endif
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kTLineFstAlarmDlyIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, TLINE_FST_ALARM_REL_DLY, byte_max);//config_get(kTLineFstAlarmRelDlyIndex)
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, config_get(kChgHTFstAlarmIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kChgHTFstAlarmDlyIndex), byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, CHG_HT_FST_ALARM_REL, byte_max);//config_get(kChgHTFstAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, CHG_HT_FST_ALARM_REL_DLY, byte_max);//config_get(kChgHTFstAlarmRelDlyIndex)
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, config_get(kChgHTSndAlarmIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kChgHTSndAlarmDlyIndex), byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, CHG_HT_SND_ALARM_REL, byte_max);//config_get(kChgHTSndAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, CHG_HT_SND_ALARM_REL_DLY, byte_max);//config_get(kChgHTSndAlarmRelDlyIndex)
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, config_get(kDChgHTTrdAlarmIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kDChgHTTrdAlarmDlyIndex), byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, DCHG_HT_TRD_ALARM_REL, byte_max);//config_get(kDChgHTTrdAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, DCHG_HT_TRD_ALARM_REL_DLY, byte_max);//config_get(kDChgHTTrdAlarmRelDlyIndex)
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, config_get(kDChgLTTrdAlarmIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kDChgLTTrdAlarmDlyIndex), byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, DCHG_LT_TRD_ALARM_REL, byte_max);//config_get(kDChgLTTrdAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, DCHG_LT_TRD_ALARM_REL_DLY, byte_max);//config_get(kDChgLTTrdAlarmRelDlyIndex)
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, config_get(kDChgDTTrdAlarmIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kDChgDTTrdAlarmDlyIndex), byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, DCHG_DT_TRD_ALARM_REL, byte_max);//config_get(kDChgDTTrdAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, DCHG_DT_TRD_ALARM_REL_DLY, byte_max);//config_get(kDChgDTTrdAlarmRelDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, TEMP_LINE_SND_ALARM_DLY_DEF, byte_max);//config_get(kTempLineSndAlarmDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, TEMP_LINE_SND_ALARM_REL_DLY_DEF, byte_max);//config_get(kTempLineSndAlarmRelDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, TEMP_LINE_TRD_ALARM_DLY_DEF, byte_max);//config_get(kTempLineTrdAlarmDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, TEMP_LINE_TRD_ALARM_REL_DLY_DEF, byte_max);//config_get(kTempLineTrdAlarmRelDlyIndex)
    
    return index;
}

INT16U ota_bcu_soc_alarm_fill(unsigned char *__FAR buf)
{
    INT8U index = 0;
    INT16U macro_temp;
    INT16U byte_max = 0xffff;

    buf[index++] = 0x00;

    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kLSOCSndAlarmIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kLSOCSndAlarmDlyIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, LSOC_SND_ALARM_REL, byte_max);//config_get(kLSOCSndAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, LSOC_SND_ALARM_REL_DLY, byte_max);//config_get(kLSOCSndAlarmRelDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kLSOCFstAlarmIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kLSOCFstAlarmDlyIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, LSOC_FST_ALARM_REL, byte_max);//config_get(kLSOCFstAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, LSOC_FST_ALARM_REL_DLY, byte_max);//config_get(kLSOCFstAlarmRelDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, SOC_HIGH_SND_ALARM_DEF, byte_max);//config_get(kHSOCSndAlarmIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, SOC_HIGH_SND_ALARM_DLY_DEF, byte_max);//config_get(kHSOCSndAlarmDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, SOC_HIGH_SND_ALARM_REL_DEF, byte_max);//config_get(kHSOCSndAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, SOC_HIGH_SND_ALARM_REL_DLY_DEF, byte_max);//config_get(kHSOCSndAlarmRelDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kHSOCFstAlarmIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kHSOCFstAlarmDlyIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, HSOC_FST_ALARM_REL, byte_max);//config_get(kHSOCFstAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, HSOC_FST_ALARM_REL_DLY, byte_max);//config_get(kHSOCFstAlarmRelDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kSOCLowTrdAlarmIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kSOCLowTrdAlarmDlyIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, SOC_LOW_TRD_ALARM_REL, byte_max);//config_get(kSOCLowTrdAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, SOC_LOW_TRD_ALARM_REL_DLY, byte_max);//config_get(kSOCLowTrdAlarmRelDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, SOC_HIGH_TRD_ALARM_DEF, byte_max);//config_get(kSOCHighTrdAlarmIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, SOC_HIGH_TRD_ALARM_DLY_DEF, byte_max);//config_get(kSOCHighTrdAlarmDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, SOC_HIGH_TRD_ALARM_REL_DEF, byte_max);//config_get(kSOCHighTrdAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, SOC_HIGH_TRD_ALARM_REL_DLY_DEF, byte_max);//config_get(kSOCHighTrdAlarmRelDlyIndex)
    
    return index;
}

INT16U ota_bcu_insu_alarm_fill(unsigned char *__FAR buf)
{
    INT8U index = 0;
    INT16U macro_temp;
    INT16U byte_max = 0xffff;

    buf[index++] = 0x00;

    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kInsuFstAlarmIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kInsuFstAlarmDlyIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, INSU_FST_ALARM_REL, byte_max);//config_get(kInsuFstAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, INSU_FST_ALARM_REL_DLY, byte_max);//config_get(kInsuFstAlarmRelDlyIndex)

    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kInsuSndAlarmIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kInsuSndAlarmDlyIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, INSU_SND_ALARM_REL, byte_max);//config_get(kInsuSndAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, INSU_SND_ALARM_REL_DLY, byte_max);//config_get(kInsuSndAlarmRelDlyIndex)

    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kInsuTrdAlarmIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kInsuTrdAlarmDlyIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, INSU_TRD_ALARM_REL, byte_max);//config_get(kInsuTrdAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, INSU_TRD_ALARM_REL_DLY, byte_max);//config_get(kInsuTrdAlarmRelDlyIndex)

    return index;
}

INT16U ota_bcu_insu2_alarm_fill(unsigned char *__FAR buf)
{
    INT8U index = 0;
    INT16U macro_temp;
    INT16U byte_max = 0xffff;

    buf[index++] = 0;

    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, config_get(kEepromInsuResCalibFlagIndex), byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, config_get(kEepromTvRangeIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kEepromMosOnDelayIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kEepromNormalCheckCycleIndex), byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, config_get(kEepromInsuResSwitchInChgIndex), byte_max);

    return index;
}

INT16U ota_bcu_relay_config_fill(unsigned char *__FAR buf)
{
    INT8U index = 0;
    INT16U macro_temp;
    INT16U byte_max = 0xffff;

    buf[index++] = 0x00;

    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kChgRelayOnDlyIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kChgRelayOffDlyIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kDisChgRelayOnDlyIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kDisChgRelayOffDlyIndex), byte_max);

    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kPreChgRelayOnDlyIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kPreChgRelayOffDlyIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, RESERVE_RELAY_ON_DLY_DEF, byte_max);//config_get(kReserveRelayOnDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, RESERVE_RELAY_OFF_DLY_DEF, byte_max);//config_get(kReserveRelayOffDlyIndex)

    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, config_get(kRelayDischargeSamePort), byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, config_get(kRelayChargeMutex), byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, config_get(kRelayPreCharge), byte_max);

    return index;
}

INT16U ota_bcu_heat_cool_config_fill(unsigned char *__FAR buf)
{
    INT8U index = 0;
    INT16U macro_temp;
    INT16U byte_max = 0xffff;

    buf[index++] = 0x00;

    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, config_get(kHeatOnTemperature), byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, config_get(kHeatOffTemperature), byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, config_get(kCoolOnTemperature), byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, config_get(kCoolOffTemperature), byte_max);

    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, CFG_HEAT_ON_DLY_DEF, byte_max);//config_get(kHeatOnTempDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, CFG_HEAT_OFF_DLY_DEF, byte_max);//config_get(kHeatOffTempDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, CFG_COOL_ON_DLY_DEF, byte_max);//config_get(kCoolOnTempDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, CFG_COOL_OFF_DLY_DEF, byte_max);//config_get(kCoolOffTempDlyIndex)

    return index;
}


INT16U ota_bcu_charger_config_fill(unsigned char *__FAR buf)
{
    INT8U index = 0;
    INT16U macro_temp;
    INT16U byte_max = 0xffff;

    buf[index++] = 0x00;

    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, config_get(kChargerProtocolIndex), byte_max);

    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kChargerVoltIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kChargerCurIndex), byte_max);
    WRITE_LT_INT16U_WITH_BUFF_SIZE(buf, index, 0, byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, BMS_FULL_CHG_RELEASE_VOLT, byte_max);//config_get(kChgFullChgTVReleaseIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kChgCutOffCurIndex), byte_max);

    return index;
}

INT16U ota_bcu_chg_volt_alarm_config_fill(unsigned char *__FAR buf)
{
    INT8U index = 0;           
    INT16U macro_temp;
    INT16U byte_max = 0xffff;

    buf[index++] = 0x00;

    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kChgHTVFstAlarmIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kChgHTVFstAlarmDlyIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, CHG_HTV_FST_ALARM_REL, byte_max);//config_get(kChgHTVFstAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, CHG_HTV_FST_ALARM_REL_DLY, byte_max);//config_get(kChgHTVFstAlarmRelDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kChgHTVSndAlarmIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kChgHTVSndAlarmDlyIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, CHG_HTV_SND_ALARM_REL, byte_max);//config_get(kChgHTVSndAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, CHG_HTV_SND_ALARM_REL_DLY, byte_max);//config_get(kChgHTVSndAlarmRelDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kChgHTVTrdAlarmIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kChgHTVTrdAlarmDlyIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, CHG_HTV_TRD_ALARM_REL, byte_max);//config_get(kChgHTVTrdAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, CHG_HTV_TRD_ALARM_REL_DLY, byte_max);//config_get(kChgHTVTrdAlarmRelDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kChgLTVFstAlarmIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kChgLTVFstAlarmDlyIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, CHG_LTV_FST_ALARM_REL, byte_max);//config_get(kChgLTVFstAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, CHG_LTV_FST_ALARM_REL_DLY, byte_max);//config_get(kChgLTVFstAlarmRelDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kChgLTVSndAlarmIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kChgLTVSndAlarmDlyIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, CHG_LTV_SND_ALARM_REL, byte_max);//config_get(kChgLTVSndAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, CHG_LTV_SND_ALARM_REL_DLY, byte_max);//config_get(kChgLTVSndAlarmRelDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kChgLTVTrdAlarmIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kChgLTVTrdAlarmDlyIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, CHG_LTV_TRD_ALARM_REL, byte_max);//config_get(kChgLTVTrdAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, CHG_LTV_TRD_ALARM_REL_DLY, byte_max);//config_get(kChgLTVTrdAlarmRelDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kChgHVFstAlarmIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kChgHVFstAlarmDlyIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, CHG_HV_FST_ALARM_REL, byte_max);//config_get(kChgHVFstAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, CHG_HV_FST_ALARM_REL_DLY, byte_max);//config_get(kChgHVFstAlarmRelDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kChgHVSndAlarmIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kChgHVSndAlarmDlyIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, CHG_HV_SND_ALARM_REL, byte_max);//config_get(kChgHVSndAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, CHG_HV_SND_ALARM_REL_DLY, byte_max);//config_get(kChgHVSndAlarmRelDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kChgHVTrdAlarmIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kChgHVTrdAlarmDlyIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, CHG_HV_TRD_ALARM_REL, byte_max);//config_get(kChgHVTrdAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, CHG_HV_TRD_ALARM_REL_DLY, byte_max);//config_get(kChgHVTrdAlarmRelDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kChgLVFstAlarmIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kChgLVFstAlarmDlyIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, CHG_LV_FST_ALARM_REL, byte_max);//config_get(kChgLVFstAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, CHG_LV_FST_ALARM_REL_DLY, byte_max);//config_get(kChgLVFstAlarmRelDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kChgLVSndAlarmIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kChgLVSndAlarmDlyIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, CHG_LV_SND_ALARM_REL, byte_max);//config_get(kChgLVSndAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, CHG_LV_SND_ALARM_REL_DLY, byte_max);//config_get(kChgLVSndAlarmRelDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kChgLVTrdAlarmIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kChgLVTrdAlarmDlyIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, CHG_LV_TRD_ALARM_REL, byte_max);//config_get(kChgLVTrdAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, CHG_LV_TRD_ALARM_REL_DLY, byte_max);//config_get(kChgLVTrdAlarmRelDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kChgDVFstAlarmIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kChgDVFstAlarmDlyIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, CHG_DV_FST_ALARM_REL, byte_max);//config_get(kChgDVFstAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, CHG_DV_FST_ALARM_REL_DLY, byte_max);//config_get(kChgDVFstAlarmRelDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kChgDVSndAlarmIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kChgDVSndAlarmDlyIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, CHG_DV_SND_ALARM_REL, byte_max);//config_get(kChgDVSndAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, CHG_DV_SND_ALARM_REL_DLY, byte_max);//config_get(kChgDVSndAlarmRelDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kChgDVTrdAlarmIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kChgDVTrdAlarmDlyIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, CHG_DV_TRD_ALARM_REL, byte_max);//config_get(kChgDVTrdAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, CHG_DV_TRD_ALARM_REL_DLY, byte_max);//config_get(kChgDVTrdAlarmRelDlyIndex)
    
    return index;
}


INT16U ota_bcu_chg_temp_alarm_config_fill(unsigned char *__FAR buf)
{
    INT8U index = 0;
    INT16U macro_temp;
    INT16U byte_max = 0xffff;

    buf[index++] = 0x00;

    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, config_get(kChgHTFstAlarmIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kChgHTFstAlarmDlyIndex), byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, CHG_HT_FST_ALARM_REL, byte_max);//config_get(kChgHTFstAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, CHG_HT_FST_ALARM_REL_DLY, byte_max);//config_get(kChgHTFstAlarmRelDlyIndex)
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, config_get(kChgHTSndAlarmIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kChgHTSndAlarmDlyIndex), byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, CHG_HT_SND_ALARM_REL, byte_max);//config_get(kChgHTSndAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, CHG_HT_SND_ALARM_REL_DLY, byte_max);//config_get(kChgHTSndAlarmRelDlyIndex)
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, config_get(kChgHTTrdAlarmIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kChgHTTrdAlarmDlyIndex), byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, CHG_HT_TRD_ALARM_REL, byte_max);//config_get(kChgHTTrdAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, CHG_HT_TRD_ALARM_REL_DLY, byte_max);//config_get(kChgHTTrdAlarmRelDlyIndex)
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, config_get(kChgLTFstAlarmIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kChgLTFstAlarmDlyIndex), byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, CHG_LT_FST_ALARM_REL, byte_max);//config_get(kChgLTFstAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, CHG_LT_FST_ALARM_REL_DLY, byte_max);//config_get(kChgLTFstAlarmRelDlyIndex)
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, config_get(kChgLTSndAlarmIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kChgLTSndAlarmDlyIndex), byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, CHG_LT_SND_ALARM_REL, byte_max);//config_get(kChgLTSndAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, CHG_LT_SND_ALARM_REL_DLY, byte_max);//config_get(kChgLTSndAlarmRelDlyIndex)
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, config_get(kChgLTTrdAlarmIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kChgLTTrdAlarmDlyIndex), byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, CHG_LT_TRD_ALARM_REL, byte_max);//config_get(kChgLTTrdAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, CHG_LT_TRD_ALARM_REL_DLY, byte_max);//config_get(kChgLTTrdAlarmRelDlyIndex)
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, config_get(kChgDTFstAlarmIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kChgDTFstAlarmDlyIndex), byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, CHG_DT_FST_ALARM_REL, byte_max);//config_get(kChgDTFstAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, CHG_DT_FST_ALARM_REL_DLY, byte_max);//config_get(kChgDTFstAlarmRelDlyIndex)
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, config_get(kChgDTSndAlarmIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kChgDTSndAlarmDlyIndex), byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, CHG_DT_SND_ALARM_REL, byte_max);//config_get(kChgDTSndAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, CHG_DT_SND_ALARM_REL_DLY, byte_max);//config_get(kChgDTSndAlarmRelDlyIndex)
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, config_get(kChgDTTrdAlarmIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kChgDTTrdAlarmDlyIndex), byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, CHG_DT_TRD_ALARM_REL, byte_max);//config_get(kChgDTTrdAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, CHG_DT_TRD_ALARM_REL_DLY, byte_max);//config_get(kChgDTTrdAlarmRelDlyIndex)
    
    return index;
}


INT16U ota_bms_mlock_info_message_fill(unsigned char *__FAR buf)
{
    INT8U index = 0;
    INT8U mlock_type = 0;
    INT8U mlock_param_len = 0;
    INT16U byte_max = 0xffff;
    
    buf[index++] = 0x00;

    mlock_type      = (INT8U)config_get(kMlockType);
    if(mlock_type == 0)
    {
        WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, mlock_type,byte_max);
        return 2;
    }
    mlock_param_len = (INT8U)config_get(kMlockParamLength);
    

    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, mlock_type,     byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, chglock_get_logic_state(),   byte_max);//0 - 解锁, 1 - 锁止, FF-未知
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, 0x00,           byte_max); //00: normal, 01:force unlock, 02:force lock
    if((mlock_type == 0x02)||(mlock_type == 0x01)||(mlock_type == 0x03))//01: 巴斯巴 02:Amphenol
    {
        WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, 5,    byte_max);
        WRITE_LT_INT16U_WITH_BUFF_SIZE(buf, index, config_get(kMlockParam1), byte_max);
        WRITE_LT_INT16U_WITH_BUFF_SIZE(buf, index, config_get(kMlockParam2), byte_max);
        WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, config_get(kMlockParam3), byte_max);
    }
    else
    {
        WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, 0,    byte_max);
    }

    return index;

}

INT16U ota_bcu_chgr_outlet_config_fill(unsigned char *__FAR buf)
{
    INT8U index = 0;
    INT16U macro_temp;
    INT16U byte_max = 0xffff;

    buf[index++] = 0x00;

    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, config_get(kChgrAcTempNumIndex), byte_max);
    
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, config_get(kChgrAcOutletHTFstAlarmIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kChgrAcOutletHTFstAlarmDlyIndex), byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, CHGR_AC_OUTLET_HT_FST_ALARM_REL, byte_max);//config_get(kChgrAcOutletHTFstAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, CHGR_AC_OUTLET_HT_FST_ALARM_REL_DLY, byte_max);//config_get(kChgrAcOutletHTFstAlarmRelDlyIndex)
    
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, config_get(kChgrAcOutletHTSndAlarmIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kChgrAcOutletHTSndAlarmDlyIndex), byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, CHGR_AC_OUTLET_HT_SND_ALARM_REL, byte_max);//config_get(kChgrAcOutletHTSndAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, CHGR_AC_OUTLET_HT_SND_ALARM_REL_DLY, byte_max);//config_get(kChgrAcOutletHTSndAlarmRelDlyIndex)
    
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, CHG_OUTLET_HT_TRD_ALARM_DEF, byte_max);//config_get(kChgrAcOutletHTTrdAlarmIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, CHG_OUTLET_HT_TRD_ALARM_DLY_DEF, byte_max);//config_get(kChgrAcOutletHTTrdAlarmDlyIndex)
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, CHG_OUTLET_HT_TRD_ALARM_REL_DEF, byte_max);//config_get(kChgrAcOutletHTTrdAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, CHG_OUTLET_HT_TRD_ALARM_REL_DLY_DEF, byte_max);//config_get(kChgrAcOutletHTTrdAlarmRelDlyIndex)
    
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, config_get(kChgrDcTempNumIndex), byte_max);
    
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, config_get(kChgrDcOutletHTFstAlarmIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kChgrDcOutletHTFstAlarmDlyIndex), byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, CHGR_DC_OUTLET_HT_FST_ALARM_REL, byte_max);//config_get(kChgrDcOutletHTFstAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, CHGR_DC_OUTLET_HT_FST_ALARM_REL_DLY, byte_max);//config_get(kChgrDcOutletHTFstAlarmRelDlyIndex)
    
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, config_get(kChgrDcOutletHTSndAlarmIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kChgrDcOutletHTSndAlarmDlyIndex), byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, CHGR_DC_OUTLET_HT_SND_ALARM_REL, byte_max);//config_get(kChgrDcOutletHTSndAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, CHGR_DC_OUTLET_HT_SND_ALARM_REL_DLY, byte_max);//config_get(kChgrDcOutletHTSndAlarmRelDlyIndex)
    
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, CHG_OUTLET_HT_TRD_ALARM_DEF, byte_max);//config_get(kChgrDcOutletHTTrdAlarmIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, CHG_OUTLET_HT_TRD_ALARM_DLY_DEF, byte_max);//config_get(kChgrDcOutletHTTrdAlarmDlyIndex)
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, CHG_OUTLET_HT_TRD_ALARM_REL_DEF, byte_max);//config_get(kChgrDcOutletHTTrdAlarmRelIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, CHG_OUTLET_HT_TRD_ALARM_REL_DLY_DEF, byte_max);//config_get(kChgrDcOutletHTTrdAlarmRelDlyIndex)
    
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kChgrOutletTempLineFstAlarmDlyIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, CHGR_OUTLET_TEMP_LINE_FST_ALARM_REL_DLY, byte_max);//config_get(kChgrOutletTempLineFstAlarmRelDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kChgrOutletTempLineSndAlarmDlyIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, CHGR_OUTLET_TEMP_LINE_SND_ALARM_REL_DLY, byte_max);//config_get(kChgrOutletTempLineSndAlarmRelDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, TEMP_LINE_TRD_ALARM_DLY_DEF, byte_max);//config_get(kChgrOutletTempLineTrdAlarmDlyIndex)
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, TEMP_LINE_TRD_ALARM_REL_DLY_DEF, byte_max);//config_get(kChgrOutletTempLineTrdAlarmRelDlyIndex)
    
    return index;
}

INT16U ota_bmu_base_config_fill(unsigned char *__FAR buf)
{
    INT8U index = 0;
    INT16U temp = 0;
    INT32U macro_temp;
    INT16U byte_max = 0xffff;

  #if  BMS_SUPPORT_HARDWARE_LTC6803 == 1   
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, BYU_USE_SLAVE_INDEX + 1, byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, BYU_USE_SLAVE_INDEX + 1, byte_max);
  #endif
  #if  BMS_SUPPORT_HARDWARE_LTC6804 == 1   
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, 1, byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, 1, byte_max);
  #endif
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, bmu_get_voltage_num(0), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kBYUVoltCorrectIndex), byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, 0, byte_max);
    WRITE_LT_INT16U_WITH_BUFF_SIZE(buf, index, 0, byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, bmu_get_temperature_num(0), byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, BCU_CAN_ADDR_DEF, byte_max);//config_get(kBcuCanAddrIndex)
    
    return index;
}

INT16U ota_bmu_relay_config_fill(unsigned char *__FAR buf)
{
    INT8U index = 0;
    INT16U temp = 0;
    INT16U byte_max = 0xffff;

   #if  BMS_SUPPORT_HARDWARE_LTC6803 == 1   
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, BYU_USE_SLAVE_INDEX + 1, byte_max);
   #endif
  #if  BMS_SUPPORT_HARDWARE_LTC6804 == 1   
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index,  1, byte_max);
  #endif 
    
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, 0, byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, 0, byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, 0, byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, 0, byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, 2, byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, 0, byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, 0, byte_max);
    
    return index;
}


INT16U ota_bmu_alarm_config_fill(unsigned char *__FAR buf)
{
    INT8U index = 0;
    INT16U temp = 0;
    INT16U byte_max = 0xffff;

   #if  BMS_SUPPORT_HARDWARE_LTC6803 == 1  
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, BYU_USE_SLAVE_INDEX + 1, byte_max);
   #endif
  #if  BMS_SUPPORT_HARDWARE_LTC6804 == 1   
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index,  1, byte_max);
  #endif 
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, 0, byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, 0, byte_max);
    WRITE_LT_INT16U_WITH_BUFF_SIZE(buf, index, 0, byte_max);
    WRITE_LT_INT16U_WITH_BUFF_SIZE(buf, index, 0, byte_max);
    WRITE_LT_INT16U_WITH_BUFF_SIZE(buf, index, 0, byte_max);
    WRITE_LT_INT16U_WITH_BUFF_SIZE(buf, index, 0, byte_max);
    WRITE_LT_INT16U_WITH_BUFF_SIZE(buf, index, 0, byte_max);
    WRITE_LT_INT16U_WITH_BUFF_SIZE(buf, index, 0, byte_max);
    WRITE_LT_INT16U_WITH_BUFF_SIZE(buf, index, 0, byte_max);
    WRITE_LT_INT16U_WITH_BUFF_SIZE(buf, index, 0, byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, 0, byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, 0, byte_max);
    
    return index;
}


INT16U ota_bmu_balance_config_fill(unsigned char *__FAR buf)
{
    INT8U index = 0;
    INT16U temp = 0;
    INT32U macro_temp;
    INT16U byte_max = 0xffff;

  #if  BMS_SUPPORT_HARDWARE_LTC6803 == 1   
    buf[index++] = BYU_USE_SLAVE_INDEX + 1;
    #endif
  #if  BMS_SUPPORT_HARDWARE_LTC6804 == 1   
    buf[index++] =  1;
  #endif  
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kBYUBalDiffVoltMaxIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kBYUBalStartVoltIndex), byte_max);
    WRITE_LT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf, index, config_get(kBYUBalDiffVoltMinIndex), byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, config_get(kBYUBalNumIndex), byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, config_get(kChgHTFstAlarmIndex), byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, config_get(kBYUTotalBalEnIndex), byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, CHG_HT_FST_ALARM_REL, byte_max);//config_get(kChgHTFstAlarmRelIndex)
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, config_get(kBYUBalTypeIndex), byte_max);
    
    return index;
}


INT16U ota_dtu_config_message_fill(unsigned char *__FAR buf) {
    INT8U index = 0;
    unsigned char addr[40];
    unsigned int port = 0;
    unsigned char type;
    INT16U byte_max = 0xffff;

    memset(addr, 0, sizeof(addr));

    if(input_signal_is_high_by_name("GSM_ID"))
    {
        type = 0; // 无DTU
    } 
    else 
    {
        type = 1; // 板载DTU   
    }
    config_get_dtu_type(&type);
 //   config_get_dtu_server((unsigned char *_PAGED)addr, sizeof(addr), &port);

    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, 0x00, byte_max);
    WRITE_BUFFER(buf, index, addr, strlen((char *)addr)+ 1);
    WRITE_BT_INT16U_WITH_BUFF_SIZE(buf, index, port, byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, type, byte_max);

    return index;
}


INT16U ota_bmu_5160_config_fill(unsigned char *__FAR buf)
{
    INT8U index = 0;
    INT8U i = 0;
    INT16U tmp_16u = 0;
    INT16U byte_max = 0xffff;

    #if  BMS_SUPPORT_HARDWARE_LTC6803 == 1   
    buf[index++] = BYU_USE_SLAVE_INDEX + 1;
    #endif
  #if  BMS_SUPPORT_HARDWARE_LTC6804 == 1   
    buf[index++] =  1;
  #endif  
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, PARAM_BSU_NUM_MAX, byte_max);
    for(i=0; i<PARAM_BSU_NUM_MAX; i++)
    {
        tmp_16u = config_get(kBYUVoltTempCnt1Index+i);
        tmp_16u = (tmp_16u>>8);
        WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, (INT8U)tmp_16u, byte_max);
    }
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, PARAM_BSU_NUM_MAX, byte_max);
    for(i=0; i<PARAM_BSU_NUM_MAX; i++)
    {
        tmp_16u = config_get(kBYUVoltTempCnt1Index+i);
        tmp_16u = tmp_16u&0x00FF;
        WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, (INT8U)tmp_16u, byte_max);
    }
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, config_get(kBYUBalTypeIndex), byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, PARAM_BSU_NUM_MAX, byte_max);
    for(i=0; i<PARAM_BSU_NUM_MAX; i++)
    {
        tmp_16u = config_get(kBSU1VoltSampleNoUseBitIndex + i);
        tmp_16u = tmp_16u&0x0FFF;
        WRITE_LT_INT16U_WITH_BUFF_SIZE(buf, index, tmp_16u, byte_max);
    }
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, config_get(kBsuRelay1CfgType), byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, config_get(kBsuRelay2CfgType), byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, get_main_bsu_relay_force_cmd(1), byte_max);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf, index, get_main_bsu_relay_force_cmd(2), byte_max);
    
    return index;
}



static INT16U Ota_Config_Pgn_Fill(INT16U pgn, unsigned char *__FAR buf)
{
    switch(pgn)
    {
    case BMS_DEVICE_RELAY_PGN:
        return ota_bms_query_relay_info_fill(buf);
        break;
    case BCU_BASE_CONFIG_PGN:
        return ota_bcu_base_config_fill(buf);
        break;
    case BCU_COMMON_ALARM_CONFIG_PGN:
        return ota_bcu_common_alarm_fill(buf);
        break;
    case BCU_VOLT_ALARM_CONFIG_PGN:
        return ota_bcu_volt_alarm_fill(buf);
        break;
    case BCU_CURRENT_ALARM_CONFIG_PGN:
        return ota_bcu_current_alarm_fill(buf);
        break;
    case BCU_TEMPERATURE_ALARM_CONFIG_PGN:
        return ota_bcu_temperature_message_fill(buf);
        break;
    case BCU_SOC_ALARM_CONFIG_PGN:
        return ota_bcu_soc_alarm_fill(buf);
        break;
    case BCU_INSU_ALARM_CONFIG_PGN:
        return ota_bcu_insu_alarm_fill(buf);
        break;
    case BCU_INSU2_ALARM_CONFIG_PGN:
        return ota_bcu_insu2_alarm_fill(buf);
        break;    
    case BCU_RELAY_CONFIG_PGN:
        return ota_bcu_relay_config_fill(buf);
        break;
    case BCU_HEAT_COOL_CONFIG_PGN:
        return ota_bcu_heat_cool_config_fill(buf);
        break;
    case BCU_CHARGER_CONFIG_PGN:
        return ota_bcu_charger_config_fill(buf);
        break;
    case BCU_CHG_VOLT_ALARM_CONFIG_PGN:
        return ota_bcu_chg_volt_alarm_config_fill(buf);
        break;
    case BCU_CHG_TEMP_ALARM_CONFIG_PGN:
        return ota_bcu_chg_temp_alarm_config_fill(buf);
        break;
    case BCU_MLOCK_CONTROL_PGN:
        return ota_bms_mlock_info_message_fill(buf);
        break;
    case BCU_CHGR_OUTLET_TEMP_CONFIG_PGN:
        return ota_bcu_chgr_outlet_config_fill(buf);
        break;
    case BMU_BASE_CONFIG_PGN:
        return ota_bmu_base_config_fill(buf);
        break;
    case BMU_RELAY_CONFIG_PGN:
        return ota_bmu_relay_config_fill(buf);
        break;
    case BMU_ALARM_CONFIG_PGN:
        return ota_bmu_alarm_config_fill(buf);
        break;
    case BMU_BALANCE_CONFIG_PGN:
        return ota_bmu_balance_config_fill(buf);
        break;
    case DTU_CONFIG_MESSAGE_PGN:
        return ota_dtu_config_message_fill(buf);
        break;
    case BMU_5160_CONFIG_PGN:
        return ota_bmu_5160_config_fill(buf);
        break;
    default:
        return 0;
        break;
    }

}
     /*
static INT16U ota_bms_query_relay_info_len_get(void)
{
    INT16U len = 0;
    INT8U relay_num = 0;
    
    relay_num = relay_count();
    len = 3+ relay_num*3;
    
    relay_num = input_signal_get_count();
    len = len + relay_num*2;
    
    return len;
}
 
void INT16U ota_bms_mlock_info_len_get(void)
{
    INT8U mlock_type = 0;

    mlock_type      = (INT8U)config_get(kMlockType);
    if(mlock_type == 0)
    {
        return 2;
    }

    if(mlock_type == 0x02)//02:Amphenol
    {
        return 8;
    }
    else if(mlock_type == 0x01)//01: 巴斯巴
    {
        return 8;
    }
    else
    {
        return 3;
    }

}
     
static void Ota_Config_Pgn_Len_Get(INT16U pgn)
{
    switch(pgn)
    {
    case BMS_DEVICE_RELAY_PGN:
         return ota_bms_query_relay_info_len_get();
        break;
    case BCU_BASE_CONFIG_PGN:
        return 93;
        break;
    case BCU_COMMON_ALARM_CONFIG_PGN:
        return 29;
        break;
    case BCU_VOLT_ALARM_CONFIG_PGN:
        return 133;
        break;
    case BCU_CURRENT_ALARM_CONFIG_PGN:
        return 50;
        break;
    case BCU_TEMPERATURE_ALARM_CONFIG_PGN:
        return 103;
        break;
    case BCU_SOC_ALARM_CONFIG_PGN:
        return 49;
        break;
    case BCU_INSU_ALARM_CONFIG_PGN:
        return 17;
        break;
    case BCU_INSU2_ALARM_CONFIG_PGN:
        return 8;
        break;    
    case BCU_RELAY_CONFIG_PGN:
        return 20;
        break;
    case BCU_HEAT_COOL_CONFIG_PGN:
        return 13;
        break;
    case BCU_CHARGER_CONFIG_PGN:
        return 12;
        break;
    case BCU_CHG_VOLT_ALARM_CONFIG_PGN:
        return 121;
        break;
    case BCU_CHG_TEMP_ALARM_CONFIG_PGN:
        return 55;
        break;
    case BCU_MLOCK_CONTROL_PGN:
        return ota_bms_mlock_info_len_get();
        break;
    case BCU_CHGR_OUTLET_TEMP_CONFIG_PGN:
        return 51;
        break;
    case BMU_BASE_CONFIG_PGN:
        return 10;
        break;
    case BMU_RELAY_CONFIG_PGN:
        return 8;
        break;
    case BMU_ALARM_CONFIG_PGN:
        return 21;
        break;
    case BMU_BALANCE_CONFIG_PGN:
        return 12;
        break;
    case DTU_CONFIG_MESSAGE_PGN:
        return 45;
        break;
    case BMU_5160_CONFIG_PGN:
        return 4+MAX_LTC6803_NUM+MAX_LTC6803_NUM;
        break;
    default:
        bms_confirm_message_fill();
        break;
    }

    return;
}      */

static void Ota_Config_Data_Fill(INT16U pgn_num, INT8U upgrade_pgn_fill, INT8U err_flag) 
{   
    INT16U cmpVal = 0; 
    //pgn_num = Ota_Config_Pgn_Num_Get(pgn_index);
   // pgn_len = Ota_Config_Pgn_Len_Get(pgn_index);
    mUpgradeData[0] = 0x30;
    mUpgradeData[1] = 0x1e;
    mUpgradeData[2] = 0x0;
    mUpgradeData[3] = 0x0;
    mUpgradeData[4] = 0x1c;
    safe_memcpy((PINT8U)(mUpgradeData+5), (PINT8U)"ota/cfgack/", 11);
    safe_memcpy((PINT8U)(mUpgradeData+16), (PINT8U)ota_client_id, 17);
    
    if((1 == upgrade_pgn_fill) && (err_flag == 0)) 
    {
        mUpgradeData[33] = (INT8U)(pgn_num >> 8);
        mUpgradeData[34] = (INT8U)(pgn_num & 0xf); 
        mUpgradeDataIndex = 35 + Ota_Config_Pgn_Fill(pgn_num, mUpgradeData+35);
    } 
    else
    {
        mUpgradeData[33] = 0x0;
        mUpgradeData[34] = err_flag; 
        mUpgradeDataIndex = 35;
    }
    
    cmpVal = calcCrc16(mUpgradeData, 33, mUpgradeDataIndex-33, 0xffff);
    
    mUpgradeData[mUpgradeDataIndex] = (INT8U)(cmpVal >> 8);
    mUpgradeData[mUpgradeDataIndex+1] = (INT8U)(cmpVal & 0xff);
    mUpgradeDataIndex = mUpgradeDataIndex + 2; 
    
    if(mUpgradeDataIndex >= 131) 
    {
        mUpgradeData[1] = (((mUpgradeDataIndex-3)%128) | 0x80);
        mUpgradeData[2] = (INT8U)(mUpgradeDataIndex-3)/128;
    } 
    else
    {
        mUpgradeData[1] = ((mUpgradeDataIndex-3) | 0x80);
        mUpgradeData[2] = 0x0;
    }
    
    return;               

}

/********************************************************************************
* Function Name:taskUpdata
*               
* Description: 对接收到的数据进行传输解析。在对数据进行操作的时候，需要以topic 
               mUpgradeData[4]长度作为基准进行偏移操作  
*               
* Inputs:       
                
*               
* Outputs:      
* 
* Limitations:  
********************************************************************************/  
  
static UINT8 taskUpdata(void) 
{
    UINT8 topic_index = 0;
    UINT16 upgrade_pgn_id = 0;
    UINT8 upgrade_pgn_fill = 0;
 
    /*判断接收的字节长度，如果字节数最高位置位1，则表示长度大于256，否则小于256*/
    if(0x80 == (mUpgradeData[1]&0x80)) 
    {
        topic_index = 5; 
    }
    else
    {
        topic_index = 4; 
    } 
    
     
    if(safe_memcmp((PINT8U)(mUpgradeData+topic_index), (PINT8U)"ota/s19", 7) == 0)
    {
        if(taskOtaProgramCmd()) //升级数据解析     
        {
            Ota_Program_Data_Fill(1); //升级反馈数据填充  
        } 
        else
        {
            Ota_Program_Data_Fill(0); //升级反馈数据填充  
        }
    }
    else if(safe_memcmp((PINT8U)(mUpgradeData+topic_index), (PINT8U)"ota/cfg", 7) == 0)
    { 
        if(taskOtaConfigCmd(&upgrade_pgn_id, &upgrade_pgn_fill)) 
        {
            Ota_Config_Data_Fill(upgrade_pgn_id, upgrade_pgn_fill, 1);
        } 
        else
        {   
            Ota_Config_Data_Fill(upgrade_pgn_id, upgrade_pgn_fill, 0);    
        }
    }
    else
    {
        return RES_ERR;
    }
     
    return RES_OK;

} 
  
      
      
/// return ==0 need_restart
/// return !=0 run is ok
static char taskWaitEventToHandle(INT16U t) {
    INT8U err;
    INT8U type;
    INT16U len;
    OS_CPU_SR cpu_sr = 0;

    // 等待数据发送的信号量.
   // runtimeStatusSetCurrent(DTUM35_RUNTIME_STATUS_WAIT_EVENT);
    OSSemPend(mEvent, t, &err);
    if (err != OS_ERR_NONE) {
        OS_ENTER_CRITICAL();
        RingBuffer_Clear(&mReceivedMessage);
        OS_EXIT_CRITICAL();
        return 1;
    }


    OS_ENTER_CRITICAL();
    if (1 != RingBuffer_Read(&mReceivedMessage, &type, 1)) {
        RingBuffer_Clear(&mReceivedMessage);
        OS_EXIT_CRITICAL();
        return 1;
    }

    if (2 != RingBuffer_Read(&mReceivedMessage, (unsigned char *)&len, 2)) {
        RingBuffer_Clear(&mReceivedMessage);
        OS_EXIT_CRITICAL();
        return 1;
    }
    OS_EXIT_CRITICAL();

    if (type == DATA_FOR_SEND_WITH_CALLBACK) {
        Bool ret;
        struct DTUM35_SendDataInfo info;

        OS_ENTER_CRITICAL();
        if (sizeof(info) != RingBuffer_Read(&mReceivedMessage, (unsigned char *)&info, len)) {
            RingBuffer_Clear(&mReceivedMessage);
            OS_EXIT_CRITICAL();
            return 1;
        }
        OS_EXIT_CRITICAL();

        ret = ATCMD_SendDataViaGPRSCallback(info.private_data, info.get_data, info.len);
        if (info.finished) {
            info.finished((char)ret);
        }

        if (ret) { // 连续多次发送失败返回错误.
            return (++mTimesSendFail < MAX_TIMES_DATA_SEND_FAIL);
        }

        OS_ENTER_CRITICAL();
        ++mPacketCounter;
        OS_EXIT_CRITICAL();
        mTimesSendFail = 0;
        return 1;
    }

    if (type == RECVED_CMT) {
        /*char oa[15];
        char content[50];
        runtimeStatusSetCurrent(DTUM35_RUNTIME_STATUS_RECV_SMS);
        len -= RingBuffer_PopString(&mSerialRecvBuffer, oa, sizeof(oa));
        len -= RingBuffer_PopString(&mSerialRecvBuffer, content, sizeof(content));
        (void)RingBuffer_DropDataFromTail(&mSerialRecvBuffer, len);
        if (strncmp(content, "YOUARE:", 7) == 0) {
            dtum35_SetSIMInfo(oa, &content[7]);
        }*/
        return 1;
    }
    return 1;
}

/*M35有使用，800没有使用，防止编译报警先注释掉*/
//#if 0
static char taskUpdateLACCI(void) {
    if (OSTimeGet() > mTickToUpdateLacCi) {
        unsigned short lacci[2];
        if (!ATCMD_GetLacCi(lacci)) {
            return 1;
        }
        mLacCi[0] = lacci[0];
        mLacCi[1] = lacci[1];
        frameHeaderUpdateLACCI();
        mTickToUpdateLacCi = OSTimeGet() + UPDATE_LAC_CI_INTERVAL;
    }

    return 1;

}
//#endif

static char taskIntToStr(INT8U num) {
    if(num <= 0x9) 
    {
        return num+0x30;
    }
    else if((0xa <= num) && (0xf >= num)) 
    {
        return num+0x57;
    }
    
    return num;

}


Bool taskSendVersionData(void) {
    INT8U index = 0;             
    char buf[19] = {0x30,0x27,0x0,0x0f,'c','u','r','r','e','n','t','/','v','e', 'r','s','i','o','n'};
    char buf2[22] = {0};
    
    INIT_WRITE_BUFFFER();

    (void)ATCMD_SendDataViaGPRS((unsigned char *__FAR)buf, 19);
    
    WRITE_BT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf2, index, config_get(kUUIDPart1), 22);//UUID 1
    WRITE_BT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf2, index, config_get(kUUIDPart2), 22);//UUID 2
    WRITE_BT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf2, index, config_get(kUUIDPart3), 22);//UUID 3
    WRITE_BT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf2, index, config_get(kUUIDPart4), 22);//UUID 4
    WRITE_BT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf2, index, config_get(kUUIDPart5), 22);//UUID 5
    WRITE_BT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf2, index, config_get(kUUIDPart6), 22);//UUID 6
    WRITE_BT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf2, index, config_get(kUUIDPart7), 22);//UUID 7
    WRITE_BT_INT16U_WITH_FUNC_DATA_AND_BUFF_SIZE(buf2, index, config_get(kUUIDPart8), 22);//UUID 8
    
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf2, index, ORDER_SW_MAJOR_VER, 22);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf2, index, ORDER_SW_MINOR_VER, 22);
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf2, index, ORDER_SW_REV_VER, 22);
    WRITE_LT_INT16U_WITH_BUFF_SIZE(buf2, index, ORDER_SW_BUILD_VER, 22);
    
    WRITE_LT_INT8U_WITH_BUFF_SIZE(buf2, index, OTA_S19_PART-0xa, 22);    
       
    (void)ATCMD_SendDataViaGPRS((unsigned char *__FAR)buf2, 22);
        
    
    return RES_OK;
}



static void taskUpdateGetUUID(void) {
    INT8U uuid_temp[17] = {0};
    
    uuid_temp[0]  = taskIntToStr((INT8U)((config_get(kUUIDPart4) >> 0) & 0xf));
    uuid_temp[1]  = taskIntToStr((INT8U)((config_get(kUUIDPart5) >> 12) & 0xf));
    uuid_temp[2]  = taskIntToStr((INT8U)((config_get(kUUIDPart5) >> 8) & 0xf));
    uuid_temp[3]  = taskIntToStr((INT8U)((config_get(kUUIDPart5) >> 4) & 0xf));
    uuid_temp[4]  = taskIntToStr((INT8U)((config_get(kUUIDPart5) >> 0) & 0xf));
    uuid_temp[5]  = taskIntToStr((INT8U)((config_get(kUUIDPart6) >> 12) & 0xf));
    uuid_temp[6]  = taskIntToStr((INT8U)((config_get(kUUIDPart6) >> 8) & 0xf)); 
    uuid_temp[7]  = taskIntToStr((INT8U)((config_get(kUUIDPart6) >> 4) & 0xf));
    uuid_temp[8]  = taskIntToStr((INT8U)((config_get(kUUIDPart6) >> 0) & 0xf));
    uuid_temp[9] = taskIntToStr((INT8U)((config_get(kUUIDPart7) >> 12) & 0xf));
    uuid_temp[10] = taskIntToStr((INT8U)((config_get(kUUIDPart7) >> 8) & 0xf));
    uuid_temp[11] = taskIntToStr((INT8U)((config_get(kUUIDPart7) >> 4) & 0xf));
    uuid_temp[12] = taskIntToStr((INT8U)((config_get(kUUIDPart7) >> 0) & 0xf));
    uuid_temp[13] = taskIntToStr((INT8U)((config_get(kUUIDPart8) >> 12) & 0xf));
    uuid_temp[14] = taskIntToStr((INT8U)((config_get(kUUIDPart8) >> 8) & 0xf)); 
    uuid_temp[15] = taskIntToStr((INT8U)((config_get(kUUIDPart8) >> 4) & 0xf));
    uuid_temp[16] = taskIntToStr((INT8U)((config_get(kUUIDPart8) >> 0) & 0xf));
    
    (void)safe_memcpy((PINT8U)(ota_connect_buf+10+ota_connect_buf[3]),(PINT8U)uuid_temp,17);
    (void)safe_memcpy((PINT8U)(program_subscribe_buf+program_subscribe_buf[5]-11),(PINT8U)uuid_temp,17);
    (void)safe_memcpy((PINT8U)(config_subscribe_buf+config_subscribe_buf[5]-11),(PINT8U)uuid_temp,17);
    (void)safe_memcpy((PINT8U)(csf_subscribe_buf+csf_subscribe_buf[5]-11),(PINT8U)uuid_temp,17);
    (void)safe_memcpy((PINT8U)(rule_subscribe_buf+rule_subscribe_buf[5]-11),(PINT8U)uuid_temp,17);
    (void)safe_memcpy((PINT8U)ota_client_id,(PINT8U)uuid_temp,17);
    //(void)safe_memcpy((PINT8U)(publish_buf+publish_buf[3]-13),(PINT8U)uuid_temp,17);
     
    return;

}
/// \brief task M35模块任务函数.
///

static char taskSendMqttData(void) {
    OS_CPU_SR cpu_sr = 0;
    
    
    //发送连接    
    if ((mqtt_data_type == 0) && (1 == mqtt_data_ready) && (0 == upgrade_data_ready))   
    {
        (void)ATCMD_SendDataViaGPRS(ota_connect_buf, 50);
        mqtt_data_ready = 0;
        return 0;
    }
    
    //发送s19升级订阅
    if ((mqtt_data_type == 1) && (1 == mqtt_data_ready) && (0 == upgrade_data_ready)) 
    {
        (void)ATCMD_SendDataViaGPRS(program_subscribe_buf, 32);
        mqtt_data_ready = 0;
        return 0;
    }
    
    //发送cfg升级订阅
    if ((mqtt_data_type == 2) && (1 == mqtt_data_ready) && (0 == upgrade_data_ready)) 
    {
        (void)ATCMD_SendDataViaGPRS(config_subscribe_buf, 32);
        mqtt_data_ready = 0;
        return 0;
    }
    
    //发送csf升级订阅
    if ((mqtt_data_type == 3) && (1 == mqtt_data_ready) && (0 == upgrade_data_ready)) 
    {
        (void)ATCMD_SendDataViaGPRS(csf_subscribe_buf, 32);
        mqtt_data_ready = 0;
        return 0;
    }
    
    //发送ping
    if ((mqtt_data_type == 4) && (1 == mqtt_data_ready) && (0 == upgrade_data_ready)) 
    {
        (void)ATCMD_SendDataViaGPRS(ping_buf, 2);
        mqtt_data_ready = 0;
        OSTimeDly(OS_TICKS_PER_SEC * 5);
        return 0;
    }
    
    //发送升级数据处理后的响应
    if ((mqtt_data_type == 5) && (1 == mqtt_data_ready) && (0 == upgrade_data_ready)) 
    {
        (void)ATCMD_SendDataViaGPRS(mUpgradeData, mUpgradeDataIndex);
        mqtt_data_ready = 0;
        if(1 == upgrade_data_over) 
        { 
            mqtt_data_type = 1;
            mqtt_data_ready = 1;
            upgrade_data_over = 0;    
        }
        return 0;
    }
    
    if(0 == mqtt_data_ready) 
    {
        return 1;
    }
    
    if(1 == upgrade_data_ready) 
    {
        return 0;
    }

    return 1;
}


static void task(void *nouse) {

    INT8U err = 0;
    INT8U dtu_data_wait = 0;
    INT8U version_data_wait = 6; 
    (void)nouse;

    bspInterface->m35_control_init(9600);
    ATCMD_RuntimeInit(bspInterface->m35_serial_send);
    runtimeStatusInit();
    //DTUGPS_Init(&g_DTUGPS_BspInterface);

    for (;;) {
        dprintf("Hello M35 task\n");
        reinitDtuInfo();

        // 开始模块失败, 延时重启模块.
        if (!startAndConfigModem()) goto __need_restart;
        
        (void)getOpsData();
        (void)getIMEI();
        // 更新SIM卡信息.
        // 设置协议头.
        runtimeStatusSetCurrent(DTUM35_RUNTIME_STATUS_CONNECT_SERVER);

        // 连接服务器失败, 延时重启模块.
        if (!mIsForTest) {            
            if (!ATCMD_ConnectTcpServer("220.180.238.114", 5522)) {
                goto __need_restart;
            }
        }
        
        //设置连接为非透传模式
        if (!ATCMD_SetCPIMode(2)) {
            break;
        }	
        OSTimeDly(OS_TICKS_PER_SEC);
        // 获取sim卡号
        (void)getSimCardData();
        frameHeaderSetPhoneNum(mSimCardData.phone); // set frame header
        
        // 更新信号强度.
        if (!taskUpdateLACCI()) goto __need_restart;
        if (!taskUpdateSignalStrength()) goto __need_restart;
        	//设置连接为透传模式
        if (!ATCMD_SetCPIMode(3)) {
            goto __need_restart;
        }
        OSTimeDly(OS_TICKS_PER_SEC);
        cipsend_mode = 1;
        // 复位一些计数器.
        resetCounters();
        taskUpdateGetUUID();
        // 等待数据发送的信号量.

        runtimeStatusSetCurrent(DTUM35_RUNTIME_STATUS_WAIT_EVENT);
        
        
        for (;;) {
            OSTimeDly(500);
            // 发送远程升级数据到mqtt服务器 ，连续1分钟服务器无响应则重新连接
            if (!mIsForTest) {                
                if (taskSendMqttData()) 
                {
                    err++;
                    if(err > 120) 
                    {
                        break;
                    }
                } 
                else 
                {
                    err = 0; 
                }
                
                // 处理mqtt发送过来的远程升级数据 
                if(1 == upgrade_data_ready) 
                {
                    if (taskUpdata()) 
                    {
                        break;
                    } 
                    else
                    {
                        mqtt_data_ready = 1;
                        upgrade_data_ready = 0;
                    }
                }
                           
                // 发送DTU数据，在远程升级时保证升级速度，每20秒检测一次发送信号量
                if(4 == mqtt_data_type) 
                {   
                    OSTimeDly(500);            
                    if (!taskWaitEventToHandle(OS_TICKS_PER_SEC * 1)) break;
                } 
                else
                {
                    if(40 == dtu_data_wait) 
                    {
                        OSTimeDly(500);
                        if (!taskWaitEventToHandle(OS_TICKS_PER_SEC * 1)) break;
                        dtu_data_wait = 0;    
                    } 
                    else
                    {
                        dtu_data_wait++;    
                    }
                }
                
                if(4 == mqtt_data_type) 
                {
                    if(6 == version_data_wait) 
                    {
                        OSTimeDly(500);
                        if (taskSendVersionData()) break;
                        version_data_wait = 0;    
                    } 
                    else
                    {
                        version_data_wait++;    
                    }
                }
                 
            } 
              
        }
             
           

__need_restart:
        mqtt_data_type  = 0;
        mqtt_data_ready  = 1;
        upgrade_data_ready = 0;
        mSerialRecvLineIndex = 0;
        cipsend_mode = 0;
        dprintf("[M35]: Something error, stop M35\n");
        runtimeStatusPush(DTUM35_RUNTIME_STATUS_DELAT_TO_RESTART);
        waitSomeTimeForRestart();
    }
}

//static unsigned char recvWhat = RECV_ATREPLY;
static INT32U tickEnterRecvAtReply = 0;
static INT16U dataLen;


//"+CMT: \"+8615021012496\",,\"2010/08/18 17:32:02+32\"\r\ntest\r\n";
// recvByte_RECV_XXX: 处在XXX状态的时候的接收函数;
// 返回值如果是!=0,表示这个状态的数据接完毕.
// 返回值如果是==0,表示仍需要接受XXXX.
//
// 接收电话号码前的引号.
static void recvByte_RECV_CMT_NUM_PRE_QUOTE(unsigned char b);
// 接收电话号码.
static void recvByte_RECV_CMT_NUM(unsigned char b);
// 接收短信内容前面的回车换行.
static void recvByte_RECV_CMT_PRE_CONTENT(unsigned char b);
// 接收短信内容.
static void recvByte_RECV_CMT_CONTENT(unsigned char b);
// 接收GPRS数据的长度.
static void recvByte_RECV_IPD_LENGTH(unsigned char b);
// 接收GPRS数据的内容.
static void recvByte_RECV_IPD_CONTENT(unsigned char b);
// recvByte_RECV_ATREPLY: 接收AT命令应答或接收自动上报报文.
static void recvByte_RECV_ATREPLY(unsigned char b);


static void (*recvByte)(unsigned char b) =  recvByte_RECV_ATREPLY;

static void recvByte_RECV_CMT_NUM_PRE_QUOTE(unsigned char b) {
    if (b == '\"') {
        recvByte = recvByte_RECV_CMT_NUM;
    }
}

static void recvByte_RECV_CMT_NUM(unsigned char b) {
    if (isdigit(b) || b == '+') {
        (void)RingBuffer_Write(&mSerialRecvBuffer, &b, 1);
        ++dataLen;
    } else if (b == '\"') {
        b = 0;
        (void)RingBuffer_Write(&mSerialRecvBuffer, &b, 1);
        ++dataLen;
        recvByte = recvByte_RECV_CMT_PRE_CONTENT;
    }
}

static void recvByte_RECV_CMT_PRE_CONTENT(unsigned char b) {
    if (b == '\n') {
        recvByte = recvByte_RECV_CMT_CONTENT;
    }
}

static void recvByte_RECV_CMT_CONTENT(unsigned char b) {
    if (b == '\r') {
        b = 0;
        (void)RingBuffer_Write(&mSerialRecvBuffer, &b, 1);
        ++dataLen;
        /*b = RECVED_CMT;
        (void)RingBuffer_Write(&mReceivedMessage, &b, 1);
        (void)RingBuffer_Write(&mReceivedMessage, (const unsigned char *__FAR)&dataLen, 2);
        (void)OSSemPost(mEvent);
        */mSerialRecvLineIndex = 0;
        recvByte = recvByte_RECV_ATREPLY;
    } else if (b != '\n') {
        (void)RingBuffer_Write(&mSerialRecvBuffer, &b, 1);
        ++dataLen;
    }
}

static void recvByte_RECV_IPD_LENGTH(unsigned char b) {
    (void)b;
    recvByte = recvByte_RECV_IPD_CONTENT;
}

static void recvByte_RECV_IPD_CONTENT(unsigned char b) {
    (void)b;
    mSerialRecvLineIndex = 0;
    recvByte = recvByte_RECV_ATREPLY;
}

static void recvByte_RECV_ATREPLY(unsigned char b) {
    if (b == '\r') {
        if (mSerialRecvLineIndex < 2) {
            mSerialRecvLineIndex = 0;
            return;
        }

        mSerialRecvLine[mSerialRecvLineIndex++] = 0;

        ATCMD_SerialRecvLine(mSerialRecvLine, mSerialRecvLineIndex);
        mSerialRecvLineIndex = 0;
        return;
    }

    if (b == '\n') {
        return;
    }
    if (mSerialRecvLineIndex >= (sizeof(mSerialRecvLine) - 2)) {
        return;
    }

    mSerialRecvLine[mSerialRecvLineIndex++] = b;
    if((b == '>')&&(mSerialRecvLineIndex == 1)) 
    {
        mSerialRecvLine[mSerialRecvLineIndex++] = 0;

        ATCMD_SerialRecvLine(mSerialRecvLine, mSerialRecvLineIndex);
        mSerialRecvLineIndex = 0;
        return;
    }
    
    if (mSerialRecvLineIndex == 3) {
        if (memcmp((unsigned char *__FAR)mSerialRecvLine, (unsigned char *__FAR)"IPD", 3) != 0) {
            return;
        }
        // GOT "IPD" header, enter IPD recv flow.
        dataLen = 0;
        recvByte = recvByte_RECV_IPD_LENGTH;
        tickEnterRecvAtReply = OSTimeGet() + OS_TICKS_PER_SEC;
        return;
    }

    if (mSerialRecvLineIndex == 5) {
        if (memcmp((unsigned char *__FAR)mSerialRecvLine, (unsigned char *__FAR)"+CMT:", 5) != 0) {
            return;
        }
        // GOT "+CMT" header, enter CMT recv flow.
        dataLen = 0;
        recvByte = recvByte_RECV_CMT_NUM_PRE_QUOTE;
        tickEnterRecvAtReply = OSTimeGet() + OS_TICKS_PER_SEC * 3 / 2;
        return;
    }
}

static void recvByte_RECV_DATA(unsigned char b) {
    
    if (mSerialRecvLineIndex >= (sizeof(mSerialRecvLine))) {
        return;
    }

    if(0 == mqtt_data_type)
    {
        if((0 == mSerialRecvLineIndex) && (0x20 != b))
        { 
            return;
        }
        if((1 == mSerialRecvLineIndex) && (0x02 != b))
        { 
            mSerialRecvLineIndex = 0;
            return;
        }
        if((2 == mSerialRecvLineIndex) && (0x00 != b))
        { 
            mSerialRecvLineIndex = 0;
            return;
        }
        if((3 == mSerialRecvLineIndex) && (0x00 != b))
        { 
            mSerialRecvLineIndex = 0;
            return;
        }
        mSerialRecvLine[mSerialRecvLineIndex++] = b;
        
        if(4 == mSerialRecvLineIndex) 
        {
            mSerialRecvLineIndex = 0;
            mqtt_data_ready = 1;
            mqtt_data_type = 1;
            return;
        }
    }
    
    if((3 >= mqtt_data_type) && (mqtt_data_type > 0)) 
    {
        if((0 == mSerialRecvLineIndex) && (0x90 != b) && (0x30 != (b & 0xf0))) 
        { 
            return;
        }
        
        mSerialRecvLine[mSerialRecvLineIndex++] = b;
        
        if(0x90 == mSerialRecvLine[0]) 
        {
            if((2 == mSerialRecvLineIndex) && (0x03 != b))
            { 
                mSerialRecvLineIndex = 0;
                return;
            }
            if((3 == mSerialRecvLineIndex) && (0x00 != b))
            { 
                mSerialRecvLineIndex = 0;
                return;
            }
            if((4 == mSerialRecvLineIndex) && (0x01 != b))
            { 
                mSerialRecvLineIndex = 0;
                return;
            }
            if((5 == mSerialRecvLineIndex) && (0x00 != b))
            { 
                mSerialRecvLineIndex = 0;
                return;
            }
            
            if(5 == mSerialRecvLineIndex) 
            {
                mSerialRecvLineIndex = 0;
                mqtt_data_ready = 1;
                mqtt_data_type++;
                return;
            } 
        }
        
        if(0x30 == (mSerialRecvLine[0] & 0xf0)) 
        { 
            mqtt_data_type = 5;
            mqtt_data_ready = 0;
            return;
        }
    }                       
    
    
    if(4 == mqtt_data_type) 
    {
        if((0 == mSerialRecvLineIndex) && (0xd0 != b) && (0x30 != (b & 0xf0))) 
        { 
            return;
        }
        
        mSerialRecvLine[mSerialRecvLineIndex++] = b;
        if(0xd0 == mSerialRecvLine[0]) 
        { 
            if((2 == mSerialRecvLineIndex) && (0x00 != b))
            { 
                mSerialRecvLineIndex = 0;
                return;
            }
            
            if(2 == mSerialRecvLineIndex) 
            {
                mSerialRecvLineIndex = 0;
                mqtt_data_ready = 1;
                return;
            }
        }
        
        if(0x30 == (mSerialRecvLine[0] & 0xf0)) 
        { 
            mqtt_data_type = 5;
            mqtt_data_ready = 0;
            return;
        }

    }
    
    if(5 == mqtt_data_type)
    {
        if(0 == mSerialRecvLineIndex)
        { 
            if(0x30 != (b & 0xf0)) 
            {
                return;
            }
        }
        
        mSerialRecvLine[mSerialRecvLineIndex++] = b;
        
        if( ((0 == mSerialRecvLine[1] >>7) && (mSerialRecvLineIndex == mSerialRecvLine[1] + 2)) ||
            ((1 == mSerialRecvLine[1] >>7) && (mSerialRecvLineIndex == (mSerialRecvLine[1]&0x7f) + mSerialRecvLine[2]*128 + 3)) )
        {
            (void)safe_memcpy((PINT8U)mUpgradeData,(PINT8U)mSerialRecvLine,mSerialRecvLineIndex);
            mSerialRecvLineIndex = 0;
            upgrade_data_ready = 1;
            return;
        }
        
    }
    
    
    
    return;
}

void DTUM35_RecvByte(unsigned char b) {

    if(0 == cipsend_mode) 
    {
        if (recvByte != recvByte_RECV_ATREPLY) {
            if (OSTimeGet() > tickEnterRecvAtReply) {
                mSerialRecvLineIndex = 0;
                recvByte = recvByte_RECV_ATREPLY;
            }
        } 
    } 
    else
    {
        recvByte = recvByte_RECV_DATA;
    }
    recvByte(b);
}


INT16U DTUM35_SendData(const unsigned char *__FAR dat, INT16U len, INT8U isMaster) {
    OS_CPU_SR cpu_sr = 0;
    INT8U type;
    OS_ENTER_CRITICAL();
    if (RingBuffer_LeftSpace(&mReceivedMessage) >= len + 3) {
        type = isMaster ? DATA_FOR_MASTER_SEND : DATA_FOR_SLAVE_SEND;
        (void)RingBuffer_Write(&mReceivedMessage, &type, 1);
        (void)RingBuffer_Write(&mReceivedMessage, (const unsigned char *)&len, 2);
        (void)RingBuffer_Write(&mReceivedMessage, dat, len);
        (void)OSSemPost(mEvent);
        OS_EXIT_CRITICAL();
        return len;
    }

    OS_EXIT_CRITICAL();
    return 0;
}

char DTUM35_SendDataWithCallback(struct DTUM35_SendDataInfo *__FAR info) {
    OS_CPU_SR cpu_sr = 0;
    INT8U type;
    INT16U len;
    OS_ENTER_CRITICAL();
    if (RingBuffer_LeftSpace(&mReceivedMessage) >= (sizeof(*info) + 3)) {
        type = DATA_FOR_SEND_WITH_CALLBACK;
        len = sizeof(*info);
        (void)RingBuffer_Write(&mReceivedMessage, &type, 1);
        (void)RingBuffer_Write(&mReceivedMessage, (const unsigned char *)&len, 2);
        (void)RingBuffer_Write(&mReceivedMessage, (const unsigned char *__FAR)info, sizeof(*info));
        (void)OSSemPost(mEvent);
        OS_EXIT_CRITICAL();
        return 1;
    }

    OS_EXIT_CRITICAL();
    return 0;
}
         
void DTUM35_Init(const DTUM35_BspInterface *__FAR interface, char for_test) {
    if (!DTUM35_INTERFACE_IS_VALID(interface)) {
        return;
    }
    
    mIsForTest = for_test;

    frameHeaderInit();
    bspInterface = interface;
    memset(mTaskStack, 0xFF, sizeof(mTaskStack));
    mEvent = OSSemCreate(0);
    (void)OSTaskCreate(task, (void *)0, (OS_STK *)&mTaskStack[sizeof(mTaskStack) - 1], DTU_TX_TASK_PRIO);
}

INT8U DTUM35_GetRuntimeStatus(INT8U index) {
    if (index < sizeof(mRuntimeStatus)) {
        INT8U ret;
        OS_CPU_SR cpu_sr = 0;
        OS_ENTER_CRITICAL();
        ret = mRuntimeStatus[index];
        OS_EXIT_CRITICAL();
        return ret;

    } else {
        return DTUM35_RUNTIME_STATUS_UNKNOWN;
    }
}

void DTUM35_GetRuntimeStatusAll(INT8U *buf, unsigned char buffer_len) {
    OS_CPU_SR cpu_sr = 0;

    if (buffer_len > sizeof(mRuntimeStatus)) {
        buffer_len = sizeof(mRuntimeStatus);
    }

    OS_ENTER_CRITICAL();
    memcpy(buf, mRuntimeStatus, buffer_len);
    OS_EXIT_CRITICAL();
}

/*
static void test_simulate_recv_byte(unsigned char b) {
    OS_CPU_SR cpu_sr = 0;
    OS_ENTER_CRITICAL();
    DTUM35_RecvByte(b);
    OS_EXIT_CRITICAL();
}

static void test_simulate_recv_string(const unsigned char *__FAR s) {
    while (*s) {
        test_simulate_recv_byte(*s++);
    }
}

static void unit_test_task(void *unused) {
    INT8U err;
    static const char *const test_cmt = "+CMT: \"+8615021012496\",,\"2010/08/18 17:32:02+32\"\r\ntest\r\n";

    (void)unused;

    for (;;) {
        test_simulate_recv_string((const unsigned char *__FAR)test_cmt);
        OSSemPend(mEvent, OS_TICKS_PER_SEC * 5, &err);
        if (err == OS_ERR_NONE) {
            INT8U type;
            INT16U len;
            unsigned char buf[20];
            OS_CPU_SR cpu_sr = 0;
            OS_ENTER_CRITICAL();
            (void)RingBuffer_Read(&mReceivedMessage, &type, 1);
            (void)RingBuffer_Read(&mReceivedMessage, (unsigned char *)&len, 2);
            OS_EXIT_CRITICAL();
            if (type == DATA_FOR_SLAVE_SEND || type == DATA_FOR_MASTER_SEND) {
                while (len > 0) {
                    len -= RingBuffer_Read(&mReceivedMessage, buf, len > sizeof(buf) ? sizeof(buf) : len);
                }
            } else if (type == RECVED_IPD || type == RECVED_CMT) {
                while (len > 0) {
                    len -= RingBuffer_Read(&mSerialRecvBuffer, buf, len > sizeof(buf) ? sizeof(buf) : len);
                }
            }
        }
    }
}

void DTUM35_UnitTestInit(void) {
    frameHeaderInit();
    memset(mTaskStack, 0xFF, sizeof(mTaskStack));
    mEvent = OSSemCreate(0);
    (void)OSTaskCreate(unit_test_task, (void *)0, (OS_STK *)&mTaskStack[sizeof(mTaskStack) - 1], GPRS_TASK_PRIO);
}
*/


unsigned char DTUM35_GetOPS(void) {
    unsigned char ret;
    OS_CPU_SR cpu_sr = 0;
    OS_ENTER_CRITICAL();
    ret = mOps;
    OS_EXIT_CRITICAL();
    return ret;
}

void DTUM35_GetSingalQuality(unsigned char *strength, unsigned char *err_rate) {
    OS_CPU_SR cpu_sr = 0;
    OS_ENTER_CRITICAL();
    if (strength) {
        *strength = mSignalStrength;
    }
    if (err_rate) {
        *err_rate = mBitErrorRate;
    }
    OS_EXIT_CRITICAL();
}

void DTUM35_GetLacCi(unsigned short lac_ci[2]) {
    if (lac_ci) {
        OS_CPU_SR cpu_sr = 0;
        OS_ENTER_CRITICAL();
        lac_ci[0] = mLacCi[0];
        lac_ci[1] = mLacCi[1];
        OS_EXIT_CRITICAL();
    }
}
void DTUM35_GetCIMI(char *cimi, unsigned char len) {
    if (!cimi) {
        return;
    }
    if (!len) {
        return;
    }
    if (len > strlen(mSimCardData.cimi)) {
        len = (unsigned char)strlen(mSimCardData.cimi);
    } else {
        len = len - 1;
    }
    {
        OS_CPU_SR cpu_sr = 0;
        OS_ENTER_CRITICAL();
        memcpy((unsigned char *__FAR)cimi, (unsigned char *__FAR)mSimCardData.cimi, len);
        OS_EXIT_CRITICAL();
        cimi[len] = 0;
    }
}

void DTUM35_GetIMEI(char *imei, unsigned char len) {

    if (!imei) {
        return;
    }
    if (!len) {
        return;
    }
    if (len > strlen(mIMEI)) {
        len = (unsigned char)strlen(mIMEI);
    } else {
        len = len - 1;
    }
    {
        OS_CPU_SR cpu_sr = 0;
        OS_ENTER_CRITICAL();
        memcpy((unsigned char *__FAR)imei, (unsigned char *__FAR)mIMEI, len);
        OS_EXIT_CRITICAL();
        imei[len] = 0;
    }
}
void DTUM35_GetPhoneNum(char *phone, unsigned char len) {
    if (!phone) {
        return;
    }
    if (!len) {
        return;
    }
    if (len > strlen(mSimCardData.phone)) {
        len = (unsigned char)strlen(mSimCardData.phone);
    } else {
        len = len - 1;
    }
    {
        OS_CPU_SR cpu_sr = 0;
        OS_ENTER_CRITICAL();
        memcpy((unsigned char *__FAR)phone, (unsigned char *__FAR)mSimCardData.phone, len);
        OS_EXIT_CRITICAL();
        phone[len] = 0;
    }
}

INT32U DTUM35_GetDataNum(void) 
{
    return mPacketCounter;
}

