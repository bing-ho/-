/**
*
* Copyright (c) 2016 Ligoo Inc.
*
* @file soc0_dischg_factor_impl.c
* @brief SOC放电积分因子计算
* @note
* @author
* @date 2012-5-9
*
*/

#include "soc0_dischg_factor_impl.h"

#pragma MESSAGE DISABLE C5703
#pragma MESSAGE DISABLE C3604
#pragma MESSAGE DISABLE C5919

static CapAms_t soc0_cap_Ams; //soc0积分容量  单位：1Ams
static SOC0IntegralStatus  soc0_integral_status = kSOC0Init;

static INT8U soc_query_from_volt(INT16U volt, const INT16U *__FAR table);

  volatile INT16U factor1 = SOC0_DISCHG_FACTOR_DEFAULT_VAULE;


#define CURRENT_CONVERT_TO_MAS(CURRENT, TIME) ((INT32U)(CURRENT) * (TIME) / 10)

#pragma push
#pragma DATA_SEG __PPAGE_SEG CONST_TABLES
//磷酸铁锂SOC对应电压表
const INT16U __FAR kSOCTab_LiFePO4[101] = {2750,2800,2870,2940,3020,3060,3100,3110,3120,3130,
                                    3140,3150,3160,3170,3180,3190,3200,3202,3203,3205,
                                    3207,3208,3210,3212,3213,3215,3217,3218,3220,3222,
                                    3223,3225,3227,3228,3230,3232,3233,3235,3237,3238,
                                    3240,3242,3243,3245,3247,3248,3250,3252,3253,3255,
                                    3257,3258,3260,3262,3263,3265,3267,3268,3270,3272,
                                    3273,3275,3277,3278,3280,3282,3283,3285,3287,3288,
                                    3290,3291,3293,3295,3296,3298,3300,3304,3308,3310,
                                    3312,3314,3316,3318,3320,3322,3324,3326,3328,3330,
                                    3332,3334,3336,3340,3346,3352,3364,3376,3388,3400,
                                    3420};

//三元材料SOC对应电压表
const INT16U __FAR kSOCTab_LiNiCoMnO2[101] = {3000,3034,3068,3102,3137,3171,3205,3240,3247,3255,
                                        3263,3270,3278,3286,3294,3302,3310,3317,3325,3333,
                                        3341,3348,3356,3364,3372,3379,3387,3395,3403,3410,
                                        3418,3426,3434,3441,3449,3457,3465,3472,3480,3489,
                                        3497,3506,3514,3523,3531,3540,3549,3557,3566,3574,
                                        3583,3591,3600,3609,3617,3626,3634,3643,3651,3660,
                                        3669,3677,3686,3694,3703,3711,3720,3729,3738,3748,
                                        3757,3766,3775,3785,3794,3803,3812,3822,3831,3840,
                                        3849,3858,3868,3877,3886,3895,3905,3914,3923,3932,
                                        3942,3951,3960,3978,3995,4013,4030,4048,4065,4083,
                                        4100};
                                            
//钛酸锂SOC对应电压表
const INT16U __FAR kSOCTab_Li2TiO3[101] = {1969,1990,2010,2028,2044,2058,2072,2085,2098,2109,
                                     2119,2128,2137,2145,2152,2159,2165,2171,2175,2179,
                                     2182,2185,2187,2189,2190,2192,2195,2197,2199,2201,
                                     2202,2204,2207,2209,2210,2213,2214,2216,2219,2220,
                                     2222,2224,2226,2229,2231,2233,2235,2237,2239,2241,
                                     2244,2247,2250,2252,2254,2257,2259,2263,2266,2269,
                                     2272,2276,2279,2283,2286,2291,2295,2299,2303,2308,
                                     2312,2317,2322,2328,2331,2337,2342,2349,2354,2361,
                                     2367,2374,2381,2388,2395,2402,2408,2417,2425,2434,
                                     2443,2453,2463,2474,2486,2498,2513,2532,2558,2601,
                                     2663};  

