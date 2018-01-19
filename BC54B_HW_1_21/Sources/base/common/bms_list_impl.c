#include "bms_list.h"


#if BMS_SUPPORT_LIST

LIST_ITEM* list_init(void)
{
    LIST_ITEM* list_item = (LIST_ITEM*) MALLOC(sizeof(LIST_ITEM));
    if (list_item == NULL) return list_item;
    list_item->next = NULL;
    return list_item;
}

void list_uninit(LIST_ITEM* header)
{
    if (header) FREE(header);
}

void list_add(LIST_ITEM* header, LIST_ITEM* item)
{
    if (header == NULL || item == NULL) return;
    while (header->next != NULL)
        header = header->next;
    header->next = item;
}

void list_remove(LIST_ITEM* header, LIST_ITEM* item)
{
    if (header == NULL || item == NULL) return;

    while (header->next != item && header->next != NULL)
        header = header->next;
    if (header->next == item) header->next = item->next;
}

LIST_ITEM* list_next(LIST_ITEM* next)
{
    if (next == NULL) return NULL;
    return next->next;
}

INT8U list_find(LIST_ITEM* header, LIST_ITEM* item)
{
    if (header == NULL || item == NULL) return 0;
    while (header->next != NULL)
    {
        if(header->next == item) return 1;
        header = header->next;
    }
    return 0;
}

#endif

