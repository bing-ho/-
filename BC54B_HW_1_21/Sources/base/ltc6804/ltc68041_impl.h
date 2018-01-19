/*******************************************************************************
 **                       安徽新能源技术有限公司 Copyright (c)
 **                            http://www...
 **文 件 名:ltc68041_impl.h
 **作    者:曹志勇
 **创建日期:2016.12.15
 **文件说明:
 **修改记录:
 **版    本:V1.0
 **备    注:
 *******************************************************************************/

#ifndef _LTC68041_IMPL_H_
#define _LTC68041_IMPL_H_

//#include "SPI1.h"
#include "includes.h" 

#define  PCF8574_WRITE_ADDR    0x40
#define  PCF8574_CHANNEL_MAX_NUM        7
/***************************寄存器长度等定义************************/
#define  MAX_LTC6804_IC_NUM             PARAM_BSU_NUM_MAX  //一条总线上LTC6804个数
#define  REG_LEN                        6    //寄存器长度
#define  REG_WITH_PEC_LEN               (REG_LEN +2)
#define  LTC6804_MAX_CELL_NUM           12   //支持最大电压数
#define  LTC6804_HALF_CELL_NUM          6    //一半


/**********************故障码定义***********************************/
typedef  INT8U  LtcResult;
#define  LTC_OK                      0//正常
#define  LTC_ERROR                   1//错误
#define  LTC_INIT_ERROR              2//初始化失败
#define  LTC_PTR_NULL                3//指针为空
#define  LTC_COMM_ERROR              4//通信失败
#define  LTC_CMD_ERROR               5//命令错误
#define  LTC_CHANNEL_ERROR           6//通道选择错误
#define  LTC_BUFFER_LEN_ERROR        7//数组长度错误
#define  LTC6804_NUM_OUT_OF_RANGE    8//6804数目超过限制
#define  PCF8574_CHANNEL_ERROR       9//8574通道选择失败
#define  LTC3300_CRC4CHECK_ERROR     10//crc4校验失败
#define  PCF8574_WRITE_ERROR         11

/****************************命令代码定义******************************/
#define WRCFG    0x001  //写配置寄存器
#define RDCFG    0x002  //读配置寄存器
#define RDCVA    0x004  //读电池电压寄存器组A
#define RDCVB    0x006  //读电池电压寄存器组B
#define RDCVC    0x008  //读电池电压寄存器组C
#define RDCVD    0x00A  //读电池电压寄存器组D
#define RDAUXA   0x00C  //读辅助寄存器组A
#define RDAUXB   0x00E  //读辅助寄存器组B
#define RDSTATA  0x010  //读状态寄存器组A
#define RDSTATB  0x012  //读状态寄存器组B

#define REFON    1
#define ADCOPT   1//0-27KHZ、7KHZ、26HZ;1-14KHZ、3KHZ、2KHZ。
#define DCTO     1
//电压采集模式
#define MD_FAST     1
#define MD_NORMAL   2
#define MD_FILTERED 3


#if  ADCOPT == 0
#define FILTERED_MODE_AD_TIME 220  //ms
#else  ADCOPT == 1
#define FILTERED_MODE_AD_TIME 15    //ms
#endif



//电压采集通道
#define CELL_CH_ALL    0
#define CELL_CH_1and7  1
#define CELL_CH_2and8  2
#define CELL_CH_3and9  3
#define CELL_CH_4and10 4
#define CELL_CH_5and11 5
#define CELL_CH_6and12 6

#define AUX_BUFF_LEN  3
//辅助采集通道
#define AUX_CH_ALL   0
#define AUX_CH_GPIO1 1
#define AUX_CH_GPIO2 2
#define AUX_CH_GPIO3 3
#define AUX_CH_GPIO4 4
#define AUX_CH_GPIO5 5
#define AUX_CH_VREF2 6
//采集时均衡使能
#define DCP_DISABLED 0
#define DCP_ENABLED  1
//上下拉电流
#define PUP_UP   1
#define PUP_DOWN 0
//自测试
#define CVST0   1
#define CVST1   2
//状态组测量通道
#define STAT_CHST_ALL   0
#define STAT_CHST_SOC   1
#define STAT_CHST_ITMP  2
#define STAT_CHST_VA    3
#define STAT_CHST_VD    4

