/**
 *
 * Copyright (c) 2012 Ligoo Inc.
 *
 * @file bms_eeeprom_impl.c
 * @brief
 * @note
 * @author
 * @date 2012-5-16
 *
 */
#include "bms_eeeprom_impl.h"

#pragma MESSAGE DISABLE C1420 // Result of function-call is ignored
#pragma MESSAGE DISABLE C4000
#pragma MESSAGE DISABLE C12056  // SP debug info incorrect because of optimization or inline assembler

#pragma DATA_SEG __RPAGE_SEG PAGED_RAM

INT16U g_eee_modify_max_count = 0;
INT16U g_eee_backup_array[EEEPROM_ACTUAL_PARAM_NUM];
INT8U g_eeeprom_error_status = 0;

#pragma DATA_SEG DEFAULT

#pragma DATA_SEG EEPROM_DATA
INT16U g_eee_data[EEEPROM_ACTUAL_PARAM_NUM];
INT16U g_eee_data_rev[EEEPROM_ACTUAL_PARAM_NUM];
INT16U g_eee_data_backup[EEEPROM_ACTUAL_PARAM_NUM];
INT16U g_eee_data_backup_rev[EEEPROM_ACTUAL_PARAM_NUM];
#pragma DATA_SEG DEFAULT
/*************************************************************************
 函数名称：EmulatedEepromInit
 功    能：EEEPROM功能初始化
 参    数：无
 返 回 值：无
 作    者：董丽伟
 创建时间：2011年10月12日
 修改时间：
 备    注：
 **************************************************************************/
Result eeeprom_init(INT8U format)
{
    volatile ErrType status;
    signed char dfPart, erPart; //Holds EEE partition size of data Flash and buffer RAM

    g_eeeprom_error_status = 0;
    g_eee_modify_max_count = (62 * (121 - BMS_EEEPROM_DFPART) - (EEEPROM_ACTUAL_PARAM_NUM << 1));

    /*********SET UP THE FLASH CLOCK DIVIDE REGISTER********/
    while (!FSTAT_CCIF); //Wait for FTM to be ready
    FCLKDIV = FCLK_DIV; //Now write FCLKDIV before using the FTM.
    if (FCLKDIV != (FCLK_DIV | 0x80)) //Ensure the value is written.
        eeeprom_report_error(FDIV_NOT_SET); //Report an error code.

    /***********(1)PERFORM FULL PARTITION OF EEE RAM***********/
   if (MODE == 0x00 && format)
   {
        //Run eeeprom_launch_flash_command to partition and erase the EEE in debug mode.
        //Return the status of the FSTAT and FERSTAT Flash registers.
        status = eeeprom_launch_flash_command(2, FULL_PARTITION_D_FLASH, 0, BMS_EEEPROM_DFPART, EEE_RAM, 0, 0, 0, 0, 0);

        //Always check if any error bits are set.
        eeeprom_error_check(status, accerr | fpviol | mgstat1 | mgstat0,
                erserif | pgmerif | epviolif | ersvif1 | ersvif0 | dfdif | sfdif);
   }
    /*****************(2)CHECK EEE PARTITIONING****************/
    //Use flash command function to query the EEEPROM partitioning.
    //Return the status of the FSTAT and FERSTAT Flash registers.
    status = eeeprom_launch_flash_command(0, EEPROM_QUERY, 0, 0, 0, 0, 0, 0, 0, 0); //Check the EEE status

    //Check if any error bits are set.
    eeeprom_error_check(status, (accerr | fpviol | mgstat1 | mgstat0),
            (erserif | pgmerif | epviolif | ersvif1 | ersvif0 | dfdif | sfdif));

    FCCOBIX = 1; //Set CCOB index to get Data flash partition result.
    dfPart = FCCOBLO; //Copy Data flash partition result.
    FCCOBIX++; //Set CCOB index to get EEE RAM partition result.
    erPart = FCCOBLO; //Copy EEE RAM partition result.

    // TODO: need to confirm the setting
    //if (g_sysParam[EEE_START_FLAG_INDEX] == 0)
    if (format)
    {
        //Use query results to determine if D-Flash has been partitioned
        //for EEEPROM with erPart == EEE_RAM and dfPart == BMS_EEEPROM_DFPART.
        if (EEE_RAM > 0)
        {
            if ((erPart != EEE_RAM) || (dfPart != BMS_EEEPROM_DFPART)) eeeprom_report_error(PARTITION_MISMATCH); //Full Partition was UNsuccessful
        }
        else if ((erPart != -1) || (dfPart != -1))
        {
            eeeprom_report_error(PARTITION_MISMATCH); //EEE_RAM might be out of range (0-16)
        }
    }
    /**************(3)ENABLE WRITES OF EEE RECORDS***************/
    //This enables the FTM to take any revised data written to the EEE
    //partitioned section(s) of the buffer RAM and update the record(s)
    //in the EEE partitioned section(s) of data Flash.
#ifdef ENABLE_EEE_RECORD_WRITING
    if (erPart > 0)
    {
        //Use flash command function to enable the FTM to manage EEE data and records.
        //Return the status of the FSTAT and FERSTAT Flash registers.
        status = eeeprom_launch_flash_command(0, ENABLE_EEPROM_EMULATION, 0, 0, 0, 0, 0, 0, 0, 0);

        //Check if any error bits are set.
        eeeprom_error_check(status, accerr | fpviol | mgstat1 | mgstat0,
                erserif | pgmerif | epviolif | ersvif1 | ersvif0 | dfdif | sfdif);
    }
#endif

    return (Result)g_eeeprom_error_status;
}

