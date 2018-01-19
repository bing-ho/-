#ifndef __RTC_IMPL_H__
#define __RTC_IMPL_H__


enum {
    /// 正常模式.
    RTC_TEST_NONE,
    /// 永远读到的时间是设置好的时间, 这个时间不会跑.
    RTC_TEST_FAKE_STOP,
    /// 读到的时间会根据系统的tick跑, 起始时间是设置好的.
    RTC_TEST_FAKE_RUN,

    /// 用于判断MODE有消息的定义, 其他的定义都在这个之前.
    RTC_TEST_MODE_MAX,
};

char rtc_init(void);
char rtc_set_test(unsigned char mode);
char rtc_write_time(unsigned long time);
char rtc_read_time(unsigned long *__FAR time);

#endif

