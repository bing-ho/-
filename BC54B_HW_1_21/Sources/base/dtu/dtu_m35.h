/// \file dtu_m35.h
/// \brief M35驱动声明.
/// \author Xi Qingping, qingping.xi@ligoo.cn
/// \version
/// \date 2015-09-01
#ifndef __DTU_M35_H__
#define __DTU_M35_H__

#include "includes.h"
#include "dtu_atcmd.h"

#ifndef __FAR
#define __FAR
#endif

#ifndef DTUM35_CHECK
#define DTUM35_CHECK 1
#endif

#define DTUM35_CHECK_MAGIC (0x77348c6dUL)

#define OTA_PGN_NUM_MAX                (22)

typedef struct {
    char cimi[22];
    char phone[12];
} simcard_data_t;


/// ADS1015底层接口.
/// 硬件层需要向这个功能模块提供的接口.
typedef struct {
#if DTUM35_CHECK != 0
    unsigned long magic;
#endif
    /// \brief 保存相关信息到EEPROM.
    ///
    /// \param dat: 需要保存的信息.
    /// \param size: 保存信息的长度, 最大64byte.
    /// \return 保存的数据长度.
    int (*nvm_restore_data)(simcard_data_t *__FAR dat);

    /// \brief 读取保存在EEPROM里的相关信息.
    ///
    /// \param dat: 读取的信息放在这个地址.
    /// \param size: 最大读取size字节的数据
    ///
    /// \return 读取的数据的长度.
    int (*nvm_store_data)(const simcard_data_t *__FAR dat);

    /// \brief 初始化控制IO.
    ///
    /// \param baud 与M35通讯的串口使用的波特率.
    ///
    /// \note 这个函数会在Commu_Init中被调用.
    void (*m35_control_init)(int baud);

    /// \brief 设置M35的电源.
    ///
    /// \param isEnable: 1 打开M35电源; 0 关闭M35电源.
    void (*m35_power_enable)(char isEnable);

    /// \brief 设置M35的PWR_KEY.
    ///
    /// \param isAssert: 1 拉低M35的PWR_KEY管脚; 0 拉高M35的PWR_KEY管脚.
    void (*m35_set_pwrkey)(char isAssert);

    /// \brief 查询M35是否上电正常.
    ///
    /// \return 1 M35已经上电; 0 M35没有上电.
    char (*m35_is_poweron)(void);

    /// \brief 发送一个字节的数据到M35.
    ///
    /// \param b需要发送的数据.
    void (*m35_serial_send)(const unsigned char *__FAR bytes, int len);

} DTUM35_BspInterface;

/// \brief DTUM35_RecvByte 从串口接收一个M35发送的数据, 这个函数应该在串口的接收中断中调用.
///
/// \param b 接收的数据.
void DTUM35_RecvByte(unsigned char b);

/// \brief DTUM35_Init 初始化DTUM35功能模块.
/// 这个函数中会创建一个任务.
///
/// \param bspInterface 和BSP相关的函数的实现.
void DTUM35_Init(const DTUM35_BspInterface *__FAR bspInterface, char for_test);
void DTUM35_UnitTestInit(void);

enum {
    DTUM35_RUNTIME_STATUS_UNKNOWN = 0,
    DTUM35_RUNTIME_STATUS_POWERUP,
    DTUM35_RUNTIME_STATUS_CONFIGURE,
    DTUM35_RUNTIME_STATUS_WAIT_REGISTER,
    DTUM35_RUNTIME_STATUS_SEND_SMS,
    DTUM35_RUNTIME_STATUS_CONNECT_SERVER,
    DTUM35_RUNTIME_STATUS_GET_SIGNAL_QUALITY,
    DTUM35_RUNTIME_STATUS_CHECK_CONNECTION,
    DTUM35_RUNTIME_STATUS_WAIT_EVENT,
    DTUM35_RUNTIME_STATUS_SEND_DATA,
    DTUM35_RUNTIME_STATUS_RECV_DATA,
    DTUM35_RUNTIME_STATUS_RECV_SMS,
    DTUM35_RUNTIME_STATUS_DELAT_TO_RESTART,
    DTUM35_RUNTIME_STATUS_SIM_INIT,
};

