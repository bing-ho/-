#include "sd2405al_impl.h"
#include "sd2405al_private.h"

#include "derivative.h"
#include "softi2c.h"
#include "ucos_ii.h"
#include "power_down_data_save_impl.h"


#include "hcs12_i2c_private.h"

#pragma push
#pragma DATA_SEG __RPAGE_SEG PAGED_RAM_INSULATION
static struct hcs12_i2c i2c1;
#pragma pop

static void platform_init(const struct sd2405_platform *platform) {
    struct hcs12_i2c *__FAR i2c = platform->platform_pdata;
    i2c->regs = (struct hcs12_i2c_regs *)0x000000B0;
    hcs12_i2c_init(i2c, 0x48); // 0x1F : BUS_CLOCK / 160
}

char i2c_transmit(const struct sd2405_platform *platform,
                  const unsigned char *__FAR w,
                  unsigned char wlen,
                  unsigned char *__FAR r,
                  unsigned char rlen) {
    struct hcs12_i2c *__FAR i2c = (struct hcs12_i2c * __FAR)platform->platform_pdata;
    return hcs12_i2c_write_then_read(i2c, 0x32, w, wlen, r, rlen);
}

static void delay_ms(const struct sd2405_platform *platform, unsigned short howmany) {
    (void)platform;
    OSTimeDly(howmany);
}

void i2c1_isr_handler(void) {
    hcs12_i2c_isr(&i2c1);
}

static const struct sd2405_platform this_sd2405_platform = {
    (void *__FAR) &i2c1, // slave addr
    platform_init,
    i2c_transmit,
    delay_ms,
};
struct sd2405 this_sd2405_impl = {
    &this_sd2405_platform,
};

sd2405_t const sd2405_impl = &this_sd2405_impl; 



