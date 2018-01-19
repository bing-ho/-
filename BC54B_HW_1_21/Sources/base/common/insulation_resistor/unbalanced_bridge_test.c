#include "unbalanced_bridge.h"
#include "unity_fixture.h"



TEST_GROUP(insu_res_unbalanced_bridge);

TEST_SETUP(insu_res_unbalanced_bridge) {
}

TEST_TEAR_DOWN(insu_res_unbalanced_bridge) {
}

static float R1, R2;
static float Rp, Rn;
static float voltage;
static struct insu_res_unbalanced_bridge_sample_value sample_value;
static struct insu_res_unbalanced_bridge_result result;

static void fake_sample_value(void) {
    float R = R1 + R2;
    float RN = Rn * R / (Rn + R);
    float RP = Rp * R / (Rp + R);

#pragma push
#pragma MESSAGE DISABLE C5919 //Warning : C5919: Conversion of floating to unsigned integral
    sample_value.p_val_pmos_on_nmos_on = (unsigned short)(float)(voltage * R2 * RP / R / (RN + RP) * 40 + 0.5);
    sample_value.p_val_pmos_on_nmos_off = (unsigned short)(float)(voltage * R2 * RP / R / (Rn + RP) * 40 + 0.5);
    sample_value.n_val_pmos_on_nmos_on = (unsigned short)(float)(voltage * R2 * RN / R / (RN + RP) * 40 + 0.5);
    sample_value.n_val_pmos_off_nmos_on = (unsigned short)(float)(voltage * R2 * RN / R / (RN + Rp) * 40 + 0.5);
#pragma pop
}

TEST(insu_res_unbalanced_bridge, test1) {
    voltage = 320.0 * 1000.0;
    R1 = 71.5 * 6;
    R2 = 0.392;

    Rn = 10000.0;
    Rp = 10000.0;

    fake_sample_value();
    insu_res_unbalanced_bridge_calculate(&result, &sample_value);
    (void)printf("res_n=%f, res_p=%f\n", result.ratio_n * (R1 + R2), result.ratio_p * (R1 + R2));

    TEST_ASSERT_FLOAT_WITHIN(0.005 * Rn, Rn, result.ratio_n * (R1 + R2));
    TEST_ASSERT_FLOAT_WITHIN(0.005 * Rp, Rp, result.ratio_p * (R1 + R2));
}

TEST_GROUP_RUNNER(insu_res_unbalanced_bridge) {
    RUN_TEST_CASE(insu_res_unbalanced_bridge, test1);
}

