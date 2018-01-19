/**
*
* Copyright (c) 2011 Ligoo Inc. 
* 
* @file  NTC.c
* @brief 
* @note 
* @version 1.00
* @author 
* @date 2011/10/24  
*
*/ 
#ifndef NTC_SOURCE
#define NTC_SOURCE

#include "includes.h"
#include "NTC.h"
#include "bms_bcu.h"

#pragma MESSAGE DISABLE C5919 

#pragma push
#pragma DATA_SEG __PPAGE_SEG CONST_TABLES

/*�Ϸ����������¸�(100K��)����֮һ��ѹ��ѹ��*/ 
const INT16U NTCTAB_HFSJ_ref_5000mV[]=
{
    2972, 2966, 2960, 2953, 2946, 2939, 2931, 2923, 2914, 2905,//0-9
    2896, 2886, 2876, 2865, 2853, 2842, 2829, 2816, 2803, 2789,
    2774, 2759, 2743, 2727, 2710, 2692, 2674, 2655, 2635, 2614,
    2594, 2572, 2549, 2526, 2502, 2478, 2452, 2427, 2400, 2373,
    2346, 2318, 2289, 2260, 2230, 2200, 2169, 2138, 2106, 2074,
    2041, 2008, 1975, 1942, 1908, 1874, 1840, 1806, 1771, 1737,
    1703, 1668, 1634, 1600, 1566, 1532, 1498, 1465, 1432, 1399,
    1366, 1334, 1302, 1270, 1239, 1208, 1177, 1148, 1118, 1090,
    1061, 1033, 1006, 979,  953,  927,  902,  877,  853,  829,
    806,  784,  762,  741,  720,  700,  680,  661,  642,  623,
    606,  588,  571,  555,  539,  523,  508,  493,  479,  465,
    452,  439,  426,  413,  401,  390,  378,  367,  356,  346,
    336,  326,  317,  308,  299,  290,  282,  274,  266,  258,
    251,  244,  237,  230,  223,  217,  211,  205,  199,  194,
    188,  183,  178,  173,  169,  164,  160,  155,  151,  147,
    143,  139,  135,  132,  128,  125,  122,  119,  115,  113,
    110,  107,  104,  101,  99,   96,   94,   91,   89,   87,
    85
};

const INT16U NTCTAB_HFSJ_ref_4096mV[]=
{
    2435, 2430, 2425, 2419, 2413, 2408, 2401, 2395, 2387, 2380,
    2372, 2364, 2356, 2347, 2337, 2328, 2318, 2307, 2296, 2285,
    2272, 2260, 2247, 2234, 2220, 2205, 2191, 2175, 2159, 2141,
    2125, 2107, 2088, 2069, 2050, 2030, 2009, 1988, 1966, 1944,
    1922, 1899, 1875, 1851, 1827, 1802, 1777, 1751, 1725, 1699,
    1672, 1645, 1618, 1591, 1563, 1535, 1507, 1479, 1451, 1423,
    1395, 1366, 1339, 1311, 1283, 1255, 1227, 1200, 1173, 1146,
    1119, 1093, 1067, 1040, 1015, 990 , 964 , 940 , 916 , 893 ,
    869 , 846 , 824 , 802 , 781 , 759 , 739 , 718 , 699 , 679 ,
    660 , 642 , 624 , 607 , 590 , 573 , 557 , 541 , 526 , 510 ,
    496 , 482 , 468 , 455 , 442 , 428 , 416 , 404 , 392 , 381 ,
    370 , 360 , 349 , 338 , 328 , 319 , 310 , 301 , 292 , 283 ,
    275 , 267 , 260 , 252 , 245 , 238 , 231 , 224 , 218 , 211 ,
    206 , 200 , 194 , 188 , 183 , 178 , 173 , 168 , 163 , 159 ,
    154 , 150 , 146 , 142 , 138 , 134 , 131 , 127 , 124 , 120 ,
    117 , 114 , 111 , 108 , 105 , 102 , 100 , 97  , 94  , 93  ,
    90  , 88  , 85  , 83  , 81  , 79  , 77  , 75  , 73  , 71  ,
    70
};

