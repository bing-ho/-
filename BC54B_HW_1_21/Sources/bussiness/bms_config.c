/**
 *
 * Copyright (c) 2012 Ligoo Inc.
 *
 * @file bms_config.c
 * @brief
 * @note
 * @author
 * @date 2012-5-9
 *
 */
#include "bms_config.h"
#include "main_bsu_relay_ctrl.h"

#pragma MESSAGE DISABLE C1420 // Result of function-call is ignored
#pragma MESSAGE DISABLE C1860 // Pointer conversion: possible loss of data
#pragma MESSAGE DISABLE C4000 // Condition always TRUE
#pragma MESSAGE DISABLE C4200 // Other segment than in previous declaration

#pragma CONST_SEG  DEFAULT

#pragma CONST_SEG  DEFAULT

#define CONFIG_START_SAVE_END_CHECK()   g_bms_config_save_last_tick = get_tick_count()
#define CONFIG_STOP_SAVE_END_CHECK()    g_bms_config_save_last_tick = 0

typedef struct _ConfigObserver
{
    ConfigIndex start_index;
    ConfigIndex end_index;
    ConfigChangingHandler handler;
}ConfigObserver;

#define CONFIG_OBSERVER_MAX_COUNT 10

//static lock_t         g_config_lock = NULL;

#pragma push
#pragma DATA_SEG __RPAGE_SEG PAGED_RAM_CONFIG

