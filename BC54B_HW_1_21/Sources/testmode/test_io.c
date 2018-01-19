#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "stdint.h"

#include "derivative.h"
#include "bms_input_signal_impl.h"
#include "bms_bcu.h"
#include "bms_mlock_impl.h"
#include "run_mode.h"

#define ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))

const char shell_help_input[] = "\n\
	输出指定输入IO的详细信息,\n\
";

const char shell_summary_input[] =
    "输入IO相关";

void input_gpio_init(void) 
{
    input_signal_init();
}


uint8_t input_gpio_is_high_with_name(const char *name, uint8_t *is_high) {
    uint8_t i;
    for (i = 0; i < 8; ++i) {
        if (strcmp(name, pt_signal_input_pins[i].gpio.name) == 0) {
            *is_high = hcs12_gpio_input_is_high(&pt_signal_input_pins[i].gpio.io);
            return 1;
        }
    }
    return 0;
}

static int __read_all(void) {
    uint8_t i;
    for (i = 0; i < 8; ++i) {
        (void)printf("%s:INPUT:%d\n", pt_signal_input_pins[i].gpio.name, hcs12_gpio_input_is_high(&pt_signal_input_pins[i].gpio.io));
    }

    return 0;
}

static int __read(const char *name) {
    uint8_t i;
    for (i = 0; i < 8; ++i) {
        if (strcmp(name, pt_signal_input_pins[i].gpio.name) == 0) {
            (void)printf("%s:INPUT:%d\n", pt_signal_input_pins[i].gpio.name, hcs12_gpio_input_is_high(&pt_signal_input_pins[i].gpio.io));
            return 0;
        }
    }
    return -1;
}

extern void print_arg_err_hint(char *arg);
int shell_func_input(int argc, char **argv) {
    if (argc == 1) {
        return __read_all();
    }

    if (argc == 2) {
        return __read(argv[1]);
    }

    print_arg_err_hint(argv[0]);
    return -1;
}



struct output_control {
    const char *name;
    volatile unsigned char *dat;  //输入数据寄存器
    volatile unsigned char *dir;  //方向寄存器
    unsigned char bit; // 对应的BIT
    unsigned char default_value:1;
    unsigned char is_revert:1;
};


extern const struct hcs12_gpio_detail relay_contrl_v114[];
extern const struct hcs12_gpio_detail relay_contrl_v116[];
extern const struct hcs12_gpio_detail shdn_24v_v116[];
extern const struct hcs12_gpio_detail insu_gain_ctrl_1_14[];
extern const struct hcs12_gpio_detail insu_gain_ctrl_1_16[];
extern const struct hcs12_gpio_detail insu_mos_ctrl[];
extern const struct hcs12_gpio_detail gsm_gpio_out_v114[];
extern const struct hcs12_gpio_detail gsm_gpio_out_v116[];
extern const struct hcs12_gpio_detail beep_output_pins_1_14[];
extern const struct hcs12_gpio_detail beep_output_pins_1_16[];
extern const struct hcs12_gpio_detail mlock_gpio_out_v120[];


const struct hcs12_gpio_detail other_gpio_out[] = {
    { "SYSTEM_POWER_LATCH",     { &PT1AD1,  &PT1AD1,    &DDR1AD1,   1 << 6}, 0 , 0},
    { "CHR_EN",                 { &PTIP,    &PTP,       &DDRP,      1 << 5}, 0 , 0},
    {(const char *)0, { (volatile uint8_t *)0, (volatile uint8_t *)0, (volatile uint8_t *)0, 0}, 0, 0},
};


#pragma push
#pragma DATA_SEG __PPAGE_SEG CONST_TABLES

typedef const struct hcs12_gpio_detail *hcs12_gpio_detail_list_t;

static hcs12_gpio_detail_list_t gpio_out_lists_114[] = {
    relay_contrl_v114,
    insu_gain_ctrl_1_14,
    insu_mos_ctrl,
    other_gpio_out,
    gsm_gpio_out_v114,
    beep_output_pins_1_14,
    (struct hcs12_gpio_detail *)0,
};

static hcs12_gpio_detail_list_t gpio_out_lists_116[] = {
    relay_contrl_v116,
    shdn_24v_v116,
    insu_gain_ctrl_1_16,
    insu_mos_ctrl,
    other_gpio_out,
    gsm_gpio_out_v116,
    beep_output_pins_1_16,
    (struct hcs12_gpio_detail *)0,
};

static hcs12_gpio_detail_list_t gpio_out_lists_120[] = {
    relay_contrl_v116,
    shdn_24v_v116,
    insu_gain_ctrl_1_16,
    insu_mos_ctrl,
    other_gpio_out,
    gsm_gpio_out_v116,
    beep_output_pins_1_16,
    mlock_gpio_out_v120,
    (struct hcs12_gpio_detail *)0,
};

