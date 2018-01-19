/**
*
* Copyright (c) 2012 Ligoo Inc.
*
* @file bms_vm_engine_impl.c
* @brief
* @note
* @author Xiangyong Zhang
* @date 2012-7-30
*
*/

#include "bms_vm_engine_impl.h"

#pragma MESSAGE DISABLE C1420 // Result of function-call is ignoredff
#pragma MESSAGE DISABLE C4001 // Condition always FALSE
#pragma MESSAGE DISABLE C5919 // Conversion of floating to unsigned integral
#pragma MESSAGE DISABLE C12056 // SP debug info incorrect because of optimization or inline assembler

#define VM_MAX_STACK 50
#define VM_MAX_CONST_NUM	100
#define VM_MAX_VARIABLE_NUM	290
#define VM_MAX_FUNCTION_NUM	80

#define VM_MAX_PARAM_NUM 3
#define VM_MAX_VARIABLE_LEN	50

static int g_vm_variable_num = 0;

#ifdef CW5
#pragma DATA_SEG __RPAGE_SEG PAGED_RAM_VM2
#endif

static VmVariableItem g_vm_variables[VM_MAX_VARIABLE_NUM];

#ifdef CW5
#pragma DATA_SEG __RPAGE_SEG PAGED_RAM_VM3
#endif

static VmVariableItem g_vm_variables1[VM_MAX_VARIABLE_NUM];

#ifdef CW5
#pragma DATA_SEG __RPAGE_SEG PAGED_RAM_VM
#endif
//static VmNode g_vm_temp_node;
static VmNode g_vm_node_buffer[VM_MAX_STACK];
static int    g_vm_node_buffer_len = 0;
static int    g_vm_node_parser_index = 0;

static VmToken g_vm_token_stack[VM_MAX_STACK];
static int	   g_vm_token_stack_pos = 0;

static VmTokenItem g_vm_tokens[VM_MAX_STACK];
static int g_vm_token_num = 0;

static INT8U g_vm_pri[kVmTokenMax] = {0};

static VmConstItem g_vm_const_variables[VM_MAX_CONST_NUM];
static int g_vm_const_variable_num = 0;

static VmFunctionItem g_vm_functions[VM_MAX_FUNCTION_NUM];
static int g_vm_function_num = 0;
#ifdef CW5
#pragma DATA_SEG DEFAULT
#endif

static char g_vm_variable_name[VM_MAX_VARIABLE_LEN];
//static VmValue g_vm_params[VM_MAX_PARAM_NUM];


typedef void (*VmSetInt32VariableFunc)(INT32S value);
typedef INT32S (*VmGetInt32VariableFunc)(void);
typedef void (*VmSetInt32VariableExFunc)(INT32S value, void* user_data);
typedef void (*VmGetInt32VariableExFunc)(INT32S* value, void* user_data);
typedef void (*VmSetInt16VariableFunc)(INT16S value);
typedef INT16S (*VmGetInt16VariableFunc)(void);
typedef void (*VmSetInt16VariableExFunc)(INT16S value, void* user_data);
typedef void (*VmGetInt16VariableExFunc)(INT16S* value, void* user_data);

typedef void (*VmFuncVI)(INT32S);
typedef void (*VmFuncVW)(INT16S);
typedef void (*VmFuncVIP)(INT32S, void*);
typedef void (*VmFunVPP)(void*, void*);
typedef void (*VmFuncVV)(void);
typedef INT16U (*VmFuncWV)(void);
typedef INT8U (*VmFuncBV)(void);
typedef INT16U (*VmFuncWW)(INT16U);
typedef INT16U (*VmFuncWB)(INT8U);
typedef double (*VmFuncFF)(double);
typedef INT16U (*VmFuncWWW)(INT16U, INT16U);
typedef INT16U (*VmFuncWWI)(INT16U, INT32U);
typedef INT8U  (*VmFuncBB)(INT8U);
typedef INT8U  (*VmFuncBW)(INT16U);


#define VM_CALC(RESULT, VAULE1, VALUE2, OP)\
    if ((VAULE1)->type == kVmDouble || (VALUE2)->type == kVmDouble)\
{\
    (RESULT)->type = kVmDouble;\
    (RESULT)->value.f_value = ((VAULE1)->type == kVmDouble ? (VAULE1)->value.f_value : (VAULE1)->value.i_value)\
    OP ((VALUE2)->type == kVmDouble ? (VALUE2)->value.f_value : (VALUE2)->value.i_value);\
}\
else\
{\
    (RESULT)->type = kVmInt32;\
    (RESULT)->value.i_value = (VAULE1)->value.i_value OP (VALUE2)->value.i_value;\
}

VmValue* vm_int_to_value_ptr(INT32U value)
{
    static VmValue vm_value;
    vm_value.type = kVmInt32;
    vm_value.value.i_value = value;
    return &vm_value;
}

VmValue vm_int_to_value(INT32U value)
{
    VmValue vm_value;
    vm_value.type = kVmInt32;
    vm_value.value.i_value = value;
    return vm_value;
}

VmValue vm_float_to_value(double value)
{
    VmValue vm_value;
    vm_value.type = kVmDouble;
    vm_value.value.f_value = value;
    return vm_value;
}

double vm_value_to_float(const VmValue* _PAGED value)
{
    switch(value->type)
    {
    case kVmInt32:
    case kVmInt8:
    case kVmInt8U:
    case kVmInt16:
    case kVmInt16U:
    case kVmInt32U:
        return (double)(value->value.i_value);
        break;
    case kVmDouble:
        return value->value.f_value;
        break;
    case kVmFunction:
    case kVmVariable:
        return value->value.id_value;
        break;
    default:
        return 0;
    }
}

INT8U vm_value_to_int8u(const VmValue* _PAGED value)
{
    switch(value->type)
    {
    case kVmInt32:
    case kVmInt8:
    case kVmInt8U:
    case kVmInt16:
    case kVmInt16U:
    case kVmInt32U:
        return (INT8U)value->value.i_value;
        break;
    case kVmDouble:
        return (INT8U)(value->value.f_value);
        break;
    case kVmFunction:
    case kVmVariable:
        return (INT8U)value->value.id_value;
        break;
    default:
        return 0;
    }
}

INT16U vm_value_to_int16u(const VmValue* _PAGED value)
{
    switch(value->type)
    {
    case kVmInt32:
    case kVmInt8:
    case kVmInt8U:
    case kVmInt16:
    case kVmInt16U:
    case kVmInt32U:
        return (INT16U)value->value.i_value;
        break;
    case kVmDouble:
        return (INT16U)(value->value.f_value);
        break;
    case kVmFunction:
    case kVmVariable:
        return value->value.id_value;
        break;
    default:
        return 0;
    }
}