const ConfigAttribute g_config_attribute[kSysParamMaxCount]=
{
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      EEEPROM_FORMAT_FLAG_DEF,            0,                                  1,                                  //kEepromFormated
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kUserProgramUsedFlagIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      kBcuBC5xB,                          0,                                  0,                                  //kDeviceTypeIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kDeviceNumIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      DEVICE_SW_MAJOR_VER,                0,                                  0,                                  //kUserProgramMajorVerIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      DEVICE_SW_MINOR_VER,                0,                                  0,                                  //kUserProgramMinorVerIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      DEVICE_SW_REV_VER,                  0,                                  0,                                  //kUserProgramRevVerIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      kModeNotUpdate,                     0,                                  0,                                  //kBootloaderUpdateModeIndex

    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kBootloaderParamEnd
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kUUIDPart1
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kUUIDPart2
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kUUIDPart3
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kUUIDPart4
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kUUIDPart5
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kUUIDPart6
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kUUIDPart7
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kUUIDPart8
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kProductDatePart1
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kProductDatePart2
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       ISENSOR_TYPE_DEF,                   ISENSOR_TYPE_MIN_DEF,               ISENSOR_TYPE_MAX_DEF,               //kCurSenTypeIndex
    CFG_SIGN_PARA+CFG_CHK_RANGE_ON,         CCHK_DEF,                           CCHK_MIN_DEF,                       CCHK_MAX_DEF,                       //kCchkIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_HV_FST_ALARM_DEF,              SYS_DEFAULT_VOLT_MIN_DEF,           SYS_DEFAULT_VOLT_MAX_DEF,           //kDChgHVFstAlarmIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_HV_FST_ALARM_REL_DEF,          SYS_DEFAULT_VOLT_MIN_DEF,           SYS_DEFAULT_VOLT_MAX_DEF,           //kDChgHVFstAlarmRelIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_HV_SND_ALARM_DEF,              SYS_DEFAULT_VOLT_MIN_DEF,           SYS_DEFAULT_VOLT_MAX_DEF,           //kDChgHVSndAlarmIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_HV_SND_ALARM_REL_DEF,          SYS_DEFAULT_VOLT_MIN_DEF,           SYS_DEFAULT_VOLT_MAX_DEF,           //kDChgHVSndAlarmRelIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_HV_FST_ALARM_DLY_DEF,          SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kDChgHVFstAlarmDlyIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_HV_FST_ALARM_REL_DLY_DEF,      SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kDChgHVFstAlarmRelDlyIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_HV_SND_ALARM_DLY_DEF,          SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kDChgHVSndAlarmDlyIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_HV_SND_ALARM_REL_DLY_DEF,      SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kDChgHVSndAlarmRelDlyIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_LV_FST_ALARM_DEF,              SYS_DEFAULT_VOLT_MIN_DEF,           SYS_DEFAULT_VOLT_MAX_DEF,           //kDChgLVFstAlarmIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_LV_FST_ALARM_REL_DEF,          SYS_DEFAULT_VOLT_MIN_DEF,           SYS_DEFAULT_VOLT_MAX_DEF,           //kDChgLVFstAlarmRelIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_LV_SND_ALARM_DEF,              SYS_DEFAULT_VOLT_MIN_DEF,           SYS_DEFAULT_VOLT_MAX_DEF,           //kDChgLVSndAlarmIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_LV_SND_ALARM_REL_DEF,          SYS_DEFAULT_VOLT_MIN_DEF,           SYS_DEFAULT_VOLT_MAX_DEF,           //kDChgLVSndAlarmRelIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_LV_FST_ALARM_DLY_DEF,          SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kDChgLVFstAlarmDlyIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_LV_FST_ALARM_REL_DLY_DEF,      SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kDChgLVFstAlarmRelDlyIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_LV_SND_ALARM_DLY_DEF,          SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kDChgLVSndAlarmDlyIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_LV_SND_ALARM_REL_DLY_DEF,      SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kDChgLVSndAlarmRelDlyIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_DV_FST_ALARM_DEF,              SYS_DEFAULT_DV_MIN_DEF,             SYS_DEFAULT_DV_MAX_DEF,             //kDChgDVFstAlarmIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_DV_FST_ALARM_REL_DEF,          SYS_DEFAULT_DV_MIN_DEF,             SYS_DEFAULT_DV_MAX_DEF,             //kDChgDVFstAlarmRelIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_DV_SND_ALARM_DEF,              SYS_DEFAULT_DV_MIN_DEF,             SYS_DEFAULT_DV_MAX_DEF,             //kDChgDVSndAlarmIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_DV_SND_ALARM_REL_DEF,          SYS_DEFAULT_DV_MIN_DEF,             SYS_DEFAULT_DV_MAX_DEF,             //kDChgDVSndAlarmRelIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_DV_FST_ALARM_DLY_DEF,          SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kDChgDVFstAlarmDlyIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_DV_FST_ALARM_REL_DLY_DEF,      SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kDChgDVFstAlarmRelDlyIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_DV_SND_ALARM_DLY_DEF,          SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kDChgDVSndAlarmDlyIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_DV_SND_ALARM_REL_DLY_DEF,      SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kDChgDVSndAlarmRelDlyIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       VOLT_LINE_FST_ALARM_DLY_DEF,        SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kVLineFstAlarmDlyIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       VOLT_LINE_FST_ALARM_REL_DLY_DEF,    SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kVLineFstAlarmRelDlyIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_HT_FST_ALARM_DEF,              SYS_DEFAULT_TEMP_MIN_DEF,           SYS_DEFAULT_TEMP_MAX_DEF,           //kDChgHTFstAlarmIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_HT_FST_ALARM_REL_DEF,          SYS_DEFAULT_TEMP_MIN_DEF,           SYS_DEFAULT_TEMP_MAX_DEF,           //kDChgHTFstAlarmRelIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_HT_SND_ALARM_DEF,              SYS_DEFAULT_TEMP_MIN_DEF,           SYS_DEFAULT_TEMP_MAX_DEF,           //kDChgHTSndAlarmIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_HT_SND_ALARM_REL_DEF,          SYS_DEFAULT_TEMP_MIN_DEF,           SYS_DEFAULT_TEMP_MAX_DEF,           //kDChgHTSndAlarmRelIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_HT_FST_ALARM_DLY_DEF,          SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kDChgHTFstAlarmDlyIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_HT_FST_ALARM_REL_DLY_DEF,      SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kDChgHTFstAlarmRelDlyIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_HT_SND_ALARM_DLY_DEF,          SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kDChgHTSndAlarmDlyIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_HT_SND_ALARM_REL_DLY_DEF,      SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kDChgHTSndAlarmRelDlyIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_LT_FST_ALARM_DEF,              SYS_DEFAULT_TEMP_MIN_DEF,           SYS_DEFAULT_TEMP_MAX_DEF,           //kDChgLTFstAlarmIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_LT_FST_ALARM_REL_DEF,          SYS_DEFAULT_TEMP_MIN_DEF,           SYS_DEFAULT_TEMP_MAX_DEF,           //kDChgLTFstAlarmRelIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_LT_SND_ALARM_DEF,              SYS_DEFAULT_TEMP_MIN_DEF,           SYS_DEFAULT_TEMP_MAX_DEF,           //kDChgLTSndAlarmIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_LT_SND_ALARM_REL_DEF,          SYS_DEFAULT_TEMP_MIN_DEF,           SYS_DEFAULT_TEMP_MAX_DEF,           //kDChgLTSndAlarmRelIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_LT_FST_ALARM_DLY_DEF,          SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kDChgLTFstAlarmDlyIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_LT_FST_ALARM_REL_DLY_DEF,      SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kDChgLTFstAlarmRelDlyIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_LT_SND_ALARM_DLY_DEF,          SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kDChgLTSndAlarmDlyIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_LT_SND_ALARM_REL_DLY_DEF,      SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kDChgLTSndAlarmRelDlyIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_DT_FST_ALARM_DEF,              SYS_DEFAULT_DT_MIN_DEF,             SYS_DEFAULT_DT_MAX_DEF,             //kDChgDTFstAlarmIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_DT_FST_ALARM_REL_DEF,          SYS_DEFAULT_DT_MIN_DEF,             SYS_DEFAULT_DT_MAX_DEF,             //kDChgDTFstAlarmRelIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_DT_SND_ALARM_DEF,              SYS_DEFAULT_DT_MIN_DEF,             SYS_DEFAULT_DT_MAX_DEF,             //kDChgDTSndAlarmIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_DT_SND_ALARM_REL_DEF,          SYS_DEFAULT_DT_MIN_DEF,             SYS_DEFAULT_DT_MAX_DEF,             //kDChgDTSndAlarmRelIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_DT_FST_ALARM_DLY_DEF,          SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kDChgDTFstAlarmIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_DT_FST_ALARM_REL_DLY_DEF,      SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kDChgDTFstAlarmRelDlyIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_DT_SND_ALARM_DLY_DEF,          SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kDChgDTSndAlarmIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_DT_SND_ALARM_REL_DLY_DEF,      SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kDChgDTSndAlarmRelDlyIndex
#if 0//!REMOVE_NO_USED_IDX_TO_SAVE_EEPROM
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       TEMP_RISE_FAST_DEF,                 TEMP_RISE_FAST_MIN_DEF,             TEMP_RISE_FAST_MAX_DEF,             //kRiseTempFstAlarmIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       TEMP_RISE_FAST_FREE_DEF,            TEMP_RISE_FAST_FREE_MIN_DEF,        TEMP_RISE_FAST_FREE_MAX_DEF,        //kRiseTempFstAlarmRelIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       TEMP_RISE_OVER_DEF,                 TEMP_RISE_OVER_MIN_DEF,             TEMP_RISE_OVER_MAX_DEF,             //kRiseTempSndAlarmIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       TEMP_RISE_OVER_FREE_DEF,            TEMP_RISE_OVER_FREE_MIN_DEF,        TEMP_RISE_OVER_FREE_MAX_DEF,        //kRiseTempSndAlarmRelIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       TEMP_RISE_FAST_TIME_DEF,            SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kRiseTempFstAlarmTimeIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       TEMP_RISE_FAST_FREE_TIME_DEF,       SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kRiseTempFstAlarmRelTimeIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       TEMP_RISE_OVER_TIME_DEF,            SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kRiseTempSndAlarmTimeIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       TEMP_RISE_OVER_FREE_TIME_DEF,       SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kRiseTempSndAlarmRelTimeIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       TEMP_FALL_FAST_DEF,                 TEMP_FALL_FAST_MIN_DEF,             TEMP_FALL_FAST_MAX_DEF,             //kFallTempFstAlarmIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       TEMP_FALL_FAST_FREE_DEF,            TEMP_FALL_FAST_FREE_MIN_DEF,        TEMP_FALL_FAST_FREE_MAX_DEF,        //kFallTempFstAlarmRelIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       TEMP_FALL_OVER_DEF,                 TEMP_FALL_OVER_MIN_DEF,             TEMP_FALL_OVER_MAX_DEF,             //kFallTempSndAlarmIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       TEMP_FALL_OVER_FREE_DEF,            TEMP_FALL_OVER_FREE_MIN_DEF,        TEMP_FALL_OVER_FREE_MAX_DEF,        //kFallTempSndAlarmRelIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       TEMP_FALL_FAST_TIME_DEF,            SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kFallTempFstAlarmTimeIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       TEMP_FALL_FAST_FREE_TIME_DEF,       SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kFallTempFstAlarmTimeRelIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       TEMP_FALL_OVER_TIME_DEF,            SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kFallTempSndAlarmTimeIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       TEMP_FALL_OVER_FREE_TIME_DEF,       SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kFallTempSndAlarmTimeRelIndex
#endif
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       TEMP_LINE_FST_ALARM_DLY_DEF,        SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kTLineFstAlarmDlyIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       TEMP_LINE_FST_ALARM_REL_DLY_DEF,    SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kTLineFstAlarmRelDlyIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_HTV_FST_ALARM_DEF,             SYS_DEFAULT_TV_MIN_DEF,             SYS_DEFAULT_TV_MAX_DEF,             //kDChgHTVFstAlarmIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_HTV_FST_ALARM_REL_DEF,         SYS_DEFAULT_TV_MIN_DEF,             SYS_DEFAULT_TV_MAX_DEF,             //kDChgHTVFstAlarmRelIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_HTV_SND_ALARM_DEF,             SYS_DEFAULT_TV_MIN_DEF,             SYS_DEFAULT_TV_MAX_DEF,             //kDChgHTVSndAlarmIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_HTV_SND_ALARM_REL_DEF,         SYS_DEFAULT_TV_MIN_DEF,             SYS_DEFAULT_TV_MAX_DEF,             //kDChgHTVSndAlarmRelIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_HTV_FST_ALARM_DLY_DEF,         SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kDChgHTVFstAlarmDlyIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_HTV_FST_ALARM_REL_DLY_DEF,     SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kDChgHTVFstAlarmRelDlyIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_HTV_SND_ALARM_DLY_DEF,         SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kDChgHTVSndAlarmDlyIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_HTV_SND_ALARM_REL_DLY_DEF,     SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kDChgHTVSndAlarmRelDlyIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_LTV_FST_ALARM_DEF,             SYS_DEFAULT_TV_MIN_DEF,             SYS_DEFAULT_TV_MAX_DEF,             //kDChgLTVFstAlarmIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_LTV_FST_ALARM_REL_DEF,         SYS_DEFAULT_TV_MIN_DEF,             SYS_DEFAULT_TV_MAX_DEF,             //kDChgLTVFstAlarmRelIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_LTV_SND_ALARM_DEF,             SYS_DEFAULT_TV_MIN_DEF,             SYS_DEFAULT_TV_MAX_DEF,             //kDChgLTVSndAlarmIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_LTV_SND_ALARM_REL_DEF,         SYS_DEFAULT_TV_MIN_DEF,             SYS_DEFAULT_TV_MAX_DEF,             //kDChgLTVSndAlarmRelIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_LTV_FST_ALARM_DLY_DEF,         SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kDChgLTVFstAlarmDlyIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_LTV_FST_ALARM_REL_DLY_DEF,     SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kDChgLTVFstAlarmRelDlyIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_LTV_SND_ALARM_DLY_DEF,         SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kDChgLTVSndAlarmDlyIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_LTV_SND_ALARM_REL_DLY_DEF,     SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kDChgLTVSndAlarmRelDlyIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_OC_FST_ALARM_DEF,               SYS_DEFAULT_CUR_MIN_DEF,            SYS_DEFAULT_CUR_MAX_DEF,            //kChgOCFstAlarmIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_OC_FST_ALARM_REL_DEF,           SYS_DEFAULT_CUR_MIN_DEF,            SYS_DEFAULT_CUR_MAX_DEF,            //kChgOCFstAlarmRelIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_OC_SND_ALARM_DEF,               SYS_DEFAULT_CUR_MIN_DEF,            SYS_DEFAULT_CUR_MAX_DEF,            //kChgOCSndAlarmIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_OC_SND_ALARM_REL_DEF,           SYS_DEFAULT_CUR_MIN_DEF,            SYS_DEFAULT_CUR_MAX_DEF,            //kChgOCSndAlarmRelIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_OC_FST_ALARM_DLY_DEF,           SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kChgOCFstAlarmDlyIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_OC_FST_ALARM_REL_DLY_DEF,       SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kChgOCFstAlarmRelDlyIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_OC_SND_ALARM_DLY_DEF,           SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kChgOCSndAlarmDlyIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_OC_SND_ALARM_REL_DLY_DEF,       SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kChgOCSndAlarmRelDlyIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_OC_FST_ALARM_DEF,              SYS_DEFAULT_CUR_MIN_DEF,            SYS_DEFAULT_CUR_MAX_DEF,            //kDChgOCFstAlarmIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_OC_FST_ALARM_REL_DEF,          SYS_DEFAULT_CUR_MIN_DEF,            SYS_DEFAULT_CUR_MAX_DEF,            //kDChgOCFstAlarmRelIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_OC_SND_ALARM_DEF,              SYS_DEFAULT_CUR_MIN_DEF,            SYS_DEFAULT_CUR_MAX_DEF,            //kDChgOCSndAlarmIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_OC_SND_ALARM_REL_DEF,          SYS_DEFAULT_CUR_MIN_DEF,            SYS_DEFAULT_CUR_MAX_DEF,            //kDChgOCSndAlarmRelIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_OC_FST_ALARM_DLY_DEF,          SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kDChgOCFstAlarmDlyIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_OC_FST_ALARM_REL_DLY_DEF,      SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kDChgOCFstAlarmRelDlyIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_OC_SND_ALARM_DLY_DEF,          SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kDChgOCSndAlarmDlyIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_OC_SND_ALARM_REL_DLY_DEF,      SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kDChgOCSndAlarmRelDlyIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       SOC_HIGH_FST_ALARM_DEF,             SYS_DEFAULT_SOC_MIN_DEF,            SYS_DEFAULT_SOC_MAX_DEF,            //kHSOCFstAlarmIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       SOC_HIGH_FST_ALARM_REL_DEF,         SYS_DEFAULT_SOC_MIN_DEF,            SYS_DEFAULT_SOC_MAX_DEF,            //kHSOCFstAlarmRelIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       SOC_HIGH_SND_ALARM_DEF,             SYS_DEFAULT_SOC_MIN_DEF,            SYS_DEFAULT_SOC_MAX_DEF,            //kHSOCSndAlarmIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       SOC_HIGH_SND_ALARM_REL_DEF,         SYS_DEFAULT_SOC_MIN_DEF,            SYS_DEFAULT_SOC_MAX_DEF,            //kHSOCSndAlarmRelIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       SOC_HIGH_FST_ALARM_DLY_DEF,         SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kHSOCFstAlarmDlyIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       SOC_HIGH_FST_ALARM_REL_DLY_DEF,     SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kHSOCFstAlarmRelDlyIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       SOC_HIGH_SND_ALARM_DLY_DEF,         SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kHSOCSndAlarmDlyIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       SOC_HIGH_SND_ALARM_REL_DLY_DEF,     SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kHSOCSndAlarmRelDlyIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       SOC_LOW_FST_ALARM_DEF,              SYS_DEFAULT_SOC_MIN_DEF,            SYS_DEFAULT_SOC_MAX_DEF,            //kLSOCFstAlarmIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       SOC_LOW_FST_ALARM_REL_DEF,          SYS_DEFAULT_SOC_MIN_DEF,            SYS_DEFAULT_SOC_MAX_DEF,            //kLSOCFstAlarmRelIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       SOC_LOW_SND_ALARM_DEF,              SYS_DEFAULT_SOC_MIN_DEF,            SYS_DEFAULT_SOC_MAX_DEF,            //kLSOCSndAlarmIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       SOC_LOW_SND_ALARM_REL_DEF,          SYS_DEFAULT_SOC_MIN_DEF,            SYS_DEFAULT_SOC_MAX_DEF,            //kLSOCSndAlarmRelIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       SOC_LOW_FST_ALARM_DLY_DEF,          SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kLSOCFstAlarmDlyIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       SOC_LOW_FST_ALARM_REL_DLY_DEF,      SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kLSOCFstAlarmRelDlyIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       SOC_LOW_SND_ALARM_DLY_DEF,          SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kLSOCSndAlarmDlyIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       SOC_LOW_SND_ALARM_REL_DLY_DEF,      SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kLSOCSndAlarmRelDlyIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       PINSU_FST_ALARM_DEF,                SYS_DEFAULT_PINSU_MIN_DEF,          SYS_DEFAULT_PINSU_MAX_DEF,          //kInsuFstAlarmIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       PINSU_FST_ALARM_REL_DEF,            SYS_DEFAULT_PINSU_MIN_DEF,          SYS_DEFAULT_PINSU_MAX_DEF,          //kInsuFstAlarmRelIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       PINSU_SND_ALARM_DEF,                SYS_DEFAULT_PINSU_MIN_DEF,          SYS_DEFAULT_PINSU_MAX_DEF,          //kInsuSndAlarmIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       PINSU_SND_ALARM_REL_DEF,            SYS_DEFAULT_PINSU_MIN_DEF,          SYS_DEFAULT_PINSU_MAX_DEF,          //kInsuSndAlarmRelIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       PINSU_TRD_ALARM_DEF,                SYS_DEFAULT_PINSU_MIN_DEF,          SYS_DEFAULT_PINSU_MAX_DEF,          //kInsuTrdAlarmIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       PINSU_TRD_ALARM_REL_DEF,            SYS_DEFAULT_PINSU_MIN_DEF,          SYS_DEFAULT_PINSU_MAX_DEF,          //kInsuTrdAlarmRelIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       PINSU_FST_ALARM_DLY_DEF,            SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kInsuFstAlarmDlyIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       PINSU_FST_ALARM_REL_DLY_DEF,        SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kInsuFstAlarmRelDlyIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       PINSU_SND_ALARM_DLY_DEF,            SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kInsuSndAlarmDlyIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       PINSU_SND_ALARM_REL_DLY_DEF,        SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kInsuSndAlarmRelDlyIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       PINSU_TRD_ALARM_DLY_DEF,            SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kInsuTrdAlarmDlyIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       PINSU_TRD_ALARM_REL_DLY_DEF,        SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kInsuTrdAlarmRelDlyIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       COMM_FST_ALARM_DLY_DEF,             COMM_ABORT_ALARM_DLY_MIN,           COMM_ABORT_ALARM_DLY_MAX,           //kCommFstAlarmDlyIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       COMM_FST_ALARM_REL_DLY_DEF,         COMM_ABORT_ALARM_REL_DLY_MIN,       COMM_ABORT_ALARM_REL_DLY_MAX,       //kCommFstAlarmRelDlyIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_RELAY_ON_DLY_DEF,               SYS_DEFAULT_DLY_MIN_DEF,            SYS_DEFAULT_DLY_MAX_DEF,            //kChgRelayOnDlyIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_RELAY_OFF_DLY_DEF,              SYS_DEFAULT_DLY_MIN_DEF,            SYS_DEFAULT_DLY_MAX_DEF,            //kChgRelayOffDlyIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DISCHG_RELAY_ON_DLY_DEF,            SYS_DEFAULT_DLY_MIN_DEF,            SYS_DEFAULT_DLY_MAX_DEF,            //kDisChgRelayOnDlyIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DISCHG_RELAY_OFF_DLY_DEF,           SYS_DEFAULT_DLY_MIN_DEF,            SYS_DEFAULT_DLY_MAX_DEF,            //kDisChgRelayOffDlyIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       PRECHG_RELAY_ON_DLY_DEF,            SYS_DEFAULT_DLY_MIN_DEF,            SYS_DEFAULT_DLY_MAX_DEF,            //kPreChgRelayOnDlyIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       PRECHG_RELAY_OFF_DLY_DEF,           SYS_DEFAULT_DLY_MIN_DEF,            SYS_DEFAULT_DLY_MAX_DEF,            //kPreChgRelayOffDlyIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       RESERVE_RELAY_ON_DLY_DEF,           SYS_DEFAULT_DLY_MIN_DEF,            SYS_DEFAULT_DLY_MAX_DEF,            //kReserveRelayOnDlyIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       RESERVE_RELAY_OFF_DLY_DEF,          SYS_DEFAULT_DLY_MIN_DEF,            SYS_DEFAULT_DLY_MAX_DEF,            //kReserveRelayOffDlyIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHARGER_CUR,                        CHARGER_CUR_MIN_DEF,                CHARGER_CUR_MAX_DEF,                //kChargerCurIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       NOMINAL_CAP_DEF,                    CAP_MIN_DEF,                        NOMINAL_CAP_MAX_DEF,                //kNominalCapIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       TOTALCAP_DEF,                       CAP_MIN_DEF,                        CAP_MAX_DEF,                        //kTotalCapIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       SAVE_FILE_A_FLAG_DEF,               0,                                  1,                                  //kSaveFileAFlag
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       BMS_CHARGER_MAX_VOLTAGE,            BMS_CHARGER_MAX_VOLTAGE_MIN,        BMS_CHARGER_MAX_VOLTAGE_MAX,        //kChargerVoltIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       BMS_CHARGER_PROTOCOL_DEFAULT,       BMS_CHARGER_PROTOCOL_MIN_DEF,       BMS_CHARGER_PROTOCOL_MAX_DEF,       //kChargerProtocolIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       SLAVE_NUM_DEF,                      SLAVE_NUM_MIN_DEF,                  SLAVE_NUM_MAX_DEF,                  //kSlaveNumIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       SLAVE_TEMPERA_NUM_DEF,              SLAVE_TEMPERA_NUM_MIN_DEF,          SLAVE_TEMPERA_NUM_MAX_DEF,          //kTemperatureNumIndex
#if 0//!REMOVE_NO_USED_IDX_TO_SAVE_EEPROM
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      MOTO_VICHLE_TYRE_DIAMETER,          0,                                  0,                                  //kMotorTyremm
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      MOTO_SPEED_RATE,                    0,                                  0,                                  //kMotorSpeedRate
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      MOTO_GEAR_RATE,                     0,                                  0,                                  //kMotorGearRate
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      MOTO_TYRE_RATE,                     0,                                  0,                                  //kMotorTyreRate
#endif
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      CFG_SYS_VOLT_HIGH_DEF,              CFG_SYS_VOLT_HIGH_MIN_DEF,          CFG_SYS_VOLT_HIGH_MAX_DEF,          //kSystemVoltHVIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CFG_SYS_VOLT_HIGH_DEF,              CFG_SYS_VOLT_HIGH_MIN_DEF,          CFG_SYS_VOLT_HIGH_MAX_DEF,          //kSystemVoltHVReleaseIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CFG_SYS_VOLT_LOW_DEF,               CFG_SYS_VOLT_LOW_MIN_DEF,           CFG_SYS_VOLT_LOW_MAX_DEF,           //kSystemVoltLVIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CFG_SYS_VOLT_LOW_DEF,               CFG_SYS_VOLT_LOW_MIN_DEF,           CFG_SYS_VOLT_LOW_MAX_DEF,           //kSystemVoltLVReleaseIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CFG_SYS_VOLT_HIGH_DLY_DEF,          CFG_SYS_VOLT_HIGH_DLY_MIN_DEF,      CFG_SYS_VOLT_HIGH_DLY_MAX_DEF,      //kSystemVoltHVDlyIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CFG_SYS_VOLT_HIGH_DLY_DEF,          CFG_SYS_VOLT_HIGH_DLY_MIN_DEF,      CFG_SYS_VOLT_HIGH_DLY_MAX_DEF,      //kSystemVoltHVReleaseDlyIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CFG_SYS_VOLT_LOW_DLY_DEF,           CFG_SYS_VOLT_LOW_DLY_MIN_DEF,       CFG_SYS_VOLT_LOW_DLY_MAX_DEF,       //kSystemVoltLVDlyIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CFG_SYS_VOLT_LOW_DLY_DEF,           CFG_SYS_VOLT_LOW_DLY_MIN_DEF,       CFG_SYS_VOLT_LOW_DLY_MAX_DEF,       //kSystemVoltLVReleaseDlyIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CFG_SYS_VOLT_CALIB_DEF,             CFG_SYS_VOLT_CALIB_MIN_DEF,         CFG_SYS_VOLT_CALIB_MAX_DEF,         //kSystemVoltageCalibration
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CFG_HEAT_ON_DEF,                    CFG_HEAT_ON_MIN_DEF,                CFG_HEAT_ON_MAX_DEF,                //kHeatOnTemperature
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CFG_HEAT_OFF_DEF,                   CFG_HEAT_OFF_MIN_DEF,               CFG_HEAT_OFF_MAX_DEF,               //kHeatOffTemperature
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CFG_COOL_ON_DEF,                    CFG_COOL_ON_MIN_DEF,                CFG_COOL_ON_MAX_DEF,                //kCoolOnTemperature
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CFG_COOL_OFF_DEF,                   CFG_COOL_OFF_MIN_DEF,               CFG_COOL_OFF_MAX_DEF,               //kCoolOffTemperature
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      CFG_HEAT_ON_DLY_DEF,                0,                                  0,                                  //kHeatOnTempDlyIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      CFG_HEAT_OFF_DLY_DEF,               0,                                  0,                                  //kHeatOffTempDlyIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      CFG_COOL_ON_DLY_DEF,                0,                                  0,                                  //kCoolOnTempDlyIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      CFG_COOL_OFF_DLY_DEF,               0,                                  0,                                  //kCoolOffTempDlyIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CAP_CALIB_HIGH_AVER_VOLT_DEF,       CAP_CALIB_HIGH_AVER_VOLT_MIN_DEF,   CAP_CALIB_HIGH_AVER_VOLT_MAX_DEF,   //kCapCalibHighAverageVoltage
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CAP_CALIB_LOW_AVER_VOLT_DEF,        CAP_CALIB_LOW_AVER_VOLT_MIN_DEF,    CAP_CALIB_LOW_AVER_VOLT_MAX_DEF,    //kCapCalibLowAverageVoltage
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CAP_CALIB_LOW_SOC_DEF,              CAP_CALIB_LOW_SOC_MIN_DEF,          CAP_CALIB_LOW_SOC_MAX_DEF,          //kCapCalibLowSoc
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       LOW_CALIBRATE_SOC_DLY_DEF,          LOW_CALIBRATE_SOC_DLY_MIN,          LOW_CALIBRATE_SOC_DLY_MAX,          //kCapCalibLowSocDly
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       HMI_FRAME_COMM_ABORT_TIMEOUT,       HMI_FRAME_COMM_ABORT_TIMEOUT_MIN,   HMI_FRAME_COMM_ABORT_TIMEOUT_MAX,   //kHmiFrameTimeoutIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       HMI_COMM_BPS_DEF,                   RS485_BPS_MIN_DEF,                  RS485_BPS_MAX_DEF,                  //kHmiCommBpsIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DTU_SEND_INTERVAL_DEF,              DTU_SEND_INTERVAL_MIN_DEF,          DTU_SEND_INTERVAL_MAX_DEF,          //kDTUSendIntervalIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       UPPER_COMPUTER_CAN_ADDR_DEF,        UPPER_COMPUTER_CAN_ADDR_MIN,        UPPER_COMPUTER_CAN_ADDR_MAX,        //kUpperComputerCanAddrIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       BCU_CAN_ADDR_DEF,                   BCU_CAN_ADDR_MIN_DEF,               BCU_CAN_ADDR_MAX_DEF,               //kBcuCanAddrIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       BMS_SLAVE_CAN_QUERY_SLAVE_DELAY,    BMS_SLAVE_CAN_QUERY_SLAVE_DLY_MIN,  BMS_SLAVE_CAN_QUERY_SLAVE_DLY_MAX,  //kReqBmuMessageIntervalIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       BMS_SLAVE_CAN_RECEIVE_RESPONSE_TIMEOUT, BMS_SLAVE_CAN_RECEIVE_RESPONSE_TIMEOUT_MIN,BMS_SLAVE_CAN_RECEIVE_RESPONSE_TIMEOUT_MAX,    //kBmuResponseTimeoutIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       SOC_CURRENT_FILTER_MICRO_CURRENT,   SOC_CURRENT_FILTER_MICRO_CURRENT_MIN, SOC_CURRENT_FILTER_MICRO_CURRENT_MAX,    //kCurZeroFilterIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       SOC_CURRENT_SAMPLE_PERIOD,          SOC_CURRENT_SAMPLE_PERIOD_MIN,      SOC_CURRENT_SAMPLE_PERIOD_MAX,      //kCurSampleintervalIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       SOC_CURRENT_SAMPLE_COUNT,           SOC_CURRENT_SAMPLE_COUNT_MIN,       SOC_CURRENT_SAMPLE_COUNT_MAX,       //kCurSampleCntIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       SYS_STATUS_SAVE_INTERVAL_DEF,       SYS_STATUS_SAVE_INTERVAL_MIN_DEF,   SYS_STATUS_SAVE_INTERVAL_MAX_DEF,   //kSysStatusSaveIntervalIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       SYSTEM_STATE_AVAILABLE_DELAY_DEF,   SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kSysStateAvailableDlyIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       BMU_BALANCE_ENABLE_DEF,             BMU_BALANCE_ENABLE_MIN_DEF,         BMU_BALANCE_ENABLE_MAX_DEF,         //kBmuBalanceEnableIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       MCU_WORK_STATE_DELAY_DEF,           SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kWorkStateDlyIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       ALARM_BEEP_INTERVAL_DEF,            ALARM_BEEP_INTERVAL_MIN_DEF,        ALARM_BEEP_INTERVAL_MAX_DEF,        //kAlarmBeepInterval
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       ALARM_BEEP_MASK_HIGH_DEF,           ALARM_BEEP_MASK_HIGH_MIN_DEF,       ALARM_BEEP_MASK_HIGH_MAX_DEF,       //kAlarmBeepMaskHigh
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       ALARM_BEEP_MASK_LOW_DEF,            ALARM_BEEP_MASK_LOW_MIN_DEF,        ALARM_BEEP_MASK_LOW_MAX_DEF,        //kAlarmBeepMaskLow
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       RELAY_DISCHARGE_SAME_PORT_DEF,      RELAY_DISCHARGE_SAME_PORT_MIN_DEF,  RELAY_DISCHARGE_SAME_PORT_MAX_DEF,  //kRelayDischargeSamePort
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       RELAY_CHARGE_MUTEX_DEF,             RELAY_CHARGE_MUTEX_MIN_DEF,         RELAY_CHARGE_MUTEX_MAX_DEF,         //kRelayChargeMutex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       RELAY_PRECHARGE_DEF,                RELAY_PRECHARGE_MIN_DEF,            RELAY_PRECHARGE_MAX_DEF,            //kRelayPreCharge
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       RELAY_RELAY1_TYPE_DEF,              RELAY_TYPE_MIN_DEF,                 RELAY_TYPE_MAX_DEF,                 //kCfgRelay1Type
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       RELAY_RELAY2_TYPE_DEF,              RELAY_TYPE_MIN_DEF,                 RELAY_TYPE_MAX_DEF,                 //kCfgRelay2Type
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       RELAY_RELAY3_TYPE_DEF,              RELAY_TYPE_MIN_DEF,                 RELAY_TYPE_MAX_DEF,                 //kCfgRelay3Type
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       RELAY_RELAY4_TYPE_DEF,              RELAY_TYPE_MIN_DEF,                 RELAY_TYPE_MAX_DEF,                 //kCfgRelay4Type
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       RELAY_RELAY5_TYPE_DEF,              RELAY_TYPE_MIN_DEF,                 RELAY_TYPE_MAX_DEF,                 //kCfgRelay5Type
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       RELAY_RELAY6_TYPE_DEF,              RELAY_TYPE_MIN_DEF,                 RELAY_TYPE_MAX_DEF,                 //kCfgRelay6Type
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       RELAY_RELAY7_TYPE_DEF,              RELAY_TYPE_MIN_DEF,                 RELAY_TYPE_MAX_DEF,                 //kCfgRelay7Type
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       RELAY_RELAY8_TYPE_DEF,              RELAY_TYPE_MIN_DEF,                 RELAY_TYPE_MAX_DEF,                 //kCfgRelay8Type
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       RELAY_RELAY9_TYPE_DEF,              RELAY_TYPE_MIN_DEF,                 RELAY_TYPE_MAX_DEF,                 //kCfgRelay9Type
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       RELAY_RELAY10_TYPE_DEF,             RELAY_TYPE_MIN_DEF,                 RELAY_TYPE_MAX_DEF,                 //kCfgRelay10Type
    
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       INSULATION_TYPE_DEF,                INSULATION_TYPE_MIN_DEF,            INSULATION_TYPE_MAX_DEF,            //kInsulationType
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       BATTERY_TOTAL_NUM_DEF,              BATTERY_TOTAL_NUM_MIN_DEF,          BATTERY_TOTAL_NUM_MAX_DEF,          //kBatteryTotalNum
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       NOMINAL_TOTAL_VOLTAGE_DEF,          NOMINAL_TOTAL_VOLTAGE_MIN_DEF,      NOMINAL_TOTAL_VOLTAGE_MAX_DEF,      //kNominalTotalVolt
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHARGE_EFFICIENCY_FACTOR_DEF,       CHARGE_EFFICIENCY_FACTOR_MIN_DEF,   CHARGE_EFFICIENCY_FACTOR_MAX_DEF,   //kChargeEfficiencyFactor
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DISCHARGE_EFFICIENCY_FACTOR_DEF,    DISCHARGE_EFFICIENCY_FACTOR_MIN_DEF,DISCHARGE_EFFICIENCY_FACTOR_MAX_DEF,//kDisChargeEfficiencyFactor
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       BMS_BMU_DEFAULT_VOLTAGE_NUM,        0,                                  BMS_BMU_BATTERY_MAX_NUM,            //kBmuMaxVoltageNumIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       BMS_BMU_DEFAULT_TEMPERATURE_NUM,    0,                                  BMS_BMU_TEMPERATURE_MAX_NUM,        //kBmuMaxTempNumIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       BMS_CHARGER_CUT_OFF_CURRENT,        BMS_CHARGER_CUT_OFF_CURRENT_MIN,    BMS_CHARGER_CUT_OFF_CURRENT_MAX,    //kChgCutOffCurIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       BMS_CHARGER_CUT_OFF_CURRENT_DELAY,  SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kChgCutOffCurDlyIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       BMS_CHARGER_TOTAL_TIME_DEF,         BMS_CHARGER_TOTAL_TIME_MIN,         BMS_CHARGER_TOTAL_TIME_MAX,         //kChgTotalTimeMaxIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       BMS_FULL_CHG_RELEASE_VOLT,          BMS_FULL_CHG_RELEASE_VOLT_MIN,      BMS_FULL_CHG_RELEASE_VOLT_MAX,      //kChgFullChgTVReleaseIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_HT_FST_ALARM_DEF,               SYS_DEFAULT_TEMP_MIN_DEF,           SYS_DEFAULT_TEMP_MAX_DEF,           //kChgHTFstAlarmIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_HT_FST_ALARM_REL_DEF,           SYS_DEFAULT_TEMP_MIN_DEF,           SYS_DEFAULT_TEMP_MAX_DEF,           //kChgHTFstAlarmRelIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_HT_SND_ALARM_DEF,               SYS_DEFAULT_TEMP_MIN_DEF,           SYS_DEFAULT_TEMP_MAX_DEF,           //kChgHTSndAlarmIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_HT_SND_ALARM_REL_DEF,           SYS_DEFAULT_TEMP_MIN_DEF,           SYS_DEFAULT_TEMP_MAX_DEF,           //kChgHTSndAlarmRelIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_HT_FST_ALARM_DLY_DEF,           SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kChgHTFstAlarmDlyIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_HT_FST_ALARM_REL_DLY_DEF,       SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kChgHTFstAlarmRelDlyIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_HT_SND_ALARM_DLY_DEF,           SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kChgHTSndAlarmDlyIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_HT_SND_ALARM_REL_DLY_DEF,       SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kChgHTSndAlarmRelDlyIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kCustomEeParaOneIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kCustomEeParaTwoIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_HV_FST_ALARM_DEF,               SYS_DEFAULT_VOLT_MIN_DEF,           SYS_DEFAULT_VOLT_MAX_DEF,           //kChgHVFstAlarmIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_HV_FST_ALARM_REL_DEF,           SYS_DEFAULT_VOLT_MIN_DEF,           SYS_DEFAULT_VOLT_MAX_DEF,           //kChgHVFstAlarmRelIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_HV_SND_ALARM_DEF,               SYS_DEFAULT_VOLT_MIN_DEF,           SYS_DEFAULT_VOLT_MAX_DEF,           //kChgHVSndAlarmIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_HV_SND_ALARM_REL_DEF,           SYS_DEFAULT_VOLT_MIN_DEF,           SYS_DEFAULT_VOLT_MAX_DEF,           //kChgHVSndAlarmRelIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_HV_TRD_ALARM_DEF,               SYS_DEFAULT_VOLT_MIN_DEF,           SYS_DEFAULT_VOLT_MAX_DEF,           //kChgHVTrdAlarmIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_HV_TRD_ALARM_REL_DEF,           SYS_DEFAULT_VOLT_MIN_DEF,           SYS_DEFAULT_VOLT_MAX_DEF,           //kChgHVTrdAlarmRelIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_HV_FST_ALARM_DLY_DEF,           SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kChgHVFstAlarmDlyIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_HV_FST_ALARM_REL_DLY_DEF,       SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kChgHVFstAlarmRelDlyIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_HV_SND_ALARM_DLY_DEF,           SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kChgHVSndAlarmDlyIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_HV_SND_ALARM_REL_DLY_DEF,       SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kChgHVSndAlarmRelDlyIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_HV_TRD_ALARM_DLY_DEF,           SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kChgHVTrdAlarmDlyIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_HV_TRD_ALARM_REL_DLY_DEF,       SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kChgHVTrdAlarmRelDlyIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_HV_TRD_ALARM_DEF,              SYS_DEFAULT_VOLT_MIN_DEF,           SYS_DEFAULT_VOLT_MAX_DEF,           //kDChgHVTrdAlarmIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_HV_TRD_ALARM_REL_DEF,          SYS_DEFAULT_VOLT_MIN_DEF,           SYS_DEFAULT_VOLT_MAX_DEF,           //kDChgHVTrdAlarmRelIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_HV_TRD_ALARM_DLY_DEF,          SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kDChgHVTrdAlarmDlyIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_HV_TRD_ALARM_REL_DLY_DEF,      SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kDChgHVTrdAlarmRelDlyIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_LV_FST_ALARM_DEF,               SYS_DEFAULT_VOLT_MIN_DEF,           SYS_DEFAULT_VOLT_MAX_DEF,           //kChgLVFstAlarmIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_LV_FST_ALARM_REL_DEF,           SYS_DEFAULT_VOLT_MIN_DEF,           SYS_DEFAULT_VOLT_MAX_DEF,           //kChgLVFstAlarmRelIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_LV_SND_ALARM_DEF,               SYS_DEFAULT_VOLT_MIN_DEF,           SYS_DEFAULT_VOLT_MAX_DEF,           //kChgLVSndAlarmIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_LV_SND_ALARM_REL_DEF,           SYS_DEFAULT_VOLT_MIN_DEF,           SYS_DEFAULT_VOLT_MAX_DEF,           //kChgLVSndAlarmRelIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_LV_TRD_ALARM_DEF,               SYS_DEFAULT_VOLT_MIN_DEF,           SYS_DEFAULT_VOLT_MAX_DEF,           //kChgLVTrdAlarmIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_LV_TRD_ALARM_REL_DEF,           SYS_DEFAULT_VOLT_MIN_DEF,           SYS_DEFAULT_VOLT_MAX_DEF,           //kChgLVTrdAlarmRelIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_LV_FST_ALARM_DLY_DEF,           SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kChgLVFstAlarmDlyIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_LV_FST_ALARM_REL_DLY_DEF,       SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kChgLVFstAlarmRelDlyIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_LV_SND_ALARM_DLY_DEF,           SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kChgLVSndAlarmDlyIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_LV_SND_ALARM_REL_DLY_DEF,       SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kChgLVSndAlarmRelDlyIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_LV_TRD_ALARM_DLY_DEF,           SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kChgLVTrdAlarmDlyIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_LV_TRD_ALARM_REL_DLY_DEF,       SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kChgLVTrdAlarmRelDlyIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_LV_TRD_ALARM_DEF,              SYS_DEFAULT_VOLT_MIN_DEF,           SYS_DEFAULT_VOLT_MAX_DEF,           //kDChgLVTrdAlarmIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_LV_TRD_ALARM_REL_DEF,          SYS_DEFAULT_VOLT_MIN_DEF,           SYS_DEFAULT_VOLT_MAX_DEF,           //kDChgLVTrdAlarmRelIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_LV_TRD_ALARM_DLY_DEF,          SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kDChgLVTrdAlarmDlyIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_LV_TRD_ALARM_REL_DLY_DEF,      SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kDChgLVTrdAlarmRelDlyIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_OC_TRD_ALARM_DEF,               SYS_DEFAULT_CUR_MIN_DEF,            SYS_DEFAULT_CUR_MAX_DEF,            //kChgOCTrdAlarmIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_OC_TRD_ALARM_REL_DEF,           SYS_DEFAULT_CUR_MIN_DEF,            SYS_DEFAULT_CUR_MAX_DEF,            //kChgOCTrdAlarmRelIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_OC_TRD_ALARM_DLY_DEF,           SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kChgOCTrdAlarmDlyIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_OC_TRD_ALARM_REL_DLY_DEF,       SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kChgOCTrdAlarmRelDlyIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_OC_TRD_ALARM_DEF,              SYS_DEFAULT_CUR_MIN_DEF,            SYS_DEFAULT_CUR_MAX_DEF,            //kDChgOCTrdAlarmIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_OC_TRD_ALARM_REL_DEF,          SYS_DEFAULT_CUR_MIN_DEF,            SYS_DEFAULT_CUR_MAX_DEF,            //kDChgOCTrdAlarmRelIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_OC_TRD_ALARM_DLY_DEF,          SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kDChgOCTrdAlarmDlyIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_OC_TRD_ALARM_REL_DLY_DEF,      SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kDChgOCTrdAlarmRelDlyIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_HT_TRD_ALARM_DEF,               SYS_DEFAULT_TEMP_MIN_DEF,           SYS_DEFAULT_TEMP_MAX_DEF,           //kChgHTTrdAlarmIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_HT_TRD_ALARM_REL_DEF,           SYS_DEFAULT_TEMP_MIN_DEF,           SYS_DEFAULT_TEMP_MAX_DEF,           //kChgHTTrdAlarmRelIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_HT_TRD_ALARM_DLY_DEF,           SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kChgHTTrdAlarmDlyIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_HT_TRD_ALARM_REL_DLY_DEF,       SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kChgHTTrdAlarmRelDlyIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_HT_TRD_ALARM_DEF,              SYS_DEFAULT_TEMP_MIN_DEF,           SYS_DEFAULT_TEMP_MAX_DEF,           //kDChgHTTrdAlarmIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_HT_TRD_ALARM_REL_DEF,          SYS_DEFAULT_TEMP_MIN_DEF,           SYS_DEFAULT_TEMP_MAX_DEF,           //kDChgHTTrdAlarmRelIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_HT_TRD_ALARM_DLY_DEF,          SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kDChgHTTrdAlarmDlyIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_HT_TRD_ALARM_REL_DLY_DEF,      SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kDChgHTTrdAlarmRelDlyIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_LT_FST_ALARM_DEF,               SYS_DEFAULT_TEMP_MIN_DEF,           SYS_DEFAULT_TEMP_MAX_DEF,           //kChgLTFstAlarmIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_LT_FST_ALARM_REL_DEF,           SYS_DEFAULT_TEMP_MIN_DEF,           SYS_DEFAULT_TEMP_MAX_DEF,           //kChgLTFstAlarmRelIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_LT_SND_ALARM_DEF,               SYS_DEFAULT_TEMP_MIN_DEF,           SYS_DEFAULT_TEMP_MAX_DEF,           //kChgLTSndAlarmIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_LT_SND_ALARM_REL_DEF,           SYS_DEFAULT_TEMP_MIN_DEF,           SYS_DEFAULT_TEMP_MAX_DEF,           //kChgLTSndAlarmRelIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_LT_TRD_ALARM_DEF,               SYS_DEFAULT_TEMP_MIN_DEF,           SYS_DEFAULT_TEMP_MAX_DEF,           //kChgLTTrdAlarmIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_LT_TRD_ALARM_REL_DEF,           SYS_DEFAULT_TEMP_MIN_DEF,           SYS_DEFAULT_TEMP_MAX_DEF,           //kChgLTTrdAlarmRelIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_LT_FST_ALARM_DLY_DEF,           SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kChgLTFstAlarmDlyIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_LT_FST_ALARM_REL_DLY_DEF,       SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kChgLTFstAlarmRelDlyIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_LT_SND_ALARM_DLY_DEF,           SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kChgLTSndAlarmDlyIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_LT_SND_ALARM_REL_DLY_DEF,       SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kChgLTSndAlarmRelDlyIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_LT_TRD_ALARM_DLY_DEF,           SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kChgLTTrdAlarmDlyIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_LT_TRD_ALARM_REL_DLY_DEF,       SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kChgLTTrdAlarmRelDlyIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_LT_TRD_ALARM_DEF,              SYS_DEFAULT_TEMP_MIN_DEF,           SYS_DEFAULT_TEMP_MAX_DEF,           //kDChgLTTrdAlarmIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_LT_TRD_ALARM_REL_DEF,          SYS_DEFAULT_TEMP_MIN_DEF,           SYS_DEFAULT_TEMP_MAX_DEF,           //kDChgLTTrdAlarmRelIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_LT_TRD_ALARM_DLY_DEF,          SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kDChgLTTrdAlarmDlyIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_LT_TRD_ALARM_REL_DLY_DEF,      SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kDChgLTTrdAlarmRelDlyIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_DT_FST_ALARM_DEF,               SYS_DEFAULT_DT_MIN_DEF,             SYS_DEFAULT_DT_MAX_DEF,             //kChgDTFstAlarmIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_DT_FST_ALARM_REL_DEF,           SYS_DEFAULT_DT_MIN_DEF,             SYS_DEFAULT_DT_MAX_DEF,             //kChgDTFstAlarmRelIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_DT_SND_ALARM_DEF,               SYS_DEFAULT_DT_MIN_DEF,             SYS_DEFAULT_DT_MAX_DEF,             //kChgDTSndAlarmIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_DT_SND_ALARM_REL_DEF,           SYS_DEFAULT_DT_MIN_DEF,             SYS_DEFAULT_DT_MAX_DEF,             //kChgDTSndAlarmRelIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_DT_TRD_ALARM_DEF,               SYS_DEFAULT_DT_MIN_DEF,             SYS_DEFAULT_DT_MAX_DEF,             //kChgDTTrdAlarmIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_DT_TRD_ALARM_REL_DEF,           SYS_DEFAULT_DT_MIN_DEF,             SYS_DEFAULT_DT_MAX_DEF,             //kChgDTTrdAlarmRelIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_DT_FST_ALARM_DLY_DEF,           SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kChgDTFstAlarmDlyIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_DT_FST_ALARM_REL_DLY_DEF,       SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kChgDTFstAlarmRelDlyIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_DT_SND_ALARM_DLY_DEF,           SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kChgDTSndAlarmDlyIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_DT_SND_ALARM_REL_DLY_DEF,       SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kChgDTSndAlarmRelDlyIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_DT_TRD_ALARM_DLY_DEF,           SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kChgDTTrdAlarmDlyIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_DT_TRD_ALARM_REL_DLY_DEF,       SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kChgDTTrdAlarmRelDlyIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_DT_TRD_ALARM_DEF,              SYS_DEFAULT_DT_MIN_DEF,             SYS_DEFAULT_DT_MAX_DEF,             //kDChgDTTrdAlarmIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_DT_TRD_ALARM_REL_DEF,          SYS_DEFAULT_DT_MIN_DEF,             SYS_DEFAULT_DT_MAX_DEF,             //kDChgDTTrdAlarmRelIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_DT_TRD_ALARM_DLY_DEF,          SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kDChgDTTrdAlarmDlyIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_DT_TRD_ALARM_REL_DLY_DEF,      SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kDChgDTTrdAlarmRelDlyIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       COMM_SND_ALARM_DLY_DEF,             COMM_ABORT_ALARM_DLY_MIN,           COMM_ABORT_ALARM_DLY_MAX,           //kCommSndAlarmDlyIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       COMM_SND_ALARM_REL_DLY_DEF,         COMM_ABORT_ALARM_REL_DLY_MIN,       COMM_ABORT_ALARM_REL_DLY_MAX,       //kCommSndAlarmRelDlyIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       COMM_TRD_ALARM_DLY_DEF,             COMM_ABORT_ALARM_DLY_MIN,           SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kCommTrdAlarmDlyIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       COMM_TRD_ALARM_REL_DLY_DEF,         COMM_ABORT_ALARM_REL_DLY_MIN,       COMM_ABORT_ALARM_REL_DLY_MAX,       //kCommTrdAlarmRelDlyIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       TEMP_LINE_SND_ALARM_DLY_DEF,        SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kTempLineSndAlarmDlyIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       TEMP_LINE_SND_ALARM_REL_DLY_DEF,    SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kTempLineSndAlarmRelDlyIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       TEMP_LINE_TRD_ALARM_DLY_DEF,        SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kTempLineTrdAlarmDlyIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       TEMP_LINE_TRD_ALARM_REL_DLY_DEF,    SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kTempLineTrdAlarmRelDlyIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       VOLT_LINE_SND_ALARM_DLY_DEF,        SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kVoltLineSndAlarmDlyIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       VOLT_LINE_SND_ALARM_REL_DLY_DEF,    SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kVoltLineSndAlarmRelDlyIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       VOLT_LINE_TRD_ALARM_DLY_DEF,        SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kVoltLineTrdAlarmDlyIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       VOLT_LINE_TRD_ALARM_REL_DLY_DEF,    SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kVoltLineTrdAlarmRelDlyIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       SOC_HIGH_TRD_ALARM_DEF,             SYS_DEFAULT_SOC_MIN_DEF,            SYS_DEFAULT_SOC_MAX_DEF,            //kSOCHighTrdAlarmIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       SOC_HIGH_TRD_ALARM_REL_DEF,         SYS_DEFAULT_SOC_MIN_DEF,            SYS_DEFAULT_SOC_MAX_DEF,            //kSOCHighTrdAlarmRelIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       SOC_HIGH_TRD_ALARM_DLY_DEF,         SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kSOCHighTrdAlarmDlyIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       SOC_HIGH_TRD_ALARM_REL_DLY_DEF,     SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kSOCHighTrdAlarmRelDlyIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       SOC_LOW_TRD_ALARM_DEF,              SYS_DEFAULT_SOC_MIN_DEF,            SYS_DEFAULT_SOC_MAX_DEF,            //kSOCLowTrdAlarmIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       SOC_LOW_TRD_ALARM_REL_DEF,          SYS_DEFAULT_SOC_MIN_DEF,            SYS_DEFAULT_SOC_MAX_DEF,            //kSOCLowTrdAlarmRelIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       SOC_LOW_TRD_ALARM_DLY_DEF,          SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kSOCLowTrdAlarmDlyIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       SOC_LOW_TRD_ALARM_REL_DLY_DEF,      SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kSOCLowTrdAlarmRelDlyIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_DV_FST_ALARM_DEF,               SYS_DEFAULT_DV_MIN_DEF,             SYS_DEFAULT_DV_MAX_DEF,             //kChgDVFstAlarmIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_DV_FST_ALARM_REL_DEF,           SYS_DEFAULT_DV_MIN_DEF,             SYS_DEFAULT_DV_MAX_DEF,             //kChgDVFstAlarmRelIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_DV_SND_ALARM_DEF,               SYS_DEFAULT_DV_MIN_DEF,             SYS_DEFAULT_DV_MAX_DEF,             //kChgDVSndAlarmIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_DV_SND_ALARM_REL_DEF,           SYS_DEFAULT_DV_MIN_DEF,             SYS_DEFAULT_DV_MAX_DEF,             //kChgDVSndAlarmRelIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_DV_TRD_ALARM_DEF,               SYS_DEFAULT_DV_MIN_DEF,             SYS_DEFAULT_DV_MAX_DEF,             //kChgDVTrdAlarmIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_DV_TRD_ALARM_REL_DEF,           SYS_DEFAULT_DV_MIN_DEF,             SYS_DEFAULT_DV_MAX_DEF,             //kChgDVTrdAlarmRelIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_DV_FST_ALARM_DLY_DEF,           SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kChgDVFstAlarmDlyIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_DV_FST_ALARM_REL_DLY_DEF,       SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kChgDVFstAlarmRelDlyIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_DV_SND_ALARM_DLY_DEF,           SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kChgDVSndAlarmDlyIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_DV_SND_ALARM_REL_DLY_DEF,       SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kChgDVSndAlarmRelDlyIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_DV_TRD_ALARM_DLY_DEF,           SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kChgDVTrdAlarmDlyIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_DV_TRD_ALARM_REL_DLY_DEF,       SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kChgDVTrdAlarmRelDlyIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_DV_TRD_ALARM_DEF,              SYS_DEFAULT_DV_MIN_DEF,             SYS_DEFAULT_DV_MAX_DEF,             //kDChgDVTrdAlarmIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_DV_TRD_ALARM_REL_DEF,          SYS_DEFAULT_DV_MIN_DEF,             SYS_DEFAULT_DV_MAX_DEF,             //kDChgDVTrdAlarmRelIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_DV_TRD_ALARM_DLY_DEF,          SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kDChgDVTrdAlarmDlyIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_DV_TRD_ALARM_REL_DLY_DEF,      SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kDChgDVTrdAlarmRelDlyIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_HTV_FST_ALARM_DEF,              SYS_DEFAULT_TV_MIN_DEF,             SYS_DEFAULT_TV_MAX_DEF,             //kChgHTVFstAlarmIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_HTV_FST_ALARM_REL_DEF,          SYS_DEFAULT_TV_MIN_DEF,             SYS_DEFAULT_TV_MAX_DEF,             //kChgHTVFstAlarmRelIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_HTV_SND_ALARM_DEF,              SYS_DEFAULT_TV_MIN_DEF,             SYS_DEFAULT_TV_MAX_DEF,             //kChgHTVSndAlarmIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_HTV_SND_ALARM_REL_DEF,          SYS_DEFAULT_TV_MIN_DEF,             SYS_DEFAULT_TV_MAX_DEF,             //kChgHTVSndAlarmRelIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_HTV_TRD_ALARM_DEF,              SYS_DEFAULT_TV_MIN_DEF,             SYS_DEFAULT_TV_MAX_DEF,             //kChgHTVTrdAlarmIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_HTV_TRD_ALARM_REL_DEF,          SYS_DEFAULT_TV_MIN_DEF,             SYS_DEFAULT_TV_MAX_DEF,             //kChgHTVTrdAlarmRelIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_HTV_FST_ALARM_DLY_DEF,          SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kChgHTVFstAlarmDlyIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_HTV_FST_ALARM_REL_DLY_DEF,      SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kChgHTVFstAlarmRelDlyIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_HTV_SND_ALARM_DLY_DEF,          SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kChgHTVSndAlarmDlyIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_HTV_SND_ALARM_REL_DLY_DEF,      SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kChgHTVSndAlarmRelDlyIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_HTV_TRD_ALARM_DLY_DEF,          SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kChgHTVTrdAlarmDlyIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_HTV_TRD_ALARM_REL_DLY_DEF,      SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kChgHTVTrdAlarmRelDlyIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_HTV_TRD_ALARM_DEF,             SYS_DEFAULT_TV_MIN_DEF,             SYS_DEFAULT_TV_MAX_DEF,             //kDChgHTVTrdAlarmIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_HTV_TRD_ALARM_REL_DEF,         SYS_DEFAULT_TV_MIN_DEF,             SYS_DEFAULT_TV_MAX_DEF,             //kDChgHTVTrdAlarmRelIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_HTV_TRD_ALARM_DLY_DEF,         SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kDChgHTVTrdAlarmDlyIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_HTV_TRD_ALARM_REL_DLY_DEF,     SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kDChgHTVTrdAlarmRelDlyIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_LTV_FST_ALARM_DEF,              SYS_DEFAULT_TV_MIN_DEF,             SYS_DEFAULT_TV_MAX_DEF,             //kChgLTVFstAlarmIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_LTV_FST_ALARM_REL_DEF,          SYS_DEFAULT_TV_MIN_DEF,             SYS_DEFAULT_TV_MAX_DEF,             //kChgLTVFstAlarmRelIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_LTV_SND_ALARM_DEF,              SYS_DEFAULT_TV_MIN_DEF,             SYS_DEFAULT_TV_MAX_DEF,             //kChgLTVSndAlarmIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_LTV_SND_ALARM_REL_DEF,          SYS_DEFAULT_TV_MIN_DEF,             SYS_DEFAULT_TV_MAX_DEF,             //kChgLTVSndAlarmRelIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_LTV_TRD_ALARM_DEF,              SYS_DEFAULT_TV_MIN_DEF,             SYS_DEFAULT_TV_MAX_DEF,             //kChgLTVTrdAlarmIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_LTV_TRD_ALARM_REL_DEF,          SYS_DEFAULT_TV_MIN_DEF,             SYS_DEFAULT_TV_MAX_DEF,             //kChgLTVTrdAlarmRelIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_LTV_FST_ALARM_DLY_DEF,          SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kChgLTVFstAlarmDlyIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_LTV_FST_ALARM_REL_DLY_DEF,      SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kChgLTVFstAlarmRelDlyIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_LTV_SND_ALARM_DLY_DEF,          SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kChgLTVSndAlarmDlyIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_LTV_SND_ALARM_REL_DLY_DEF,      SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kChgLTVSndAlarmRelDlyIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_LTV_TRD_ALARM_DLY_DEF,          SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kChgLTVTrdAlarmDlyIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_LTV_TRD_ALARM_REL_DLY_DEF,      SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kChgLTVTrdAlarmRelDlyIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_LTV_TRD_ALARM_DEF,             SYS_DEFAULT_TV_MIN_DEF,             SYS_DEFAULT_TV_MAX_DEF,             //kDChgLTVTrdAlarmIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_LTV_TRD_ALARM_REL_DEF,         SYS_DEFAULT_TV_MIN_DEF,             SYS_DEFAULT_TV_MAX_DEF,             //kDChgLTVTrdAlarmRelIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_LTV_TRD_ALARM_DLY_DEF,         SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kDChgLTVTrdAlarmDlyIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DCHG_LTV_TRD_ALARM_REL_DLY_DEF,     SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kDChgLTVTrdAlarmRelDlyIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CAN_DEV_1_BPS_DEF,                  CAN_DEV_BPS_MIN_DEF,                CAN_DEV_BPS_MAX_DEF,                //kCAN1BpsIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CAN_DEV_2_BPS_DEF,                  CAN_DEV_BPS_MIN_DEF,                CAN_DEV_BPS_MAX_DEF,                //kCAN2BpsIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CAN_DEV_3_BPS_DEF,                  CAN_DEV_BPS_MIN_DEF,                CAN_DEV_BPS_MAX_DEF,                //kCAN3BpsIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       RS485_DTU_BPS_DEF,                  RS485_BPS_MIN_DEF,                  RS485_BPS_MAX_DEF,                  //kRS4851BpsIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       RS485_SLAVE_BPS_DEF,                RS485_BPS_MIN_DEF,                  RS485_BPS_MAX_DEF,                  //kRS4852BpsIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       INPUT_SIGNAL1_TYPE_DEF,             INPUT_TYPE_MIN_DEF,                 INPUT_TYPE_MAX_DEF,                 //kCfgInput1TypeIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       INPUT_SIGNAL2_TYPE_DEF,             INPUT_TYPE_MIN_DEF,                 INPUT_TYPE_MAX_DEF,                 //kCfgInput2TypeIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       INPUT_SIGNAL3_TYPE_DEF,             INPUT_TYPE_MIN_DEF,                 INPUT_TYPE_MAX_DEF,                 //kCfgInput3TypeIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       INPUT_SIGNAL4_TYPE_DEF,             INPUT_TYPE_MIN_DEF,                 INPUT_TYPE_MAX_DEF,                 //kCfgInput4TypeIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       INPUT_SIGNAL5_TYPE_DEF,             INPUT_TYPE_MIN_DEF,                 INPUT_TYPE_MAX_DEF,                 //kCfgInput5TypeIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       INPUT_SIGNAL6_TYPE_DEF,             INPUT_TYPE_MIN_DEF,                 INPUT_TYPE_MAX_DEF,                 //kCfgInput6TypeIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       INPUT_SIGNAL7_TYPE_DEF,             INPUT_TYPE_MIN_DEF,                 INPUT_TYPE_MAX_DEF,                 //kCfgInput7TypeIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       INPUT_SIGNAL8_TYPE_DEF,             INPUT_TYPE_MIN_DEF,                 INPUT_TYPE_MAX_DEF,                 //kCfgInput8TypeIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,       BYU_VOLT_TEMP_CNT_1_DEF,                 BYU_VOLT_TEMP_CNT_MIN_DEF,               BYU_VOLT_TEMP_CNT_MAX_DEF,               //kBYUVoltTempCnt1Index
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,       BYU_VOLT_TEMP_CNT_2_DEF,                 BYU_VOLT_TEMP_CNT_MIN_DEF,               BYU_VOLT_TEMP_CNT_MAX_DEF,               //kBYUVoltTempCnt2Index
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,       BYU_VOLT_TEMP_CNT_3_DEF,                 BYU_VOLT_TEMP_CNT_MIN_DEF,               BYU_VOLT_TEMP_CNT_MAX_DEF,               //kBYUVoltTempCnt3Index
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,       BYU_VOLT_TEMP_CNT_4_DEF,                 BYU_VOLT_TEMP_CNT_MIN_DEF,               BYU_VOLT_TEMP_CNT_MAX_DEF,               //kBYUVoltTempCnt4Index
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,       BYU_VOLT_TEMP_CNT_5_DEF,                 BYU_VOLT_TEMP_CNT_MIN_DEF,               BYU_VOLT_TEMP_CNT_MAX_DEF,               //kBYUVoltTempCnt5Index
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,       BYU_VOLT_TEMP_CNT_6_DEF,                 BYU_VOLT_TEMP_CNT_MIN_DEF,               BYU_VOLT_TEMP_CNT_MAX_DEF,               //kBYUVoltTempCnt6Index
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,       BYU_VOLT_TEMP_CNT_7_DEF,                 BYU_VOLT_TEMP_CNT_MIN_DEF,               BYU_VOLT_TEMP_CNT_MAX_DEF,               //kBYUVoltTempCnt7Index
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,       BYU_VOLT_TEMP_CNT_8_DEF,                 BYU_VOLT_TEMP_CNT_MIN_DEF,               BYU_VOLT_TEMP_CNT_MAX_DEF,               //kBYUVoltTempCnt8Index
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,       BYU_VOLT_TEMP_CNT_9_DEF,                 BYU_VOLT_TEMP_CNT_MIN_DEF,               BYU_VOLT_TEMP_CNT_MAX_DEF,               //kBYUVoltTempCnt9Index
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,       BYU_VOLT_TEMP_CNT_10_DEF,                BYU_VOLT_TEMP_CNT_MIN_DEF,               BYU_VOLT_TEMP_CNT_MAX_DEF,               //kBYUVoltTempCnt10Index
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,       BYU_VOLT_TEMP_CNT_11_DEF,                 BYU_VOLT_TEMP_CNT_MIN_DEF,               BYU_VOLT_TEMP_CNT_MAX_DEF,               //kBYUVoltTempCnt11Index
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,       BYU_VOLT_TEMP_CNT_12_DEF,                 BYU_VOLT_TEMP_CNT_MIN_DEF,               BYU_VOLT_TEMP_CNT_MAX_DEF,               //kBYUVoltTempCnt12Index
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,       BYU_VOLT_TEMP_CNT_13_DEF,                 BYU_VOLT_TEMP_CNT_MIN_DEF,               BYU_VOLT_TEMP_CNT_MAX_DEF,               //kBYUVoltTempCnt13Index
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,       BYU_VOLT_TEMP_CNT_14_DEF,                 BYU_VOLT_TEMP_CNT_MIN_DEF,               BYU_VOLT_TEMP_CNT_MAX_DEF,               //kBYUVoltTempCnt14Index
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,       BYU_VOLT_TEMP_CNT_15_DEF,                 BYU_VOLT_TEMP_CNT_MIN_DEF,               BYU_VOLT_TEMP_CNT_MAX_DEF,               //kBYUVoltTempCnt15Index
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,       BYU_VOLT_TEMP_CNT_16_DEF,                 BYU_VOLT_TEMP_CNT_MIN_DEF,               BYU_VOLT_TEMP_CNT_MAX_DEF,               //kBYUVoltTempCnt16Index
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,       BYU_VOLT_TEMP_CNT_17_DEF,                 BYU_VOLT_TEMP_CNT_MIN_DEF,               BYU_VOLT_TEMP_CNT_MAX_DEF,               //kBYUVoltTempCnt17Index
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,       BYU_VOLT_TEMP_CNT_18_DEF,                 BYU_VOLT_TEMP_CNT_MIN_DEF,               BYU_VOLT_TEMP_CNT_MAX_DEF,               //kBYUVoltTempCnt18Index
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,       BYU_VOLT_TEMP_CNT_19_DEF,                 BYU_VOLT_TEMP_CNT_MIN_DEF,               BYU_VOLT_TEMP_CNT_MAX_DEF,               //kBYUVoltTempCnt19Index
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,       BYU_VOLT_TEMP_CNT_20_DEF,                BYU_VOLT_TEMP_CNT_MIN_DEF,               BYU_VOLT_TEMP_CNT_MAX_DEF,               //kBYUVoltTempCnt20Index
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,       BYU_VOLT_TEMP_CNT_21_DEF,                 BYU_VOLT_TEMP_CNT_MIN_DEF,               BYU_VOLT_TEMP_CNT_MAX_DEF,               //kBYUVoltTempCnt21Index
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,       BYU_VOLT_TEMP_CNT_22_DEF,                 BYU_VOLT_TEMP_CNT_MIN_DEF,               BYU_VOLT_TEMP_CNT_MAX_DEF,               //kBYUVoltTempCnt22Index
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,       BYU_VOLT_TEMP_CNT_23_DEF,                 BYU_VOLT_TEMP_CNT_MIN_DEF,               BYU_VOLT_TEMP_CNT_MAX_DEF,               //kBYUVoltTempCnt23Index
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,       BYU_VOLT_TEMP_CNT_24_DEF,                 BYU_VOLT_TEMP_CNT_MIN_DEF,               BYU_VOLT_TEMP_CNT_MAX_DEF,               //kBYUVoltTempCnt24Index
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,       BYU_VOLT_TEMP_CNT_25_DEF,                 BYU_VOLT_TEMP_CNT_MIN_DEF,               BYU_VOLT_TEMP_CNT_MAX_DEF,               //kBYUVoltTempCnt25Index
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,       BYU_VOLT_TEMP_CNT_26_DEF,                 BYU_VOLT_TEMP_CNT_MIN_DEF,               BYU_VOLT_TEMP_CNT_MAX_DEF,               //kBYUVoltTempCnt26Index
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,       BYU_VOLT_TEMP_CNT_27_DEF,                 BYU_VOLT_TEMP_CNT_MIN_DEF,               BYU_VOLT_TEMP_CNT_MAX_DEF,               //kBYUVoltTempCnt27Index
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,       BYU_VOLT_TEMP_CNT_28_DEF,                 BYU_VOLT_TEMP_CNT_MIN_DEF,               BYU_VOLT_TEMP_CNT_MAX_DEF,               //kBYUVoltTempCnt28Index
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,       BYU_VOLT_TEMP_CNT_29_DEF,                 BYU_VOLT_TEMP_CNT_MIN_DEF,               BYU_VOLT_TEMP_CNT_MAX_DEF,               //kBYUVoltTempCnt29Index
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,       BYU_VOLT_TEMP_CNT_30_DEF,                BYU_VOLT_TEMP_CNT_MIN_DEF,               BYU_VOLT_TEMP_CNT_MAX_DEF,               //kBYUVoltTempCnt30Index

    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,        0,                                       0,                                       0x0FFF, //kBSU1VoltSampleNoUseBitIndex              
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,        0,                                       0,                                       0x0FFF, //kBSU2VoltSampleNoUseBitIndex 
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,        0,                                       0,                                       0x0FFF, //kBSU3VoltSampleNoUseBitIndex 
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,        0,                                       0,                                       0x0FFF, //kBSU4VoltSampleNoUseBitIndex 
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,        0,                                       0,                                       0x0FFF, //kBSU5VoltSampleNoUseBitIndex 
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,        0,                                       0,                                       0x0FFF, //kBSU6VoltSampleNoUseBitIndex 
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,        0,                                       0,                                       0x0FFF, //kBSU7VoltSampleNoUseBitIndex 
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,        0,                                       0,                                       0x0FFF, //kBSU8VoltSampleNoUseBitIndex 
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,        0,                                       0,                                       0x0FFF, //kBSU9VoltSampleNoUseBitIndex 
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,        0,                                       0,                                       0x0FFF, //kBSU10VoltSampleNoUseBitIndex 
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,        0,                                       0,                                       0x0FFF, //kBSU11VoltSampleNoUseBitIndex 
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,        0,                                       0,                                       0x0FFF, //kBSU12VoltSampleNoUseBitIndex 
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,        0,                                       0,                                       0x0FFF, //kBSU13VoltSampleNoUseBitIndex 
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,        0,                                       0,                                       0x0FFF, //kBSU14VoltSampleNoUseBitIndex 
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,        0,                                       0,                                       0x0FFF, //kBSU15VoltSampleNoUseBitIndex 
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,        0,                                       0,                                       0x0FFF, //kBSU16VoltSampleNoUseBitIndex 
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,        0,                                       0,                                       0x0FFF, //kBSU17VoltSampleNoUseBitIndex 
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,        0,                                       0,                                       0x0FFF, //kBSU18VoltSampleNoUseBitIndex 
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,        0,                                       0,                                       0x0FFF, //kBSU19VoltSampleNoUseBitIndex 
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,        0,                                       0,                                       0x0FFF, //kBSU20VoltSampleNoUseBitIndex 
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,        0,                                       0,                                       0x0FFF, //kBSU21VoltSampleNoUseBitIndex 
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,        0,                                       0,                                       0x0FFF, //kBSU22VoltSampleNoUseBitIndex 
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,        0,                                       0,                                       0x0FFF, //kBSU23VoltSampleNoUseBitIndex 
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,        0,                                       0,                                       0x0FFF, //kBSU24VoltSampleNoUseBitIndex 
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,        0,                                       0,                                       0x0FFF, //kBSU25VoltSampleNoUseBitIndex 
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,        0,                                       0,                                       0x0FFF, //kBSU26VoltSampleNoUseBitIndex 
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,        0,                                       0,                                       0x0FFF, //kBSU27VoltSampleNoUseBitIndex 
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,        0,                                       0,                                       0x0FFF, //kBSU28VoltSampleNoUseBitIndex 
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,        0,                                       0,                                       0x0FFF, //kBSU29VoltSampleNoUseBitIndex 
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,        0,                                       0,                                       0x0FFF, //kBSU30VoltSampleNoUseBitIndex 

    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,        kBsuRelayTypeUnknown,                    kBsuRelayTypeUnknown,                    kBsuRelayTypeHeat,//kBsuRelay1CfgType
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,        kBsuRelayTypeUnknown,                    kBsuRelayTypeUnknown,                    kBsuRelayTypeHeat,//kBsuRelay2CfgType
          
            
    CFG_SIGN_PARA+CFG_CHK_RANGE_ON,         BYU_VOLT_CORRECT_DEF,               BYU_VOLT_CORRECT_MIN_DEF,           BYU_VOLT_CORRECT_MAX_DEF,           //kBYUVoltCorrectIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       BYU_BAL_START_DV_DEF,               SYS_DEFAULT_DV_MIN_DEF,             SYS_DEFAULT_DV_MAX_DEF,             //kBYUBalDiffVoltMinIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       BYU_BAL_STOP_DV_DEF,                SYS_DEFAULT_DV_MIN_DEF,             SYS_DEFAULT_DV_MAX_DEF,             //kBYUBalDiffVoltMaxIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       BYU_BAL_START_VOLT_DEF,             SYS_DEFAULT_VOLT_MIN_DEF,           SYS_DEFAULT_VOLT_MAX_DEF,           //kBYUBalStartVoltIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       BYU_BAL_NUM_DEF,                    BYU_BAL_NUM_MIN_DEF,                BYU_BAL_NUM_MAX_DEF,                //kBYUBalNumIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       BYU_TOTAL_BAL_EN_DEF,               BYU_TOTAL_BAL_EN_MIN_DEF,           BYU_TOTAL_BAL_EN_MAX_DEF,           //kBYUTotalBalEnIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       BYU_BAL_TYPE_DEF,                   BYU_BAL_TYPE_MIN_DEF,               BYU_BAL_TYPE_MAX_DEF,               //kBYUBalTypeIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       BMS_COMMUNICATION_MODE,             BMS_COMM_MODE_MIN,                  BMS_COMM_MODE_MAX,                  //kBCUCommModeIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       BMS_GB_CHARGER_CONNECT_MODE_DEF,    BMS_GB_CHARGER_CONNECT_MODE_MIN,    BMS_GB_CHARGER_CONNECT_MODE_MAX,    //kGBChargerConnectModeIndex
    CFG_SIGN_PARA+CFG_CHK_RANGE_ON,         CCHK_DEF,                           CCHK_MIN_DEF,                       CCHK_MAX_DEF,                       //kHighCurChkIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       UPPER_COMPUTER_CAN_DEV,             SYS_CAN_CHANNEL_MIN_DEF,            SYS_CAN_CHANNEL_MAX_DEF,            //kSlaveCanChannelIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       BMS_CHARGER_CAN_DEV,                SYS_CAN_CHANNEL_MIN_DEF,            SYS_CAN_CHANNEL_MAX_DEF,            //kChargerCanChannelIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       EMS_CAN_DEV,                        SYS_CAN_CHANNEL_MIN_DEF,            SYS_CAN_CHANNEL_MAX_DEF,            //kEmsCanChannelIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       HMI_DEV_NAME,                       SYS_RS485_CHANNEL_MIN_DEF,          SYS_RS485_CHANNEL_MAX_DEF,          //kHmiRS485ChannelIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DTU_DEV_NAME,                       SYS_RS485_CHANNEL_MIN_DEF,          SYS_RS485_CHANNEL_MAX_DEF,          //kDtuRS485ChannelIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       ISENSOR_RANGE_TYPE_DEF,             ISENSOR_RANGE_TYPE_MIN,             ISENSOR_RANGE_TYPE_MAX,             //kCurSenRangeTypeIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       DTU_CAN_DEV,                        SYS_CAN_CHANNEL_MIN_DEF,            SYS_CAN_CHANNEL_MAX_DEF,            //kDtuCanChannelIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CAN_DEV_5_BPS_DEF,                  CAN_DEV_BPS_MIN_DEF,                CAN_DEV_BPS_MAX_DEF,                //kCAN5BpsIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kSNNumber1Index
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kSNNumber2Index
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kSNNumber3Index
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kSNNumber4Index
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kSNNumber5Index
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kSNNumber6Index
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kSNNumber7Index
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      DEVICE_HW_MAJOR_VER*256 + DEVICE_HW_MINOR_VER,     0,                   0,                                  //kHardwareRevisionIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       1,                                  0,                                  1,                                  //kEepromTvCheckIndex,    
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       1,                                  1,                                  3,                                  //kEepromTvRangeIndex,    
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       EEPROM_NORMAL_CHECK_CYCLE_DEF,      EEPROM_NORMAL_CHECK_CYCLE_MIN_DEF,  EEPROM_NORMAL_CHECK_CYCLE_MAX_DEF,  //kEepromNormalCheckCycleIndex,                   /*正常状态下绝缘信息更新周期*/
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       1,                                  0,                                  1,                                  //kEepromInsuResCalibFlagIndex,    
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       EEPROM_MOS_ON_DEF,                  EEPROM_MOS_ON_MIN_DEF,              EEPROM_MOS_ON_MAX_DEF,              //kEepromMosOnDelayIndex,                         /*0表示MOS常闭*/
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       1,                                  0,                                  1,                                  //kEepromInsuResSwitchInChgIndex,        
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       INSULATION_ON_BOARD_MODE,           INSULATION_ON_BOARD_MODE,           INSULATION_SLAVE_MODE,              //kEepromInsuCommModeIndex,        
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,        0,                                 0,                                  1,                                  //kCurSensorReverseIndex
    
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       BMS_CHGR_AC_OUTLET_NUM_DEF,         BMS_CHGR_AC_OUTLET_NUM_MIN,         BMS_CHGR_AC_OUTLET_NUM_MAX,         //kChgrAcTempNumIndex,                 
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       BMS_CHGR_DC_OUTLET_NUM_DEF,         BMS_CHGR_DC_OUTLET_NUM_MIN,         BMS_CHGR_DC_OUTLET_NUM_MAX,         //kChgrDcTempNumIndex,    
                
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_OUTLET_HT_FST_ALARM_DEF,        SYS_DEFAULT_TEMP_MIN_DEF,           CHG_OUTLET_ALARM_MAX_DEF,           //kChgrAcOutletHTFstAlarmIndex,          
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_OUTLET_HT_FST_ALARM_REL_DEF,    SYS_DEFAULT_TEMP_MIN_DEF,           SYS_DEFAULT_TEMP_MAX_DEF,           //kChgrAcOutletHTFstAlarmRelIndex,       
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_OUTLET_HT_FST_ALARM_DLY_DEF,    SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kChgrAcOutletHTFstAlarmDlyIndex,       
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_OUTLET_HT_FST_ALARM_REL_DLY_DEF,SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kChgrAcOutletHTFstAlarmRelDlyIndex,    
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_OUTLET_HT_SND_ALARM_DEF,        SYS_DEFAULT_TEMP_MIN_DEF,           CHG_OUTLET_ALARM_MAX_DEF,           //kChgrAcOutletHTSndAlarmIndex,          
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_OUTLET_HT_SND_ALARM_REL_DEF,    SYS_DEFAULT_TEMP_MIN_DEF,           SYS_DEFAULT_TEMP_MAX_DEF,           //kChgrAcOutletHTSndAlarmRelIndex,       
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_OUTLET_HT_SND_ALARM_DLY_DEF,    SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kChgrAcOutletHTSndAlarmDlyIndex,       
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_OUTLET_HT_SND_ALARM_REL_DLY_DEF,SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kChgrAcOutletHTSndAlarmRelDlyIndex,    
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_OUTLET_HT_TRD_ALARM_DEF,        SYS_DEFAULT_TEMP_MIN_DEF,           SYS_DEFAULT_TEMP_MAX_DEF,           //kChgrAcOutletHTTrdAlarmIndex,          
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_OUTLET_HT_TRD_ALARM_REL_DEF,    SYS_DEFAULT_TEMP_MIN_DEF,           SYS_DEFAULT_TEMP_MAX_DEF,           //kChgrAcOutletHTTrdAlarmRelIndex,       
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_OUTLET_HT_TRD_ALARM_DLY_DEF,    SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kChgrAcOutletHTTrdAlarmDlyIndex,       
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_OUTLET_HT_TRD_ALARM_REL_DLY_DEF,SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kChgrAcOutletHTTrdAlarmRelDlyIndex,    
    
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_OUTLET_HT_FST_ALARM_DEF,        SYS_DEFAULT_TEMP_MIN_DEF,           CHG_OUTLET_ALARM_MAX_DEF,           //kChgrDcOutletHTFstAlarmIndex,          
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_OUTLET_HT_FST_ALARM_REL_DEF,    SYS_DEFAULT_TEMP_MIN_DEF,           SYS_DEFAULT_TEMP_MAX_DEF,           //kChgrDcOutletHTFstAlarmRelIndex,       
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_OUTLET_HT_FST_ALARM_DLY_DEF,    SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kChgrDcOutletHTFstAlarmDlyIndex,       
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_OUTLET_HT_FST_ALARM_REL_DLY_DEF,SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kChgrDcOutletHTFstAlarmRelDlyIndex,    
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_OUTLET_HT_SND_ALARM_DEF,        SYS_DEFAULT_TEMP_MIN_DEF,           CHG_OUTLET_ALARM_MAX_DEF,           //kChgrDcOutletHTSndAlarmIndex,          
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_OUTLET_HT_SND_ALARM_REL_DEF,    SYS_DEFAULT_TEMP_MIN_DEF,           SYS_DEFAULT_TEMP_MAX_DEF,           //kChgrDcOutletHTSndAlarmRelIndex,       
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_OUTLET_HT_SND_ALARM_DLY_DEF,    SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kChgrDcOutletHTSndAlarmDlyIndex,       
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_OUTLET_HT_SND_ALARM_REL_DLY_DEF,SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kChgrDcOutletHTSndAlarmRelDlyIndex,    
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_OUTLET_HT_TRD_ALARM_DEF,        SYS_DEFAULT_TEMP_MIN_DEF,           SYS_DEFAULT_TEMP_MAX_DEF,           //kChgrDcOutletHTTrdAlarmIndex,          
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_OUTLET_HT_TRD_ALARM_REL_DEF,    SYS_DEFAULT_TEMP_MIN_DEF,           SYS_DEFAULT_TEMP_MAX_DEF,           //kChgrDcOutletHTTrdAlarmRelIndex,       
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_OUTLET_HT_TRD_ALARM_DLY_DEF,    SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kChgrDcOutletHTTrdAlarmDlyIndex,       
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CHG_OUTLET_HT_TRD_ALARM_REL_DLY_DEF,SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kChgrDcOutletHTTrdAlarmRelDlyIndex,    
    
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       TEMP_LINE_FST_ALARM_DLY_DEF,        SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kChgrOutletTempLineFstAlarmDlyIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       TEMP_LINE_FST_ALARM_REL_DLY_DEF,    SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kChgrOutletTempLineFstAlarmRelDlyIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       TEMP_LINE_SND_ALARM_DLY_DEF,        SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kChgrOutletTempLineSndAlarmDlyIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       TEMP_LINE_SND_ALARM_REL_DLY_DEF,    SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kChgrOutletTempLineSndAlarmRelDlyIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       TEMP_LINE_TRD_ALARM_DLY_DEF,        SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kChgrOutletTempLineTrdAlarmDlyIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       TEMP_LINE_TRD_ALARM_REL_DLY_DEF,    SYS_DEFAULT_ALARM_DLY_MIN_DEF,      SYS_DEFAULT_ALARM_DLY_MAX_DEF,      //kChgrOutletTempLineTrdAlarmRelDlyIndex
    
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      BMS_VIN_DEFAULT,                    0,                                  0,                                  //kVIN1Index
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      BMS_VIN_DEFAULT,                    0,                                  0,                                  //kVIN2Index
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      BMS_VIN_DEFAULT,                    0,                                  0,                                  //kVIN3Index
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      BMS_VIN_DEFAULT,                    0,                                  0,                                  //kVIN4Index
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      BMS_VIN_DEFAULT,                    0,                                  0,                                  //kVIN5Index
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      BMS_VIN_DEFAULT,                    0,                                  0,                                  //kVIN6Index
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      BMS_VIN_DEFAULT,                    0,                                  0,                                  //kVIN7Index
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      BMS_VIN_DEFAULT,                    0,                                  0,                                  //kVIN8Index
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      BMS_VIN_DEFAULT,                    0,                                  0,                                  //kVIN9Index
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      WAKEUP_INTERVAL_MINUTES,            0,                                  0,                                  //kWakeupIntervalMinutes
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      WORK_MAX_TIME_AFTER_WAKEUP_SECTONDS,0,                                  0,                                  //kWorkMaxTimeAfterWakeupSeconds 
    
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      BMS_MLOCK_TYPE_DFT,                 0,                                  0,                                  //kMlockType
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      BMS_MLOCK_PARAM_LEN_DFT,            0,                                  0,                                  //kMlockParamLength,      /* Max Length = 8*2,     over part will be cut*/
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      BMS_MLOCK_LOCK_TIMEOUT_DFT,         0,                                  0,                                  //kMlockParam1,           /*Amphenol,     lock_time,     busbar,     lock_timeout*/
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      BMS_MLOCK_UNLOCK_TIMEOUT_DFT,       0,                                  0,                                  //kMlockParam2,           /*Amphenol,     unlock_time,     busbar,     unlock_timeout*/
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      BMS_MLOCK_FEEDBACK_DIR_DFT,         0,                                  0,                                  //kMlockParam3,           /*Amphenol,     反馈方向*/
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kMlockParam4
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kMlockParam5
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kMlockParam6
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kMlockParam7
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kMlockParam8    

    
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kReserve1Index    
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kReserve2Index
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kReserve3Index
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kReserve4Index
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kReserve5Index
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kReserve6Index
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kReserve7Index
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kReserve8Index
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kReserve9Index
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kReserve10Index
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kReserve11Index
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kReserve12Index
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kReserve13Index
    
   

    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      1,                                  0,                                  0,                                  //kDTUInfoType
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kDTUInfoServer_1
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kDTUInfoServer_2
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kDTUInfoServer_3
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kDTUInfoServer_4
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kDTUInfoServer_5
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kDTUInfoServer_6
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kDTUInfoServer_7
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kDTUInfoServer_8
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kDTUInfoServer_9
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kDTUInfoServer_10
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kDTUInfoServer_11
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kDTUInfoServer_12
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kDTUInfoServer_13
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kDTUInfoServer_14
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kDTUInfoServer_15
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kDTUInfoPort
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kDTUInfoCIMI_1
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kDTUInfoCIMI_2
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kDTUInfoCIMI_3
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kDTUInfoCIMI_4
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kDTUInfoCIMI_5
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kDTUInfoCIMI_6
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kDTUInfoCIMI_7
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kDTUInfoCIMI_8
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kDTUInfoCIMI_9
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kDTUInfoCIMI_10
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kDTUInfoCIMI_11
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kDTUInfoPhoneNum_1
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kDTUInfoPhoneNum_2
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kDTUInfoPhoneNum_3
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kDTUInfoPhoneNum_4
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kDTUInfoPhoneNum_5
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kDTUInfoPhoneNum_6

    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kSparePartNumber_MSB01                  //0xF187
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kSparePartNumber_MSB23
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kSparePartNumber_MSB45
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kSparePartNumber_MSB67
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kSparePartNumber_LSB89
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0x4568,                             0,                                  0,                                  //kVehicleCode12
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0x7548,                             0,                                  0,                                  //kVehicleCode34
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0x49,                               0,                                  0,                                  //kVehicleCode5
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kSupplierIdentifier_MSB01               //0xF18A
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kSupplierIdentifier_LSB23
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kECUManufacturingDateDataIDYear         //0xF18B    20 17
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kECUManufacturingDateDataIDMonthDay
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kECUSerialNumberDataIdentifier01        //0xF18C
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kECUSerialNumberDataIdentifier23
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kECUSerialNumberDataIdentifier45
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kECUSerialNumberDataIdentifier67
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kSystemSupplierECUHardwareNumber0001    //0xF192
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kSystemSupplierECUHardwareNumber0203
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kSystemSupplierECUHardwareNumber0405
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kSystemSupplierECUHardwareNumber0607
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kSystemSupplierECUHardwareNumber0809
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kSystemSupplierECUHardwareNumber1011
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kSystemSupplierECUHardwareNumber1213
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kSystemSupplierECUHardwareNumber1415
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kSystemSupplierECUHardwareVerNum01      //0xF193
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kSystemSupplierECUHardwareVerNum23
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kSystemSupplierECUSoftwareNumber0001    //0xF194
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kSystemSupplierECUSoftwareNumber0203
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kSystemSupplierECUSoftwareNumber0405
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kSystemSupplierECUSoftwareNumber0607
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kSystemSupplierECUSoftwareNumber0809
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kSystemSupplierECUSoftwareNumber1011
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kSystemSupplierECUSoftwareNumber1213
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kSystemSupplierECUSoftwareNumber1415
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kSystemSupplierECUSoftwareVerNum01      //0xF195
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kSystemSupplierECUSoftwareVerNum23
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kRepairShopCodeType_MSB01               //0xF198
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kRepairShopCodeType_LSB23
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kRepairShopCodeNumber_MSB45
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kRepairShopCodeNumber_LSB67
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kTesterSerialManufacturerCode_MSB89
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kTesterSerialManufacturerCode_LSB1011
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kTesterSerialNumber_MSB1213
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kTesterSerialNumber_LSB1415
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kECUInstallationDateDataIDYear          //0xF19D
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kECUInstallationDateDataIDMonthDate
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kTheNewestBlockFingerprintInfoYear_00   //0xF15A   only ONE byte
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kTheNewestBlockFingerprintInfoMonthDay_12
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kTesterSerialNumber_MSB34
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kTesterSerialNumber_MSB56
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kTesterSerialNumber_LSB78
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kTheAllBlockFingerprintInfo_BlockIDYear01,      //0xF15B
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kTheAllBlockFingerprintInfo_MonthDay23
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kTheAllBlockFingerprintInfo_TesterID45
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kTheAllBlockFingerprintInfo_TesterID67
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kTheAllBlockFingerprintInfo_TesterID89
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       ISENSOR_TYPE_DEF,                   ISENSOR_TYPE_MIN_DEF,               ISENSOR_TYPE_MAX_DEF,                 //kCurSenTypeIndex
	         CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,        0,                                  0,                                  0,                                  //kOtaOverFlag
	    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,        0,                                  0,                                  0,                                  //kOtaErrFlag
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,     0,     0,    //kEepromParamEnd    
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0, 
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0, 
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0, 
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0, 
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0, 
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0, 
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0, 
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0, 
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0, 
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0, 
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0, 
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0, 
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0, //kSocFixAsEmptyIndex 
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0, //kDetectFlagIndex 
    CFG_SIGN_PARA+CFG_CHK_RANGE_ON,      1000, CURRENT_CALIB_K_MIN, CURRENT_CALIB_K_MAX, 
    CFG_SIGN_PARA+CFG_CHK_RANGE_ON,      0, CURRENT_CALIB_B_MIN, CURRENT_CALIB_B_MAX, 
    CFG_SIGN_PARA+CFG_CHK_RANGE_ON,      1000, CURRENT_CALIB_K_MIN, CURRENT_CALIB_K_MAX, 
    CFG_SIGN_PARA+CFG_CHK_RANGE_ON,      0, CURRENT_CALIB_B_MIN, CURRENT_CALIB_B_MAX,
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CURRENT_RES_DEF,                    CURRENT_RES_MIN,                    CURRENT_RES_MAX,
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CAN_WKUP_DEF,                       CAN_WKUP_MIN,                       CAN_WKUP_MAX,
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,  0, 0, 0, //kSOCBMAvailableFlag
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,  0, 0, 0, //KSOCBMHigh
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,  0, 0, 0, //KSOCBMLow
    

    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kEeepromModifyCntIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       LEFTCAP_DEF,                        CAP_MIN_DEF,                        CAP_MAX_DEF,                        //kLeftCapIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kMotorMileLowIndex
    //CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kMotorMileHighIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kCycleStsIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kCycleCntIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       0,                                  0,                                  0,                                  //kDeepDisChgCntIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       CAP_CALIB_RESOLUTION_MAX/2,         TOTAL_CAP_DECIMAL_MIN,              TOTAL_CAP_DECIMAL_MAX,              //kTotalCapDecimalIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kOcvSocTimeHighIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kOcvSocTimeLowIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kCustomEeeParaOneIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kCustomEeeParaTwoIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kCapAmsSaveLowIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kCapAmsSaveHighIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_ON,       SOC0_DISCHG_FACTOR_DEFAULT_VAULE,   SOC0_DISCHG_FACTOR_MIN_VAULE,       SOC0_DISCHG_FACTOR_MAX_VAULE,        //kSOC0DischgFactorIndex soc0放电积分系数
   
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kChgTotalCapLowIndex,           
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kChgTotalCapHighIndex, 
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                  //kDchgTotalCapLowIndex, 
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF,      0,                                  0,                                  0,                                   //kDchgTotalCapHighIndex,
    
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF, 0, 0, 0,//kDeltaCapAMSAfterCalibLowIndex
	  CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF, 0, 0, 0,//kDeltaCapAMSAfterCalibHighIndex
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF, 0, 0, 0,//kDeltaCapSign
    CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF, 0, 0, 0,//kLeftCapAmsLowIndex
	  CFG_UNSIGN_PARA+CFG_CHK_RANGE_OFF, 0, 0, 0//kLeftCapAmsHighIndex 

};

