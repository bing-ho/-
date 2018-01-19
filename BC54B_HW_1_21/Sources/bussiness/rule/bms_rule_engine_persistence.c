/**
 *
 * Copyright (c) 2012 Ligoo Inc.
 *
 * @file bms_rule_engine_persistence.c
 * @brief
 * @note
 * @author Xiangyong Zhang
 * @date 2012-8-14
 *
 */
#include "bms_rule_engine_persistence.h"


#pragma MESSAGE DISABLE C1420  //Result of function-call is ignored
#pragma MESSAGE DISABLE C4001  //Condition always FALSE
#pragma MESSAGE DISABLE C12056 // SP debug info incorrect because of optimization or inline assembler

#define RULE_CONFIG_FLASH_ADDRESS   0x7F0000UL

#define RULE_SERIAL_VERSION 1
#define RULE_SERIAL_TRIGGER_VERSION 1
#define RULE_SERIAL_CONDITION_VERSION 1
#define RULE_SERIAL_ACTION_VERSION 1
#define RULE_CONFIG_VERSION 1

#define RULE_CONFIG_MAGIC_NUMBER "@@"

#define BMS_ASSERT_WITH_RETURN(CONDITION, RES) if(!(CONDITION)) return RES;

#define SERIAL_ASSERT(VALUE) res = (VALUE); if (res != RES_OK) return res
#define SERIAL_ASSERT_WITH_EXIT_CRITICAL(VALUE) res = (VALUE); if (res != RES_OK) { OS_EXIT_CRITICAL(); return res; }

#define BLOCK_HEAD_LEN  sizeof(INT16U)

//const char g_demo[] = "AAAAAAAAAAAAAAAAA";
//const char g_demo2[] = "BBBBBBBBBBBBBBBBBB";

static mmap_t g_rule_serial_mmap = NULL;

#pragma DATA_SEG __RPAGE_SEG PAGED_RAM_RULE

#define RULE_SERIAL_BUFFER_SIZE 1000
static INT8U g_rule_serial_buffer[RULE_SERIAL_BUFFER_SIZE];
static FAR_PINT8U g_rule_config_pointer = NULL;
static INT16U g_rule_config_pos = 0;

//INT8U g_tmp_for_complier[256];
/**
 * Serial a rule to a buffer
 *
 *  */
Result rule_serial_to(RuleItem* _PAGED rule, PINT8U buffer, INT16U size, INT16U* write_size)
{
    Result res;
    RuleBuffer rule_buffer;
    rule_buffer.buffer = buffer;
    rule_buffer.pos = 0;
    rule_buffer.size = size;

    if (write_size) *write_size = 0;

    // write the basic information
    SERIAL_ASSERT(rule_buffer_write_int8(&rule_buffer, RULE_SERIAL_VERSION));
    SERIAL_ASSERT(rule_buffer_write_int16(&rule_buffer, rule->id));
    SERIAL_ASSERT(rule_buffer_write_int8(&rule_buffer, rule->status));
    SERIAL_ASSERT(rule_buffer_write_int8(&rule_buffer, rule->type));
    SERIAL_ASSERT(rule_buffer_write_string(&rule_buffer, rule->name));
    //SERIAL_ASSERT(rule_buffer_write_string(&rule_buffer, rule->description));

    // write the next information
    SERIAL_ASSERT(rule_buffer_write_int8(&rule_buffer, rule->tigger.type));

    SERIAL_ASSERT(rule_trigger_serial_to(&rule->tigger, &rule_buffer));

    SERIAL_ASSERT(rule_condition_serial_to(&rule->condition, &rule_buffer));

    SERIAL_ASSERT(rule_action_serial_to(&rule->action, &rule_buffer));

    if (write_size) *write_size = rule_buffer.pos;

    return RES_OK;
}

/**
 * Serial a rule from a freeze buffer
 * Note:
 * the buffer space cannot be freed. This is for the optimization of the memory size
 * */