INT32U vm_value_to_int32u(const VmValue* _PAGED value)
{
    switch(value->type)
    {
    case kVmInt32:
    case kVmInt8:
    case kVmInt8U:
    case kVmInt16:
    case kVmInt16U:
    case kVmInt32U:
        return (INT32U)value->value.i_value;
        break;
    case kVmDouble:
        return (INT32U)value->value.f_value;
        break;
    case kVmFunction:
    case kVmVariable:
        return value->value.id_value;
        break;
    default:
        return 0;
    }
}

void vm_init(void)
{
	INT8U pri = 0;
	/* according to C style */
	g_vm_pri[kVmTokenRightBrace] = ++pri;

	g_vm_pri[kVmTokenSemicolon] = ++pri;

	g_vm_pri[kVmTokenRightBracket] = ++pri;

	g_vm_pri[kVmTokenSemicolon] = ++pri;

	g_vm_pri[kVmTokenComma] = ++pri;

	g_vm_pri[kVmTokenAssignment] = ++pri;

	g_vm_pri[kVmTokenOr] = ++pri;

	g_vm_pri[kVmTokenAnd] = ++pri;

	g_vm_pri[kVmTokenBitOr] = ++pri;

	g_vm_pri[kVmTokenBitAnd] = ++pri;

	g_vm_pri[kVmTokenNEQ] = ++pri;
	g_vm_pri[kVmTokenEQ] = pri;             

	g_vm_pri[kVmTokenLT] = ++pri;
	g_vm_pri[kVmTokenLE] = pri;
	g_vm_pri[kVmTokenGT] = pri;
	g_vm_pri[kVmTokenGE] = pri;
	g_vm_pri[kVmTokenEQ] = pri;

	g_vm_pri[kVmTokenAdd] = ++pri;
	g_vm_pri[kVmTokenSub] = pri;

	g_vm_pri[kVmTokenMul] = ++pri;
	g_vm_pri[kVmTokenDiv] = pri;
	g_vm_pri[kVmTokenMod] = pri;

	g_vm_pri[kVmTokenLeftBrace] = ++pri;
	g_vm_pri[kVmTokenLeftBracket] = ++pri;
}


void vm_uninit(void)
{
}

INT8U vm_get_function_param_from_prototype(VmFunctionPrototype prototype)
{
    switch (prototype)
    {
    case kVmPrototypeVI:
    case kVmPrototypeVW:
    case kVmPrototypeWW:
    case kVmPrototypeFF:
    case kVmPrototypeWB:
    case kVmPrototypeBB:
    case kVmPrototypeBW:
        return 1;
        break;
    case kVmPrototypeWWW:
    case kVmPrototypeWWI:
        return 2;
        break;
    case kVmPrototypeVV:
    case kVmPrototypeWV:
    case kVmPrototypeBV:
    default:
        return 0;
    }
}

Result vm_register_function(INT16U id, const char* name, VmFunc func, VmFunctionPrototype prototype)
{
    VmFunctionItem item;
    UNUSED(id);

    LG_ASSERT_RETURN(g_vm_function_num < VM_MAX_FUNCTION_NUM, RES_EXCEED_LIMITED);

    item.name = name;
    item.func = func;
    item.id = g_vm_function_num;
    item.prototype = (INT8U)prototype;

    g_vm_functions[g_vm_function_num++] = item;

    return RES_OK;
}

VmFunctionItem* _PAGED vm_find_function(const char* name)
{
    int index;

    for (index = 0; index < g_vm_function_num; ++index)
    {
        if (safe_strcmp((PCSTR)name, g_vm_functions[index].name) == 0)
        {
            return &(g_vm_functions[index]);
        }
    }
    return NULL;
}

VmFunctionItem* _PAGED vm_find_function_by_id(VmID id)
{
    if (id < 0 || id >= g_vm_function_num) return NULL;

    return &(g_vm_functions[id]);
}

INT16U vm_get_function_num(void)
{
    return g_vm_function_num;
}

Result vm_register_constant(VmID id, const char* name, const VmValue* value)
{
    VmConstItem item;
    UNUSED(id);

    LG_ASSERT_RETURN(g_vm_const_variable_num < VM_MAX_CONST_NUM, RES_EXCEED_LIMITED);

    item.name = name;
    item.value = *value;
    //if (id == 0)
    item.id = g_vm_const_variable_num;
    //else
    //	item.id = id;

    g_vm_const_variables[g_vm_const_variable_num++] = item;

    return RES_OK;
}

VmConstItem* _PAGED vm_find_const(const char* name)
{
    int index;

    for (index = 0; index < g_vm_const_variable_num; ++index)
    {
        if (strcmp(name, g_vm_const_variables[index].name) == 0)
        {
            return &(g_vm_const_variables[index]);
        }
    }
    return NULL;
}

VmConstItem* _PAGED vm_find_const_by_id(VmID id)
{
    if (id < 0 || id >= g_vm_const_variable_num) return NULL;

    return &(g_vm_const_variables[id]);
}

INT16U vm_get_const_num(void)
{
    return g_vm_const_variable_num;
}

Result vm_update_variables_item(int index, VmVariableItem *item)
{
    LG_ASSERT_RETURN(g_vm_variable_num < (VM_MAX_VARIABLE_NUM << 1), RES_EXCEED_LIMITED);

    if(index >= VM_MAX_VARIABLE_NUM) g_vm_variables1[index - VM_MAX_VARIABLE_NUM] = *item;
    else g_vm_variables[index] = *item;
}

Result vm_register_variable(INT16U id, const char* name, VmValueType type, VmFunc set_func, VmFunc get_func)
{
    VmVariableItem item;
    UNUSED(id);

    LG_ASSERT_RETURN(g_vm_variable_num < (VM_MAX_VARIABLE_NUM << 1), RES_EXCEED_LIMITED);

    item.mode = VM_VAR_MODE_FUNC;
    item.name = name;
    item.type = (INT8U)type;
    item.data.func_data.set_func = set_func;
    item.data.func_data.get_func = get_func;
    item.data.func_data.user_data = NULL;

    item.id = g_vm_variable_num;

    //g_vm_variables[g_vm_variable_num++] = item;
    vm_update_variables_item(g_vm_variable_num++, &item);

    return RES_OK;
}