#pragma pop
/*****************************************************************************
 *函数名称:soc_query_from_volt
 *函数功能:根据电压值查询对应的SOC值,运用二分法快速查表
 *参    数:INT16U volt单体电池电压值 const INT16U *table表格
 *返 回 值:SOC 单位1%/bit(0-100%)
 *修订信息:
 ******************************************************************************/
static INT8U soc_query_from_volt(INT16U volt, const INT16U *__FAR table)
{
    INT8U top = 0, bot = 0, n = 100;
    INT8U mid = 0;
    INT8U soc = 0;
    if (volt <= *table)
    {
        soc = 0;
    }
    else if (volt >= *(table + n))
    {
        soc = 100;
    }
    else if ((volt >= *table) && (volt <= *(table + n)))
    {
        bot = 0;
        top = n;
        for (; bot < top;)
        {
            mid = (INT8U) ((top + bot) >> 1);//mid?n/2
            if (volt >= (*(table + mid - 1)) && volt <= (*(table + mid + 1)))
            {
                if (volt >= *(table + mid))
                {
                    if ((*(mid + table + 1) - volt) < (volt - (*(mid + table))))
                    {
                        soc = mid + 1;
                    }
                    else
                    {
                        soc = mid;
                    }
                }
                else
                {
                    if (((*(table + mid)) - volt) < (volt - (*(table + mid - 1))))
                    {
                        soc = mid;
                    }
                    else
                    {
                        soc = mid - 1;
                    }
                }
                return soc;
            }
            else if (volt < *(table + mid))
            {
                top = mid - 1;
            }
            else
            {
                bot = mid + 1;
            }
        }

        if (volt > *(table + bot))
        {
            soc = bot;
        }
        else
        {
            soc = top;
        }
    }
    return soc;
}
/*****************************************************************************
 *函数名称:get_soc_from_volt_table
 *函数功能:根据不同的电池不同的单体电压查出对应的SOC值
 *参    数:INT16U volt电池电压值 
 *返 回 值:SOC 单位 0.01%/bit(0-10000)
 *修订信息:
 ******************************************************************************/
 INT16U get_soc_from_volt_table(INT16U volt)
{
    INT16U soc = 0;

#if  GUOBIAO_BATTERY_TYPE == 3
     soc = soc_query_from_volt(volt, kSOCTab_LiFePO4);
#elif  GUOBIAO_BATTERY_TYPE == 6
     soc = soc_query_from_volt(volt, kSOCTab_LiNiCoMnO2);
#elif  GUOBIAO_BATTERY_TYPE == 8
     soc = soc_query_from_volt(volt, kSOCTab_Li2TiO3);
#else
     soc = soc_query_from_volt(volt, kSOCTab_LiFePO4);
#endif

    soc = soc * 100;//放大100倍
    if (soc > SOC_MAX_VALUE)
        soc = SOC_MAX_VALUE;

    return soc;
}
/*****************************************************************************
 *函数名称:get_dischg_factor
 *函数功能:获取放电积分因子从增强型EEPROM获取
 *参    数:无
 *返 回 值:积分因子(分辨率0.001/bit)
 *修订信息:
 ******************************************************************************/
INT16U get_dischg_factor(void)
{
    INT16U tmp = 0;
    
    tmp = config_get(kSOC0DischgFactorIndex);
    
    if (tmp > SOC0_DISCHG_FACTOR_MAX_VAULE)
    {    
        tmp = SOC0_DISCHG_FACTOR_MAX_VAULE;
    }
    else if (tmp < SOC0_DISCHG_FACTOR_MIN_VAULE)
    {
        tmp = SOC0_DISCHG_FACTOR_MIN_VAULE;
    }
    
    return tmp;
}
/*****************************************************************************
 *函数名称:save_dischg_factor
 *函数功能:存储放电积分因子到增强型EEPROM中,仅当因子改变才存储(分辨率0.001/bit)
 *         放电积分因子限幅1-1.5
 *参    数:INT16U factor
 *返 回 值:无
 *修订信息:
 ******************************************************************************/