INT8U           g_config_observer_num           = 0;
static INT8U    g_config_save_err_flag          = 0;
INT32U          g_bms_config_save_last_tick     = 0;
INT16U          g_bms_config[kSysParamMaxCount] ={ 0 };
ConfigObserver  g_config_observers[CONFIG_OBSERVER_MAX_COUNT];

#pragma pop

Bool is_sign_config_between_upper_and_lower_limit(ConfigIndex index, INT16S value);
Bool is_unsign_config_between_upper_and_lower_limit(ConfigIndex index, INT16U value);
void config_save_end_check(void* data);

void config_init(void)
{
    Result res;

    //g_config_lock = lock_create();
    eeprom_init();
    
    if (config_load(kEepromFormated) == 1)
    {
        res = eeeprom_init(0);
    }
    else
    {
        res = eeeprom_init(1);
        if (res == RES_OK) 
            config_save(kEepromFormated, 1);
    }

    config_check();
    config_load_all();
    
 #if (BMS_SUPPORT_BY5248D == 1)
    if(config_get(kDeviceTypeIndex)!=kBcuBY5xD) 
    {
        config_save(kDeviceTypeIndex,kBcuBY5xD );
    }
 #else
    if(config_get(kDeviceTypeIndex)!=kBcuBC5xB) 
    {
        config_save(kDeviceTypeIndex,kBcuBC5xB );
    }
 #endif   
}
///////////////////////////////////////////////////////////
INT16U config_rel(INT16U index,INT16S diff_value)
{  INT32S rel_value;
   rel_value=(INT32S)(config_get(index)+(INT32S)diff_value); 
   if((rel_value<0)||(rel_value>65535))    return config_get(index);
   else return ((INT16U)rel_value);     
}                                                               
//////////////////////////////////////////////////////////
void config_uninit(void)
{
    eeeprom_uninit();
    eeprom_uninit();
}