Result vm_register_variable_ex(INT16U id, const char* name, VmValueType type,
        VmFunc set_func, VmFunc get_func, void* user_data)
{
    VmVariableItem item;
    UNUSED(id);

    LG_ASSERT_RETURN(g_vm_variable_num < (VM_MAX_VARIABLE_NUM << 1), RES_EXCEED_LIMITED);

    item.id = g_vm_variable_num;
    item.mode = VM_VAR_MODE_EXTEND | VM_VAR_MODE_FUNC;
    item.name = name;
    item.type = (INT8U)type;
    item.data.func_data.set_func = set_func;
    item.data.func_data.get_func = get_func;
    item.data.func_data.user_data = user_data;

    //g_vm_variables[g_vm_variable_num++] = item;
    vm_update_variables_item(g_vm_variable_num++, &item);

    return RES_OK;
}

Result vm_register_direct_variable(INT16U id, const char* name, VmValueType type, void* variable)
{
    VmVariableItem item;
    UNUSED(id);

    if (!name || !variable) return RES_INVALID_PARAM;
    LG_ASSERT_RETURN(g_vm_variable_num < (VM_MAX_VARIABLE_NUM << 1), RES_EXCEED_LIMITED);

    item.mode = VM_VAR_MODE_ADDRESS;
    item.name = name;
    item.type = (INT8U)type;
    item.data.address_data.address = variable;
    item.id = g_vm_variable_num;

    //g_vm_variables[g_vm_variable_num++] = item;
    vm_update_variables_item(g_vm_variable_num++, &item);

    return RES_OK;
}

VmVariableItem* _PAGED vm_find_variable(const char* name)
{
    int index;

    for (index = 0; index < g_vm_variable_num; ++index)
    {
        if (index < VM_MAX_VARIABLE_NUM)
        {
            if (strcmp(name, g_vm_variables[index].name) == 0)
            {
                return (VmVariableItem* _PAGED)&g_vm_variables[index];
            }
        }
        else
        {
            if (strcmp(name, g_vm_variables1[index - VM_MAX_VARIABLE_NUM].name) == 0)
            {
                return (VmVariableItem* _PAGED)&g_vm_variables1[index - VM_MAX_VARIABLE_NUM];
            }
        }
    }

    return NULL;
}

VmVariableItem* _PAGED vm_find_variable_by_id(VmID id)
{
    if (id < 0 || id >= g_vm_variable_num) return NULL;

    if (id < VM_MAX_VARIABLE_NUM) 
        return &(g_vm_variables[id]);
    else 
        return &(g_vm_variables1[id - VM_MAX_VARIABLE_NUM]);
}

INT16U vm_get_variable_num(void)
{
    return g_vm_variable_num;
}

//static tmp_expression[256];
Result vm_compile(PCSTR expression, PINT8U buffer, INT16U size, INT16U* output_size)
{
    VmExpr expr;
    Result res;

    if (!expression) return RES_INVALID_PARAM;

    //strcpy(tmp_expression, "post_event(7)");
    memset(&expr, 0, sizeof(expr));
    expr.expression = expression;
    expr.expression_len = safe_strlen(expression);

    res = vm_parser_init(&expr);
    if (res != RES_OK) return res;

    res = vm_parser_parse_tokens(&expr);
    if (res != RES_OK) return res;

    res = vm_parser_parse_gramma(&expr);
    if (res != RES_OK) return res;

    if (buffer && size < g_vm_node_buffer_len * sizeof(VmNode)) return RES_NO_MEM;
    if (buffer) safe_memcpy(buffer, (PINT8U)g_vm_node_buffer, g_vm_node_buffer_len * sizeof(VmNode));
    if (output_size) *output_size = g_vm_node_buffer_len * sizeof(VmNode);

    return RES_OK;
}

Result vm_run(PINT8U buffer, INT16U size, VmValue* result)
{
    if (result == NULL) return RES_INVALID_PARAM;

    return vm_executor_exec(buffer, size, result);
}

Result vm_parser_output_buffer_push(VmNode* _PAGED node)
{
    INT8U param_count;
    if (g_vm_node_buffer_len >= VM_MAX_STACK) return RES_EXCEED_LIMITED;

    switch(node->type)
    {
    case kVmNodeOperator:
        if (node->data.token == kVmTokenComma) return RES_OK;
        if (node->data.token != kVmTokenSemicolon && g_vm_node_buffer_len - g_vm_node_parser_index < 2)
            return RES_INVALID_FORMAT;
        g_vm_node_parser_index += 2;
        break;
    case kVmNodeFunction:
        param_count = vm_get_function_param_from_prototype(vm_find_function_by_id(node->data.operand.value.id_value)->prototype);
        if ((g_vm_node_buffer_len - g_vm_node_parser_index) < param_count) return RES_ERROR_FUNC_PARAM;
        g_vm_node_parser_index += param_count;
        break;
    case kVmNodeOperand:
        break;
    }

    g_vm_node_buffer[g_vm_node_buffer_len++] = *node;

    return RES_OK;
}

Result vm_parser_token_stack_push(VmToken token)
{
    g_vm_token_stack[g_vm_token_stack_pos++] = token;
    return RES_OK;
}

Result vm_parser_token_stack_pop(VmToken* token)
{
    if (g_vm_token_stack_pos == 0) return RES_NO_DATA;

    if (token) *token = g_vm_token_stack[--g_vm_token_stack_pos];

    return RES_OK;
}

Result vm_parser_token_stack_lookup(VmToken* token)
{
    if (g_vm_token_stack_pos == 0) return RES_NO_DATA;

    if (token) *token = g_vm_token_stack[g_vm_token_stack_pos - 1];

    return RES_OK;
}

Result vm_parser_token_is_value(VmToken token)
{
    return (token == kVmTokenFunction ||
            token == kVmTokenVariable ||
            token == kVmTokenValue) ? RES_TRUE : RES_FALSE;
}

Result vm_parser_lookup_token(VmToken* token)
{
    if (g_vm_token_num == 0) return RES_NO_DATA;
    *token = g_vm_tokens[g_vm_token_num - 1].token;
    return RES_OK;
}

Result vm_parser_add_token(VmTokenItem* token)
{
    if (token == NULL) return RES_ERROR;


    if (g_vm_token_num > 0
            && vm_parser_token_is_value(token->token) == RES_TRUE
            && vm_parser_token_is_value(g_vm_tokens[g_vm_token_num - 1].token) == RES_TRUE)
    {
        return RES_INVALID_FORMAT;
    }

    g_vm_tokens[g_vm_token_num++] = *token;

    return RES_OK;
}


Result vm_parser_init(VmExpr* _PAGED expr)
{
    UNUSED(expr);
    g_vm_node_buffer_len = 0;
    g_vm_token_num = 0;
    g_vm_token_stack_pos = 0;

    return RES_OK;
}

