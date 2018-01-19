/**
*
* Copyright (c) 2012 Ligoo Inc. 
* 
* @file     LTC6803.h
* @brief    LTC6803�ײ����� 
* @note 
* @version 1.00
* @author 
* @date 2012/10/5 
* 
*/
#ifndef _LTC6803_H
#define _LTC6803_H

#include "includes.h"
#include "SPI2.h"
#include "bms_config.h"

#define LTC_TEST_SUPPORT        0

#define SPIDLY100US             200

#define LTC_CELLT_MIN_NUM       2        

#define LTC_INIT_OK             1
#define LTC_INIT_ERR            0

#define LTC_ZERO_VALUE          512
#define LTC_MAX_AD_DATA         3950       //  4095
#define LTC_BROADCAST_ADDR      255        //�㲥��ַ
/*LTC6803������*/
#define WRCFG_REG               0x01                    //д���üĴ�����
#define RDCFG_REG               0x02                    //�����üĴ�����
#define RDCV_REG                0x04                    //�����е�ѹ�Ĵ���
#define RDFLG_REG               0x0c                    //����־λ�Ĵ���
#define RDTMP_REG               0x0e                    //�����¶ȼĴ���

#define STCVAD_REG              0x10
#define CLRV_REG                0x1D                   //���� (FF)
#define SELFVADTEST1_REG        0x1E 
#define SELFVADTEST2_REG        0x1F 
#define STOWAD_REG              0x20
#define STTMPAD_REG             0x30
#define SELFTADTEST1_REG        0x3E
#define SELFTADTEST2_REG        0x3F
#define PLADC_REG               0x40
#define PLINT_REG               0x50
#define STDGN_REG               0x52
#define RDDGN_REG               0x54
#define STCVDC_REG              0x60
#define STOWDC_REG              0x70

/*ÿ���Ĵ����鳤��*/
#define CFGREG_LEN              6
#define CVREG_LEN               18 
#define FLGREG_LEN              3
#define TMPREG_LEN              5
#define PECREG_LEN              1
#define DGNREG_LEN              2

/*ltc6803-3ϵͳ���ò���*/
#define LTC6803_CDC             0x01                    //13ms����ʱ��
#define LTC6803_CELL10          0x00                    //cell11��cell12����
#define LTC6803_LVLPL           0x01                    //level polling
#define LTC6803_GPIO1           0x01                    
#define LTC6803_GPIO2           0x01
#define LTC6803_WDT             0x01
#define LTC6803_MCXI            0x0000                  //cell1��cell12�ж�ʹ��
#define LTC6803_UV              1000                    //Ƿѹ1.5V
#define LTC6803_OV              3000                    //��ѹ4.5V   

/*LTC6803�Ĵ����ṹ��*/
typedef struct LtcRegGrop
{
    INT8U CfgReg[CFGREG_LEN];                     /*>LCC6803�����üĴ���*/
    INT8U RDCfgReg[CFGREG_LEN]; 
    INT8U VReg[CVREG_LEN];                        /*>LCC6803�ĵ�ѹ�Ĵ���*/
    INT8U TReg[TMPREG_LEN];
 #if LTC_TEST_SUPPORT
    INT8U FlagReg[FLGREG_LEN];
    INT8U DgnReg[DGNREG_LEN];
 #endif
    INT16U dcc;                                    /*>����״̬λ*/
}LTC_REG_GROP;

/*LTC6803Ƭѡ*/
#define LTC_CS_OUT              DDRH_DDRH7 = 1
#define UP_LTC_CS               PTH_PTH7 = 1
#define DN_LTC_CS               PTH_PTH7 = 0

#define SendReadSPI             SPI2_SendByte

extern INT8U Ltc6803Init(LTC_REG_GROP * _PAGED ltcRegGrop);
extern void LTCSendCommand (INT8U ltc_addr,INT8U command, LTC_REG_GROP * _PAGED ltcRegGrop);
extern INT8U TLCReceiveCommand(INT8U ltc_addr,INT8U command, LTC_REG_GROP * _PAGED ltcRegGrop); 
extern INT8U ReadLtcVoltage(INT8U ltc_addr,LTC_REG_GROP * _PAGED ltcRegGrop);
extern INT8U ReadLtcTemperature(INT8U ltc_addr,LTC_REG_GROP * _PAGED ltcRegGrop);
extern INT8U ReadLtcDgnReg(INT8U ltc_addr,LTC_REG_GROP * _PAGED ltcRegGrop);
extern void WriteLtcCfgReg(INT8U ltc_addr,LTC_REG_GROP * _PAGED ltcRegGrop );
extern void ClrLtcVReg(void);
extern void StartLtcTAD(void);
extern void StartLtcVAD(void);
extern void TestLtcVADCommand(void);
extern void TestLtcTADCommand(void);
extern void TestLtcREFCommand(void);
extern void BroadCastCommand(INT8U command);
extern void VWireOff(void);

#if LTC_TEST_SUPPORT
extern INT8U VADDiagnose(INT8U ltc_addr);
extern INT8U TADDiagnose(INT8U ltc_addr);
extern INT8U REFDiagnose(INT8U ltc_addr);
#endif

//extern void safe_memset(void* _PAGED  buffer, INT8U value, INT16U size);
 
 #if  LTC_CELLT_MIN_NUM !=2
#error "LTC_CELLT_MIN_NUM == 2"
#endif
 
#endif

/***********END OF FILE***********/