/// \brief DTUM35_GetRunTimeStatus 获取运行状态.
///
/// \param index 运行状态的序号, 0当前运行状态, 1上一次重启的最后状态, 2上上次重启的错误状态...
///
/// \return 运行状态.
INT8U DTUM35_GetRuntimeStatus(INT8U index);
void DTUM35_GetRuntimeStatusAll(INT8U *buf, unsigned char buffer_len);

/// \brief DTUM35_SendData 通过DTUM35这个功能模块发送数据.
///
/// \param dat 需要发送的数据.
/// \param len 数据的长度.
/// \param isMaster 是否为主机数据.
/// \return 保存到发送缓冲的长度.
//INT16U DTUM35_SendData(const unsigned char *__FAR dat, INT16U len, INT8U isMaster);


struct DTUM35_SendDataInfo  {
    ATCMD_GetSentData get_data;
    void *__FAR private_data;
    void (*finished)(char is_ok); // 数据发送成功之后调用这个函数.
    INT16U len;
};

char DTUM35_SendDataWithCallback(struct DTUM35_SendDataInfo *__FAR info);
INT16U DTUM35_ProtocolGetBCUHeader(unsigned char *__FAR dat, INT16U packed_dat_length);
INT16U DTUM35_ProtocolGetBMUHeader(unsigned char *__FAR dat, INT16U packed_dat_length);
INT16U DTUM35_ProtocolGetHeaderLength(void);
INT32U DTUM35_GetPacketCounter(void);


/// DTU运营商列表.
enum DTU_OPS {
    /// 未知运营商.
    DTU_OPS_UNKNOWN = 0,
    /// 中国联通.
    DTU_OPS_CHINA_UNIONCOM = 1,
    /// 中国移动.
    DTU_OPS_CHINA_MOBILE = 2,
    /// 中国电信.
    DTU_OPS_CHINA_TELECOM = 3,
};

/// \brief DTUM35_GetOPS 获取注册的网络的运营商.
///
/// \return DTU_OPS定义的运营商.
unsigned char DTUM35_GetOPS(void);

/// \brief DTUM35_GetSingalQuality 获取信号强度及误码率.
///
/// \param strength 信号强度存储在这个指针指向的字节.
/// \param err_rate 误码率存储在这个指针指向的字节.
void DTUM35_GetSingalQuality(unsigned char *strength, unsigned char *err_rate);

/// \brief DTUM35_GetLacCi 获取注册的基站的LAC和CI．
///
/// \param lacci[2]　LAC,CI存储在这个指针指向的内存, 先存LAC, 再存CI.
void DTUM35_GetLacCi(unsigned short lacci[2]);

/// \brief DTUM35_GetCIMI 获取SIM卡的唯一标识.
///
/// \param cimi CIMI存储在这个指针指向的内存.
/// \param len cimi这个内存的长度, 防止越界操作.
void DTUM35_GetCIMI(char *cimi, unsigned char len);

/// \brief DTUM35_GetIMEI 获取GSM模块的唯一标识.
///
/// \param imei IMEI存储在这个指针指向的内存.
/// \param len imei这个内存的长度, 防止越界操作.
void DTUM35_GetIMEI(char *imei, unsigned char len);

/// \brief DTUM35_GetPhoneNum 获取SIM卡对应的电话号.
///
/// \param phone 电话号码存储在这个指针指向的内存.
/// \param len phone这个内存的长度, 防止越界操作.
void DTUM35_GetPhoneNum(char *phone, unsigned char len);


INT32U DTUM35_GetDataNum(void);


#endif