Result vm_parser_add_operator_token(VmExpr* _PAGED expr, int token_len, VmToken token)
{
    VmTokenItem item;
    item.start = expr->expression_pos;
    item.end = expr->expression_pos + token_len;
    item.token = token;

    expr->expression_pos += token_len;

    return vm_parser_add_token(&item);
}

Result vm_parser_parse_tokens_alpha(VmExpr* _PAGED expr)
{
    char chr;
    VmTokenItem item;
    int index = 0;

    item.start = expr->expression_pos;

    g_vm_variable_name[0] = '\0';
    while(expr->expression_pos < expr->expression_len && index < VM_MAX_VARIABLE_LEN)
    {
        chr = expr->expression[expr->expression_pos];
        if (isalpha(chr) || isdigit(chr) || chr == '_')
            g_vm_variable_name[index++] = chr;
        else
            break;
        ++expr->expression_pos;
    }

    item.end = expr->expression_pos - 1;

    g_vm_variable_name[index] = '\0';

    if (expr->expression_pos < expr->expression_len && expr->expression[expr->expression_pos] == '(')
    {
        VmFunctionItem* _PAGED found;

        item.token = kVmTokenFunction;

        found = vm_find_function(g_vm_variable_name);
        if (found == NULL) return RES_NOT_FOUND_FUNC;

        item.value.value.id_value = found->id;
        item.value.type = kVmFunction;
    }
    else
    {
        VmConstItem* _PAGED found;

        item.token = kVmTokenValue;

        found = vm_find_const(g_vm_variable_name);
        if (found == NULL) return RES_NOT_FOUND_CONST;

        item.value.value.id_value = found->id;
        item.value.type = kVmConstant;
    }

    return vm_parser_add_token(&item);
}

Result vm_parser_parse_tokens_number(VmExpr* _PAGED expr)
{
    Result res = RES_OK;
    INT32U i_value = 0;
    double f_value = 0;
    INT8U float_pos = 0;
    INT8U is_negative = 0;
    char chr;
    VmTokenItem item;

    item.start = expr->expression_pos;

    if (expr->expression[expr->expression_pos] == '-')
    {
        is_negative = 1;
        ++expr->expression_pos;
    }

    while(expr->expression_pos < expr->expression_len)
    {
        chr = expr->expression[expr->expression_pos];
        if (chr == '.')
        {
            if (float_pos > 0)
            {
                res = RES_INVALID_FORMAT;
                break;
            }

            float_pos = 1;
        }
        else if(isdigit(chr))
        {
            if (float_pos)
            {
                f_value += (double)(chr - '0') / pow(10, float_pos);
                ++float_pos;
            }
            else
            {
                i_value = i_value * 10 + (expr->expression[expr->expression_pos] - '0');
            }
        }
        else
        {
            break;
        }

        item.end = expr->expression_pos;
        ++(expr->expression_pos);
    }

    /* push the value into the operand stack */
    item.token = kVmTokenValue;
    if (float_pos == 0)
    {
        //if (i_value < 0xFFFF)
        //{
        //	item.value.type = kVmInt16;
        //	item.value.value.i_value = i_value;
        //}
        //else
        //{
        item.value.type = kVmInt32;
        item.value.value.i_value = i_value;
        if (is_negative) item.value.value.i_value = -item.value.value.i_value;
        //}
    }
    else
    {
        item.value.type = kVmDouble;
        item.value.value.f_value = (double)i_value + f_value;
        if (is_negative) item.value.value.f_value = -item.value.value.f_value;
    }

    return vm_parser_add_token(&item);
}

Result vm_parser_parse_tokens_variable(VmExpr* _PAGED expr)
{
    char chr;
    VmTokenItem item;
    int index = 0;
    VmVariableItem* _PAGED function;
    INT16U num = 0;

    item.start = expr->expression_pos;

    if (expr->expression[expr->expression_pos] != '$') return RES_INVALID_FORMAT;
    ++expr->expression_pos;

    g_vm_variable_name[0] = '\0';
    while(expr->expression_pos < expr->expression_len && index < VM_MAX_VARIABLE_LEN)
    {
        chr = expr->expression[expr->expression_pos];
        if (isalpha(chr) || isdigit(chr) || chr == '_')
            g_vm_variable_name[index++] = chr;
        else
            break;
        ++expr->expression_pos;
    }
    g_vm_variable_name[index] = '\0';
    item.end = expr->expression_pos - 1;

    item.token = kVmTokenValue;
    item.value.type = kVmVariable;
    if (vm_is_number(g_vm_variable_name, &num) == RES_OK)
    {
        vm_find_variable_by_id(num);
    }
    else
    {
        function = vm_find_variable(g_vm_variable_name);
    }

    if (function == NULL) return RES_NOT_FOUND_VAR;
    item.value.value.id_value = function->id;

    return vm_parser_add_token(&item);
}

