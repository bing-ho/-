/**
  * @file       relay_fault_check.h
  * @brief      继电器故障检测
  *	@copyright	Ligoo Inc.
  *	@date       2017.03.17
  *	@author     Ligoo软件逻辑组
  */
#ifndef	_RELAY_FAULT_CHECK_H_
#define	_RELAY_FAULT_CHECK_H_
#include "includes.h"


/**
  * @enum   RelayFaultCheckStatus
  * @brief  继电器故障检测状态
  */
typedef enum
{
	kRelayFaultCheck_Nomal, //正常
	kRelayFaultCheck_Checking, //检测中
	kRelayFaultCheck_Fault, //故障
} RelayFaultCheckStatus;

/**
  * @enum   RelayFaultCheckType
  * @brief  继电器故障检测类型
  */
typedef enum
{
	kRelayFaultCheckAdhesion, //粘连检测
	kRelayFaultCheckOpenCircuit, //开路检测
} RelayFaultCheckType;

//回调函数类型
typedef BOOLEAN (*_FAR RelayFaultCheckCondFuncHandler)(void);

/**
  * @struct   RelayFaultCheckParam
  * @brief    故障检测参数
  */
typedef struct
{
	RelayFaultCheckType type; //检测类型
	RelayFaultCheckCondFuncHandler cond_func; //检测条件函数
	INT16U confirm_time; //故障确认时间
	INT16U check_time; //检测时长
} RelayFaultCheckParam;


/**  
 *  @brief       启动继电器故障检测
 *  @param[in]	 relay_id    继电器编号
 *  @param[in]   check_param 检测参数
 *  @return      无
 */
void start_relay_fault_check(INT8U relay_id,RelayFaultCheckParam *far check_param);
/**  
 *  @brief       获取继电器故障检测状态,根据继电器编号返回相应的状态
 *  @param[in]	 relay_id 继电器编号
 *  @param[in]	 type 检测类型
 *  @return      kRelayFaultCheck_Nomal 正常,kRelayFaultCheck_Checking 检测中，kRelayFaultCheck_Fault故障
 */
RelayFaultCheckStatus get_relay_fault_status(INT8U relay_id, RelayFaultCheckType type);
/**  
 *  @brief       继电器故障检测运行,该函数对所有继电器进行检测，需要无条件周期调用该函数，周期建议小于50ms。
 *  @param[in]	 无
 *  @return      无
 */
void relay_fault_check_run(void);

#endif