Result rule_serial_from(RuleItem* _PAGED rule, PINT8U buffer, INT16U size, INT16U* read_size)
{
    Result res;
    RuleBuffer rule_buffer;
    INT8U version;

    rule_buffer.buffer = buffer;
    rule_buffer.pos = 0;
    rule_buffer.size = size;

    safe_memset(rule, 0, sizeof(RuleItem));

    // read the basic information
    SERIAL_ASSERT(rule_buffer_read_int8(&rule_buffer, &version));
    SERIAL_ASSERT(rule_buffer_read_int16(&rule_buffer, &rule->id));
    SERIAL_ASSERT(rule_buffer_read_int8(&rule_buffer, &rule->status));
    SERIAL_ASSERT(rule_buffer_read_int8(&rule_buffer, &rule->type));
    SERIAL_ASSERT(rule_buffer_read_string(&rule_buffer, &rule->name));
    //SERIAL_ASSERT(rule_buffer_read_string(&rule_buffer, &rule->description));

    // read the next information
    SERIAL_ASSERT(rule_buffer_read_int8(&rule_buffer, &rule->tigger.type));

    SERIAL_ASSERT(rule_trigger_serial_from(&rule_buffer, &rule->tigger));

    SERIAL_ASSERT(rule_condition_serial_from(&rule_buffer, &rule->condition));

    SERIAL_ASSERT(rule_action_serial_from(&rule_buffer, &rule->action));

    if (read_size) *read_size = rule_buffer.pos;

    return RES_OK;
}

Result rule_trigger_serial_to(RuleTrigger* _PAGED trigger, RuleBuffer* buffer)
{
    Result res;

    if (trigger == NULL) return RES_ERROR;

    SERIAL_ASSERT(rule_buffer_write_int8(buffer, RULE_SERIAL_TRIGGER_VERSION));
    SERIAL_ASSERT(rule_buffer_write_int8(buffer, trigger->type));

    switch (trigger->type)
    {
    case kRuleTriggerTypeTimer:
        SERIAL_ASSERT(rule_buffer_write_int16(buffer, trigger->data.interval));
        break;
    case kRuleTriggerTypeEvent:
        SERIAL_ASSERT(rule_buffer_write_int16(buffer, trigger->data.event));
        break;
    default:
        break;
    }

    return RES_OK;
}

Result rule_trigger_serial_from(RuleBuffer* buffer, RuleTrigger* _PAGED trigger)
{
    Result res;
    INT8U version;

    if (trigger == NULL) return RES_ERROR;

    SERIAL_ASSERT(rule_buffer_read_int8(buffer, &version));
    SERIAL_ASSERT(rule_buffer_read_int8(buffer, &trigger->type));

    switch (trigger->type)
    {
    case kRuleTriggerTypeTimer:
        SERIAL_ASSERT(rule_buffer_read_int16(buffer, &trigger->data.interval));
        break;
    case kRuleTriggerTypeEvent:
        SERIAL_ASSERT(rule_buffer_read_int16(buffer, &trigger->data.event));
        break;
    default:
        break;
    }

    return RES_OK;
}

Result rule_condition_serial_to(RuleCondition* _PAGED condition, RuleBuffer* buffer)
{
    Result res;

    if (condition == NULL) return RES_ERROR;

    SERIAL_ASSERT(rule_buffer_write_int8(buffer, RULE_SERIAL_CONDITION_VERSION));
    SERIAL_ASSERT(rule_buffer_write_int8(buffer, condition->type));
    SERIAL_ASSERT(rule_buffer_write_string(buffer, condition->expression));

    switch (condition->type)
    {
    case kRuleConditionTypeExpression:
        /** build the expression */
        /** !!! 采用了buffer优化，如果write_block有改动，可能有潜在的问题 */
        if (condition->data.express.vm_buffer == NULL && buffer->pos + BLOCK_HEAD_LEN < buffer->size)
        {
            INT16U out_len = 0;
            res = vm_compile(condition->expression, buffer->buffer + buffer->pos + BLOCK_HEAD_LEN,
                    buffer->size - buffer->pos - sizeof(INT16U), &out_len);
            if (res != RES_OK) return res;
            //log_buffer("rule", buffer->buffer + buffer->pos + BLOCK_HEAD_LEN, out_len);
            SERIAL_ASSERT(rule_buffer_write_block(buffer, buffer->buffer + buffer->pos + BLOCK_HEAD_LEN, out_len));//TODO:block ptr should be null
        }
        else
        {
            SERIAL_ASSERT(
                    rule_buffer_write_block(buffer, condition->data.express.vm_buffer, condition->data.express.vm_length));
        }
        break;
    case kRuleConditionTypeFunction:
        break;
    default:
        break;
    }

    return RES_OK;
}