Result vm_parser_parse_tokens(VmExpr* _PAGED expr)
{
    Result res = RES_OK;
    char chr;

    while (expr->expression_pos < expr->expression_len)
    {
        chr = expr->expression[expr->expression_pos];
        if (chr == ' ' || chr == '\r' || chr == '\n' || chr == '\t')
        {
            // ignore the white space character
            expr->expression_pos++;
        }
        else if (chr == '.' || isdigit(chr))
        {
            res = vm_parser_parse_tokens_number(expr);
        }
        else if (isalpha(chr))
        {
            res = vm_parser_parse_tokens_alpha(expr);
        }
        else if (chr == '$')
        {
            res = vm_parser_parse_tokens_variable(expr);
        }
        else if (chr == '+')
        {
            res = vm_parser_add_operator_token(expr, 1, kVmTokenAdd);
        }
        else if (chr == '-')
        {
            VmToken last_token;
			if (vm_parser_lookup_token(&last_token) == RES_OK &&
				(last_token == kVmTokenVariableGet
				 || last_token == kVmTokenFunction
				 || last_token == kVmTokenValue
				 || last_token == kVmTokenVariable
				 || last_token == kVmTokenRightBracket
				 || last_token == kVmTokenRightBrace
				))
			{
				res = vm_parser_add_operator_token(expr, 1, kVmTokenSub);
			}
			else
			{
				if (expr->expression_pos + 1 < expr->expression_len
					&& (expr->expression[expr->expression_pos + 1] == '.'
					|| isdigit(expr->expression[expr->expression_pos + 1])))
				{
					res = vm_parser_parse_tokens_number(expr);
				}
				else
				{
					res = RES_INVALID_FORMAT;
				}
			}
			/*
            if(vm_parser_lookup_token(&last_token) != RES_OK
                || (last_token >= kVmTokenAgiStart && last_token <= kVmTokenAgiEnd)
				|| (last_token >= kVmTokenCompareStart && last_token <= kVmTokenCompareEnd)
                || last_token == kVmTokenLeftBracket || last_token == kVmTokenLeftBrace
				|| last_token == kVmTokenComma || last_token == kVmTokenSemicolon
				)
            {
                if (expr->expression_pos + 1 < expr->expression_len
                    && (expr->expression[expr->expression_pos + 1] == '.'
                    || isdigit(expr->expression[expr->expression_pos + 1])))
                {
                    res = vm_parser_parse_tokens_number(expr);
                }
                else
                {
                    res = RES_INVALID_FORMAT;
                }
            }
            else
            {
                res = vm_parser_add_operator_token(expr, 1, kVmTokenSub);
            }
			*/
        }
        else if (chr == '*')
        {
            res = vm_parser_add_operator_token(expr, 1, kVmTokenMul);
        }
        else if (chr == '/')
        {
            res = vm_parser_add_operator_token(expr, 1, kVmTokenDiv);
        }
        else if (chr == '%')
        {
            res = vm_parser_add_operator_token(expr, 1, kVmTokenMod);
        }
        else if (chr == '(')
        {
            res = vm_parser_add_operator_token(expr, 1, kVmTokenLeftBracket);
        }
        else if (chr == ')')
        {
            res = vm_parser_add_operator_token(expr, 1, kVmTokenRightBracket);
        }
        else if (chr == '>')
        {
            if (expr->expression_pos + 1 < expr->expression_len
                && expr->expression[expr->expression_pos + 1] == '=')
            {
                res = vm_parser_add_operator_token(expr, 2, kVmTokenGE);
            }
            else
            {
                res = vm_parser_add_operator_token(expr, 1, kVmTokenGT);
            }
        }
        else if (chr == '<')
        {
            if (expr->expression_pos + 1 < expr->expression_len
                && expr->expression[expr->expression_pos + 1] == '=')
            {
                res = vm_parser_add_operator_token(expr, 2, kVmTokenLE);
            }
            else
            {
                res = vm_parser_add_operator_token(expr, 1, kVmTokenLT);
            }
        }
        else if (chr == '=')
        {
            if (expr->expression_pos + 1 < expr->expression_len
                && expr->expression[expr->expression_pos + 1] == '=')
            {
                res = vm_parser_add_operator_token(expr, 2, kVmTokenEQ);
            }
            else
            {
                res = vm_parser_add_operator_token(expr, 1, kVmTokenAssignment);
            }
        }
        else if (chr == '!')
        {
            if (expr->expression_pos + 1 < expr->expression_len
                && expr->expression[expr->expression_pos + 1] == '=')
            {
                res = vm_parser_add_operator_token(expr, 2, kVmTokenNEQ);
            }
        }
        else if (chr == '&')
        {
            if (expr->expression_pos + 1 < expr->expression_len
                && expr->expression[expr->expression_pos + 1] == '&')
            {
                res = vm_parser_add_operator_token(expr, 2, kVmTokenAnd);
            }
            else
            {
                res = vm_parser_add_operator_token(expr, 1, kVmTokenBitAnd);
            }
        }
        else if (chr == '|')
        {
            if (expr->expression_pos + 1 < expr->expression_len
                && expr->expression[expr->expression_pos + 1] == '|')
            {
                res = vm_parser_add_operator_token(expr, 2, kVmTokenOr);
            }
            else
            {
                res = vm_parser_add_operator_token(expr, 1, kVmTokenBitOr);
            }
        }
        else if (chr == ';')
        {
            res = vm_parser_add_operator_token(expr, 1, kVmTokenSemicolon);
        }
        else if (chr == ',')
        {
            res = vm_parser_add_operator_token(expr, 1, kVmTokenComma);
        }
        else if (chr == '{')
        {
            res = vm_parser_add_operator_token(expr, 1, kVmTokenLeftBrace);
        }
        else if (chr == '}')
        {
            res = vm_parser_add_operator_token(expr, 1, kVmTokenRightBrace);
        }
        else
        {
            ++(expr->expression_pos);
        }

        if (res != RES_OK) break;
    }

    return res;
}

Result vm_parser_output_buffer_add_operator(VmToken token)
{
    VmNode node;
    node.type = kVmNodeOperator;
    node.data.token = token;
    return vm_parser_output_buffer_push(&node);
}

Result vm_parser_parse_gramma(VmExpr* _PAGED expr)
{
    int index;
    VmToken current_token;
    Result res = RES_OK;
    VmToken top_token = 0;
    g_vm_node_parser_index = 0;
    UNUSED(expr);

    /** output to buffer */
    for (index = g_vm_token_num - 1; index >= 0; --index)
    {
        current_token = g_vm_tokens[index].token;
        switch(current_token)
        {
        case kVmTokenValue:
            {
                VmNode node;
                node.type = kVmNodeOperand;
                node.data.operand = g_vm_tokens[index].value;
                res = vm_parser_output_buffer_push(&node);
            }
            break;
        case kVmTokenFunction:
            {
                VmNode node;
                node.type = kVmNodeFunction;
                node.data.operand = g_vm_tokens[index].value;
                res = vm_parser_output_buffer_push(&node);
            }
            break;
        case kVmTokenRightBracket:
            res = vm_parser_token_stack_push(current_token);
            break;
        case kVmTokenLeftBracket:
            {
                while(vm_parser_token_stack_pop(&top_token) == RES_OK && top_token != kVmTokenRightBracket )
                {
                    res = vm_parser_output_buffer_add_operator(top_token);
                }
            }
            break;
        case kVmTokenRightBrace:
            res = vm_parser_token_stack_push(current_token);
            break;
        case kVmTokenLeftBrace:
            {
                while(vm_parser_token_stack_pop(&top_token) == RES_OK && top_token != kVmTokenRightBrace)
                {
                    res = vm_parser_output_buffer_add_operator(top_token);
                    if (res != RES_OK) break;
                }
            }
            break;
        default:
            {
                while(vm_parser_token_stack_lookup(&top_token) == RES_OK
                    && g_vm_pri[current_token] < g_vm_pri[top_token])
                {
                    vm_parser_token_stack_pop(&top_token);
                    res = vm_parser_output_buffer_add_operator(top_token);
                    if (res != RES_OK) break;
                }
                res = vm_parser_token_stack_push(current_token);
            }
            break;

        }
    }

    /** pop all token */
    {
        VmToken top_token;
        while(res == RES_OK && vm_parser_token_stack_pop(&top_token) == RES_OK)
        {
            res = vm_parser_output_buffer_add_operator(top_token);
        }
    }

    if (g_vm_node_buffer_len - g_vm_node_parser_index > 1) return RES_INVALID_FORMAT;

    /** reserve the output buffer */
    for (index = 0; index < g_vm_node_buffer_len / 2; ++index)
    {
        VmNode tmp;
        tmp = g_vm_node_buffer[index];
        g_vm_node_buffer[index] = g_vm_node_buffer[g_vm_node_buffer_len - 1 - index];
        g_vm_node_buffer[g_vm_node_buffer_len - 1 - index] = tmp;
    }

    return res;
}

