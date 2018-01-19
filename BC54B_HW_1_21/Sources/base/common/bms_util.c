/**
 *
 * Copyright (c) 2012 Ligoo Inc.
 *
 * @file bms_util.c
 * @brief
 * @note
 * @author
 * @date 2012-5-9
 *
 */
#include "bms_util.h"

#pragma MESSAGE DISABLE C1420 // Result of function-call is ignored

#if BMS_SUPPORT_UTIL

/* CRC 高位字节值表 */
const INT8U g_crc_byte_hi[] =
{ 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00,
        0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00,
        0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00,
        0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00,
        0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00,
        0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00,
        0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01,
        0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00,
        0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00,
        0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00,
        0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00,
        0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00,
        0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00,
        0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01,
        0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00,
        0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00,
        0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00,
        0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00,
        0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00,
        0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00,
        0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00,
        0xC1, 0x81, 0x40 };

/* CRC低位字节值表*/
const INT8U g_crc_byte_lo[] =
{ 0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06, 0x07, 0xC7, 0x05,
        0xC5, 0xC4, 0x04, 0xCC, 0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E, 0x0A,
        0xCA, 0xCB, 0x0B, 0xC9, 0x09, 0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B,
        0xDB, 0xDA, 0x1A, 0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC, 0x14,
        0xD4, 0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3, 0x11,
        0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 0xF2, 0x32, 0x36,
        0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4, 0x3C, 0xFC, 0xFD, 0x3D, 0xFF,
        0x3F, 0x3E, 0xFE, 0xFA, 0x3A, 0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28,
        0xE8, 0xE9, 0x29, 0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF, 0x2D,
        0xED, 0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26, 0x22,
        0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60, 0x61, 0xA1, 0x63,
        0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64, 0xA4, 0x6C,
        0xAC, 0xAD, 0x6D, 0xAF, 0x6F, 0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69,
        0xA9, 0xA8, 0x68, 0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA, 0xBE,
        0x7E, 0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5, 0x77,
        0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 0x70, 0xB0, 0x50,
        0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92, 0x96, 0x56, 0x57, 0x97, 0x55,
        0x95, 0x94, 0x54, 0x9C, 0x5C, 0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A,
        0x9A, 0x9B, 0x5B, 0x99, 0x59, 0x58, 0x98, 0x88, 0x48, 0x49, 0x89, 0x4B,
        0x8B, 0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C, 0x44,
        0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42, 0x43, 0x83, 0x41,
        0x81, 0x80, 0x40 };


INT16U crc_check_bt(PINT8U msg, INT16U len)
{
    INT8U uCRCByteHi = 0xFF; /* 高CRC字节初始化 */
    INT8U uCRCByteLo = 0xFF; /* 低CRC 字节初始化 */
    INT32U uIndex; /* CRC循环中的索引 */
    while (len--) /* 传输消息缓冲区 */
    {
        uIndex = uCRCByteHi ^ (*msg++); /* 计算CRC */
        uCRCByteHi = uCRCByteLo ^ g_crc_byte_hi[uIndex];
        uCRCByteLo = g_crc_byte_lo[uIndex];
    }
    return (uCRCByteHi << 8 | uCRCByteLo);
}

INT16U crc_check(PINT8U msg, INT16U len)
{
    INT8U uCRCByteHi = 0xFF; /* 高CRC字节初始化 */
    INT8U uCRCByteLo = 0xFF; /* 低CRC 字节初始化 */
    INT32U uIndex; /* CRC循环中的索引 */
    while (len--) /* 传输消息缓冲区 */
    {
        uIndex = uCRCByteLo ^ (*msg++); /* 计算CRC */
        uCRCByteLo = uCRCByteHi ^ g_crc_byte_hi[uIndex];
        uCRCByteHi = g_crc_byte_lo[uIndex];
    }
    return (uCRCByteHi << 8 | uCRCByteLo);
}

