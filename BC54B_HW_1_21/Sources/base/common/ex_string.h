/// \file ex_string.h
/// \brief  string��չ����������
/// \author Xi Qingping, qingping.xi@ligoo.cn
/// \version
/// \date 2015-09-06
#ifndef __EX_STRING_H__
#define __EX_STRING_H__

#ifndef __FAR
#define __FAR
#endif

/// \brief �ж��ַ����Ƿ�ȫ��һ��ȫ����[0-9].
///
/// \param s ��Ҫ�жϵ��ַ���.
///
/// \return !=0 ȫ���������;
/// \return ==0 �ַ����а����������ַ�.
char ExString_IsDigitString(const char *__FAR s);
/// \brief �ж�ָ�����ȵ��ַ����Ƿ�ȫ��һ��ȫ����[0-9].
///
/// \param s ��Ҫ�жϵ��ַ���.
/// \param len ��Ҫ�жϵ��ַ�������.
///
/// \return !=0 ȫ���������;
/// \return ==0 �ַ����а����������ַ�.
char ExString_NIsDigitString(const char *__FAR s, int len);

/// \brief �ж��ַ����Ƿ�ȫ��һ��ȫ16��������[0-9a-fA-F]
///
/// \param s ��Ҫ�жϵ��ַ���.
///
/// \return !=0 ȫ��16�����������;
/// \return ==0 �ַ����а�����16���������ַ�.
char ExString_IsHexString(const char *__FAR s);

/// \brief �ж�һ���ַ����Ƿ�Ϊһ��IP��ַ.
///
/// \param s ��Ҫ�жϵ��ַ���.
///
/// \return !=0 ��һ��IP��ַ�ַ���;
/// \return ==0 ����һ��IP��ַ�ַ���.
unsigned char ExString_IsIPString(const char *__FAR s);

long ExString_Atol(const char *__FAR s);
long ExString_AtolSkipBankPrefix(const char *__FAR s);
long ExString_StrToLDecade(const char *__FAR s, const char *__FAR *left);
long ExString_StrToLHex(const char *__FAR s, const char *__FAR *left);


#endif