static void save_dischg_factor(INT16U factor)
{
    INT16U tmp_16u = 0;
    OS_CPU_SR cpu_sr = 0;
    
    if (factor > SOC0_DISCHG_FACTOR_MAX_VAULE)
        tmp_16u = SOC0_DISCHG_FACTOR_MAX_VAULE;
    else if (factor < SOC0_DISCHG_FACTOR_MIN_VAULE)
        tmp_16u = SOC0_DISCHG_FACTOR_MIN_VAULE;
    else
        tmp_16u = factor;
     factor1 =  tmp_16u;
    if (get_dischg_factor() != tmp_16u)
    {
        config_save(kSOC0DischgFactorIndex, tmp_16u);
    }
    
}
/*****************************************************************************
 *函数名称:get_soc_by_Ams
 *函数功能:通过Ams获取SOC
 *参    数:CapAms_t Ams容量
 *返 回 值:SOC  0.01%
 *修订信息:
 ******************************************************************************/
INT16U get_soc_by_Ams(CapAms_t cap)
{
    INT16U cap_ah;
    CapAms_t cap_Ams_max = 0;
    
    cap_Ams_max = CAP_TENFOLD_AH_TO_MAS(config_get(kTotalCapIndex));//cap_Ams_max = CAP_AH_TO_MAS(config_get(kTotalCapIndex));
    if (cap > cap_Ams_max) 
    {
        return SOC_MAX_VALUE;
    }
    cap_ah =  CAP_MAS_TO_TENFOLD_AH(cap);//cap_ah =  CAP_MAS_TO_AH(cap);
    return soc_from_cap(cap_ah, config_get(kTotalCapIndex));
}
/*****************************************************************************
 *函数名称:full_chg_set_dischg_factor
 *函数功能:充满计算放电积分系数,并存储。
 *参    数:无
 *返 回 值:无
 *修订信息:
 ******************************************************************************/

void full_chg_set_dischg_factor(void)
{
    #if BMS_SUPPORT_SOC0_DISCHG_FACTOR ==1
    
    INT16U factor = SOC0_DISCHG_FACTOR_DEFAULT_VAULE;
    INT16U soc0 = 0;
    CapAms_t tmp_Ams = 0,total_cap_ams = CAP_TENFOLD_AH_TO_MAS(config_get(kTotalCapIndex));//CAP_AH_TO_MAS(config_get(kTotalCapIndex));
    OS_CPU_SR cpu_sr = 0;
    
    if (!charger_is_connected())
        return;
    
    OS_ENTER_CRITICAL();
    tmp_Ams = soc0_cap_Ams;
    OS_EXIT_CRITICAL();
    soc0 = (INT16U)(((double)tmp_Ams * SOC_MAX_VALUE) / total_cap_ams + 0.5);//soc_from_cap2(tmp_Ams, config_get(kTotalCapIndex));//get_soc_by_Ams(tmp_Ams);

    factor = (INT16U) (((INT32U)soc0 * 1000) / bcu_get_SOC());//factor = (INT16U) (((INT32U) bcu_get_SOC() * 1000) / soc0);
    save_dischg_factor(factor);
        
    #endif
}
/*****************************************************************************
 *函数名称:set_chg_start_SOC0
 *函数功能:计算SOC0,在充电起始时刻调用.用于获取较准确的初始SOC值
 *参    数:CapAms_t battery_Ams原积分安时
 *返 回 值:无
 *修订信息:
 ******************************************************************************/
static void set_chg_start_SOC0()
{
    volatile INT16U volt_soc = 0;
    INT32U tmp_cap = 0;
    OS_CPU_SR cpu_sr = 0;
    volt_soc = bcu_get_low_voltage();
    volt_soc = get_soc_from_volt_table(bcu_get_low_voltage());
    
    tmp_cap = ((INT32U) volt_soc * config_get(kTotalCapIndex)) / SOC_MAX_VALUE;
    //查表SOC范围判断 20-80%范围的电压变化幅度不大,不准确
    if ((volt_soc <= PERCENT_TO_SOC(20)) || (volt_soc >= PERCENT_TO_SOC(80)))
    {
        OS_ENTER_CRITICAL();
        soc0_cap_Ams = CAP_TENFOLD_AH_TO_MAS(tmp_cap);//CAP_AH_TO_MAS(tmp_cap);
        OS_EXIT_CRITICAL();
    }
    else
    {
        OS_ENTER_CRITICAL();
        soc0_cap_Ams = bcu_get_left_cap_interm();//使用原Ams 
        OS_EXIT_CRITICAL();
    }
    
}
/*****************************************************************************
 *函数名称:SOC0_IntegralAlgorithm
 *函数功能:SOC0充电积分计算
 *参    数:INT32U integral_time积分时间
 *         INT16S current电流
 *返 回 值:无
 *修订信息:
 ******************************************************************************/

