#ifndef __INSU_RES_UNBALANCED_BRIDGE_H__
#define __INSU_RES_UNBALANCED_BRIDGE_H__

/*
http://i13.tietuku.com/5678389443fa817f.jpg

假设:
RP = (Rp // (R1 + R2))
RN = (Rn // (R1 + R2))
Rref = R1 + R2
K = VBAT * R2 / (R1 + R2)

分压公式:
VP0 = VBAT * RP / (RP + RN) * R2 / (R1 + R2)
VN0 = VBAT * RN / (RP + RN) * R2 / (R1 + R2)
VP1 = VBAT * RP / (RP + Rn) * R2 / (R1 + R2)
VN1 = VBAT * RN / (Rp + RN) * R2 / (R1 + R2)

==>
VP0 = RP / (RP + RN) * K
VN0 = RN / (RP + RN) * K
VP1 = RP / (RP + Rn) * K
VN1 = RN / (Rp + RN) * K

==>
K = VP0 + VN0
(RP + Rn) / RP = K / VP1
(Rp + RN) / RN = K / VN1

==>
K = VP0 + VN0
Rn / RP = K / VP1 - 1
Rp / RN = K / VN1 - 1

==>
K = VP0 + VN0
Rn = (K / VP1 - 1) * RP
Rp = (K / VN1 - 1) * RN

假设
Kp = (K / VP1 - 1) = (VP0 + VN0- VP1) / VP1
Kn = (K / VN1 - 1) = (VP0 + VN0- VN1) / VN1
R = R1 + R2
==>
Rn = Kp * (Rp // R)
Rp = Kn * (Rn // R)

=>
Kp = (VP0 + VN0 - VP1) / VP1
Kn = (VP0 + VN0 - VN1) / VN1

Rn = R * (Kp * Kn - 1)  / (1 + Kn)
Rp = R * (Kp * Kn - 1)  / (1 + Kp)

Ratio_N = (Kp * Kn - 1)  / (1 + Kn)
Ratio_P = (Kp * Kn - 1)  / (1 + Kp)

Ratio_N = (VP0 + VN0 - VN1 - VP1) / VP1
Ratio_P = (VP0 + VN0 - VN1 - VP1) / VN1

*/


struct insu_res_unbalanced_bridge_sample_value {
    // VP0
    unsigned short p_val_pmos_on_nmos_on;
    // VP1
    unsigned short p_val_pmos_on_nmos_off;
    // VN0
    unsigned short n_val_pmos_on_nmos_on;
    // VN1
    unsigned short n_val_pmos_off_nmos_on;
};

struct insu_res_unbalanced_bridge_result {
    float ratio_n;
    float ratio_p;
};

void insu_res_unbalanced_bridge_calculate(
        struct insu_res_unbalanced_bridge_result *__FAR result,
        const struct insu_res_unbalanced_bridge_sample_value *__FAR value);

#endif
