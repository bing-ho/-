/**
*
* Copyright (c) 2012 Ligoo Inc.
*
* @file bms_rule_stock.h
* @brief
* @note
* @author Xiangyong Zhang
* @date 2012-8-21
*
*/

#ifndef BMS_RULE_STOCK_H_
#define BMS_RULE_STOCK_H_
#include "bms_defs.h"
#include "bms_rule_engine_impl.h"
#include "includes.h"
#include "bms_system.h"
#include "bms_util.h"
#include "bms_log.h"
#include "bms_config.h"

#define RULE_ENGINE_MAX_CUSTOM_VAR_NUM  620

enum {
    kAlarmState_normal,
    kAlarmState_trigger,
    kAlarmState_alarm,
    kAlarmState_release,
};

#define RULE_STOCK_HIGH_ALARM_REL_DELAY_MIN     200
#define RULE_STOCK_GET_VALUE_WITH_MASK(VALUE, MASK) (VALUE & MASK)
#define RULE_STOCK_SET_VALUE_WITH_MASK(DEST, SOURCE, MASK)  ((DEST&(~MASK))|(SOURCE&MASK))

#define RULE_STOCK_HIGHER_LEVEL_PRIORITY_CHECK_ALARM(cond_level, alarm_level, last_cond_level, last_alarm_level) \
    RULE_STOCK_HIGHER_LEVEL_PRIORITY_CHECK_ALARM_EX(cond_level, alarm_level, last_cond_level, last_alarm_level)

#if BMS_FORTH_ALARM_SUPPORT
#define RULE_STOCK_HIGHER_LEVEL_PRIORITY_UPDATE_ALARM_REL() \
    switch(cxt->cur_state) \
    {\
        case kAlarmForthLevel:\
            cxt->cur_state = kAlarmThirdLevel;\
            if(cxt->third_alarm_cond){cxt->last_state = kAlarmThirdLevel;return RES_OK;}break;\
        case kAlarmThirdLevel:\
            cxt->cur_state = kAlarmSecondLevel;\
            if(cxt->second_alarm_cond){cxt->last_state = kAlarmSecondLevel;return RES_OK;}break;\
        case kAlarmSecondLevel:\
            cxt->cur_state = kAlarmFirstLevel;\
            if(cxt->first_alarm_cond){cxt->last_state = kAlarmFirstLevel;return RES_OK;}break;\
        case kAlarmFirstLevel:\
            cxt->cur_state = kAlarmNone;\
            cxt->last_state = kAlarmNone;\
            return RES_OK;\
    }
#else
#define RULE_STOCK_HIGHER_LEVEL_PRIORITY_UPDATE_ALARM_REL() \
    switch(cxt->cur_state) \
    {\
        case kAlarmThirdLevel:\
            cxt->cur_state = kAlarmSecondLevel;\
            if(cxt->second_alarm_cond){cxt->last_state = kAlarmSecondLevel;return RES_OK;}break;\
        case kAlarmSecondLevel:\
            cxt->cur_state = kAlarmFirstLevel;\
            if(cxt->first_alarm_cond){cxt->last_state = kAlarmFirstLevel;return RES_OK;}break;\
        case kAlarmFirstLevel:\
            cxt->cur_state = kAlarmNone;\
            cxt->last_state = kAlarmNone;\
            return RES_OK;\
    }
#endif
    