Bool is_config_between_upper_and_lower_limit(ConfigIndex index, INT16U value)
{
    Bool res = TRUE;

    if((g_config_attribute[index].flag & CFG_CHK_RANGE_ON) != CFG_CHK_RANGE_ON)
    {
        return TRUE;
    }

    if(g_config_attribute[index].flag & CFG_SIGN_PARA)
    {
        return is_sign_config_between_upper_and_lower_limit(index, value);
    }
    else
    {
        return is_unsign_config_between_upper_and_lower_limit(index, value);
    }
}

Bool is_sign_config_between_upper_and_lower_limit(ConfigIndex index, INT16S value)
{
    if((value < (INT16S)g_config_attribute[index].config_min) || (value > (INT16S)g_config_attribute[index].config_max))
    {
        return FALSE;
    }
    return TRUE;
}

Bool is_unsign_config_between_upper_and_lower_limit(ConfigIndex index, INT16U value)
{
    if((value < g_config_attribute[index].config_min) || (value > g_config_attribute[index].config_max))
    {
        return FALSE;
    }
    return TRUE;
}

void config_save_all(void)
{
    INT16U index;
    Result res;

    CONFIG_START_SAVE_END_CHECK();
    /** write EEPROM area */
    for (index = kEepromParamStart; index < kEepromParamEnd; ++index)
    {
        res |= eeprom_save_int16u(index - kEepromParamStart, g_bms_config[index]);
    }

    /** write EEEPROM area */
    for (index = kEeepromParamStart; index < kEeepromParamEnd; ++index)
    {
        res |= eeeprom_save_int16u(index - kEeepromParamStart, g_bms_config[index], 1);
    }

    if (res != 0)
        bcu_flag_pack_state(1 << EEPROM_ERR_BIT);
    else
        bcu_unflag_pack_state(1 << EEPROM_ERR_BIT);
}

