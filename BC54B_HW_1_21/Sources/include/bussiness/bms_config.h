/**
 *
 * Copyright (c) 2012 Ligoo Inc.
 *
 * @file bms_config.h
 * @brief
 * @note
 * @author
 * @date 2012-5-9
 *
 */

#ifndef BMS_CONFIG_H_
#define BMS_CONFIG_H_
#include "bms_defs.h"
#include "bms_business_defs.h"
//#include "bms_base_cfg.h"
#include "bms_eeeprom.h"
#include "bms_eeprom.h"
#include "bms_bcu.h"
#include "bms_relay_control.h"
#include "bms_charger_gb.h"
#include "bms_relay.h"

#define OTA_S19_PART                  (0xa)

#define REMOVE_NO_USED_IDX_TO_SAVE_EEPROM 1

#if( SWITCH_ON == UDS_SWITCH_CONFIG)
#define CONFIG_START_SAVE_END_CHECK()   g_bms_config_save_last_tick = get_tick_count()
#define CONFIG_STOP_SAVE_END_CHECK()    g_bms_config_save_last_tick = 0
#endif

#define  DCHG_HV_FST_ALARM_REL       config_rel(kDChgHVFstAlarmIndex,-200) //放电单体高压一级报警释放阀值
#define  DCHG_HV_SND_AlARM_REL       config_rel(kDChgHVSndAlarmIndex,-100) //放电单体高压二级报警释放阀值
#define  DCHG_HV_TRD_ALARM_REL       config_rel(kDChgHVTrdAlarmIndex,-200)//放电高压三级报警释放阀值

#define  DCHG_HV_FST_AlARM_REL_DLY   config_rel(kDChgHVFstAlarmDlyIndex,1000) //放电单体高压一级报警释放延时
#define  DCHG_HV_SND_AlARM_REL_DLY   config_rel(kDChgHVSndAlarmDlyIndex,1000) //放电单体高压二级报警释放延时
#define  DCHG_HV_TRD_ALARM_REL_DLY   config_rel(kDChgHVTrdAlarmDlyIndex,1000)//放电高压三级报警释放延时

#define  DCHG_LV_FST_ALARM_REL       config_rel(kDChgLVFstAlarmIndex,100) //放电单体低压一级报警释放阀值 
#define  DCHG_LV_SND_ALARM_REL       config_rel(kDChgLVSndAlarmIndex,200) //放电单体低压二级报警释放阀值
#define  DCHG_LV_TRD_ALARM_REL       config_rel(kDChgLVTrdAlarmIndex,100)//放电低压三级报警释放阀值
 
#define  DCHG_LV_FST_AlARM_REL_DLY   config_rel(kDChgLVFstAlarmDlyIndex,1000) //放电单体低压一级报警释放延时 
#define  DCHG_LV_SND_AlARM_REL_DLY   config_rel(kDChgLVSndAlarmDlyIndex,1000) //放电单体低压二级报警释放延时
#define  DCHG_LV_TRD_ALARM_REL_DLY   config_rel(kDChgLVTrdAlarmDlyIndex,1000)//放电低压三级报警释放延时

#define  DCHG_DV_FST_ALARM_REL       config_rel(kDChgDVFstAlarmIndex,-200) //放电单体压差一级报警释放阀值 
#define  DCHG_DV_SND_AlARM_REL       config_rel(kDChgDVSndAlarmIndex,-200) //放电单体压差二级报警释放阀值
#define  DCHG_DV_TRD_ALARM_REL       config_rel(kDChgDVTrdAlarmIndex,-300)//放电压差三级报警释放阀值

#define  DCHG_DV_FST_AlARM_REL_DLY   config_rel(kDChgDVFstAlarmDlyIndex,1000) //放电单体压差一级报警释放延时
#define  DCHG_DV_SND_AlARM_REL_DLY   config_rel(kDChgDVSndAlarmDlyIndex,1000) //放电单体压差二级报警释放延时，保留
#define  DCHG_DV_TRD_ALARM_REL_DLY   config_rel(kDChgDVTrdAlarmDlyIndex,1000)//放电压差三级报警释放延时

#define  VLINE_FST_ALARM_REL_DLY     config_rel(kVLineFstAlarmDlyIndex,-7000) //电压检测排线异常一级报警释放延时

#define  DCHG_HT_FST_ALARM_REL       config_rel(kDChgHTFstAlarmIndex,-10)  //放电高温一级报警释放阈值
#define  DCHG_HT_SND_ALARM_REL       config_rel(kDChgHTSndAlarmIndex,-10) //放电高温二级报警释放阈值
#define  DCHG_HT_TRD_ALARM_REL       config_rel(kDChgHTTrdAlarmIndex,-5)//放电高温三级报警释放阀值

#define  DCHG_HT_FST_ALARM_REL_DLY   config_rel(kDChgHTFstAlarmDlyIndex,1000) //放电高温一级报警释放延时
#define  DCHG_HT_SND_ALARM_REL_DLY   config_rel(kDChgHTSndAlarmDlyIndex,1000) //放电高温二级报警释放延时 
#define  DCHG_HT_TRD_ALARM_REL_DLY   config_rel(kDChgHTTrdAlarmDlyIndex,1000)//放电高温三级报警释放延时

#define  DCHG_LT_FST_ALARM_REL       config_rel(kDChgLTFstAlarmIndex,5)  //放电低温一级报警释放阈值
#define  DCHG_LT_SND_ALARM_REL       config_rel(kDChgLTSndAlarmIndex,10)  //放电低温二级报警释放阀值
#define  DCHG_LT_TRD_ALARM_REL       config_rel(kDChgLTTrdAlarmIndex,15)//放电低温三级报警释放阀值

#define  DCHG_LT_FST_ALARM_REL_DLY   config_rel(kDChgLTFstAlarmDlyIndex,1000)//放电低温一级报警释放延时
#define  DCHG_LT_SND_ALARM_REL_DLY   config_rel(kDChgLTSndAlarmDlyIndex,1000)//放电低温二级报警释放延时
#define  DCHG_LT_TRD_ALARM_REL_DLY   config_rel(kDChgLTTrdAlarmDlyIndex,1000)//放电低温三级报警释放延时

#define  DCHG_DT_FST_ALARM_REL       config_rel(kDChgDTFstAlarmIndex,-5) //放电温差一级报警释放阀值   
#define  DCHG_DT_SND_ALARM_REL       config_rel(kDChgDTSndAlarmIndex,-3) //放电温差二级报警释放阀值
#define  DCHG_DT_TRD_ALARM_REL       config_rel(kDChgDTTrdAlarmIndex,-5)//放电温差三级报警释放阀值

#define  DCHG_DT_FST_ALARM_REL_DLY   config_rel(kDChgDTFstAlarmDlyIndex,1000) //放电温差一级报警释放延时
#define  DCHG_DT_SND_ALARM_REL_DLY   config_rel(kDChgDTSndAlarmDlyIndex,1000) //放电温差二级报警释放延时 
#define  DCHG_DT_TRD_ALARM_REL_DLY   config_rel(kDChgDTTrdAlarmDlyIndex,1000)//放电温差三级报警释放延时
  
#define  TLINE_FST_ALARM_REL_DLY     config_rel(kTLineFstAlarmDlyIndex,-7000) //温度检测异常一级释放延时

#define  DCHG_HTV_FST_ALARM_REL      config_rel(kDChgHTVFstAlarmIndex,-100) //放电总压高一级报警释放阀值 
#define  DCHG_HTV_SND_ALARM_REL      config_rel(kDChgHTVSndAlarmIndex,-50) //放电总压高二级报警释放阀值 
#define  DCHG_HTV_TRD_ALARM_REL      config_rel(kDChgHTVTrdAlarmIndex,-50)//放电总压高三级报警释放阀值

#define  DCHG_HTV_FST_ALARM_REL_DLY  config_rel(kDChgHTVFstAlarmDlyIndex,1000)//放电总压高一级报警释放延时
#define  DCHG_HTV_SND_ALARM_REL_DLY  config_rel(kDChgHTVSndAlarmDlyIndex,1000)//放电总压高二级报警释放延时
#define  DCHG_HTV_TRD_ALARM_REL_DLY  config_rel(kDChgHTVTrdAlarmDlyIndex,1000)//放电总压高三级报警释放延时

#define  DCHG_LTV_FST_ALARM_REL      config_rel(kDChgLTVFstAlarmIndex,100) //放电总压低一级报警释放阀值
#define  DCHG_LTV_SND_ALARM_REL      config_rel(kDChgLTVSndAlarmIndex,100) //放电总压低二级报警释放阀值
#define  DCHG_LTV_TRD_ALARM_REL      config_rel(kDChgLTVTrdAlarmIndex,100)//放电总压低三级报警释放阀值

#define  DCHG_LTV_FST_ALARN_REL_DLY  config_rel(kDChgLTVFstAlarmDlyIndex,1000)//放电总压低一级报警释放延时
#define  DCHG_LTV_SND_ALARM_REL_DLY  config_rel(kDChgLTVSndAlarmDlyIndex,1000)//放电总压低二级报警释放延时
#define  DCHG_LTV_TRD_ALARM_REL_DLY  config_rel(kDChgLTVTrdAlarmDlyIndex,1000)//放电总压低三级报警释放延时

#define  CHG_OC_FST_ALARM_REL        config_rel(kChgOCFstAlarmIndex,0)//充电过流一级报警释放阀值，保留
#define  CHG_OC_SND_ALARM_REL        config_rel(kChgOCSndAlarmIndex,0)//充电过流二级报警释放阀值
#define  CHG_OC_TRD_ALARM_REL        config_rel(kChgOCTrdAlarmIndex,0)//充电过流三级报警释放阀值

