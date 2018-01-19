 /*******************************************************************************
 **                       安徽新能源技术有限公司 Copyright (c)
 **                            http://www...
 **文 件 名:ltc_util.c
 **作    者:曹志勇
 **创建日期:2016.12.15
 **文件说明:
 **修改记录:
 **版    本:V1.0
 **备    注:
 *******************************************************************************/
#ifndef  _LTC_UTIL_C_
#define  _LTC_UTIL_C_

#include "ltc_util.h"
#include "ltc68041_impl.h" 
//#include "SPI1.h"

/*******************PEC15校验表*********************************/
#define LTC6804_NTC_TABLE_LEN      166
#pragma push
#pragma DATA_SEG __PPAGE_SEG CONST_TABLES
const INT16U  pec15Table[256] =
{ 
    0x0000, 0xc599, 0xceab, 0x0b32, 0xd8cf, 0x1d56, 0x1664, 0xd3fd, 0xf407, 0x319e, 0x3aac, 0xff35, 0x2cc8, 0xe951, 0xe263, 0x27fa, 0xad97, 0x680e, 0x633c,
    0xa6a5, 0x7558, 0xb0c1, 0xbbf3, 0x7e6a, 0x5990, 0x9c09, 0x973b, 0x52a2, 0x815f, 0x44c6, 0x4ff4, 0x8a6d, 0x5b2e, 0x9eb7, 0x9585, 0x501c, 0x83e1, 0x4678,
    0x4d4a, 0x88d3, 0xaf29, 0x6ab0, 0x6182, 0xa41b, 0x77e6, 0xb27f, 0xb94d, 0x7cd4, 0xf6b9, 0x3320, 0x3812, 0xfd8b, 0x2e76, 0xebef, 0xe0dd, 0x2544, 0x02be,
    0xc727, 0xcc15, 0x098c, 0xda71, 0x1fe8, 0x14da, 0xd143, 0xf3c5, 0x365c, 0x3d6e, 0xf8f7, 0x2b0a, 0xee93, 0xe5a1, 0x2038, 0x07c2, 0xc25b, 0xc969, 0x0cf0,
    0xdf0d, 0x1a94, 0x11a6, 0xd43f, 0x5e52, 0x9bcb, 0x90f9, 0x5560, 0x869d, 0x4304, 0x4836, 0x8daf, 0xaa55, 0x6fcc, 0x64fe, 0xa167, 0x729a, 0xb703, 0xbc31,
    0x79a8, 0xa8eb, 0x6d72, 0x6640, 0xa3d9, 0x7024, 0xb5bd, 0xbe8f, 0x7b16, 0x5cec, 0x9975, 0x9247, 0x57de, 0x8423, 0x41ba, 0x4a88, 0x8f11, 0x057c, 0xc0e5,
    0xcbd7, 0x0e4e, 0xddb3, 0x182a, 0x1318, 0xd681, 0xf17b, 0x34e2, 0x3fd0, 0xfa49, 0x29b4, 0xec2d, 0xe71f, 0x2286, 0xa213, 0x678a, 0x6cb8, 0xa921, 0x7adc,
    0xbf45, 0xb477, 0x71ee, 0x5614, 0x938d, 0x98bf, 0x5d26, 0x8edb, 0x4b42, 0x4070, 0x85e9, 0x0f84, 0xca1d, 0xc12f, 0x04b6, 0xd74b, 0x12d2, 0x19e0, 0xdc79,
    0xfb83, 0x3e1a, 0x3528, 0xf0b1, 0x234c, 0xe6d5, 0xede7, 0x287e, 0xf93d, 0x3ca4, 0x3796, 0xf20f, 0x21f2, 0xe46b, 0xef59, 0x2ac0, 0x0d3a, 0xc8a3, 0xc391,
    0x0608, 0xd5f5, 0x106c, 0x1b5e, 0xdec7, 0x54aa, 0x9133, 0x9a01, 0x5f98, 0x8c65, 0x49fc, 0x42ce, 0x8757, 0xa0ad, 0x6534, 0x6e06, 0xab9f, 0x7862, 0xbdfb,
    0xb6c9, 0x7350, 0x51d6, 0x944f, 0x9f7d, 0x5ae4, 0x8919, 0x4c80, 0x47b2, 0x822b, 0xa5d1, 0x6048, 0x6b7a, 0xaee3, 0x7d1e, 0xb887, 0xb3b5, 0x762c, 0xfc41,
    0x39d8, 0x32ea, 0xf773, 0x248e, 0xe117, 0xea25, 0x2fbc, 0x0846, 0xcddf, 0xc6ed, 0x0374, 0xd089, 0x1510, 0x1e22, 0xdbbb, 0x0af8, 0xcf61, 0xc453, 0x01ca,
    0xd237, 0x17ae, 0x1c9c, 0xd905, 0xfeff, 0x3b66, 0x3054, 0xf5cd, 0x2630, 0xe3a9, 0xe89b, 0x2d02, 0xa76f, 0x62f6, 0x69c4, 0xac5d, 0x7fa0, 0xba39, 0xb10b,
    0x7492, 0x5368, 0x96f1, 0x9dc3, 0x585a, 0x8ba7, 0x4e3e, 0x450c, 0x8095 
};
/*南京时恒NTC阻值表(是阻值表)25℃10K  本表阻值放大100倍，第一个是-40摄氏度的电阻值*/
const INT16U kLTC6804NTCTab[LTC6804_NTC_TABLE_LEN]= 
{
    
    41000 ,32261 ,30037 ,27996 ,26118 ,24389 ,22795 ,21323 ,19962 ,18703 ,
    17536 ,16454 ,15449 ,14515 ,13646 ,12837 ,12083 ,11380 ,10723 ,10110 ,
    9537 ,9000 ,8498 ,8028 ,7587 ,7173 ,6785 ,6420 ,6078 ,5755 ,
    5453 ,5168 ,4899 ,4646 ,4408 ,4184 ,3972 ,3772 ,3583 ,3405 ,
    3237 ,3078 ,2928 ,2786 ,2651 ,2524 ,2404 ,2290 ,2182 ,2079 ,
    1982 ,1890 ,1803 ,1720 ,1642 ,1567 ,1496 ,1429 ,1365 ,1305 ,
    1247 ,1192 ,1140 ,1091 ,1044 ,1000 ,956 ,915 ,877 ,840 ,
    805 ,771 ,739 ,709 ,680 ,652 ,626 ,600 ,576 ,553 ,
    531 ,510 ,490 ,471 ,453 ,435 ,419 ,403 ,387 ,373 ,
    359 ,345 ,333 ,320 ,308 ,297 ,286 ,276 ,266 ,257 ,
    247 ,239 ,230 ,222 ,215 ,207 ,200 ,193 ,187 ,180 ,
    174 ,168 ,163 ,157 ,152 ,147 ,142 ,138 ,133 ,129 ,
    125 ,121 ,117 ,113 ,110 ,107 ,103 ,100 ,97 ,94 ,
    91 ,88 ,86 ,83 ,81 ,78 ,76 ,74 ,72 ,70 ,
    68 ,66 ,64 ,62 ,60 ,59 ,57 ,55 ,54 ,52 ,
    51 ,50 ,48 ,47 ,46 ,45 ,43 ,42 ,41 ,40 ,
    39 ,38 ,37 ,36 ,35 ,32
};
#pragma pop
/*****************************************************************************
 *函数名称:pec15_calc
 *函数功能:PEC校验计算
 *参    数:len 数据长度  data 数据内容
 *返 回 值:PEC校验值
 *修订信息:
 ******************************************************************************/
