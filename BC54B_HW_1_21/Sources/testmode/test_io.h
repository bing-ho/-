#ifndef __TEST_IO__
#define __TEST_IO__

#include "stdint.h"

void input_gpio_init(void);
uint8_t input_gpio_is_high_with_name(const char *name, uint8_t *is_high);

void output_ctrl_init(void);
uint8_t output_ctrl_set_with_name(const char *name, uint8_t is_high);
int8_t output_ctrl_is_high_with_name(const char *name, uint8_t *is_high);

uint8_t output_ctrl_dump_with_name(const char *name);
void output_ctrl_dump_all(void);

#endif
