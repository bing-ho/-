#include "bms_log_impl.h"
#include <stdarg.h>

#if BMS_SUPPORT_LOG

#pragma MESSAGE DISABLE C4301
#pragma MESSAGE DISABLE C5703 // Parameter is not used
/**********************************************
 *
 * variable for log
 *
 ***********************************************/
#if BMS_LOG_TYPE != BMS_LOG_NONE
#define BMS_MAX_LOG_LEN 100
char g_log_buffer[BMS_MAX_LOG_LEN] = {0};
rs485_t g_rs485_log = NULL;
lock_t g_log_lock = NULL;
INT8U  g_log_init_count = 0;
INT16U g_log_index = 0;
#endif

#define LOG_FORMAT_TO_BUFFER(FMT, BUFFER) \
    { \
    va_list args; \
    va_start(args, FMT); \
    vsprintf(BUFFER, fmt, args); \
    va_end(args);\
    }

//#define LOG_LOCK() lock_acquire(g_log_lock)
//#define LOG_UNLOCK()  lock_release(g_log_lock)
#define LOG_LOCK()
#define LOG_UNLOCK()


/**********************************************
 *
 * Implementation
 *
 ***********************************************/
#if BMS_LOG_TYPE == BMS_LOG_RS485
void TERMIO_Init(void)
{
   // g_rs485_log = rs485_init(BMS_LOG_RS485_DEV, BMS_LOG_RS485_BAND, NULL, NULL);
    Rs485Param rs485_param = {BMS_LOG_RS485_BAND, BMS_LOG_RS485_DATAFORMAT, BMS_LOG_RS485_PARITY_ENABLE, BMS_LOG_RS485_PARITY_TYPE};
    g_rs485_log = Rs485_Hardware_Init(BMS_LOG_RS485_DEV, &rs485_param, NULL, NULL);  
}

void TERMIO_PutChar(char ch)
{
    rs485_send(g_rs485_log, ch);
}
#endif

void log_init(void)
{
#if BMS_LOG_TYPE != BMS_LOG_NONE
    OS_CPU_SR cpu_sr = 0;
    OS_ENTER_CRITICAL();

    ++g_log_init_count;
    if(g_log_init_count == 1)
    {
        g_log_lock = lock_create();
        TERMIO_Init();
    }
    OS_EXIT_CRITICAL();
#endif
#if BMS_LOG_TYPE == BMS_LOG_RS485
    rs485_send(g_rs485_log, '\0');
    rs485_send(g_rs485_log, '\0');
    rs485_send(g_rs485_log, '\0');
    rs485_send(g_rs485_log, '\0');
#endif
}

void log_uninit(void)
{
#if BMS_LOG_TYPE != BMS_LOG_NONE
    OS_CPU_SR cpu_sr = 0;
    OS_ENTER_CRITICAL();

    --g_log_init_count;
    if(g_log_init_count == 0)
    {
        lock_destroy(g_log_lock);
        g_log_lock = NULL;
    }

    OS_EXIT_CRITICAL();
#endif
}

//void log(int level, const char* module, const char* fmt, ...)
//{
//    LOG_FORMAT_TO_BUFFER(fmt, g_log_buffer);
//    trace(level, module, g_log_buffer);
//}

void debug(const char* module, const char* fmt, ...)
{
#if BMS_LOG_TYPE != BMS_LOG_NONE
    OS_INIT_CRITICAL(); OS_ENTER_CRITICAL();
    LOG_LOCK();

    LOG_FORMAT_TO_BUFFER(fmt, g_log_buffer);
    trace_impl(kDebug, module, g_log_buffer);

    LOG_UNLOCK();
    OS_EXIT_CRITICAL();
#endif
}

void debug_far(const char* module, PCSTR fmt)
{
#if BMS_LOG_TYPE != BMS_LOG_NONE
    INT16U len;
    OS_INIT_CRITICAL(); OS_ENTER_CRITICAL();
    LOG_LOCK();

    if (fmt)
    {
      len = safe_strlen(fmt);
      if (len > BMS_MAX_LOG_LEN - 1) len = BMS_MAX_LOG_LEN - 1;

      safe_memcpy((PINT8U)g_log_buffer, (PINT8U)fmt, len);
      g_log_buffer[len] = 0;
      trace_impl(kDebug, module, g_log_buffer);
    }

    LOG_UNLOCK();
    OS_EXIT_CRITICAL();
#endif
}


void warn(const char* module, const char* fmt, ...)
{
#if BMS_LOG_TYPE != BMS_LOG_NONE
    LOG_LOCK();

    LOG_FORMAT_TO_BUFFER(fmt, g_log_buffer);
    trace_impl(kWarning, module, g_log_buffer);

    LOG_UNLOCK();
#endif
}

void error(const char* module, const char* fmt, ...)
{
#if BMS_LOG_TYPE != BMS_LOG_NONE
    LOG_LOCK();

    LOG_FORMAT_TO_BUFFER(fmt, g_log_buffer);
    trace_impl(kError, module, g_log_buffer);

    LOG_UNLOCK();
#endif
}

void info(const char* module, const char* fmt, ...)
{
#if BMS_LOG_TYPE != BMS_LOG_NONE
    LOG_LOCK();

    LOG_FORMAT_TO_BUFFER(fmt, g_log_buffer);
    trace_impl(kInfo, module, g_log_buffer);

    LOG_UNLOCK();
#endif
}

void trace(int level, const char* module, const char* text)
{
#if BMS_LOG_TYPE != BMS_LOG_NONE
    LOG_LOCK();

    trace_impl(level, module, text);

    LOG_UNLOCK();
#endif
}

void log_buffer(const char* module, PINT8U buffer, int size)
{
#if BMS_LOG_TYPE != BMS_LOG_NONE
    int index;

    if (size * 3 + 1 >= BMS_MAX_LOG_LEN) return;

    LOG_LOCK();

    for (index = 0; index < size; ++index)
    {
        sprintf(g_log_buffer + index * 3, "%02X ", buffer[index]);
    }

    g_log_buffer[size * 3] = '\0';

    trace_impl(kDebug, module, g_log_buffer);

    LOG_UNLOCK();
#endif
}

void trace_impl(int level, const char* module, const char* text)
{
#if BMS_LOG_TYPE == BMS_LOG_RS485
    printf("[i:%d][l:%d][m:%s]%s\r\n", g_log_index++, level, module, text);
    rs485_send(g_rs485_log, '\0');
    rs485_send(g_rs485_log, '\0');
    rs485_send(g_rs485_log, '\0');
    rs485_send(g_rs485_log, '\0');
#endif
}

#else
void log_init(void)
{
}
#endif
