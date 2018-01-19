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

#define  DCHG_HV_FST_ALARM_REL       config_rel(kDChgHVFstAlarmIndex,-200) //�ŵ絥���ѹһ�������ͷŷ�ֵ
#define  DCHG_HV_SND_AlARM_REL       config_rel(kDChgHVSndAlarmIndex,-100) //�ŵ絥���ѹ���������ͷŷ�ֵ
#define  DCHG_HV_TRD_ALARM_REL       config_rel(kDChgHVTrdAlarmIndex,-200)//�ŵ��ѹ���������ͷŷ�ֵ

#define  DCHG_HV_FST_AlARM_REL_DLY   config_rel(kDChgHVFstAlarmDlyIndex,1000) //�ŵ絥���ѹһ�������ͷ���ʱ
#define  DCHG_HV_SND_AlARM_REL_DLY   config_rel(kDChgHVSndAlarmDlyIndex,1000) //�ŵ絥���ѹ���������ͷ���ʱ
#define  DCHG_HV_TRD_ALARM_REL_DLY   config_rel(kDChgHVTrdAlarmDlyIndex,1000)//�ŵ��ѹ���������ͷ���ʱ

#define  DCHG_LV_FST_ALARM_REL       config_rel(kDChgLVFstAlarmIndex,100) //�ŵ絥���ѹһ�������ͷŷ�ֵ 
#define  DCHG_LV_SND_ALARM_REL       config_rel(kDChgLVSndAlarmIndex,200) //�ŵ絥���ѹ���������ͷŷ�ֵ
#define  DCHG_LV_TRD_ALARM_REL       config_rel(kDChgLVTrdAlarmIndex,100)//�ŵ��ѹ���������ͷŷ�ֵ
 
#define  DCHG_LV_FST_AlARM_REL_DLY   config_rel(kDChgLVFstAlarmDlyIndex,1000) //�ŵ絥���ѹһ�������ͷ���ʱ 
#define  DCHG_LV_SND_AlARM_REL_DLY   config_rel(kDChgLVSndAlarmDlyIndex,1000) //�ŵ絥���ѹ���������ͷ���ʱ
#define  DCHG_LV_TRD_ALARM_REL_DLY   config_rel(kDChgLVTrdAlarmDlyIndex,1000)//�ŵ��ѹ���������ͷ���ʱ

#define  DCHG_DV_FST_ALARM_REL       config_rel(kDChgDVFstAlarmIndex,-200) //�ŵ絥��ѹ��һ�������ͷŷ�ֵ 
#define  DCHG_DV_SND_AlARM_REL       config_rel(kDChgDVSndAlarmIndex,-200) //�ŵ絥��ѹ����������ͷŷ�ֵ
#define  DCHG_DV_TRD_ALARM_REL       config_rel(kDChgDVTrdAlarmIndex,-300)//�ŵ�ѹ�����������ͷŷ�ֵ

#define  DCHG_DV_FST_AlARM_REL_DLY   config_rel(kDChgDVFstAlarmDlyIndex,1000) //�ŵ絥��ѹ��һ�������ͷ���ʱ
#define  DCHG_DV_SND_AlARM_REL_DLY   config_rel(kDChgDVSndAlarmDlyIndex,1000) //�ŵ絥��ѹ����������ͷ���ʱ������
#define  DCHG_DV_TRD_ALARM_REL_DLY   config_rel(kDChgDVTrdAlarmDlyIndex,1000)//�ŵ�ѹ�����������ͷ���ʱ

#define  VLINE_FST_ALARM_REL_DLY     config_rel(kVLineFstAlarmDlyIndex,-7000) //��ѹ��������쳣һ�������ͷ���ʱ

#define  DCHG_HT_FST_ALARM_REL       config_rel(kDChgHTFstAlarmIndex,-10)  //�ŵ����һ�������ͷ���ֵ
#define  DCHG_HT_SND_ALARM_REL       config_rel(kDChgHTSndAlarmIndex,-10) //�ŵ���¶��������ͷ���ֵ
#define  DCHG_HT_TRD_ALARM_REL       config_rel(kDChgHTTrdAlarmIndex,-5)//�ŵ�������������ͷŷ�ֵ

#define  DCHG_HT_FST_ALARM_REL_DLY   config_rel(kDChgHTFstAlarmDlyIndex,1000) //�ŵ����һ�������ͷ���ʱ
#define  DCHG_HT_SND_ALARM_REL_DLY   config_rel(kDChgHTSndAlarmDlyIndex,1000) //�ŵ���¶��������ͷ���ʱ 
#define  DCHG_HT_TRD_ALARM_REL_DLY   config_rel(kDChgHTTrdAlarmDlyIndex,1000)//�ŵ�������������ͷ���ʱ

#define  DCHG_LT_FST_ALARM_REL       config_rel(kDChgLTFstAlarmIndex,5)  //�ŵ����һ�������ͷ���ֵ
#define  DCHG_LT_SND_ALARM_REL       config_rel(kDChgLTSndAlarmIndex,10)  //�ŵ���¶��������ͷŷ�ֵ
#define  DCHG_LT_TRD_ALARM_REL       config_rel(kDChgLTTrdAlarmIndex,15)//�ŵ�������������ͷŷ�ֵ

#define  DCHG_LT_FST_ALARM_REL_DLY   config_rel(kDChgLTFstAlarmDlyIndex,1000)//�ŵ����һ�������ͷ���ʱ
#define  DCHG_LT_SND_ALARM_REL_DLY   config_rel(kDChgLTSndAlarmDlyIndex,1000)//�ŵ���¶��������ͷ���ʱ
#define  DCHG_LT_TRD_ALARM_REL_DLY   config_rel(kDChgLTTrdAlarmDlyIndex,1000)//�ŵ�������������ͷ���ʱ

#define  DCHG_DT_FST_ALARM_REL       config_rel(kDChgDTFstAlarmIndex,-5) //�ŵ��²�һ�������ͷŷ�ֵ   
#define  DCHG_DT_SND_ALARM_REL       config_rel(kDChgDTSndAlarmIndex,-3) //�ŵ��²���������ͷŷ�ֵ
#define  DCHG_DT_TRD_ALARM_REL       config_rel(kDChgDTTrdAlarmIndex,-5)//�ŵ��²����������ͷŷ�ֵ

#define  DCHG_DT_FST_ALARM_REL_DLY   config_rel(kDChgDTFstAlarmDlyIndex,1000) //�ŵ��²�һ�������ͷ���ʱ
#define  DCHG_DT_SND_ALARM_REL_DLY   config_rel(kDChgDTSndAlarmDlyIndex,1000) //�ŵ��²���������ͷ���ʱ 
#define  DCHG_DT_TRD_ALARM_REL_DLY   config_rel(kDChgDTTrdAlarmDlyIndex,1000)//�ŵ��²����������ͷ���ʱ
  
#define  TLINE_FST_ALARM_REL_DLY     config_rel(kTLineFstAlarmDlyIndex,-7000) //�¶ȼ���쳣һ���ͷ���ʱ

#define  DCHG_HTV_FST_ALARM_REL      config_rel(kDChgHTVFstAlarmIndex,-100) //�ŵ���ѹ��һ�������ͷŷ�ֵ 
#define  DCHG_HTV_SND_ALARM_REL      config_rel(kDChgHTVSndAlarmIndex,-50) //�ŵ���ѹ�߶��������ͷŷ�ֵ 
#define  DCHG_HTV_TRD_ALARM_REL      config_rel(kDChgHTVTrdAlarmIndex,-50)//�ŵ���ѹ�����������ͷŷ�ֵ

#define  DCHG_HTV_FST_ALARM_REL_DLY  config_rel(kDChgHTVFstAlarmDlyIndex,1000)//�ŵ���ѹ��һ�������ͷ���ʱ
#define  DCHG_HTV_SND_ALARM_REL_DLY  config_rel(kDChgHTVSndAlarmDlyIndex,1000)//�ŵ���ѹ�߶��������ͷ���ʱ
#define  DCHG_HTV_TRD_ALARM_REL_DLY  config_rel(kDChgHTVTrdAlarmDlyIndex,1000)//�ŵ���ѹ�����������ͷ���ʱ

#define  DCHG_LTV_FST_ALARM_REL      config_rel(kDChgLTVFstAlarmIndex,100) //�ŵ���ѹ��һ�������ͷŷ�ֵ
#define  DCHG_LTV_SND_ALARM_REL      config_rel(kDChgLTVSndAlarmIndex,100) //�ŵ���ѹ�Ͷ��������ͷŷ�ֵ
#define  DCHG_LTV_TRD_ALARM_REL      config_rel(kDChgLTVTrdAlarmIndex,100)//�ŵ���ѹ�����������ͷŷ�ֵ

#define  DCHG_LTV_FST_ALARN_REL_DLY  config_rel(kDChgLTVFstAlarmDlyIndex,1000)//�ŵ���ѹ��һ�������ͷ���ʱ
#define  DCHG_LTV_SND_ALARM_REL_DLY  config_rel(kDChgLTVSndAlarmDlyIndex,1000)//�ŵ���ѹ�Ͷ��������ͷ���ʱ
#define  DCHG_LTV_TRD_ALARM_REL_DLY  config_rel(kDChgLTVTrdAlarmDlyIndex,1000)//�ŵ���ѹ�����������ͷ���ʱ

#define  CHG_OC_FST_ALARM_REL        config_rel(kChgOCFstAlarmIndex,0)//������һ�������ͷŷ�ֵ������
#define  CHG_OC_SND_ALARM_REL        config_rel(kChgOCSndAlarmIndex,0)//���������������ͷŷ�ֵ
#define  CHG_OC_TRD_ALARM_REL        config_rel(kChgOCTrdAlarmIndex,0)//���������������ͷŷ�ֵ

