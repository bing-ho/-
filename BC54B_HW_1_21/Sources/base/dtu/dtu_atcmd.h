/// \file dtu_atcmd.h
/// \brief DTU AT命令的封装声明.
/// \author Xi Qingping, qingping.xi@ligoo.cn
/// \version
/// \date 2015-09-01

#ifndef __ATCMD_H__
#define __ATCMD_H__

#include "includes.h"

#ifndef __FAR
#define __FAR
#endif


typedef struct {
    /// 期望应答的起始字符串.
    const char *__FAR expect;
    /// 用于保存匹配的应答行的缓存.
    char *recvBuffer;
    /// 缓存大小.
    INT8U recvLen;
} ATCMD_Expect;

/// \brief ATCMD_SerialRecvLine AT命令接收一行数据.
///
/// \param line 接收的数据
/// \param len 接收的数据长度
void ATCMD_SerialRecvLine(unsigned char *__FAR line, INT16U len);

/// \brief ATCMD_RuntimeInit AT命令运行环境初始化.
///
/// \param send_byte 通过AT串口发送数据的函数.
void ATCMD_RuntimeInit(void (*send_bytes)(const unsigned char *__FAR bytes, int len));

/// \brief ATCMD_ExecCommand 执行AT命令, 并等待回应.
///
/// \param cmd AT命令字符串, 不包括'\r'.
/// \param expect 期望的应答.
/// \param timeoutTick 超时时间.
///
/// \return TRUE AT命令等到了期望应答; FALSE 等待期望应答超时.
Bool ATCMD_ExecCommand(const char *__FAR cmd, const ATCMD_Expect *__FAR expect, INT32U timeoutTick);

/// \brief ATCMD_RetryUntilExpect 多次尝试发送AT命令, 直到等到期望应答或超时.
///
/// \param atcmd 发送的AT命令.
/// \param expect 期望应答的字符串.
/// \param times 尝试的次数.
/// \param timeout 每次等待的超时.
///
/// \return TRUE 发送成功, 并等到了期望应答; FALSE 尝试多次没有等到期望应答.
Bool ATCMD_RetryUntilExpect(const char *__FAR atcmd, const char *__FAR expect, INT8U times, INT16U timeout);
Bool ATCMD_RetryUntilExpects(const char *__FAR atcmd, const char *__FAR const *__FAR expects, INT16U times, INT16U timeout);

/// \brief ATCMD_ConnectTcpServer 连接到服务器.
///
/// \param addr 服务器的地址, 可以是IP地址或者域名.
/// \param port 连接的TCP端口.
///
/// \return TRUE 连接服务器成功; FALSE 连接服务器失败.
Bool ATCMD_ConnectTcpServer(const char *__FAR addr, INT16U port);

/// \brief ATCMD_SetCPIMode 设置透传模式
///
/// \param cpi_mode 透传模式 0：非透传  1：透传
///
/// \return TRUE 设置成功; FALSE 设置失败.
Bool ATCMD_SetCPIMode(INT8U cpi_mode);


/// \brief ATCMD_SendSMS 发送短信.
///
/// \param dest 接收端的电话号码.
/// \param msg 发送的信息内容.
///
/// \return TRUE 发送成功; FALSE 发送失败.
Bool ATCMD_SendSMS(const char *__FAR dest, const char *__FAR msg);

/// \brief ATCMD_WaitTextSMSFrom 等待某个手机号发过来的短信.
///
/// \param from 发送端的电话号码.
/// \param msg 保存收到的信息的指针.
/// \param len 可以最大保存的接收到的信息的长度.
/// \param timeoutTick 超时时间.
///
/// \return TRUE 接收到短信; FALSE 接收超时.
Bool ATCMD_WaitTextSMSFrom(const char *__FAR from, char *__FAR msg, INT8U len, INT32U timeoutTick);

/// \brief ATCMD_SendDataViaGPRS 通过GPRS发送数据.
///
/// \param dat 需要发送的数据的指针.
/// \param len 需要发送的数据的长度.
///
/// \return TRUE 发送成功; FALSE 发送错误.
Bool ATCMD_SendDataViaGPRS(const unsigned char *__FAR dat, INT16U len);

typedef INT16U (*ATCMD_GetSentData)(void *__FAR private_data, char *buf, INT16U len);

/// \brief ATCMD_SendDataViaGPRSCallback 通过GPRS发送数据, 发送的数据通过传入的一个函数读取.
///
/// \param len 需要发送的数据的长度.
/// \param getSendData 读取数据的函数.
///
/// \return TRUE 发送成功; FALSE 发送错误.
Bool ATCMD_SendDataViaGPRSCallback(void *__FAR private_data, ATCMD_GetSentData getSendData, INT16U len);

Bool ATCMD_GetOPS(char *buf, INT8U len);
Bool ATCMD_GetLacCi(unsigned short lac_ci[2]);
Bool ATCMD_GetCIMI(char *buf, INT8U len);
Bool ATCMD_GetIMEI(char *buf, INT8U len);
Bool ATCMD_GetSignalQuality(unsigned char *rssi, unsigned char *ber);
Bool ATCMD_GetDataSentInfo(unsigned long *__FAR sent, unsigned long *__FAR acked, unsigned long *__FAR unacked);

#endif

