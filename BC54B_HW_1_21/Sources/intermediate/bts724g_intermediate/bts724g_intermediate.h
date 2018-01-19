/*******************************************************************************
**                       安徽力高新能源有限公司 Copyright (c)
**                            http://www.ligoo.cn/ 
** @file	    bts724g_intermediate.h                                       

** @brief       1.完成bts724g模块继电器控制接口函数处理
                2.完成bts724g模块控制任务函数
                
** @copyright		Ligoo Inc.         
** @date		    2017-04-11.
** @author            
*******************************************************************************/ 

#ifndef __BTS724G_INTERMEDIATE_H__
#define __BTS724G_INTERMEDIATE_H__


#include "bms_list.h"
//#include "MC9S12XEP100.h"
#include "bms_base_cfg.h"
#include "bts724g_hardware.h"


#define DIS_CHG_CTL_BIT 1
#define CHG_CTL_BIT     2
#define PRE_CHG_CTL_BIT 4


#define BMS_LOAD_RELAY_ON_TIME  1500 //负载继电器闭合耗时 1ms/bit


#define RELAY_PENDING_CHECK_CYCLE       50 //ms 继电器等待闭合检查周期


#define BMS_RELAY_MAX_COUNT 10

#define BMS_RELAY_ADHESION_VOLT_MIN     70 // v/bit 继电器粘连最小电压阀值,使用绝缘检测时注意最小检测电压(60V)

/************************************************
  * @enum     RelayCtlStatus
  * @brief    继电器工作枚举类型
  ***********************************************/
typedef enum
{
    kRelayOff = 0,
    kRelayOn,
    kRelayForceStart,
    kRelayForceOff = kRelayForceStart,
    kRelayForceOn
}RelayCtlStatus;

/************************************************
  * @enum     RelayTroubleStatus
  * @brief    继电器故障枚举类型
  ***********************************************/
typedef enum
{
    kRelayTroubleNoCheck=0,
    kRelayTroubleChecking,
    kRelayNormal,
    kRelayAdhesionTrouble,
    kRelayOpenCircuitTrouble,
    kRelayOverTemperature,
    kRelayShortCircuit,
	kRelayTroubleMax
}RelayTroubleStatus;      


/************************************************
  * @struct     RelayPendingItem
  * @brief      RelayPendingItem结构体变量类型，继电器工作链表
  ***********************************************/
typedef struct
{
    LIST_ITEM* next;
    byte id;
    byte value;
    INT16U delay;
    INT32U last_tick;
}RelayPendingItem;


/***********************************************************************
  * @brief           等待队列继电器处理函数
  * @param[in]       pdata  空的函数指针 
  * @return          无
***********************************************************************/
void relay_pending_check(void* pdata);

/***********************************************************************
  * @brief           增加待控制的继电器到队列中
  * @param[in]       pdata  空的函数指针 
  * @return          1：Ok  0： Err
***********************************************************************/
Result relay_pending_list_add(INT8U id);

/***********************************************************************
  * @brief           待控制的继电器从队列中删除
  * @param[in]       pdata  空的函数指针 
  * @return          1：Ok  0： Err
***********************************************************************/
Result relay_pending_list_remove(INT8U id);

/***********************************************************************
  * @brief           获取继电器故障采集通道
  * @param[in]       id   继电器编号   
  * @return          采集通道号
***********************************************************************/
INT8U relay_get_724g_st_num(INT8U id);

/***********************************************************************
  * @brief           更新继电器等待控制状态
  * @param[in]       id  继电器编号
  * @param[in]       value  继电器操作状态  
  * @param[in]       delay  延时时长 
  * @param[in]       restart  是否加入队列    
  * @return          无
***********************************************************************/
void relay_pending_para_update(INT8U id, INT8U value, INT16U delay, INT8U restart);

/***********************************************************************
  * @brief           用于对bts724g模块以及工作任务初始化
  * @param[in]       无
  * @return          无
***********************************************************************/
void relay_init(void);

/***********************************************************************
  * @brief           闭合继电器
  * @param[in]       id     继电器编号  
  * @return          RES_OK： 成功  RES_ERROR：失败
***********************************************************************/
Result relay_on(INT8U id);

/***********************************************************************
  * @brief           断开继电器
  * @param[in]       id     继电器编号  
  * @return          RES_OK： 成功  RES_ERROR：失败
***********************************************************************/
Result relay_off(INT8U id);

/***********************************************************************
  * @brief           设置指定名称的继电器输出指定状态
  * @param[in]       name  继电器名称
  * @param[in]       value  继电器编号
  * @return          RES_OK： 成功  RES_ERROR：失败
***********************************************************************/
Result relay_io_set_by_name(char* name, INT8U value);

/***********************************************************************
  * @brief           设置指定id继电器规定时间后输出指定状态
  * @param[in]       id     继电器id
  * @param[in]       value  继电器编号
  * @param[in]       delay  继电器动作等待时间
  * @return          RES_OK： 成功  RES_ERROR：失败
***********************************************************************/
Result relay_set(INT8U id, INT8U value, INT16U delay);

