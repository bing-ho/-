/**
 *
 * Copyright (c) 2012 Ligoo Inc.
 *
 * @file bms_rule_engine_persistence.h
 * @brief
 * @note
 * @author Xiangyong Zhang
 * @date 2012-8-14
 *
 */

#ifndef BMS_RULE_ENGINE_PERSISTENCE_H_
#define BMS_RULE_ENGINE_PERSISTENCE_H_
#include "bms_rule_engine_impl.h"
#include "bms_buffer.h"
#include "includes.h"
#include "bms_pflash_mmap.h"
#include "bms_util.h"
#include "bms_vm_engine.h"

/**
 * Serial a rule to a buffer
 *
 *  */
Result rule_serial_to(RuleItem* _PAGED rule, PINT8U buffer, INT16U size, INT16U* write_size);

/**
 * Serial a rule from a freeze buffer
 * Note:
 * the buffer space cannot be freed. This is for the optimization of the memory size
 * */
Result rule_serial_from(RuleItem* _PAGED rule, PINT8U buffer, INT16U size, INT16U* read_size);

typedef struct
{
    PINT8U buffer;
    INT16U size;
    INT16U pos;
}RuleBuffer;


Result rule_trigger_serial_to(RuleTrigger* _PAGED trigger, RuleBuffer* buffer);
Result rule_trigger_serial_from(RuleBuffer* buffer, RuleTrigger* _PAGED trigger);

Result rule_condition_serial_to(RuleCondition* _PAGED condition, RuleBuffer* buffer);
Result rule_condition_serial_from(RuleBuffer* buffer, RuleCondition* _PAGED condition);

Result rule_action_serial_to(RuleAction* _PAGED action, RuleBuffer* buffer);
Result rule_action_serial_from(RuleBuffer* buffer, RuleAction* _PAGED action);

Result rule_buffer_write_int16(RuleBuffer* buffer, INT16U value);
Result rule_buffer_write_int8(RuleBuffer* buffer, INT8U value);
Result rule_buffer_write_int32(RuleBuffer* buffer, INT32U value);
Result rule_buffer_write_string(RuleBuffer* buffer, PCSTR string);
Result rule_buffer_write_block(RuleBuffer* buffer, PINT8U block, INT16U size);

Result rule_buffer_read_int16(RuleBuffer* buffer, PINT16U value);
Result rule_buffer_read_int8(RuleBuffer* buffer, PINT8U value);
Result rule_buffer_read_int32(RuleBuffer* buffer, PINT32U value);
Result rule_buffer_read_string(RuleBuffer* buffer, PSTR* _PAGED value);
Result rule_buffer_read_block(RuleBuffer* buffer, PPINT8U block, PINT16U size);


typedef enum
{
    kRuleConfigTypeKnown,
    kRuleConfigTypeRuleStatus,
    kRuleConfigTypeRule,
    kRuleConfigTypeEof = 0xFF
}RuleConfigType;

Result rule_config_reader_open(void);
Result rule_config_reader_next_type(RuleConfigType* type);
Result rule_config_reader_read_rule(RuleItem* _PAGED rule);
Result rule_config_reader_rule_read_status(INT16U* rule_id, INT8U* status);
Result rule_config_reader_close(void);

Result rule_config_reader_read_header(RuleBuffer* buffer, INT8U* type);
Result rule_config_reader_read_tail(RuleBuffer* buffer);


Result rule_config_writer_open(void);
Result rule_config_writer_write_rule_status(INT16U rule_id, INT8U status);
Result rule_config_writer_write_rule(RuleItem* _PAGED rule);
Result rule_config_writer_close(void);
BOOLEAN rule_config_writer_is_open(void);

Result rule_config_writer_write_header(RuleBuffer* buffer, INT8U type);
Result rule_config_writer_write_tail(RuleBuffer* buffer);


#endif /* BMS_RULE_ENGINE_PERSISTENCE_H_ */