Result rule_condition_serial_from(RuleBuffer* buffer, RuleCondition* _PAGED condition)
{
    Result res;
    INT8U version;

    if (condition == NULL) return RES_ERROR;

    SERIAL_ASSERT(rule_buffer_read_int8(buffer, &version));
    SERIAL_ASSERT(rule_buffer_read_int8(buffer, &condition->type));
    SERIAL_ASSERT(rule_buffer_read_string(buffer, &condition->expression));

    switch (condition->type)
    {
    case kRuleConditionTypeExpression:
        SERIAL_ASSERT(rule_buffer_read_block(buffer, &condition->data.express.vm_buffer, &condition->data.express.vm_length));
        //log_buffer("serial", action->data.express.vm_buffer, action->data.express.vm_length);
        break;
    case kRuleConditionTypeFunction:
        condition->data.function.func = NULL;
        break;
    default:
        break;
    }

    return RES_OK;
}

Result rule_action_serial_to(RuleAction* _PAGED action, RuleBuffer* buffer)
{
    Result res;

    if (action == NULL) return RES_ERROR;

    SERIAL_ASSERT(rule_buffer_write_int8(buffer, RULE_SERIAL_ACTION_VERSION));
    SERIAL_ASSERT(rule_buffer_write_int8(buffer, action->type));
    SERIAL_ASSERT(rule_buffer_write_string(buffer, action->expression));

    switch (action->type)
    {
    case kRuleActionTypeExpression:
        /** build the expression */
        /** !!! 采用了buffer优化，如果write_block有改动，可能有潜在的问题 */
        if (action->data.express.vm_buffer == NULL && buffer->pos + BLOCK_HEAD_LEN < buffer->size)
        {
            INT16U out_len = 0;
            res = vm_compile(action->expression, buffer->buffer + buffer->pos + BLOCK_HEAD_LEN,
                    buffer->size - buffer->pos - sizeof(INT16U), &out_len);
            if (res != RES_OK) return res;
            //log_buffer("rule", buffer->buffer + buffer->pos + BLOCK_HEAD_LEN, out_len);
            SERIAL_ASSERT(rule_buffer_write_block(buffer, buffer->buffer + buffer->pos + BLOCK_HEAD_LEN, out_len));
        }
        else
        {
            SERIAL_ASSERT(
                    rule_buffer_write_block(buffer, action->data.express.vm_buffer, action->data.express.vm_length));
        }
        break;
    case kRuleActionTypeFunction:
        break;
    default:
        break;
    }

    return RES_OK;
}

Result rule_action_serial_from(RuleBuffer* buffer, RuleAction* _PAGED action)
{
    Result res;
    INT8U version;

    if (action == NULL) return RES_ERROR;

    SERIAL_ASSERT(rule_buffer_read_int8(buffer, &version));
    SERIAL_ASSERT(rule_buffer_read_int8(buffer, &action->type));
    SERIAL_ASSERT(rule_buffer_read_string(buffer, &action->expression));

    switch (action->type)
    {
    case kRuleActionTypeExpression:
        SERIAL_ASSERT(rule_buffer_read_block(buffer, &action->data.express.vm_buffer, &action->data.express.vm_length));
        //log_buffer("serial", action->data.express.vm_buffer, action->data.express.vm_length);
        break;
    case kRuleActionTypeFunction:
        action->data.function.func = NULL;
        break;
    default:
        break;
    }

    return RES_OK;
}

Result rule_buffer_write_int16(RuleBuffer* buffer, INT16U value)
{
    if (buffer->pos + sizeof(INT16U) > buffer->size) return RES_NO_MEM;

    WRITE_LT_INT16U(buffer->buffer, buffer->pos, value);

    return RES_OK;
}

Result rule_buffer_write_int8(RuleBuffer* buffer, INT8U value)
{
    if (buffer->pos + sizeof(INT8U) > buffer->size) return RES_NO_MEM;

    WRITE_LT_INT8U(buffer->buffer, buffer->pos, value);

    return RES_OK;
}

Result rule_buffer_write_int32(RuleBuffer* buffer, INT32U value)
{
    if (buffer->pos + sizeof(INT32U) > buffer->size) return RES_NO_MEM;

    WRITE_LT_INT32U(buffer->buffer, buffer->pos, value);

    return RES_OK;
}

