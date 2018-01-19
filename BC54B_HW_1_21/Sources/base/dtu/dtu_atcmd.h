/// \file dtu_atcmd.h
/// \brief DTU AT����ķ�װ����.
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
    /// ����Ӧ�����ʼ�ַ���.
    const char *__FAR expect;
    /// ���ڱ���ƥ���Ӧ���еĻ���.
    char *recvBuffer;
    /// �����С.
    INT8U recvLen;
} ATCMD_Expect;

/// \brief ATCMD_SerialRecvLine AT�������һ������.
///
/// \param line ���յ�����
/// \param len ���յ����ݳ���
void ATCMD_SerialRecvLine(unsigned char *__FAR line, INT16U len);

/// \brief ATCMD_RuntimeInit AT�������л�����ʼ��.
///
/// \param send_byte ͨ��AT���ڷ������ݵĺ���.
void ATCMD_RuntimeInit(void (*send_bytes)(const unsigned char *__FAR bytes, int len));

/// \brief ATCMD_ExecCommand ִ��AT����, ���ȴ���Ӧ.
///
/// \param cmd AT�����ַ���, ������'\r'.
/// \param expect ������Ӧ��.
/// \param timeoutTick ��ʱʱ��.
///
/// \return TRUE AT����ȵ�������Ӧ��; FALSE �ȴ�����Ӧ��ʱ.
Bool ATCMD_ExecCommand(const char *__FAR cmd, const ATCMD_Expect *__FAR expect, INT32U timeoutTick);

/// \brief ATCMD_RetryUntilExpect ��γ��Է���AT����, ֱ���ȵ�����Ӧ���ʱ.
///
/// \param atcmd ���͵�AT����.
/// \param expect ����Ӧ����ַ���.
/// \param times ���ԵĴ���.
/// \param timeout ÿ�εȴ��ĳ�ʱ.
///
/// \return TRUE ���ͳɹ�, ���ȵ�������Ӧ��; FALSE ���Զ��û�еȵ�����Ӧ��.
Bool ATCMD_RetryUntilExpect(const char *__FAR atcmd, const char *__FAR expect, INT8U times, INT16U timeout);
Bool ATCMD_RetryUntilExpects(const char *__FAR atcmd, const char *__FAR const *__FAR expects, INT16U times, INT16U timeout);

/// \brief ATCMD_ConnectTcpServer ���ӵ�������.
///
/// \param addr �������ĵ�ַ, ������IP��ַ��������.
/// \param port ���ӵ�TCP�˿�.
///
/// \return TRUE ���ӷ������ɹ�; FALSE ���ӷ�����ʧ��.
Bool ATCMD_ConnectTcpServer(const char *__FAR addr, INT16U port);

/// \brief ATCMD_SetCPIMode ����͸��ģʽ
///
/// \param cpi_mode ͸��ģʽ 0����͸��  1��͸��
///
/// \return TRUE ���óɹ�; FALSE ����ʧ��.
Bool ATCMD_SetCPIMode(INT8U cpi_mode);


/// \brief ATCMD_SendSMS ���Ͷ���.
///
/// \param dest ���ն˵ĵ绰����.
/// \param msg ���͵���Ϣ����.
///
/// \return TRUE ���ͳɹ�; FALSE ����ʧ��.
Bool ATCMD_SendSMS(const char *__FAR dest, const char *__FAR msg);

/// \brief ATCMD_WaitTextSMSFrom �ȴ�ĳ���ֻ��ŷ������Ķ���.
///
/// \param from ���Ͷ˵ĵ绰����.
/// \param msg �����յ�����Ϣ��ָ��.
/// \param len ������󱣴�Ľ��յ�����Ϣ�ĳ���.
/// \param timeoutTick ��ʱʱ��.
///
/// \return TRUE ���յ�����; FALSE ���ճ�ʱ.
Bool ATCMD_WaitTextSMSFrom(const char *__FAR from, char *__FAR msg, INT8U len, INT32U timeoutTick);

/// \brief ATCMD_SendDataViaGPRS ͨ��GPRS��������.
///
/// \param dat ��Ҫ���͵����ݵ�ָ��.
/// \param len ��Ҫ���͵����ݵĳ���.
///
/// \return TRUE ���ͳɹ�; FALSE ���ʹ���.
Bool ATCMD_SendDataViaGPRS(const unsigned char *__FAR dat, INT16U len);

typedef INT16U (*ATCMD_GetSentData)(void *__FAR private_data, char *buf, INT16U len);

/// \brief ATCMD_SendDataViaGPRSCallback ͨ��GPRS��������, ���͵�����ͨ�������һ��������ȡ.
///
/// \param len ��Ҫ���͵����ݵĳ���.
/// \param getSendData ��ȡ���ݵĺ���.
///
/// \return TRUE ���ͳɹ�; FALSE ���ʹ���.
Bool ATCMD_SendDataViaGPRSCallback(void *__FAR private_data, ATCMD_GetSentData getSendData, INT16U len);

Bool ATCMD_GetOPS(char *buf, INT8U len);
Bool ATCMD_GetLacCi(unsigned short lac_ci[2]);
Bool ATCMD_GetCIMI(char *buf, INT8U len);
Bool ATCMD_GetIMEI(char *buf, INT8U len);
Bool ATCMD_GetSignalQuality(unsigned char *rssi, unsigned char *ber);
Bool ATCMD_GetDataSentInfo(unsigned long *__FAR sent, unsigned long *__FAR acked, unsigned long *__FAR unacked);

#endif

