/**
*
* Copyright (c) 2012 Ligoo Inc.
*
* @file bms_clock.h
* @brief
* @note
* @author
* @date 2012-5-9
*
*/

#ifndef BMS_CLOCK_H_
#define BMS_CLOCK_H_
#include "bms_defs.h"
#include "bms_system.h"
#include "bms_util.h"
#include "bms_time.h"
#include "bms_base_cfg.h"
#include "second_datetime.h"
#include "rtc_hardware.h"

typedef struct datetime DateTime;


#ifndef BMS_SUPPORT_CLOCK

#define BMS_CLOCK_IIC   1
#define BMS_CLOCK_I2C   2

#define BMS_SUPPORT_CLOCK       BMS_CLOCK_IIC

#endif

/**
 *
 */
/**
 *
 */
typedef enum
{
    kDateTimeYearBit = 0, //!< kDateTimeYearBit
    kDateTimeMonthBit = 1,//!< kDateTimeMonthBit
    kDateTimeDayBit = 2,  //!< kDateTimeDayBit
    kDateTimeWeekBit = 3,     //!< kDateTimeWeekBit
    kDateTimeHourBit = 4,     //!< kDateTimeHourBit
    kDateTimeMinuteBit = 5,   //!< kDateTimeMinuteBit
    kDateTimeSecondBit = 6,   //!< kDateTimeSecondBit
    kDateTimeMaxBit
}DateTimeFlag;


/**********************************************
 *
 * Time Range
 *
 ***********************************************/
#define CLOCK_YEAR_MIN  0
#define CLOCK_YEAR_MAX  99

#define CLOCK_MONTH_MIN 1
#define CLOCK_MONTH_MAX 12

#define CLOCK_WEEK_MIN  0
#define CLOCK_WEEK_MAX  6

#define CLOCK_DAY_MIN   1
#define CLOCK_DAY_MAX   31

#define CLOCK_HOUR_MIN  0
#define CLOCK_HOUR_MAX  23

#define CLOCK_MINUTE_MIN    0
#define CLOCK_MINUTE_MAX    59

#define CLOCK_SECOND_MIN    0
#define CLOCK_SECOND_MAX    59

#define CLOCK_YEAR_TO_SHORT(YEAR) (YEAR - 2000)
#define CLOCK_YEAR_TO_LONG(YEAR) (YEAR + 2000)

/**
 *
 */
void clock_init(void);

/**
 *
 */
void clock_uninit(void);


/**
 *
 * @param time
 * @return
 */
Result clock_set(DateTime* time);

/**
 *
 * @param time
 * @return
 */
Result clock_get(DateTime* time);

/**
 *
 * @param flag
 * @param value
 * @return
 */
Result clock_set_item(DateTimeFlag flag, ubyte value);

/**
 *
 * @param flag
 * @return
 */
ubyte clock_get_item(DateTimeFlag flag);

void clock_to_tm(DateTime* time, struct tm* tm_t);

void clock_from_tm(DateTime* time, struct tm* tm_t);

long clock_to_time_t(DateTime* time);

void clock_from_time_t(unsigned long second, DateTime* time);

Result clock_format_string(const DateTime* time, char* output);

Result clock_aquire_resource(INT16U t);
void clock_release_resource(void);

Result clock_get_seconds(unsigned long *seconds);
Result clock_config_interrupt(rtc_interrupt_type int_type, rtc_interrupt_single int_type_single) ;
Result clock_get_alarm_time(unsigned long *seconds) ;
Result clock_set_alarm( uint32_t seconds) ;
Result clock_clear_interrupt(void) ;
uint16_t clock_get_ram_size_bits();

Result clock_store_data(const unsigned char *__FAR dat,
                        unsigned char offset,
                        unsigned char len) ;

Result clock_restore_data(unsigned char *__FAR dat,
                            unsigned char offset,
                            unsigned char len) ;
#endif /* BMS_CLOCK_H_ */
