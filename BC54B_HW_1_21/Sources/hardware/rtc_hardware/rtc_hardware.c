/*******************************************************************************
**                       °²»ÕÁ¦¸ßÐÂÄÜÔ´ÓÐÏÞ¹«Ë¾ Copyright (c)
**                            http://www.ligoo.cn/ 
** @file	 £b rtc_hardware.c                                       

** @brief       1.Íê³ÉRTCÄ£¿éµÄÊý¾ÝÀàÐÍµÄ¶¨Òå 
                2.Íê³ÉRTCÄ£¿éµÄ³õÊ¼»¯
                3.Íê³ÉRTCÄ£¿éµÄ¸÷ÖÖ¶¨Ê±Æ÷µÄ¹¦ÄÜº¯Êý
** @copyright		Ligoo Inc.         
** @date		    2017-03-27.
** @author      º« »Û      
*******************************************************************************/ 
#include <string.h>
#include "rtc_hardware.h"
#include "second_datetime.h"
#include "bms_util.h"
#include "iic_interface.h"
#include "iic_hardware.h"



/*
# register map
| Address | Function | bit7 | bit6 | bit5 | bit4 | bit3 | bit2 | bit1 | bit0 | READ | Write |
| - | - | - | - | - | - | - | - | - | - | - | - |
| 0x00 | Control_status_1 | TEST1 | N | STOP | N | TESTC | N | N | N | - | - |
| 0x01 | Control_status_2 | N | N | N | TI_TP | AF | TF | AIE | TIE | - | - |
| 0x02 | VL_seconds | VL | 6:0 SECONDS | Y | Y |
| 0x03 | Minute | x | 6:0 MINUTES(0~59) | Y | Y |
| 0x04 | Hour | x | x | 5:0 hours(0~23) | Y | Y |
| 0x05 | Days | x | x | 5:0 Days(1~31) | Y | Y |
| 0x06 | WeekDays | x | x | x | x | x | 2:0 Weekdays(0~6) | Y | Y |
| 0x07 | Century_Months | C | x | x | 4:0 Months(1~12) | Y | Y |
| 0x08 | Years | 7:0 (0~99) | Y | Y |
| 0x09 | Minute_alarm | AE_M | 6:0 Minute_alarm(0~59) | Y | Y |
| 0x0A | Hour_alarm | AE_H | x | 5:0 HOUR_ALARM (0 to 23) | Y | Y |
| 0x0B | Day_alarm | AE_D | x | 5:0 DAY_ALARM (1 to 31) | Y | Y |
| 0x0C | Weekday_alarm | AE_W | x | x | x | x | 2:0 WEEKDAY _ALARM (0 to 6) | Y | Y |
| 0x0D | CLKOUT_control | FE | x | x | x | x | x | FD[1:0] | Y | Y |
| 0x0E | Timer_control | TE | x | x | x | x | x | TD[1:0] | Y | Y |
| 0x0F | Timer | TIMER[7:0] | Y | Y |
*/

#define PCF8563_STATUS_NOT_INIT 0
#define PCF8563_STATUS_OK       1
#define PCF8563_STATUS_RESET    2

#define PCF8563_I2C_ADDRESS         (0x51) //Chip address is 0xA2, but here need pass    0xA2 >> 1 = 0x51

/***********************************************************************
  * @brief           Ð´ÇëÇó 
  * @param[in]       PitContext PITÄ£¿éÅäÖÃÊôÐÔ
  * @return          1£ºPIT_ERROR   0£ºPIT_OK
***********************************************************************/
static uint8_t write_regs(const struct i2c_bus *__FAR   bus,
                          const uint8_t                *reg_and_dat,
                          uint8_t                       len) 
{
    if (len != bus_i2c_transmit(bus, PCF8563_I2C_ADDRESS, reg_and_dat, len, 0, 0)) 
    {
        return 0;
    }
    return 1;
}

/***********************************************************************
  * @brief           ¶ÁÇëÇó 
  * @param[in]       PitContext PITÄ£¿éÅäÖÃÊôÐÔ
  * @return          1£ºPIT_ERROR   0£ºPIT_OK
***********************************************************************/
static uint8_t read_regs(const struct i2c_bus *__FAR    bus,
                         uint8_t                        reg,
                         uint8_t *__FAR                 dat,
                         uint8_t                        len) 
{
    if (len + 1 != bus_i2c_transmit(bus, PCF8563_I2C_ADDRESS, &reg, 1, dat, len)) 
    {
        return 0;
    }
    return 1;
}

