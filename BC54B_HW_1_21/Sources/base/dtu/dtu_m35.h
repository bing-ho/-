/// \file dtu_m35.h
/// \brief M35��������.
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


/// ADS1015�ײ�ӿ�.
/// Ӳ������Ҫ���������ģ���ṩ�Ľӿ�.
typedef struct {
#if DTUM35_CHECK != 0
    unsigned long magic;
#endif
    /// \brief ���������Ϣ��EEPROM.
    ///
    /// \param dat: ��Ҫ�������Ϣ.
    /// \param size: ������Ϣ�ĳ���, ���64byte.
    /// \return ��������ݳ���.
    int (*nvm_restore_data)(simcard_data_t *__FAR dat);

    /// \brief ��ȡ������EEPROM��������Ϣ.
    ///
    /// \param dat: ��ȡ����Ϣ���������ַ.
    /// \param size: ����ȡsize�ֽڵ�����
    ///
    /// \return ��ȡ�����ݵĳ���.
    int (*nvm_store_data)(const simcard_data_t *__FAR dat);

    /// \brief ��ʼ������IO.
    ///
    /// \param baud ��M35ͨѶ�Ĵ���ʹ�õĲ�����.
    ///
    /// \note �����������Commu_Init�б�����.
    void (*m35_control_init)(int baud);

    /// \brief ����M35�ĵ�Դ.
    ///
    /// \param isEnable: 1 ��M35��Դ; 0 �ر�M35��Դ.
    void (*m35_power_enable)(char isEnable);

    /// \brief ����M35��PWR_KEY.
    ///
    /// \param isAssert: 1 ����M35��PWR_KEY�ܽ�; 0 ����M35��PWR_KEY�ܽ�.
    void (*m35_set_pwrkey)(char isAssert);

    /// \brief ��ѯM35�Ƿ��ϵ�����.
    ///
    /// \return 1 M35�Ѿ��ϵ�; 0 M35û���ϵ�.
    char (*m35_is_poweron)(void);

    /// \brief ����һ���ֽڵ����ݵ�M35.
    ///
    /// \param b��Ҫ���͵�����.
    void (*m35_serial_send)(const unsigned char *__FAR bytes, int len);

} DTUM35_BspInterface;

/// \brief DTUM35_RecvByte �Ӵ��ڽ���һ��M35���͵�����, �������Ӧ���ڴ��ڵĽ����ж��е���.
///
/// \param b ���յ�����.
void DTUM35_RecvByte(unsigned char b);

/// \brief DTUM35_Init ��ʼ��DTUM35����ģ��.
/// ��������лᴴ��һ������.
///
/// \param bspInterface ��BSP��صĺ�����ʵ��.
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

/// \brief DTUM35_GetRunTimeStatus ��ȡ����״̬.
///
/// \param index ����״̬�����, 0��ǰ����״̬, 1��һ�����������״̬, 2���ϴ������Ĵ���״̬...
///
/// \return ����״̬.
INT8U DTUM35_GetRuntimeStatus(INT8U index);
void DTUM35_GetRuntimeStatusAll(INT8U *buf, unsigned char buffer_len);

/// \brief DTUM35_SendData ͨ��DTUM35�������ģ�鷢������.
///
/// \param dat ��Ҫ���͵�����.
/// \param len ���ݵĳ���.
/// \param isMaster �Ƿ�Ϊ��������.
/// \return ���浽���ͻ���ĳ���.
//INT16U DTUM35_SendData(const unsigned char *__FAR dat, INT16U len, INT8U isMaster);


struct DTUM35_SendDataInfo  {
    ATCMD_GetSentData get_data;
    void *__FAR private_data;
    void (*finished)(char is_ok); // ���ݷ��ͳɹ�֮������������.
    INT16U len;
};

char DTUM35_SendDataWithCallback(struct DTUM35_SendDataInfo *__FAR info);
INT16U DTUM35_ProtocolGetBCUHeader(unsigned char *__FAR dat, INT16U packed_dat_length);
INT16U DTUM35_ProtocolGetBMUHeader(unsigned char *__FAR dat, INT16U packed_dat_length);
INT16U DTUM35_ProtocolGetHeaderLength(void);
INT32U DTUM35_GetPacketCounter(void);


/// DTU��Ӫ���б�.
enum DTU_OPS {
    /// δ֪��Ӫ��.
    DTU_OPS_UNKNOWN = 0,
    /// �й���ͨ.
    DTU_OPS_CHINA_UNIONCOM = 1,
    /// �й��ƶ�.
    DTU_OPS_CHINA_MOBILE = 2,
    /// �й�����.
    DTU_OPS_CHINA_TELECOM = 3,
};

/// \brief DTUM35_GetOPS ��ȡע����������Ӫ��.
///
/// \return DTU_OPS�������Ӫ��.
unsigned char DTUM35_GetOPS(void);

/// \brief DTUM35_GetSingalQuality ��ȡ�ź�ǿ�ȼ�������.
///
/// \param strength �ź�ǿ�ȴ洢�����ָ��ָ����ֽ�.
/// \param err_rate �����ʴ洢�����ָ��ָ����ֽ�.
void DTUM35_GetSingalQuality(unsigned char *strength, unsigned char *err_rate);

/// \brief DTUM35_GetLacCi ��ȡע��Ļ�վ��LAC��CI��
///
/// \param lacci[2]��LAC,CI�洢�����ָ��ָ����ڴ�, �ȴ�LAC, �ٴ�CI.
void DTUM35_GetLacCi(unsigned short lacci[2]);

/// \brief DTUM35_GetCIMI ��ȡSIM����Ψһ��ʶ.
///
/// \param cimi CIMI�洢�����ָ��ָ����ڴ�.
/// \param len cimi����ڴ�ĳ���, ��ֹԽ�����.
void DTUM35_GetCIMI(char *cimi, unsigned char len);

/// \brief DTUM35_GetIMEI ��ȡGSMģ���Ψһ��ʶ.
///
/// \param imei IMEI�洢�����ָ��ָ����ڴ�.
/// \param len imei����ڴ�ĳ���, ��ֹԽ�����.
void DTUM35_GetIMEI(char *imei, unsigned char len);

/// \brief DTUM35_GetPhoneNum ��ȡSIM����Ӧ�ĵ绰��.
///
/// \param phone �绰����洢�����ָ��ָ����ڴ�.
/// \param len phone����ڴ�ĳ���, ��ֹԽ�����.
void DTUM35_GetPhoneNum(char *phone, unsigned char len);


INT32U DTUM35_GetDataNum(void);


#endif