void crc_check_with_byte(INT8U data, INT16U* crc)
{
    INT8U uCRCByteHi = 0xFF; /* 高CRC字节初始化 */
    INT8U uCRCByteLo = 0xFF; /* 低CRC 字节初始化 */
    INT32U uIndex; /* CRC循环中的索引 */
    
    uCRCByteHi = *crc >> 8;
    uCRCByteLo = (INT8U)(*crc);
    
    uIndex = uCRCByteLo ^ (data); /* 计算CRC */
    uCRCByteLo = uCRCByteHi ^ g_crc_byte_hi[uIndex];
    uCRCByteHi = g_crc_byte_lo[uIndex];
    
    *crc = (uCRCByteHi << 8 | uCRCByteLo);
}


INT16U crc_check_accum(PINT8U msg, INT16U len, INT16U last)
{
    INT8U uCRCByteHi = (INT8U) (last >> 8); /* 高CRC字节初始化 */
    INT8U uCRCByteLo = (INT8U) last; /* 低CRC 字节初始化 */
    INT32U uIndex; /* CRC循环中的索引 */
    while (len--) /* 传输消息缓冲区 */
    {
        uIndex = uCRCByteHi ^ (*msg++); /* 计算CRC */
        uCRCByteHi = uCRCByteLo ^ g_crc_byte_hi[uIndex];
        uCRCByteLo = g_crc_byte_lo[uIndex];
    }
    return (uCRCByteHi << 8 | uCRCByteLo);
}

INT8U acc_check(PINT8U data, INT8U length)
{
    INT8U total, index;

    if (data == NULL) return 0;
    
    total = 0;
    for (index = 0; index < length; index++)
    {
        total += data[index];
    }

    return total;
}

INT16U average_check(PINT16U data, INT16U length)
{
    INT16U index;
    INT32U total = 0;
    
    if (data == NULL) return 0;
    
    for (index = 0; index < length; index++)
    {
        total += data[index];
    }
    
    return (INT16U)(total / length);
}

float float_average_check(PFLOAT data, INT16U length)
{
    INT16U index;
    double total = 0;
    
    if (data == NULL) return 0;
    
    for (index = 0; index < length; index++)
    {
        total += data[index];
    }
    
    return (float)(total / length);
}

INT8U bcd_to_hex(INT8U value)
{
    return value % 0x10 + (value / 0x10) * 10;
}

INT8U hex_to_bcd(INT8U value)
{
    return (value / 10) * 16 + value % 10;
}

void reverse_string(char *str)            //将字符串倒置
{
    char *tailor;
    INT8U temp;

    for(tailor = str + strlen(str) - 1; str < tailor ; tailor--, str++)
    {
        temp = *str;
        *str = *tailor;
        *tailor = temp;
    }
}

void reverse_buffer(unsigned char *buff, int len)            //将数组倒置
{
    INT8U temp;
    INT16U i, tail;
    
    if(buff == NULL) return;
    
    tail = len-1;
    len = len >> 1;
    for(i=0; i<len; i++)
    {
        temp = buff[i];
        buff[i] = buff[tail];
        buff[tail] = temp;
        tail--;
    }
}

void char_to_bcd_string(INT8U value, char *str)
{
    INT32U n;

    n = value;

    *str++ = (INT8U) ((n / 10) + '0');
    *str++ = (INT8U) ((n % 10) + '0');

    *str = 0x00;
}

INT8U int_to_bcd_string(INT32U value, char *str) //将无符号整数转换成BCD码表示的字符串
{
    INT32U n;
    char *t;
    INT8U i = 0;

    n = value;
    t = str;
    if (value == 0)
    {
        *str++ = '0';
        *str = 0x00;
        return 1;
    }
    do
    {
        *str++ = (INT8U) ((n % 10) + '0');
        i++;
    } while ((n /= 10) > 0);

    *str = 0x00;

    reverse_string((char*)t);
    return i;
}

void float_to_bcd_string(FP64 value, char *str, INT8U fractional_cnt)
{
    INT8U i;
    INT32S temp;

    if (value < 0) *str++ = '-';
    temp = (INT32S)value;
    temp = ABS(temp);

    str += int_to_bcd_string((INT32U)temp, str);
    *str++ = '.';
    if (value < 0)
        value = value * (-1) - temp;
    else
        value =value - temp + 0.000005;
    for (i = 0; i < fractional_cnt; i++)
    {
        
        value *= (double)10.0;
        temp = value;
        str+=int_to_bcd_string((INT32U)temp, str);
        value -= temp;
    }
}

