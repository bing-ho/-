#include "async.h"
#include "async_timer.h"
#include "async_event_call.h"
#include "ads1015_impl.h"
#include "bms_bcu.h"
#include "unbalanced_bridge.h"
#include "hcf4052_impl.h"

#pragma MESSAGE DISABLE C1860

#pragma DATA_SEG __RPAGE_SEG PAGED_RAM_INSULATION

#define PCH_FILTER_SIZE 5
#define PCH_SAMPLE_INTERVAL 50

#define PRE_FILTER_SIZE 5
#define PRE_SAMPLE_INTERVAL 50

#define BAT_HV_FILTER_SIZE 5
#define BAT_HV_SAMPLE_INTERVAL 50


#define STRING_HCF_U37_A        "HCF_U37_A"
#define STRING_HCF_U37_B        "HCF_U37_B"
#define STRING_HCF_U37_INH      "HCF_U37_INH"

const struct hcs12_gpio_detail insu_gain_ctrl_1_14[] = {
    { STRING_HCF_U37_A,     { &PTIP,  &PTP, &DDRP, 1 << 3}, 1  , 0},
    { STRING_HCF_U37_B,     { &PTIP,  &PTP, &DDRP, 1 << 2}, 1  , 0},
    { STRING_HCF_U37_INH,   { &PTIJ,  &PTJ, &DDRJ, 1 << 2}, 1  , 0}
};

const struct hcs12_gpio_detail insu_gain_ctrl_1_16[] = {
    { STRING_HCF_U37_A,     { &PTIP,  &PTP, &DDRP, 1 << 3}, 1  , 0},
    { STRING_HCF_U37_B,     { &PTIT,  &PTT, &DDRT, 1 << 0}, 1  , 0},
    { STRING_HCF_U37_INH,   { &PTIJ,  &PTJ, &DDRJ, 1 << 2}, 1  , 0}
};
const struct hcs12_gpio_detail *insu_gain_ctrl;

static signed short average_filter(const signed short *__FAR buf, unsigned char size) {
    unsigned char i;
    signed long res = 0;

    for (i = 0; i < size; ++i) {
        res += *buf++;
    }

    if (res >= 0) {
        res += size / 2;
    } else {
        res -= size / 2;
    }

    return (signed short)((signed long)(res / size));
}


typedef void(*ads1015_finished)(const struct ads1015_sample_task_param *__FAR param);
struct ads1015_sample_task_param {
    unsigned char channel;
    unsigned char scale_voltage;
    signed short *__FAR buffer;
    unsigned char buffer_size;
    unsigned char *__FAR p_buffer_index;
    ASYNC_TIMER_TIMEOUT_TYPE interval;
    ads1015_finished finished_func;
};

static signed short ads1015_average_finished(const struct ads1015_sample_task_param *__FAR param) {
    signed short res = average_filter(param->buffer, param->buffer_size);
    return ADS1015_ResultToVoltageMv(res, param->scale_voltage);
}


static void pch_sample_finished(const struct ads1015_sample_task_param *__FAR param) {
    // 分压电阻: 510K * 5 | 10K
    signed long vol = ads1015_average_finished(param);
    if (vol > 0) {
        vol = vol * ((510 * 5 + 10) / 10);
    } else  {
        vol = 0;
    }
    bcu_set_insulation_pch_total_voltage(vol);
}
static unsigned char pch_buffer_index;
static signed short pch_buffer[PCH_FILTER_SIZE];
const struct ads1015_sample_task_param pch_sample_param = {
    ADS1015_CHANNEL_0,
    ADS1015_FULL_SCALE_4096_MV,
    pch_buffer,
    PCH_FILTER_SIZE,
    &pch_buffer_index,
    PCH_SAMPLE_INTERVAL,
    pch_sample_finished,
};