#define CLRCELL     0x711 //清除电池电压寄存器
#define CLRAUX      0x712 //清除辅助寄存器
#define CLRSTAT     0x713 //清除状态寄存器
#define PLADC       0x714 //轮询ADC转换状态
#define DIAGN       0x715 //诊断MUX和轮询状态
#define WRCOMM      0x721 //写COMM存储器组
#define RDCOMM      0x722 //读COMM寄存器组
#define STCOMM      0x723 //启动I2C/SPI通信


//温度通道
#define T1_CHANNEL    0
#define T2_CHANNEL    1
#define T3_CHANNEL    2
#define T4_CHANNEL    3
#define T5_CHANNEL    4
#define T6_CHANNEL    5
/*****************配置寄存器初始化定义*****************************/
#define CFGR_REFON     0x01   //保持上电,1 更快的启动AD
#define CFGR_SWTRD     0x00   //不需要,置0
#define CFGR_ADCOPT    0x00   //标准
#define CFGR_VUV       0x0000 //不需要,置0
#define CFGR_VOV       0x0000 //不需要,置0
#define CFGR_DCTO      0x00   //均衡超时设置


//读写位
#define IIC_ADDR_WRITE_BIT   0
#define IIC_ADDR_READ_BIT    1
//初始通信控制位
#define IIC_ICOM_WRITE_START          0x06 
#define IIC_ICOM_WRITE_STOP           0x01
#define IIC_ICOM_WRITE_BLANK          0x00
#define IIC_ICOM_WRITE_NO_TRANSMIT    0x07

#define IIC_ICOM_READ_START                 0x06
#define IIC_ICOM_READ_STOP                  0x01
#define IIC_ICOM_READ_SDA_LOW               0x00
#define IIC_ICOM_READ_SDA_HIGH              0x07
//最终通控制信位
#define IIC_FCOM_WRITE_ACK            0x00
#define IIC_FCOM_WRITE_NACK           0x08
#define IIC_FCOM_WRITE_NACK_STOP      0x09

#define IIC_FCOM_READ_ACK_FROM_MIAN         0x00
#define IIC_FCOM_READ_ACK_FROM_SLAVE        0x07
#define IIC_FCOM_READ_NACK_FROM_SLAVE       0x0F
#define IIC_FCOM_READ_ACK_STOP_FROM_SLAVE   0x01
#define IIC_FCOM_READ_NACK_STOP_FROM_SLAVE  0x09


typedef enum
{
   kIICSPIRead = 0,
   kIICSPIWrite = 1,
}Ltc6804IICSPIWRFlag;


typedef enum
{
   kReadCfg = 0,
   kReadCV,
   kReadGpio,
   kReadPUCV,
   kReadPUDV,
}Ltc6804RegReadType;
//通信定义
typedef union
{
	INT16U value;
	struct
	{
		INT8U read_cfg_reg :1;//读配置寄存器错误标志
		INT8U read_cv :1; //读所有电压
		INT8U read_pucv :1;//读所上拉有电压
		INT8U read_pudv :1;//读所下拉有电压
		INT8U read_gpio1 :1; //读GPIO1电压
		INT8U read_gpio2 :1; //读GPIO1电压
		INT8U read_gpio3 :1; //读GPIO1电压
		INT8U read_gpio4 :1; //读GPIO1电压
		INT8U read_gpio5 :1; //读GPIO1电压
		INT8U NA :7;
	} Bits;
} Ltc6804CommErrorByte;
//GPIO定义
typedef union
{
	INT8U value;
	struct
	{
		INT8U GPIO1 :1;
		INT8U GPIO2 :1;
		INT8U GPIO3 :1;
		INT8U GPIO4 :1;
		INT8U GPIO5 :1;
		INT8U NA :3;
	} Bits;
} Ltc6804Gpio;