void eeeprom_uninit(void)
{
}

/** 按特定特定保存数据到eeprom */
Result eeeprom_save_int16u(INT16U index, INT16U data, INT8U flag)
{
    Result result = eeeprom_save_int16u_inside(index, data, flag);
    if (result != RES_OK) return result;

    if (flag & kEEEPROMLogTimesFlag)
        eeeprom_check_data_protect();

    return RES_OK;
}

Result eeeprom_save_int16u_inside(INT16U index, INT16U data, INT8U flag)
{
    INT8U cnt = 0;
    if (index >= EEEPROM_ACTUAL_PARAM_NUM) return ERR_EEPROM_INVALID_ADDR;

    eeeprom_wait_ftm_to_idle(); //等待内存控制器空闲
    if (g_eee_data[index] != data)
    {
        g_eee_data[index] = data;
        cnt++;
    }

    eeeprom_wait_ftm_to_idle(); //等待内存控制器空闲
    if (g_eee_data_rev[index] != ~data)
    {
        g_eee_data_rev[index] = ~data;
        cnt++;
    }

    eeeprom_wait_ftm_to_idle(); //等待内存控制器空闲
    if (g_eee_data_backup[index] != data)
    {
        g_eee_data_backup[index] = data;
        cnt++;
    }

    eeeprom_wait_ftm_to_idle(); //等待内存控制器空闲
    if (g_eee_data_backup_rev[index] != ~data)
    {
        g_eee_data_backup_rev[index] = ~data;
        cnt++;
    }
    if (index != EEEPROM_MODIFY_CNT_INDEX && (cnt != 0) && (flag == kEEEPROMLogTimesFlag))
    {
        eeeprom_wait_ftm_to_idle(); //等待内存控制器空闲
        g_eee_data[EEEPROM_MODIFY_CNT_INDEX] += (4 + cnt);

        eeeprom_wait_ftm_to_idle(); //等待内存控制器空闲
        g_eee_data_rev[EEEPROM_MODIFY_CNT_INDEX] -= (4 + cnt);

        eeeprom_wait_ftm_to_idle(); //等待内存控制器空闲
        g_eee_data_backup[EEEPROM_MODIFY_CNT_INDEX] += (4 + cnt);

        eeeprom_wait_ftm_to_idle(); //等待内存控制器空闲
        g_eee_data_backup_rev[EEEPROM_MODIFY_CNT_INDEX] -= (4 + cnt);

        eeeprom_wait_ftm_to_idle(); //等待内存控制器空闲
    }

    return RES_OK;
}

Result eeeprom_load_int16u(INT16U index, INT16U* _PAGED value)
{
    if (value == NULL) return ERR_INVALID_ARG;
    if (index >= EEEPROM_ACTUAL_PARAM_NUM) return ERR_EEPROM_INVALID_ADDR;

    if(g_eee_data[index] + g_eee_data_rev[index] == 0xFFFF)
    {
        *value = g_eee_data[index];
        return RES_OK;
    }

    if(g_eee_data_backup[index] + g_eee_data_backup_rev[index] == 0xFFFF)
    {
        *value = g_eee_data_backup[index];
        return RES_OK;
    }

    return ERR_EEPROM_DATA_FAIL;
}

