/// \file ex_string.h
/// \brief  string扩展操作的声明
/// \author Xi Qingping, qingping.xi@ligoo.cn
/// \version
/// \date 2015-09-06
#ifndef __EX_STRING_H__
#define __EX_STRING_H__

#ifndef __FAR
#define __FAR
#endif

/// \brief 判断字符串是否全是一个全数字[0-9].
///
/// \param s 需要判断的字符串.
///
/// \return !=0 全是数字组成;
/// \return ==0 字符串中包含非数字字符.
char ExString_IsDigitString(const char *__FAR s);
/// \brief 判断指定长度的字符串是否全是一个全数字[0-9].
///
/// \param s 需要判断的字符串.
/// \param len 需要判断的字符串长度.
///
/// \return !=0 全是数字组成;
/// \return ==0 字符串中包含非数字字符.
char ExString_NIsDigitString(const char *__FAR s, int len);

/// \brief 判断字符串是否全是一个全16进制数字[0-9a-fA-F]
///
/// \param s 需要判断的字符串.
///
/// \return !=0 全是16进制数字组成;
/// \return ==0 字符串中包含非16进制数字字符.
char ExString_IsHexString(const char *__FAR s);

/// \brief 判断一个字符串是否为一个IP地址.
///
/// \param s 需要判断的字符串.
///
/// \return !=0 是一个IP地址字符串;
/// \return ==0 不是一个IP地址字符串.
unsigned char ExString_IsIPString(const char *__FAR s);

long ExString_Atol(const char *__FAR s);
long ExString_AtolSkipBankPrefix(const char *__FAR s);
long ExString_StrToLDecade(const char *__FAR s, const char *__FAR *left);
long ExString_StrToLHex(const char *__FAR s, const char *__FAR *left);


#endif
