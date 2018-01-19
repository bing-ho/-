/**
  * @file       relay_fault_check.h
  * @brief      �̵������ϼ��
  *	@copyright	Ligoo Inc.
  *	@date       2017.03.17
  *	@author     Ligoo����߼���
  */
#ifndef	_RELAY_FAULT_CHECK_H_
#define	_RELAY_FAULT_CHECK_H_
#include "includes.h"


/**
  * @enum   RelayFaultCheckStatus
  * @brief  �̵������ϼ��״̬
  */
typedef enum
{
	kRelayFaultCheck_Nomal, //����
	kRelayFaultCheck_Checking, //�����
	kRelayFaultCheck_Fault, //����
} RelayFaultCheckStatus;

/**
  * @enum   RelayFaultCheckType
  * @brief  �̵������ϼ������
  */
typedef enum
{
	kRelayFaultCheckAdhesion, //ճ�����
	kRelayFaultCheckOpenCircuit, //��·���
} RelayFaultCheckType;

//�ص���������
typedef BOOLEAN (*_FAR RelayFaultCheckCondFuncHandler)(void);

/**
  * @struct   RelayFaultCheckParam
  * @brief    ���ϼ�����
  */
typedef struct
{
	RelayFaultCheckType type; //�������
	RelayFaultCheckCondFuncHandler cond_func; //�����������
	INT16U confirm_time; //����ȷ��ʱ��
	INT16U check_time; //���ʱ��
} RelayFaultCheckParam;


/**  
 *  @brief       �����̵������ϼ��
 *  @param[in]	 relay_id    �̵������
 *  @param[in]   check_param ������
 *  @return      ��
 */
void start_relay_fault_check(INT8U relay_id,RelayFaultCheckParam *far check_param);
/**  
 *  @brief       ��ȡ�̵������ϼ��״̬,���ݼ̵�����ŷ�����Ӧ��״̬
 *  @param[in]	 relay_id �̵������
 *  @param[in]	 type �������
 *  @return      kRelayFaultCheck_Nomal ����,kRelayFaultCheck_Checking ����У�kRelayFaultCheck_Fault����
 */
RelayFaultCheckStatus get_relay_fault_status(INT8U relay_id, RelayFaultCheckType type);
/**  
 *  @brief       �̵������ϼ������,�ú��������м̵������м�⣬��Ҫ���������ڵ��øú��������ڽ���С��50ms��
 *  @param[in]	 ��
 *  @return      ��
 */
void relay_fault_check_run(void);

#endif