#define  CHG_OC_FST_ALARM_REL_DLY    config_rel(kChgOCFstAlarmDlyIndex,1000)//充电过流一级报警释放延时，保留
#define  CHG_OC_SND_ALARM_REL_DLY    config_rel(kChgOCSndAlarmDlyIndex,1000)//充电过流二级报警释放延时
#define  CHG_OC_TRD_ALARM_REL_DLY    config_rel(kChgOCTrdAlarmDlyIndex,1000)//充电过流三级报警释放延时

#define  DCHG_OC_FST_ALARM_REL       config_rel(kDChgOCFstAlarmIndex,0) //放电过流一级报警释放阀值，保留
#define  DCHG_OC_SND_ALARM_REL       config_rel(kDChgOCSndAlarmIndex,0) //放电过流二级报警释放阀值
#define  DCHG_OC_TRD_ALARM_REL       config_rel(kDChgOCTrdAlarmIndex,0) //放电过流三级报警释放阀值

#define  DCHG_OC_FST_ALARM_REL_DLY   config_rel(kDChgOCFstAlarmDlyIndex,1000) //放电过流一级报警释放延时，保留
#define  DCHG_OC_SND_ALARM_REL_DLY   config_rel(kDChgOCSndAlarmDlyIndex,1000)//放电过流二级报警释放延时
#define  DCHG_OC_TRD_ALARM_REL_DLY   config_rel(kDChgOCTrdAlarmDlyIndex,1000)//放电过流三级报警释放延时

#define  HSOC_FST_ALARM_REL          config_rel(kHSOCFstAlarmIndex,-100) //SOC高一级报警释放阀值，保留
#define  HSOC_FST_ALARM_REL_DLY      config_rel(kHSOCFstAlarmDlyIndex,1000)//SOC高一级报警释放延时，保留

#define  LSOC_FST_ALARM_REL          config_rel(kLSOCFstAlarmIndex,500) //SOC低一级报警释放阀值，保留
#define  LSOC_FST_ALARM_REL_DLY      config_rel(kLSOCFstAlarmDlyIndex,1000)//SOC低一级报警释放延时，保留 
#define  LSOC_SND_ALARM_REL          config_rel(kLSOCSndAlarmIndex,500) //SOC低二级报警释放阀值
#define  LSOC_SND_ALARM_REL_DLY      config_rel(kLSOCSndAlarmDlyIndex,1000) //SOC低二级报警释放延时
#define  SOC_LOW_TRD_ALARM_REL       config_rel(kSOCLowTrdAlarmIndex,500)//SOC低三级报警释放阀值
#define  SOC_LOW_TRD_ALARM_REL_DLY   config_rel(kSOCLowTrdAlarmDlyIndex,1000)//SOC低三级报警释放延时

#define  INSU_FST_ALARM_REL          config_rel(kInsuFstAlarmIndex,50) //轻微漏电报警释放
#define  INSU_SND_ALARM_REL          config_rel(kInsuSndAlarmIndex,50)//漏电释放
#define  INSU_TRD_ALARM_REL          config_rel(kInsuTrdAlarmIndex,50) //严重漏电释放

#define  INSU_FST_ALARM_REL_DLY      config_rel(kInsuFstAlarmDlyIndex,1000) //轻微漏电报警释放延时
#define  INSU_SND_ALARM_REL_DLY      config_rel(kInsuSndAlarmDlyIndex,1000)//漏电释放延时
#define  INSU_TRD_ALARM_REL_DLY      config_rel(kInsuTrdAlarmDlyIndex,1000) //严重漏电释放延时

#define  COMM_FST_ALARM_REL_DLY      config_rel(kCommFstAlarmDlyIndex,-10) //TODO:通信中断释放延时

#define  CHG_HT_FST_ALARM_REL        config_rel(kChgHTFstAlarmIndex,-5) //单体温度偏高报警释放阈值
#define  CHG_HT_SND_ALARM_REL        config_rel(kChgHTSndAlarmIndex,-5) //单体过温释放
#define  CHG_HT_TRD_ALARM_REL        config_rel(kChgHTTrdAlarmIndex,-5)//充电高温三级报警释放阀值

#define  CHG_HT_FST_ALARM_REL_DLY    config_rel(kChgHTFstAlarmDlyIndex,1000) //单体温度偏高报警释放阈值延时
#define  CHG_HT_SND_ALARM_REL_DLY    config_rel(kChgHTSndAlarmDlyIndex,1000) //单体过温释放延时
#define  CHG_HT_TRD_ALARM_REL_DLY    config_rel(kChgHTTrdAlarmDlyIndex,1000)//充电高温三级报警释放延时

#define  CHG_HV_FST_ALARM_REL        config_rel(kChgHVFstAlarmIndex,-200) //充电高压一级报警释放阀值
#define  CHG_HV_SND_ALARM_REL        config_rel(kChgHVSndAlarmIndex,-100) //充电高压二级报警释放阀值
#define  CHG_HV_TRD_ALARM_REL        config_rel(kChgHVTrdAlarmIndex,-200)//充电高压三级报警释放阀值

#define  CHG_HV_FST_ALARM_REL_DLY    config_rel(kChgHVFstAlarmDlyIndex,1000)//充电高压一级报警释放延时
#define  CHG_HV_SND_ALARM_REL_DLY    config_rel(kChgHVSndAlarmDlyIndex,1000)//充电高压二级报警释放延时
#define  CHG_HV_TRD_ALARM_REL_DLY    config_rel(kChgHVTrdAlarmDlyIndex,1000)//充电高压三级报警释放延时


#define  CHG_LV_FST_ALARM_REL        config_rel(kChgLVFstAlarmIndex,100)//充电低压一级报警释放阀值
#define  CHG_LV_SND_ALARM_REL        config_rel(kChgLVSndAlarmIndex,200)//充电低压二级报警释放阀值
#define  CHG_LV_TRD_ALARM_REL        config_rel(kChgLVTrdAlarmIndex,100)//充电低压三级报警释放阀值
#define  CHG_LV_FST_ALARM_REL_DLY    config_rel(kChgLVFstAlarmDlyIndex,1000)//充电低压一级报警释放延时
#define  CHG_LV_SND_ALARM_REL_DLY    config_rel(kChgLVSndAlarmDlyIndex,1000)//充电低压二级报警释放延时
#define  CHG_LV_TRD_ALARM_REL_DLY    config_rel(kChgLVTrdAlarmDlyIndex,1000)//充电低压三级报警释放延时

#define  CHG_LT_FST_ALARM_REL        config_rel(kChgLTFstAlarmIndex,5)//充电低温一级报警释放阀值
#define  CHG_LT_SND_ALARM_REL        config_rel(kChgLTSndAlarmIndex,5)//充电低温二级报警释放阀值
#define  CHG_LT_TRD_ALARM_REL        config_rel(kChgLTTrdAlarmIndex,10)//充电低温三级报警释放阀值
#define  CHG_LT_FST_ALARM_REL_DLY    config_rel(kChgLTFstAlarmDlyIndex,1000)//充电低温一级报警释放延时
#define  CHG_LT_SND_ALARM_REL_DLY    config_rel(kChgLTSndAlarmDlyIndex,1000)//充电低温二级报警释放延时
#define  CHG_LT_TRD_ALARM_REL_DLY    config_rel(kChgLTTrdAlarmDlyIndex,1000)//充电低温三级报警释放延时

#define  CHG_DT_FST_ALARM_REL        config_rel(kChgDTFstAlarmIndex,-5)//充电温差一级报警释放阀值
#define  CHG_DT_SND_ALARM_REL        config_rel(kChgDTSndAlarmIndex,-3)//充电温差二级报警释放阀值
#define  CHG_DT_TRD_ALARM_REL        config_rel(kChgDTTrdAlarmIndex,-5)//充电温差三级报警释放阀值
#define  CHG_DT_FST_ALARM_REL_DLY    config_rel(kChgDTFstAlarmDlyIndex,1000)//充电温差一级报警释放延时
#define  CHG_DT_SND_ALARM_REL_DLY    config_rel(kChgDTSndAlarmDlyIndex,1000)//充电温差二级报警释放延时
#define  CHG_DT_TRD_ALARM_REL_DLY    config_rel(kChgDTTrdAlarmDlyIndex,1000)//充电温差三级报警释放延时

#define  CHG_DV_FST_ALARM_REL        config_rel(kChgDVFstAlarmIndex,-100)//充电压差一级报警释放阀值
#define  CHG_DV_SND_ALARM_REL        config_rel(kChgDVSndAlarmIndex,-100)//充电压差二级报警释放阀值
#define  CHG_DV_TRD_ALARM_REL        config_rel(kChgDVTrdAlarmIndex,-100)//充电压差三级报警释放阀值
#define  CHG_DV_FST_ALARM_REL_DLY    config_rel(kChgDVFstAlarmDlyIndex,1000)//充电压差一级报警释放延时
#define  CHG_DV_SND_ALARM_REL_DLY    config_rel(kChgDVSndAlarmDlyIndex,1000)//充电压差二级报警释放延时
#define  CHG_DV_TRD_ALARM_REL_DLY    config_rel(kChgDVTrdAlarmDlyIndex,1000)//充电压差三级报警释放延时

