#ifndef __RUN_MODE_H__
#define __RUN_MODE_H__

enum runmode {
    RUN_MODE_NORMAL,   //正常模式
    RUN_MODE_WAKEUP,   //唤醒模式
    RUN_MODE_TEST,     //工装测试模式
    RUN_MODE_READ,     //读取模式
    RUN_MODE_DETECT,   //强检模式
};

extern enum runmode mode;
void runmode_init(void);
void runmode_reset_to(enum runmode mode);

// 给代工厂的程序需要把RUN_TEST_MODE_ALWAYS设置成1
#define RUN_TEST_MODE_ALWAYS 0

#if defined(RUN_TEST_MODE_ALWAYS) && RUN_TEST_MODE_ALWAYS != 0
#define runmode_get() (RUN_MODE_TEST)
#else
enum runmode runmode_get(void);
#endif

#endif