Result vm_executor_exec_add_node(VmExecutorContext* context, VmValue* value)
{
    Result res;
    VmValue value1, value2;

    res = vm_executor_exec_node(context, &value1);
    if (res != RES_OK) return res;

    res = vm_executor_exec_node(context, &value2);
    if (res != RES_OK) return res;

    if (value == NULL) return res;

    VM_CALC(value, &value1, &value2, +);

    return RES_OK;
}


Result vm_executor_exec_and_node(VmExecutorContext* context, VmToken token, VmValue* value)
{
    Result res = RES_OK;
    VmValue value1, value2;
    UNUSED(token);

    res = vm_executor_exec_node(context, &value1);
    if (res != RES_OK) return res;

    if (context->flag & VM_EXECUTOR_FLAG_IGNORE_EXECUTING)
    {
        res = vm_executor_exec_node(context, &value2);
        return res;
    }

    if (vm_value_to_int32u(&value1) == 0)
    {
        context->flag |= VM_EXECUTOR_FLAG_IGNORE_EXECUTING;

        res = vm_executor_exec_node(context, NULL);
        value2.type = kVmInt32;
        value2.value.i_value = 0;

        context->flag &= (~VM_EXECUTOR_FLAG_IGNORE_EXECUTING);
    }
    else
    {
        res = vm_executor_exec_node(context, &value2);
    }

    VM_CALC(value, &value1, &value2, &&);

    return res;
}

Result vm_executor_exec_or_node(VmExecutorContext* context, VmToken token, VmValue* value)
{
    Result res = RES_OK;
    VmValue value1, value2;
    UNUSED(token);

    res = vm_executor_exec_node(context, &value1);
    if (res != RES_OK) return res;

    if (context->flag & VM_EXECUTOR_FLAG_IGNORE_EXECUTING)
    {
        res = vm_executor_exec_node(context, &value2);
        return res;
    }

    if ((value1.type == kVmInt32 && value1.value.i_value != 0)
        || (value1.type == kVmDouble && value1.value.f_value != 0))
    {
        context->flag |= VM_EXECUTOR_FLAG_IGNORE_EXECUTING;

        res = vm_executor_exec_node(context, &value2);
        value2.type = kVmInt32;
        value2.value.i_value = 0;

        context->flag &= (~VM_EXECUTOR_FLAG_IGNORE_EXECUTING);
    }
    else
    {
        res = vm_executor_exec_node(context, &value2);
    }

    VM_CALC(value, &value1, &value2, ||);

    return res;
}

Result vm_executor_exec_arithmetic_node(VmExecutorContext* context, VmToken token, VmValue* value)
{
    Result res = RES_OK;
    VmValue value1, value2;

    res = vm_executor_exec_node(context, &value1);
    if (res != RES_OK) return res;

    res = vm_executor_exec_node(context, &value2);
    if (res != RES_OK) return res;

    if (context->flag & VM_EXECUTOR_FLAG_IGNORE_EXECUTING) return RES_OK;

    switch(token)
    {
    case kVmTokenAdd:
        VM_CALC(value, &value1, &value2, +);
        break;
    case kVmTokenSub:
        VM_CALC(value, &value1, &value2, -);
        break;
    case kVmTokenMul:
        VM_CALC(value, &value1, &value2, *);
        break;
    case kVmTokenDiv:
        VM_CALC(value, &value1, &value2, /);
        break;
    case kVmTokenMod:
        value->type = kVmInt32;
        value->value.i_value = value1.value.i_value % value2.value.i_value;
        break;
    case kVmTokenLE:
        VM_CALC(value, &value1, &value2, <=);
        break;
    case kVmTokenLT:
        VM_CALC(value, &value1, &value2, <);
        break;
    case kVmTokenGT:
        VM_CALC(value, &value1, &value2, >);
        break;
    case kVmTokenGE:
        VM_CALC(value, &value1, &value2, >=);
        break;
    case kVmTokenEQ:
        VM_CALC(value, &value1, &value2, ==);
        break;
    case kVmTokenNEQ:
        VM_CALC(value, &value1, &value2, !=);
        break;
    case kVmTokenAnd:
        VM_CALC(value, &value1, &value2, &&);
        break;
    case kVmTokenOr:
        VM_CALC(value, &value1, &value2, ||);
        break;
    }

    return res;
}

Result vm_executor_exec_const_value_node(VmExecutorContext* context, VmNode* _PAGED current_node, VmValue* value)
{
    VmConstItem* _PAGED item;
    UNUSED(context);

    item = vm_find_const_by_id(current_node->data.operand.value.id_value);
    if (item == NULL) return RES_NOT_FOUND_CONST;

    *value = item->value;

    return RES_OK;
}

Result vm_executor_exec_variable_value_node(VmExecutorContext* context, VmNode* _PAGED current_node, VmValue* value)
{
    VmVariableItem* _PAGED function_item;

    UNUSED(context);

    function_item = vm_find_variable_by_id(current_node->data.operand.value.id_value);
    if (!function_item) return RES_NOT_FOUND_VAR;
    if (!function_item->data.func_data.get_func && !function_item->data.func_data.user_data) return RES_NOT_FOUND_VAR;

    if (context->flag & VM_EXECUTOR_FLAG_IGNORE_EXECUTING) return RES_OK;

    return vm_executor_read_variable(function_item, value);
}