void SOC0_IntegralAlgorithm(INT32U integral_time, INT16S current)
{
    #if BMS_SUPPORT_SOC0_DISCHG_FACTOR == 1
    static INT8U dly = 0;
    static INT16U last_soc0 = 0; //上一次存储的SOC0
    INT16U factor = 0;
    INT16U soc0 = 0; //本次计算的SOC
    OS_CPU_SR cpu_sr = 0;
    //CapAms_t max_Ams = 0; //最大容量
    CapAms_t tmp_Ams = 0,total_cap_ams = CAP_TENFOLD_AH_TO_MAS(config_get(kTotalCapIndex));//CAP_AH_TO_MAS(config_get(kTotalCapIndex));
    CapAms_t single_integral_max_value = 0;
    
    //max_Ams = CAP_AH_TO_MAS(config_get(kTotalCapIndex));
    single_integral_max_value = CURRENT_CONVERT_TO_MAS(config_get(kCurSenTypeIndex) * 10, integral_time);
    
    switch(soc0_integral_status)
    {
        case kSOC0Init:
            last_soc0 = 0;
            //不在充电时,计算SOC0
            
            if ((charger_is_connected()==TRUE) || (charger_is_charging()==TRUE)) //充电
            {
                if(++dly>50)
                {
                dly = 0;    
                soc0_integral_status = kSOC0Integraling;
                //set_chg_start_SOC0();
                OS_ENTER_CRITICAL();
                soc0_cap_Ams = bcu_get_left_cap_interm();//使用原Ams 
                OS_EXIT_CRITICAL();
                }
            }
            else
            {
               dly = 0;
            }

        break;
        case kSOC0Integraling:
        dly = 0;
           if(current > 0)
            {
                tmp_Ams = CURRENT_CONVERT_TO_MAS(current, integral_time);
            }
            else
            {
                tmp_Ams = 0;
            }
            if (tmp_Ams > single_integral_max_value)
                tmp_Ams = single_integral_max_value;

            OS_ENTER_CRITICAL();
            soc0_cap_Ams = soc0_cap_Ams + tmp_Ams;
            //if (soc0_cap_Ams > max_Ams)
            //    soc0_cap_Ams = max_Ams;
             tmp_Ams =  soc0_cap_Ams;
            OS_EXIT_CRITICAL();

            soc0 = (INT16U)(((double)tmp_Ams * SOC_MAX_VALUE) / total_cap_ams + 0.5);//soc_from_cap2(tmp_Ams, config_get(kTotalCapIndex));//get_soc_by_Ams(tmp_Ams);
           // _soc0 =  soc0;
           // if (soc0 >= SOC_MAX_VALUE)//积满,放电积分因子为1
           // {
           //     save_dischg_factor(SOC0_DISCHG_FACTOR_DEFAULT_VAULE);
           // }
           // else if (abs(soc0 - last_soc0) >= PERCENT_TO_SOC(1)) //1%存储一次
            if (abs(soc0 - last_soc0) >= PERCENT_TO_SOC(1)) //1%存储一次
            {
                last_soc0 = soc0;

                factor = (INT16U) (((INT32U)soc0 * 1000)/bcu_get_SOC());//factor = (INT16U) (((INT32U)bcu_get_SOC() * 1000) / soc0);
                save_dischg_factor(factor);
            }
            if ((charger_is_connected()==FALSE) && (charger_is_charging()==FALSE))//if (!charger_is_connected()) //非充电
            {
                soc0_integral_status = kSOC0Init;
            }

        break;    
        default:
        break;
    }
    
    #endif
}