#define RULE_STOCK_HIGHER_LEVEL_PRIORITY_CHECK_ALARM_EX(cond_level, alarm_level, last_cond_level, last_alarm_level) \
Result rule_stock_higher_level_priority_check_##cond_level##_alarm(HigherLevelPriorityAlarmContext* _PAGED ctx)\
{\
    INT16U delay = RULE_STOCK_HIGH_ALARM_REL_DELAY_MIN;\
    INT32U now_tick = get_tick_count();\
                                       \
    if(ctx == NULL) return RES_ERROR;\
                                     \
    if(ctx->cond_level##_info.bits.trigger_cond)\
    { \
        if(ctx->cond_level##_info.bits.state == kAlarmState_normal){ \
            ctx->cond_level##_info.last_tick = now_tick; \
            ctx->cond_level##_info.bits.state = kAlarmState_trigger; \
        }else if(ctx->cond_level##_info.bits.state == kAlarmState_trigger && ctx->cond_level##_info.trigger_dly <= get_interval_by_tick(ctx->cond_level##_info.last_tick, now_tick)) \
        {\
            ctx->cond_level##_info.bits.state = kAlarmState_alarm; \
         } \
    }else { \
        if(ctx->cond_level##_info.bits.state == kAlarmState_trigger) { \
            ctx->cond_level##_info.bits.state = kAlarmState_normal; \
        } \
    }\
    if(ctx->cond_level##_info.bits.release_cond)\
    { \
        if(ctx->cond_level##_info.bits.state == kAlarmState_alarm){ \
            ctx->cond_level##_info.last_tick = now_tick; \
            ctx->cond_level##_info.bits.state = kAlarmState_release; \
        }else if(ctx->cond_level##_info.bits.state == kAlarmState_release && ctx->cond_level##_info.release_dly <= get_interval_by_tick(ctx->cond_level##_info.last_tick, now_tick)) \
        {\
            ctx->cond_level##_info.bits.state = kAlarmState_normal; \
         }\
    } else {\
        if(ctx->cond_level##_info.bits.state == kAlarmState_release) {\
            ctx->cond_level##_info.bits.state = kAlarmState_alarm; \
         } \
    }\
    return RES_ERROR;\
}

#if defined(__BIG_ENDIAN__)
#define AlarmBaseInfoMask_TriggerCond     0x1UL
#define AlarmBaseInfoMask_ReleaseCond     0x10UL
#else
#define AlarmBaseInfoMask_TriggerCond     0x100UL
#define AlarmBaseInfoMask_ReleaseCond     0x1000UL
#endif
struct AlarmBaseInfo {
    struct {
        INT16U state        : 8;
        INT16U trigger_cond : 1;
        INT16U              : 3;
        INT16U release_cond : 1;
        INT16U              : 3;
    } bits;
    INT16U trigger_dly;
    INT16U release_dly;
    INT32U last_tick;
};

typedef struct _HigherLevelPriorityAlarmContext
{
    INT16U cur_state;
    struct AlarmBaseInfo first_info;
    struct AlarmBaseInfo second_info;
    struct AlarmBaseInfo third_info;
#if BMS_FORTH_ALARM_SUPPORT   
    struct AlarmBaseInfo forth_info;
#endif    
}HigherLevelPriorityAlarmContext, *high_level_pri_alarm_t;

typedef enum _AlarmLevel
{
    kAlarmNone = 0,
    kAlarmFirstLevel,
    kAlarmFirstRelLevel,
    kAlarmSecondLevel,
    kAlarmSecondRelLevel,
    kAlarmThirdLevel,
    kAlarmThirdRelLevel,
#if BMS_FORTH_ALARM_SUPPORT
    kAlarmForthLevel,
    kAlarmForthRelLevel,
#endif
    kAlarmMax
}AlarmLevel;


/** check if a status is keep for the duration */
INT8U rule_stock_check_keep(INT16U id, INT8U status, INT32U keep_time);
Result rule_stock_rule_check_keep(INT16U status, INT32U keep_time);

/** check and try to fix the total cap */
void rule_stock_check_total_cap(void);
void rule_stock_fix_as_full_cap(void);
void rule_stock_fix_as_empty_cap(void);

/** 进行指数充电 */
void rule_stock_charger_set_exp_current(void);

void rule_stock_test_beep(INT16U value);
void rule_stock_dump_cpu_usage(void);

Result rule_stock_set_var(INT16U index, INT16U value);
INT16U rule_stock_get_var(INT16U index);
INT16U rule_stock_get_var_buff_size(void);
void rule_stock_test(INT16U value);
void rule_stock_higher_level_priority_alarm_run(INT16U pos);

#endif /* BMS_RULE_STOCK_H_ */