void config_load_all(void)
{
    INT16U index;
    Result res;
    INT16U value = 0;

    /** read EEPROM area */
    for (index = kEepromParamStart; index < kEepromParamEnd; ++index)
    {
        value = g_bms_config[index];
        res = eeprom_load_int16u(index - kEepromParamStart, &value);
        g_bms_config[index] = value;
        if (res != RES_OK)
        {
            g_bms_config[index] = g_config_attribute[index].config_default;
            bcu_flag_sys_exception_flags(SYS_EXCEPTION_CONFIG_READ_EEPROM_ERROR);//TODO:测试用，此标志不清零
        }
    }

    /** read EEEPROM area */
    for (index = kEeepromParamStart; index < kEeepromParamEnd; ++index)
    {
        value = g_bms_config[index];
        res = eeeprom_load_int16u(index - kEeepromParamStart, &value);
        g_bms_config[index] = value;
        if (res != RES_OK)
        {
            g_bms_config[index] = g_config_attribute[index].config_default;
            bcu_flag_sys_exception_flags(SYS_EXCEPTION_CONFIG_READ_EEEPROM_ERROR);//TODO:测试用，此标志不清零
        }
    }
    
    if (config_get(kHardwareRevisionIndex) == 0)
    {
        value = DEVICE_HW_MAJOR_VER*256+DEVICE_HW_MINOR_VER;
        config_save(kHardwareRevisionIndex, value);
    }
}