#define  CHG_OC_FST_ALARM_REL_DLY    config_rel(kChgOCFstAlarmDlyIndex,1000)//������һ�������ͷ���ʱ������
#define  CHG_OC_SND_ALARM_REL_DLY    config_rel(kChgOCSndAlarmDlyIndex,1000)//���������������ͷ���ʱ
#define  CHG_OC_TRD_ALARM_REL_DLY    config_rel(kChgOCTrdAlarmDlyIndex,1000)//���������������ͷ���ʱ

#define  DCHG_OC_FST_ALARM_REL       config_rel(kDChgOCFstAlarmIndex,0) //�ŵ����һ�������ͷŷ�ֵ������
#define  DCHG_OC_SND_ALARM_REL       config_rel(kDChgOCSndAlarmIndex,0) //�ŵ�������������ͷŷ�ֵ
#define  DCHG_OC_TRD_ALARM_REL       config_rel(kDChgOCTrdAlarmIndex,0) //�ŵ�������������ͷŷ�ֵ

#define  DCHG_OC_FST_ALARM_REL_DLY   config_rel(kDChgOCFstAlarmDlyIndex,1000) //�ŵ����һ�������ͷ���ʱ������
#define  DCHG_OC_SND_ALARM_REL_DLY   config_rel(kDChgOCSndAlarmDlyIndex,1000)//�ŵ�������������ͷ���ʱ
#define  DCHG_OC_TRD_ALARM_REL_DLY   config_rel(kDChgOCTrdAlarmDlyIndex,1000)//�ŵ�������������ͷ���ʱ

#define  HSOC_FST_ALARM_REL          config_rel(kHSOCFstAlarmIndex,-100) //SOC��һ�������ͷŷ�ֵ������
#define  HSOC_FST_ALARM_REL_DLY      config_rel(kHSOCFstAlarmDlyIndex,1000)//SOC��һ�������ͷ���ʱ������

#define  LSOC_FST_ALARM_REL          config_rel(kLSOCFstAlarmIndex,500) //SOC��һ�������ͷŷ�ֵ������
#define  LSOC_FST_ALARM_REL_DLY      config_rel(kLSOCFstAlarmDlyIndex,1000)//SOC��һ�������ͷ���ʱ������ 
#define  LSOC_SND_ALARM_REL          config_rel(kLSOCSndAlarmIndex,500) //SOC�Ͷ��������ͷŷ�ֵ
#define  LSOC_SND_ALARM_REL_DLY      config_rel(kLSOCSndAlarmDlyIndex,1000) //SOC�Ͷ��������ͷ���ʱ
#define  SOC_LOW_TRD_ALARM_REL       config_rel(kSOCLowTrdAlarmIndex,500)//SOC�����������ͷŷ�ֵ
#define  SOC_LOW_TRD_ALARM_REL_DLY   config_rel(kSOCLowTrdAlarmDlyIndex,1000)//SOC�����������ͷ���ʱ

#define  INSU_FST_ALARM_REL          config_rel(kInsuFstAlarmIndex,50) //��΢©�籨���ͷ�
#define  INSU_SND_ALARM_REL          config_rel(kInsuSndAlarmIndex,50)//©���ͷ�
#define  INSU_TRD_ALARM_REL          config_rel(kInsuTrdAlarmIndex,50) //����©���ͷ�

#define  INSU_FST_ALARM_REL_DLY      config_rel(kInsuFstAlarmDlyIndex,1000) //��΢©�籨���ͷ���ʱ
#define  INSU_SND_ALARM_REL_DLY      config_rel(kInsuSndAlarmDlyIndex,1000)//©���ͷ���ʱ
#define  INSU_TRD_ALARM_REL_DLY      config_rel(kInsuTrdAlarmDlyIndex,1000) //����©���ͷ���ʱ

#define  COMM_FST_ALARM_REL_DLY      config_rel(kCommFstAlarmDlyIndex,-10) //TODO:ͨ���ж��ͷ���ʱ

#define  CHG_HT_FST_ALARM_REL        config_rel(kChgHTFstAlarmIndex,-5) //�����¶�ƫ�߱����ͷ���ֵ
#define  CHG_HT_SND_ALARM_REL        config_rel(kChgHTSndAlarmIndex,-5) //��������ͷ�
#define  CHG_HT_TRD_ALARM_REL        config_rel(kChgHTTrdAlarmIndex,-5)//���������������ͷŷ�ֵ

#define  CHG_HT_FST_ALARM_REL_DLY    config_rel(kChgHTFstAlarmDlyIndex,1000) //�����¶�ƫ�߱����ͷ���ֵ��ʱ
#define  CHG_HT_SND_ALARM_REL_DLY    config_rel(kChgHTSndAlarmDlyIndex,1000) //��������ͷ���ʱ
#define  CHG_HT_TRD_ALARM_REL_DLY    config_rel(kChgHTTrdAlarmDlyIndex,1000)//���������������ͷ���ʱ

#define  CHG_HV_FST_ALARM_REL        config_rel(kChgHVFstAlarmIndex,-200) //����ѹһ�������ͷŷ�ֵ
#define  CHG_HV_SND_ALARM_REL        config_rel(kChgHVSndAlarmIndex,-100) //����ѹ���������ͷŷ�ֵ
#define  CHG_HV_TRD_ALARM_REL        config_rel(kChgHVTrdAlarmIndex,-200)//����ѹ���������ͷŷ�ֵ

#define  CHG_HV_FST_ALARM_REL_DLY    config_rel(kChgHVFstAlarmDlyIndex,1000)//����ѹһ�������ͷ���ʱ
#define  CHG_HV_SND_ALARM_REL_DLY    config_rel(kChgHVSndAlarmDlyIndex,1000)//����ѹ���������ͷ���ʱ
#define  CHG_HV_TRD_ALARM_REL_DLY    config_rel(kChgHVTrdAlarmDlyIndex,1000)//����ѹ���������ͷ���ʱ


#define  CHG_LV_FST_ALARM_REL        config_rel(kChgLVFstAlarmIndex,100)//����ѹһ�������ͷŷ�ֵ
#define  CHG_LV_SND_ALARM_REL        config_rel(kChgLVSndAlarmIndex,200)//����ѹ���������ͷŷ�ֵ
#define  CHG_LV_TRD_ALARM_REL        config_rel(kChgLVTrdAlarmIndex,100)//����ѹ���������ͷŷ�ֵ
#define  CHG_LV_FST_ALARM_REL_DLY    config_rel(kChgLVFstAlarmDlyIndex,1000)//����ѹһ�������ͷ���ʱ
#define  CHG_LV_SND_ALARM_REL_DLY    config_rel(kChgLVSndAlarmDlyIndex,1000)//����ѹ���������ͷ���ʱ
#define  CHG_LV_TRD_ALARM_REL_DLY    config_rel(kChgLVTrdAlarmDlyIndex,1000)//����ѹ���������ͷ���ʱ

#define  CHG_LT_FST_ALARM_REL        config_rel(kChgLTFstAlarmIndex,5)//������һ�������ͷŷ�ֵ
#define  CHG_LT_SND_ALARM_REL        config_rel(kChgLTSndAlarmIndex,5)//�����¶��������ͷŷ�ֵ
#define  CHG_LT_TRD_ALARM_REL        config_rel(kChgLTTrdAlarmIndex,10)//���������������ͷŷ�ֵ
#define  CHG_LT_FST_ALARM_REL_DLY    config_rel(kChgLTFstAlarmDlyIndex,1000)//������һ�������ͷ���ʱ
#define  CHG_LT_SND_ALARM_REL_DLY    config_rel(kChgLTSndAlarmDlyIndex,1000)//�����¶��������ͷ���ʱ
#define  CHG_LT_TRD_ALARM_REL_DLY    config_rel(kChgLTTrdAlarmDlyIndex,1000)//���������������ͷ���ʱ

#define  CHG_DT_FST_ALARM_REL        config_rel(kChgDTFstAlarmIndex,-5)//����²�һ�������ͷŷ�ֵ
#define  CHG_DT_SND_ALARM_REL        config_rel(kChgDTSndAlarmIndex,-3)//����²���������ͷŷ�ֵ
#define  CHG_DT_TRD_ALARM_REL        config_rel(kChgDTTrdAlarmIndex,-5)//����²����������ͷŷ�ֵ
#define  CHG_DT_FST_ALARM_REL_DLY    config_rel(kChgDTFstAlarmDlyIndex,1000)//����²�һ�������ͷ���ʱ
#define  CHG_DT_SND_ALARM_REL_DLY    config_rel(kChgDTSndAlarmDlyIndex,1000)//����²���������ͷ���ʱ
#define  CHG_DT_TRD_ALARM_REL_DLY    config_rel(kChgDTTrdAlarmDlyIndex,1000)//����²����������ͷ���ʱ

#define  CHG_DV_FST_ALARM_REL        config_rel(kChgDVFstAlarmIndex,-100)//���ѹ��һ�������ͷŷ�ֵ
#define  CHG_DV_SND_ALARM_REL        config_rel(kChgDVSndAlarmIndex,-100)//���ѹ����������ͷŷ�ֵ
#define  CHG_DV_TRD_ALARM_REL        config_rel(kChgDVTrdAlarmIndex,-100)//���ѹ�����������ͷŷ�ֵ
#define  CHG_DV_FST_ALARM_REL_DLY    config_rel(kChgDVFstAlarmDlyIndex,1000)//���ѹ��һ�������ͷ���ʱ
#define  CHG_DV_SND_ALARM_REL_DLY    config_rel(kChgDVSndAlarmDlyIndex,1000)//���ѹ����������ͷ���ʱ
#define  CHG_DV_TRD_ALARM_REL_DLY    config_rel(kChgDVTrdAlarmDlyIndex,1000)//���ѹ�����������ͷ���ʱ

