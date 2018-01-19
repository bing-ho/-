#ifndef _BMS_TIME_H_
#define _BMS_TIME_H_

#include <time.h>

#define _DAY_SEC           (24L * 60L * 60L)    /* secs in a day */

#define _YEAR_SEC          (365L * _DAY_SEC)    /* secs in a year */

#define _FOUR_YEAR_SEC     (1461L * _DAY_SEC)   /* secs in a 4 year interval */

#define _DEC_SEC           315532800L           /* secs in 1970-1979 */

#define _BASE_YEAR         70L                  /* 1970 is the base year */

#define _BASE_DOW          4                    /* 01-01-70 was a Thursday */

#define _LEAP_YEAR_ADJUST  17L                  /* Leap years 1900 - 1970 */

#define _MAX_YEAR          138L                 /* 2038 is the max year */

struct tm * date_time_from_time_t(const time_t *timp);
time_t date_time_to_time_t(struct tm *tb);
void date_time_set_default(struct tm *tb);

#endif