#define  CHG_HTV_FST_ALARM_REL       config_rel(kChgHTVFstAlarmIndex,-100)//充电总压高一级报警释放阀值
#define  CHG_HTV_SND_ALARM_REL       config_rel(kChgHTVSndAlarmIndex,-50)//充电总压高二级报警释放阀值
#define  CHG_HTV_TRD_ALARM_REL       config_rel(kChgHTVTrdAlarmIndex,-50)//充电总压高三级报警释放阀值
#define  CHG_HTV_FST_ALARM_REL_DLY   config_rel(kChgHTVFstAlarmDlyIndex,1000)//充电总压高一级报警释放延时
#define  CHG_HTV_SND_ALARM_REL_DLY   config_rel(kChgHTVSndAlarmDlyIndex,1000)//充电总压高二级报警释放延时
#define  CHG_HTV_TRD_ALARM_REL_DLY   config_rel(kChgHTVTrdAlarmDlyIndex,1000)//充电总压高三级报警释放延时

#define  CHG_LTV_FST_ALARM_REL       config_rel(kChgLTVFstAlarmIndex,100)//充电总压低一级报警释放阀值
#define  CHG_LTV_SND_ALARM_REL       config_rel(kChgLTVSndAlarmIndex,200)//充电总压低二级报警释放阀值
#define  CHG_LTV_TRD_ALARM_REL           config_rel(kChgLTVTrdAlarmIndex,100)//充电总压低三级报警释放阀值
#define  CHG_LTV_FST_ALARM_REL_DLY       config_rel(kChgLTVFstAlarmDlyIndex,1000)//充电总压低一级报警释放延时
#define  CHG_LTV_SND_ALARM_REL_DLY       config_rel(kChgLTVSndAlarmDlyIndex,1000)//充电总压低二级报警释放延时
#define  CHG_LTV_TRD_ALARM_REL_DLY       config_rel(kChgLTVTrdAlarmDlyIndex,1000)//充电总压低三级报警释放延时

#define  CHGR_AC_OUTLET_HT_FST_ALARM_REL config_rel(kChgrAcOutletHTFstAlarmIndex,-10)// 慢充充电插座高温一级报警释放阀值
#define  CHGR_AC_OUTLET_HT_FST_ALARM_REL_DLY config_rel(kChgrAcOutletHTFstAlarmDlyIndex,1000)// 慢充充电插座高温一级报警释放延时
#define  CHGR_AC_OUTLET_HT_SND_ALARM_REL config_rel(kChgrAcOutletHTSndAlarmIndex,-10)// 慢充充电插座高温二级报警释放阀值
#define  CHGR_AC_OUTLET_HT_SND_ALARM_REL_DLY config_rel(kChgrAcOutletHTSndAlarmDlyIndex,1000)// 慢充充电插座高温二级报警释放延时
#define  CHGR_DC_OUTLET_HT_FST_ALARM_REL config_rel(kChgrDcOutletHTFstAlarmIndex,-10)// 快充充电插座高温一级报警释放阀值
#define  CHGR_DC_OUTLET_HT_FST_ALARM_REL_DLY config_rel(kChgrDcOutletHTFstAlarmDlyIndex,1000)// 快充充电插座高温一级报警释放延时
#define  CHGR_DC_OUTLET_HT_SND_ALARM_REL config_rel(kChgrDcOutletHTSndAlarmIndex,-10)// 快充充电插座高温二级报警释放阀值
#define  CHGR_DC_OUTLET_HT_SND_ALARM_REL_DLY config_rel(kChgrDcOutletHTSndAlarmDlyIndex,1000)// 快充充电插座高温二级报警释放延时
#define  CHGR_OUTLET_TEMP_LINE_FST_ALARM_REL_DLY config_rel(kChgrOutletTempLineFstAlarmDlyIndex,-7000)// 充电插座温感排线报警一级释放延时
#define  CHGR_OUTLET_TEMP_LINE_SND_ALARM_REL_DLY config_rel(kChgrOutletTempLineSndAlarmDlyIndex,-17000)// 充电插座温感排线报警二级释放延时
           
