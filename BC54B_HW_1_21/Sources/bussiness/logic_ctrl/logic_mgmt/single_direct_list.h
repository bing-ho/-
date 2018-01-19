/**
  * @file       single_direct_list.h
  * @brief      单向链表基本函数
  *	@copyright	Ligoo Inc.
  *	@date       2017-03-03
  *	@author     Ligoo软件逻辑组
  * @par
  * @li 链表首节点为SDLIST_ITEM类型；
  * @li 链表中的数据从首节点的下一个节点开始存储。
  */
#ifndef	SINGLE_DIRECT_LIST_H
#define	SINGLE_DIRECT_LIST_H
#include "includes.h"

#ifndef _FAR
#define _FAR
#endif

/**
  * @struct SDLIST_ITEM
  *	@brief  单向链表节点
 */
typedef struct _SDLIST_ITEM
{
    struct _SDLIST_ITEM *_FAR next;
}SDLIST_ITEM;

/**
  * @brief	            将节点添加到SDLIST_ITEM链表的表头
  *	@param[in]			header			链表头（指向一个LIST_ITEM节点，其后才是数据内容）
  *	@param[in]			item			待添加的链表节点
  *	@return	            无
 */
void list_add_to_head(SDLIST_ITEM *_FAR header, SDLIST_ITEM *_FAR item);


/**
  * @brief	            SDLIST_ITEM链表的释放
  *	@param[in]			header			链表头（指向一个LIST_ITEM节点，其后才是数据内容）
  *	@param[in]			item			待释放的链表节点
  *	@param[in]			free_list		回收节点空间的自由链表
  *	@return	无
 */
void list_release(SDLIST_ITEM *_FAR header, SDLIST_ITEM *_FAR item, SDLIST_ITEM *_FAR free_list);

/**
  * @brief	            从自由SDLIST_ITEM链表中申请节点空间
  *	@param[in]			free_list		回收节点空间的自由链表
  *	@return	无
 */
SDLIST_ITEM *_FAR list_apply(SDLIST_ITEM *_FAR free_list);

#endif
