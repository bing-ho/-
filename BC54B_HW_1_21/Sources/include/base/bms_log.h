#ifndef _BMS_LOG_H__
#define _BMS_LOG_H__

#include "rs485_hardware.h"
#include "bms_system.h"
#include "bms_error.h"
#include "bms_base_cfg.h"
#include "bms_util.h"

#pragma MESSAGE DISABLE C4002   // Result not used


#ifndef BMS_SUPPORT_LOG
#define BMS_SUPPORT_LOG     1
#endif

#ifndef BMS_LOG_LEVEL
#define BMS_LOG_LEVEL DISABLE_LEVEL // DEBUG_LEVEL
#endif
#ifndef BMS_LOG_TYPE
#define BMS_LOG_TYPE  BMS_LOG_NONE // BMS_LOG_RS485
#endif
/**********************************************
 *
 * Define
 *
 ***********************************************/
typedef enum
{
    kDebug = DEBUG_LEVEL, //!< kDebug
    kInfo = INFO_LEVEL, //!< kInfo
    kWarning = WARNING_LEVEL, //!< kWarning
    kError = ERROR_LEVEL //!< kError
} LogLevel;

#if BMS_LOG_LEVEL <= DEBUG_LEVEL
#define DEBUG debug
#else
#define DEBUG
#endif

#if BMS_LOG_LEVEL <= INFO_LEVEL
#define INFO info
#else
#define INFO
#endif

#if BMS_LOG_LEVEL <= WARNING_LEVEL
#define WARN warn
#else
#define WARN
#endif

#if BMS_LOG_LEVEL <= ERROR_LEVEL
#define ERROR error
#else
#define ERROR
#endif


#define LG_ASSERT(_EXPR) \
if (!(_EXPR)) { \
ERROR("assert", "assert failed, %s, file:%s, line:%s ", #_EXPR, __FILE__, __LINE__); \
}


#define LG_ASSERT_RETURN(_EXPR, _RETURN) \
if (!(_EXPR)) { \
ERROR("assert", "assert failed, %s, file:%s, line:%s ", #_EXPR, __FILE__, __LINE__); \
return _RETURN; \
}

/**********************************************
 *
 * Functions
 *
 ***********************************************/
void log_init(void);
void log_uninit(void);

//void log(int level, const char* module, const char* fmt, ...);
void debug(const char* module, const char* fmt, ...);
void debug_far(const char* module, PCSTR fmt);
void warn(const char* module, const char* fmt, ...);
void error(const char* module, const char* fmt, ...);
void info(const char* module, const char* fmt, ...);
void trace(int level, const char* module, const char* text);
void log_buffer(const char* module, PINT8U buffer, int size);

#endif
