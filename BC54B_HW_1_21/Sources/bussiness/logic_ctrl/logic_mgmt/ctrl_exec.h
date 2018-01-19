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
  * @brief  ���ض�������
  */
typedef enum
{
    LCOT_VOID = 0,			///< ��
    LCOT_RELAY,             ///< �̵���
    LCOT_VOLTAGE,           ///< ��ѹ
    LCOT_CURRENT            ///< ����
}LogicControlObjectType;


/**
  * @brief  �������Ʋ���
  *	@param[in]			object_type			�ܿض�������
  *	@param[in]			object_id			�ܿض���ID
  *	@param[in]			ctrl_state          ��Ҫ�ﵽ��״̬
  * @return  RES_OK��ִ�гɹ�
  * @return  ������δִ�гɹ�������״̬��
 */
INT16U atom_ctrl_operate(LogicControlObjectType object_type, INT8U object_id, INT16U ctrl_state);


#endif
