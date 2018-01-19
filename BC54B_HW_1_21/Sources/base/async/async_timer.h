#ifndef __ASYNC_TIMER_H__
#define __ASYNC_TIMER_H__

#ifndef ASYNC_TIMER_TIMEOUT_TYPE
typedef unsigned short ASYNC_TIMER_TIMEOUT_TYPE;
#endif

#ifndef __FAR
#define __FAR
#endif

typedef struct async_timer async_timer_t;

#define ASYNC_TIMER_CANCEL 0
#define ASYNC_TIMER_ERROR ((async_timer_t *__FAR)0)

/// \brief timer_handler timer�Ĵ�����.
/// \param timer: ��Ӧ��Timer����, ������
/// \return 0(ASYNC_TIMER_CANCEL): ȡ����ǰtimer;
/// \return !=0 �´ε��õ���ʱ;
typedef ASYNC_TIMER_TIMEOUT_TYPE (*timer_handler)(async_timer_t *__FAR timer);

/// ע��һ����ʱ��.
/// \return ASYNC_TIMER_ERROR: ע�ᶨʱ��ʧ��;
/// \return ����ֵ, ע�ᵽ�Ķ�ʱ��.
async_timer_t *__FAR async_timer_register(timer_handler func, ASYNC_TIMER_TIMEOUT_TYPE timeout, void *__FAR dat);

/// ���ö�ʱ�����õĺ���.
void async_timer_set_handler(async_timer_t *__FAR timer, timer_handler func);
/// ���ö�ʱ���ĸ�������.
void async_timer_set_data(async_timer_t *__FAR timer, void *__FAR dat);
void *__FAR async_timer_get_data(async_timer_t *__FAR timer);
/// ɾ��һ����ʱ��.
void async_timer_cancel(async_timer_t *__FAR timer);

#endif