INT16U pec15_calc(INT8U len, const INT8U *far data)
{
	INT16U remainder = 0, addr = 0;
	INT8U i = 0;
	const INT16U *far table = pec15Table;

	if ((len == 0) || (data == NULL))
	{
		return 0;
	}

	remainder = 16;
	for (i = 0; i < len; i++)
	{
		addr = ((remainder >> 7) ^ data[i]) & 0xff;
		remainder = (remainder << 8) ^ table[addr];
	}
	return (remainder * 2);
}
/*****************************************************************************
 *函数名称:crc4_calc
 *函数功能:CRC4校验计算
 *参    数:data的低4为必须为0
 *返 回 值:CRC4校验值
 *修订信息:
 ******************************************************************************/
INT8U crc4_calc(INT16U data)
{
	const INT8U crc_base = 0x13;
	INT8U crc = 0;
	INT8U i = 0, index = 0;
	//低4位清零
	data = data & 0xFFF0;
	//第一次先取最高4位
	crc = (INT8U) ((data >> 12) & 0x000F);
	//二元模数除法
	for (i = 0; i < 12; i++)
	{
		index = 11 - i;
		//补一位
		crc = crc << 1;
		if (GET(data, index))
		{
			SET(crc, 0);
		}
		//异或
		if (GET(crc, 4))
		{
			crc = crc ^ crc_base;
		}
		//取低4位
		crc = crc & 0x0F;
	}
	return crc;
}
/*****************************************************************************
 *函数名称:ltc6804_buffer_set
 *函数功能:bsu数组设置
 *参    数:dest目标数组,value设置值,size数组大小
 *返 回 值:无
 *修订信息:
 ******************************************************************************/
