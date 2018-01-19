/**
  * @file			  ctrl_mgmt.h   
  * @brief			  �߼��������̴������ƹ���ͷ�ļ�
  *	@copyright	      Ligoo Inc.
  *	@date			  2017-03-03
  *	@author
  */
#ifndef	CTRL_MGMT_H
#define	CTRL_MGMT_H

#include "includes.h"
#include "ctrl_exec.h"
#include "single_direct_list.h"

/***************************************************************
���ݽṹ����
***************************************************************/

/// ������ռ������
typedef	void	(*_FAR PreemptFuncHandler)(void *_FAR pdata);

/// �������ƺ���
typedef	INT16U	(*_FAR ObjectStateCtrlFuncHandler)(LogicControlObjectType object_type, INT8U object_id, INT16U state);

#define	MAX_CTRL_ON_OBJ         10
#define	MAX_OBJ_COUNT           11

#define STATE_OPEN              0               ///< �̵����Ͽ�״̬
#define STATE_CLOSE             1               ///< �̵����պ�״̬
#define STATE_INVALID           0xFFFF          ///< ��Ч״̬
#define STATE_RELEASE           0xFFFE          ///< ״̬�ͷ�
#define STATE_FAULT_ADHESION    0x11            ///< �̵���ճ������
#define STATE_FAULT_OPEN        0x22            ///< �̵�����·����

#define PRIO_INVALID            0xFF            ///< ��Ч���ȼ�
#define CONTROL_LOCK            1               ///< ��������
#define CONTROL_UNLOCK          0               ///< �����ͷ�


/**
  * @struct     ObjectStateControl
  * @brief      �Ե������ض����״̬���ƣ���Ӧstruct StateControl
  */
typedef struct
{
    SDLIST_ITEM                     *_FAR next;                      ///< �������ݽṹ

    void                            *_FAR ptr_ctrl_process;          ///< ״̬���Ʒ�����

    PreemptFuncHandler              func_ctrl_preempt;          ///< ������ռ������

    ObjectStateCtrlFuncHandler  	func_state_ctrl;            ///< �������ƺ���

    /**
      * @brief ��ǰ״̬
      *
      * @par ״̬���壺
      * - STATE_INVALID����Ч
      * - object_type=LCOT_RELAY��
      *     -# STATE_OPEN��				�Ͽ�
      *     -# STATE_CLOSE��				�պ�
      *     -# STATE_FAULT_ADHESION��	ճ�����ϼ��
      *     -# STATE_FAULT_OPEN��		��·���ϼ��
      * - object_type=LCOT_VOLTAGE��		��ѹֵ����λ��0.001V��
      * - object_type=LCOT_CURRENT��		����ֵ����λ��0.001A��
	  *	- STATE_RELEASE��״̬�ͷ�
      */
    INT16U                          state_to_switch;

    INT8U                           ctrl_prio;                  ///< ���ȼ�����ֵԽС���ȼ�Խ�ߣ�����ͬ
}ObjectStateControl;

/**
  * @struct     ControlManagement
  * @brief      ���ƹ���
  */
typedef struct
{
    SDLIST_ITEM               *_FAR next;                      ///< �������ݽṹ

    //OS_EVENT                *pSem;                      ///< ��д����

    LogicControlObjectType  object_type;                ///< ���ض������ͣ���Ϣ����ʱ��ʹ��LCOT_VOID
    INT8U                   object_id;                  ///< ���ض�����

    /**
      * @brief ��ǰ״̬
      *
      * @par ״̬���壺
      * - STATE_INVALID����Ч
      * - object_type=LCOT_RELAY��
      *     -# STATE_OPEN��				�Ͽ�
      *     -# STATE_CLOSE��				�պ�
      *     -# STATE_FAULT_ADHESION��	ճ�����ϼ��
      *     -# STATE_FAULT_OPEN��		��·���ϼ��
      * - object_type=LCOT_VOLTAGE��		��ѹֵ����λ��0.001V��
      * - object_type=LCOT_CURRENT��		����ֵ����λ��0.001A��
	  *	- STATE_RELEASE��״̬�ͷ�
      */
    INT16U                  cur_state;

    SDLIST_ITEM             ctrl_list;             ///< ���Բ�ͬ�������̵�״̬����
}ControlManagement;


/***************************************************************
�ӿ�
***************************************************************/
/**
  * @brief				���뵥�����̶Ե����ܿض����״̬����
  *	@param[in]			ptr_ctrl_process		���̵�ָ��
  *	@param[in]			func_ctrl_preempt       ��ռ�ص�����
  *	@param[in]			func_state_ctrl			���ƺ���
  *	@param[in]			ctrl_prio				���Ƶ����ȼ�
  *	@param[in]			object_type				�ܿض��������
  *	@param[in]			object_id				�ܿض����ID
  *	@param[in]			state_to_switch			�ܿض���Ҫ�ﵽ��״̬
  *	@return	��
 */
INT8U apply_state_ctrl(void *_FAR ptr_ctrl_process,
                      PreemptFuncHandler func_ctrl_preempt,
                      ObjectStateCtrlFuncHandler func_state_ctrl,
                      INT8U ctrl_prio,
                      LogicControlObjectType object_type,
                      INT8U object_id,
                      INT16U state_to_switch);

/**
  * @brief				�ͷŵ������̶Ե����ܿض����״̬����
  *	@param[in]			ptr_ctrl_process		���̵�ָ��
  *	@param[in]			object_type				�ܿض��������
  *	@param[in]			object_id				�ܿض����ID
  *	@return	��
 */
void release_state_ctrl(void *_FAR ptr_ctrl_process,
                        LogicControlObjectType object_type,
                        INT8U object_id);

/**
  * @brief				�ͷŵ������̶������ܿض����״̬����
  *	@param[in]			ptr_ctrl_process		���̵�ָ��
  *	@return	��
 */
void release_state_ctrl_all(void *_FAR ptr_ctrl_process);

/**
  * @brief        ���ƹ�������
  *	@return         ��
 */
void basic_logic_control_run(void);

/**
  * @brief	���������ʼ��
  *	@return	��
 */
void ctrl_mgmt_init(void);

#endif
