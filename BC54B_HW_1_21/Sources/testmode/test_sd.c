#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "stdint.h"
#include "diskio.h"
#include "ff.h"
#include "bms_util.h"
#include "bms_bcu.h"

#pragma MESSAGE DISABLE C4001 // Condition always FALSE

#define memcpy safe_memcpy
#define memset safe_memset

#if BMS_SUPPORT_DATA_SAVE
extern FATFS g_fat_fs;
 
    
void test_sd_init(void) 
{
    FRESULT ret = FR_OK;
    
    if(hardware_io_revision_get() < HW_VER_116)
    {
        sdcard_init();
        (void)disk_initialize(0);
        ret = f_mount(&g_fat_fs, "0", 0);
        if(ret != FR_OK)
        {
            (void)printf("f_mount fail\n");
        }
    }
    else
    {
        (void)printf("NO sdcard support!\n");
    }
}

struct sdcard_info {
    unsigned short sector_size;
    unsigned long sector_count;
    unsigned long block_size;
    unsigned char card_type;
};

static int dump_sd_info(void) {
    struct sdcard_info info;

    if (DRES_OK != disk_ioctl(0, GET_SECTOR_SIZE, &info.sector_size)) {
        return -1;
    }

    if (DRES_OK != disk_ioctl(0, GET_SECTOR_COUNT, &info.sector_count)) {
        return -2;
    }
    if (DRES_OK != disk_ioctl(0, GET_BLOCK_SIZE, &info.block_size)) {
        return -3;
    }
    if (DRES_OK != disk_ioctl(0, MMC_GET_TYPE, &info.card_type)) {
        return -4;
    }

    (void)printf("card_type:%d\n", info.card_type);
    (void)printf("sector_size:%d\n", info.sector_size);
    (void)printf("sector_count:%.0f\n", (float)info.sector_count);
    (void)printf("block_size:%.0f\n", (float)info.block_size);

    return 0;
}

static int dump_sd_sector(uint32_t sector) {
    uint16_t i;
    uint16_t sector_size;
    unsigned char *sector_buffer = g_fat_fs.win;

    if (DRES_OK != disk_ioctl(0, GET_SECTOR_SIZE, &sector_size)) {
        return -1;
    }

    if (DRES_OK != disk_read(0, sector_buffer, sector, 1)) {
        return -2;
    }

    for (i = 0; i < sector_size - 1; ++i) {
        (void)printf("%02X ", *sector_buffer++);
    }
    (void)printf("%02X\n", *sector_buffer);
    return 0;
}


static int fill_sd_sector(uint32_t sector, uint8_t dat) {
    uint16_t sector_size;
    unsigned char *sector_buffer = g_fat_fs.win;

    if (DRES_OK != disk_ioctl(0, GET_SECTOR_SIZE, &sector_size)) {
        return -1;
    }

    (void)memset(sector_buffer, dat, sector_size);

    if (DRES_OK != disk_write(0, sector_buffer, sector, 1)) {
        return -2;
    }

    return 0;
}

static int fs_touch(char *filename)
{
    FRESULT ret;
    FIL file = {0};
           
    ret = f_open(&file, filename, FA_WRITE | FA_READ | FA_CREATE_NEW);
    if(ret == FR_OK)
    {
        ret = f_close(&file);
    }
    else
    {
        (void)printf("create %s fail ret = %d \n", filename,  ret);
    } 
    
    return ret;
}

static int fs_ls(const char *path) {
    FRESULT ret = 0;
    int cnt = 0;
    DIR dir = {0};
    char buffer[64] = {0};
    
    do {
        FILINFO fileinfo = {0};
        fileinfo.lfname = buffer;
        fileinfo.lfsize = sizeof(buffer);
         
        ret = f_opendir(&dir, path);
        if (FR_OK != ret) break;
        
        for(;;) 
        {
            ret = f_readdir(&dir, &fileinfo);
            if ((FR_OK != ret) || (fileinfo.fname[0] == 0)) 
            {
                break;
            }
            (void)printf("%s\t\t %s\n", fileinfo.lfname, fileinfo.fname); 
        }
        (void)f_closedir(&dir);
    } while (0);
    return ret;
}