void ltc6804_buffer_set(void*far dest, INT8U value, INT16U size)
{
	INT8U*far data = (INT8U*far) dest;
	INT16U index;
	if((dest == NULL)||(size == 0))
		return;
	for (index = 0; index < size; ++index)
		data[index] = value;
}
/*****************************************************************************
 *函数名称:ltc6804_cs_pin_low
 *函数功能:置片选信号为低电平
 *参    数:无     
 *返 回 值:无
 *修订信息:
 ******************************************************************************/
void ltc6804_cs_pin_low(void)
{
	(void) spi_select(&ltc6804_high_speed_spi_bus, 0, 1);
}
/*****************************************************************************
 *函数名称:ltc6804_cs_pin_high
 *函数功能:置片选信号为高电平
 *参    数:无       
 *返 回 值:无
 *修订信息:
 ******************************************************************************/
void ltc6804_cs_pin_high(void)
{
	(void) spi_select(&ltc6804_high_speed_spi_bus, 0, 0);
}
/*****************************************************************************
 *函数名称:wakeup_iso_spi_idle
 *函数功能:将isoSPI总线唤醒 ,注意每次时间必须小于5ms;次数和6804-1芯片数有关,这里为5次可支持30个芯片
 *参    数:无
 *返 回 值:无
 *作    者:无
 *修订信息:
 ******************************************************************************/
void wakeup_iso_spi_idle(void)
{
	INT8U i = 0;
	INT8U data = 0;
	for (i = 0; i < 3; i++)
	{
		ltc6804_cs_pin_low();
		data = 0xFF;
		(void) spi_transmit_byte(&ltc6804_high_speed_spi_bus, &data);
		OSTimeDly(3);
		ltc6804_cs_pin_high();
	}
}

/*****************************************************************************
 *函数名称:wakeup_ltc6804_sleep
 *函数功能:将IC唤醒,上电的时候,或通信中断需要调用,时间与6804-1个数有关
 *         本函数使用了OSTimeDly函数,需要在任务中调用,不要在定时中断或初始化中调用
 *         这里为12ms可支持30个芯片
 *参    数:无
 *返 回 值:无
 *修订信息:
 ******************************************************************************/
void wakeup_ltc6804_sleep(void)
{
	ltc6804_cs_pin_low();
	OSTimeDly(15);
	ltc6804_cs_pin_high();
}
/*******************************************************************
*函数名称：ltc6804_temp_query
*函数功能：温度查表,根据阻值查表得出温度
*入口参数：INT16U R电阻值, const INT16U *table表, INT8U n 表长度-1
*出口参数：无
*返 回 值：温度值(分辨率1℃/bit,偏移量-40℃)
*  备  注：0xFF温感或排线短路，0xFE 温感或排线开路，其他为正常温度
*******************************************************************/
INT8U ltc6804_temp_query(INT16U R)
{
	INT8U top, bot;
	INT8U mid;
	static INT8U T;
	const INT16U *far table = kLTC6804NTCTab;
	INT8U n = LTC6804_NTC_TABLE_LEN-1;
	if ((R <= *table) && (R >= *(table + n)))
	{
		bot = 0;
		top = n;
		for (; bot < top;)
		{
			mid = (INT8U) ((top + bot) >> 1);//mid?n/2
			if (R <= (*(table + mid - 1)) && R >= (*(table + mid + 1)))
			{
				if (R >= *(table + mid))
				{
					if (((*(table + mid - 1)) - R) > (R - (*(table + mid))))
					{
						T = mid;
					}
					else
					{
						T = mid - 1;
					}
				}
				else
				{
					if ((*(mid + table) - R) > (R - (*(mid + 1 + table))))
					{
						T = mid + 1;
					}
					else
					{
						T = mid;
					}
				}
				return T;
			}
			else if (R < *(table + mid))
			{
				bot = mid + 1;
			}
			else
			{
				top = mid - 1;
			}
		}

		if (R < *(table + bot))
		{
			T = bot;
		}
		else
		{
			T = top;
		}
	}
	else if (R > *table)
	{
		T = TEMP_CABLE_OPEN_VAULE;
	}
	else
	{
		T = TEMP_CABLE_SHORT_VAULE;
	}
	return T;
}

#endif