Result rule_buffer_write_string(RuleBuffer* buffer, PCSTR str)
{
    INT16U len = str ? safe_strlen(str) : 0;
    if (buffer->pos + sizeof(INT16U) + len + sizeof(INT8U) > buffer->size) return RES_NO_MEM;

    /** write the length of the string */
    WRITE_LT_INT16U(buffer->buffer, buffer->pos, len);

    /** write the string */
    safe_memcpy(buffer->buffer + buffer->pos, (PVOID)str, len);
    buffer->pos += len;

    /** write the term character */
    buffer->buffer[buffer->pos++] = '\0';

    return RES_OK;
}

Result rule_buffer_write_block(RuleBuffer* buffer, PINT8U block, INT16U size)
{
    INT16U old_pos = buffer->pos;
    if (buffer->pos + BLOCK_HEAD_LEN + size > buffer->size) return RES_NO_MEM;

    WRITE_LT_INT16U(buffer->buffer, buffer->pos, size);

    /** write the data */
    if (block && size > 0) safe_memcpy(buffer->buffer + buffer->pos, block, size);
    buffer->pos += size;

    return RES_OK;
}

Result rule_buffer_read_int16(RuleBuffer* buffer, PINT16U value)
{
    if (buffer->pos + sizeof(INT16U) > buffer->size) return RES_ERROR;

    if (value)
        *value = READ_LT_INT16U(buffer->buffer, buffer->pos);
    else
        buffer->pos += 2;

    return RES_OK;
}

Result rule_buffer_read_int8(RuleBuffer* buffer, PINT8U value)
{
    if (buffer->pos + sizeof(INT8U) > buffer->size) return RES_ERROR;

    if (value)
        *value = READ_LT_INT8U(buffer->buffer, buffer->pos);
    else
        buffer->pos += 2;

    return RES_OK;
}

Result rule_buffer_read_int32(RuleBuffer* buffer, PINT32U value)
{
    if (buffer->pos + sizeof(INT32U) > buffer->size) return RES_ERROR;

    if (value)
        *value = READ_LT_INT32U(buffer->buffer, buffer->pos);
    else
        buffer->pos += 2;

    return RES_OK;
}

Result rule_buffer_read_string(RuleBuffer* buffer, PSTR* _PAGED value)
{
    INT16U len = 0;
    if (buffer->pos + sizeof(INT16U) + sizeof(INT8U) > buffer->size) return RES_ERROR;

    len = READ_LT_INT16U(buffer->buffer, buffer->pos);
    if (buffer->pos + len + sizeof(INT8U) > buffer->size) return RES_ERROR;

    if (buffer->buffer[buffer->pos + len] != '\0') return RES_ERROR;

    if (value) *value = (len == 0 ? NULL : (buffer->buffer + buffer->pos));

    buffer->pos += (len + sizeof(INT8U));

    return RES_OK;
}

Result rule_buffer_read_block(RuleBuffer* buffer, PPINT8U block, PINT16U size)
{
    INT16U len;

    if (buffer->pos + sizeof(INT16U) > buffer->size) return RES_ERROR;

    len = READ_LT_INT16U(buffer->buffer, buffer->pos);
    if (buffer->pos + len > buffer->size) return RES_ERROR;

    if (size) *size = len;
    if (block) *block = ((len == 0) ? NULL : (buffer->buffer + buffer->pos));

    buffer->pos += len;

    return RES_OK;
}

Result rule_config_reader_open(void)
{
    g_rule_config_pointer = mmap_flash_to_logical_address(RULE_CONFIG_FLASH_ADDRESS);
    g_rule_config_pos = 0;

    return RES_OK;
}

Result rule_config_reader_next_type(RuleConfigType* type)
{
    INT16U pos;
    INT8U cfg_type;

    if (g_rule_config_pointer == NULL) return RES_ERROR;

    pos = g_rule_config_pos + strlen(RULE_CONFIG_MAGIC_NUMBER) + sizeof(INT16U) + sizeof(INT8U);
    cfg_type = GET_LT_INT8U(g_rule_config_pointer, pos);

    *type = cfg_type;

    return RES_OK;
}

Result rule_config_reader_read_rule(RuleItem* _PAGED rule)
{
    Result res;
    INT8U type;
    INT16U read_size;
    RuleBuffer buffer;
    buffer.buffer = g_rule_config_pointer + g_rule_config_pos;
    buffer.pos = 0;
    buffer.size = RULE_SERIAL_BUFFER_SIZE;

    if (g_rule_config_pointer == NULL) return RES_ERROR;

    SERIAL_ASSERT(rule_config_reader_read_header(&buffer, &type));

    if (type != kRuleConfigTypeRule) return RES_ERROR;

    SERIAL_ASSERT(rule_serial_from(rule, buffer.buffer + buffer.pos, buffer.size - buffer.pos, &read_size));
    buffer.pos += read_size;

    SERIAL_ASSERT(rule_config_reader_read_tail(&buffer));

    g_rule_config_pos += buffer.pos;

    return RES_OK;
}