void config_set_save_err_flag(INT8U value)
{
    OS_INIT_CRITICAL();

    OS_ENTER_CRITICAL();
    g_config_save_err_flag = value;
    OS_EXIT_CRITICAL();
}

INT8U config_get_save_err_flag(void)
{
    INT8U value;
    OS_INIT_CRITICAL();

    OS_ENTER_CRITICAL();
    value = g_config_save_err_flag;
    OS_EXIT_CRITICAL();

    return value;
}

void config_set(ConfigIndex index, INT16U value)
{
    OS_INIT_CRITICAL();
    RPAGE_INIT();

    RPAGE_SAVE();
    /** the function does not check the index because of the performance */
    if(is_config_between_upper_and_lower_limit(index, value) == FALSE)
    {
        bcu_flag_sys_exception_flags(SYS_EXCEPTION_CONFIG_LIMIT_ERROR);//TODO:测试用，此标志不清零
        config_set_save_err_flag(CFG_SAVE_UPPER_LOWER_LIMIT_ERR);
        RPAGE_RESTORE();
        return;
    }

    if (config_notify_observer(index, value) == RES_OK)
    {
        OS_ENTER_CRITICAL();
        g_bms_config[index] = value;
        OS_EXIT_CRITICAL();
    }
    RPAGE_RESTORE();
}

