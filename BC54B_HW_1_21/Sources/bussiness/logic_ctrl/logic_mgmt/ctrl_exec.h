#ifndef CTRL_EXEC_H
#define CTRL_EXEC_H

#include "os_cpu.h"
#include "includes.h"
//#define _DEBUG_

#ifdef _DEBUG_
#define dbgprintf printf
#else
#define dbgprintf
#endif

/**
  * @enum   LogicControlObjectType
  * @brief  被控对象类型
  */
typedef enum
{
    LCOT_VOID = 0,			///< 空
    LCOT_RELAY,             ///< 继电器
    LCOT_VOLTAGE,           ///< 电压
    LCOT_CURRENT            ///< 电流
}LogicControlObjectType;


/**
  * @brief  基本控制操作
  *	@param[in]			object_type			受控对象类型
  *	@param[in]			object_id			受控对象ID
  *	@param[in]			ctrl_state          需要达到的状态
  * @return  RES_OK：执行成功
  * @return  其他：未执行成功（错误状态）
 */
INT16U atom_ctrl_operate(LogicControlObjectType object_type, INT8U object_id, INT16U ctrl_state);


#endif