Result vm_executor_exec_function_node(VmExecutorContext* context, VmNode* _PAGED current_node, VmValue* value)
{
    VmFunctionItem* _PAGED function_item;
    int index, param_num = 0;
    Result res;
    VmValue result;
    VmValue g_vm_params[VM_MAX_PARAM_NUM];
    
    function_item = vm_find_function_by_id(current_node->data.operand.value.id_value);
    if (function_item == NULL || function_item->func == NULL) return RES_NOT_FOUND_FUNC;

    param_num = vm_get_function_param_from_prototype(function_item->prototype);
    result.type = kVmNone;
    result.value.i_value = 0;

    for (index = 0; index < param_num; ++index)
    {
        res = vm_executor_exec_node(context, &g_vm_params[index]);
        if (res != RES_OK) return res;
    }

    if (context->flag & VM_EXECUTOR_FLAG_IGNORE_EXECUTING) return RES_OK;

    switch(function_item->prototype)
    {
    case kVmPrototypeVV:
        ((VmFuncVV)function_item->func)();
        break;
    case kVmPrototypeVW:
        ((VmFuncVW)function_item->func)(vm_value_to_int16u(&g_vm_params[0]));
        break;
    case kVmPrototypeBB:
        result.type = kVmInt32;
        result.value.i_value = ((VmFuncBB)function_item->func)(vm_value_to_int8u(&g_vm_params[0]));
        break;
    case kVmPrototypeBW:
        result.type = kVmInt32;
        result.value.i_value = ((VmFuncBW)function_item->func)(vm_value_to_int16u(&g_vm_params[0]));
        break;
   case kVmPrototypeVI:
        ((VmFuncVI)function_item->func)(vm_value_to_int32u(&g_vm_params[0]));
        break;
    case kVmPrototypeWV:
        result.type = kVmInt32;
        result.value.i_value = ((VmFuncWV)function_item->func)();
        break;
    case kVmPrototypeBV:
        result.type = kVmInt32;
        result.value.i_value = ((VmFuncBV)function_item->func)();
        break;
    case kVmPrototypeWW:
        result.type = kVmInt32;
        result.value.i_value = ((VmFuncWW)function_item->func)(vm_value_to_int16u(&g_vm_params[0]));
        break;
    case kVmPrototypeWB:
        result.type = kVmInt32;
        result.value.i_value = ((VmFuncWB)function_item->func)(vm_value_to_int8u(&g_vm_params[0]));
        break;
    case kVmPrototypeWWW:
        result.type = kVmInt32;
        result.value.i_value = ((VmFuncWWW)function_item->func)(vm_value_to_int16u(&g_vm_params[0]),
                vm_value_to_int16u(&g_vm_params[1]));
        break;
    case kVmPrototypeWWI:
        result.type = kVmInt32;
        result.value.i_value = ((VmFuncWWI)function_item->func)(vm_value_to_int16u(&g_vm_params[0]),
                vm_value_to_int32u(&g_vm_params[1]));
        break;
    case kVmPrototypeFF:
        result.type = kVmDouble;
        result.value.f_value = ((VmFuncFF)function_item->func)(vm_value_to_float(&g_vm_params[0]));
        break;
    default:
        return RES_NOT_SUPPORT;
    }

    if (value) *value = result;

    return RES_OK;
}

Result vm_executor_exec_value_node(VmExecutorContext* context, VmNode* _PAGED current_node, VmValue* value)
{
    if (value == NULL) return RES_OK;

    if (context->flag & VM_EXECUTOR_FLAG_IGNORE_EXECUTING)
    {
        return RES_OK;
    }

    if (current_node->data.operand.type == kVmConstant)
    {
        return vm_executor_exec_const_value_node(context, current_node, value);
    }
    else if (current_node->data.operand.type == kVmVariable)
    {
        return vm_executor_exec_variable_value_node(context, current_node, value);
    }
    else
    {
        *value = current_node->data.operand;
    }

    return RES_OK;
}

Result vm_executor_exec_semicolon_node(VmExecutorContext* context,
    VmNode* _PAGED current_node, VmValue* value)
{
    Result res;
    UNUSED(current_node);

    res = vm_executor_exec_node(context, value);
    if (res != RES_OK) return res;

    if (context->node_pos < context->node_len)
        res = vm_executor_exec_node(context, value);

    return res;
}

Result vm_executor_exec_comma_node(VmExecutorContext* context,
    VmNode* _PAGED current_node, VmValue* value)
{
    Result res;
    UNUSED(current_node);
    res = vm_executor_exec_node(context, value);
    if (res != RES_OK) return res;

    return res;
}

Result vm_executor_exec_assignment_variable(VmExecutorContext* context, VmValue* value)
{
    Result res = RES_OK;
    VmNode* _PAGED current_node = context->nodes + (context->node_pos++);

    if (current_node->type != kVmNodeOperand) return RES_INVALID_FORMAT;
    if (current_node->data.operand.type != kVmVariable) return RES_INVALID_FORMAT;

    if (value) *value = current_node->data.operand;

    return RES_OK;
}

Result vm_executor_assign_variable(VmVariableItem* _PAGED item, VmValue* value)
{
    //Result res;
    switch (item->type)
     {
     case kVmInt32:
     case kVmInt32U:
         if ((item->mode & VM_VAR_MODE_FUNC) && item->data.func_data.set_func)
         {
             if (item->mode & VM_VAR_MODE_EXTEND)
                 ((VmSetInt32VariableExFunc)item->data.func_data.set_func)(value->value.i_value,
                         item->data.func_data.user_data);
             else
                 ((VmSetInt32VariableFunc)item->data.func_data.set_func)(value->value.i_value);
         }
         else if ((item->mode & VM_VAR_MODE_ADDRESS) && item->data.address_data.address)
         {
             *((INT32S* _PAGED)item->data.address_data.address) = value->value.i_value;
         }
         break;
     case kVmInt16:
         if ((item->mode & VM_VAR_MODE_FUNC) && item->data.func_data.set_func)
         {
             if (item->mode & VM_VAR_MODE_EXTEND)
                 ((VmSetInt16VariableExFunc)item->data.func_data.set_func)((INT16U)value->value.i_value,
                         item->data.func_data.user_data);
             else
                 ((VmSetInt16VariableFunc)item->data.func_data.set_func)((INT16U)value->value.i_value);
         }
         else if ((item->mode & VM_VAR_MODE_ADDRESS) && item->data.address_data.address)
         {
             *((INT16S* _PAGED)item->data.address_data.address) = (INT16S)value->value.i_value;
         }
         break;
     case kVmInt16U:
         if ((item->mode & VM_VAR_MODE_FUNC) && item->data.func_data.set_func)
         {
             if (item->mode & VM_VAR_MODE_EXTEND)
                 ((VmSetInt16VariableExFunc)item->data.func_data.set_func)((INT16U)value->value.i_value,
                         item->data.func_data.user_data);
             else
                 ((VmSetInt16VariableFunc)item->data.func_data.set_func)((INT16U)value->value.i_value);
         }
         else if ((item->mode & VM_VAR_MODE_ADDRESS) && item->data.address_data.address)
         {
             *((INT16U* _PAGED)item->data.address_data.address) = (INT16U)value->value.i_value;
         }
         break;

     default:
         return RES_NOT_SUPPORT;
         break;
     }

     return RES_OK;
}

