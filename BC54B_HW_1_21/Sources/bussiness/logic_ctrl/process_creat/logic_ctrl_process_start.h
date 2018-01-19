/**
  * @file       logic_ctrl_process_start.h
  * @brief      流程启动
  *	@copyright	Ligoo Inc.
  *	@date       2017.03.17
  *	@author     Ligoo软件逻辑组
  */
#ifndef	_LOGIC_CTRL_PROCESS_START_H_
#define	_LOGIC_CTRL_PROCESS_START_H_
#include "includes.h"
#include "process_mgmt.h"

/**
  * @enum   LogicControlNodeID
  * @brief  节点编号
  */
typedef enum
{
    KNodeID_None = 0,
    KNodeID_1,
    KNodeID_2,
    KNodeID_3,
    KNodeID_4,
    KNodeID_5,
    KNodeID_6,
    KNodeID_7,
    KNodeID_8,
    KNodeID_9,
    KNodeID_10,
    KNodeID_11,
    KNodeID_12,
    KNodeID_13,
    KNodeID_14,
    KNodeID_15,
    KNodeID_16,
    KNodeID_17,
    KNodeID_18,
    KNodeID_19,
    KNodeID_20,
    KNodeID_21,
    KNodeID_22,
    KNodeID_23,
    KNodeID_24,
    KNodeID_25,
    KNodeID_26,
    KNodeID_27,
    KNodeID_28,
    KNodeID_29,
    KNodeID_30,
    KNodeID_31,
    KNodeID_32,
    KNodeID_33,
    KNodeID_34,
    KNodeID_35,
    KNodeID_36,
    KNodeID_37,
    KNodeID_38,
    KNodeID_39,
    KNodeID_40,
    KNodeID_41,
    KNodeID_42,
    KNodeID_43,
    KNodeID_44,
    KNodeID_45,
    KNodeID_46,
    KNodeID_47,
    KNodeID_48,
    KNodeID_49,
    KNodeID_50,
    KNodeID_51,
    KNodeID_52,
    KNodeID_53,
    KNodeID_54,
    KNodeID_55,
    KNodeID_56,
    KNodeID_57,
    KNodeID_58,
    KNodeID_59,
    KNodeID_60,
    KNodeID_61,
    KNodeID_62,
    KNodeID_63,
    KNodeID_64,
    KNodeID_65,
    KNodeID_66,
    KNodeID_67,
    KNodeID_68,
    KNodeID_69,
    KNodeID_70,
    KNodeID_71,
    KNodeID_72,
    KNodeID_73,
    KNodeID_74,
    KNodeID_75,
    KNodeID_76,
    KNodeID_77,
    KNodeID_78,
    KNodeID_79, 
    KNodeID_80,   
}LogicControlNodeID;

#define ON    1
#define OFF   0

/**
  * @struct     ProcessAddrNameMap
  * @brief      流程地址与名字映射
  */
typedef struct
{
    const LogicControlNodeContext   *_FAR ptr_ctrl_process; //流程数组首地址        
    const char   *_FAR process_name; //流程名称
}ProcessAddrNameMap;

//流程地址与名字映射
extern const  ProcessAddrNameMap  main_process_addr_name_map[];
extern const  ProcessAddrNameMap  sub_process_addr_name_map[];
/**
  * @brief  获取所有流程长度
  */
INT8U  get_process_addr_name_map_lenth(void);
/**
  * @brief  启动所有流程
  */
void start_all_logic_ctrl_process(void);
INT32U relay_on_outtime(void);
INT32U relay_off_outtime(void);    


#endif