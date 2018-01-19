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
 * �ۼӺ�У�������
 * @param data Ҫ���������
 * @param length Ҫ��������ݳ���
 * @return �������ۼӺ�У����
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
 * ���ַ�������
 * @param str Ŀ���ַ�����ԭ��ת��
 */
void reverse_string(char *str);

/**
 * �����鵹��
 * @param buff Ŀ������
 * @param len  ���鳤��
 */
void reverse_buffer(unsigned char *buff, int len);

/**
 * ���޷����ַ�ת����BCD���ʾ���ַ���
 * @param value ��ת����
 * @param str ת�����ַ����洢λ��
 */
void char_to_bcd_string(INT8U value, char *str);

/**
 * ��BCD��ת�����з���������
 * @param input1 ��ת��BCD��
 * @param sign ����BCD��ķ���
 * @return �޷�����ֵ
 */
INT8U bcd_to_int(INT8U input1, BOOLEAN sign);

/**
 * ��������ת�����ַ���
 * @param value ��ת��������
 * @param str ת�����ַ����洢λ��
 * @param fractional_cnt ��ת���ĸ�������С��λ��
 */
void float_to_bcd_string(FP64 value, char *str, INT8U fractional_cnt);

/**
 * ���޷�������ת����BCD���ʾ���ַ���
 * @param value ��ת����
 * @param str ת�����ַ����洢λ��
 * @return ��������Чλ��
 */
INT8U int_to_bcd_string(INT32U value, char *str);

/**
 * ���ַ�����ת�����ַ���
 * @param value ��ת����
 * @param str ת�����ַ����洢λ��
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
INT16U bms_get_word_range_index(INT16U* _PAGED buff, INT16U size, INT16U value);//��ȡ��ֵ�������е��ĸ������ڣ���ȡ��������(����)

float math_linear_differential_fun(float para_x1, float para_y1, float para_x2, float para_y2, float para_x);

INT16U bms_middle_average_value_filter(INT16S* _PAGED value,INT8U size, INT8U filter_num);

#endif /* BMS_UTIL_H_ */
