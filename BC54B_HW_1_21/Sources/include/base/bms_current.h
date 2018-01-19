#ifndef _BMS_CURRENT_H__
#define _BMS_CURRENT_H__

#include "includes.h"
#include "bms_unit.h"
#include "bms_log.h"
#include "ads8317.h"
#include "adc.h"
#include "bms_board_temperature.h"

#ifndef BMS_SUPPORT_CURRENT
#define BMS_SUPPORT_CURRENT     1
#endif

typedef void (*current_auto_check_callback)(void* data);


typedef enum
{
    kCurrentTypeNone = 0,
    kCurrent100Type = 100,
    kCurrentTypeStart = kCurrent100Type,
    kCurrent150Type = 150,
    kCurrent200Type = 200,
    kCurrent300Type = 300,
    kCurrent350Type = 350,
    kCurrent500Type = 500,
    kCurrent600Type = 600,
    kCurrent800Type = 800,
    kCurrentTypeStop = kCurrent800Type
}CURRENT_TYPE;

typedef enum
{
    kCurrentRangeTypeSingle = 0,
    kCurrentRangeTypeDouble,
    kCurrentRangeTypeTwoSingle
}CURRENT_RANGE_TYPE;

typedef struct 
{
  INT16U ad_high_value;
  INT16U ad_low_value;
  INT16U ad_single_value;
  //INT16S* _PAGED current;
  //INT16S* _PAGED current1;
  INT16S calibration_value;
  INT16S high_calib_value;
  INT16U cur_zero_filter;
}Current_platform;


Result current_sample_init(INT16U sample_peroid, INT8U sample_count, INT16S* _PAGED low_buffer, INT16S* _PAGED high_buffer,INT16S* _PAGED single_buffer,
        INT8U min_sample_count, INT8U filter_num, INT16U range, INT16U range1,CURRENT_RANGE_TYPE range_type);
void current_sample_uninit(void);

/**
 * 对电流进行采样
 * @param current 返回电流值
 * @param calibration_value 校准值
 * @return 如果成功为0
 */

//Result current_sample_get(INT16U ad_high_value, INT16U ad_low_value, INT16U ad_single_value, INT16S* _PAGED current,INT16S* _PAGED current1, INT16S calibration_value, INT16S high_calib_value, INT16U CurZeroFilter_value);
void bcu_check_current_auto_to_zero_self(void);//Result current_sample_auto_check(current_auto_check_callback auto_check_callback);
//Result  current_sample_auto_to_zero(current_auto_check_callback auto_check_callback);
INT8U current_signal_is_valid(void);
INT8U current1_signal_is_valid(void);
Result get_sample_current(Current_platform platform);
//Result get_sample_current(Current_platform platform); 

#endif