Result eeeprom_repair_int16u_item(INT16U index, INT16U default_value)
{
    Result res, old_value;
    if (index >= EEEPROM_ACTUAL_PARAM_NUM) return ERR_EEPROM_INVALID_ADDR;

    res = eeeprom_load_int16u(index, &old_value);
    if (res != RES_OK)
    {
        /** repair the value with the default value if fail to load */
        return eeeprom_save_int16u(index, default_value, 1);
    }

    if (g_eee_data[index] + g_eee_data_rev[index] != 0xFFFF
            || g_eee_data_backup[index] + g_eee_data_backup_rev[index] != 0xFFFF)
    {
        /** repair the value if validating does not pass */
        return eeeprom_save_int16u(index, old_value, 1);
    }

    return RES_OK;
}

/*********************************************************************************
**函数名称：EEERAMDataProtect
**功    能：保护增强型EEPROM中存放在EEE_RAM中的数据不会因为DFLASH的周期擦除而丢失
**参    数：void
**返 回 值：void
**备    注：相当于在整个可用的DFLASH快要用完时对所有参数进行一次写操作，以便重新生成记录
*********************************************************************************/
void eeeprom_check_data_protect(void)
{
    INT8U index;
    if (g_eee_data[EEEPROM_MODIFY_CNT_INDEX] <= g_eee_modify_max_count) return;

    for (index = 0; index < BMS_EEEPROM_PARAM_MAX_NUM; index++)
    {
        g_eee_backup_array[index] = g_eee_data[index];
    }
    for (index = 0; index < BMS_EEEPROM_PARAM_MAX_NUM; index++)
    {
        /** 强行变换数据确保数据被真正写入 */
        if (g_eee_backup_array[index] != 0xFFFF)
            eeeprom_save_int16u_inside(index, 0xFFFF, 1);
        else
            eeeprom_save_int16u_inside(index, 0xFFFE, 1);
        eeeprom_save_int16u_inside(index, g_eee_backup_array[index], 1);
    }
    eeeprom_save_int16u_inside(EEEPROM_MODIFY_CNT_INDEX, 0, 0);
}

/**
 ************************************************************************
 函数名称：eeeprom_report_error
 功    能：EEEPROM错误报告
 参    数：unsigned char error_code：错误代号
 返 回 值：void
 作    者：董丽伟
 创建时间：2011年10月12日
 修改时间：
 备    注：
 *************************************************************************
 */
void eeeprom_report_error(unsigned char error_code)
{
#if BMS_SUPPORT_EVENT
    static EEEPROMErrorEvent event;
    event.error_code = error_code;
    event_dispatch(kEEEPROMErrorEvent, &event);
#endif
    g_eeeprom_error_status = error_code;
}

/************************************************************************
函数名称：eeeprom_error_check
功    能：EEEPROM错误检查
参    数：volatile ErrType status：EEEPROM状态
          volatile unsigned statbits：待检测的状态位
          volatile unsigned char ferstatbits：待检测的状态位
返 回 值：void
作    者：董丽伟
创建时间：2011年10月12日
修改时间：
备    注：
*************************************************************************
*/
void eeeprom_error_check(ErrType status, unsigned char statbits, unsigned char ferstatbits)
{
    //Compare the copied FSTAT and FERSTAT register results against the selected
    //error bits. A match indicates an error and loops forever.
    if((status.fstat_var & statbits) || (status.ferfstat_var & ferstatbits))
    {
        eeeprom_report_error(status.ferfstat_var);
        //g_bmsInfo.PackState |= (1<<EEPROM_ERR_BIT);
    }
}

