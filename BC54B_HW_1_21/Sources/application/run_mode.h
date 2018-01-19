#ifndef __RUN_MODE_H__
#define __RUN_MODE_H__

enum runmode {
    RUN_MODE_NORMAL,   //����ģʽ
    RUN_MODE_WAKEUP,   //����ģʽ
    RUN_MODE_TEST,     //��װ����ģʽ
    RUN_MODE_READ,     //��ȡģʽ
    RUN_MODE_DETECT,   //ǿ��ģʽ
};

extern enum runmode mode;
void runmode_init(void);
void runmode_reset_to(enum runmode mode);

// ���������ĳ�����Ҫ��RUN_TEST_MODE_ALWAYS���ó�1
#define RUN_TEST_MODE_ALWAYS 0

#if defined(RUN_TEST_MODE_ALWAYS) && RUN_TEST_MODE_ALWAYS != 0
#define runmode_get() (RUN_MODE_TEST)
#else
enum runmode runmode_get(void);
#endif

#endif