/***********************************************************************
  * @brief           RTCÐ´ÈëÊ±¼ä
  * @param[in]       Pcf8563½á¹¹ÌåµÄ²ÎÊý
                     seconds£ºÊ±¼ä
  * @return          1£ºÊ§°Ü   0£º³É¹¦
***********************************************************************/
static char Rtc_Hardware_Write_Time( void *__FAR     _dev,
                                uint32_t        seconds) 
{
    struct pcf8563 *__FAR dev = (struct pcf8563 *__FAR) _dev;
    struct datetime t;
    uint8_t reg_and_dat[8];

    (void)second_to_datetime(&t, seconds);

    reg_and_dat[0] = 0x02;
    reg_and_dat[1] = hex_to_bcd(t.second);// 32(0x20) => 0x32
    reg_and_dat[2] = hex_to_bcd(t.minute);
    reg_and_dat[3] = hex_to_bcd(t.hour);
    reg_and_dat[4] = hex_to_bcd(t.day);
    reg_and_dat[5] = t.weekday;
    reg_and_dat[6] = hex_to_bcd(t.month); //bit7: current century
    reg_and_dat[7] = hex_to_bcd(t.year);

    return write_regs(dev->bus, reg_and_dat, sizeof(reg_and_dat));
}

#if 0
static char pcf8563_set_backup_config(struct pcf8563 *__FAR dev) {
    uint8_t dat[2];
    dat[0] = 0x18;
    dat[1] = 0x0C;        
    return write_regs(dev->bus, dat, 2);
}
#endif

/***********************************************************************
  * @brief           RTCµôµç¼ì²â 
  * @param[in]       pcf8563 *__FAR dev½á¹¹Ìå²ÎÊý
  * @return          1£ºÊ§°Ü   0£º³É¹¦
***********************************************************************/
static char pcf8563_lvf_init(struct pcf8563 *__FAR dev) 
{
    uint8_t dat[8];
    uint8_t i;

    if (0 == read_regs(dev->bus, 0x02, dat, 1)) 
    {
        return 0;
    }
    
    if ((dat[0] & 0x80) == 0) { //if VL == 1, integrity of the clock information is NOT guaranteed (include start up)
        return 1;
    }
    
    i = 0;
    do  
    {
        ++i;
        if (i > 100) 
        {
            return 0;
        }
        dat[1] = dat[0] & 0x7F; // VLF = 0
        dat[0] = 0x02;
        if (0 == write_regs(dev->bus, dat, 2)) {
            return 0;
        }
        if (0 == read_regs(dev->bus, 0x02, dat, 1)) {
            return 0;
        }
    } while (dat[0] & 0x80);
    if(i>=3) {    ///////////////////////20170119sunyan
    dat[0] = 0x00;
    dat[1] = 0x00;
    if (0 == write_regs(dev->bus, dat, 2)) {
        return 0;
    }
    if (0 == Rtc_Hardware_Write_Time(dev, 0)) {
        return 0;   
    }
    }
    return 1;
}

/***********************************************************************
  * @brief           RTCÓ²¼þ³õÊ¼»¯ 
  * @param[in]       pcf8563 *__FAR dev½á¹¹Ìå²ÎÊý
  * @return          1£ºÊ§°Ü   0£º³É¹¦
***********************************************************************/
static char Rtc_Hardware_Init(struct pcf8563 *__FAR dev) 
{                 
    unsigned char buf[1];
    unsigned char dat[2];
    
    dev->status = PCF8563_STATUS_NOT_INIT;

    if (0 == bus_i2c_init(dev->bus)) 
    {
        return 0;
    }
    
    if (0 == pcf8563_lvf_init(dev)) 
    {
        return 0;
    }
    
    if (!read_regs(dev->bus, 0x01, buf, sizeof(buf))) {
        return 0;
    }
        
    dat[0] = 0x0D; 
    dat[1] = 0x81; //FE=1, FD=01(1kHz)
    
    if(write_regs(dev->bus, dat, sizeof(dat)) == 0)
    {
        return 0;
    }
    
    dev->status = PCF8563_STATUS_OK;
    return 1;
}