Result config_save(ConfigIndex index, INT16U value)
{
    /** the function does not check the index because of the performance */
    Result res = RES_OK;
    OS_INIT_CRITICAL();
    RPAGE_INIT();

    RPAGE_SAVE();
    if(g_bms_config[index] == value)
    {
        RPAGE_RESTORE();
        return res;
    }
    
    CONFIG_START_SAVE_END_CHECK();
    
    if(is_config_between_upper_and_lower_limit(index, value) == FALSE)
    {
        config_set_save_err_flag(CFG_SAVE_UPPER_LOWER_LIMIT_ERR);
        bcu_flag_sys_exception_flags(SYS_EXCEPTION_CONFIG_LIMIT_ERROR);//TODO:测试用，此标志不清零
        RPAGE_RESTORE();
        return RES_ERR;
    }

    if (config_notify_observer(index, value) == RES_OK)
    {
        OS_ENTER_CRITICAL();
        g_bms_config[index] = value;
        OS_EXIT_CRITICAL();
        if (index >= kEepromParamStart && index < kEepromParamEnd)
            res = eeprom_save_int16u(index - kEepromParamStart, value);
        else if (index >= kEeepromParamStart && index < kEeepromParamEnd)
        {
            if((index == kLeftCapIndex)&&(value==0))  
              res = eeeprom_save_int16u(index - kEeepromParamStart, value, 1); 
            else 
              res = eeeprom_save_int16u(index - kEeepromParamStart, value, 1);
        }
    }

    if(res != RES_OK)
    {
        bcu_flag_sys_exception_flags(SYS_EXCEPTION_CONFIG_WRITE_ERROR);//TODO:测试用，此标志不清零
        config_set_save_err_flag(CFG_SAVE_WRITE_FLASH_ERR);
    }

    RPAGE_RESTORE();

    return res;
}