typedef enum 
{
/* 0 */     kEepromParamStart=0,
            kEepromFormated=kEepromParamStart,                //TODO:为保持与模板程序兼容放在此处
            kBootloaderParamStart,
            kUserProgramUsedFlagIndex   = kBootloaderParamStart, //Bootloader使用
            kDeviceTypeIndex,                       //设备类型
            kDeviceNumIndex,                        //设备编号
            kUserProgramMajorVerIndex,
            kUserProgramMinorVerIndex,
            kUserProgramRevVerIndex,
            kBootloaderUpdateModeIndex,             //升级模式 0-延时判断模式 1-不升级 2-立即升级模式
            kBootloaderParamEnd, 
            
            //kEepromParamStart= kBootloaderParamEnd,
            kUUIDPart1=kBootloaderParamEnd,                //UUID-128bit
            kUUIDPart2,
/* 10  */   kUUIDPart3,                            
            kUUIDPart4,
            kUUIDPart5,
            kUUIDPart6,
            kUUIDPart7,
            kUUIDPart8,
            kProductDatePart1,              //产品日期 自1970年1月1号流逝的秒数 32位
            kProductDatePart2,

            kCurSenTypeIndex,               //电流传感器型号
            kCchkIndex,                     //小量程电流校准
            
/* 20  */   kDChgHVFstAlarmIndex,           //放电单体高压一级报警阀值
            //kDChgHVFstAlarmRelIndex,        //放电单体高压一级报警释放阀值
            kDChgHVSndAlarmIndex,           //放电单体高压二级报警阀值
            //kDChgHVSndAlarmRelIndex,        //放电单体高压二级报警释放阀值
            kDChgHVFstAlarmDlyIndex,        //放电单体高压一级报警延时 
            //kDChgHVFstAlarmRelDlyIndex,     //放电单体高压一级报警释放延时
            kDChgHVSndAlarmDlyIndex,        //放电单体高压二级报警延时
            //kDChgHVSndAlarmRelDlyIndex,     //放电单体高压二级报警释放延时
            kDChgLVFstAlarmIndex,           //放电单体低压一级报警阀值 
            //kDChgLVFstAlarmRelIndex,        //放电单体低压一级报警释放阀值 
            kDChgLVSndAlarmIndex,           //放电单体低压二级报警阀值
            //kDChgLVSndAlarmRelIndex,        //放电单体低压二级报警释放阀值 
            kDChgLVFstAlarmDlyIndex,        //放电单体低压一级报警延时
            //kDChgLVFstAlarmRelDlyIndex,     //放电单体低压一级报警释放延时 
            kDChgLVSndAlarmDlyIndex,        //放电单体低压二级报警延时 
            //kDChgLVSndAlarmRelDlyIndex,     //放电单体低压二级报警释放延时
            kDChgDVFstAlarmIndex,           //放电单体压差一级报警阀值
            //kDChgDVFstAlarmRelIndex,        //放电单体压差一级报警释放阀值 
            kDChgDVSndAlarmIndex,           //放电单体压差二级报警阀值
            //kDChgDVSndAlarmRelIndex,        //放电单体压差二级报警释放阀值
/* 30  */   kDChgDVFstAlarmDlyIndex,        //放电单体压差一级报警延时
            //kDChgDVFstAlarmRelDlyIndex,     //放电单体压差一级报警释放延时
            kDChgDVSndAlarmDlyIndex,        //放电单体压差二级报警延时，保留
            //kDChgDVSndAlarmRelDlyIndex,     //放电单体压差二级报警释放延时，保留
            kVLineFstAlarmDlyIndex,         //电压检测排线异常一级报警延时
            //kVLineFstAlarmRelDlyIndex,      //电压检测排线异常一级报警释放延时
            kDChgHTFstAlarmIndex,           //放电高温一级报警阈值    
            //kDChgHTFstAlarmRelIndex,        //放电高温一级报警释放阈值        
            kDChgHTSndAlarmIndex,           //放电高温二级报警阈值         
            //kDChgHTSndAlarmRelIndex,        //放电高温二级报警释放阈值
            kDChgHTFstAlarmDlyIndex,        //放电高温一级报警延时
            //kDChgHTFstAlarmRelDlyIndex,     //放电高温一级报警释放延时   
            kDChgHTSndAlarmDlyIndex,        //放电高温二级报警延时        
            //kDChgHTSndAlarmRelDlyIndex,     //放电高温二级报警释放延时   
            kDChgLTFstAlarmIndex,           //放电低温一级报警阈值            
            //kDChgLTFstAlarmRelIndex,        //放电低温一级报警释放阈值      
            kDChgLTSndAlarmIndex,           //放电低温二级报警阀值     
            //kDChgLTSndAlarmRelIndex,        //放电低温二级报警释放阀值
            kDChgLTFstAlarmDlyIndex,        //单体低温报警延时  
            //kDChgLTFstAlarmRelDlyIndex,     //单体低温报警释放延时    
/* 40 */    kDChgLTSndAlarmDlyIndex,        //单体过低温延时
            //kDChgLTSndAlarmRelDlyIndex,     //单体过低温释放延时
            kDChgDTFstAlarmIndex,           //放电温差一级报警阀值
            //kDChgDTFstAlarmRelIndex,        //放电温差一级报警释放阀值      
            kDChgDTSndAlarmIndex,           //放电温差二级报警阀值         
            //kDChgDTSndAlarmRelIndex,        //放电温差二级报警释放阀值         
            kDChgDTFstAlarmDlyIndex,        //放电温差一级报警延时         
            //kDChgDTFstAlarmRelDlyIndex,     //放电温差一级报警释放延时    
            kDChgDTSndAlarmDlyIndex,        //放电温差二级报警延时 
            //kDChgDTSndAlarmRelDlyIndex,     //放电温差二级报警释放延时         
#if 0//!REMOVE_NO_USED_IDX_TO_SAVE_EEPROM
            kRiseTempFstAlarmIndex,         //温升快报警，保留
            kRiseTempFstAlarmRelIndex,      //温升快报警释放，保留
            kRiseTempSndAlarmIndex,         //温升过快，保留
            kRiseTempSndAlarmRelIndex,      //温升过快释放，保留

            kRiseTempFstAlarmTimeIndex,     //温升快报警检测时间，保留
            kRiseTempFstAlarmRelTimeIndex,  //温升快报警释放检测时间，保留
            kRiseTempSndAlarmTimeIndex,     //温升过快检测时间，保留
            kRiseTempSndAlarmRelTimeIndex,  //温升过快释放检测时间，保留

            kFallTempFstAlarmIndex,         //温降快报警，保留
            kFallTempFstAlarmRelIndex,      //温降快报警释放，保留
            kFallTempSndAlarmIndex,         //温降过快，保留
            kFallTempSndAlarmRelIndex,      //温降过快释放，保留

            kFallTempFstAlarmTimeIndex,     //温降快报警检测时间，保留
            kFallTempFstAlarmTimeRelIndex,  //温降快报警释放检测时间，保留
            kFallTempSndAlarmTimeIndex,     //温降过快检测时间，保留
            kFallTempSndAlarmTimeRelIndex,  //温降过快释放检测时间，保留
#endif
            kTLineFstAlarmDlyIndex,         //温度检测异常一级延时      
            //kTLineFstAlarmRelDlyIndex,      //温度检测异常一级释放延时
            kDChgHTVFstAlarmIndex,          //放电总压高一级报警阀值
            //kDChgHTVFstAlarmRelIndex,       //放电总压高一级报警释放阀值    
            kDChgHTVSndAlarmIndex,          //放电总压高二级报警阀值          
            //kDChgHTVSndAlarmRelIndex,       //放电总压高二级报警释放阀值  
            kDChgHTVFstAlarmDlyIndex,       //放电总压高一级报警延时
            //kDChgHTVFstAlarmRelDlyIndex,    //放电总压高一级报警释放延时
            kDChgHTVSndAlarmDlyIndex,       //放电总压高二级报警延时
            //kDChgHTVSndAlarmRelDlyIndex,    //放电总压高二级报警释放延时
/* 50 */    kDChgLTVFstAlarmIndex,          //放电总压低一级报警阀值
            //kDChgLTVFstAlarmRelIndex,       //放电总压低一级报警释放阀值
            kDChgLTVSndAlarmIndex,          //放电总压低二级报警阀值
            //kDChgLTVSndAlarmRelIndex,       //放电总压低二级报警释放阀值 
            kDChgLTVFstAlarmDlyIndex,       //放电总压低一级报警延时
            //kDChgLTVFstAlarmRelDlyIndex,    //放电总压低一级报警释放延时
            kDChgLTVSndAlarmDlyIndex,       //放电总压低二级报警延时
            //kDChgLTVSndAlarmRelDlyIndex,    //放电总压低二级报警释放延时
            kChgOCFstAlarmIndex,            //充电过流一级报警阀值，保留 
            //kChgOCFstAlarmRelIndex,         //充电过流一级报警释放阀值，保留
            kChgOCSndAlarmIndex,            //充电过流二级报警阀值
            //kChgOCSndAlarmRelIndex,         //充电过流二级报警释放阀值
            kChgOCFstAlarmDlyIndex,         //充电过流一级报警延时，保留
            //kChgOCFstAlarmRelDlyIndex,      //充电过流一级报警释放延时，保留
            kChgOCSndAlarmDlyIndex,         //充电过流二级报警延时
            //kChgOCSndAlarmRelDlyIndex,      //充电过流二级报警释放延时
            kDChgOCFstAlarmIndex,           //放电过流一级报警阀值，保留
            //kDChgOCFstAlarmRelIndex,        //放电过流一级报警释放阀值，保留
            kDChgOCSndAlarmIndex,           //放电过流二级报警阀值
            //kDChgOCSndAlarmRelIndex,        //放电过流二级报警释放阀值
/* 60 */    kDChgOCFstAlarmDlyIndex,        //放电过流一级报警延时，保留
            //kDChgOCFstAlarmRelDlyIndex,     //放电过流一级报警释放延时，保留
            kDChgOCSndAlarmDlyIndex,        //放电过流二级报警延时
            //kDChgOCSndAlarmRelDlyIndex,     //放电过流二级报警释放延时
            kHSOCFstAlarmIndex,             //SOC高一级报警阀值，保留 
            //kHSOCFstAlarmRelIndex,          //SOC高一级报警释放阀值，保留        
            //kHSOCSndAlarmIndex,             //SOC高二级报警阀值，保留         
            //kHSOCSndAlarmRelIndex,          //SOC高二级报警释放阀值，保留    
            kHSOCFstAlarmDlyIndex,          //SOC高一级报警延时，保留          
            //kHSOCFstAlarmRelDlyIndex,       //SOC高一级报警释放延时，保留
            //kHSOCSndAlarmDlyIndex,          //SOC高二级报警延时，保留 
            //kHSOCSndAlarmRelDlyIndex,       //SOC高二级报警释放延时，保留
            kLSOCFstAlarmIndex,             //SOC低一级报警阀值，保留 
            //kLSOCFstAlarmRelIndex,          //SOC低一级报警释放阀值，保留
            kLSOCSndAlarmIndex,             //SOC低二级报警阀值，保留
            //kLSOCSndAlarmRelIndex,          //SOC低二级报警释放阀值，保留 
            kLSOCFstAlarmDlyIndex,          //SOC低一级报警延时，保留
            //kLSOCFstAlarmRelDlyIndex,       //SOC低一级报警释放延时，保留  
            kLSOCSndAlarmDlyIndex,          //SOC低二级报警延时，保留
            //kLSOCSndAlarmRelDlyIndex,       //SOC低二级报警释放延时，保留
            kInsuFstAlarmIndex,             //轻微漏电报警
            //kInsuFstAlarmRelIndex,          //轻微漏电报警释放
            kInsuSndAlarmIndex,             //漏电
            //kInsuSndAlarmRelIndex,          //漏电释放
/* 70 */    kInsuTrdAlarmIndex,             //严重漏电
            //kInsuTrdAlarmRelIndex,          //严重漏电释放
            kInsuFstAlarmDlyIndex,          //轻微漏电报警延时
            //kInsuFstAlarmRelDlyIndex,       //轻微漏电报警释放延时
            kInsuSndAlarmDlyIndex,          //漏电延时
            //kInsuSndAlarmRelDlyIndex,       //漏电释放延时
            kInsuTrdAlarmDlyIndex,          //严重漏电延时
            //kInsuTrdAlarmRelDlyIndex,       //严重漏电释放延时
            kCommFstAlarmDlyIndex,          //TODO:通信中断延时
            //kCommFstAlarmRelDlyIndex,       //TODO:通信中断释放延时
            kChgRelayOnDlyIndex,            //充电继电器断开延时
            kChgRelayOffDlyIndex,           //充电继电器闭合延时
            kDisChgRelayOnDlyIndex,         //放电继电器断开延时
            kDisChgRelayOffDlyIndex,        //充电继电器闭合延时
            kPreChgRelayOnDlyIndex,         //预充继电器断开延时，保留
/* 80*/     kPreChgRelayOffDlyIndex,        //预充继电器闭合延时
            //kReserveRelayOnDlyIndex,        //预留继电器断开延时，保留
            //kReserveRelayOffDlyIndex,       //预留继电器闭合延时，保留
            kChargerCurIndex,               //充电机最大充电电流
            kNominalCapIndex,               //额定容量
            kTotalCapIndex,                 //总容量
            kSaveFileAFlag,                 //SD卡中A类文件是否已存储标志
            kChargerVoltIndex,              //充电机最大充电电压
            kChargerProtocolIndex,          //充电机通信协议
            kSlaveNumIndex,                 //从机个数
            //kTemperatureNumIndex,           //温感个数
#if 0//!REMOVE_NO_USED_IDX_TO_SAVE_EEPROM
            kMotorTyremm,                   //轮胎直径 0.1 Inch/bit
            kMotorSpeedRate,                //速比 0.01/bit
            kMotorGearRate,                 //减速箱齿轮比 0.01/bit
            kMotorTyreRate,                 //后轮比 0.01 /bit
#endif
            //kSystemVoltHVIndex,             //系统电压过高报警, mV
            //kSystemVoltHVReleaseIndex,      //系统电压过高报警释放，mV
            //kSystemVoltLVIndex,             //系统电压过低报警 , mV
            //kSystemVoltLVReleaseIndex,      //系统电压过低报警释放，mV

            //kSystemVoltHVDlyIndex,          //系统电压过高报警延时, ms
            //kSystemVoltHVReleaseDlyIndex,   //系统电压过高报警释放延时，ms
            //kSystemVoltLVDlyIndex,          //系统电压过低报警 延时, ms
            //kSystemVoltLVReleaseDlyIndex,   //系统电压过低报警释放延时，ms
            kSystemVoltageCalibration,      // 电压校准值
            kHeatOnTemperature,             //制热开启温度
/* 90 */    kHeatOffTemperature,            //制热关闭温度
            kCoolOnTemperature,             //制冷开启温度
            kCoolOffTemperature,            //制冷关闭温度
            //kHeatOnTempDlyIndex,            //制热开启延时
            //kHeatOffTempDlyIndex,           //制热关闭延时
            //kCoolOnTempDlyIndex,            //制冷开启延时
            //kCoolOffTempDlyIndex,           //制冷关闭延时
            kCapCalibHighAverageVoltage,    //容量校正平均高单体电压阈值
            kCapCalibLowAverageVoltage,     //容量校正平均低单体电压阈值
            kCapCalibLowSoc,                //容量校正低SOC阈值
            kCapCalibLowSocDly,             //校准延时
            //kHmiFrameTimeoutIndex,          //HMI帧超时时间，保留
            //kHmiCommBpsIndex,               //HMI通信波特率，保留
            kDTUSendIntervalIndex,          //DTU发送周期
            //kUpperComputerCanAddrIndex,     //TODO:上位机端CAN地址
            //kBcuCanAddrIndex,               //BCU端CAN地址
            kReqBmuMessageIntervalIndex,    //请求从机信息周期
            kBmuResponseTimeoutIndex,       //bmu响应超时时间
/* 100 */   kCurZeroFilterIndex,            //电流零点滤波范围
            //kCurSampleintervalIndex,        //电流采样周期
            //kCurSampleCntIndex,             //电流采样次数
            kSysStatusSaveIntervalIndex,    //状态数据保存频率
            //kSysStateAvailableDlyIndex,     // 系统状态开始进入可用状态延时
            kBmuBalanceEnableIndex,         //bmu均衡使能控制
            //kWorkStateDlyIndex,             // MCU工作状态延时
            //kAlarmBeepInterval,             //报警蜂鸣器时间间隔
            //kAlarmBeepMaskHigh,             //报警蜂鸣器掩码
            //kAlarmBeepMaskLow,
            kRelayDischargeSamePort,        // 放电同口
            kRelayChargeMutex,              // 充电互斥
            kRelayPreCharge,                // 预充电
            kCfgRelayTypeStart,
            kCfgRelay1Type = kCfgRelayTypeStart,
            kCfgRelay2Type,
            kCfgRelay3Type,
            kCfgRelay4Type,
/* 110 */   kCfgRelay5Type,
            kCfgRelay6Type,
            kCfgRelay7Type,
            kCfgRelay8Type,
            kCfgRelay9Type,
            kCfgRelay10Type,
            kCfgRelayTypeEnd = kCfgRelay10Type,
            kInsulationType,
            //kBatteryTotalNum,
            kNominalTotalVolt,
            //kChargeEfficiencyFactor,
            //kDisChargeEfficiencyFactor,
            kBmuMaxVoltageNumIndex,         // 从机最大电压个数
            kBmuMaxTempNumIndex,            // 从机最大温感个数
/* 120 */   kChgCutOffCurIndex,             //截止充电电流
            kChgCutOffCurDlyIndex,          //截止充电电流持续时间
            kChgTotalTimeMaxIndex,          //最长充电时间
            //kChgFullChgTVReleaseIndex,      //满充释放总压
            kChgHTFstAlarmIndex,            //单体温度偏高报警阈值
            //kChgHTFstAlarmRelIndex,         //单体温度偏高报警释放阈值
            kChgHTSndAlarmIndex,            //单体过温
            //kChgHTSndAlarmRelIndex,         //单体过温释放
            kChgHTFstAlarmDlyIndex,         //单体温度偏高报警阈值延时
            //kChgHTFstAlarmRelDlyIndex,      //单体温度偏高报警释放阈值延时
            kChgHTSndAlarmDlyIndex,         //单体过温延时
            //kChgHTSndAlarmRelDlyIndex,      //单体过温释放延时
            kCustomEeParaOneIndex,          //客户使用eeprom参数1
            kCustomEeParaTwoIndex,          //客户使用eeprom参数2
            kChgHVFstAlarmIndex,            //充电高压一级报警阀值
            //kChgHVFstAlarmRelIndex,         //充电高压一级报警释放阀值
/* 130 */   kChgHVSndAlarmIndex,            //充电高压二级报警阀值
            //kChgHVSndAlarmRelIndex,         //充电高压二级报警释放阀值
            kChgHVTrdAlarmIndex,            //充电高压三级报警阀值
            //kChgHVTrdAlarmRelIndex,         //充电高压三级报警释放阀值
            kChgHVFstAlarmDlyIndex,         //充电高压一级报警延时
            //kChgHVFstAlarmRelDlyIndex,      //充电高压一级报警释放延时
            kChgHVSndAlarmDlyIndex,         //充电高压二级报警延时
            //kChgHVSndAlarmRelDlyIndex,      //充电高压二级报警释放延时
            kChgHVTrdAlarmDlyIndex,         //充电高压三级报警延时
            //kChgHVTrdAlarmRelDlyIndex,      //充电高压三级报警释放延时
            kDChgHVTrdAlarmIndex,           //放电高压三级报警阀值
            //kDChgHVTrdAlarmRelIndex,        //放电高压三级报警释放阀值
            kDChgHVTrdAlarmDlyIndex,        //放电高压三级报警延时
            //kDChgHVTrdAlarmRelDlyIndex,     //放电高压三级报警释放延时
            kChgLVFstAlarmIndex,            //充电低压一级报警阀值
            //kChgLVFstAlarmRelIndex,         //充电低压一级报警释放阀值
            kChgLVSndAlarmIndex,            //充电低压二级报警阀值
            //kChgLVSndAlarmRelIndex,         //充电低压二级报警释放阀值
            kChgLVTrdAlarmIndex,            //充电低压三级报警阀值
            //kChgLVTrdAlarmRelIndex,         //充电低压三级报警释放阀值
/* 140 */   kChgLVFstAlarmDlyIndex,         //充电低压一级报警延时
            //kChgLVFstAlarmRelDlyIndex,      //充电低压一级报警释放延时
            kChgLVSndAlarmDlyIndex,         //充电低压二级报警延时
            //kChgLVSndAlarmRelDlyIndex,      //充电低压二级报警释放延时
            kChgLVTrdAlarmDlyIndex,         //充电低压三级报警延时
            //kChgLVTrdAlarmRelDlyIndex,      //充电低压三级报警释放延时
            kDChgLVTrdAlarmIndex,           //放电低压三级报警阀值
            //kDChgLVTrdAlarmRelIndex,        //放电低压三级报警释放阀值
            kDChgLVTrdAlarmDlyIndex,        //放电低压三级报警延时
            //kDChgLVTrdAlarmRelDlyIndex,     //放电低压三级报警释放延时
            kChgOCTrdAlarmIndex,            //充电过流三级报警阀值
            //kChgOCTrdAlarmRelIndex,         //充电过流三级报警释放阀值
            kChgOCTrdAlarmDlyIndex,         //充电过流三级报警延时
            //kChgOCTrdAlarmRelDlyIndex,      //充电过流三级报警释放延时
            kDChgOCTrdAlarmIndex,           //放电过流三级报警阀值
            //kDChgOCTrdAlarmRelIndex,        //放电过流三级报警释放阀值
            kDChgOCTrdAlarmDlyIndex,        //放电过流三级报警延时
            //kDChgOCTrdAlarmRelDlyIndex,     //放电过流三级报警释放延时
            kChgHTTrdAlarmIndex,            //充电高温三级报警阀值
            //kChgHTTrdAlarmRelIndex,         //充电高温三级报警释放阀值
/* 150 */   kChgHTTrdAlarmDlyIndex,         //充电高温三级报警延时
            //kChgHTTrdAlarmRelDlyIndex,      //充电高温三级报警释放延时
            kDChgHTTrdAlarmIndex,           //放电高温三级报警阀值
            //kDChgHTTrdAlarmRelIndex,        //放电高温三级报警释放阀值
            kDChgHTTrdAlarmDlyIndex,        //放电高温三级报警延时
            //kDChgHTTrdAlarmRelDlyIndex,     //放电高温三级报警释放延时
            kChgLTFstAlarmIndex,            //充电低温一级报警阀值
            //kChgLTFstAlarmRelIndex,         //充电低温一级报警释放阀值
            kChgLTSndAlarmIndex,            //充电低温二级报警阀值
            //kChgLTSndAlarmRelIndex,         //充电低温二级报警释放阀值
            kChgLTTrdAlarmIndex,            //充电低温三级报警阀值
            //kChgLTTrdAlarmRelIndex,         //充电低温三级报警释放阀值
            kChgLTFstAlarmDlyIndex,         //充电低温一级报警延时
            //kChgLTFstAlarmRelDlyIndex,      //充电低温一级报警释放延时
            kChgLTSndAlarmDlyIndex,         //充电低温二级报警延时
            //kChgLTSndAlarmRelDlyIndex,      //充电低温二级报警释放延时
            kChgLTTrdAlarmDlyIndex,         //充电低温三级报警延时
            //kChgLTTrdAlarmRelDlyIndex,      //充电低温三级报警释放延时
            kDChgLTTrdAlarmIndex,           //放电低温三级报警阀值
            //kDChgLTTrdAlarmRelIndex,        //放电低温三级报警释放阀值
/* 160 */   kDChgLTTrdAlarmDlyIndex,        //放电低温三级报警延时
            //kDChgLTTrdAlarmRelDlyIndex,     //放电低温三级报警释放延时
            kChgDTFstAlarmIndex,            //充电温差一级报警阀值
            //kChgDTFstAlarmRelIndex,         //充电温差一级报警释放阀值
            kChgDTSndAlarmIndex,            //充电温差二级报警阀值
            //kChgDTSndAlarmRelIndex,         //充电温差二级报警释放阀值
            kChgDTTrdAlarmIndex,            //充电温差三级报警阀值
            //kChgDTTrdAlarmRelIndex,         //充电温差三级报警释放阀值
            kChgDTFstAlarmDlyIndex,         //充电温差一级报警延时
            //kChgDTFstAlarmRelDlyIndex,      //充电温差一级报警释放延时
            kChgDTSndAlarmDlyIndex,         //充电温差二级报警延时
            //kChgDTSndAlarmRelDlyIndex,      //充电温差二级报警释放延时
            kChgDTTrdAlarmDlyIndex,         //充电温差三级报警延时
            //kChgDTTrdAlarmRelDlyIndex,      //充电温差三级报警释放延时
            kDChgDTTrdAlarmIndex,           //放电温差三级报警阀值
            //kDChgDTTrdAlarmRelIndex,        //放电温差三级报警释放阀值
            kDChgDTTrdAlarmDlyIndex,        //放电温差三级报警延时
            //kDChgDTTrdAlarmRelDlyIndex,     //放电温差三级报警释放延时
            //kCommSndAlarmDlyIndex,          //通信中断二级报警延时
            //kCommSndAlarmRelDlyIndex,       //通信中断二级报警释放延时
            //kCommTrdAlarmDlyIndex,          //通信中断三级报警延时
            //kCommTrdAlarmRelDlyIndex,       //通信中断三级报警释放延时
            //kTempLineSndAlarmDlyIndex,      //温感线异常二级报警延时
            //kTempLineSndAlarmRelDlyIndex,   //温感线异常二级报警释放延时
            //kTempLineTrdAlarmDlyIndex,      //温感线异常三级报警延时
            //kTempLineTrdAlarmRelDlyIndex,   //温感线异常三级报警释放延时
            //kVoltLineSndAlarmDlyIndex,      //电压线异常二级报警延时
            //kVoltLineSndAlarmRelDlyIndex,   //电压线异常二级报警释放延时
            //kVoltLineTrdAlarmDlyIndex,      //电压线异常三级报警延时
            //kVoltLineTrdAlarmRelDlyIndex,   //电压线异常三级报警释放延时
            //kSOCHighTrdAlarmIndex,          //SOC高三级报警阀值
            //kSOCHighTrdAlarmRelIndex,       //SOC高三级报警释放阀值
            //kSOCHighTrdAlarmDlyIndex,       //SOC高三级报警延时
            //kSOCHighTrdAlarmRelDlyIndex,    //SOC高三级报警释放延时
            kSOCLowTrdAlarmIndex,           //SOC低三级报警阀值
            //kSOCLowTrdAlarmRelIndex,        //SOC低三级报警释放阀值
/* 170 */   kSOCLowTrdAlarmDlyIndex,        //SOC低三级报警延时
            //kSOCLowTrdAlarmRelDlyIndex,     //SOC低三级报警释放延时
            kChgDVFstAlarmIndex,            //充电压差一级报警阀值
            //kChgDVFstAlarmRelIndex,         //充电压差一级报警释放阀值
            kChgDVSndAlarmIndex,            //充电压差二级报警阀值
            //kChgDVSndAlarmRelIndex,         //充电压差二级报警释放阀值
            kChgDVTrdAlarmIndex,            //充电压差三级报警阀值
            //kChgDVTrdAlarmRelIndex,         //充电压差三级报警释放阀值
            kChgDVFstAlarmDlyIndex,         //充电压差一级报警延时
            //kChgDVFstAlarmRelDlyIndex,      //充电压差一级报警释放延时
            kChgDVSndAlarmDlyIndex,         //充电压差二级报警延时
            //kChgDVSndAlarmRelDlyIndex,      //充电压差二级报警释放延时
            kChgDVTrdAlarmDlyIndex,         //充电压差三级报警延时
            //kChgDVTrdAlarmRelDlyIndex,      //充电压差三级报警释放延时
            kDChgDVTrdAlarmIndex,           //放电压差三级报警阀值
            //kDChgDVTrdAlarmRelIndex,        //放电压差三级报警释放阀值
            kDChgDVTrdAlarmDlyIndex,        //放电压差三级报警延时
            //kDChgDVTrdAlarmRelDlyIndex,     //放电压差三级报警释放延时
            kChgHTVFstAlarmIndex,           //充电总压高一级报警阀值
            //kChgHTVFstAlarmRelIndex,        //充电总压高一级报警释放阀值
/* 180 */   kChgHTVSndAlarmIndex,           //充电总压高二级报警阀值
            //kChgHTVSndAlarmRelIndex,        //充电总压高二级报警释放阀值
            kChgHTVTrdAlarmIndex,           //充电总压高三级报警阀值
            //kChgHTVTrdAlarmRelIndex,        //充电总压高三级报警释放阀值
            kChgHTVFstAlarmDlyIndex,        //充电总压高一级报警延时
            //kChgHTVFstAlarmRelDlyIndex,     //充电总压高一级报警释放延时
            kChgHTVSndAlarmDlyIndex,        //充电总压高二级报警延时
            //kChgHTVSndAlarmRelDlyIndex,     //充电总压高二级报警释放延时
            kChgHTVTrdAlarmDlyIndex,        //充电总压高三级报警延时
            //kChgHTVTrdAlarmRelDlyIndex,     //充电总压高三级报警释放延时
            kDChgHTVTrdAlarmIndex,          //放电总压高三级报警阀值
            //kDChgHTVTrdAlarmRelIndex,       //放电总压高三级报警释放阀值
            kDChgHTVTrdAlarmDlyIndex,       //放电总压高三级报警延时
            //kDChgHTVTrdAlarmRelDlyIndex,    //放电总压高三级报警释放延时
            kChgLTVFstAlarmIndex,           //充电总压低一级报警阀值
            //kChgLTVFstAlarmRelIndex,        //充电总压低一级报警释放阀值
            kChgLTVSndAlarmIndex,           //充电总压低二级报警阀值
            //kChgLTVSndAlarmRelIndex,        //充电总压低二级报警释放阀值
            kChgLTVTrdAlarmIndex,           //充电总压低三级报警阀值
            //kChgLTVTrdAlarmRelIndex,        //充电总压低三级报警释放阀值
/* 190 */   kChgLTVFstAlarmDlyIndex,        //充电总压低一级报警延时
            //kChgLTVFstAlarmRelDlyIndex,     //充电总压低一级报警释放延时
            kChgLTVSndAlarmDlyIndex,        //充电总压低二级报警延时
            //kChgLTVSndAlarmRelDlyIndex,     //充电总压低二级报警释放延时
            kChgLTVTrdAlarmDlyIndex,        //充电总压低三级报警延时
            //kChgLTVTrdAlarmRelDlyIndex,     //充电总压低三级报警释放延时
            kDChgLTVTrdAlarmIndex,          //放电总压低三级报警阀值
            //kDChgLTVTrdAlarmRelIndex,       //放电总压低三级报警释放阀值
            kDChgLTVTrdAlarmDlyIndex,       //放电总压低三级报警延时
            //kDChgLTVTrdAlarmRelDlyIndex,    //放电总压低三级报警释放延时
            kCAN1BpsIndex,                  //CAN1波特率
            kCAN2BpsIndex,                  //CAN2波特率
            kCAN3BpsIndex,                  //CAN3波特率
            //kRS4851BpsIndex,                //RS485-1波特率
            //kRS4852BpsIndex,                //RS485-2波特率
            kCfgInputTypeStart,
            kCfgInput1TypeIndex = kCfgInputTypeStart, //输入型号1类型
            kCfgInput2TypeIndex,            //输入型号2类型
/* 200 */   kCfgInput3TypeIndex,            //输入型号3类型
            kCfgInput4TypeIndex,            //输入型号4类型
            kCfgInput5TypeIndex,            //输入型号5类型
            kCfgInput6TypeIndex,            //输入型号6类型
            kCfgInput7TypeIndex,            //输入型号8类型
            kCfgInput8TypeIndex,            //输入型号7类型
            kCfgInputTypeEnd = kCfgInput8TypeIndex,
            kBYUVoltTempCnt1Index,              //BYU模块电压温度采集板1串数  高8位为电池数、低8位为温度数
            kBYUVoltTempCnt2Index,              //BYU模块电压温度采集板2串数  高8位为电池数、低8位为温度数
            kBYUVoltTempCnt3Index,              //BYU模块电压温度采集板3串数
            kBYUVoltTempCnt4Index,              //BYU模块电压温度采集板4串数
/* 210 */   kBYUVoltTempCnt5Index,              //BYU模块电压温度采集板5串数
            kBYUVoltTempCnt6Index,              //BYU模块电压温度采集板6串数
            kBYUVoltTempCnt7Index,              //BYU模块电压温度采集板7串数
            kBYUVoltTempCnt8Index,              //BYU模块电压温度采集板8串数
            kBYUVoltTempCnt9Index,              //BYU模块电压温度采集板9串数
            kBYUVoltTempCnt10Index,             //BYU模块电压温度采集板10串数
            kBYUVoltTempCnt11Index,              //BYU模块电压温度采集板11串数
            kBYUVoltTempCnt12Index,              //BYU模块电压温度采集板12串数
            kBYUVoltTempCnt13Index,              //BYU模块电压温度采集板13串数
            kBYUVoltTempCnt14Index,              //BYU模块电压温度采集板14串数
/* 220 */   kBYUVoltTempCnt15Index,              //BYU模块电压温度采集板15串数
            kBYUVoltTempCnt16Index,              //BYU模块电压温度采集板16串数
            kBYUVoltTempCnt17Index,              //BYU模块电压温度采集板17串数
            kBYUVoltTempCnt18Index,              //BYU模块电压温度采集板18串数
            kBYUVoltTempCnt19Index,              //BYU模块电压温度采集板19串数
            kBYUVoltTempCnt20Index,             //BYU模块电压温度采集板20串数
            kBYUVoltTempCnt21Index,              //BYU模块电压温度采集板21串数
            kBYUVoltTempCnt22Index,              //BYU模块电压温度采集板22串数
            kBYUVoltTempCnt23Index,              //BYU模块电压温度采集板23串数
            kBYUVoltTempCnt24Index,              //BYU模块电压温度采集板24串数
/* 230 */   kBYUVoltTempCnt25Index,              //BYU模块电压温度采集板25串数
            kBYUVoltTempCnt26Index,              //BYU模块电压温度采集板26串数
            kBYUVoltTempCnt27Index,              //BYU模块电压温度采集板27串数
            kBYUVoltTempCnt28Index,              //BYU模块电压温度采集板28串数
            kBYUVoltTempCnt29Index,              //BYU模块电压温度采集板29串数
            kBYUVoltTempCnt30Index,             //BYU模块电压温度采集板30串数
            
            kBSU1VoltSampleNoUseBitIndex,        //BSU1模块电压采集跨箱位 bit0~bit12，bit0代表B1-与B1+之间的电池(第一节电池不采集)
            kBSU2VoltSampleNoUseBitIndex,        //BSU2模块电压采集跨箱位
            kBSU3VoltSampleNoUseBitIndex,        //BSU3模块电压采集跨箱位
            kBSU4VoltSampleNoUseBitIndex,        //BSU4模块电压采集跨箱位
/* 240 */   kBSU5VoltSampleNoUseBitIndex,        //BSU5模块电压采集跨箱位
            kBSU6VoltSampleNoUseBitIndex,        //BSU6模块电压采集跨箱位
            kBSU7VoltSampleNoUseBitIndex,        //BSU7模块电压采集跨箱位
            kBSU8VoltSampleNoUseBitIndex,        //BSU8模块电压采集跨箱位
            kBSU9VoltSampleNoUseBitIndex,        //BSU9模块电压采集跨箱位
            kBSU10VoltSampleNoUseBitIndex,       //BSU10模块电压采集跨箱位
            
            kBSU11VoltSampleNoUseBitIndex,        //BSU11模块电压采集跨箱位 bit0~bit12，bit0代表B1-与B1+之间的电池(第一节电池不采集)
            kBSU12VoltSampleNoUseBitIndex,        //BSU12模块电压采集跨箱位
            kBSU13VoltSampleNoUseBitIndex,        //BSU13模块电压采集跨箱位
            kBSU14VoltSampleNoUseBitIndex,        //BSU14模块电压采集跨箱位
/* 250 */   kBSU15VoltSampleNoUseBitIndex,        //BSU15模块电压采集跨箱位
            kBSU16VoltSampleNoUseBitIndex,        //BSU16模块电压采集跨箱位
            kBSU17VoltSampleNoUseBitIndex,        //BSU17模块电压采集跨箱位
            kBSU18VoltSampleNoUseBitIndex,        //BSU18模块电压采集跨箱位
            kBSU19VoltSampleNoUseBitIndex,        //BSU19模块电压采集跨箱位
            kBSU20VoltSampleNoUseBitIndex,        //BSU20模块电压采集跨箱位
            
            kBSU21VoltSampleNoUseBitIndex,        //BSU21模块电压采集跨箱位 bit0~bit12，bit0代表B1-与B1+之间的电池(第一节电池不采集)
            kBSU22VoltSampleNoUseBitIndex,        //BSU22模块电压采集跨箱位
            kBSU23VoltSampleNoUseBitIndex,        //BSU23模块电压采集跨箱位
            kBSU24VoltSampleNoUseBitIndex,        //BSU24模块电压采集跨箱位
/* 260 */   kBSU25VoltSampleNoUseBitIndex,        //BSU25模块电压采集跨箱位
            kBSU26VoltSampleNoUseBitIndex,        //BSU26模块电压采集跨箱位
            kBSU27VoltSampleNoUseBitIndex,        //BSU27模块电压采集跨箱位
            kBSU28VoltSampleNoUseBitIndex,        //BSU28模块电压采集跨箱位
            kBSU29VoltSampleNoUseBitIndex,        //BSU29模块电压采集跨箱位
            kBSU30VoltSampleNoUseBitIndex,        //BSU30模块电压采集跨箱位
            
            kBsuRelayCfgTypeStartIndex,
            kBsuRelay1CfgType = kBsuRelayCfgTypeStartIndex,//S型从机1号继电器类型配置
            kBsuRelay2CfgType,                            //S型2号继电器类型配置
            kBsuRelayCfgTypeEndIndex = kBsuRelay2CfgType,
             
            kBYUVoltCorrectIndex,           //BYU模块电压校正值
            kBYUBalDiffVoltMinIndex,        //BYU模块均衡开启压差
/* 270 */   kBYUBalDiffVoltMaxIndex,        //BYU模块均衡关闭压差
            kBYUBalStartVoltIndex,          //BYU模块均衡开启电压
            kBYUBalNumIndex,                //BYU模块均衡路数
            kBYUTotalBalEnIndex,            //BYU模块整体均衡使能
            kBYUBalTypeIndex,               //BYU模块均衡类型
            kBCUCommModeIndex,              //BCU模块通信模式
            kGBChargerConnectModeIndex,     //国标充电机充电连接确认模式
            kHighCurChkIndex,               //大量程电流传感器校正
            kSlaveCanChannelIndex,          //主从通信CAN通道
            kChargerCanChannelIndex,        //充电机通信CAN通道
/* 280 */   kEmsCanChannelIndex,            //整车通信CAN通道
            //kHmiRS485ChannelIndex,          //屏通信485通道
            //kDtuRS485ChannelIndex,          //DTU通信485通道
            kCurSenRangeTypeIndex,          //电流传感器量程类型
            //kDtuCanChannelIndex,            //DTU通信CAN通道
            kCAN5BpsIndex,                  //CAN5波特率
            kSNNumber1Index,                //SN码编号1
            kSNNumber2Index,                //SN码编号2
            kSNNumber3Index,                //SN码编号3
            kSNNumber4Index,                //SN码编号4
            kSNNumber5Index,                //SN码编号5
            kSNNumber6Index,                //SN码编号6
            kSNNumber7Index,                //SN码编号7
/* 290 */   kHardwareRevisionIndex,         //硬件版本号
            //kEepromTvCheckIndex,
            kEepromTvRangeIndex,
            kEepromNormalCheckCycleIndex,   /*正常状态下绝缘信息更新周期*/
            kEepromInsuResCalibFlagIndex,
            kEepromMosOnDelayIndex,         /*0表示MOS常闭*/
            kEepromInsuResSwitchInChgIndex,
            kEepromInsuCommModeIndex,
            kCurSensorReverseIndex,        
            kChgrAcTempNumIndex,                    // 交流充电插座温感个数
            kChgrDcTempNumIndex,                    // 直流充电插座温感个数
/* 300 */   kChgrAcOutletHTFstAlarmIndex,           // 慢充插座高温一级报警阀值
            //kChgrAcOutletHTFstAlarmRelIndex,        // 慢充充电插座高温一级报警释放阀值
            kChgrAcOutletHTFstAlarmDlyIndex,        // 慢充充电插座高温一级报警延时
            //kChgrAcOutletHTFstAlarmRelDlyIndex,     // 慢充充电插座高温一级报警释放延时
            kChgrAcOutletHTSndAlarmIndex,           // 慢充充电插座高温二级报警阀值
            //kChgrAcOutletHTSndAlarmRelIndex,        // 慢充充电插座高温二级报警释放阀值
            kChgrAcOutletHTSndAlarmDlyIndex,        // 慢充充电插座高温二级报警延时
            //kChgrAcOutletHTSndAlarmRelDlyIndex,     // 慢充充电插座高温二级报警释放延时
            //kChgrAcOutletHTTrdAlarmIndex,           // 慢充充电插座高温三级报警阀值
            //kChgrAcOutletHTTrdAlarmRelIndex,        // 慢充充电插座高温三级报警释放阀值
            //kChgrAcOutletHTTrdAlarmDlyIndex,        // 慢充充电插座高温三级报警延时
            //kChgrAcOutletHTTrdAlarmRelDlyIndex,     // 慢充充电插座高温三级报警释放延时
            kChgrDcOutletHTFstAlarmIndex,           // 快充充电插座高温一级报警阀值
            //kChgrDcOutletHTFstAlarmRelIndex,        // 快充充电插座高温一级报警释放阀值
            kChgrDcOutletHTFstAlarmDlyIndex,        // 快充充电插座高温一级报警延时
            //kChgrDcOutletHTFstAlarmRelDlyIndex,     // 快充充电插座高温一级报警释放延时
            kChgrDcOutletHTSndAlarmIndex,           // 快充充电插座高温二级报警阀值
            //kChgrDcOutletHTSndAlarmRelIndex,        // 快充充电插座高温二级报警释放阀值
            kChgrDcOutletHTSndAlarmDlyIndex,        // 快充充电插座高温二级报警延时
            //kChgrDcOutletHTSndAlarmRelDlyIndex,     // 快充充电插座高温二级报警释放延时
            //kChgrDcOutletHTTrdAlarmIndex,           // 快充充电插座高温三级报警阀值
            //kChgrDcOutletHTTrdAlarmRelIndex,        // 快充充电插座高温三级报警释放阀值
            //kChgrDcOutletHTTrdAlarmDlyIndex,        // 快充充电插座高温三级报警延时
            //kChgrDcOutletHTTrdAlarmRelDlyIndex,     // 快充充电插座高温三级报警释放延时
            kChgrOutletTempLineFstAlarmDlyIndex,    // 充电插座温感排线报警一级延时
            //kChgrOutletTempLineFstAlarmRelDlyIndex, // 充电插座温感排线报警一级释放延时
            kChgrOutletTempLineSndAlarmDlyIndex,    // 充电插座温感排线报警二级延时
            //kChgrOutletTempLineSndAlarmRelDlyIndex, // 充电插座温感排线报警二级释放延时
            //kChgrOutletTempLineTrdAlarmDlyIndex,    // 充电插座温感排线报警三级延时
            //kChgrOutletTempLineTrdAlarmRelDlyIndex, // 充电插座温感排线报警三级释放延时
            
/* 310 */   kVIN1Index,                             //车辆VIN码 17 Bytes
            kVIN2Index,
            kVIN3Index,
            kVIN4Index,
            kVIN5Index,
            kVIN6Index,
            kVIN7Index,
            kVIN8Index,
            kVIN9Index,
            kWakeupIntervalMinutes,                 /** 唤醒参数 */
/* 320 */   kWorkMaxTimeAfterWakeupSeconds,
            
            kMlockType,                             /* 01: busbar, 02: amphenol */
            kMlockParamLength,                      /* Max Length = 8, over part will be cut*/
            kMlockParam1,                           /*Amphenol, lock_unlock_time, busbar, lock_unlock_timeout*/
            kMlockParam2,                           /*Amphenol, unlock_time, busbar, unlock_timeout*/
            kMlockParam3,                           /*Amphenol, 反馈方向*/
            kMlockParam4,                           /**/
            kMlockParam5,                           /**/
            kMlockParam6,
            kMlockParam7,
/* 330 */   kMlockParam8,
            kMlockParamEnd,
            //kReserve1Index,
            //kReserve2Index,
            //kReserve3Index,
            //kReserve4Index,
            //kReserve5Index,
            //kReserve6Index,
            //kReserve7Index,
            //kReserve8Index,
            //kReserve9Index,
            //kReserve10Index,
            //kReserve11Index,
            //kReserve12Index,
            //kReserve13Index,
                          
            kDTUInfoStart=kMlockParamEnd,
            kDTUInfoType = kDTUInfoStart,           // 长度 1 (2字节) 
            //kDTUInfoServer_1,                       // 长度 15(30字节)
            //kDTUInfoServer_2,  
            //kDTUInfoServer_3,
            //kDTUInfoServer_4,
            //kDTUInfoServer_5,
            //kDTUInfoServer_6,
            //kDTUInfoServer_7,
            //kDTUInfoServer_8,
            //kDTUInfoServer_9,
            //kDTUInfoServer_10,
            //kDTUInfoServer_11,
            //kDTUInfoServer_12,
            //kDTUInfoServer_13,
            //kDTUInfoServer_14,
            //kDTUInfoServer_15,
            //kDTUInfoPort,                           // 长度 1 (2字节)
            //kDTUInfoCIMI_1,                         // 长度 11(22字节)
            //kDTUInfoCIMI_2, 
            //kDTUInfoCIMI_3,
            //kDTUInfoCIMI_4,
            //kDTUInfoCIMI_5,
            //kDTUInfoCIMI_6, 
            //kDTUInfoCIMI_7,
            //kDTUInfoCIMI_8,
            //kDTUInfoCIMI_9,
            //kDTUInfoCIMI_10,
            //kDTUInfoCIMI_11,
            //kDTUInfoPhoneNum_1,                     // 长度 6(12字节)
            //kDTUInfoPhoneNum_2,
            //kDTUInfoPhoneNum_3,
            //kDTUInfoPhoneNum_4,
            //kDTUInfoPhoneNum_5,
            //kDTUInfoPhoneNum_6,  
            //kDTUInfoEnd,    
/* 332 */   kCurSenType1Index,          //电流传感器量程类型    
            kOtaOverFlag,
            kOtaErrFlag,  
/**********************动力电池编码信息**********************************/

            
            kcompany_1,                      //企业序号前2字节
            kcompany_2_with_producttype,     //企业序号第3字节和产品类型
            k_batterytype_with_standard1,    //电池类型和规格代码第1字节
            kstandard2_with_retrace_info1,   //规格代码第2字节追溯信息1字节                      
            kretrace_info2,                  //追溯信息2-7字节
/* 340 */   kretrace_info3, 
            kretrace_info4,                  //追溯信息1字节和生产日期1字节
            kproduct_date1,                    //生产日期1-2字节
            kproduct_date2_with_battery_SN1,  //生产日期3字节和序列号1字节
            kbattery_SN2,                     //序列号2-7字节
            kbattery_SN3,
            kbattery_SN4,
            kladder_utilization,              //阶梯利用代码  

/* 348 */   kSocFixAsEmptyIndex, 
            kDetectFlagIndex,       //强检模式标志 
            kCurrentCalibrationChgkIndex,
            kCurrentCalibrationChgbIndex,
            kCurrentCalibrationDchgkIndex,
            kCurrentCalibrationDchgbIndex,
            kCurrentDivideResIndex,           //分流器电阻值
            kCanWkupEnableIndex,              //唤醒使能
            
            kSOCBMAvailableFlag, 
            kSOCBMHigh,
            kSOCBMLow,
  
            kEepromParamEnd
            /** EEEPROM param */
} ConfigIndex;