/***********************************************************************
  * @brief           RTC¶ÁÈ¡Ê±¼ä
  * @param[in]       Pcf8563½á¹¹ÌåµÄ²ÎÊý
                     seconds£ºÊ±¼ä
  * @return          1£ºÊ§°Ü   0£º³É¹¦
***********************************************************************/
static char Rtc_Hardware_Read_Time(  struct pcf8563 *__FAR   dev,
                                uint32_t                *seconds) 
{
    uint8_t treg[7];
    struct datetime t;

    if (seconds == 0) {
        return 0;
    }

    if (!read_regs(dev->bus, 0x02, treg, sizeof(treg))) {
        return 0;
    }

    t.second = bcd_to_hex(treg[0] & 0x7F);
    t.minute = bcd_to_hex(treg[1] & 0x7F);
    t.hour   = bcd_to_hex(treg[2] & 0x3F);
    t.day    = bcd_to_hex(treg[3] & 0x3F);
    t.weekday= treg[4] & 0x07;
    t.month  = bcd_to_hex(treg[5] & 0x1F);
    t.year   = bcd_to_hex(treg[6]);

    return datetime_to_second(seconds, &t);
}

/***********************************************************************
  * @brief           RTC´æ´¢Êý¾Ý
  * @param[in]       Pcf8563½á¹¹ÌåµÄ²ÎÊý£»dat Êý¾Ý£»
  * @return          1£ºÊ§°Ü   0£º³É¹¦
***********************************************************************/
static char Rtc_Hardware_Store_Data(struct pcf8563 *__FAR    dev,
                           const uint8_t *__FAR         dat,
                           uint8_t                      offset,
                           uint8_t                      len)
{
    (void)dev;
    (void)dat;
    (void)offset;
    (void)len;
    return 0;
}

/***********************************************************************
  * @brief           RTC´æ´¢Êý¾Ý
  * @param[in]       Pcf8563½á¹¹ÌåµÄ²ÎÊý£»dat Êý¾Ý£»
  * @return          1£ºÊ§°Ü   0£º³É¹¦
***********************************************************************/
static char Rtc_Hardware_Restore_Data(struct pcf8563 *__FAR  dev,
                          unsigned char *__FAR          dat,
                          unsigned char                 offset,
                          unsigned char                 len)
{
    (void)dev;
    (void)dat;
    (void)offset;
    (void)len;
    return 0;
}

/***********************************************************************
  * @brief           RTCÅäÖÃÖÐ¶Ï 
  * @param[in]       pcf8563 *__FAR   dev£»
                     rtc_interrupt_type£ºÖÐ¶ÏÀàÐÍ
                     rtc_interrupt_single£ºµ¥¶ÀÖÐ¶Ï
  * @return          1£ºÊ§°Ü   0£º³É¹¦
***********************************************************************/
static char Rtc_Hardware_Config_Interrupt(   struct pcf8563 *__FAR   dev,  
                                        rtc_interrupt_type      int_type,
                                        rtc_interrupt_single    int_type_single)
{
    unsigned char ret = 0;
    unsigned char dat[2];
    unsigned char buf[1];

    (void)int_type_single;
    
    if (!read_regs(dev->bus, 0x01, buf, sizeof(buf))) {
        return 0;
    }
        
    dat[0] = 0x01; 

    switch (int_type ) {
    case RTC_INTERRUPT_DISABLE:
        dat[1] |= buf[0] & 0xFE; //bit0 = 0
        break;
    case RTC_INTERRUPT_ALARM:
        dat[1] |= buf[0] | 0x02; //[0F].TIE/AIE =1
        break;
    case RTC_INTERRUPT_FREQUENCY:
    case RTC_INTERRUPT_COUNTDOWN:
    default:
        return 0;
    }

    ret = write_regs(dev->bus, dat, sizeof(dat));
    
    return ret;
}

/***********************************************************************
  * @brief           RTCÇå³ýÖÐ¶Ï 
  * @param[in]       pcf8563 *__FAR   dev£»
  * @return          1£ºÊ§°Ü   0£º³É¹¦
***********************************************************************/
static char Rtc_Hardware_Clear_Interrupt(struct pcf8563 *__FAR dev)
{
    unsigned char dat[2];
    
    if (!read_regs(dev->bus, 0x01, &dat[1], sizeof(char))) {
        return 0;
    }
    dat[0] = 0x01;
    dat[1] = dat[1] & 0xF0;// clear [01].AF/TF/AIE/TIE.
    if (!write_regs(dev->bus, dat, 2)) {
        return 0;
    }
    return 1;
}