static void pre_sample_finished(const struct ads1015_sample_task_param *__FAR param) {
    // 分压电阻: 510K * 5 | 10K
    signed long vol = ads1015_average_finished(param);
    if (vol > 0) {
        vol = vol * ((510 * 5 + 10) / 10);
    } else  {
        vol = 0;
    }
    bcu_set_insulation_pre_total_voltage(vol);
}
static unsigned char pre_buffer_index;
static signed short pre_buffer[PRE_FILTER_SIZE];
const struct ads1015_sample_task_param pre_sample_param = {
    ADS1015_CHANNEL_1,
    ADS1015_FULL_SCALE_4096_MV,
    pre_buffer,
    PRE_FILTER_SIZE,
    &pre_buffer_index,
    PRE_SAMPLE_INTERVAL,
    pre_sample_finished,
};


static void bat_hv_sample_finished(const struct ads1015_sample_task_param *__FAR param) {
    // 分压电阻: 510K * 5 | 10K
    signed long vol = ads1015_average_finished(param);
    if (vol > 0) {
        vol = vol * ((510 * 5 + 10) / 10);
    } else  {
        vol = 0;
    }
    bcu_set_insulation_total_voltage(vol);
}
static unsigned char bat_hv_buffer_index;
static signed short bat_hv_buffer[BAT_HV_FILTER_SIZE];
const struct ads1015_sample_task_param bat_hv_sample_param = {
    ADS1015_CHANNEL_2,
    ADS1015_FULL_SCALE_4096_MV,
    bat_hv_buffer,
    BAT_HV_FILTER_SIZE,
    &bat_hv_buffer_index,
    BAT_HV_SAMPLE_INTERVAL,
    bat_hv_sample_finished,
};

static ASYNC_TIMER_TIMEOUT_TYPE ads1015_sample_task(async_timer_t *__FAR t) {
    const struct ads1015_sample_task_param *__FAR param = async_timer_get_data(t);

    signed short res = ADS1015_SingleConvert(
                           &g_ADS1015_BspInterface,
                           param->channel,
                           param->scale_voltage);
    if (res != ADS1015_RESULT_ERROR) {
        unsigned char index = *param->p_buffer_index;
        param->buffer[index++] = res;
        if (index >= param->buffer_size) index = 0;

        *param->p_buffer_index  = index;

        if (param->finished_func) param->finished_func(param);
    }
    return param->interval;
}

static ASYNC_TIMER_TIMEOUT_TYPE ads1015_sample_task_first(async_timer_t *__FAR t) {
    const struct ads1015_sample_task_param *__FAR param = async_timer_get_data(t);

    unsigned char index;

    for (index = 0; index < param->buffer_size;) {
        signed short res = ADS1015_SingleConvert(
                               &g_ADS1015_BspInterface,
                               param->channel,
                               param->scale_voltage);

        if (res != ADS1015_RESULT_ERROR) {
            param->buffer[index++] = res;
        }
    }

    *param->p_buffer_index = 0;
    async_timer_set_handler(t, ads1015_sample_task);
    if (param->finished_func) param->finished_func(param);
    return param->interval;
}


/*MOS开关控制*/
#define PositiveMosOn()  do { DDRS_DDRS1 = 1, PTS_PTS1 = 0; } while(0)
#define PositiveMosOff() do { DDRS_DDRS1 = 1, PTS_PTS1 = 1; } while(0)
#define NegativeMosOn()  do { DDRS_DDRS0 = 1, PTS_PTS0 = 0; } while(0)
#define NegativeMosOff() do { DDRS_DDRS0 = 1, PTS_PTS0 = 1; } while(0)
#define GainContol_A0(is_high) do { DDRP_DDRP3 = 1, PTP_PTP3 = is_high; }while(0)
//#define GainContol_A1(is_high)  do { DDRP_DDRP2 = 1, PTP_PTP2 = is_high; }while(0)
#define GainContol_GEN(is_high) do { DDRJ_DDRJ2 = 1, PTJ_PTJ2 = is_high; }while(0)

#define RES_SAMPLE_FILTER_SIZE 20
#define RES_ADC_CHANNEL_P 7
#define RES_ADC_CHANNEL_N 14
#define RES_SAMPLE_RESISTOR (71.5 * 6 + 0.392)


static void  GainContol_A1(INT8U is_high)
{
     io_pin_set(&pt_insu_input_pins[0],  is_high) ;
}