typedef enum
{
/* 349 */   kEeepromParamStart = kEepromParamEnd,
            kEeepromModifyCntIndex = kEeepromParamStart,
            kLeftCapIndex,
            //kMotorMileLowIndex,
            //kMotorMileHighIndex,
            kCycleStsIndex,                         //电池充放电切换标志
/* 352 */   kCycleCntIndex,                         //电池充放电次数
            kDeepDisChgCntIndex,                    //深度放电次数，保留
            //kSysRunTotalTimeIndex,                //系统总运行时间，保留
            kTotalCapDecimalIndex,
            kOcvSocTimeHighIndex,                   //OCV-SOC时间记录高字节
            kOcvSocTimeLowIndex,                    //OCV-SOC时间记录低字节
            kCustomEeeParaOneIndex,                 //客户使用eeeprom参数1
            kCustomEeeParaTwoIndex,                 //客户使用eeeprom参数2
            kCapAmsSaveLowIndex,                    //剩余容量Ams格式保存
            kCapAmsSaveHighIndex,
            kSOC0DischgFactorIndex,
/* 362 */   kChgTotalCapLowIndex,                   //累计充电电量 
            kChgTotalCapHighIndex, 
            kDChgTotalCapLowIndex,                  //累计放电电量
            kDChgTotalCapHighIndex,
            
            kDeltaCapAMSAfterCalibLowIndex,         // ocv-soc查表后安时积分电量 
            kDeltaCapAMSAfterCalibHighIndex,
            kDeltaCapSign,
            kLeftCapAmsLowIndex,
            kLeftCapAmsHighIndex,
    
/* 366 */   kEeepromParamEnd,
            kSysParamMaxCount = kEeepromParamEnd
}EEEPromConfigIndex;