//设置6804引脚2为高电平
void set_ltc6804_gpio2_pin_high(INT8U current_ic);
//设置6804引脚2为低电平
void set_ltc6804_gpio2_pin_low(INT8U current_ic);
//设置6804引脚1为高电平
void set_ltc6804_gpio1_pin_high(INT8U current_ic);
//设置6804引脚1为低电平
void set_ltc6804_gpio1_pin_low(INT8U current_ic);
INT8U is_ltc6804_gpio1_pin_high(INT8U current_ic);
INT8U is_ltc6804_gpio2_pin_high(INT8U current_ic);
//发送均衡命令道6804
LtcResult send_balance_cmd_to_ltc6804(INT8U ltc6804_num);
//写配置命令
LtcResult write_cfg_reg_to_ltc6804(INT8U ltc6804_num,BOOLEAN balance_en);
//设置单个6804芯片均衡控制字
void set_ltc6804_balance_ctrl_word(INT8U current_ic, INT16U ctrl_word);
//清除6804寄存器数据
void clr_ltc6804_reg(void);
//6804初始化
LtcResult ltc6804_init(INT8U ltc6804_num);
//唤醒并配置6804
void wakeup_ltc6804_with_write_cfg_reg(INT8U ltc6804_num,BOOLEAN balance_en);
//清除所有6804均衡控制字
void clr_all_ltc6804_balance_ctrl_word(void);
//启动辅助通道电压采集
void start_ltc6804_aux_adc(INT8U MD, INT8U AUX_CH);
//启动电池电压采集
void start_ltc6804_cells_volt_adc(INT8U MD, INT8U DCP, INT8U CH);
//读取电池电压
LtcResult read_ltc6804_cells_volt(INT8U ltc6804_num, INT16U  (*far cell_volt)[LTC6804_MAX_CELL_NUM], Ltc6804CommErrorByte * far comm_err,Ltc6804RegReadType type);
//启动开路电压采集
void start_ltc6804_volt_cable_open_adc(INT8U MD, INT8U DCP, INT8U PUP, INT8U CH);
//启动状态采集
void start_ltc6804_stat_adc(INT8U MD, INT8U CHST);
//读取6804辅助寄存器A电压
LtcResult read_ltc6804_aux_a_volt(INT8U ltc6804_num, INT16U  (*far cell_volt)[PARAM_BSU_NUM_MAX], Ltc6804CommErrorByte * far comm_err);
//读取6804辅助寄存器B电压
LtcResult read_ltc6804_aux_b_volt(INT8U ltc6804_num, INT16U  (*far cell_volt)[PARAM_BSU_NUM_MAX], Ltc6804CommErrorByte * far comm_err);
//PCF8574通道选择
LtcResult pcf8574_write_channel(INT8U ltc6804_num, INT8U channel);
//通过LTC6804的IIC来写DSP
LtcResult write_balance_info_to_dsp(INT8U ltc6804_num,INT8U addr,INT16U*far active_balance_word,INT8U current);
//通过LTC6804的IIC来读DSP
LtcResult read_balance_current_from_dsp(INT8U ltc6804_num,INT8U addr,INT8U *FAR balance_current);
//清除均衡电流
void clear_bsu_active_balance_current(INT8U bsu_num);
//采集板IO引脚初始化
void bsu_board_gpio_init(INT8U current_ic);

 /***************************宏定义检查***************************************/
#if REG_LEN != 6
#error "REG_LEN must == 6"
#endif
#if LTC6804_MAX_CELL_NUM != 12
#error "LTC6804_MAX_CELL_NUM must == 12"
#endif

#if LTC6804_MAX_CELL_NUM != 2*LTC6804_HALF_CELL_NUM
#error "LTC6804_HALF_CELL_NUM must == LTC6804_MAX_CELL_NUM/2"
#endif

#endif