struct insu_res_sample_param {
    unsigned char stage;
    struct insu_res_unbalanced_bridge_sample_value filted_value;
    unsigned char gain[4];
    signed int sample_data[RES_SAMPLE_FILTER_SIZE];
    ADC_ChannelParameter adc_param;
    unsigned short res_p;
    unsigned short res_n;
    async_event_call_t *__FAR finished_event_call;
};

// stage   MOS_P  MOS_N   ADC_CHANNEL   FILTED_DATA_INDEX
// 0        1       1        P              0
// 1        1       1        N              1
// 2        1       0        P              2
// 3        0       1        N              3

#define SAMPLE_VOLTAGE_P_11  filted_data[0] // MOS管都闭合, 正端电压采集到的电压
#define SAMPLE_VOLTAGE_N_11  filted_data[1] // MOS管都闭合, 负端电压采集到的电压
#define SAMPLE_VOLTAGE_P_10  filted_data[2] // 正端闭合,负端断开, 正端采集到的电压
#define SAMPLE_VOLTAGE_N_01  filted_data[3] // 负端闭合,正端断开, 负端采集到的电压




static void insu_sample_set_mos_and_gain(struct insu_res_sample_param *__FAR param) {
    unsigned char stage = param->stage;
    unsigned char gain = param->gain[stage];

    switch (gain) {
    case 0:
        GainContol_GEN(1);
        break;
    case 1:
        GainContol_A0(1);
        GainContol_A1(1);
        GainContol_GEN(0);
        break;
    case 2:
        GainContol_A0(0);
        GainContol_A1(1);
        GainContol_GEN(0);
        break;
    case 3:
        GainContol_A0(1);
        GainContol_A1(0);
        GainContol_GEN(0);
        break;
    case 4:
        GainContol_A0(0);
        GainContol_A1(0);
        GainContol_GEN(0);
        break;
    default:
        break;
    }

    switch (stage) {
    case 0:
    case 1:
        PositiveMosOn();
        NegativeMosOn();
        break;
    case 2:
        PositiveMosOn();
        NegativeMosOff();
        break;
    case 3:
        PositiveMosOff();
        NegativeMosOn();
        break;
    default:
        PositiveMosOff();
        NegativeMosOff();
        break;
    }
}

static unsigned char insu_sample_which_channel(struct insu_res_sample_param *__FAR param) {
    unsigned char stage = param->stage;
    if (stage == 0 || stage == 2)  return RES_ADC_CHANNEL_P;
    return RES_ADC_CHANNEL_N;
}

static void insu_sample_set_filted_value(struct insu_res_sample_param *__FAR param, unsigned short v) {
    float real_value;
    unsigned char stage = param->stage;
    unsigned char gain = param->gain[stage];
    static const float gain_table[] = {
        5,
        5 + 80 / (24.9 + 0.045),
        5 + 80 / (9.31 + 0.045),
        5 + 80 / (5.49 + 0.045),
        5 + 80 / (2.49 + 0.045)
    };


    if (v > 3096) {
        if (gain > 0) --(param->gain[stage]);
    } else if (v < 2048) {
        if (gain < 4) ++(param->gain[stage]);
    }

    real_value = 0.5 + ((float)v) / gain_table[gain] * 64;
#pragma push
#pragma MESSAGE DISABLE C5919 //Warning : C5919: Conversion of floating to unsigned integral
    v = (unsigned short)real_value; 
#pragma pop

    switch (param->stage) {
    case 0:
        param->filted_value.p_val_pmos_on_nmos_on = v;
        return;
    case 1:
        param->filted_value.n_val_pmos_on_nmos_on = v;
        return;
    case 2:
        param->filted_value.p_val_pmos_on_nmos_off = v;
        return;
    case 3:
        param->filted_value.n_val_pmos_off_nmos_on = v;
        return;
    default:
        break;
    }
}

struct insu_res_sample_param insu_res_sample_param;
static void insu_sample_set_mos_and_gain(struct insu_res_sample_param *__FAR param);
static ASYNC_TIMER_TIMEOUT_TYPE insu_sample_new_cycle(async_timer_t *__FAR t);
static void insu_sample_finished_in_isr(const ADC_ChannelParameter *__FAR adc_param);
static ASYNC_TIMER_TIMEOUT_TYPE insu_res_start_sample(async_timer_t *__FAR t);
static ASYNC_TIMER_TIMEOUT_TYPE insu_res_start_sample(async_timer_t *__FAR t);
static char insu_sample_finished(struct insu_res_sample_param *__FAR param);
static ASYNC_TIMER_TIMEOUT_TYPE insu_res_sample_init(async_timer_t *__FAR t);

