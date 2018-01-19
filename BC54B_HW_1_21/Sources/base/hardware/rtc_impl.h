#ifndef __RTC_IMPL_H__
#define __RTC_IMPL_H__


enum {
    /// ����ģʽ.
    RTC_TEST_NONE,
    /// ��Զ������ʱ�������úõ�ʱ��, ���ʱ�䲻����.
    RTC_TEST_FAKE_STOP,
    /// ������ʱ������ϵͳ��tick��, ��ʼʱ�������úõ�.
    RTC_TEST_FAKE_RUN,

    /// �����ж�MODE����Ϣ�Ķ���, �����Ķ��嶼�����֮ǰ.
    RTC_TEST_MODE_MAX,
};

char rtc_init(void);
char rtc_set_test(unsigned char mode);
char rtc_write_time(unsigned long time);
char rtc_read_time(unsigned long *__FAR time);

#endif

