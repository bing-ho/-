/**
  * @file       single_direct_list.c
  * @brief      单向链表基本函数
  *	@copyright	Ligoo Inc.
  *	@date       2017-03-03
  *	@author     Ligoo软件逻辑组
  * @par
  * @li 链表首节点为SDLIST_ITEM类型；
  * @li 链表中的数据从首节点的下一个节点开始存储。
  */
#include "single_direct_list.h"

#ifndef _FAR
#define _FAR
#endif

void list_add_to_head(SDLIST_ITEM *_FAR header, SDLIST_ITEM *_FAR item)
{
    if (header == NULL || item == NULL)
    {
        return;
    }

    item->next = header->next;
    header->next = item;
}

void list_release(SDLIST_ITEM *_FAR header, SDLIST_ITEM *_FAR item, SDLIST_ITEM *_FAR free_list)
{
    if (header == NULL || item == NULL || free_list == NULL)
    {
        return;
    }

    while (header->next != item && header->next != NULL)
    {
        header = header->next;
    }

    if (header->next == item)
    {
        header->next = item->next;

        list_add_to_head(free_list, item);
    }
}

SDLIST_ITEM *_FAR list_apply(SDLIST_ITEM *_FAR free_list)
{
    SDLIST_ITEM *_FAR node;

    if( NULL == free_list || NULL == free_list->next )
    {
        return NULL;
    }

    node = free_list->next;
    free_list->next = node->next;
    node->next = NULL;

    return node;
}