Result rule_config_reader_rule_read_status(INT16U* rule_id, INT8U* status)
{
    Result res;
    INT8U type;
    RuleBuffer buffer;
    buffer.buffer = g_rule_config_pointer + g_rule_config_pos;
    buffer.pos = 0;
    buffer.size = RULE_SERIAL_BUFFER_SIZE;

    if (g_rule_config_pointer == NULL) return RES_ERROR;

    SERIAL_ASSERT(rule_config_reader_read_header(&buffer, &type));

    if (type != kRuleConfigTypeRuleStatus) return RES_ERROR;

    SERIAL_ASSERT(rule_buffer_read_int16(&buffer, rule_id));
    SERIAL_ASSERT(rule_buffer_read_int8(&buffer, status));

    SERIAL_ASSERT(rule_config_reader_read_tail(&buffer));

    g_rule_config_pos += buffer.pos;

    return RES_OK;
}

Result rule_config_reader_close(void)
{
    g_rule_config_pointer = NULL;
    g_rule_config_pos = 0;

    return RES_OK;
}

Result rule_config_reader_read_header(RuleBuffer* buffer, INT8U* type)
{
    INT8U index;
    Result res;
    INT8U data, version;
    INT16U size;
    INT16U crc;

    for (index = 0; index < strlen(RULE_CONFIG_MAGIC_NUMBER); ++index)
    {
        SERIAL_ASSERT(rule_buffer_read_int8(buffer, &data));
        if (RULE_CONFIG_MAGIC_NUMBER[index] != data) return RES_ERROR;
    }

    SERIAL_ASSERT(rule_buffer_read_int16(buffer, &size));
    if (size <= sizeof(INT16U)) return RES_ERROR;

    crc = crc_check(buffer->buffer, size - sizeof(INT16U));
    if (crc != GET_LT_INT16U(buffer->buffer,  size - sizeof(INT16U)))
    {
        return ERR_CRC;
    }

    SERIAL_ASSERT(rule_buffer_read_int8(buffer, &version));
    SERIAL_ASSERT(rule_buffer_read_int8(buffer, type));

    return RES_OK;
}

Result rule_config_reader_read_tail(RuleBuffer* buffer)
{
    Result res;
    INT16U crc;
    SERIAL_ASSERT(rule_buffer_read_int16(buffer, &crc));

    return RES_OK;
}

Result rule_config_writer_open(void)
{
    if (rule_config_writer_is_open()) return RES_OK;

    g_rule_serial_mmap = mmap_open(RULE_CONFIG_FLASH_ADDRESS);
    if (g_rule_serial_mmap == NULL) return RES_ERROR;

    return RES_OK;
/*
    INT8U res;
    mmap_t mmap;
    OS_INIT_CRITICAL();
    OS_ENTER_CRITICAL();

    FPROT = 0xCF;
    FSEC = 0xFE;
    FCLKDIV = 0x07;


    //res = PFlash_EraseSector(0x780000UL);
    //res = PFlash_Program(0x780000UL, (UINT16*)g_demo, strlen(g_demo));
    //res = PFlash_Program(0x780008UL, (UINT16*)g_demo2, strlen(g_demo2));
    mmap = mmap_open(0x780000UL);
    mmap_write(mmap, g_demo, strlen(g_demo));
    mmap_write(mmap, g_demo2, strlen(g_demo2));
    mmap_close(mmap);
    OS_EXIT_CRITICAL();

    return RES_OK;
*/
}

