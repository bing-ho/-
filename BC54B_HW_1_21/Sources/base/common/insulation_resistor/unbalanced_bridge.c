#include "unbalanced_bridge.h"

/*
Kp = (VP0 + VN0 - VP1) / VP1
Kn = (VP0 + VN0 - VN1) / VN1

Rn = R * (Kp * Kn - 1) / (1 + Kn)
Rp = R * (Kp * Kn - 1) / (1 + Kp)
*/


void insu_res_unbalanced_bridge_calculate(
    struct insu_res_unbalanced_result *__FAR result,
    const struct insu_res_unbalanced_sample_value *__FAR value) {

    signed long tmp;

    if ((!result) || (!value)) return;

    tmp = value->p_val_pmos_on_nmos_on
          + value->n_val_pmos_on_nmos_on
          - value->p_val_pmos_on_nmos_off
          - value->n_val_pmos_off_nmos_on;

    result->ratio_n = (float)tmp / (float)value->p_val_pmos_on_nmos_off;
    result->ratio_p = (float)tmp / (float)value->n_val_pmos_off_nmos_on;
}