INT16U config_load(ConfigIndex index)
{
    Result res;
    INT16U value = 0xFFFF;
    res = eeprom_load_int16u(index - kEepromParamStart, &value);
    
    if(res != RES_OK) value = g_config_attribute[index].config_default;
    return value;
}

INT16U config_get(ConfigIndex index)
{
    /** the function does not check the index because of the performance */
    INT16U value;
    RPAGE_INIT();
    OS_INIT_CRITICAL();

    RPAGE_SAVE();
    OS_ENTER_CRITICAL();
    value = g_bms_config[index];
    OS_EXIT_CRITICAL();
    RPAGE_RESTORE();

    return value;
}

PINT16U config_items(void)
{
    return g_bms_config;
}

/**
 * 检查配置是否存在异常，并尝试修复
 */
void config_check(void)
{
    INT8U flag = 0;
    INT16U index;

    /** 检查eeprom区域 */
    for (index = kEepromParamStart; index < kEepromParamEnd; index++)
    {
        eeprom_repair_int16u_item(index - kEepromParamStart, g_config_attribute[index].config_default);
    }

    /** 检查eeeprom区域 */
    for (index = kEeepromParamStart; index < kEeepromParamEnd; index++)
    {
        eeeprom_repair_int16u_item(index - kEeepromParamStart, g_config_attribute[index].config_default);
    }
}

Result config_register_observer(ConfigIndex start_index, ConfigIndex end_index, ConfigChangingHandler handler)
{
    if (g_config_observer_num >= CONFIG_OBSERVER_MAX_COUNT) return RES_ERROR;

    g_config_observers[g_config_observer_num].start_index = start_index;
    g_config_observers[g_config_observer_num].end_index = end_index;
    g_config_observers[g_config_observer_num].handler = handler;

    g_config_observer_num++;

    return RES_OK;
}

Result config_notify_observer(ConfigIndex index, INT16U value)
{
    Result res = RES_OK;
    INT8U observer_index;
    for (observer_index = 0; observer_index < g_config_observer_num; ++observer_index)
    {
        if (index >= g_config_observers[observer_index].start_index && index <= g_config_observers[observer_index].end_index)
        {
            res = g_config_observers[observer_index].handler(index, value);
            if (res != RES_OK) return res;
        }
    }
    return RES_OK;
}


#if 0
static Result dtu_config_get_string_with_index_and_length(unsigned short index, unsigned char *dat, unsigned char size) {
    Result res = RES_OK;
    unsigned char i;
    if (0 != dat) {
        INT16U *_PAGED p = (INT16U *_PAGED)dat;
        for (i = 0; i < size/2; ++i) {
            res = eeprom_load_int16u(index + i, &p[i]);
            if(res != RES_OK) return res;
        }
        dat[size-1] = 0;
    }
    return res;
}

static Result dtu_config_save_string_with_index_and_length(unsigned short index, unsigned char *dat, unsigned char size) {
    Result res = RES_OK;
    if (0 != dat) {
        INT16U *_PAGED p = (INT16U *_PAGED)dat;
        unsigned char i;
        for (i = 0; i < size/2; ++i) {
            res = eeprom_save_int16u(index + i, p[i]);
            if(res != RES_OK) return res;
        }
    }
    return res;
}
#endif
#define DTU_INFO_TYPE_INDEX     kDTUInfoType//(BMS_DTU_CONFIG_START_INDEX + 0)    // 长度 1(2字节)
#define DTU_INFO_TYPE_LENGTH    1    // 长度 1(2字节) 
//#define DTU_INFO_SERVER_INDEX   kDTUInfoServer_1//(BMS_DTU_CONFIG_START_INDEX +1)    // 长度 15(30字节)
//#define DTU_INFO_SERVER_LEHGTH  15   // 长度 15(30字节)
//#define DTU_INFO_PORT_INDEX     kDTUInfoPort//(BMS_DTU_CONFIG_START_INDEX +16)   // 长度 1(2字节)
//#define DTU_INFO_PORT_LENGTH    1    // 长度 1(2字节)
//#define DTU_INFO_CIMI_INDEX     kDTUInfoCIMI_1//(BMS_DTU_CONFIG_START_INDEX +17)   // 长度 11(22字节)
//#define DTU_INFO_CIMI_LENGTH    11   // 长度 11(22字节)
//#define DTU_INFO_PHONE_INDEX    kDTUInfoPhoneNum_1//(BMS_DTU_CONFIG_START_INDEX +28)   // 长度 6(12字节)
//#define DTU_INFO_PHONE_LENGTH   6    // 长度 6(12字节)

Result config_save_dtu_type(unsigned char type) 
{
    return eeprom_save_int16u(DTU_INFO_TYPE_INDEX, type);
}

Result config_get_dtu_type(unsigned char *_PAGED type) 
{
    INT16U t;                                   

    Result res = eeprom_load_int16u(DTU_INFO_TYPE_INDEX, &t);
    if(res != RES_OK) return res;
    
    if (type != 0) *type = (unsigned char)t;
    return res;
}

Result config_save_dtu_server(unsigned char *_PAGED server, unsigned short port) 
{
    /*INT16U *_PAGED p = (INT16U *_PAGED)server;
    Result res = eeprom_save_int16u(DTU_INFO_PORT_INDEX, port);
    if(res != RES_OK) return res;
    return dtu_config_save_string_with_index_and_length(DTU_INFO_SERVER_INDEX, server, DTU_INFO_SERVER_LEHGTH * 2); */
    return RES_OK;
}


Result config_get_dtu_server(unsigned char *_PAGED server, unsigned char size, INT16U *_PAGED port) 
{ 
   /* INT16U *_PAGED p = (INT16U *_PAGED)server;
    Result res = RES_OK;
    if (0 != port) {
        res = eeprom_load_int16u(DTU_INFO_PORT_INDEX, port);
        if(res != RES_OK) return res;
    }
    if (size >= DTU_INFO_SERVER_LEHGTH*2) {
        size = DTU_INFO_SERVER_LEHGTH * 2;
    }
    return dtu_config_get_string_with_index_and_length(DTU_INFO_SERVER_INDEX, server, size); */
    return RES_OK;
}

Result config_save_dtu_cimi(unsigned char *_PAGED cimi) 
{
   // return dtu_config_save_string_with_index_and_length(DTU_INFO_CIMI_INDEX, cimi, DTU_INFO_CIMI_LENGTH * 2 ); 
   return RES_OK;
}

Result config_get_dtu_cimi(unsigned char *_PAGED cimi, unsigned char size) 
{    
  /*  if (size >= DTU_INFO_CIMI_LENGTH*2) {
        size = DTU_INFO_CIMI_LENGTH * 2;
    }
    return dtu_config_get_string_with_index_and_length(DTU_INFO_CIMI_INDEX, cimi, size );*/
    return RES_OK;
}

Result config_save_dtu_phone(unsigned char *_PAGED phone) {
   // return dtu_config_save_string_with_index_and_length(DTU_INFO_PHONE_INDEX, phone, DTU_INFO_PHONE_LENGTH * 2 ); 
    return RES_OK;
}

Result config_get_dtu_phone(unsigned char *_PAGED phone, unsigned char size) 
{
   /* if (size >= DTU_INFO_PHONE_LENGTH*2) {
        size = DTU_INFO_PHONE_LENGTH * 2;
    }
    return dtu_config_get_string_with_index_and_length(DTU_INFO_CIMI_INDEX, phone, size); */
    return RES_OK;
}

void config_save_end_check(void* data)
{
    UNUSED(data);
    
    if(g_bms_config_save_last_tick == 0) return;
    
    if(get_interval_by_tick(g_bms_config_save_last_tick, get_tick_count()) >= CONFIG_SAVE_END_DELAY)
    {
        CONFIG_STOP_SAVE_END_CHECK();
        if(bcu_config_para_cannot_restart_check() == 0) mcu_reset();
    }
}

void config_eeeprom_init_check(void)
{
    if (config_load(kEepromFormated) != 1 || config_get(kEepromFormated) != 1)  //增强型EEPROM是否正常初始化
    {
        bcu_flag_sys_exception_flags(SYS_EXCEPTION_EEEPROM_FORMAT_ERROR);//此标志不清零
    }
}
