#ifndef __SECOND_DATE_H__
#define __SECOND_DATE_H__

#ifndef __FAR
#define __FAR
#endif

struct datetime {
    /// ��, ��1��ʾ 2001��.
    unsigned char year;
    /// ��, 1-12.
    unsigned char month;
    /// ��, 1-31.
    unsigned char day;
    /// ����, 0-6.
    unsigned char weekday;
    /// ʱ, 0-23.
    unsigned char hour;
    /// ��, 0-59.
    unsigned char minute;
    /// ��, 0-59.
    unsigned char second;
};

typedef unsigned long second_t;

char second_to_datetime(struct datetime *__FAR datetime, second_t second);
char datetime_to_second(second_t *__FAR seconds, const struct datetime *__FAR datetime);
char datetime_is_valid(const struct datetime *__FAR datetime, char need_check_weekday);

#endif