/***********************************************************************
  * @brief           返回继电器数量
  * @param[in]       无 
  * @return          BMS_RELAY_MAX_COUNT
***********************************************************************/
INT8U relay_count(void);

/***********************************************************************
  * @brief           强制闭合继电器
  * @param[in]       id     继电器编号  
  * @return          RES_OK： 成功  RES_ERROR：失败
***********************************************************************/
Result relay_force_on(INT8U id);

/***********************************************************************
  * @brief           强制断开继电器
  * @param[in]       id     继电器编号  
  * @return          RES_OK： 成功  RES_ERROR：失败
***********************************************************************/
Result relay_force_off(INT8U id);

/***********************************************************************
  * @brief           取消强制控制状态
  * @param[in]       id     继电器编号  
  * @return          RES_OK： 成功  RES_ERROR：失败
***********************************************************************/
Result relay_force_cancle(INT8U id);

/***********************************************************************
  * @brief           判断继电器是否闭合
  * @param[in]       id     继电器编号  
  * @return          1： 闭合  0：失败
***********************************************************************/
INT8U relay_is_on(INT8U id);

/***********************************************************************
  * @brief           判断继电器等待控制的状态
  * @param[in]       id     继电器编号  
  * @return          RelayCtlStatus型继电器状态
***********************************************************************/
RelayCtlStatus relay_get_pending_status(INT8U id);

/***********************************************************************
  * @brief           判断继电器是否为等待闭合状态
  * @param[in]       id     继电器编号  
  * @return          1：闭合  0：断开
***********************************************************************/
INT8U relay_is_pending_on(INT8U id);

/***********************************************************************
  * @brief           判断继电器是否为等待断开状态
  * @param[in]       id     继电器编号  
  * @return          1：断开  0：闭合
***********************************************************************/
INT8U relay_is_pending_off(INT8U id);

/***********************************************************************
  * @brief           判断继电器是否为等待强制闭合状态
  * @param[in]       id     继电器编号  
  * @return          1：是  0：不是
***********************************************************************/
INT8U relay_is_pending_force_on(INT8U id);

/***********************************************************************
  * @brief           判断继电器是否为等待强制断开状态
  * @param[in]       id     继电器编号  
  * @return          1：是  0：不是
***********************************************************************/
INT8U relay_is_pending_force_off(INT8U id);

/***********************************************************************
  * @brief           判断继电器是否为工作等待强制状态
  * @param[in]       id     继电器编号  
  * @return          1：是  0：不是
***********************************************************************/
INT8U relay_is_pending_force(INT8U id);

/***********************************************************************
  * @brief           获取继电器强制工作状态
  * @param[in]       id     继电器编号  
  * @return          0：失败 >0: 工作状态
***********************************************************************/
INT8U relay_get_force_status(INT8U id);

/***********************************************************************
  * @brief           判断继电器是否工作在强制状态
  * @param[in]       id     继电器编号  
  * @return          1： 强制  0：普通
***********************************************************************/
INT8U relay_is_force_status(INT8U id);

/***********************************************************************
  * @brief           获取继电器故障
  * @param[in]       id   继电器编号   
  * @return          RelayTroubleStatus故障类型
***********************************************************************/
RelayTroubleStatus relay_get_trouble(INT8U id);

/***********************************************************************
  * @brief           设置继电器故障状态
  * @param[in]       id   继电器编号   
  * @return          RelayTroubleStatus故障类型
***********************************************************************/
void relay_set_trouble(INT8U id, RelayTroubleStatus status);

/***********************************************************************
  * @brief           获取继电器实时故障状态
  * @param[in]       id   继电器编号
  * @param[in]       input_signal_id   输入信号编号  
  * @return          RelayTroubleStatus 型故障类型
***********************************************************************/
RelayTroubleStatus relay_get_trouble_status(INT8U id, INT8U input_signal_id);

/***********************************************************************
  * @brief           获取继电器实时故障状态
  * @param[in]       id   继电器编号
  * @param[in]       input_signal_id   输入信号编号  
  * @return          RelayTroubleStatus 型故障类型
***********************************************************************/
RelayTroubleStatus relay_get_instant_trouble_status(INT8U id, INT8U input_signal_id);

/***********************************************************************
  * @brief           延时闭合继电器
  * @param[in]       id  继电器编号
  * @param[in]       delay  延时时长    
  * @return          RES_OK： 成功  RES_ERROR：失败
***********************************************************************/
Result relay_delay_on(INT8U id, INT16U delay);

/***********************************************************************
  * @brief           延时断开继电器
  * @param[in]       id  继电器编号
  * @param[in]       delay  延时时长    
  * @return          RES_OK： 成功  RES_ERROR：失败
***********************************************************************/
Result relay_delay_off(INT8U id, INT16U delay);

#endif /* __BTS724G_INTERMEDIATE_H__ */