Result vm_executor_read_variable(VmVariableItem* _PAGED item, VmValue* value)
{
    if (value == NULL) return RES_OK;

    switch (item->type)
    {
    case kVmInt32:
    case kVmInt32U:
        value->type = kVmInt32;
        value->value.i_value = 0;
        if ((item->mode & VM_VAR_MODE_FUNC) && item->data.func_data.get_func)
        {
            if (item->mode & VM_VAR_MODE_EXTEND)
                ((VmGetInt32VariableExFunc)item->data.func_data.get_func)(&value->value.i_value, item->data.func_data.user_data);
            else
                value->value.i_value = ((VmGetInt32VariableFunc)item->data.func_data.get_func)();
        }
        else if ((item->mode & VM_VAR_MODE_ADDRESS) && item->data.address_data.address)
            value->value.i_value = *((INT32U* _PAGED)item->data.address_data.address);
        break;

    case kVmInt16:
    case kVmInt16U:
        {
            INT16S tmp = 0;
            value->type = kVmInt32;
            value->value.i_value = 0;
            if ((item->mode & VM_VAR_MODE_FUNC) && item->data.func_data.get_func)
            {
                if (item->mode & VM_VAR_MODE_EXTEND)
                    ((VmGetInt16VariableExFunc)item->data.func_data.get_func)(&tmp, item->data.func_data.user_data);
                else
                    tmp = ((VmGetInt16VariableFunc)item->data.func_data.get_func)();
                if (item->type == kVmInt16)
                    value->value.i_value = (INT16S)tmp;
                else
                    value->value.i_value = (INT16U)tmp;
            }
            else if ((item->mode & VM_VAR_MODE_ADDRESS) && item->data.address_data.address)
            {
                value->value.i_value = (item->type == kVmInt16 ? *((INT16S* _PAGED)item->data.address_data.address) : *((INT16U* _PAGED)item->data.address_data.address));
            }
        }
        break;

    default:
        return RES_NOT_SUPPORT;
        break;

    }

    return RES_OK;
}

Result vm_executor_exec_assignement_node(VmExecutorContext* context,
    VmNode* _PAGED current_node, VmValue* value)
{
    Result res;
    VmValue variable_value, assignment_value;
    VmVariableItem* _PAGED function_item;

    UNUSED(current_node);

    res = vm_executor_exec_assignment_variable(context, &variable_value);
    if (res != RES_OK) return res;

    function_item = vm_find_variable_by_id(variable_value.value.id_value);
    if (!function_item || (!function_item->data.func_data.set_func && !function_item->data.address_data.address))
        return RES_NOT_FOUND_VAR;

    res = vm_executor_exec_node(context, &assignment_value);
    if (res != RES_OK) return res;

    if (context->flag & VM_EXECUTOR_FLAG_IGNORE_EXECUTING) return RES_OK;

    res = vm_executor_assign_variable(function_item, &assignment_value);
    if (res != RES_OK) return res;

    if (value) *value = assignment_value;

    return RES_OK;
}

Result vm_executor_exec_node(VmExecutorContext* context, VmValue* value)
{
    Result res = RES_OK;
    VmNode* _PAGED current_node;

    if (context->node_pos >= context->node_len) return res;

    current_node = context->nodes + context->node_pos;
    ++context->node_pos;
    if (current_node->type == kVmNodeOperand)
    {
        return vm_executor_exec_value_node(context, current_node, value);
    }
    else if(current_node->type == kVmNodeFunction)
    {
        return vm_executor_exec_function_node(context, current_node, value);
    }

    if (current_node->data.token == kVmTokenAnd)
    {
        return vm_executor_exec_and_node(context, current_node->data.token, value);
    }
    else if (current_node->data.token == kVmTokenOr)
    {
        return vm_executor_exec_or_node(context, current_node->data.token, value);
    }
    else if ((current_node->data.token >= kVmTokenAgiStart && current_node->data.token <= kVmTokenAgiEnd)
        || (current_node->data.token >= kVmTokenCompareStart && current_node->data.token <= kVmTokenCompareEnd))
    {
        return vm_executor_exec_arithmetic_node(context, current_node->data.token, value);
    }
    else if (current_node->data.token == kVmTokenSemicolon)
    {
        return vm_executor_exec_semicolon_node(context, current_node, value);
    }
    else if (current_node->data.token == kVmTokenComma)
    {
        return vm_executor_exec_comma_node(context, current_node, value);
    }
    else if (current_node->data.token == kVmTokenAssignment)
    {
        return vm_executor_exec_assignement_node(context, current_node, value);
    }
    return res;
}

Result vm_executor_exec(PINT8U buffer, int len, VmValue* value)
{
    Result rc;
    VmExecutorContext context;
    OS_INIT_CRITICAL();
    OS_ENTER_CRITICAL();
    context.nodes = (VmNode* _PAGED)buffer;
    context.node_len = len / sizeof(VmNode);
    context.node_pos = 0;
    context.flag = 0;
    rc = vm_executor_exec_node(&context, value);
    OS_EXIT_CRITICAL();
    return rc;
}

Result vm_interpret(PCSTR expression, VmValue* result)
{
    Result res;
    if (result == NULL) return RES_INVALID_PARAM;

    res = vm_compile(expression, NULL, 0, NULL);
    if (res != RES_OK) return res;

    return vm_executor_exec((INT8U* _PAGED)g_vm_node_buffer,
        sizeof(VmNode) * g_vm_node_buffer_len, result);
    /*
    int index;
    VmExpr expr;
    const char* pos = expression;
    Result res;

    if (!expression) return RES_ERROR;

    expr.expression = expression;
    expr.expression_len = strlen(expression);
    expr.expression_pos = 0;

    res = vm_parser_init(&expr);

    if (res == RES_OK) res = vm_parser_parse_tokens(&expr);

    if (res == RES_OK) res = vm_parser_parse_gramma(&expr);

    if (res == RES_OK) res = vm_executor_exec((INT8U*)g_vm_node_buffer,
    sizeof(VmNode) * g_vm_node_buffer_len, result);

    return res;
    */
}

Result vm_is_number(const char* expr, INT16U* num)
{
    const char* pos = expr;
    if (expr == NULL || strlen(expr) == 0) return RES_ERROR;
    while (*pos != '\0' && !isdigit(*pos++)) return RES_ERROR;

    *num = atoi(expr);

    return RES_OK;
}
