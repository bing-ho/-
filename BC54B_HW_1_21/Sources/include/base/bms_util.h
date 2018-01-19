/**
 *
 * Copyright (c) 2012 Ligoo Inc.
 *
 * @file bms_util.h
 * @brief
 * @note
 * @author
 * @date 2012-5-9
 *
 */

#ifndef BMS_UTIL_H_
#define BMS_UTIL_H_
#include "bms_defs.h"
#include "includes.h"

#ifndef BMS_SUPPORT_UTIL
#define BMS_SUPPORT_UTIL        1
#endif

#define ABS(VALUE) ((VALUE) >= 0 ? (VALUE) : (-(VALUE)))
#define SAFE_CALL(FUN)  do{OS_CPU_SR cpu_sr = 0; OS_ENTER_CRITICAL(); (void)FUN; OS_EXIT_CRITICAL();}while(0)
#define SAFE_CALL_WITH_RETURN(FUN, RES)  do{OS_CPU_SR cpu_sr = 0; OS_ENTER_CRITICAL(); RES = FUN; OS_EXIT_CRITICAL();}while(0)

/**********************************************
 *
 * CRC function
 *
 ***********************************************/
INT16U crc_check(PINT8U msg, INT16U len);
void crc_check_with_byte(INT8U data, INT16U* crc);
INT16U crc_check_bt(PINT8U msg, INT16U len);
INT16U crc_check_accum(PINT8U msg, INT16U len, INT16U last);


/**********************************************
 *
 * Accumulative Checking function
 *
 ***********************************************/
/**
 * 累加和校验码计算
 * @param data 要计算的数据
 * @param length 要计算的数据长度
 * @return 计算后的累加和校验码
 */
INT8U acc_check(PINT8U data, INT8U length);
INT16U average_check(PINT16U data, INT16U length);
float float_average_check(PFLOAT data, INT16U length);
/**********************************************
 *
 * BCD/HEX Conversion
 *
 ***********************************************/
INT8U bcd_to_hex(INT8U value);
INT8U hex_to_bcd(INT8U value);


/**********************************************
 *
 * BCD/String Conversion
 *
 ***********************************************/
/**
 * 将字符串倒置
 * @param str 目标字符串，原地转换
 */
void reverse_string(char *str);

/**
 * 将数组倒置
 * @param buff 目标数组
 * @param len  数组长度
 */
void reverse_buffer(unsigned char *buff, int len);

/**
 * 将无符号字符转换成BCD码表示的字符串
 * @param value 待转换数
 * @param str 转换后字符串存储位置
 */
void char_to_bcd_string(INT8U value, char *str);

/**
 * 将BCD码转换成有符号整型数
 * @param input1 待转换BCD码
 * @param sign 代表BCD码的符号
 * @return 无符号数值
 */
INT8U bcd_to_int(INT8U input1, BOOLEAN sign);

/**
 * 将浮点数转换成字符串
 * @param value 待转换浮点数
 * @param str 转换后字符串存储位置
 * @param fractional_cnt 待转换的浮点数的小数位数
 */
void float_to_bcd_string(FP64 value, char *str, INT8U fractional_cnt);

/**
 * 将无符号整数转换成BCD码表示的字符串
 * @param value 待转换数
 * @param str 转换后字符串存储位置
 * @return 整数的有效位数
 */
INT8U int_to_bcd_string(INT32U value, char *str);

/**
 * 将字符常数转换成字符串
 * @param value 待转换数
 * @param str 转换后字符串存储位置
 */
void char_to_bcd_string(INT8U value, char *str);

void safe_memset(PVOID buffer, INT8U value, INT16U size);
void safe_memcpy(PINT8U dest, PCINT8U src, INT16U size);
int safe_memcmp(PCINT8U ptr1, PCINT8U ptr2, INT16U count);
INT16U safe_strlen(PCSTR text);
int safe_strcmp(PCSTR str1, PCSTR str2);
PSTR safe_strcpy(PSTR str_d, PCSTR str_s);
PSTR safe_strstr(PCSTR str1, PCSTR str2);

INT32U offset_convert(INT32U value,INT32S offset);

INT8U bms_value_get_nearby(INT16U value_left, INT16U value_right, INT16U value_middle);
INT16U bms_get_byte_nearby_index(INT8U* _PAGED buff, INT16U size, INT8U value);
INT16U bms_get_word_nearby_index(INT16U* _PAGED buff, INT16U size, INT16U value);
INT16U bms_get_byte_range_index(INT8U* _PAGED buff, INT16U size, INT16U value);
INT16U bms_get_word_range_index(INT16U* _PAGED buff, INT16U size, INT16U value);//获取数值在数组中的哪个区间内，获取区间索引(趋左)

float math_linear_differential_fun(float para_x1, float para_y1, float para_x2, float para_y2, float para_x);

INT16U bms_middle_average_value_filter(INT16S* _PAGED value,INT8U size, INT8U filter_num);

#endif /* BMS_UTIL_H_ */
