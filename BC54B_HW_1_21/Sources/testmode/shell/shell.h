#ifndef __SHELL_H__
#define __SHELL_H__


#ifndef __FAR
#define __FAR
#endif


/// \brief shell_loop 执行shell循环.
void shell_loop(void);

/// \brief shell_ask_yes_no 从stdin询问, 并选择Yes or No.
///
/// \param prompt 询问时候的提示语.
///
/// \return !=0 输入的是Yes; ==0 输入的是No.
char shell_ask_yes_no(const char *prompt);

#endif

