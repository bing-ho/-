#include "test_adc.h"
#include "ads1015_impl.h"
#include "adc.h"

/*
const struct adc_onchip_channel *const adc_channels_v120[] = {
    &adc_onchip_temp_u48,            // PAD0
    //&adc_onchip_8vhall_adc_v116,      // 1 // removed from V120
    &adc_onchip_vbat_det_adc_v116,      // 2
    &adc_onchip_temp_t3_det_adc_v120,   // 3
    &adc_onchip_temp_t1_det_adc_v120,   // 4
    &adc_onchip_temp_u3_v116,           // 5
    &adc_onchip_cc_adc,                 // 6
    &adc_onchip_imd_hvp,                // 7
    &adc_onchip_motor_det_adc_v120,     // 8
    &adc_onchip_5vdet_adc_v116,         // 9
    &adc_onchip_temp_t4_det_adc_v120,   // 10
    &adc_onchip_temp_t2_det_adc_v120,   // 11
    &adc_onchip_v24_det_v116,           // 12
    &adc_onchip_cc2_adc_v116,           // 13
    &adc_onchip_imd_hvn,                // 14
    &adc_onchip_battery_v,              // 15
    NULL
};*/
enum {
    ADC_CHANNEL_ADC0_START,
    ADC_CHANNEL_TEMP_U48 = ADC_CHANNEL_ADC0_START,
    ADC_CHANNEL_8V_HALL,
    ADC_CHANNEL_VBAT_DET,
    ADC_CHANNEL_TEMP_T3,
    ADC_CHANNEL_TEMP_T1,
    ADC_CHANNEL_TEMP_U3,
    ADC_CHANNEL_CC,
    ADC_CHANNEL_IMD_HVP,
    ADC_CHANNEL_MOTOR_DET,
    ADC_CHANNEL_5V_DET,
    ADC_CHANNEL_TEMP_T4,
    ADC_CHANNEL_TEMP_T2,
    ADC_CHANNEL_24V_DET,
    ADC_CHANNEL_CC2,
    ADC_CHANNEL_IMD_HVN,
    ADC_CHANNEL_BATTERY,
    ADC_CHANNEL_ADC0_END,

    ADC_CHANNEL_ADS1015_START,
    ADC_CHANNEL_PCHP = ADC_CHANNEL_ADS1015_START,
    ADC_CHANNEL_PREP,
    ADC_CHANNEL_BATHV,
    ADC_CHANNEL_ADS1015_END,

    ADC_CHANNEL_ADC1_START,
    ADC_CHANNEL_HALL_C1 = ADC_CHANNEL_ADC1_START,
    ADC_CHANNEL_HALL_C2,
    ADC_CHANNEL_ADC1_END,
};

/*
static unsigned char channel_adc_to_adc0(unsigned char channel) {
    unsigned char i;
    struct channel_adc_adc0_map {
        unsigned char adc_channel;
        unsigned char adc0_channel;
    };

    static const struct channel_adc_adc0_map maps_v114[] = {
        {ADC_CHANNEL_CC2, 5},
        {ADC_CHANNEL_CC,        6},
        {ADC_CHANNEL_IMD_HVP,   7},
        {ADC_CHANNEL_24V_DET, 13},
        {ADC_CHANNEL_IMD_HVN,   14},
        {ADC_CHANNEL_BATTERY,   15},
    };
    static const struct channel_adc_adc0_map maps_v116[] = {
        {ADC_CHANNEL_CC, 6},
        {ADC_CHANNEL_IMD_HVP,   7},
        {ADC_CHANNEL_24V_DET,   12}, // 13 ==> 12
        {ADC_CHANNEL_CC2,       13}, // 5 ==> 13
        {ADC_CHANNEL_IMD_HVN, 14},
        {ADC_CHANNEL_BATTERY, 15},
    };

    const struct channel_adc_adc0_map *map;
    if(hardware_io_revision_get() >= HW_VER_116)//v1.16 & v1.20
    {
        map = &maps_v116[0];
    }
    else
    {
        map = &maps_v114[0];
    }
    for (i = 0; i < sizeof(maps_v114) / sizeof(maps_v114[0]); ++i) {
        if (map->adc_channel == channel) {
            return map->adc0_channel;
        }
        ++map;
    }

    return 0;
}
*/
static unsigned char channel_adc_to_adc1(unsigned char channel) {
    if (ADC_CHANNEL_HALL_C1 == channel) {
        return 3;
    }
    if (ADC_CHANNEL_HALL_C2 == channel) {
        return 2;
    }
    return 0;
}