/*
 ************************************************************************
 函数名称：eeeprom_launch_flash_command
 功    能：通过FTM来读、写或配置存储器区域，命令必须装载到FTM中执行。这个过程
 包括一个指令序列给FTM来确定命令参数及启动执行。
 参    数  :   params: ccob1 - ccob7要使用的参数个数.
 : ccob0high: 分区命令的高字节.
 : ccob0low: 分区命令的低字节.
 : ccob1: 第一个分区命令参数（可选）
 : ccob2: 第二个分区命令参数（可选）
 : ccob3: 第三个分区命令参数（可选）
 : ccob4: 第四个分区命令参数（可选）
 : ccob5: 第五个分区命令参数（可选）
 : ccob6: 第六个分区命令参数（可选）
 : ccob7: 第七个分区命令参数（可选）

 返 回 值：ErrType：命令执行结果错误标志
 作    者：董丽伟
 创建时间：2011年10月12日
 修改时间：
 备    注：
 *************************************************************************
 */
ErrType eeeprom_launch_flash_command(char params, unsigned char ccob0high, unsigned char ccob0low,
        unsigned int ccob1, unsigned int ccob2, unsigned int ccob3, unsigned int ccob4,
        unsigned int ccob5, unsigned int ccob6, unsigned int ccob7)
{
    unsigned char temp; //Temporary variable.
    volatile ErrType status; //Used to copy and store Flash status registers.

    temp = FCLKDIV; //Read the FCLKDIV register
    if ((temp & 0x80) != 0x80) //If FDIVLD not set, then write FCLKDIV.
    {
        FCLKDIV = FCLK_DIV; //Write FCLKDIV before launching FTM command.
        if (FCLKDIV != (FCLK_DIV | 0x80)) //Check to make sure value is written.
            eeeprom_report_error(0);
    }
    while (FSTAT_CCIF != 1)
        ; //Wait a while for CCIF to be set
    if (FSTAT_CCIF == 1) //Ensure CCIF set before using the FTM module.
    {
        FSTAT = 0x30; //Use store instruction to clear ACCERR, FPVIOL.
        /**********SET UP THE FTM COMMAND AND PARAMETERS***********/
        FCCOBIX = 0; //Set CCOB index to 0 to begin command setup.
        FCCOBHI = ccob0high; //Write ccob0 high-byte command value.
        FCCOBLO = ccob0low; //Write ccob0 low-byte parameter, if used.
        if (params > 0) //Continue if more parameters to specify.
        {
            FCCOBIX++;
            FCCOB = ccob1; //Write next word parameter to CCOB1 buffer.
            if (params > 1) //Continue if more parameters to specify.
            {
                FCCOBIX++;
                FCCOB = ccob2; //Write next word parameter to CCOB2 buffer.
                if (params > 2) //Continue if more parameters to specify.
                {
                    FCCOBIX++;
                    FCCOB = ccob3; //Write next word parameter to CCOB3 buffer.
                    if (params > 3) //Continue if more parameters to specify.
                    {
                        FCCOBIX++;
                        FCCOB = ccob4; //Write next word parameter to CCOB4 buffer.
                        if (params > 4) //Continue if more parameters to specify.
                        {
                            FCCOBIX++;
                            FCCOB = ccob5; //Write next word parameter to CCOB5 buffer.
                            if (params > 5) //Continue if more parameters to specify.
                            {
                                FCCOBIX++;
                                FCCOB = ccob6; //Write next word parameter to CCOB6 buffer.
                                if (params > 6) //Continue if more parameters to specify.
                                {
                                    FCCOBIX++;
                                    FCCOB = ccob7; //Write next word parameter to CCOB7 buffer.
                                }
                            }
                        }
                    }
                }
            }
        }
        /**********************************************************/
        FSTAT = 0x80; //Clear buffer-empty-flag to start FTM command.
        while (!FSTAT_CCIF); //Now wait for the FTM command to complete.
        status.fstat_var = FSTAT; //Copy FSTAT register state for later comparison.
        status.ferfstat_var = FERSTAT; //Copy FERSTAT register state for later comparison.
        return (status); //After FTM command completed, return status.
    }
    else
    {
        eeeprom_report_error(COMMAND_BUSY); //FTM is currently busy.
        status.fstat_var = 1;
        status.ferfstat_var = 0;
        return (status);
    }
}

INT8U eeeprom_wait_ftm_to_idle(void)
{
    INT16U Cnt = 5000;

    while ((Cnt-- != 0) && ((FSTAT_MGBUSY != 0) || (FSTAT_CCIF == 0)||(ETAG > 0))); //等待内存控制器空闲
    if (Cnt == 0)
        return COMMAND_BUSY;
    else
        return COMMAND_IDLE;
}

