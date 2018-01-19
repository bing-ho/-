#ifndef __ASYNC_EVENT_CALL_H__
#define __ASYNC_EVENT_CALL_H__

#ifndef __FAR
#define __FAR
#endif

typedef struct async_event_call async_event_call_t;

#define ASYNC_EVENT_CALL_CANCEL 0
#define ASYNC_EVENT_CALL_ERROR ((async_event_call_t *__FAR)0)

/// \brief �첽�¼�ִ�еĺ�������.
/// return 0(ASYNC_EVENT_CALL_CANCEL): ȡ����ǰ�¼�;
/// return !=0: �����ȴ��¼�;
typedef char (*event_handler)(void *__FAR dat);

/// �첽ִ�к���������.
/// �첽ִ�к���.
async_event_call_t *__FAR async_event_call_post(event_handler func, void *__FAR dat);
char async_event_call_trigger(async_event_call_t *__FAR);

#endif

