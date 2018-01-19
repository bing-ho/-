#include "second_datetime.h"

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))
#endif

#define inline

#define SECONDS_PER_MINUTE ((second_t)60)
#define SECONDS_PER_HOUR (SECONDS_PER_MINUTE * 60)
#define SECONDS_PER_DAY (SECONDS_PER_HOUR * 24)
#define SECONDS_PER_WEEK (SECONDS_PER_DAY * 7)
#define SECONDS_PER_COM_YEAR (SECONDS_PER_DAY * 365)
#define SECONDS_PER_LEAP_YEAR (SECONDS_PER_DAY * 366)
#define SECONDS_PER_FOUR_YEARS (SECONDS_PER_LEAP_YEAR + SECONDS_PER_COM_YEAR*3)


static inline char __isLeapYear(const struct datetime *__FAR datetime) {
    return (datetime->year % 4) == 0;
}

static const second_t __fourYearSecondsTable[] = {
    0,
    SECONDS_PER_LEAP_YEAR,
    SECONDS_PER_LEAP_YEAR + SECONDS_PER_COM_YEAR * 1,
    SECONDS_PER_LEAP_YEAR + SECONDS_PER_COM_YEAR * 2,
};

static inline second_t __calYear(struct datetime *__FAR datetime, second_t second) {
    int i;

    datetime->year = (second / SECONDS_PER_FOUR_YEARS) * 4;
    second = second % SECONDS_PER_FOUR_YEARS; // seconds left for 4 year;
    for (i = 0; i < ARRAY_SIZE(__fourYearSecondsTable); ++i) {
        if (second < __fourYearSecondsTable[i])
            break;
    }
    --i;
    datetime->year += i;
    return second - __fourYearSecondsTable[i];
}



static inline second_t __yearToSeconds(const struct datetime *__FAR datetime) {
    return (datetime->year / 4) * SECONDS_PER_FOUR_YEARS + __fourYearSecondsTable[datetime->year % 4];
}


static const unsigned short __comYearMonthDaysNumAccTable[] = {
    0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334
};
static const unsigned short __leapYearMonthDaysNumAccTable[] = {
    0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335
};

static inline void __calMonthDate(struct datetime *__FAR datetime, int dayOffsetOfYear) {
    char i;
    const unsigned short *table;
    table = __isLeapYear(datetime) ? __leapYearMonthDaysNumAccTable : __comYearMonthDaysNumAccTable;

    for (i = 1; i < 12; ++i) {
        if (dayOffsetOfYear < table[i])
            break;
    }
    datetime->month = i;
    datetime->day = dayOffsetOfYear - table[i - 1] + 1;
}

static inline second_t __monthDateToSeconds(const struct datetime *__FAR datetime) {
    const unsigned short *table;
    table = __isLeapYear(datetime) ? __leapYearMonthDaysNumAccTable : __comYearMonthDaysNumAccTable;

    return (table[datetime->month - 1] + datetime->day - 1) * SECONDS_PER_DAY;
}


static inline void __calWeekDay(struct datetime *__FAR datetime, second_t second) {
    char day;
    second = (second % SECONDS_PER_WEEK);
    day = (char)((second_t)(second / SECONDS_PER_DAY));
    day += 5; // 2000-1-1ÊÇÐÇÆÚÁù
    if (day >= 7) {
        datetime->weekday = day - 7;
    } else {
        datetime->weekday = day;
    }
}

static inline void __calTime(struct datetime *__FAR datetime, second_t second) {
    second = second % SECONDS_PER_DAY;
    datetime->hour = (char)((second_t)(second / SECONDS_PER_HOUR));
    second = second % SECONDS_PER_HOUR; // seconds left for one hour;
    datetime->minute = (char)((second_t)(second / SECONDS_PER_MINUTE));
    datetime->second = (char)((second_t)(second % SECONDS_PER_MINUTE));
}

static inline second_t __timeToSeconds(const struct datetime *__FAR datetime) {
    return datetime->hour * SECONDS_PER_HOUR + datetime->minute * SECONDS_PER_MINUTE + datetime->second;
}

static char __isValidDateTime(const struct datetime *__FAR datetime) {
    return (datetime->month > 0 &&
            datetime->month <= 12 &&
            datetime->day > 0 &&
            datetime->day <= 31 &&
            datetime->hour < 24 &&
            datetime->minute <= 59 &&
            datetime->second <= 59);
}

static second_t __toSeconds(const struct datetime *__FAR datetime) {
    return __yearToSeconds(datetime) + __monthDateToSeconds(datetime) + __timeToSeconds(datetime);
}

char second_to_datetime(struct datetime *__FAR datetime, second_t second) {
    if (!datetime)
        return 0;
    __calWeekDay(datetime, second);
    second = __calYear(datetime, second);
    __calMonthDate(datetime, second / SECONDS_PER_DAY);
    __calTime(datetime, second);
    return 1;
}

char datetime_to_second(second_t *__FAR seconds, const struct datetime *__FAR datetime) {
    if (!seconds || !datetime)
        return 0;
    if (!__isValidDateTime(datetime))
        return 0;
    *seconds = __toSeconds(datetime);
    return 1;
}

char datetime_is_valid(const struct datetime *__FAR datetime, char need_check_weekday) {
    if (!__isValidDateTime(datetime)) return 0;

    if (!need_check_weekday) return 1;

    if (datetime->weekday > 6) return 0;

    {
        struct datetime t;
        (void)second_to_datetime(&t, __toSeconds(datetime));
        return t.weekday == datetime->weekday;
    }
}