#define NCP_RES_TO_VOLT(RES)    ((INT16U)((float)5000*RES/(RES+10)))

const INT16U NTCTAB_NCP18XH103_ref_5000mV[] =
{
#if 0
    //RES  25���Ӧ10k��ֵ factor: k��/5��  range:-40~125
    /*195.652, 148.171, 113.347, 87.559, 68.237, 53.65,   42.506,  33.892,  27.219, 22.021,
    17.926,  14.674,  12.081,  10,     8.315,  6.948,   5.834,   4.917,   4.161,  3.535,
    3.014,   2.586,   2.228,   1.925,  1.669,  1.452,   1.268,   1.11,    0.974,  0.858,
    0.758,   0.672,   0.596,   0.531*/

    NCP_RES_TO_VOLT(195.652), NCP_RES_TO_VOLT(148.171), NCP_RES_TO_VOLT(113.347), NCP_RES_TO_VOLT(87.559), NCP_RES_TO_VOLT(68.237), 
    NCP_RES_TO_VOLT(53.65),   NCP_RES_TO_VOLT(42.506),  NCP_RES_TO_VOLT(33.892),  NCP_RES_TO_VOLT(27.219), NCP_RES_TO_VOLT(22.021),
    NCP_RES_TO_VOLT(17.926),  NCP_RES_TO_VOLT(14.674),  NCP_RES_TO_VOLT(12.081),  NCP_RES_TO_VOLT(10),     NCP_RES_TO_VOLT(8.315),  
    NCP_RES_TO_VOLT(6.948),   NCP_RES_TO_VOLT(5.834),   NCP_RES_TO_VOLT(4.917),   NCP_RES_TO_VOLT(4.161),  NCP_RES_TO_VOLT(3.535),
    NCP_RES_TO_VOLT(3.014),   NCP_RES_TO_VOLT(2.586),   NCP_RES_TO_VOLT(2.228),   NCP_RES_TO_VOLT(1.925),  NCP_RES_TO_VOLT(1.669),  
    NCP_RES_TO_VOLT(1.452),   NCP_RES_TO_VOLT(1.268),   NCP_RES_TO_VOLT(1.11),    NCP_RES_TO_VOLT(0.974),  NCP_RES_TO_VOLT(0.858),
    NCP_RES_TO_VOLT(0.758),   NCP_RES_TO_VOLT(0.672),   NCP_RES_TO_VOLT(0.596),   NCP_RES_TO_VOLT(0.531)
#endif
//�ο���ѹ 5000mV       4757=((INT16U)((float)5000*195.652/(195.652+10)))
    4757, 4684, 4595, 4487, 4361, 4214, 4048, 3861, 3657, 3439, 
    3210, 2974, 2736, 2500, 2270, 2050, 1842, 1648, 1469, 1306, 
    1158, 1027,  911,  807,  715,  634,  563,  500,  444,  395, 
     352,  315,  281,  252, 
};


const INT16U NTCTAB_NCP18XH103_ref_4096mV[] =
{
    3897, 3837, 3764, 3676, 3572, 3452, 3316, 3163, 2995, 2817, 
    2629, 2436, 2241, 2048, 1860, 1679, 1509, 1350, 1204, 1070, 
    949 , 842 , 746 , 661 , 586 , 519 , 461 , 409 , 364 , 324 ,
    289 , 258 , 230 , 207
};