BOOLEAN rule_config_writer_is_open(void)
{
    return g_rule_serial_mmap != NULL;
}
//#pragma CODE_SEG DEFAULT
Result rule_config_writer_write_rule_status(INT16U rule_id, INT8U status)
{
    Result res;
    RuleBuffer buffer;
    OS_INIT_CRITICAL();

    if (!rule_config_writer_is_open()) return RES_WRITER_NOT_OPEN;

    OS_ENTER_CRITICAL();

    buffer.buffer = g_rule_serial_buffer;
    buffer.pos = 0;
    buffer.size = RULE_SERIAL_BUFFER_SIZE;

    /* write the header */
    SERIAL_ASSERT_WITH_EXIT_CRITICAL(rule_config_writer_write_header(&buffer, kRuleConfigTypeRuleStatus));

    SERIAL_ASSERT_WITH_EXIT_CRITICAL(rule_buffer_write_int16(&buffer, rule_id));
    SERIAL_ASSERT_WITH_EXIT_CRITICAL(rule_buffer_write_int8(&buffer, status));

    /** write the tailer */
    SERIAL_ASSERT_WITH_EXIT_CRITICAL(rule_config_writer_write_tail(&buffer));

    /** write into the flash */
    res = mmap_write(g_rule_serial_mmap, buffer.buffer, buffer.pos);

    OS_EXIT_CRITICAL();

    return res;
}

Result rule_config_writer_write_rule(RuleItem* _PAGED rule)
{
    Result res;
    INT16U size;
    RuleBuffer buffer;
    OS_INIT_CRITICAL();

    if (!rule_config_writer_is_open()) return RES_WRITER_NOT_OPEN;

    OS_ENTER_CRITICAL();
    buffer.buffer = g_rule_serial_buffer;
    buffer.pos = 0;
    buffer.size = RULE_SERIAL_BUFFER_SIZE;


    /* write the header */
    SERIAL_ASSERT_WITH_EXIT_CRITICAL(rule_config_writer_write_header(&buffer, kRuleConfigTypeRule));

    /** write the rule */
    res = rule_serial_to(rule, buffer.buffer + buffer.pos, buffer.size - buffer.pos, &size);
    if (res != RES_OK) {
      OS_EXIT_CRITICAL();
      return res;
    }
    //log_buffer("rule", buffer.buffer + buffer.pos, size);

    buffer.pos += size;

    /** write the tailer */
    SERIAL_ASSERT_WITH_EXIT_CRITICAL(rule_config_writer_write_tail(&buffer));


    /** write into the flash */
    res = mmap_write(g_rule_serial_mmap, buffer.buffer, buffer.pos);
    OS_EXIT_CRITICAL();

    return res;
}

Result rule_config_writer_write_eof()
{
    Result res;
    RuleBuffer buffer;
    OS_INIT_CRITICAL();

    if (!rule_config_writer_is_open()) return RES_WRITER_NOT_OPEN;

    OS_ENTER_CRITICAL();
    buffer.buffer = g_rule_serial_buffer;
    buffer.pos = 0;
    buffer.size = RULE_SERIAL_BUFFER_SIZE;

    /* write the header */
    SERIAL_ASSERT_WITH_EXIT_CRITICAL(rule_config_writer_write_header(&buffer, kRuleConfigTypeEof));

    /** write the tailer */
    SERIAL_ASSERT_WITH_EXIT_CRITICAL(rule_config_writer_write_tail(&buffer));

    /** write into the flash */
    res = mmap_write(g_rule_serial_mmap, buffer.buffer, buffer.pos);

    OS_EXIT_CRITICAL();

    return res;
}

Result rule_config_writer_close(void)
{
    OS_INIT_CRITICAL();

    if (!rule_config_writer_is_open()) return RES_OK;

    rule_config_writer_write_eof();

    OS_ENTER_CRITICAL();
    mmap_close(g_rule_serial_mmap);
    g_rule_serial_mmap = NULL;
    OS_EXIT_CRITICAL();

    return RES_OK;
}

Result rule_config_writer_write_header(RuleBuffer* buffer, INT8U type)
{
    INT8U index;
    Result res;

    for (index = 0; index < strlen(RULE_CONFIG_MAGIC_NUMBER); ++index)
    {
        SERIAL_ASSERT(rule_buffer_write_int8(buffer, RULE_CONFIG_MAGIC_NUMBER[index]));
    }

    SERIAL_ASSERT(rule_buffer_write_int16(buffer, 0)); // size
    SERIAL_ASSERT(rule_buffer_write_int8(buffer, RULE_CONFIG_VERSION)); // version
    SERIAL_ASSERT(rule_buffer_write_int8(buffer, type)); // type

    return RES_OK;
}


Result rule_config_writer_write_tail(RuleBuffer* buffer)
{
    /** rewrite the length */
    SET_LT_INT16U(buffer->buffer, strlen(RULE_CONFIG_MAGIC_NUMBER), buffer->pos + sizeof(INT16U));

    /** write CRC */
    return rule_buffer_write_int16(buffer, crc_check(buffer->buffer, buffer->pos));
}