static ADS1015_Channel channel_adc_to_ads1015(unsigned char channel) {
    if (channel == ADC_CHANNEL_PCHP) {
        return ADS1015_CHANNEL_0;
    }
    if (channel == ADC_CHANNEL_PREP) {
        return ADS1015_CHANNEL_1;
    }
    if (channel == ADC_CHANNEL_BATHV) {
        return ADS1015_CHANNEL_3;
    }
    return ADS1015_CHANNEL_2;
}

static void adc0_finished(const ADC_ChannelParameter *_PAGED param) {
    unsigned char *__FAR finished = (unsigned char *__FAR)param->pdata;
    *finished = 1;
}

#if 1
extern const struct adc_onchip_channel * const *adc_channels;
static unsigned short adc0_sample(unsigned char channel) {
    uint8_t i;
    unsigned short ret = 0;
    for (i = 0; ; ++i) 
    {
        if(adc_channels[i] == NULL)
        {
            return 0x7FFF;
        }
        if (adc_channels[i]->param.channel == channel) 
        {
            if(adc_onchip_read(adc_channels[i], &ret))
            {
                return ret;
            }
        }
    }
    return 0x7FFF;
}
#else
static unsigned short adc0_sample(unsigned char channel) {
    INT16U val;
    unsigned char finished = 0;

    ADC_ChannelParameter param;
    param.times = 1;
    param.interval = 1;
    param.buf = &val;
    param.notify_finished = adc0_finished;
    param.pdata = &finished;
    (void)adc0_channel_start(channel, &param);
    while (!finished) {
        OSTimeDly(2);
    }
    return val;
}
#endif

static unsigned short adc1_sample(unsigned char channel) {
    (void)channel;
    return 0x7FFF;
}

static unsigned short ads1015_sample(ADS1015_Channel channel) {
    ADS1015_Init(&g_ADS1015_BspInterface);
    return ADS1015_SingleConvert(&g_ADS1015_BspInterface, channel, ADS1015_FULL_SCALE_4096_MV);
}

static unsigned short test_adc_read_channel(unsigned char channel) {
    if ((channel >= ADC_CHANNEL_ADC0_START) && (channel < ADC_CHANNEL_ADC0_END) ) {
        //return adc0_sample(channel_adc_to_adc0(channel));
        return adc0_sample(channel);
    }
    if (channel >= ADC_CHANNEL_ADC1_START && channel < ADC_CHANNEL_ADC1_END) {
        return adc1_sample(channel_adc_to_adc1(channel));
    }
    if (channel >= ADC_CHANNEL_ADS1015_START && channel < ADC_CHANNEL_ADS1015_END) {
        return ads1015_sample(channel_adc_to_ads1015(channel));
    }
    return 0x7FFF;
}

static void test_adc_set_gain(unsigned char channel, unsigned char gain) {
    (void)gain;
    if (channel == ADC_CHANNEL_IMD_HVN) {
    } else if (channel == ADC_CHANNEL_IMD_HVP) {
    }
}

char test_rtc_get_channel(void *__FAR in_data, unsigned short in_len, void *__FAR out_data, unsigned short *out_len) {
    unsigned char channel;
    unsigned short value;

    if (!in_data) return 0;
    if (in_len < sizeof(channel)) return 0;
    if (!out_data) return 0;
    if (!out_len) return 0;

    channel = *((unsigned char *__FAR)in_data);
    value = test_adc_read_channel(channel);

    *((unsigned short * __FAR)out_data) = value;
    *out_len = sizeof(value);
    return 1;
}

char test_rtc_set_gain(void *__FAR in_data, unsigned short in_len, void *__FAR out_data, unsigned short *out_len) {
    unsigned char channel;
    unsigned char gain;

    if (!in_data) return 0;
    if (in_len < sizeof(channel) + sizeof(gain))  return 0;
    if (!out_len) return 0;

    channel = ((unsigned char *__FAR)out_data)[0];
    gain = ((unsigned char *__FAR)out_data)[1];

    test_adc_set_gain(channel, gain);
    *out_len = 0;
    return 1;
}