#if 0 // most the same as NTCTAB_NCP18XH103
const INT16U NTCTAB_NTCG163JF103FTB_ref_5000mV[] =
{
#if 0
    //RES  25���Ӧ10k��ֵ factor: k��/5��  range:-40~125
    /*18.85, 14.429, 11.133, 8.656, 6.779, 5.346, 4.245, 3.393, 2.728, 2.207, 
    1.796, 1.47, 1.209, 1, 0.831, 0.694, 0.583, 0.491, 0.416, 0.354, 
    0.302, 0.259, 0.223, 0.192, 0.167, 0.145, 0.127, 0.111, 0.0975, 0.086, 
    0.076, 0.0674, 0.0599, 0.0534, */
#endif
//�ο���ѹ 5000mV       4748=((INT16U)((float)5000*18.85*10/(18.85*10+10)))
    4748, 4676, 4588, 4482, 4357, 4212, 4047, 3862, 3659, 3441, 
    3212, 2976, 2737, 2500, 2269, 2048, 1841, 1647, 1469, 1307, 
    1160, 1029,  912,  805,  716,  633,  563,  500,  444,  396,  
    353,   316,  283,  253,  
};
/* //RES  25���Ӧ10k��ֵ factor: k��/5��  range:-40~125, ref_vol=4096
�C40    18.85    3890 
�C35    14.429   3831 
�C30    11.133   3758 
�C25    8.656    3672 
�C20    6.779    3569 
�C15    5.346    3451 
�C10    4.245    3315 
�C5     3.393    3164 
  0     2.728    2997 
  5     2.207    2819 
  10    1.796    2631 
  15    1.47     2438 
  20    1.209    2242 
  25    1        2048 
  30    0.831    1859 
  35    0.694    1678 
  40    0.583    1509 
  45    0.491    1349 
  50    0.416    1203 
  55    0.354    1071 
  60    0.302    950 
  65    0.259    843 
  70    0.223    747 
  75    0.192    660 
  80    0.167    586 
  85    0.145    519 
  90    0.127    462 
  95    0.111    409 
  100   0.0975   364 
  105   0.086    324 
  110   0.076    289 
  115   0.0674   259 
  120   0.0599   231 
  125   0.0534   208 
*/
const INT16U NTCTAB_NTCG163JF103FTB_ref_4096mV[] =
{
    3890, 3831, 3758, 3672, 3569, 3451, 3315, 3164, 2997, 2819, 
    2631, 2438, 2242, 2048, 1859, 1678, 1509, 1349, 1203, 1071, 
    950,  843,  747,  660,  586,  519,  462,  409,  364,  324,  
    289,  259,  231,  208
};
#endif

#if 0
const INT16U NCP18WB473[] = 
{
    //RES  25���Ӧ47k��ֵ factor: k��/5��  range:-40~125
    /*1747.92, 1245.428, 898.485, 655.802, 483.954, 360.85, 271.697, 206.463, 158.214, 122.259, 
    95.227, 74.73, 59.065, 47, 37.643, 30.334, 24.591, 20.048, 16.433, 13.539, 
    11.209, 9.328, 7.798, 6.544, 5.518, 4.674, 3.972, 3.388, 2.902, 2.494, 2.15, 1.86, 1.615, 1.406*/
    
    //volt = 2500 * RES / (RES + 47)  
    2435, 2409, 2376, 2333, 2279, 2212, 2131, 2036, 1927, 1806, 
    1674, 1535, 1392, 1250, 1112, 981,  859,  748,  648,  559, 
    481,  414,  356,  306,  263,  226,  195,  168,  145,  126, 
    109, 95, 83, 73
};
#endif

