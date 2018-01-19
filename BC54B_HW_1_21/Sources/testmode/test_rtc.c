#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "stdint.h"

#include "second_datetime.h"
#include "ucos_ii.h"
#include "test_io.h"
#include "bms_clock.h"



const char shell_help_rtc[] = "\
	[yymmddhhMMss]\n\
		设置或读取RTC的时间\n\
    alarm xxx\n\
		设置RTC在XXX产生中断\n\
";

const char shell_summary_rtc[] =
    "RTC相关";

static int dump_time(void) {
        uint32_t r;
        if (!clock_get_seconds(&r)) 
        {
            struct datetime dt;
            (void)second_to_datetime(&dt, r);
            (void)printf("%02d%02d%02d%02d%02d%02d\n",
             dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second);
            return 0;
        } 
        else 
        {
            (void)printf("rtc read time error\n");
             return -2;
        }
}

static int set_time(struct datetime *dt) {
    uint32_t t;
    if (!datetime_to_second(&t, dt)) {
        (void)printf("Invalid datatime\n");
        return -3;
    }
    
    if (clock_set(dt)) {
        (void)printf("rtc write time error\n");
        return -4;
    }
    
    return 0;
}

static void dt_frome_string(struct datetime *dt, const char *s) {
    uint8_t i;
    (void)memset(dt, 0xff, sizeof(*dt));
    if (strlen(s) != 12) return;
    for (i = 0; i < 12; ++i) {
        if (!isdigit(s[i])) {
            return;
        }
    }
    
    dt->year = (s[0] - '0') * 10 + (s[1] - '0');
    dt->month = (s[2] - '0') * 10 + (s[3] - '0');
    dt->day = (s[4] - '0') * 10 + (s[5] - '0');
    dt->hour = (s[6] - '0') * 10 + (s[7] - '0');
    dt->minute = (s[8] - '0') * 10 + (s[9] - '0');
    dt->second = (s[10] - '0') * 10 + (s[11] - '0');
}

static int set_alarm(uint32_t seconds) {
    uint32_t r;
    uint8_t is_high = 0;
    if (clock_get_seconds(&r) != RES_OK) {
        return -1;
    }
    
    
    (void)input_gpio_is_high_with_name("RTC_INT", &is_high);
    
    if (!is_high) {
        (void)clock_clear_interrupt(); 
        OSTimeDly(10);
    }
    
    if (seconds == 0) {
        return 0;
    }
    
    if (clock_config_interrupt(RTC_INTERRUPT_ALARM, RTC_INTERRUPT_SINGLE_LOWLEVEL) != RES_OK) 
    {
        return -2;
    }
    
    if (clock_set_alarm(r + seconds) != RES_OK) 
    {
        return -3;
    }
    
    return 0;
}

/*
static int dump_regs(void) 
{
    uint8_t i;
    uint8_t reg[32];
    
    if(hardware_io_revision_get() < HW_VER_116)
    {
        if (!sd2405_dump_regs(sd2405_impl, reg)) 
        {
        return -1;
    }
    for (i = 0; i < sizeof(reg); ++i) {
        (void)printf("REG[%02X]:%02X\n", i, reg[i]);
    }
    }
    else
    {
        (void)printf("V1.16 not support dump_regs\n");
    }
    
    return 0;
}
*/

extern void print_arg_err_hint(char *arg);
int shell_func_rtc(int argc, char **argv) {
    if (argc == 1) { // 读取RTC时间
        return dump_time();
    }
    
    //if ((argc == 2) && (strcmp(argv[1], "dump") == 0)) {
    //    return dump_regs();
    //}
    
    if (argc == 2) { // 设置RTC时间
        struct datetime dt;
        dt_frome_string(&dt, argv[1]);
        return set_time(&dt);
    }
    
    if (argc == 3) {
        if (strcmp(argv[1], "alarm") == 0) {
        uint32_t seconds;
        seconds = atoi(argv[2]);
        if (seconds == 0) {
            goto __error;
        }
        
        return set_alarm(seconds);
        }else if(strcmp(argv[1], "store") == 0){
            uint16_t tmp = 0;
            tmp = atoi(argv[2]);
            return clock_store_data((const unsigned char *)&tmp, 0, 2);
        }else if(strcmp(argv[1], "restore") == 0){
            Result ret = 0;
            uint16_t tmp = 0;
            ret = clock_restore_data((const unsigned char *)&tmp, 0, 2);
            (void)printf("restored data = %d\n", tmp);
            return ret;
        }
    }

__error:
    print_arg_err_hint(argv[0]);
    return -1;
}
