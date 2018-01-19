#ifndef __LOGGER_H__
#define __LOGGER_H__
#include <stdio.h>

#ifndef LOG_LEVEL
#define LOG_LEVEL   LOG_LEVEL_OFF
#endif

#define LOG_LEVEL_TRACE     0
#define LOG_LEVEL_VERBOSE   1
#define LOG_LEVEL_DEBUG     2
#define LOG_LEVEL_INFO      3
#define LOG_LEVEL_WARN      4
#define LOG_LEVEL_ERROR     5
#define LOG_LEVEL_OFF       6

#define TRACE
#define VERBOSE
#define DEBUG
#define INFO
#define WARN
#define ERROR

/**
 * [LOG description]
 * @param  lvl 日志输出级别 
 * @param  args 日志输出内容
 * @remark 为了兼容早期编译器,采用下面的方式实现
 */
#define __LOG(lvl, args) do{\
        if(lvl >= LOG_LEVEL) {\
            (void)printf("[%c]%s:%d ","TVDIWE"[lvl], __FILE__, __LINE__);\
            (void)printf args;\
            (void)printf("\n");\
        }\
    }while(0)

#define LOG(lvl, args)        __LOG(LOG_LEVEL_##lvl, args)

#endif