/*�����¸�(100K��)����֮һ��ѹ��ѹ��*/ 
/*const INT16U NTCTAB_BeiChuan_ref_5000mV[]=
{
    2986, 2980, 2975, 2969, 2963, 2956, 2949, 2942, 2934, 2926, //0-9
    2917, 2908, 2899, 2888, 2878, 2867, 2855, 2843, 2831, 2817,
    2803, 2789, 2773, 2758, 2741, 2724, 2706, 2687, 2668, 2648,
    2628, 2606, 2584, 2561, 2537, 2513, 2488, 2462, 2436, 2409,
    2381, 2352, 2323, 2293, 2262, 2231, 2199, 2167, 2135, 2102,
    2068, 2034, 2000, 1965, 1929, 1894, 1858, 1823, 1787, 1751,
    1715, 1680, 1644, 1608, 1572, 1536, 1501, 1466, 1431, 1396,
    1362, 1329, 1295, 1262, 1229, 1197, 1166, 1135, 1104, 1074,
    1045, 1015,  987,  959,  931,  905,  878,  853,  828,  804,  
    780,   756,  734,  711,  690,  669,  648,  629,  610,  591,  
    573,   555,  538,  521,  505,  489,  474,  459,  445,  431,  
    418,   404,  392,  379,  368,  356,  345,  334,  324,  314,  
    304,   295,  285,  277,  268,  260,  252,  244,  236,  229,  
    222,   215,  209,  202,  196,  190,  185,  179,  174,  168,  
    163,   158,  154,  149,  145,  140,  136,  132,  128,  125,  
    121,   117,  114,  111,  108,  104,  101,   99,   96,   93,  
    90,     88,   85,   83,   81,   78,   76,   74,   72,   70,  
    68
};
const INT16U NTCTAB_BeiChuan_ref_4096mV[]=
{
    2446, 2441, 2437, 2432, 2427, 2422, 2416, 2410, 2404, 2397, 
    2390, 2382, 2375, 2366, 2358, 2349, 2339, 2329, 2319, 2308, 
    2296, 2285, 2272, 2259, 2245, 2232, 2217, 2201, 2186, 2169, 
    2153, 2135, 2117, 2098, 2078, 2059, 2038, 2017, 1996, 1973, 
    1951, 1927, 1903, 1878, 1853, 1828, 1801, 1775, 1749, 1722, 
    1694, 1666, 1638, 1610, 1580, 1552, 1522, 1493, 1464, 1434, 
    1405, 1376, 1347, 1317, 1288, 1258, 1230, 1201, 1172, 1144,
    1116, 1089, 1061, 1034, 1007, 981,  955,  930,  904,  880, 
    856,  831,  809,  786,  763,  741,  719,  699,  678,  659, 
    639,  619,  601,  582,  565,  548,  531,  515,  500,  484, 
    469,  455,  441,  427,  414,  401,  388,  376,  365,  353, 
    342,  331,  321,  310,  301,  292,  283,  274,  265,  257, 
    249,  242,  233,  227,  220,  213,  206,  200,  193,  188, 
    182,  176,  171,  165,  161,  156,  152,  147,  143,  138, 
    134,  129,  126,  122,  119,  115,  111,  108,  105,  102, 
    99,   96,   93,   91,   88,   85,   83,   81,   79,   76, 
    74,   72,   70,   68,   66,   64,   62,   61,   59,   57, 
    56

};
*/

/*�Ͼ�ʱ�� CWF4B-103F3950 �¸�: -40~125 , Vol= 4096*R/(R+10K) */
/*����ֵ(K):     346.864, 322.61, 300.373, 279.955, 261.179, ... , 0.39, 0.38, 0.37, 0.361, 0.352, 0.343*/
const INT16U NTCTAB_CWF4B103F3950_ref_4096mV[]=  //166  ��չ�¶�ֵ��126-190
{
    3981, 3972, 3964, 3954, 3944, 3934, 3923, 3912, 3900, 3888,
    3875, 3861, 3846, 3831, 3816, 3799, 3782, 3765, 3746, 3727,
    3707, 3686, 3664, 3642, 3618, 3594, 3569, 3543, 3517, 3489,
    3461, 3431, 3401, 3370, 3338, 3305, 3272, 3237, 3202, 3166, 
    3129, 3091, 3053, 3014, 2974, 2933, 2892, 2850, 2808, 2765, 
    2722, 2678, 2634, 2590, 2545, 2500, 2455, 2409, 2364, 2318, 
    2273, 2227, 2182, 2136, 2091, 2048, 2001, 1957, 1913, 1869, 
    1826, 1783, 1740, 1698, 1657, 1616, 1576, 1536, 1497, 1458, 
    1421, 1383, 1347, 1311, 1276, 1242, 1208, 1175, 1143, 1112, 
    1081, 1051, 1022, 993,  965,  938,  911,  885,  860,  836, 
    812,  789,  766,  744,  723,  702,  682,  662,  643,  625,  
    607,  590,  573,  556,  540,  525,  510,  495,  481,  468,  
    454,  441,  429,  417,  405,  394,  382,  372,  361,  351,  
    341,  332,  323,  314,  305,  297,  289,  281,  274,  266,  
    259,  252,  246,  239,  232,  226,  220,  215,  209,  203,  
    198,  193,  188,  183,  179,  174,  170,  165,  161,  157,  
    153,  149,  146,  142,  139,  134,  131,  128,  124,  121, 
    118,  115,  112,  110,  107,  104,  102,  99,   97,   94,  
    92,   90,   88,   86,   84,   82,   80,   78,   76,   74,  
    73,   71,   69,   68,   66,   65,   63,   62,   60,   59,  
    57,   56,   55,   54,   52,   51,   50,   49,   48,   47,  
    45,   44,   43,   42,   41,   40,   39,   38,   38,   37,  
    36,   35,   34,   34,   33,   32,   31,   31,   30,   29,  
    28
};

