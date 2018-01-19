/**
*
* Copyright (c) 2012 Ligoo Inc.
*
* @file bms_vm_engine_impl.h
* @brief
* @note
* @author Xiangyong Zhang
* @date 2012-7-30
*
*/

#ifndef BMS_VM_ENGINE_IMPL_H_
#define BMS_VM_ENGINE_IMPL_H_
#include "bms_vm_engine.h"
#include "bms_defs.h"
#include "bms_log.h"
#include "bms_util.h"

#ifndef WIN32
#define CW5
#endif

typedef enum
{
	kVmTokenAssignment = 1,
	kVmTokenVariableAssign, //not use
	kVmTokenVariableGet,

	kVmTokenFunction = 10,
	kVmTokenValue,
	kVmTokenVariable,

	kVmTokenLeftBracket = 20,
	kVmTokenRightBracket,

	kVmTokenLeftBrace,
	kVmTokenRightBrace,

	kVmTokenComma,
	kVmTokenSemicolon,

	kVmTokenAgiStart = 30,
	kVmTokenAdd = kVmTokenAgiStart,
	kVmTokenSub,
	kVmTokenMul,
	kVmTokenDiv,
	kVmTokenMod,
	kVmTokenBitAnd,
	kVmTokenBitOr,
	kVmTokenAgiEnd = kVmTokenBitOr,

	kVmTokenCompareStart = 40,
	kVmTokenEQ = kVmTokenCompareStart,
	kVmTokenGT,
	kVmTokenGE,
	kVmTokenLT,
	kVmTokenLE,
	kVmTokenNEQ,

	kVmTokenAnd,
	kVmTokenOr,
	kVmTokenCompareEnd = kVmTokenOr,

	kVmTokenMax,
}VmToken;

typedef struct
{
	VmToken token;
	int start;
	int end;
	VmValue value;
}VmTokenItem;

typedef enum
{
	kVmNodeOperator,
	kVmNodeOperand,
	kVmNodeFunction,
}VmNodeType;

typedef union _VmNodeData
{
	VmValue operand;
	VmToken token;
}VmNodeData;

typedef struct
{
	VmNodeType type;
	VmNodeData data;
}VmNode;


typedef struct
{
	PCSTR expression;
	int expression_len;
	int   expression_pos;
}VmExpr;

/** Optimize for &&, || */
#define VM_EXECUTOR_FLAG_IGNORE_EXECUTING 0x01

typedef struct
{
	VmNode* _PAGED nodes;
	int  node_pos;
	int  node_len;
	INT8U flag;
}VmExecutorContext;

/* Engine Functions */
INT8U vm_get_function_param_from_prototype(VmFunctionPrototype prototype);

/** Parser Functions */
Result vm_parser_output_buffer_push(VmNode* _PAGED node);
Result vm_parser_output_buffer_add_operator(VmToken token);

Result vm_parser_token_is_value(VmToken token);

Result vm_parser_lookup_token(VmToken* token);
Result vm_parser_add_token(VmTokenItem* token);
Result vm_parser_add_operator_token(VmExpr* _PAGED expr, int token_len, VmToken token);

Result vm_parser_token_stack_push(VmToken token);
Result vm_parser_token_stack_pop(VmToken* token);
Result vm_parser_token_stack_lookup(VmToken* token);

Result vm_parser_init(VmExpr* _PAGED expr);
Result vm_parser_parse_tokens(VmExpr* _PAGED expr);
Result vm_parser_parse_gramma(VmExpr* _PAGED expr);

Result vm_parser_parse_tokens_alpha(VmExpr* _PAGED expr);
Result vm_parser_parse_tokens_number(VmExpr* _PAGED expr);
Result vm_parser_parse_tokens_variable(VmExpr* _PAGED expr);

/** Executor Functions */
Result vm_executor_exec(PINT8U buffer, int len, VmValue* value);

Result vm_executor_exec_node(VmExecutorContext* context, VmValue* value);

Result vm_executor_exec_add_node(VmExecutorContext* context, VmValue* value);
Result vm_executor_exec_and_node(VmExecutorContext* context, VmToken token, VmValue* value);
Result vm_executor_exec_or_node(VmExecutorContext* context, VmToken token, VmValue* value);
Result vm_executor_exec_arithmetic_node(VmExecutorContext* context, VmToken token, VmValue* value);
Result vm_executor_exec_const_value_node(VmExecutorContext* context, VmNode* _PAGED current_node, VmValue* value);
Result vm_executor_exec_variable_value_node(VmExecutorContext* context, VmNode* _PAGED current_node, VmValue* value);
Result vm_executor_exec_function_node(VmExecutorContext* context, VmNode* _PAGED current_node, VmValue* value);
Result vm_executor_exec_value_node(VmExecutorContext* context, VmNode* _PAGED current_node, VmValue* value);
Result vm_executor_exec_semicolon_node(VmExecutorContext* context, VmNode* _PAGED current_node, VmValue* value);
Result vm_executor_exec_comma_node(VmExecutorContext* context, VmNode* _PAGED current_node, VmValue* value);
Result vm_executor_exec_assignment_variable(VmExecutorContext* context, VmValue* value);
Result vm_executor_exec_assignement_node(VmExecutorContext* context, VmNode* _PAGED current_node, VmValue* value);

Result vm_executor_read_variable(VmVariableItem* _PAGED item, VmValue* value);
Result vm_executor_assign_variable(VmVariableItem* _PAGED item, VmValue* value);

Result  vm_is_number(const char* expr, INT16U* num);

#endif /* BMS_VM_ENGINE_IMPL_H_ */