static ASYNC_TIMER_TIMEOUT_TYPE insu_sample_new_cycle(async_timer_t *__FAR t) {
    struct insu_res_sample_param *__FAR param = async_timer_get_data(t);
    param->stage = 0;
    insu_sample_set_mos_and_gain(param);
    async_timer_set_handler(t, insu_res_start_sample);
    return config_get(kEepromMosOnDelayIndex);
}


static void insu_sample_finished_in_isr(const ADC_ChannelParameter *__FAR adc_param)  {
    struct insu_res_sample_param *__FAR param = adc_param->pdata;
    async_event_call_trigger(param->finished_event_call);
}


static ASYNC_TIMER_TIMEOUT_TYPE insu_res_start_sample(async_timer_t *__FAR t) {
    struct insu_res_sample_param *__FAR param = async_timer_get_data(t);
    param->finished_event_call = async_event_call_post(insu_sample_finished, param);
    adc0_channel_start(insu_sample_which_channel(param), &param->adc_param);
    return 0;
}


static char insu_sample_finished(struct insu_res_sample_param *__FAR param) {
    unsigned char stage = param->stage;
    insu_sample_set_filted_value(param, average_filter((short *__FAR)param->sample_data, RES_SAMPLE_FILTER_SIZE));
    if (stage < 3) { // 设置mos为下一种情况采集
        ++param->stage;
        insu_sample_set_mos_and_gain(param);
        async_timer_register(insu_res_start_sample, config_get(kEepromMosOnDelayIndex), param);
        return ASYNC_EVENT_CALL_CANCEL;
    } else {
        // 四种情况全采集完成, 计算电阻.
        struct insu_res_unbalanced_bridge_result result;
        insu_res_unbalanced_bridge_calculate(&result, &param->filted_value);

        result.ratio_n *= RES_SAMPLE_RESISTOR;
        result.ratio_p *= RES_SAMPLE_RESISTOR;

        if (result.ratio_n >= 65535.0) {
            param->res_n = 65535;
        } else if (result.ratio_n < 0) {
            param->res_n = 0;
        } else {
#pragma push
#pragma MESSAGE DISABLE C5919 //Warning : C5919: Conversion of floating to unsigned integral
            param->res_n = (unsigned short)(result.ratio_n);
#pragma pop
        }


        if (result.ratio_p >= 65535.0) {
            param->res_p = 65535;
        } else if (result.ratio_p < 0) {
            param->res_p = 0;
        } else {
            param->res_p = (unsigned short)(result.ratio_p);
        }

        async_timer_register(insu_sample_new_cycle, 4000, param);
        return ASYNC_EVENT_CALL_CANCEL;
    }
}

static ASYNC_TIMER_TIMEOUT_TYPE insu_res_sample_init(async_timer_t *__FAR t) 
{
    struct insu_res_sample_param *__FAR param = NULL; 

    if(hardware_io_revision_get() >= HW_VER_116)
    {
        pt_insu_input_pins = insu_input_pins_1_16;
    }
    else
    {
        pt_insu_input_pins = insu_input_pins_1_15;
    }

    param = async_timer_get_data(t);
    param->adc_param.interval = 1000;
    param->adc_param.times = RES_SAMPLE_FILTER_SIZE;
    param->adc_param.buf = (unsigned int *far)(param->sample_data);
    param->adc_param.notify_finished = insu_sample_finished_in_isr;
    param->adc_param.pdata = param;
    return insu_sample_new_cycle(t);
}



void insu_res_analyze_init_xx(void) {
    async_init();
    async_timer_register(ads1015_sample_task_first, 0, &bat_hv_sample_param);
    async_timer_register(ads1015_sample_task_first, 0, &pch_sample_param);
    async_timer_register(ads1015_sample_task_first, 0, &pre_sample_param);
    async_timer_register(insu_res_sample_init, 0, &insu_res_sample_param);
}