#define  CHG_HTV_FST_ALARM_REL       config_rel(kChgHTVFstAlarmIndex,-100)//�����ѹ��һ�������ͷŷ�ֵ
#define  CHG_HTV_SND_ALARM_REL       config_rel(kChgHTVSndAlarmIndex,-50)//�����ѹ�߶��������ͷŷ�ֵ
#define  CHG_HTV_TRD_ALARM_REL       config_rel(kChgHTVTrdAlarmIndex,-50)//�����ѹ�����������ͷŷ�ֵ
#define  CHG_HTV_FST_ALARM_REL_DLY   config_rel(kChgHTVFstAlarmDlyIndex,1000)//�����ѹ��һ�������ͷ���ʱ
#define  CHG_HTV_SND_ALARM_REL_DLY   config_rel(kChgHTVSndAlarmDlyIndex,1000)//�����ѹ�߶��������ͷ���ʱ
#define  CHG_HTV_TRD_ALARM_REL_DLY   config_rel(kChgHTVTrdAlarmDlyIndex,1000)//�����ѹ�����������ͷ���ʱ

#define  CHG_LTV_FST_ALARM_REL       config_rel(kChgLTVFstAlarmIndex,100)//�����ѹ��һ�������ͷŷ�ֵ
#define  CHG_LTV_SND_ALARM_REL       config_rel(kChgLTVSndAlarmIndex,200)//�����ѹ�Ͷ��������ͷŷ�ֵ
#define  CHG_LTV_TRD_ALARM_REL           config_rel(kChgLTVTrdAlarmIndex,100)//�����ѹ�����������ͷŷ�ֵ
#define  CHG_LTV_FST_ALARM_REL_DLY       config_rel(kChgLTVFstAlarmDlyIndex,1000)//�����ѹ��һ�������ͷ���ʱ
#define  CHG_LTV_SND_ALARM_REL_DLY       config_rel(kChgLTVSndAlarmDlyIndex,1000)//�����ѹ�Ͷ��������ͷ���ʱ
#define  CHG_LTV_TRD_ALARM_REL_DLY       config_rel(kChgLTVTrdAlarmDlyIndex,1000)//�����ѹ�����������ͷ���ʱ

#define  CHGR_AC_OUTLET_HT_FST_ALARM_REL config_rel(kChgrAcOutletHTFstAlarmIndex,-10)// �������������һ�������ͷŷ�ֵ
#define  CHGR_AC_OUTLET_HT_FST_ALARM_REL_DLY config_rel(kChgrAcOutletHTFstAlarmDlyIndex,1000)// �������������һ�������ͷ���ʱ
#define  CHGR_AC_OUTLET_HT_SND_ALARM_REL config_rel(kChgrAcOutletHTSndAlarmIndex,-10)// ������������¶��������ͷŷ�ֵ
#define  CHGR_AC_OUTLET_HT_SND_ALARM_REL_DLY config_rel(kChgrAcOutletHTSndAlarmDlyIndex,1000)// ������������¶��������ͷ���ʱ
#define  CHGR_DC_OUTLET_HT_FST_ALARM_REL config_rel(kChgrDcOutletHTFstAlarmIndex,-10)// ������������һ�������ͷŷ�ֵ
#define  CHGR_DC_OUTLET_HT_FST_ALARM_REL_DLY config_rel(kChgrDcOutletHTFstAlarmDlyIndex,1000)// ������������һ�������ͷ���ʱ
#define  CHGR_DC_OUTLET_HT_SND_ALARM_REL config_rel(kChgrDcOutletHTSndAlarmIndex,-10)// �����������¶��������ͷŷ�ֵ
#define  CHGR_DC_OUTLET_HT_SND_ALARM_REL_DLY config_rel(kChgrDcOutletHTSndAlarmDlyIndex,1000)// �����������¶��������ͷ���ʱ
#define  CHGR_OUTLET_TEMP_LINE_FST_ALARM_REL_DLY config_rel(kChgrOutletTempLineFstAlarmDlyIndex,-7000)// �������¸����߱���һ���ͷ���ʱ
#define  CHGR_OUTLET_TEMP_LINE_SND_ALARM_REL_DLY config_rel(kChgrOutletTempLineSndAlarmDlyIndex,-17000)// �������¸����߱��������ͷ���ʱ
           