#pragma DATA_SEG __RPAGE_SEG PAGED_RAM
static hcs12_gpio_detail_list_t *__FAR gpio_list; 
static hcs12_gpio_detail_list_t *__FAR all_hw_ver_gpio_list[]=
{
    gpio_out_lists_114,
    gpio_out_lists_114,
    gpio_out_lists_116,
    gpio_out_lists_120
}; 
#pragma pop



static hcs12_gpio_detail_list_t output_ctrl_find(const char *name) {
    hcs12_gpio_detail_list_t*__FAR list;
    hcs12_gpio_detail_list_t io;
    for (list = gpio_list; *list != 0; ++list) {
        for (io = list[0]; io->name != 0; ++io) {
            if (strcmp(name, io->name) == 0) return io;
        }
    }
    return 0;
}
void output_ctrl_init(void) {
    hcs12_gpio_detail_list_t *__FAR list;
    hcs12_gpio_detail_list_t io;

#if BMS_SUPPORT_BY5248D == 0
    gpio_list = all_hw_ver_gpio_list[hardware_io_revision_get()];
#else
    gpio_list = all_hw_ver_gpio_list[HW_VER_120];
#endif

    for (list = gpio_list; *list != 0; ++list) 
    {   if((mode!=RUN_MODE_TEST)&&(*list==gsm_gpio_out_v116))  continue;
        for (io = list[0]; io->name != 0; ++io) {
            (void)hcs12_gpio_init(&io->io, GPIO_MODE_OUTPUT_PUSHPULL);
            (void)hcs12_gpio_set_output(&io->io, io->def_val);
        }
    }
}

void output_ctrl_dump(hcs12_gpio_detail_list_t io) {
    if (!io) return;    
    (void)printf("%s:OUTPUT_PP:%d\n", io->name, hcs12_gpio_output_is_high(&io->io));
}

uint8_t output_ctrl_set_with_name(const char *name, uint8_t is_high) {
    
    hcs12_gpio_detail_list_t io = output_ctrl_find(name);
    if (io) {
        if(io->revert)
        {
            is_high = !is_high;
    }
        return hcs12_gpio_set_output(&io->io, is_high);
    }
    
    return 0;
}


uint8_t output_ctrl_is_high_with_name(const char *name, uint8_t *is_high) {
    hcs12_gpio_detail_list_t io = output_ctrl_find(name);
    if (io) {
        *is_high = hcs12_gpio_input_is_high(&io->io);        
        if(io->revert)
        {
            *is_high = !*is_high;
        }
    }

    return 0;
}
#if 0
uint8_t output_ctrl_toggle(const struct output_control *__FAR ctrl) {
    if (!ctrl) return 0;
     *ctrl->dat ^= ctrl->bit;
    return 1;
}

void toggle_do() {
    const hcs12_gpio_detail_list_t d;
    
    d = output_ctrl_find("DO0");
    if (d) 
        output_ctrl_toggle(d);
     
    d = output_ctrl_find("DO1");
    if (d) 
        output_ctrl_toggle(d); 
}
#else
void toggle_do() {
    output_ctrl_set_with_name("DO0", 0);
    output_ctrl_set_with_name("DO1", 0);
}
#endif

uint8_t output_ctrl_dump_with_name(const char *name) {
    hcs12_gpio_detail_list_t io = output_ctrl_find(name);
    if (io) {
        output_ctrl_dump(io);
    }
    return 0;
}

void output_ctrl_dump_all(void) {
    hcs12_gpio_detail_list_t *__FAR list;
    hcs12_gpio_detail_list_t io;
    for (list = gpio_list; *list != 0; ++list) {
        for (io = list[0]; io->name != 0; ++io) {
            output_ctrl_dump(io);
        }
    }
}




const char shell_help_octrl[] = "\
    [name [value]]\n\
    输出或设置指定输出IO的详细信息, 如果没有参数, 输出所有输出IO信息.\n\
";

const char shell_summary_octrl[] =
    "输出IO相关";

extern void print_arg_err_hint(char *arg);
int shell_func_octrl(int argc, char **argv) {
    if (argc == 1) {
        output_ctrl_dump_all();
        return 0;
    } else if (argc == 2) {
        if (0 == output_ctrl_dump_with_name(argv[1])) {
            return -1;
        }
        return 0;
    } else if (argc == 3) {
        uint8_t is_high = atoi(argv[2]) > 0 ? 1 : 0;
        if (0 == output_ctrl_set_with_name(argv[1], is_high)) {
            return -2;
        }
        return 0;
    } /*else if (argc == 4) {
        mlock_init();
        mlock_lock();
        sleep(5000);
        mlock_unlock();
        return 0;
    } */

    print_arg_err_hint(argv[0]);
    return -1;
}