INT8U bcd_to_int(INT8U input1, BOOLEAN sign)
{
    INT8U short1, short2;
    short1 = input1;
    short1 &= 0xf;
    short2 = short1;

    short1 = input1;
    short1 &= 0xf0;
    short1 = short1 >> 4;
    short2 = short2 + short1 * 10;

    if (sign)
    {
        short2 ^= 0xff;
        short2 = short2 + 1;
    }

    return short2;
}

void safe_memset(PVOID buffer, INT8U value, INT16U size)
{
  PINT8U data = (PINT8U)buffer;
  INT16U index;
  if (buffer == NULL) return;
  for (index = 0; index < size; ++index)
    data[index] = value;
}

void safe_memcpy(PINT8U dest, PCINT8U src, INT16U size)
{
  PINT8U dest_data = (PINT8U)dest;
  PINT8U src_data = (PINT8U)src;
  INT16U index;
  if (dest == NULL || src == NULL) return;
  for (index = 0; index < size; ++index)
    dest_data[index] = src_data[index];
}

int safe_strcmp(PCSTR str1, PCSTR str2)
{
  if (str1 == NULL || str2 == NULL) return 1;
  while(*str1 != '\0' && *str2 != '\0' && *str1 == *str2)
  {
      str1++;
      str2++;
  }

  if (*str1 == '\0' && *str2 == '\0')
      return 0;
  else if (*str1 == '\0')
      return -1;
  else if (*str2 == '\0')
      return 1;
  else
      return (*str1 - *str2);
}

int safe_memcmp(PCINT8U ptr1, PCINT8U ptr2, INT16U count)
{
  if (!count) {
    return 0;
  }

  if (ptr1 == NULL || ptr2 == NULL) {
      return 1;
  }

  while (--count) {
    if (*ptr1 != *ptr2) {
      break;
    }
    ++ptr1;
    ++ptr2;
  }
  return (*ptr1 - *ptr2);
}


PSTR safe_strcpy(PSTR str_d, PCSTR str_s) {
  PSTR sd = str_d;
  while (*str_s) {
    *str_d++ = *str_s++;
  }
  *str_d = *str_s;
  return sd;
}

INT16U safe_strlen(PCSTR text)
{
    PCSTR s;

    if (text == NULL) return 0;

    for (s = text; *s; ++s);

    return (s - text);
}


/*****************************************************/
PSTR safe_strstr(PCSTR str1, PCSTR str2) {
  register int    count;
  register size_t len;

  len = safe_strlen(str2);
  if (len == 0U) {
    return (PSTR)str1;  /*lint !e926 , MISRA 11.4 ADV, safe conversion, from 'const char *' to 'char *' */
  }
  {
    size_t tmp = (safe_strlen(str1) + (size_t)1) - len;
    count = (int)tmp;
  }
  if (count < 1) {
    return (NULL);
  }
  while (count--) {
    if (*str1 == *str2) {
      if (safe_memcmp((const unsigned char *far)str1, (const unsigned char *far)str2, len) == 0) {
        return (PSTR)str1; /*lint !e926 , MISRA 11.4 ADV, safe conversion, from 'const char *' to 'char *' */
      }
    }
    ++str1;
  }
  return (NULL);
}

INT32U offset_convert(INT32U value,INT32S offset)
{
    if(value == 0)   return value;
    if(offset>=0) return value+offset;
    if(value <= (0-offset))    return 1;
    return value + offset;

}

INT8U bms_value_get_nearby(INT16U value_left, INT16U value_right, INT16U value_middle)
{//返回0表示不再此范围内，返回1表示接近left，返回2表示接近right
    if(value_left == value_middle) return 1;
        
    if(value_right == value_middle) return 2;
        
    if(value_left <= value_right)
    {
        if(value_left > value_middle || value_right < value_middle) return 0;
        
        if(value_middle - value_left > value_right - value_middle) return 2;
        else return 1;
    }
    else
    {
        if(value_right > value_middle || value_left < value_middle) return 0;
        
        if(value_middle - value_right < value_left - value_middle) return 2;
        else return 1;
    }
}