const INT16U NTCTAB_CWF4B103F3950_ref_5000mV[166]=
{
    4860, 4849, 4839, 4827, 4814, 4802, 4789, 4775, 4761, 4746, 
    4730, 4713, 4695, 4677, 4658, 4637, 4617, 4596, 4573, 4550, 
    4525, 4500, 4473, 4446, 4417, 4387, 4357, 4325, 4293, 4259, 
    4225, 4188, 4152, 4114, 4075, 4034, 3994, 3951, 3909, 3865, 
    3820, 3773, 3727, 3679, 3630, 3580, 3530, 3479, 3428, 3375, 
    3323, 3269, 3215, 3162, 3107, 3052, 2997, 2941, 2886, 2830, 
    2775, 2719, 2664, 2607, 2552, 2500, 2443, 2389, 2335, 2281, 
    2229, 2177, 2124, 2073, 2023, 1973, 1924, 1875, 1827, 1780, 
    1735, 1688, 1644, 1600, 1558, 1516, 1475, 1434, 1395, 1357, 
    1320, 1283, 1248, 1212, 1178, 1145, 1112, 1080, 1050, 1021, 
    991,  963,  935,  908,  883,  857,  833,  808,  785,  763, 
    741,  720,  699,  679,  659,  641,  623,  604,  587,  571, 
    554,  538,  524,  509,  494,  481,  466,  454,  441,  428, 
    416,  405,  394,  383,  372,  363,  353,  343,  334,  325, 
    316,  308,  300,  292,  283,  276,  269,  262,  255,  248, 
    242,  236,  229,  223,  219,  212,  208,  201,  197,  192, 
    187,  182,  178,  173,  170,  165
};

#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))
INT8U __FAR NTCTAB_SIZE[] = 
{
    //ARRAY_SIZE(NTCTAB_HFSJ_ref_5000mV),
    //ARRAY_SIZE(NTCTAB_NCP18XH103_ref_5000mV),
    //ARRAY_SIZE(NTCTAB_CWF4B103F3950_ref_5000mV),
    //ARRAY_SIZE(NTCTAB_NTCG163JF103FTB_ref_4096mV),   
    ARRAY_SIZE(NTCTAB_HFSJ_ref_5000mV),
    ARRAY_SIZE(NTCTAB_HFSJ_ref_4096mV),
    ARRAY_SIZE(NTCTAB_NCP18XH103_ref_5000mV),
    ARRAY_SIZE(NTCTAB_NCP18XH103_ref_4096mV),
    ARRAY_SIZE(NTCTAB_CWF4B103F3950_ref_5000mV),
    ARRAY_SIZE(NTCTAB_CWF4B103F3950_ref_4096mV) 
};

