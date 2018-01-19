#ifndef __TEST_HCF4052_H__
#define __TEST_HCF4052_H__

enum hcf4052_channel {
    HCF4052_CHANNEL_0,
    HCF4052_CHANNEL_1,
    HCF4052_CHANNEL_2,
    HCF4052_CHANNEL_3,
    HCF4052_CHANNEL_NONE,
};

struct hcf4052_platform {
    const char *io_a_name;
    const char *io_b_name;
    const char *io_inh_name;
};

void hcf4052_init(const struct hcf4052_platform *__FAR platform);
void hcf4052_select_channel(const struct hcf4052_platform *__FAR platform, enum hcf4052_channel channel);
enum hcf4052_channel hcf4052_get_current_channel(const struct hcf4052_platform *__FAR platform);

extern const struct hcf4052_platform hcs4052_u37;

#endif