#if 0// removed as no one use it
static char pcf8563_dump_regs(struct pcf8563 *__FAR dev, uint8_t __FAR dat[0x20]) 
{
    return read_regs(dev->bus, 0x00, dat, 0x20);
}

/***********************************************************************
  * @brief           RTCÇå³ý±¨¾¯
  * @param[in]       pcf8563 *__FAR   dev£»
  * @return          1£ºÊ§°Ü   0£º³É¹¦
***********************************************************************/
static char pcf8563_clear_alarm(struct pcf8563 *__FAR dev) 
{
    uint8_t dat[4];
    if (0 == read_regs(dev->bus, 0x0D, &dat[1], 3)) {
        return 0;
    }

    dat[0] = 0x0D;
    dat[1] |= (1 << 6);
    dat[2] &= ~(1 << 3);
    dat[3] &= ~(1 << 3);

    if (0 == write_regs(dev->bus, dat, 4)) {
        return 0;
    }

    dat[0] = 0x08;
    dat[1] = 0x00;
    dat[2] = 0x00;
    dat[3] = 0x00;

    if (0 == write_regs(dev->bus, dat, 4)) {
        return 0;
    }

    return 1;
}
#endif

/***********************************************************************
  * @brief           RTCÉèÖÃ±¨¾¯
  * @param[in]       pcf8563 *__FAR   dev£
                     seconds Ê±¼ä
  * @return          1£ºÊ§°Ü   0£º³É¹¦
***********************************************************************/
static char Rtc_Hardware_Set_Alarm(struct pcf8563 *__FAR dev, uint32_t seconds) 
{
    struct datetime t;
    uint8_t reg_and_dat[5];

    seconds += 30;

    (void)second_to_datetime(&t, seconds);

    reg_and_dat[0] = 0x09;
    reg_and_dat[1] = hex_to_bcd(t.minute);
    reg_and_dat[2] = hex_to_bcd(t.hour);
    reg_and_dat[3] = hex_to_bcd(t.day);
    reg_and_dat[4] = t.weekday;

    if (0 == write_regs(dev->bus, reg_and_dat, sizeof(reg_and_dat))) {
        return 0;
    }


    reg_and_dat[0] = 0x0E;
    reg_and_dat[1] = 0x83;//TE=1, TD=3(1/60 Hz for power saving)

    if (0 == write_regs(dev->bus, reg_and_dat, 2)) {
        return 0;
    }

    return 1;
}

/***********************************************************************
  * @brief           RTC»ñÈ¡±¨¾¯Ê±¼ä 
  * @param[in]       pcf8563ÅäÖÃÊôÐÔ
                     seconds Ê±¼äµØÖ·
  * @return          1£ºÊ§°Ü   0£º³É¹¦
***********************************************************************/
static char Rtc_Hardware_Get_Alarm_Time(struct pcf8563 *__FAR dev, uint32_t *seconds) 
{
    unsigned char treg[4];
    struct datetime t = {0};

    if (seconds == 0) {
        return 0;
    }

    if (!read_regs(dev->bus, 0x09, treg, sizeof(treg))) {
        return 0;
    }

    t.minute = bcd_to_hex(treg[0] & 0x7F);
    t.hour   = bcd_to_hex(treg[1] & 0x3F);
    t.day    = bcd_to_hex(treg[2] & 0x3F);
    t.weekday= bcd_to_hex(treg[3] & 0x07);

    if (!datetime_to_second(seconds, &t)) {
        return 0;
    }
    return 1;
}

/***********************************************************************
  * @brief           RTC»ñÈ¡RAM¿Õ¼ä´óÐ¡ 
  * @param[in]       pcf8563ÅäÖÃÊôÐÔ
  * @return          1£ºÊ§°Ü   0£º³É¹¦
***********************************************************************/
static uint16_t Rtc_Hardware_Get_Ram_Size_bit (struct pcf8563 *__FAR dev) {
    (void)dev;
    return 0;
}
    
const rtc_oops pcf8563_oops = 
{
    Rtc_Hardware_Init,
    Rtc_Hardware_Write_Time,
    Rtc_Hardware_Read_Time,
    Rtc_Hardware_Get_Ram_Size_bit,
    Rtc_Hardware_Store_Data,
    Rtc_Hardware_Restore_Data,
    Rtc_Hardware_Config_Interrupt,
    Rtc_Hardware_Clear_Interrupt,
    Rtc_Hardware_Set_Alarm,
    Rtc_Hardware_Get_Alarm_Time
};
    