typedef struct{
    INT8U flag;
    INT16U config_default;
    INT16U config_min;
    INT16U config_max;
}ConfigAttribute;

typedef enum
{
    kModeDelay,
    kModeNotUpdate,
    kModeImmediate
}BtUpdateMode;

#define EEEPROM_INIT_FAIL_BEEP_TIME     100

#define CFG_CHK_RANGE_ON    1   //bit0
#define CFG_CHK_RANGE_OFF   0
#define CFG_SIGN_PARA       2   //bit1
#define CFG_UNSIGN_PARA     0

#define CONFIG_SAVE_END_DELAY    1000 // ms/bit

#define CFG_SAVE_UPPER_LOWER_LIMIT_ERR  0x80
#define CFG_SAVE_WRITE_FLASH_ERR        0x81

//#pragma DATA_SEG __RPAGE_SEG PAGED_RAM
extern const ConfigAttribute g_config_attribute[kSysParamMaxCount];
#if( SWITCH_ON == UDS_SWITCH_CONFIG)
extern INT32U          g_bms_config_save_last_tick;
#endif
//#pragma DATA_SEG DEFAULT

typedef Result (*ConfigChangingHandler)(ConfigIndex index, INT16U new_value);

void config_init(void);
void config_uninit(void);

/**
 * 检查配置是否存在异常，并尝试修复, 本函数在config_init被调用
 */
