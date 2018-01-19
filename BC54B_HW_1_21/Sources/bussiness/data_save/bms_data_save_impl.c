/**
*
* Copyright (c) 2012 Ligoo Inc.
*
* @file bms_data_save_impl.c
* @brief
* @note ��������RPAGE�����˷�ҳ�ڴ����⣬�ڵ�����������ʱ��
*       һ��Ҫȷ�ϱ����ú���û��ʹ�÷�ҳ�ڴ������RPAGE����
*       ���ڱ����ú���ʹ���˷�ҳ�ڴ���û��RPAGE�����ĺ�������Ҫ�ڵ��øú�����ָ�RPAGE��ֵ
* @author
* @date 2012-5-29
*
*/
#include "bms_data_save_impl.h"

#if defined(BMS_SUPPORT_DATA_SAVE) && BMS_SUPPORT_DATA_SAVE

#pragma MESSAGE DISABLE C2705 // Possible loss of data
#pragma MESSAGE DISABLE C5703 // Parameter is not used
#pragma MESSAGE DISABLE C1860 // Pointer conversion: possible loss of data
#pragma MESSAGE DISABLE C12056 // SP debug info incorrect because of optimization or inline assembler

OS_STK   g_data_save_task_save_data_stack[DATA_SAVE_STK_SIZE] = {0};
INT8U g_data_save_rpage = 0;
//extern const struct DataSaveHandler data_save_handler_sdcard;
extern const struct DataSaveHandler data_save_handler_nandflash;
static struct DataSaveHandler const *__FAR data_save_handler = &data_save_handler_nandflash;//&data_save_handler_sdcard;

#pragma push
#pragma DATA_SEG __RPAGE_SEG PAGED_RAM_SAVE //PAGED_RAM_JOB
uint8_t g_data_save_buffer[DATA_SAVE_MAX_LINE_SIZE] = {0};
#pragma pop



void data_save_init(void) {
#if BMS_SUPPORT_BY5248D == 0     //һ���ģʽ��֧��114
#if BMS_SUPPORT_HARDWARE_BEFORE == 1   // yang 20161213
    if (hardware_io_revision_get() >= HW_VER_116) { //V1.16 & V1.20
        data_save_handler = &data_save_handler_nandflash;
    } else {
        data_save_handler = &data_save_handler_sdcard;
    }
#else
    data_save_handler = &data_save_handler_nandflash;
#endif
#else
    data_save_handler = &data_save_handler_nandflash;
#endif

    data_save_handler->init();
    OSTaskCreate(data_save_handler->task, (void *) NULL, //
                 (OS_STK *) &g_data_save_task_save_data_stack[DATA_SAVE_STK_SIZE - 1], //
                 DATA_SAVE_TASK_PRIO);
}
void data_save_uninit(void)
{
    data_save_handler->uninit();
}

void data_save_start(void)
{
    data_save_handler->start();
}

void data_save_stop(void)
{
    data_save_handler->stop();
}

INT8U data_save_is_stop(void)
{
    return data_save_handler->is_stop();
}

DataSaveStatus data_save_get_status(void)
{
    return data_save_handler->get_status();
}

INT32U data_save_get_counter(void) {
    return data_save_handler->get_counter();
}

INT8U data_save_is_sd_fault(void)
{
    return data_save_handler->is_fault();
}

void data_save_set_sd_write_state(SDWriteState state)
{
}

SDWriteState data_save_get_sd_write_state(void)
{
    return kSDWriteDisable;
}

#endif
