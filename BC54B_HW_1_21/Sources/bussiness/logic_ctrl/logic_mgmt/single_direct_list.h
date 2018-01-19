/**
  * @file       single_direct_list.h
  * @brief      ���������������
  *	@copyright	Ligoo Inc.
  *	@date       2017-03-03
  *	@author     Ligoo����߼���
  * @par
  * @li �����׽ڵ�ΪSDLIST_ITEM���ͣ�
  * @li �����е����ݴ��׽ڵ����һ���ڵ㿪ʼ�洢��
  */
#ifndef	SINGLE_DIRECT_LIST_H
#define	SINGLE_DIRECT_LIST_H
#include "includes.h"

#ifndef _FAR
#define _FAR
#endif

/**
  * @struct SDLIST_ITEM
  *	@brief  ��������ڵ�
 */
typedef struct _SDLIST_ITEM
{
    struct _SDLIST_ITEM *_FAR next;
}SDLIST_ITEM;

/**
  * @brief	            ���ڵ���ӵ�SDLIST_ITEM����ı�ͷ
  *	@param[in]			header			����ͷ��ָ��һ��LIST_ITEM�ڵ㣬�������������ݣ�
  *	@param[in]			item			����ӵ�����ڵ�
  *	@return	            ��
 */
void list_add_to_head(SDLIST_ITEM *_FAR header, SDLIST_ITEM *_FAR item);


/**
  * @brief	            SDLIST_ITEM������ͷ�
  *	@param[in]			header			����ͷ��ָ��һ��LIST_ITEM�ڵ㣬�������������ݣ�
  *	@param[in]			item			���ͷŵ�����ڵ�
  *	@param[in]			free_list		���սڵ�ռ����������
  *	@return	��
 */
void list_release(SDLIST_ITEM *_FAR header, SDLIST_ITEM *_FAR item, SDLIST_ITEM *_FAR free_list);

/**
  * @brief	            ������SDLIST_ITEM����������ڵ�ռ�
  *	@param[in]			free_list		���սڵ�ռ����������
  *	@return	��
 */
SDLIST_ITEM *_FAR list_apply(SDLIST_ITEM *_FAR free_list);

#endif