static int fs_mkdir (const char* path)        /* Pointer to the directory path */
{
    FRESULT ret;

    ret = f_mkdir(path);    
    if (ret != FR_OK)
    {
        (void)printf("create folder %s fail, ret %d\n", path, ret);
    }
    return ret;
}

static int fs_cddir (const char* path)        /* Pointer to the directory path */
{
    FRESULT ret;

    ret = f_chdir(path);    
    if (ret != FR_OK)
    {
        (void)printf("open folder %s fail, ret %d\n", path, ret);
    }
    return ret;
}

static int fs_rmfile (const char* path)        /* Pointer to the directory path */
{
    FRESULT     ret;

    ret = f_unlink(path);
    if(FR_OK != ret)
    {
        (void)printf("remove %s fail, ret %d \n", path, ret);
    }
    
    return ret;
}

static int dump_sd_register(uint8_t ioctrl_code, uint8_t len) {
    uint8_t buf[64];
    uint8_t i;
    uint8_t b;
    if (len > sizeof(buf)) {
        return -1;
    }
    
    if (DRES_OK != disk_ioctl(0, ioctrl_code, buf)) {
        return -1;
    }
    
    for (i=0; i<len; ++i){
        (void)printf("[%d:%d]", (len - i) * 8 - 1, (len - 1 - i) * 8);
        for(b = 0x80; b != 0x00; b = b >> 1) {
            (void)printf(buf[i] & b ? "1":"0");
        }
        (void)printf("\n");
    }
    
    return 0;
}
#endif

const char shell_help_sdcard[] = "\
	ops [args ...]\n\
	ops: info 读取SD卡的相关信息;\n\
	ops: read xxx 读取xxx(十进制) sector的数据;\n\
	ops: fill xxx YY 用YY(十进制)填充Sector xxx(十进制);\n\
    touch/mkdir/cd/ls/rm  文件操作相关命令\n\
";

const char shell_summary_sdcard[] =
    "SD卡相关";


extern void print_arg_err_hint(char *arg);
int shell_func_sdcard(int argc, char **argv) {
#if BMS_SUPPORT_DATA_SAVE
    if (argc == 2) {
        if (strcmp(argv[1], "info") == 0) {
            return dump_sd_info();
        }
        if (strcmp(argv[1], "cid") == 0) {
            return dump_sd_register(MMC_GET_CID, 16);
        }
        
        if (strcmp(argv[1], "csd") == 0) {
            return dump_sd_register(MMC_GET_CSD, 16);
        }
        
        if (strcmp(argv[1], "scr") == 0) {            
            return dump_sd_register(MMC_GET_SCR, 8);
        }
        
        if (strcmp(argv[1], "sds") == 0) {            
            return dump_sd_register(MMC_GET_SDSTAT, 64);
        }
        if(strcmp(argv[1], "ls") == 0)
        {
            return fs_ls(".");
        }
        
    } else if (argc == 3) {
        if (strcmp(argv[1], "read") == 0) {
            return dump_sd_sector(atoi(argv[2]));
        }else if(strcmp(argv[1], "touch") == 0)
        {
            return fs_touch(argv[2]);
        }else if(strcmp(argv[1], "mkdir") == 0)
        {
            return fs_mkdir(argv[2]);
        }else if(strcmp(argv[1], "cd") == 0)
        {
            return fs_cddir(argv[2]);
        }else if(strcmp(argv[1], "ls") == 0)
        {
            return fs_ls(argv[2]);
        }else if(strcmp(argv[1], "rm") == 0)
        {
            return fs_rmfile(argv[2]);
        }
    } else if (argc == 4) {
        if (strcmp(argv[1], "fill") == 0) {
            return fill_sd_sector(atoi(argv[2]), (uint8_t)atoi(argv[3]));
        }
    }  
    print_arg_err_hint(argv[0]);
#else
    (void) argc;
    (void) argv;
#endif  
    return -1;
}
