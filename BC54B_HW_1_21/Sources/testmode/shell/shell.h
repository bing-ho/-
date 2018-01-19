#ifndef __SHELL_H__
#define __SHELL_H__


#ifndef __FAR
#define __FAR
#endif


/// \brief shell_loop ִ��shellѭ��.
void shell_loop(void);

/// \brief shell_ask_yes_no ��stdinѯ��, ��ѡ��Yes or No.
///
/// \param prompt ѯ��ʱ�����ʾ��.
///
/// \return !=0 �������Yes; ==0 �������No.
char shell_ask_yes_no(const char *prompt);

#endif

