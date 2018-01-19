#include "test_pwrctl.h"
#include "derivative.h"

#define BMS_POWER_LATCH_ON()    DDR1AD1_DDR1AD16 = 1; PT1AD1_PT1AD16 = 1;
#define BMS_POWER_LATCH_OFF()   DDR1AD1_DDR1AD16 = 1; PT1AD1_PT1AD16 = 0;

#define BMS_POWER_LATCH_IS_ON() (PT1AD1_PT1AD16 != 0)

void test_pwrctl_init(void) {
    BMS_POWER_LATCH_ON()
}

char test_pwrctl_set_hold(void *__FAR in_data, unsigned short in_len, void *__FAR out_data, unsigned short *out_len) {
    unsigned char hold;
    
    (void)out_len;
    (void)out_data;

    if (!in_data) return 0;
    if (in_len < 1) return 0;
    if (!out_len) return 0;

    hold = *((unsigned char *__FAR)in_data);
    if (hold) {
        BMS_POWER_LATCH_ON();
    } else {
        BMS_POWER_LATCH_OFF();
    }

    *out_len = 0;
    return 1;
}


char test_pwrctl_get_hold(void *__FAR in_data, unsigned short in_len, void *__FAR out_data, unsigned short *out_len) {
    (void)in_len;
    (void)in_data;

    if (!out_data) return 0;
    if (!out_len) return 0;

    *((unsigned char *__FAR)out_data) = BMS_POWER_LATCH_IS_ON();
    *out_len = 1;

    return 1;
}


