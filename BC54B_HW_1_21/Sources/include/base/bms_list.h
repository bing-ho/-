#ifndef _BMS_LIST_H__
#define _BMS_LIST_H__
#include "includes.h"
#include "bms_memory.h"

#ifndef BMS_SUPPORT_LIST
#define BMS_SUPPORT_LIST        1
#endif

typedef struct _LIST_ITEM
{
  struct _LIST_ITEM* next;
}LIST_ITEM;

LIST_ITEM* list_init(void);
void list_uninit(LIST_ITEM* header);
void list_add(LIST_ITEM* header, LIST_ITEM* item);
void list_remove(LIST_ITEM* header, LIST_ITEM* item);
LIST_ITEM* list_next(LIST_ITEM* next);
INT8U list_find(LIST_ITEM* header, LIST_ITEM* item);

#endif