typedef enum 
{
/* 0 */     kEepromParamStart=0,
            kEepromFormated=kEepromParamStart,                //TODO:Ϊ������ģ�������ݷ��ڴ˴�
            kBootloaderParamStart,
            kUserProgramUsedFlagIndex   = kBootloaderParamStart, //Bootloaderʹ��
            kDeviceTypeIndex,                       //�豸����
            kDeviceNumIndex,                        //�豸���
            kUserProgramMajorVerIndex,
            kUserProgramMinorVerIndex,
            kUserProgramRevVerIndex,
            kBootloaderUpdateModeIndex,             //����ģʽ 0-��ʱ�ж�ģʽ 1-������ 2-��������ģʽ
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
            kProductDatePart1,              //��Ʒ���� ��1970��1��1�����ŵ����� 32λ
            kProductDatePart2,

            kCurSenTypeIndex,               //�����������ͺ�
            kCchkIndex,                     //С���̵���У׼
            
/* 20  */   kDChgHVFstAlarmIndex,           //�ŵ絥���ѹһ��������ֵ
            //kDChgHVFstAlarmRelIndex,        //�ŵ絥���ѹһ�������ͷŷ�ֵ
            kDChgHVSndAlarmIndex,           //�ŵ絥���ѹ����������ֵ
            //kDChgHVSndAlarmRelIndex,        //�ŵ絥���ѹ���������ͷŷ�ֵ
            kDChgHVFstAlarmDlyIndex,        //�ŵ絥���ѹһ��������ʱ 
            //kDChgHVFstAlarmRelDlyIndex,     //�ŵ絥���ѹһ�������ͷ���ʱ
            kDChgHVSndAlarmDlyIndex,        //�ŵ絥���ѹ����������ʱ
            //kDChgHVSndAlarmRelDlyIndex,     //�ŵ絥���ѹ���������ͷ���ʱ
            kDChgLVFstAlarmIndex,           //�ŵ絥���ѹһ��������ֵ 
            //kDChgLVFstAlarmRelIndex,        //�ŵ絥���ѹһ�������ͷŷ�ֵ 
            kDChgLVSndAlarmIndex,           //�ŵ絥���ѹ����������ֵ
            //kDChgLVSndAlarmRelIndex,        //�ŵ絥���ѹ���������ͷŷ�ֵ 
            kDChgLVFstAlarmDlyIndex,        //�ŵ絥���ѹһ��������ʱ
            //kDChgLVFstAlarmRelDlyIndex,     //�ŵ絥���ѹһ�������ͷ���ʱ 
            kDChgLVSndAlarmDlyIndex,        //�ŵ絥���ѹ����������ʱ 
            //kDChgLVSndAlarmRelDlyIndex,     //�ŵ絥���ѹ���������ͷ���ʱ
            kDChgDVFstAlarmIndex,           //�ŵ絥��ѹ��һ��������ֵ
            //kDChgDVFstAlarmRelIndex,        //�ŵ絥��ѹ��һ�������ͷŷ�ֵ 
            kDChgDVSndAlarmIndex,           //�ŵ絥��ѹ�����������ֵ
            //kDChgDVSndAlarmRelIndex,        //�ŵ絥��ѹ����������ͷŷ�ֵ
/* 30  */   kDChgDVFstAlarmDlyIndex,        //�ŵ絥��ѹ��һ��������ʱ
            //kDChgDVFstAlarmRelDlyIndex,     //�ŵ絥��ѹ��һ�������ͷ���ʱ
            kDChgDVSndAlarmDlyIndex,        //�ŵ絥��ѹ�����������ʱ������
            //kDChgDVSndAlarmRelDlyIndex,     //�ŵ絥��ѹ����������ͷ���ʱ������
            kVLineFstAlarmDlyIndex,         //��ѹ��������쳣һ��������ʱ
            //kVLineFstAlarmRelDlyIndex,      //��ѹ��������쳣һ�������ͷ���ʱ
            kDChgHTFstAlarmIndex,           //�ŵ����һ��������ֵ    
            //kDChgHTFstAlarmRelIndex,        //�ŵ����һ�������ͷ���ֵ        
            kDChgHTSndAlarmIndex,           //�ŵ���¶���������ֵ         
            //kDChgHTSndAlarmRelIndex,        //�ŵ���¶��������ͷ���ֵ
            kDChgHTFstAlarmDlyIndex,        //�ŵ����һ��������ʱ
            //kDChgHTFstAlarmRelDlyIndex,     //�ŵ����һ�������ͷ���ʱ   
            kDChgHTSndAlarmDlyIndex,        //�ŵ���¶���������ʱ        
            //kDChgHTSndAlarmRelDlyIndex,     //�ŵ���¶��������ͷ���ʱ   
            kDChgLTFstAlarmIndex,           //�ŵ����һ��������ֵ            
            //kDChgLTFstAlarmRelIndex,        //�ŵ����һ�������ͷ���ֵ      
            kDChgLTSndAlarmIndex,           //�ŵ���¶���������ֵ     
            //kDChgLTSndAlarmRelIndex,        //�ŵ���¶��������ͷŷ�ֵ
            kDChgLTFstAlarmDlyIndex,        //������±�����ʱ  
            //kDChgLTFstAlarmRelDlyIndex,     //������±����ͷ���ʱ    
/* 40 */    kDChgLTSndAlarmDlyIndex,        //�����������ʱ
            //kDChgLTSndAlarmRelDlyIndex,     //����������ͷ���ʱ
            kDChgDTFstAlarmIndex,           //�ŵ��²�һ��������ֵ
            //kDChgDTFstAlarmRelIndex,        //�ŵ��²�һ�������ͷŷ�ֵ      
            kDChgDTSndAlarmIndex,           //�ŵ��²����������ֵ         
            //kDChgDTSndAlarmRelIndex,        //�ŵ��²���������ͷŷ�ֵ         
            kDChgDTFstAlarmDlyIndex,        //�ŵ��²�һ��������ʱ         
            //kDChgDTFstAlarmRelDlyIndex,     //�ŵ��²�һ�������ͷ���ʱ    
            kDChgDTSndAlarmDlyIndex,        //�ŵ��²����������ʱ 
            //kDChgDTSndAlarmRelDlyIndex,     //�ŵ��²���������ͷ���ʱ         
#if 0//!REMOVE_NO_USED_IDX_TO_SAVE_EEPROM
            kRiseTempFstAlarmIndex,         //�����챨��������
            kRiseTempFstAlarmRelIndex,      //�����챨���ͷţ�����
            kRiseTempSndAlarmIndex,         //�������죬����
            kRiseTempSndAlarmRelIndex,      //���������ͷţ�����

            kRiseTempFstAlarmTimeIndex,     //�����챨�����ʱ�䣬����
            kRiseTempFstAlarmRelTimeIndex,  //�����챨���ͷż��ʱ�䣬����
            kRiseTempSndAlarmTimeIndex,     //����������ʱ�䣬����
            kRiseTempSndAlarmRelTimeIndex,  //���������ͷż��ʱ�䣬����

            kFallTempFstAlarmIndex,         //�½��챨��������
            kFallTempFstAlarmRelIndex,      //�½��챨���ͷţ�����
            kFallTempSndAlarmIndex,         //�½����죬����
            kFallTempSndAlarmRelIndex,      //�½������ͷţ�����

            kFallTempFstAlarmTimeIndex,     //�½��챨�����ʱ�䣬����
            kFallTempFstAlarmTimeRelIndex,  //�½��챨���ͷż��ʱ�䣬����
            kFallTempSndAlarmTimeIndex,     //�½�������ʱ�䣬����
            kFallTempSndAlarmTimeRelIndex,  //�½������ͷż��ʱ�䣬����
#endif
            kTLineFstAlarmDlyIndex,         //�¶ȼ���쳣һ����ʱ      
            //kTLineFstAlarmRelDlyIndex,      //�¶ȼ���쳣һ���ͷ���ʱ
            kDChgHTVFstAlarmIndex,          //�ŵ���ѹ��һ��������ֵ
            //kDChgHTVFstAlarmRelIndex,       //�ŵ���ѹ��һ�������ͷŷ�ֵ    
            kDChgHTVSndAlarmIndex,          //�ŵ���ѹ�߶���������ֵ          
            //kDChgHTVSndAlarmRelIndex,       //�ŵ���ѹ�߶��������ͷŷ�ֵ  
            kDChgHTVFstAlarmDlyIndex,       //�ŵ���ѹ��һ��������ʱ
            //kDChgHTVFstAlarmRelDlyIndex,    //�ŵ���ѹ��һ�������ͷ���ʱ
            kDChgHTVSndAlarmDlyIndex,       //�ŵ���ѹ�߶���������ʱ
            //kDChgHTVSndAlarmRelDlyIndex,    //�ŵ���ѹ�߶��������ͷ���ʱ
/* 50 */    kDChgLTVFstAlarmIndex,          //�ŵ���ѹ��һ��������ֵ
            //kDChgLTVFstAlarmRelIndex,       //�ŵ���ѹ��һ�������ͷŷ�ֵ
            kDChgLTVSndAlarmIndex,          //�ŵ���ѹ�Ͷ���������ֵ
            //kDChgLTVSndAlarmRelIndex,       //�ŵ���ѹ�Ͷ��������ͷŷ�ֵ 
            kDChgLTVFstAlarmDlyIndex,       //�ŵ���ѹ��һ��������ʱ
            //kDChgLTVFstAlarmRelDlyIndex,    //�ŵ���ѹ��һ�������ͷ���ʱ
            kDChgLTVSndAlarmDlyIndex,       //�ŵ���ѹ�Ͷ���������ʱ
            //kDChgLTVSndAlarmRelDlyIndex,    //�ŵ���ѹ�Ͷ��������ͷ���ʱ
            kChgOCFstAlarmIndex,            //������һ��������ֵ������ 
            //kChgOCFstAlarmRelIndex,         //������һ�������ͷŷ�ֵ������
            kChgOCSndAlarmIndex,            //����������������ֵ
            //kChgOCSndAlarmRelIndex,         //���������������ͷŷ�ֵ
            kChgOCFstAlarmDlyIndex,         //������һ��������ʱ������
            //kChgOCFstAlarmRelDlyIndex,      //������һ�������ͷ���ʱ������
            kChgOCSndAlarmDlyIndex,         //����������������ʱ
            //kChgOCSndAlarmRelDlyIndex,      //���������������ͷ���ʱ
            kDChgOCFstAlarmIndex,           //�ŵ����һ��������ֵ������
            //kDChgOCFstAlarmRelIndex,        //�ŵ����һ�������ͷŷ�ֵ������
            kDChgOCSndAlarmIndex,           //�ŵ��������������ֵ
            //kDChgOCSndAlarmRelIndex,        //�ŵ�������������ͷŷ�ֵ
/* 60 */    kDChgOCFstAlarmDlyIndex,        //�ŵ����һ��������ʱ������
            //kDChgOCFstAlarmRelDlyIndex,     //�ŵ����һ�������ͷ���ʱ������
            kDChgOCSndAlarmDlyIndex,        //�ŵ��������������ʱ
            //kDChgOCSndAlarmRelDlyIndex,     //�ŵ�������������ͷ���ʱ
            kHSOCFstAlarmIndex,             //SOC��һ��������ֵ������ 
            //kHSOCFstAlarmRelIndex,          //SOC��һ�������ͷŷ�ֵ������        
            //kHSOCSndAlarmIndex,             //SOC�߶���������ֵ������         
            //kHSOCSndAlarmRelIndex,          //SOC�߶��������ͷŷ�ֵ������    
            kHSOCFstAlarmDlyIndex,          //SOC��һ��������ʱ������          
            //kHSOCFstAlarmRelDlyIndex,       //SOC��һ�������ͷ���ʱ������
            //kHSOCSndAlarmDlyIndex,          //SOC�߶���������ʱ������ 
            //kHSOCSndAlarmRelDlyIndex,       //SOC�߶��������ͷ���ʱ������
            kLSOCFstAlarmIndex,             //SOC��һ��������ֵ������ 
            //kLSOCFstAlarmRelIndex,          //SOC��һ�������ͷŷ�ֵ������
            kLSOCSndAlarmIndex,             //SOC�Ͷ���������ֵ������
            //kLSOCSndAlarmRelIndex,          //SOC�Ͷ��������ͷŷ�ֵ������ 
            kLSOCFstAlarmDlyIndex,          //SOC��һ��������ʱ������
            //kLSOCFstAlarmRelDlyIndex,       //SOC��һ�������ͷ���ʱ������  
            kLSOCSndAlarmDlyIndex,          //SOC�Ͷ���������ʱ������
            //kLSOCSndAlarmRelDlyIndex,       //SOC�Ͷ��������ͷ���ʱ������
            kInsuFstAlarmIndex,             //��΢©�籨��
            //kInsuFstAlarmRelIndex,          //��΢©�籨���ͷ�
            kInsuSndAlarmIndex,             //©��
            //kInsuSndAlarmRelIndex,          //©���ͷ�
/* 70 */    kInsuTrdAlarmIndex,             //����©��
            //kInsuTrdAlarmRelIndex,          //����©���ͷ�
            kInsuFstAlarmDlyIndex,          //��΢©�籨����ʱ
            //kInsuFstAlarmRelDlyIndex,       //��΢©�籨���ͷ���ʱ
            kInsuSndAlarmDlyIndex,          //©����ʱ
            //kInsuSndAlarmRelDlyIndex,       //©���ͷ���ʱ
            kInsuTrdAlarmDlyIndex,          //����©����ʱ
            //kInsuTrdAlarmRelDlyIndex,       //����©���ͷ���ʱ
            kCommFstAlarmDlyIndex,          //TODO:ͨ���ж���ʱ
            //kCommFstAlarmRelDlyIndex,       //TODO:ͨ���ж��ͷ���ʱ
            kChgRelayOnDlyIndex,            //���̵����Ͽ���ʱ
            kChgRelayOffDlyIndex,           //���̵����պ���ʱ
            kDisChgRelayOnDlyIndex,         //�ŵ�̵����Ͽ���ʱ
            kDisChgRelayOffDlyIndex,        //���̵����պ���ʱ
            kPreChgRelayOnDlyIndex,         //Ԥ��̵����Ͽ���ʱ������
/* 80*/     kPreChgRelayOffDlyIndex,        //Ԥ��̵����պ���ʱ
            //kReserveRelayOnDlyIndex,        //Ԥ���̵����Ͽ���ʱ������
            //kReserveRelayOffDlyIndex,       //Ԥ���̵����պ���ʱ������
            kChargerCurIndex,               //������������
            kNominalCapIndex,               //�����
            kTotalCapIndex,                 //������
            kSaveFileAFlag,                 //SD����A���ļ��Ƿ��Ѵ洢��־
            kChargerVoltIndex,              //����������ѹ
            kChargerProtocolIndex,          //����ͨ��Э��
            kSlaveNumIndex,                 //�ӻ�����
            //kTemperatureNumIndex,           //�¸и���
#if 0//!REMOVE_NO_USED_IDX_TO_SAVE_EEPROM
            kMotorTyremm,                   //��ֱ̥�� 0.1 Inch/bit
            kMotorSpeedRate,                //�ٱ� 0.01/bit
            kMotorGearRate,                 //��������ֱ� 0.01/bit
            kMotorTyreRate,                 //���ֱ� 0.01 /bit
#endif
            //kSystemVoltHVIndex,             //ϵͳ��ѹ���߱���, mV
            //kSystemVoltHVReleaseIndex,      //ϵͳ��ѹ���߱����ͷţ�mV
            //kSystemVoltLVIndex,             //ϵͳ��ѹ���ͱ��� , mV
            //kSystemVoltLVReleaseIndex,      //ϵͳ��ѹ���ͱ����ͷţ�mV

            //kSystemVoltHVDlyIndex,          //ϵͳ��ѹ���߱�����ʱ, ms
            //kSystemVoltHVReleaseDlyIndex,   //ϵͳ��ѹ���߱����ͷ���ʱ��ms
            //kSystemVoltLVDlyIndex,          //ϵͳ��ѹ���ͱ��� ��ʱ, ms
            //kSystemVoltLVReleaseDlyIndex,   //ϵͳ��ѹ���ͱ����ͷ���ʱ��ms
            kSystemVoltageCalibration,      // ��ѹУ׼ֵ
            kHeatOnTemperature,             //���ȿ����¶�
/* 90 */    kHeatOffTemperature,            //���ȹر��¶�
            kCoolOnTemperature,             //���俪���¶�
            kCoolOffTemperature,            //����ر��¶�
            //kHeatOnTempDlyIndex,            //���ȿ�����ʱ
            //kHeatOffTempDlyIndex,           //���ȹر���ʱ
            //kCoolOnTempDlyIndex,            //���俪����ʱ
            //kCoolOffTempDlyIndex,           //����ر���ʱ
            kCapCalibHighAverageVoltage,    //����У��ƽ���ߵ����ѹ��ֵ
            kCapCalibLowAverageVoltage,     //����У��ƽ���͵����ѹ��ֵ
            kCapCalibLowSoc,                //����У����SOC��ֵ
            kCapCalibLowSocDly,             //У׼��ʱ
            //kHmiFrameTimeoutIndex,          //HMI֡��ʱʱ�䣬����
            //kHmiCommBpsIndex,               //HMIͨ�Ų����ʣ�����
            kDTUSendIntervalIndex,          //DTU��������
            //kUpperComputerCanAddrIndex,     //TODO:��λ����CAN��ַ
            //kBcuCanAddrIndex,               //BCU��CAN��ַ
            kReqBmuMessageIntervalIndex,    //����ӻ���Ϣ����
            kBmuResponseTimeoutIndex,       //bmu��Ӧ��ʱʱ��
/* 100 */   kCurZeroFilterIndex,            //��������˲���Χ
            //kCurSampleintervalIndex,        //������������
            //kCurSampleCntIndex,             //������������
            kSysStatusSaveIntervalIndex,    //״̬���ݱ���Ƶ��
            //kSysStateAvailableDlyIndex,     // ϵͳ״̬��ʼ�������״̬��ʱ
            kBmuBalanceEnableIndex,         //bmu����ʹ�ܿ���
            //kWorkStateDlyIndex,             // MCU����״̬��ʱ
            //kAlarmBeepInterval,             //����������ʱ����
            //kAlarmBeepMaskHigh,             //��������������
            //kAlarmBeepMaskLow,
            kRelayDischargeSamePort,        // �ŵ�ͬ��
            kRelayChargeMutex,              // ��绥��
            kRelayPreCharge,                // Ԥ���
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
            kBmuMaxVoltageNumIndex,         // �ӻ�����ѹ����
            kBmuMaxTempNumIndex,            // �ӻ�����¸и���
/* 120 */   kChgCutOffCurIndex,             //��ֹ������
            kChgCutOffCurDlyIndex,          //��ֹ����������ʱ��
            kChgTotalTimeMaxIndex,          //����ʱ��
            //kChgFullChgTVReleaseIndex,      //�����ͷ���ѹ
            kChgHTFstAlarmIndex,            //�����¶�ƫ�߱�����ֵ
            //kChgHTFstAlarmRelIndex,         //�����¶�ƫ�߱����ͷ���ֵ
            kChgHTSndAlarmIndex,            //�������
            //kChgHTSndAlarmRelIndex,         //��������ͷ�
            kChgHTFstAlarmDlyIndex,         //�����¶�ƫ�߱�����ֵ��ʱ
            //kChgHTFstAlarmRelDlyIndex,      //�����¶�ƫ�߱����ͷ���ֵ��ʱ
            kChgHTSndAlarmDlyIndex,         //���������ʱ
            //kChgHTSndAlarmRelDlyIndex,      //��������ͷ���ʱ
            kCustomEeParaOneIndex,          //�ͻ�ʹ��eeprom����1
            kCustomEeParaTwoIndex,          //�ͻ�ʹ��eeprom����2
            kChgHVFstAlarmIndex,            //����ѹһ��������ֵ
            //kChgHVFstAlarmRelIndex,         //����ѹһ�������ͷŷ�ֵ
/* 130 */   kChgHVSndAlarmIndex,            //����ѹ����������ֵ
            //kChgHVSndAlarmRelIndex,         //����ѹ���������ͷŷ�ֵ
            kChgHVTrdAlarmIndex,            //����ѹ����������ֵ
            //kChgHVTrdAlarmRelIndex,         //����ѹ���������ͷŷ�ֵ
            kChgHVFstAlarmDlyIndex,         //����ѹһ��������ʱ
            //kChgHVFstAlarmRelDlyIndex,      //����ѹһ�������ͷ���ʱ
            kChgHVSndAlarmDlyIndex,         //����ѹ����������ʱ
            //kChgHVSndAlarmRelDlyIndex,      //����ѹ���������ͷ���ʱ
            kChgHVTrdAlarmDlyIndex,         //����ѹ����������ʱ
            //kChgHVTrdAlarmRelDlyIndex,      //����ѹ���������ͷ���ʱ
            kDChgHVTrdAlarmIndex,           //�ŵ��ѹ����������ֵ
            //kDChgHVTrdAlarmRelIndex,        //�ŵ��ѹ���������ͷŷ�ֵ
            kDChgHVTrdAlarmDlyIndex,        //�ŵ��ѹ����������ʱ
            //kDChgHVTrdAlarmRelDlyIndex,     //�ŵ��ѹ���������ͷ���ʱ
            kChgLVFstAlarmIndex,            //����ѹһ��������ֵ
            //kChgLVFstAlarmRelIndex,         //����ѹһ�������ͷŷ�ֵ
            kChgLVSndAlarmIndex,            //����ѹ����������ֵ
            //kChgLVSndAlarmRelIndex,         //����ѹ���������ͷŷ�ֵ
            kChgLVTrdAlarmIndex,            //����ѹ����������ֵ
            //kChgLVTrdAlarmRelIndex,         //����ѹ���������ͷŷ�ֵ
/* 140 */   kChgLVFstAlarmDlyIndex,         //����ѹһ��������ʱ
            //kChgLVFstAlarmRelDlyIndex,      //����ѹһ�������ͷ���ʱ
            kChgLVSndAlarmDlyIndex,         //����ѹ����������ʱ
            //kChgLVSndAlarmRelDlyIndex,      //����ѹ���������ͷ���ʱ
            kChgLVTrdAlarmDlyIndex,         //����ѹ����������ʱ
            //kChgLVTrdAlarmRelDlyIndex,      //����ѹ���������ͷ���ʱ
            kDChgLVTrdAlarmIndex,           //�ŵ��ѹ����������ֵ
            //kDChgLVTrdAlarmRelIndex,        //�ŵ��ѹ���������ͷŷ�ֵ
            kDChgLVTrdAlarmDlyIndex,        //�ŵ��ѹ����������ʱ
            //kDChgLVTrdAlarmRelDlyIndex,     //�ŵ��ѹ���������ͷ���ʱ
            kChgOCTrdAlarmIndex,            //����������������ֵ
            //kChgOCTrdAlarmRelIndex,         //���������������ͷŷ�ֵ
            kChgOCTrdAlarmDlyIndex,         //����������������ʱ
            //kChgOCTrdAlarmRelDlyIndex,      //���������������ͷ���ʱ
            kDChgOCTrdAlarmIndex,           //�ŵ��������������ֵ
            //kDChgOCTrdAlarmRelIndex,        //�ŵ�������������ͷŷ�ֵ
            kDChgOCTrdAlarmDlyIndex,        //�ŵ��������������ʱ
            //kDChgOCTrdAlarmRelDlyIndex,     //�ŵ�������������ͷ���ʱ
            kChgHTTrdAlarmIndex,            //����������������ֵ
            //kChgHTTrdAlarmRelIndex,         //���������������ͷŷ�ֵ
/* 150 */   kChgHTTrdAlarmDlyIndex,         //����������������ʱ
            //kChgHTTrdAlarmRelDlyIndex,      //���������������ͷ���ʱ
            kDChgHTTrdAlarmIndex,           //�ŵ��������������ֵ
            //kDChgHTTrdAlarmRelIndex,        //�ŵ�������������ͷŷ�ֵ
            kDChgHTTrdAlarmDlyIndex,        //�ŵ��������������ʱ
            //kDChgHTTrdAlarmRelDlyIndex,     //�ŵ�������������ͷ���ʱ
            kChgLTFstAlarmIndex,            //������һ��������ֵ
            //kChgLTFstAlarmRelIndex,         //������һ�������ͷŷ�ֵ
            kChgLTSndAlarmIndex,            //�����¶���������ֵ
            //kChgLTSndAlarmRelIndex,         //�����¶��������ͷŷ�ֵ
            kChgLTTrdAlarmIndex,            //����������������ֵ
            //kChgLTTrdAlarmRelIndex,         //���������������ͷŷ�ֵ
            kChgLTFstAlarmDlyIndex,         //������һ��������ʱ
            //kChgLTFstAlarmRelDlyIndex,      //������һ�������ͷ���ʱ
            kChgLTSndAlarmDlyIndex,         //�����¶���������ʱ
            //kChgLTSndAlarmRelDlyIndex,      //�����¶��������ͷ���ʱ
            kChgLTTrdAlarmDlyIndex,         //����������������ʱ
            //kChgLTTrdAlarmRelDlyIndex,      //���������������ͷ���ʱ
            kDChgLTTrdAlarmIndex,           //�ŵ��������������ֵ
            //kDChgLTTrdAlarmRelIndex,        //�ŵ�������������ͷŷ�ֵ
/* 160 */   kDChgLTTrdAlarmDlyIndex,        //�ŵ��������������ʱ
            //kDChgLTTrdAlarmRelDlyIndex,     //�ŵ�������������ͷ���ʱ
            kChgDTFstAlarmIndex,            //����²�һ��������ֵ
            //kChgDTFstAlarmRelIndex,         //����²�һ�������ͷŷ�ֵ
            kChgDTSndAlarmIndex,            //����²����������ֵ
            //kChgDTSndAlarmRelIndex,         //����²���������ͷŷ�ֵ
            kChgDTTrdAlarmIndex,            //����²�����������ֵ
            //kChgDTTrdAlarmRelIndex,         //����²����������ͷŷ�ֵ
            kChgDTFstAlarmDlyIndex,         //����²�һ��������ʱ
            //kChgDTFstAlarmRelDlyIndex,      //����²�һ�������ͷ���ʱ
            kChgDTSndAlarmDlyIndex,         //����²����������ʱ
            //kChgDTSndAlarmRelDlyIndex,      //����²���������ͷ���ʱ
            kChgDTTrdAlarmDlyIndex,         //����²�����������ʱ
            //kChgDTTrdAlarmRelDlyIndex,      //����²����������ͷ���ʱ
            kDChgDTTrdAlarmIndex,           //�ŵ��²�����������ֵ
            //kDChgDTTrdAlarmRelIndex,        //�ŵ��²����������ͷŷ�ֵ
            kDChgDTTrdAlarmDlyIndex,        //�ŵ��²�����������ʱ
            //kDChgDTTrdAlarmRelDlyIndex,     //�ŵ��²����������ͷ���ʱ
            //kCommSndAlarmDlyIndex,          //ͨ���ж϶���������ʱ
            //kCommSndAlarmRelDlyIndex,       //ͨ���ж϶��������ͷ���ʱ
            //kCommTrdAlarmDlyIndex,          //ͨ���ж�����������ʱ
            //kCommTrdAlarmRelDlyIndex,       //ͨ���ж����������ͷ���ʱ
            //kTempLineSndAlarmDlyIndex,      //�¸����쳣����������ʱ
            //kTempLineSndAlarmRelDlyIndex,   //�¸����쳣���������ͷ���ʱ
            //kTempLineTrdAlarmDlyIndex,      //�¸����쳣����������ʱ
            //kTempLineTrdAlarmRelDlyIndex,   //�¸����쳣���������ͷ���ʱ
            //kVoltLineSndAlarmDlyIndex,      //��ѹ���쳣����������ʱ
            //kVoltLineSndAlarmRelDlyIndex,   //��ѹ���쳣���������ͷ���ʱ
            //kVoltLineTrdAlarmDlyIndex,      //��ѹ���쳣����������ʱ
            //kVoltLineTrdAlarmRelDlyIndex,   //��ѹ���쳣���������ͷ���ʱ
            //kSOCHighTrdAlarmIndex,          //SOC������������ֵ
            //kSOCHighTrdAlarmRelIndex,       //SOC�����������ͷŷ�ֵ
            //kSOCHighTrdAlarmDlyIndex,       //SOC������������ʱ
            //kSOCHighTrdAlarmRelDlyIndex,    //SOC�����������ͷ���ʱ
            kSOCLowTrdAlarmIndex,           //SOC������������ֵ
            //kSOCLowTrdAlarmRelIndex,        //SOC�����������ͷŷ�ֵ
/* 170 */   kSOCLowTrdAlarmDlyIndex,        //SOC������������ʱ
            //kSOCLowTrdAlarmRelDlyIndex,     //SOC�����������ͷ���ʱ
            kChgDVFstAlarmIndex,            //���ѹ��һ��������ֵ
            //kChgDVFstAlarmRelIndex,         //���ѹ��һ�������ͷŷ�ֵ
            kChgDVSndAlarmIndex,            //���ѹ�����������ֵ
            //kChgDVSndAlarmRelIndex,         //���ѹ����������ͷŷ�ֵ
            kChgDVTrdAlarmIndex,            //���ѹ������������ֵ
            //kChgDVTrdAlarmRelIndex,         //���ѹ�����������ͷŷ�ֵ
            kChgDVFstAlarmDlyIndex,         //���ѹ��һ��������ʱ
            //kChgDVFstAlarmRelDlyIndex,      //���ѹ��һ�������ͷ���ʱ
            kChgDVSndAlarmDlyIndex,         //���ѹ�����������ʱ
            //kChgDVSndAlarmRelDlyIndex,      //���ѹ����������ͷ���ʱ
            kChgDVTrdAlarmDlyIndex,         //���ѹ������������ʱ
            //kChgDVTrdAlarmRelDlyIndex,      //���ѹ�����������ͷ���ʱ
            kDChgDVTrdAlarmIndex,           //�ŵ�ѹ������������ֵ
            //kDChgDVTrdAlarmRelIndex,        //�ŵ�ѹ�����������ͷŷ�ֵ
            kDChgDVTrdAlarmDlyIndex,        //�ŵ�ѹ������������ʱ
            //kDChgDVTrdAlarmRelDlyIndex,     //�ŵ�ѹ�����������ͷ���ʱ
            kChgHTVFstAlarmIndex,           //�����ѹ��һ��������ֵ
            //kChgHTVFstAlarmRelIndex,        //�����ѹ��һ�������ͷŷ�ֵ
/* 180 */   kChgHTVSndAlarmIndex,           //�����ѹ�߶���������ֵ
            //kChgHTVSndAlarmRelIndex,        //�����ѹ�߶��������ͷŷ�ֵ
            kChgHTVTrdAlarmIndex,           //�����ѹ������������ֵ
            //kChgHTVTrdAlarmRelIndex,        //�����ѹ�����������ͷŷ�ֵ
            kChgHTVFstAlarmDlyIndex,        //�����ѹ��һ��������ʱ
            //kChgHTVFstAlarmRelDlyIndex,     //�����ѹ��һ�������ͷ���ʱ
            kChgHTVSndAlarmDlyIndex,        //�����ѹ�߶���������ʱ
            //kChgHTVSndAlarmRelDlyIndex,     //�����ѹ�߶��������ͷ���ʱ
            kChgHTVTrdAlarmDlyIndex,        //�����ѹ������������ʱ
            //kChgHTVTrdAlarmRelDlyIndex,     //�����ѹ�����������ͷ���ʱ
            kDChgHTVTrdAlarmIndex,          //�ŵ���ѹ������������ֵ
            //kDChgHTVTrdAlarmRelIndex,       //�ŵ���ѹ�����������ͷŷ�ֵ
            kDChgHTVTrdAlarmDlyIndex,       //�ŵ���ѹ������������ʱ
            //kDChgHTVTrdAlarmRelDlyIndex,    //�ŵ���ѹ�����������ͷ���ʱ
            kChgLTVFstAlarmIndex,           //�����ѹ��һ��������ֵ
            //kChgLTVFstAlarmRelIndex,        //�����ѹ��һ�������ͷŷ�ֵ
            kChgLTVSndAlarmIndex,           //�����ѹ�Ͷ���������ֵ
            //kChgLTVSndAlarmRelIndex,        //�����ѹ�Ͷ��������ͷŷ�ֵ
            kChgLTVTrdAlarmIndex,           //�����ѹ������������ֵ
            //kChgLTVTrdAlarmRelIndex,        //�����ѹ�����������ͷŷ�ֵ
/* 190 */   kChgLTVFstAlarmDlyIndex,        //�����ѹ��һ��������ʱ
            //kChgLTVFstAlarmRelDlyIndex,     //�����ѹ��һ�������ͷ���ʱ
            kChgLTVSndAlarmDlyIndex,        //�����ѹ�Ͷ���������ʱ
            //kChgLTVSndAlarmRelDlyIndex,     //�����ѹ�Ͷ��������ͷ���ʱ
            kChgLTVTrdAlarmDlyIndex,        //�����ѹ������������ʱ
            //kChgLTVTrdAlarmRelDlyIndex,     //�����ѹ�����������ͷ���ʱ
            kDChgLTVTrdAlarmIndex,          //�ŵ���ѹ������������ֵ
            //kDChgLTVTrdAlarmRelIndex,       //�ŵ���ѹ�����������ͷŷ�ֵ
            kDChgLTVTrdAlarmDlyIndex,       //�ŵ���ѹ������������ʱ
            //kDChgLTVTrdAlarmRelDlyIndex,    //�ŵ���ѹ�����������ͷ���ʱ
            kCAN1BpsIndex,                  //CAN1������
            kCAN2BpsIndex,                  //CAN2������
            kCAN3BpsIndex,                  //CAN3������
            //kRS4851BpsIndex,                //RS485-1������
            //kRS4852BpsIndex,                //RS485-2������
            kCfgInputTypeStart,
            kCfgInput1TypeIndex = kCfgInputTypeStart, //�����ͺ�1����
            kCfgInput2TypeIndex,            //�����ͺ�2����
/* 200 */   kCfgInput3TypeIndex,            //�����ͺ�3����
            kCfgInput4TypeIndex,            //�����ͺ�4����
            kCfgInput5TypeIndex,            //�����ͺ�5����
            kCfgInput6TypeIndex,            //�����ͺ�6����
            kCfgInput7TypeIndex,            //�����ͺ�8����
            kCfgInput8TypeIndex,            //�����ͺ�7����
            kCfgInputTypeEnd = kCfgInput8TypeIndex,
            kBYUVoltTempCnt1Index,              //BYUģ���ѹ�¶Ȳɼ���1����  ��8λΪ���������8λΪ�¶���
            kBYUVoltTempCnt2Index,              //BYUģ���ѹ�¶Ȳɼ���2����  ��8λΪ���������8λΪ�¶���
            kBYUVoltTempCnt3Index,              //BYUģ���ѹ�¶Ȳɼ���3����
            kBYUVoltTempCnt4Index,              //BYUģ���ѹ�¶Ȳɼ���4����
/* 210 */   kBYUVoltTempCnt5Index,              //BYUģ���ѹ�¶Ȳɼ���5����
            kBYUVoltTempCnt6Index,              //BYUģ���ѹ�¶Ȳɼ���6����
            kBYUVoltTempCnt7Index,              //BYUģ���ѹ�¶Ȳɼ���7����
            kBYUVoltTempCnt8Index,              //BYUģ���ѹ�¶Ȳɼ���8����
            kBYUVoltTempCnt9Index,              //BYUģ���ѹ�¶Ȳɼ���9����
            kBYUVoltTempCnt10Index,             //BYUģ���ѹ�¶Ȳɼ���10����
            kBYUVoltTempCnt11Index,              //BYUģ���ѹ�¶Ȳɼ���11����
            kBYUVoltTempCnt12Index,              //BYUģ���ѹ�¶Ȳɼ���12����
            kBYUVoltTempCnt13Index,              //BYUģ���ѹ�¶Ȳɼ���13����
            kBYUVoltTempCnt14Index,              //BYUģ���ѹ�¶Ȳɼ���14����
/* 220 */   kBYUVoltTempCnt15Index,              //BYUģ���ѹ�¶Ȳɼ���15����
            kBYUVoltTempCnt16Index,              //BYUģ���ѹ�¶Ȳɼ���16����
            kBYUVoltTempCnt17Index,              //BYUģ���ѹ�¶Ȳɼ���17����
            kBYUVoltTempCnt18Index,              //BYUģ���ѹ�¶Ȳɼ���18����
            kBYUVoltTempCnt19Index,              //BYUģ���ѹ�¶Ȳɼ���19����
            kBYUVoltTempCnt20Index,             //BYUģ���ѹ�¶Ȳɼ���20����
            kBYUVoltTempCnt21Index,              //BYUģ���ѹ�¶Ȳɼ���21����
            kBYUVoltTempCnt22Index,              //BYUģ���ѹ�¶Ȳɼ���22����
            kBYUVoltTempCnt23Index,              //BYUģ���ѹ�¶Ȳɼ���23����
            kBYUVoltTempCnt24Index,              //BYUģ���ѹ�¶Ȳɼ���24����
/* 230 */   kBYUVoltTempCnt25Index,              //BYUģ���ѹ�¶Ȳɼ���25����
            kBYUVoltTempCnt26Index,              //BYUģ���ѹ�¶Ȳɼ���26����
            kBYUVoltTempCnt27Index,              //BYUģ���ѹ�¶Ȳɼ���27����
            kBYUVoltTempCnt28Index,              //BYUģ���ѹ�¶Ȳɼ���28����
            kBYUVoltTempCnt29Index,              //BYUģ���ѹ�¶Ȳɼ���29����
            kBYUVoltTempCnt30Index,             //BYUģ���ѹ�¶Ȳɼ���30����
            
            kBSU1VoltSampleNoUseBitIndex,        //BSU1ģ���ѹ�ɼ�����λ bit0~bit12��bit0����B1-��B1+֮��ĵ��(��һ�ڵ�ز��ɼ�)
            kBSU2VoltSampleNoUseBitIndex,        //BSU2ģ���ѹ�ɼ�����λ
            kBSU3VoltSampleNoUseBitIndex,        //BSU3ģ���ѹ�ɼ�����λ
            kBSU4VoltSampleNoUseBitIndex,        //BSU4ģ���ѹ�ɼ�����λ
/* 240 */   kBSU5VoltSampleNoUseBitIndex,        //BSU5ģ���ѹ�ɼ�����λ
            kBSU6VoltSampleNoUseBitIndex,        //BSU6ģ���ѹ�ɼ�����λ
            kBSU7VoltSampleNoUseBitIndex,        //BSU7ģ���ѹ�ɼ�����λ
            kBSU8VoltSampleNoUseBitIndex,        //BSU8ģ���ѹ�ɼ�����λ
            kBSU9VoltSampleNoUseBitIndex,        //BSU9ģ���ѹ�ɼ�����λ
            kBSU10VoltSampleNoUseBitIndex,       //BSU10ģ���ѹ�ɼ�����λ
            
            kBSU11VoltSampleNoUseBitIndex,        //BSU11ģ���ѹ�ɼ�����λ bit0~bit12��bit0����B1-��B1+֮��ĵ��(��һ�ڵ�ز��ɼ�)
            kBSU12VoltSampleNoUseBitIndex,        //BSU12ģ���ѹ�ɼ�����λ
            kBSU13VoltSampleNoUseBitIndex,        //BSU13ģ���ѹ�ɼ�����λ
            kBSU14VoltSampleNoUseBitIndex,        //BSU14ģ���ѹ�ɼ�����λ
/* 250 */   kBSU15VoltSampleNoUseBitIndex,        //BSU15ģ���ѹ�ɼ�����λ
            kBSU16VoltSampleNoUseBitIndex,        //BSU16ģ���ѹ�ɼ�����λ
            kBSU17VoltSampleNoUseBitIndex,        //BSU17ģ���ѹ�ɼ�����λ
            kBSU18VoltSampleNoUseBitIndex,        //BSU18ģ���ѹ�ɼ�����λ
            kBSU19VoltSampleNoUseBitIndex,        //BSU19ģ���ѹ�ɼ�����λ
            kBSU20VoltSampleNoUseBitIndex,        //BSU20ģ���ѹ�ɼ�����λ
            
            kBSU21VoltSampleNoUseBitIndex,        //BSU21ģ���ѹ�ɼ�����λ bit0~bit12��bit0����B1-��B1+֮��ĵ��(��һ�ڵ�ز��ɼ�)
            kBSU22VoltSampleNoUseBitIndex,        //BSU22ģ���ѹ�ɼ�����λ
            kBSU23VoltSampleNoUseBitIndex,        //BSU23ģ���ѹ�ɼ�����λ
            kBSU24VoltSampleNoUseBitIndex,        //BSU24ģ���ѹ�ɼ�����λ
/* 260 */   kBSU25VoltSampleNoUseBitIndex,        //BSU25ģ���ѹ�ɼ�����λ
            kBSU26VoltSampleNoUseBitIndex,        //BSU26ģ���ѹ�ɼ�����λ
            kBSU27VoltSampleNoUseBitIndex,        //BSU27ģ���ѹ�ɼ�����λ
            kBSU28VoltSampleNoUseBitIndex,        //BSU28ģ���ѹ�ɼ�����λ
            kBSU29VoltSampleNoUseBitIndex,        //BSU29ģ���ѹ�ɼ�����λ
            kBSU30VoltSampleNoUseBitIndex,        //BSU30ģ���ѹ�ɼ�����λ
            
            kBsuRelayCfgTypeStartIndex,
            kBsuRelay1CfgType = kBsuRelayCfgTypeStartIndex,//S�ʹӻ�1�ż̵�����������
            kBsuRelay2CfgType,                            //S��2�ż̵�����������
            kBsuRelayCfgTypeEndIndex = kBsuRelay2CfgType,
             
            kBYUVoltCorrectIndex,           //BYUģ���ѹУ��ֵ
            kBYUBalDiffVoltMinIndex,        //BYUģ����⿪��ѹ��
/* 270 */   kBYUBalDiffVoltMaxIndex,        //BYUģ�����ر�ѹ��
            kBYUBalStartVoltIndex,          //BYUģ����⿪����ѹ
            kBYUBalNumIndex,                //BYUģ�����·��
            kBYUTotalBalEnIndex,            //BYUģ���������ʹ��
            kBYUBalTypeIndex,               //BYUģ���������
            kBCUCommModeIndex,              //BCUģ��ͨ��ģʽ
            kGBChargerConnectModeIndex,     //��������������ȷ��ģʽ
            kHighCurChkIndex,               //�����̵���������У��
            kSlaveCanChannelIndex,          //����ͨ��CANͨ��
            kChargerCanChannelIndex,        //����ͨ��CANͨ��
/* 280 */   kEmsCanChannelIndex,            //����ͨ��CANͨ��
            //kHmiRS485ChannelIndex,          //��ͨ��485ͨ��
            //kDtuRS485ChannelIndex,          //DTUͨ��485ͨ��
            kCurSenRangeTypeIndex,          //������������������
            //kDtuCanChannelIndex,            //DTUͨ��CANͨ��
            kCAN5BpsIndex,                  //CAN5������
            kSNNumber1Index,                //SN����1
            kSNNumber2Index,                //SN����2
            kSNNumber3Index,                //SN����3
            kSNNumber4Index,                //SN����4
            kSNNumber5Index,                //SN����5
            kSNNumber6Index,                //SN����6
            kSNNumber7Index,                //SN����7
/* 290 */   kHardwareRevisionIndex,         //Ӳ���汾��
            //kEepromTvCheckIndex,
            kEepromTvRangeIndex,
            kEepromNormalCheckCycleIndex,   /*����״̬�¾�Ե��Ϣ��������*/
            kEepromInsuResCalibFlagIndex,
            kEepromMosOnDelayIndex,         /*0��ʾMOS����*/
            kEepromInsuResSwitchInChgIndex,
            kEepromInsuCommModeIndex,
            kCurSensorReverseIndex,        
            kChgrAcTempNumIndex,                    // �����������¸и���
            kChgrDcTempNumIndex,                    // ֱ���������¸и���
/* 300 */   kChgrAcOutletHTFstAlarmIndex,           // �����������һ��������ֵ
            //kChgrAcOutletHTFstAlarmRelIndex,        // �������������һ�������ͷŷ�ֵ
            kChgrAcOutletHTFstAlarmDlyIndex,        // �������������һ��������ʱ
            //kChgrAcOutletHTFstAlarmRelDlyIndex,     // �������������һ�������ͷ���ʱ
            kChgrAcOutletHTSndAlarmIndex,           // ������������¶���������ֵ
            //kChgrAcOutletHTSndAlarmRelIndex,        // ������������¶��������ͷŷ�ֵ
            kChgrAcOutletHTSndAlarmDlyIndex,        // ������������¶���������ʱ
            //kChgrAcOutletHTSndAlarmRelDlyIndex,     // ������������¶��������ͷ���ʱ
            //kChgrAcOutletHTTrdAlarmIndex,           // �����������������������ֵ
            //kChgrAcOutletHTTrdAlarmRelIndex,        // ����������������������ͷŷ�ֵ
            //kChgrAcOutletHTTrdAlarmDlyIndex,        // �����������������������ʱ
            //kChgrAcOutletHTTrdAlarmRelDlyIndex,     // ����������������������ͷ���ʱ
            kChgrDcOutletHTFstAlarmIndex,           // ������������һ��������ֵ
            //kChgrDcOutletHTFstAlarmRelIndex,        // ������������һ�������ͷŷ�ֵ
            kChgrDcOutletHTFstAlarmDlyIndex,        // ������������һ��������ʱ
            //kChgrDcOutletHTFstAlarmRelDlyIndex,     // ������������һ�������ͷ���ʱ
            kChgrDcOutletHTSndAlarmIndex,           // �����������¶���������ֵ
            //kChgrDcOutletHTSndAlarmRelIndex,        // �����������¶��������ͷŷ�ֵ
            kChgrDcOutletHTSndAlarmDlyIndex,        // �����������¶���������ʱ
            //kChgrDcOutletHTSndAlarmRelDlyIndex,     // �����������¶��������ͷ���ʱ
            //kChgrDcOutletHTTrdAlarmIndex,           // ����������������������ֵ
            //kChgrDcOutletHTTrdAlarmRelIndex,        // ���������������������ͷŷ�ֵ
            //kChgrDcOutletHTTrdAlarmDlyIndex,        // ����������������������ʱ
            //kChgrDcOutletHTTrdAlarmRelDlyIndex,     // ���������������������ͷ���ʱ
            kChgrOutletTempLineFstAlarmDlyIndex,    // �������¸����߱���һ����ʱ
            //kChgrOutletTempLineFstAlarmRelDlyIndex, // �������¸����߱���һ���ͷ���ʱ
            kChgrOutletTempLineSndAlarmDlyIndex,    // �������¸����߱���������ʱ
            //kChgrOutletTempLineSndAlarmRelDlyIndex, // �������¸����߱��������ͷ���ʱ
            //kChgrOutletTempLineTrdAlarmDlyIndex,    // �������¸����߱���������ʱ
            //kChgrOutletTempLineTrdAlarmRelDlyIndex, // �������¸����߱��������ͷ���ʱ
            
/* 310 */   kVIN1Index,                             //����VIN�� 17 Bytes
            kVIN2Index,
            kVIN3Index,
            kVIN4Index,
            kVIN5Index,
            kVIN6Index,
            kVIN7Index,
            kVIN8Index,
            kVIN9Index,
            kWakeupIntervalMinutes,                 /** ���Ѳ��� */
/* 320 */   kWorkMaxTimeAfterWakeupSeconds,
            
            kMlockType,                             /* 01: busbar, 02: amphenol */
            kMlockParamLength,                      /* Max Length = 8, over part will be cut*/
            kMlockParam1,                           /*Amphenol, lock_unlock_time, busbar, lock_unlock_timeout*/
            kMlockParam2,                           /*Amphenol, unlock_time, busbar, unlock_timeout*/
            kMlockParam3,                           /*Amphenol, ��������*/
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
            kDTUInfoType = kDTUInfoStart,           // ���� 1 (2�ֽ�) 
            //kDTUInfoServer_1,                       // ���� 15(30�ֽ�)
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
            //kDTUInfoPort,                           // ���� 1 (2�ֽ�)
            //kDTUInfoCIMI_1,                         // ���� 11(22�ֽ�)
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
            //kDTUInfoPhoneNum_1,                     // ���� 6(12�ֽ�)
            //kDTUInfoPhoneNum_2,
            //kDTUInfoPhoneNum_3,
            //kDTUInfoPhoneNum_4,
            //kDTUInfoPhoneNum_5,
            //kDTUInfoPhoneNum_6,  
            //kDTUInfoEnd,    
/* 332 */   kCurSenType1Index,          //������������������    
            kOtaOverFlag,
            kOtaErrFlag,  
/**********************������ر�����Ϣ**********************************/

            
            kcompany_1,                      //��ҵ���ǰ2�ֽ�
            kcompany_2_with_producttype,     //��ҵ��ŵ�3�ֽںͲ�Ʒ����
            k_batterytype_with_standard1,    //������ͺ͹������1�ֽ�
            kstandard2_with_retrace_info1,   //�������2�ֽ�׷����Ϣ1�ֽ�                      
            kretrace_info2,                  //׷����Ϣ2-7�ֽ�
/* 340 */   kretrace_info3, 
            kretrace_info4,                  //׷����Ϣ1�ֽں���������1�ֽ�
            kproduct_date1,                    //��������1-2�ֽ�
            kproduct_date2_with_battery_SN1,  //��������3�ֽں����к�1�ֽ�
            kbattery_SN2,                     //���к�2-7�ֽ�
            kbattery_SN3,
            kbattery_SN4,
            kladder_utilization,              //�������ô���  

/* 348 */   kSocFixAsEmptyIndex, 
            kDetectFlagIndex,       //ǿ��ģʽ��־ 
            kCurrentCalibrationChgkIndex,
            kCurrentCalibrationChgbIndex,
            kCurrentCalibrationDchgkIndex,
            kCurrentCalibrationDchgbIndex,
            kCurrentDivideResIndex,           //����������ֵ
            kCanWkupEnableIndex,              //����ʹ��
            
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
            kCycleStsIndex,                         //��س�ŵ��л���־
/* 352 */   kCycleCntIndex,                         //��س�ŵ����
            kDeepDisChgCntIndex,                    //��ȷŵ����������
            //kSysRunTotalTimeIndex,                //ϵͳ������ʱ�䣬����
            kTotalCapDecimalIndex,
            kOcvSocTimeHighIndex,                   //OCV-SOCʱ���¼���ֽ�
            kOcvSocTimeLowIndex,                    //OCV-SOCʱ���¼���ֽ�
            kCustomEeeParaOneIndex,                 //�ͻ�ʹ��eeeprom����1
            kCustomEeeParaTwoIndex,                 //�ͻ�ʹ��eeeprom����2
            kCapAmsSaveLowIndex,                    //ʣ������Ams��ʽ����
            kCapAmsSaveHighIndex,
            kSOC0DischgFactorIndex,
/* 362 */   kChgTotalCapLowIndex,                   //�ۼƳ����� 
            kChgTotalCapHighIndex, 
            kDChgTotalCapLowIndex,                  //�ۼƷŵ����
            kDChgTotalCapHighIndex,
            
            kDeltaCapAMSAfterCalibLowIndex,         // ocv-soc����ʱ���ֵ��� 
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
 * ��������Ƿ�����쳣���������޸�, ��������config_init������
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