const __FAR INT16U* __FAR NTCTAB[] = 
{
    NTCTAB_HFSJ_ref_5000mV,
    NTCTAB_HFSJ_ref_4096mV,
    NTCTAB_NCP18XH103_ref_5000mV,
    NTCTAB_NCP18XH103_ref_4096mV,
    NTCTAB_CWF4B103F3950_ref_5000mV,
    NTCTAB_CWF4B103F3950_ref_4096mV
    //NTCTAB_NTCG163JF103FTB_ref_5000mV,
    //NTCTAB_NTCG163JF103FTB_ref_4096mV
};

#pragma pop

/*
* 
* @brief ����NTC��ѹֵ��ͨ���۰뷨��ѯ��Ӧ���¶�,  ������������²��2�ȣ���ʹ�����Բ�ֵ��������
* @note
* @param[in]    volt: NTC��ѹֵ����λmv 
*               tbl_name: NTC volt-temperature ��
* @param[out]   ��               
* @return       NTC�¶�ֵ��
* 
*/
INT8U TempQuery(INT16U volt , NTCTAB_NAME tbl_name) 
{
    const INT16U *__FAR table = NULL;//NTCTAB[tbl_name];
    INT8U   top,bot;
    INT8U   mid;
    INT8U   T;
    INT16U   resolution = 0;
    INT8U *__FAR table_size =  (INT8U *__FAR)NTCTAB_SIZE;
    
#if BMS_SUPPORT_BY5248D == 0    
#if BMS_SUPPORT_HARDWARE_BEFORE == 1    // yang 20161213
    if(hardware_io_revision_get() == HW_VER_120)
    {
        table = (INT16U *__FAR)(NTCTAB[tbl_name*2 + 1]);
    }
    else
    {
        table = (INT16U *__FAR)(NTCTAB[tbl_name*2 + 0]);
    }
#else
    table = (INT16U *__FAR)(NTCTAB[tbl_name*2 + 1]);
#endif
  
#else
    table = (INT16U *__FAR)(NTCTAB[tbl_name*2 + 1]);
#endif  
  
  
  
    if(volt > table[0]
    ||volt < table[table_size[tbl_name*2 + 1]-1])
    {
        return 0xFF;
    }
    else// if((volt<=*table)&&(volt>=*(table+n)))
    {
        bot = 0;
        top = table_size[tbl_name*2 + 1]-1;
        for(;bot<top;)
        {
            mid = (INT8U)((top+bot)>>1);//mid��n/2
            if(volt<=(*(table+mid-1))&&volt>=(*(table+mid+1)))
            {
                if(volt>=*(table+mid))
                {
                    if(((*(table+mid-1))-volt)>(volt-(*(table+mid))))
                    { 
                        T=mid;
                    }
                    else
                    { 
                        T=mid-1;
                    }
                }
                else
                {
                    if((*(mid+table)-volt)>(volt-(*(mid+1+table))))
                    {
                        T=mid+1;
                    }
                    else
                    {
                        T=mid;
                    }
                }
                resolution = 1;//just reuse this param
                break;
            }
            else if(volt<*(table+mid))
            {
                bot=mid+1;
            }                 
            else
            {
                top=mid-1;
            }
        }
        if(resolution == 0)
        {
            if (volt < *(table+bot))
            {
                T = bot;      
            }
            else
            {
                T = top;                  
            }
        }
    }
    
    resolution =  (INT16U)(NTCTAB_TEMP_MAX_WITH_OFST_40 - NTCTAB_TEMP_MIN_WITH_OFST_40) / (table_size[tbl_name*2 + 1] - 1);
    if(resolution > 2)
    {
        INT8U temp_x, temp_y;
        INT16U volt_x, volt_y;
        
        volt_x = T;//bms_get_word_range_index(NCP18XH103, NCP18XH103_MAX, (INT16U)vol); //��ȡ����
        volt_y = table[volt_x + 1];
        temp_x = volt_x * resolution;
        temp_y = (volt_x + 1) * resolution;
        volt_x = table[volt_x];

        T = (INT8U)math_linear_differential_fun((float)volt_x, (float)temp_x, (float)volt_y, (float)temp_y, (float)volt); //offset:-40
    }

    return T;
}

 

#endif