void config_check(void);
void config_save_all(void);
void config_load_all(void);

void config_set_save_err_flag(INT8U value);
INT8U config_get_save_err_flag(void);
void config_set(ConfigIndex index, INT16U value);
INT16U config_get(ConfigIndex index);
Result config_save(ConfigIndex index, INT16U value);
INT16U config_load(ConfigIndex index);
INT16U config_rel(INT16U index,INT16S diff_value);

Result config_save_dtu_type(unsigned char type);
Result config_get_dtu_type(unsigned char *_PAGED type);
Result config_save_dtu_server(unsigned char *_PAGED server, unsigned short port);
Result config_get_dtu_server(unsigned char *_PAGED server, unsigned char size, INT16U *_PAGED port);
Result config_save_dtu_cimi(unsigned char *_PAGED cimi);
Result config_get_dtu_cimi(unsigned char *_PAGED cimi, unsigned char size);
Result config_save_dtu_phone(unsigned char *_PAGED phone);
Result config_get_dtu_phone(unsigned char *_PAGED phone, unsigned char size);

PINT16U config_items(void);

Result config_register_observer(ConfigIndex start_index, ConfigIndex end_index,
        ConfigChangingHandler handler);

Result config_notify_observer(ConfigIndex index, INT16U value);
void config_eeeprom_init_check(void);

#endif /* BMS_CONFIG_H_ */