INT16U bms_get_byte_nearby_index(INT8U* _PAGED buff, INT16U size, INT8U value)
{
    INT16U i, index, temp_index = 0xFFFF;
    
    if(buff == NULL) return 0xFFFF;
    if(size == 1) return 0;
    
    for(i=0; i<size-1; i++)
    {
        index = bms_value_get_nearby(buff[i], buff[i+1], value);
        if(index != 0)
        {
            if(index == 1) temp_index = i;
            else if(index == 2) temp_index = i + 1;
            
            break;
        }
    }
    if(temp_index >= size)
    {
        if(buff[0] >= buff[size - 1])
        {
            if(buff[0] <= value) temp_index = 0;
            else if(buff[size-1] >= value) temp_index = size - 1;
        }
        else
        {
            if(buff[0] >= value) temp_index = 0;
            else if(buff[size - 1] <= value) temp_index = size - 1;
        }
    }
    if(temp_index >= size) return 0xFFFF;
    
    return temp_index;
}

INT16U bms_get_word_nearby_index(INT16U* _PAGED buff, INT16U size, INT16U value)
{
    INT16U i, index, temp_index = 0xFFFF;
    
    if(buff == NULL) return 0xFFFF;
    if(size == 1) return 0;
    
    for(i=0; i<size-1; i++)
    {
        index = bms_value_get_nearby(buff[i], buff[i+1], value);
        if(index != 0)
        {
            if(index == 1) temp_index = i;
            else if(index == 2) temp_index = i + 1;
            
            break;
        }
    }
    
    if(temp_index >= size)
    {
        if(buff[0] >= buff[size - 1])
        {
            if(buff[0] <= value) temp_index = 0;
            else if(buff[size-1] >= value) temp_index = size - 1;
        }
        else
        {
            if(buff[0] >= value) temp_index = 0;
            else if(buff[size - 1] <= value) temp_index = size - 1;
        }
    }
    if(temp_index >= size) return 0xFFFF;
    
    return temp_index;
}

INT16U bms_get_byte_range_index(INT8U* _PAGED buff, INT16U size, INT16U value)//获取数值在数组中的哪个区间内，获取区间索引(趋左)
{
    INT16U i, index = 0xFFFF;
    
    if(buff == NULL) return index;
    if(size == 1) return 0;
    
    for(i=0; i<size - 1; i++)
    {
        if((value >= buff[i] && value <= buff[i+1]) ||
            (value <= buff[i] && value >= buff[i+1]))
        {
            index = i; break;
        }
    }
    return index;
}

INT16U bms_get_word_range_index(INT16U* _PAGED buff, INT16U size, INT16U value)//获取数值在数组中的哪个区间内，获取区间索引(趋左)
{
    INT16U i, index = 0xFFFF;
    
    if(buff == NULL) return index;
    if(size == 1) return 0;
    
    for(i=0; i<size - 1; i++)
    {
        if((value >= buff[i] && value <= buff[i+1]) ||
            (value <= buff[i] && value >= buff[i+1]))
        {
            index = i; break;
        }
    }
    return index;
}

//线性插值函数
float math_linear_differential_fun(float para_x1, float para_y1, float para_x2, float para_y2, float para_x)
{
    static float a, b; //y=ax+b
    
    a = (para_y2 - para_y1) / (para_x2 - para_x1);
    b = para_y1 - a * para_x1;
    
    return a * para_x + b;
}

//滤波
INT16U bms_middle_average_value_filter(INT16S* _PAGED value,INT8U size, INT8U filter_num)
{
    register INT8U i,j;
    INT32U temp; 
    INT16U temp1;
    
    for(i=0; i<size-1; i++)
    {
        for(j=i;j<size;j++)
        {
            if(value[i] < value[j])
            {    
                temp1 = value[i];
                value[i] = value[j];
                value[j] = temp1;                //进行排序：从大到小
            }
        }
    }
    temp = 0;
    if((filter_num << 1) >= size) filter_num = 0;
    for(i=filter_num;i<size-filter_num;i++)
    {
        temp += value[i];
    }
    temp1 = (INT16U)(temp/(size-(filter_num << 1)));            //前后各去掉10个数后取平均值。总共5个数。
    return temp1;
}

#endif


