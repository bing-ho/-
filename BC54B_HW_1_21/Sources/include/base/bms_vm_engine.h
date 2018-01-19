/**
*
* Copyright (c) 2012 Ligoo Inc.
*
* @file bms_vm_engine.h
* @brief
* @note
* @author Xiangyong Zhang
* @date 2012-7-30
*
*/

#ifndef BMS_VM_ENGINE_H_
#define BMS_VM_ENGINE_H_
#include "bms_defs.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef INT16U VmID;

typedef enum
{
    //kVmInt16 = 0,
	kVmNone,
	kVmInt8,
	kVmInt16,
    kVmInt32,
    kVmInt8U,
    kVmInt16U,
    kVmInt32U,
    kVmDouble,
    kVmVariable,
    kVmFunction,
	kVmConstant,
}VmValueType;

typedef enum
{
	kVmPrototypeVV = 0, /* void(void)*/
	kVmPrototypeVW, /* void(INT16U) */
	kVmPrototypeVI, /* void(INT32U) */
	kVmPrototypeWV, /* INT16U(void) */
	kVmPrototypeBV, /* INT8U(void) */
	kVmPrototypeWW, /* INT16U(INT16U) */
	kVmPrototypeWB, /* INT16U(INT8U) */
	kVmPrototypeFF, /* double(double) */
	kVmPrototypeWWW, /* INT16U(INT16U, INT16U) */
	kVmPrototypeWWI, /* INT16U(INT16U, INT32U) */
	kVmPrototypeBB, /* INT8U(INT8U) */
	kVmPrototypeBW, /* INT8U(INT16U) */

}VmFunctionPrototype;

typedef union
{
    //INT8S  i8_value;
    //INT8U  i8u_value;
    //INT16S i16_value;
    //INT16U i16u_value;
    INT32S i_value;
    //INT32U iu_value;
    double f_value;
    VmID   id_value;
}VmValueItem;

typedef struct
{
    //VmValueType type;
    INT8U       type;
    VmValueItem value;
}VmValue;


typedef void (*VmFunc)(void);
//typedef void(*VmSetVariableFunc)()

void vm_init(void);
void vm_uninit(void);

typedef struct
{
    VmID id;
    const char* name;
    VmValue value;
}VmConstItem;

#define VM_VAR_MODE_ADDRESS 1
#define VM_VAR_MODE_EXTEND  2
#define VM_VAR_MODE_FUNC    4

typedef struct
{
    VmFunc set_func;
    VmFunc get_func;
    void*  user_data;
}VmVariableFuncData;

typedef struct
{
    void* _PAGED address;
}VmVariableAddressData;

typedef union
{
    VmVariableFuncData func_data;
    VmVariableAddressData address_data;
}VmVariableData;

typedef struct
{
    VmID id;
    INT8U mode;
    const char* name;
    //VmValueType type;
    INT8U   type;
    VmVariableData data;
}VmVariableItem;

typedef struct _VmFunctionItem
{
    VmID id;
    const char* name;
    VmFunc func;
    INT8U prototype;
    /* INT8U   param_num; */
}VmFunctionItem;


Result vm_register_function(INT16U id, const char* function, VmFunc func, VmFunctionPrototype prototype);
VmFunctionItem* _PAGED vm_find_function(const char* name);
VmFunctionItem* _PAGED vm_find_function_by_id(VmID id);
INT16U vm_get_function_num(void);


Result vm_register_constant(INT16U id, const char* name, const VmValue* value);
VmConstItem* _PAGED vm_find_const(const char* name);
VmConstItem* _PAGED vm_find_const_by_id(VmID id);
INT16U vm_get_const_num(void);

Result vm_register_variable(INT16U id, const char* name, VmValueType type, VmFunc set_func, VmFunc get_func);
Result vm_register_direct_variable(INT16U id, const char* name, VmValueType type, void* variable);
Result vm_register_variable_ex(INT16U id, const char* name, VmValueType type,
        VmFunc set_func, VmFunc get_func, void* user_data);
VmVariableItem* _PAGED vm_find_variable(const char* name);
VmVariableItem* _PAGED vm_find_variable_by_id(VmID id);
INT16U vm_get_variable_num(void);

Result vm_compile(PCSTR expression, PINT8U buffer, INT16U size, INT16U* output_size);
Result vm_run(PINT8U buffer, INT16U size, VmValue* result);
Result vm_interpret(PCSTR expression, VmValue* result);

/* Conversion Functions */
INT32U vm_value_to_int32u(const VmValue* _PAGED value);
INT16U vm_value_to_int16u(const VmValue* _PAGED value);
double vm_value_to_float(const VmValue* _PAGED value);
VmValue vm_float_to_value(double value);
VmValue vm_int_to_value(INT32U value);
VmValue* vm_int_to_value_ptr(INT32U value);

#ifdef __cplusplus
}
#endif

#endif /* BMS_VM_ENGINE_H_ */
