/*******************************************************************************
 **                       ��������Դ�������޹�˾ Copyright (c)
 **                            http://www...
 **�� �� ��:ltc68041_impl.h
 **��    ��:��־��
 **��������:2016.12.15
 **�ļ�˵��:
 **�޸ļ�¼:
 **��    ��:V1.0
 **��    ע:
 *******************************************************************************/

#ifndef _LTC68041_IMPL_H_
#define _LTC68041_IMPL_H_

//#include "SPI1.h"
#include "includes.h" 

#define  PCF8574_WRITE_ADDR    0x40
#define  PCF8574_CHANNEL_MAX_NUM        7
/***************************�Ĵ������ȵȶ���************************/
#define  MAX_LTC6804_IC_NUM             PARAM_BSU_NUM_MAX  //һ��������LTC6804����
#define  REG_LEN                        6    //�Ĵ�������
#define  REG_WITH_PEC_LEN               (REG_LEN +2)
#define  LTC6804_MAX_CELL_NUM           12   //֧������ѹ��
#define  LTC6804_HALF_CELL_NUM          6    //һ��


/**********************�����붨��***********************************/
typedef  INT8U  LtcResult;
#define  LTC_OK                      0//����
#define  LTC_ERROR                   1//����
#define  LTC_INIT_ERROR              2//��ʼ��ʧ��
#define  LTC_PTR_NULL                3//ָ��Ϊ��
#define  LTC_COMM_ERROR              4//ͨ��ʧ��
#define  LTC_CMD_ERROR               5//�������
#define  LTC_CHANNEL_ERROR           6//ͨ��ѡ�����
#define  LTC_BUFFER_LEN_ERROR        7//���鳤�ȴ���
#define  LTC6804_NUM_OUT_OF_RANGE    8//6804��Ŀ��������
#define  PCF8574_CHANNEL_ERROR       9//8574ͨ��ѡ��ʧ��
#define  LTC3300_CRC4CHECK_ERROR     10//crc4У��ʧ��
#define  PCF8574_WRITE_ERROR         11

/****************************������붨��******************************/
#define WRCFG    0x001  //д���üĴ���
#define RDCFG    0x002  //�����üĴ���
#define RDCVA    0x004  //����ص�ѹ�Ĵ�����A
#define RDCVB    0x006  //����ص�ѹ�Ĵ�����B
#define RDCVC    0x008  //����ص�ѹ�Ĵ�����C
#define RDCVD    0x00A  //����ص�ѹ�Ĵ�����D
#define RDAUXA   0x00C  //�������Ĵ�����A
#define RDAUXB   0x00E  //�������Ĵ�����B
#define RDSTATA  0x010  //��״̬�Ĵ�����A
#define RDSTATB  0x012  //��״̬�Ĵ�����B

#define REFON    1
#define ADCOPT   1//0-27KHZ��7KHZ��26HZ;1-14KHZ��3KHZ��2KHZ��
#define DCTO     1
//��ѹ�ɼ�ģʽ
#define MD_FAST     1
#define MD_NORMAL   2
#define MD_FILTERED 3


#if  ADCOPT == 0
#define FILTERED_MODE_AD_TIME 220  //ms
#else  ADCOPT == 1
#define FILTERED_MODE_AD_TIME 15    //ms
#endif



//��ѹ�ɼ�ͨ��
#define CELL_CH_ALL    0
#define CELL_CH_1and7  1
#define CELL_CH_2and8  2
#define CELL_CH_3and9  3
#define CELL_CH_4and10 4
#define CELL_CH_5and11 5
#define CELL_CH_6and12 6

#define AUX_BUFF_LEN  3
//�����ɼ�ͨ��
#define AUX_CH_ALL   0
#define AUX_CH_GPIO1 1
#define AUX_CH_GPIO2 2
#define AUX_CH_GPIO3 3
#define AUX_CH_GPIO4 4
#define AUX_CH_GPIO5 5
#define AUX_CH_VREF2 6
//�ɼ�ʱ����ʹ��
#define DCP_DISABLED 0
#define DCP_ENABLED  1
//����������
#define PUP_UP   1
#define PUP_DOWN 0
//�Բ���
#define CVST0   1
#define CVST1   2
//״̬�����ͨ��
#define STAT_CHST_ALL   0
#define STAT_CHST_SOC   1
#define STAT_CHST_ITMP  2
#define STAT_CHST_VA    3
#define STAT_CHST_VD    4

#define CLRCELL     0x711 //�����ص�ѹ�Ĵ���
#define CLRAUX      0x712 //��������Ĵ���
#define CLRSTAT     0x713 //���״̬�Ĵ���
#define PLADC       0x714 //��ѯADCת��״̬
#define DIAGN       0x715 //���MUX����ѯ״̬
#define WRCOMM      0x721 //дCOMM�洢����
#define RDCOMM      0x722 //��COMM�Ĵ�����
#define STCOMM      0x723 //����I2C/SPIͨ��


//�¶�ͨ��
#define T1_CHANNEL    0
#define T2_CHANNEL    1
#define T3_CHANNEL    2
#define T4_CHANNEL    3
#define T5_CHANNEL    4
#define T6_CHANNEL    5
/*****************���üĴ�����ʼ������*****************************/
#define CFGR_REFON     0x01   //�����ϵ�,1 ���������AD
#define CFGR_SWTRD     0x00   //����Ҫ,��0
#define CFGR_ADCOPT    0x00   //��׼
#define CFGR_VUV       0x0000 //����Ҫ,��0
#define CFGR_VOV       0x0000 //����Ҫ,��0
#define CFGR_DCTO      0x00   //���ⳬʱ����


//��дλ
#define IIC_ADDR_WRITE_BIT   0
#define IIC_ADDR_READ_BIT    1
//��ʼͨ�ſ���λ
#define IIC_ICOM_WRITE_START          0x06 
#define IIC_ICOM_WRITE_STOP           0x01
#define IIC_ICOM_WRITE_BLANK          0x00
#define IIC_ICOM_WRITE_NO_TRANSMIT    0x07

#define IIC_ICOM_READ_START                 0x06
#define IIC_ICOM_READ_STOP                  0x01
#define IIC_ICOM_READ_SDA_LOW               0x00
#define IIC_ICOM_READ_SDA_HIGH              0x07
//����ͨ������λ
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
//ͨ�Ŷ���
typedef union
{
	INT16U value;
	struct
	{
		INT8U read_cfg_reg :1;//�����üĴ��������־
		INT8U read_cv :1; //�����е�ѹ
		INT8U read_pucv :1;//���������е�ѹ
		INT8U read_pudv :1;//���������е�ѹ
		INT8U read_gpio1 :1; //��GPIO1��ѹ
		INT8U read_gpio2 :1; //��GPIO1��ѹ
		INT8U read_gpio3 :1; //��GPIO1��ѹ
		INT8U read_gpio4 :1; //��GPIO1��ѹ
		INT8U read_gpio5 :1; //��GPIO1��ѹ
		INT8U NA :7;
	} Bits;
} Ltc6804CommErrorByte;
//GPIO����
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

//����6804����2Ϊ�ߵ�ƽ
void set_ltc6804_gpio2_pin_high(INT8U current_ic);
//����6804����2Ϊ�͵�ƽ
void set_ltc6804_gpio2_pin_low(INT8U current_ic);
//����6804����1Ϊ�ߵ�ƽ
void set_ltc6804_gpio1_pin_high(INT8U current_ic);
//����6804����1Ϊ�͵�ƽ
void set_ltc6804_gpio1_pin_low(INT8U current_ic);
INT8U is_ltc6804_gpio1_pin_high(INT8U current_ic);
INT8U is_ltc6804_gpio2_pin_high(INT8U current_ic);
//���;��������6804
LtcResult send_balance_cmd_to_ltc6804(INT8U ltc6804_num);
//д��������
LtcResult write_cfg_reg_to_ltc6804(INT8U ltc6804_num,BOOLEAN balance_en);
//���õ���6804оƬ���������
void set_ltc6804_balance_ctrl_word(INT8U current_ic, INT16U ctrl_word);
//���6804�Ĵ�������
void clr_ltc6804_reg(void);
//6804��ʼ��
LtcResult ltc6804_init(INT8U ltc6804_num);
//���Ѳ�����6804
void wakeup_ltc6804_with_write_cfg_reg(INT8U ltc6804_num,BOOLEAN balance_en);
//�������6804���������
void clr_all_ltc6804_balance_ctrl_word(void);
//��������ͨ����ѹ�ɼ�
void start_ltc6804_aux_adc(INT8U MD, INT8U AUX_CH);
//������ص�ѹ�ɼ�
void start_ltc6804_cells_volt_adc(INT8U MD, INT8U DCP, INT8U CH);
//��ȡ��ص�ѹ
LtcResult read_ltc6804_cells_volt(INT8U ltc6804_num, INT16U  (*far cell_volt)[LTC6804_MAX_CELL_NUM], Ltc6804CommErrorByte * far comm_err,Ltc6804RegReadType type);
//������·��ѹ�ɼ�
void start_ltc6804_volt_cable_open_adc(INT8U MD, INT8U DCP, INT8U PUP, INT8U CH);
//����״̬�ɼ�
void start_ltc6804_stat_adc(INT8U MD, INT8U CHST);
//��ȡ6804�����Ĵ���A��ѹ
LtcResult read_ltc6804_aux_a_volt(INT8U ltc6804_num, INT16U  (*far cell_volt)[PARAM_BSU_NUM_MAX], Ltc6804CommErrorByte * far comm_err);
//��ȡ6804�����Ĵ���B��ѹ
LtcResult read_ltc6804_aux_b_volt(INT8U ltc6804_num, INT16U  (*far cell_volt)[PARAM_BSU_NUM_MAX], Ltc6804CommErrorByte * far comm_err);
//PCF8574ͨ��ѡ��
LtcResult pcf8574_write_channel(INT8U ltc6804_num, INT8U channel);
//ͨ��LTC6804��IIC��дDSP
LtcResult write_balance_info_to_dsp(INT8U ltc6804_num,INT8U addr,INT16U*far active_balance_word,INT8U current);
//ͨ��LTC6804��IIC����DSP
LtcResult read_balance_current_from_dsp(INT8U ltc6804_num,INT8U addr,INT8U *FAR balance_current);
//����������
void clear_bsu_active_balance_current(INT8U bsu_num);
//�ɼ���IO���ų�ʼ��
void bsu_board_gpio_init(INT8U current_ic);

 /***************************�궨����***************************************/
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

