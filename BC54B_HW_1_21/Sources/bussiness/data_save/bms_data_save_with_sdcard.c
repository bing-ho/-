
#include "bms_data_save_impl.h"

#pragma MESSAGE DISABLE C1825 // Indirection to different types
#pragma MESSAGE DISABLE C1860 // Pointer conversion: possible loss of data

#define DATA_SAVE_MIN_DISK_SECTORS      256 //1M 保存最小扇区数
#define DATA_SAVE_REL_MIN_DISK_SECTORS  1280 //5M 释放最小扇区数
#define MAX_BATTERY_PER_CLUSTER_COUNT   16

#define TF_POWER_ON_INTREVAL    1000
#define TF_POWER_OFF_INTERVAL   2000  //检测到USB插入和拔出后，暂时断开卡供电的时间
#define TF_POWER_ON_DELAY       1000 //卡重新上电后等待稳定时间
#define TF_CHIP_CONNECT_DELAY   2000

#define DATA_SAVE_FILE_NAME_MAX 50

#define TF_RE_CONNECT()         TF_POWER_ON();\
                                sleep(TF_POWER_ON_DELAY);\
                                TF_CHIP_CONNECT();\
                                sleep(TF_CHIP_CONNECT_DELAY);

#define TF_DISCONNECT()         TF_CHIP_DISCONNECT();\
                                sleep(TF_POWER_ON_INTREVAL);\
                                TF_POWER_OFF();\
                                sleep(TF_POWER_OFF_INTERVAL);

#define STRING_BMU "BMU "
static void data_save_task_run_check_sdcard(void* pdata);

static FRESULT data_save_check_disk(void);
static FRESULT data_save_remove_oldest_file(void);

/**
 * 文件编码为A时，存储A类参数，且只在BMS第一次运行时（不包括测试运行）保存
 * @return
 */
static Result data_save_save_a_file(void);
/**
 * 文件编码为B时，存储B类参数（状态参数），当某一状态改变时就存储一次，不改变则不存储
 * @return
 */
static Result data_save_save_b_file(void);
/**
 * 文件编码为C时，存储C类参数（规格参数），配置一次则存储一次
 * @return
 */
static Result data_save_save_c_file(void);
/**
 * 文件编码为D时，存储D类参数（实时信息参数），以固定周期（20s）存储；但无论文件编码是多少，实时的时间、日期（公共类）都应存储
 * @return
 */
static Result data_save_save_d_file(void);

static char* data_save_get_filename(char* suffix);

static Result data_save_on_status_changed(EventTypeId event_id, void* event_data, void* user_data);
static Result data_save_on_config_changing(ConfigIndex index, INT16U new_value);

static void data_save_buffer_write_eof(void);
static void data_save_buffer_write_cr(void);
static void data_save_buffer_write_switch(INT8U value);
static void data_save_buffer_write_string(const char* value);
static void data_save_buffer_write_float(FP64 value, INT8U fractional_cnt);
static void data_save_buffer_write_int(INT32U value);
static void data_save_buffer_write_date(DateTime* value);
static void data_save_buffer_write_time(DateTime* value);
static void data_save_buffer_write_date(DateTime* value);
static void data_save_buffer_write(const char* text);
static void data_save_buffer_flush(void);
static FRESULT data_save_set_operation_exception(FRESULT result);

#define F_OPENDIR(dp, path)                 data_save_set_operation_exception(f_opendir(dp, path))
#define F_CLOSEDIR(dp)                      data_save_set_operation_exception(f_closedir(dp))
#define F_READDIR(dp, fno)                  data_save_set_operation_exception(f_readdir(dp, fno))
#define F_MKDIR(path)                       data_save_set_operation_exception(f_mkdir(path))
#define F_MOUNT(fs, path, opt)              data_save_set_operation_exception(f_mount(fs, path, opt))
#define F_WRITE(fp,    buff, btw, bw)          data_save_set_operation_exception(f_write(fp,    buff, btw, bw))
#define F_SYNC(fp)                          data_save_set_operation_exception(f_sync(fp))
#define F_UNLINK(path)                      data_save_set_operation_exception(f_unlink(path))
#define F_LSEEK(file, fsize)                data_save_set_operation_exception(f_lseek(file, fsize))
#define F_OPEN(fp, path, mode)              data_save_set_operation_exception(f_open(fp, path, mode))
#define F_CLOSE(fp)                         data_save_set_operation_exception(f_close(fp))
#define F_READ(file, buf, num, br)          data_save_set_operation_exception(f_read(file, buf, num, br))
#define F_GETFREE(path, nclst, fatfs)       data_save_set_operation_exception(f_getfree(path, nclst, fatfs))
#define F_GETFREEPROOF(path, nclst, fatfs)  data_save_set_operation_exception(f_getfreeproof(path, nclst, fatfs))


#pragma push
#pragma DATA_SEG __RPAGE_SEG PAGED_RAM_SAVE

/**
 * 外部有使用,不能改为static类型
 */
FATFS g_fat_fs;
static FIL  g_fo;
static TCHAR g_fat_path[5] = {0};

static INT8U g_data_save_request_stopped = 0;
static INT8U g_data_save_status = kDataSaveStatusNotAvaliable;
static INT8U g_data_save_last_sdcard_status = 0;

const char g_old_root_dir[] = {"BMS"};
static char g_root_dir[25] = {0}; //VRU信息所保存的一级文件夹名称 注意：不能为空

static DateTime g_data_save_time = {0};
static INT32U g_data_save_d_file_last_tick = 0;

static INT8U g_data_save_status_is_changed = 0;
static INT8U g_data_save_config_is_changed = 0;

static INT32U g_data_save_counter = 0;


static int  g_data_save_buffer_pos = 0;
static char g_data_save_bcd[30] = {0};
static TCHAR g_long_file_name_buffer[DATA_SAVE_FILE_NAME_MAX];
static TCHAR g_delete_long_file_name[DATA_SAVE_FILE_NAME_MAX];

static FRESULT g_data_save_operation_exception = FR_OK;
static INT32U g_data_save_operation_exception_tick = 0;
static INT8U g_data_save_file_d_life = 0;

#pragma pop


FRESULT f_getfreeproof(
const TCHAR *path, /* Pointer to the logical drive number (root dir) */
DWORD *nclst, /* Pointer to the variable to return number of free clusters */
FATFS **fatfs /* Pointer to pointer to corresponding file system object to return */
);

static FRESULT data_save_file_write(
    FIL* fp,            /* Pointer to the file object */
    const void *buff,    /* Pointer to the data to be written */
    UINT btw,            /* Number of bytes to write */
    UINT* bw            /* Pointer to number of bytes written */
)
{
    FRESULT rc;
    DWORD old_size;
    DWORD free_clustuters;
    FATFS *fs;
    
    if(fp == NULL || buff == NULL || btw == 0 || bw == NULL) return FR_INVALID_PARAMETER;
    
    old_size = fp->fsize;
    rc = F_WRITE(fp, buff, btw, bw);
    if(rc == FR_OK)
    {
        rc = F_SYNC(fp);
    }
    if(rc != FR_OK || fp->fsize != old_size + btw)
    {
        rc = F_GETFREEPROOF(g_fat_path, &free_clustuters, &fs); //获取磁盘卷的信息和空闲簇大小
        data_save_check_disk(); //重新检测硬件状态
    }
    return rc;
}

static void data_save_buffer_flush(void)
{
    unsigned int bw;
    
    if (g_data_save_buffer_pos == 0) return;

    data_save_file_write(&g_fo, g_data_save_buffer, g_data_save_buffer_pos, &bw);
        
    g_data_save_buffer_pos = 0;
    g_data_save_buffer[0] = '\0';
}

static void data_save_buffer_write(const char* text)
{
    INT16U len;
    unsigned int bw;

    if (text == NULL) return;

    len = strlen(text);
    if (len >= DATA_SAVE_MAX_LINE_SIZE)
    {
        data_save_buffer_flush();
        
        data_save_file_write(&g_fo, text, len, &bw);
        g_data_save_buffer_pos = 0;
        g_data_save_buffer[0] = '\0';
        return;
    }

    if (len + g_data_save_buffer_pos > DATA_SAVE_MAX_LINE_SIZE)
    {
        data_save_buffer_flush();
    }

    memcpy(g_data_save_buffer + g_data_save_buffer_pos, text, len);
    g_data_save_buffer_pos += len;
}

static void data_save_buffer_write_time(DateTime* value)
{
    char_to_bcd_string(value->hour, g_data_save_bcd);
    data_save_buffer_write(g_data_save_bcd);
    data_save_buffer_write(":");
    char_to_bcd_string(value->minute, g_data_save_bcd);
    data_save_buffer_write(g_data_save_bcd);
    data_save_buffer_write(":");
    char_to_bcd_string(value->second, g_data_save_bcd);
    data_save_buffer_write(g_data_save_bcd);
    data_save_buffer_write(",");
}

static void data_save_buffer_write_date(DateTime* value)
{
    data_save_buffer_write("20");
    char_to_bcd_string(value->year, g_data_save_bcd);
    data_save_buffer_write(g_data_save_bcd);
    data_save_buffer_write("-");
    char_to_bcd_string(value->month, g_data_save_bcd);
    data_save_buffer_write(g_data_save_bcd);
    data_save_buffer_write("-");
    char_to_bcd_string(value->day, g_data_save_bcd);
    data_save_buffer_write(g_data_save_bcd);
    data_save_buffer_write(",");
}

static void data_save_buffer_write_int(INT32U value)
{
    int_to_bcd_string(value, g_data_save_bcd);
    data_save_buffer_write(g_data_save_bcd);
    data_save_buffer_write(",");
}

static void data_save_buffer_write_alarm(INT32U value)
{
    switch(value)
    {
#if BMS_DATASAVE_SUPPORT_ENGLISH
        case 0: data_save_buffer_write("None,"); break;
        case 1: data_save_buffer_write("1st alarm,"); break;
        case 2: data_save_buffer_write("1st alarm release,"); break;
        case 3: data_save_buffer_write("2nd alarm,"); break;
        case 4: data_save_buffer_write("2nd alarm release,"); break;
        case 5: data_save_buffer_write("3rd alarm,"); break;
        case 6: data_save_buffer_write("3rd alarm release,"); break;
#if BMS_FORTH_ALARM_SUPPORT
        case 7: data_save_buffer_write("4th alarm,"); break;
        case 8: data_save_buffer_write("4th alarm release,"); break;
#endif
        default: data_save_buffer_write(","); break;
#else
        case 0: data_save_buffer_write("正常,"); break;
        case 1: data_save_buffer_write("一级报警,"); break;
        case 2: data_save_buffer_write("一级报警释放,"); break;
        case 3: data_save_buffer_write("二级报警,"); break;
        case 4: data_save_buffer_write("二级报警释放,"); break;
        case 5: data_save_buffer_write("三级报警,"); break;
        case 6: data_save_buffer_write("三级报警释放,"); break;
#if BMS_FORTH_ALARM_SUPPORT
        case 7: data_save_buffer_write("四级报警,"); break;
        case 8: data_save_buffer_write("四级报警释放,"); break;
#endif
        default: data_save_buffer_write(","); break;
#endif
    }
}

#if 0
static void data_save_buffer_write_hex_int(INT16U value)
{
    sprintf(g_data_save_bcd, "%X", value);
    data_save_buffer_write(g_data_save_bcd);
    data_save_buffer_write(",");
}
#endif

static void data_save_buffer_write_float(FP64 value, INT8U fractional_cnt)
{
    float_to_bcd_string(value, g_data_save_bcd, fractional_cnt);
    data_save_buffer_write(g_data_save_bcd);
    data_save_buffer_write(",");
}

static void data_save_buffer_write_string(const char* value)
{
    data_save_buffer_write(value);
    data_save_buffer_write(",");
}

static void data_save_buffer_write_switch(INT8U value)
{
    if (value)
        data_save_buffer_write("1,");
    else
        data_save_buffer_write("0,");
}

static void data_save_buffer_write_cr(void)
{
    data_save_buffer_write("\r\n");
}

static void data_save_buffer_write_eof(void)
{
    data_save_buffer_flush();
}

static Result data_save_check_file_end(
FIL* file,        /* Pointer to the file object */
const TCHAR* path    /* Pointer to the directory path */
)
{
    FRESULT res;
    INT8U buf[2] = {0};
    UINT br;
    
    if(file == NULL) return FR_INVALID_PARAMETER;
    
    if(file->fsize < 2) //创建文件出错，至少存在数据头(header)
    {
        data_save_set_operation_exception(FR_INVALID_OBJECT);
        F_UNLINK(path); return FR_INVALID_OBJECT;
    }
    
    res = F_LSEEK(file, file->fsize-2);                                        // 获取偏移指针
    if(res != FR_OK) return res;
    
    res = F_READ(file, buf, 2, &br);
    if(res != FR_OK) return res;
    
    if ((buf[0] != '\r') || (buf[1] != '\n'))          // 尾判断
    {
        res = F_LSEEK(file, file->fsize);
        if(res != FR_OK) return res;
        
        g_data_save_buffer[0] = '\0';
        data_save_buffer_write_cr();
        data_save_buffer_write_eof();
    }
    
    return FR_OK;
}

static void data_save_task_run_check_sdcard(void* pdata)
{
    (void)pdata;
    sdcard_heart_beat();
}

static void data_save_save(void)
{
    /** check the disk information */
    data_save_save_a_file();

    data_save_save_b_file();

    data_save_save_c_file();

    data_save_save_d_file();
}

static FRESULT data_save_find_file_in_dir(DIR* dp, FILINFO * fno)
{
    FRESULT rc=0;
    
    for (;;)
    {
        rc = F_READDIR(dp, fno); //逐个扫描目录下的文件信息
        if (rc != FR_OK) return rc;
        if (fno->fname[0] == 0) return FR_NO_FILE;

        if (fno->fname[0] == '.') continue;
        
        if (rc == FR_OK) return FR_OK;
    }
}

static FRESULT data_save_find_oldest_in_dir(char *path)
{
    FRESULT rc=0;
    DIR dir;
    FILINFO oldfno;
    FILINFO fno;

    fno.lfname = g_long_file_name_buffer;
    fno.lfsize = DATA_SAVE_FILE_NAME_MAX;
    oldfno.lfname = g_delete_long_file_name;
    oldfno.fsize = DATA_SAVE_FILE_NAME_MAX;
    rc = F_OPENDIR(&dir, path); //打开数据文件目录
    if (rc != FR_OK) return rc;

    oldfno.fdate = 0xFFFF;
    oldfno.ftime = 0xFFFF;
    for (;;) //统计日志目录下日志文件的数目及搜索最早创建的文件
    {
        rc = data_save_find_file_in_dir(&dir, &fno);
        if (rc != FR_OK) break;

        if (oldfno.fdate > fno.fdate) //搜索最早创建的日志文件
            oldfno = fno;
    }
    
    if (oldfno.fdate == 0xFFFF) return FR_NO_FILE;
    
    /** 拼写文件名 */
    strcat(path, "/");
    strcat(path, oldfno.fname); //定位最早创建的文件
    
    rc = F_CLOSEDIR(&dir); //关闭数据文件目录
    return rc;
}

static FRESULT data_save_remove_directory(char *path)
{
    FRESULT rc=0;
    DIR dir;
    FILINFO fno;

    fno.lfname = g_long_file_name_buffer;
    fno.lfsize = DATA_SAVE_FILE_NAME_MAX;
    rc = F_OPENDIR(&dir, path); //打开数据文件目录
    if (rc != FR_OK) return rc;

    for (;;) //统计日志目录下日志文件的数目并全部删除
    {
        INT8U len = 0;
        
        rc = F_READDIR(&dir, &fno); //逐个扫描目录下的文件信息
        if ((rc != FR_OK) || (fno.fname[0] == 0)) break;

        if (fno.fname[0] == '.') continue;

        /** 拼写文件名 */
        strcat(path, "/");
        strcat(path, fno.fname); //定位最早创建的文件

        rc = F_UNLINK(path); //删除文件
        if(rc != FR_OK) return rc;
        
        len = (INT8U)safe_strlen(path);
        if(len == 0) return rc;
        len--;
        while(len && path[len] != '/')
        {
            len--;
        }         
        path[len] = 0;
    }
    rc = F_CLOSEDIR(&dir); //关闭数据文件目录
    if (rc != FR_OK) return rc;
    rc = F_UNLINK(path);

    return rc;
}

static FRESULT data_save_remove_oldest_file(void)
{
    FRESULT rc=0;
    DIR dir;
    FILINFO oldfno;
    FILINFO fno;

    fno.lfname = g_long_file_name_buffer;
    fno.lfsize = DATA_SAVE_FILE_NAME_MAX;
    oldfno.lfname = g_delete_long_file_name;
    oldfno.fsize = DATA_SAVE_FILE_NAME_MAX;
    
    //旧存储格式
    g_data_save_buffer[0] = 0x00;
    strcat(g_data_save_buffer, g_old_root_dir);
    rc = F_OPENDIR(&dir, g_data_save_buffer); //打开旧数据文件目录
    if(rc == FR_OK)
    {
        rc = data_save_find_file_in_dir(&dir, &fno);
        if (rc == FR_OK)
        {
            strcat(g_data_save_buffer, "/");
            strcat(g_data_save_buffer, fno.fname); //定位最早创建的文件
        }
        rc = F_CLOSEDIR(&dir); //关闭数据文件目录
        
        if (rc != FR_OK) return rc;
        
        rc = F_UNLINK(g_data_save_buffer);
    }
    else
    {
        //新存储格式
        /** 拼写文件名 */
        g_data_save_buffer[0] = 0x00;
        strcat(g_data_save_buffer, g_root_dir);
        rc = data_save_find_oldest_in_dir(g_data_save_buffer);
        if (rc != FR_OK) return rc;
        rc = data_save_remove_directory(g_data_save_buffer); //删除文件夹
    }
    return rc;
}

static FRESULT data_save_check_disk(void)
{
    // TODO: f_getfree在sdcard拔出时会出现死锁
    FRESULT rc=0;
    FATFS *fs = NULL;
    INT8U unlink_cnt = 0;
    INT32U free_clustuters = 0; //空闲簇大小
    INT32U free_sectors = 0; //空闲扇区大小
    RPAGE_INIT();
    
    /** 创建目录 */
    rc = F_MKDIR(g_root_dir); //创建目录
    
    /** 检查磁盘空间 */
    free_sectors = 0;
    rc = F_GETFREE(g_fat_path, &free_clustuters, &fs); //获取磁盘卷的信息和空闲簇大小
    if (rc != FR_OK) return rc;
    
    /** 如果磁盘空间不足，删除早期文件 */
    if (free_clustuters < DATA_SAVE_MIN_DISK_SECTORS)
    {
        while(unlink_cnt++ < 20 && rc == FR_OK)
        {
            RPAGE_SAVE();watch_dog_disable();RPAGE_RESTORE();
            rc = data_save_remove_oldest_file();
            RPAGE_SAVE();watch_dog_enable();RPAGE_RESTORE();
            if (rc != FR_OK) return rc;
            
            rc = F_GETFREE(g_fat_path, &free_clustuters, &fs); 
            if (free_clustuters >= DATA_SAVE_REL_MIN_DISK_SECTORS) break;
        }
    }
    if (free_clustuters < DATA_SAVE_MIN_DISK_SECTORS) return FR_INT_ERR;

    return rc;
}

static void data_save_set_root_dir(void)
{
    INT8U i = 0;
    
    /** 生成文件名 */
    g_root_dir[0] = '\0';
    strcat(g_root_dir, DATA_SAVE_PATH);
    strcat(g_root_dir, "_");
    for(i=0; i<5; i++) //17个字符
    {
        INT16U uuid = config_get(kUUIDPart1 + i);
        sprintf(g_data_save_buffer, "%02X", uuid>>8);
        strcat(g_root_dir, g_data_save_buffer);
        if(i < 4)
        {
            sprintf(g_data_save_buffer, "%02X", (INT8U)uuid);
        }
        else
        {
            sprintf(g_data_save_buffer, "%01X", (INT8U)uuid);
        }
        strcat(g_root_dir, g_data_save_buffer);
    }
}

static char* data_save_get_file_dir(void)
{
    INT32U tick = get_tick_count();
    
    g_data_save_buffer[0] = '\0';
    strcat(g_data_save_buffer, g_root_dir);
    strcat(g_data_save_buffer, "/");
    //根据年月日生成日志文件夹
    char_to_bcd_string(20, g_data_save_bcd);
    strcat(g_data_save_buffer, g_data_save_bcd);
    char_to_bcd_string(g_data_save_time.year, g_data_save_bcd);
    strcat(g_data_save_buffer, g_data_save_bcd);
    char_to_bcd_string(g_data_save_time.month, g_data_save_bcd);
    strcat(g_data_save_buffer, g_data_save_bcd);
    
    return g_data_save_buffer;
}

static char* data_save_get_filename(char* suffix)
{
    data_save_get_file_dir();
    strcat(g_data_save_buffer, "/");
    //根据年月日生成日志文件名称
    char_to_bcd_string(g_data_save_time.year, g_data_save_bcd);
    strcat(g_data_save_buffer, g_data_save_bcd);
    char_to_bcd_string(g_data_save_time.month, g_data_save_bcd);
    strcat(g_data_save_buffer, g_data_save_bcd);
    char_to_bcd_string(g_data_save_time.day, g_data_save_bcd);
    strcat(g_data_save_buffer, g_data_save_bcd);
    
    if (suffix)
    {
        strcat(g_data_save_buffer, "_");
        strcat(g_data_save_buffer, suffix);
    }
    strcat(g_data_save_buffer, ".csv");

    return g_data_save_buffer;
}

/****************************A类文件保存************************/
static Result data_save_save_a_file(void)
{
    FRESULT rc;
    char* filename;
    INT8U index;

    if (config_get(kSaveFileAFlag)) return RES_OK;
    if (config_get(kProductDatePart1) == 0 && config_get(kProductDatePart2) == 0) return RES_OK;

    RPAGE_RESET();
    rc = data_save_set_operation_exception(data_save_check_disk());
    if(rc != FR_OK)
    {
        return rc;
    }
    bcu_get_system_time(&g_data_save_time);

    /** 创建目录 */
    filename = data_save_get_file_dir();
    rc = F_MKDIR(filename); //创建目录
    if(rc != FR_OK && rc != FR_EXIST) return rc;
    
    /** 生成文件名 */
    filename = data_save_get_filename("A");
    rc = F_OPEN(&g_fo, filename, FA_WRITE | FA_READ | FA_CREATE_NEW); //打开文件
    if (rc == FR_EXIST)
    {
        F_CLOSE(&g_fo);
    }

    if (rc == FR_OK)
    {
        rc = F_LSEEK(&g_fo, g_fo.fsize); //将文件指针定位到文件末尾
        if(rc != FR_OK)
        {
            F_CLOSE(&g_fo); return rc;
        }

        /* write the header */
        //一次写入字符串长度需小于DATA_SAVE_MAX_LINE_SIZE 
#if BMS_DATASAVE_SUPPORT_ENGLISH
        data_save_buffer_write("Date,Time,Unique code of BCU,Production Date,");  
#else
        data_save_buffer_write("当前日期,当前时间,主机模块唯一码,模块生产时间,");
#endif
        data_save_buffer_write("\r\n");
        data_save_buffer_write_date(&g_data_save_time);
        data_save_buffer_write_time(&g_data_save_time);
        for (index = 0; index < 8; index++)
        {
            sprintf(g_data_save_bcd, "%04X", config_get(kUUIDPart1 + index));
            data_save_buffer_write(g_data_save_bcd);
        }
        data_save_buffer_write(",");
        sprintf(g_data_save_bcd, "%d-%02d-%02d", config_get(kProductDatePart1),
                config_get(kProductDatePart2) & 0xFF, config_get(kProductDatePart2) >> 8);
        data_save_buffer_write(g_data_save_bcd);
        data_save_buffer_write(",");
        data_save_buffer_write_cr();

        data_save_buffer_write_eof();

        rc = F_CLOSE(&g_fo); //关闭文件
    }
    
    config_save(kSaveFileAFlag, 1);

    return RES_OK;
}

static Result data_save_save_b_file(void)
{
    FRESULT rc;
    char* filename;
    INT8U need_write_header = 0;

    if (g_data_save_status_is_changed == 0) return RES_OK;

    RPAGE_RESET();
    rc = data_save_set_operation_exception(data_save_check_disk());
    if(rc != FR_OK)
    {
        return rc;
    }
    bcu_get_system_time(&g_data_save_time);
    /** 创建目录 */
    filename = data_save_get_file_dir();
    rc = F_MKDIR(filename); //创建目录
    if(rc != FR_OK && rc != FR_EXIST) return rc;
    /** 生成文件名 */
    filename = data_save_get_filename("B");
    rc = F_OPEN(&g_fo, filename, FA_WRITE | FA_READ | FA_OPEN_ALWAYS); //打开文件
    if (rc == FR_OK)
    {
        if(g_fo.fsize == 0) need_write_header = 1;
    }

    if (need_write_header)
    {//一次写入字符串长度需小于DATA_SAVE_MAX_LINE_SIZE 
#if BMS_DATASAVE_SUPPORT_ENGLISH
        data_save_buffer_write("Date,Time,Over-charging state,");
        data_save_buffer_write("Over-discharging state,SOC state,High-temperature state,");
        data_save_buffer_write("Low-temperature state,Charging over-current state,");
        data_save_buffer_write("Discharge over-current state,Insulation state,");
        data_save_buffer_write("Communication abort state,Voltage wires state,");
        data_save_buffer_write("Temperature wires state,Total voltage high state,");
        data_save_buffer_write("Total voltage low state,Delta voltage state,");
        data_save_buffer_write("Delta temperature state");  
#else
        data_save_buffer_write("日期,时间,过充状态,过放状态,soc状态,过温状态,");
        data_save_buffer_write("低温状态,充电过流状态,放电过流状态,漏电状态,");
        data_save_buffer_write("通信中断状态,电压检测异常状态,温感异常状态,");
        data_save_buffer_write("总压高状态,总压低状态,压差状态,温差状态");
#endif
        data_save_buffer_write_cr();
        data_save_buffer_write_eof();
    }

    if (rc == FR_OK) //成功打开了最新需要更新的文件
    {
        if(need_write_header) filename = data_save_get_filename("B");
        rc = data_save_check_file_end(&g_fo, filename);
        if(rc != FR_OK) 
        {
            F_CLOSE(&g_fo); return rc;
        }
        
        rc = F_LSEEK(&g_fo, g_fo.fsize); //将文件指针定位到文件末尾
        if(rc != FR_OK)
        {
            F_CLOSE(&g_fo); return rc;
        }

        data_save_buffer_write_date(&g_data_save_time);
        data_save_buffer_write_time(&g_data_save_time);
        data_save_buffer_write_alarm(bcu_get_charge_state()); //过充状态
        data_save_buffer_write_alarm(bcu_get_discharge_state()); //过放状态
        data_save_buffer_write_alarm(bcu_get_low_soc_state()); //SOC状态
        data_save_buffer_write_alarm(bcu_get_high_temperature_state()); //过高温
        data_save_buffer_write_alarm(bcu_get_low_temperature_state()); //过低温
        data_save_buffer_write_alarm(bcu_get_chg_oc_state()); //充电过流
        data_save_buffer_write_alarm(bcu_get_dchg_oc_state()); //充电过流
        data_save_buffer_write_alarm(bcu_get_battery_insulation_state()); //绝缘状态
        data_save_buffer_write_alarm(bcu_get_slave_communication_state()); //通信中断状态
        data_save_buffer_write_alarm(bcu_get_voltage_exception_state()); //电流检测异常状态
        data_save_buffer_write_alarm(bcu_get_temp_exception_state()); //温感异常状态
        data_save_buffer_write_alarm(bcu_get_high_total_volt_state()); //总压状态
        data_save_buffer_write_alarm(bcu_get_low_total_volt_state()); //总压状态
        data_save_buffer_write_alarm(bcu_get_delta_voltage_state()); //压差状态
        data_save_buffer_write_alarm(bcu_get_delta_temperature_state()); //温差状态
        data_save_buffer_write_cr();

        data_save_buffer_write_eof();

        g_data_save_status_is_changed = 0;
    }

    rc = F_CLOSE(&g_fo); //关闭文件
    
    return RES_OK;
}

static Result data_save_save_c_file(void)
{
    FRESULT rc;
    char* filename;
    FP64 fdata;
    INT8U need_write_header = 0;

    if (g_data_save_config_is_changed == 0) return RES_OK;
    if (g_data_save_config_is_changed < CONFIG_CHANGED_DELAY)
    {
        g_data_save_config_is_changed++;
        return RES_OK;
    }
    RPAGE_RESET();
    rc = data_save_set_operation_exception(data_save_check_disk());
    if(rc != FR_OK)
    {
        return rc;
    }
    bcu_get_system_time(&g_data_save_time);
    /** 创建目录 */
    filename = data_save_get_file_dir();
    rc = F_MKDIR(filename); //创建目录
    if(rc != FR_OK && rc != FR_EXIST) return rc;
    /** 生成文件名 */
    filename = data_save_get_filename("C");
    //memset(&g_fo, 0, sizeof(g_fo));
    rc = F_OPEN(&g_fo, filename, FA_WRITE | FA_READ | FA_OPEN_ALWAYS); //打开文件
    if (rc == FR_OK)
    {
        if(g_fo.fsize == 0) need_write_header = 1;
    }

    if (need_write_header)
    {//一次写入字符串长度需小于DATA_SAVE_MAX_LINE_SIZE 
#if BMS_DATASAVE_SUPPORT_ENGLISH
        data_save_buffer_write("Date,Time,Rated capacity(AH),Total capacity(AH),Left capacity(AH),");
        data_save_buffer_write("Calibration current,Max Charging Current(A),Max Discharging Current(A),");
        data_save_buffer_write("Full charging voltage release(V),");
        data_save_buffer_write("Dchg 1st cell high voltage(mV),Dchg 1st cell high voltage release(mV),");
        data_save_buffer_write("Dchg 2st cell high voltage(mV),Dchg 2st cell high voltage release(mV),");
        data_save_buffer_write("Dchg 3rd cell high voltage(mV),Dchg 3rd cell high voltage release(mV),");
        data_save_buffer_write("Dchg 1st cell low voltage(mV),Dchg 1st cell low voltage release(mV),");
        data_save_buffer_write("Dchg 2st cell low voltage(mV),Dchg 2st cell low voltage release(mV),");
        data_save_buffer_write("Dchg 3rd cell low voltage(mV),Dchg 3rd cell low voltage release(mV),");
        data_save_buffer_write("Dchg 1st high temperature(℃),Dchg 1st high temperature release(℃),");
        data_save_buffer_write("Dchg 2st high temperature(℃),Dchg 2st high temperautre release(℃),");
        data_save_buffer_write("Dchg 3rd high temperature(℃),Dchg 3rd high temperautre release(℃),");
        data_save_buffer_write("Dchg 1st low temperature(℃),Dchg 1st low temperature release(℃),");
        data_save_buffer_write("Dchg 2st low temperature(℃),Dchg 2st low temperautre release(℃),");
        data_save_buffer_write("Dchg 3rd low temperature(℃),Dchg 3rd low temperautre release(℃),");
        data_save_buffer_write("Dchg 1st delta temperature(℃),Dchg 1st delta temperature release(℃),");
        data_save_buffer_write("Dchg 2st delta temperature(℃),Dchg 2st delta temperautre release(℃),");
        data_save_buffer_write("Dchg 3rd delta temperature(℃),Dchg 3rd delta temperautre release(℃),");
        data_save_buffer_write("Dchg 1st delta voltage(mV),Dchg 1st delta voltage release(mV),");
        data_save_buffer_write("Dchg 2st delta voltage(mV),Dchg 2st delta voltage release(mV),");
        data_save_buffer_write("Dchg 3rd delta voltage(mV),Dchg 3rd delta voltage release(mV),");
        data_save_buffer_write("Dchg 1st high tv(average volt:mV),Dchg 1st high tv release(mV),");
        data_save_buffer_write("Dchg 2st high tv(mV),Dchg 2st high tv release(mV),");
        data_save_buffer_write("Dchg 3rd high tv(mV),Dchg 3rd high tv release(mV),");
        data_save_buffer_write("Dchg 1st low tv(average volt:mV),Dchg 1st low tv release(mV),");
        data_save_buffer_write("Dchg 2st low tv(mV),Dchg 2st low tv release(mV),");
        data_save_buffer_write("Dchg 3rd low tv(mV),Dchg 3rd low tv release(mV),");
        data_save_buffer_write("Dchg 1st over current(A),Dchg 1st over current release(A),");
        data_save_buffer_write("Dchg 2st over current(A),Dchg 2st over current release(A),");
        data_save_buffer_write("Dchg 3rd over current(A),Dchg 3rd over current release(A),");
        data_save_buffer_write("Chg 1st cell high voltage(mV),Chg 1st cell high voltage release(mV),");
        data_save_buffer_write("Chg 2st cell high voltage(mV),Chg 2st cell high voltage release(mV),");
        data_save_buffer_write("Chg 3rd cell high voltage(mV),Chg 3rd cell high voltage release(mV),");
        data_save_buffer_write("Chg 1st cell low voltage(mV),Chg 1st cell low voltage release(mV),");
        data_save_buffer_write("Chg 2st cell low voltage(mV),Chg 2st cell low voltage release(mV),");
        data_save_buffer_write("Chg 3rd cell low voltage(mV),Chg 3rd cell low voltage release(mV),");
        data_save_buffer_write("Chg 1st high temperature(℃),Chg 1st high temperature release(℃),");
        data_save_buffer_write("Chg 2st high temperature(℃),Chg 2st high temperautre release(℃),");
        data_save_buffer_write("Chg 3rd high temperature(℃),Chg 3rd high temperautre release(℃),");
        data_save_buffer_write("Chg 1st low temperature(℃),Chg 1st low temperature release(℃),");
        data_save_buffer_write("Chg 2st low temperature(℃),Chg 2st low temperautre release(℃),");
        data_save_buffer_write("Chg 3rd low temperature(℃),Chg 3rd low temperautre release(℃),");
        data_save_buffer_write("Chg 1st delta temperature(℃),Chg 1st delta temperature release(℃),");
        data_save_buffer_write("Chg 2st delta temperature(℃),Chg 2st delta temperautre release(℃),");
        data_save_buffer_write("Chg 3rd delta temperature(℃),Chg 3rd delta temperautre release(℃),");
        data_save_buffer_write("Chg 1st delta voltage(mV),Chg 1st delta voltage release(mV),");
        data_save_buffer_write("Chg 2st delta voltage(mV),Chg 2st delta voltage release(mV),");
        data_save_buffer_write("Chg 3rd delta voltage(mV),Chg 3rd delta voltage release(mV),");
        data_save_buffer_write("Chg 1st high tv(average volt:mV),Chg 1st high tv release(mV),");
        data_save_buffer_write("Chg 2st high tv(mV),Chg 2st high tv release(mV),");
        data_save_buffer_write("Chg 3rd high tv(mV),Chg 3rd high tv release(mV),");
        data_save_buffer_write("Chg 1st low tv(average volt:mV),Chg 1st low tv release(mV),");
        data_save_buffer_write("Chg 2st low tv(mV),Chg 2st low tv release(mV),");
        data_save_buffer_write("Chg 3rd low tv(mV),Chg 3rd low tv release(mV),");
        data_save_buffer_write("Chg 1st over current(A),Chg 1st over current release(A),");
        data_save_buffer_write("Chg 2st over current(A),Chg 2st over current release(A),");
        data_save_buffer_write("Chg 3rd over current(A),Chg 3rd over current release(A),");
        data_save_buffer_write("Refrigeration starting temperature(℃),");
        data_save_buffer_write("Refrigeration stopping temperature(℃),");
        data_save_buffer_write("Heating starting temperature(℃),");
        data_save_buffer_write("Heating stopping temperature(℃)");
#else
        data_save_buffer_write("日期,时间,额定容量(AH),总容量(AH),剩余容量(AH),");
        data_save_buffer_write("电流校准值,最大充电电流,最大放电电流,满充释放总压,");
        data_save_buffer_write("放电单体高压一级报警值(mV),放电单体高压一级释放值(mV),");
        data_save_buffer_write("放电单体高压二级报警值(mV),放电单体高压二级释放值(mV),");
        data_save_buffer_write("放电单体高压三级报警值(mV),放电单体高压三级释放值(mV),");
        data_save_buffer_write("放电单体低压一级报警值(mV),放电单体低压一级释放值(mV),");
        data_save_buffer_write("放电单体低压二级报警值(mV),放电单体低压二级释放值(mV),");
        data_save_buffer_write("放电单体低压三级报警值(mV),放电单体低压三级释放值(mV),");
        data_save_buffer_write("放电单体高温一级报警值(℃),放电单体高温一级释放值(℃),");
        data_save_buffer_write("放电单体高温二级报警值(℃),放电单体高温二级释放值(℃),");
        data_save_buffer_write("放电单体高温三级报警值(℃),放电单体高温三级释放值(℃),");
        data_save_buffer_write("放电单体低温一级报警值(℃),放电单体低温一级释放值(℃),");
        data_save_buffer_write("放电单体低温二级报警值(℃),放电单体低温二级释放值(℃),");
        data_save_buffer_write("放电单体低温三级报警值(℃),放电单体低温三级释放值(℃),");
        data_save_buffer_write("放电单体温差一级报警值(℃),放电单体温差一级释放值(℃),");
        data_save_buffer_write("放电单体温差二级报警值(℃),放电单体温差二级释放值(℃),");
        data_save_buffer_write("放电单体温差三级报警值(℃),放电单体温差三级释放值(℃),");
        data_save_buffer_write("放电单体压差一级报警值(mV),放电单体压差一级释放值(mV),");
        data_save_buffer_write("放电单体压差二级报警值(mV),放电单体压差二级释放值(mV),");
        data_save_buffer_write("放电单体压差三级报警值(mV),放电单体压差三级释放值(mV),");
        data_save_buffer_write("放电总压高一级报警值(平均电压：mV),放电总压高一级释放值(mV),");
        data_save_buffer_write("放电总压高二级报警值(mV),放电总压高二级释放值(mV),");
        data_save_buffer_write("放电总压高三级报警值(mV),放电总压高三级释放值(mV),");
        data_save_buffer_write("放电总压低一级报警值(平均电压：mV),放电总压低一级释放值(mV),");
        data_save_buffer_write("放电总压低二级报警值(mV),放电总压低二级释放值(mV),");
        data_save_buffer_write("放电总压低三级报警值(mV),放电总压低三级释放值(mV),");
        data_save_buffer_write("放电过流一级报警值(A),放电过流一级释放值(A),");
        data_save_buffer_write("放电过流二级报警值(A),放电过流二级释放值(A),");
        data_save_buffer_write("放电过流三级报警值(A),放电过流三级释放值(A),");
        data_save_buffer_write("充电单体高压一级报警值(mV),充电单体高压一级释放值(mV),");
        data_save_buffer_write("充电单体高压二级报警值(mV),充电单体高压二级释放值(mV),");
        data_save_buffer_write("充电单体高压三级报警值(mV),充电单体高压三级释放值(mV),");
        data_save_buffer_write("充电单体低压一级报警值(mV),充电单体低压一级释放值(mV),");
        data_save_buffer_write("充电单体低压二级报警值(mV),充电单体低压二级释放值(mV),");
        data_save_buffer_write("充电单体低压三级报警值(mV),充电单体低压三级释放值(mV),");
        data_save_buffer_write("充电单体高温一级报警值(℃),充电单体高温一级释放值(℃),");
        data_save_buffer_write("充电单体高温二级报警值(℃),充电单体高温二级释放值(℃),");
        data_save_buffer_write("充电单体高温三级报警值(℃),充电单体高温三级释放值(℃),");
        data_save_buffer_write("充电单体低温一级报警值(℃),充电单体低温一级释放值(℃),");
        data_save_buffer_write("充电单体低温二级报警值(℃),充电单体低温二级释放值(℃),");
        data_save_buffer_write("充电单体低温三级报警值(℃),充电单体低温三级释放值(℃),");
        data_save_buffer_write("充电单体温差一级报警值(℃),充电单体温差一级释放值(℃),");
        data_save_buffer_write("充电单体温差二级报警值(℃),充电单体温差二级释放值(℃),");
        data_save_buffer_write("充电单体温差三级报警值(℃),充电单体温差三级释放值(℃),");
        data_save_buffer_write("充电单体压差一级报警值(mV),充电单体压差一级释放值(mV),");
        data_save_buffer_write("充电单体压差二级报警值(mV),充电单体压差二级释放值(mV),");
        data_save_buffer_write("充电单体压差三级报警值(mV),充电单体压差三级释放值(mV),");
        data_save_buffer_write("充电总压高一级报警值(平均电压：mV),充电总压高一级释放值(mV),");
        data_save_buffer_write("充电总压高二级报警值(mV),充电总压高二级释放值(mV),");
        data_save_buffer_write("充电总压高三级报警值(mV),充电总压高三级释放值(mV),");
        data_save_buffer_write("充电总压低一级报警值(平均电压：mV),充电总压低一级释放值(mV),");
        data_save_buffer_write("充电总压低二级报警值(mV),充电总压低二级释放值(mV),");
        data_save_buffer_write("充电总压低三级报警值(mV),充电总压低三级释放值(mV),");
        data_save_buffer_write("充电过流一级报警值(A),充电过流一级释放值(A),");
        data_save_buffer_write("充电过流二级报警值(A),充电过流二级释放值(A),");
        data_save_buffer_write("充电过流三级报警值(A),充电过流三级释放值(A),");
        data_save_buffer_write("制冷开启温度(℃),制冷关闭温度(℃),");
        data_save_buffer_write("制热开启温度(℃),制热关闭温度(℃)");
#endif
        data_save_buffer_write_cr();
        data_save_buffer_write_eof();
    }
    if (rc != FR_OK)
    {
        F_CLOSE(&g_fo); //关闭文件
        return RES_ERROR;
    }

    g_data_save_config_is_changed = 0;
    
    if(need_write_header) filename = data_save_get_filename("C");
    rc = data_save_check_file_end(&g_fo, filename);
    if(rc != FR_OK) 
    {
        F_CLOSE(&g_fo); return rc;
    }
    
    //成功打开了最新需要更新的文件
    rc = F_LSEEK(&g_fo, g_fo.fsize); //将文件指针定位到文件末尾
    if(rc != FR_OK)
    {
        F_CLOSE(&g_fo); return rc;
    }

    data_save_buffer_write_date(&g_data_save_time);
    data_save_buffer_write_time(&g_data_save_time);
    //额定容量
    data_save_buffer_write_int(config_get(kNominalCapIndex));
    //总容量
    data_save_buffer_write_int(config_get(kTotalCapIndex));
    //剩余容量
    data_save_buffer_write_int(config_get(kLeftCapIndex));
    //电流校准值
    fdata = (INT16S)config_get(kCchkIndex);
    fdata = fdata / 10;
    data_save_buffer_write_float(fdata, 3);
    //最大充电电流
    fdata = config_get(kChgOCSndAlarmIndex);
    fdata = fdata / 10;
    data_save_buffer_write_float(fdata, 1);
    //最大放电电流
    fdata = config_get(kDChgOCSndAlarmIndex);
    fdata = fdata / 10;
    data_save_buffer_write_float(fdata, 1);
    //满充释放总压
    data_save_buffer_write_float((float)config_get(kChgFullChgTVReleaseIndex) / 10, 1);
    //放电单体高压一级报警值
    data_save_buffer_write_int(config_get(kDChgHVFstAlarmIndex));
    //放电单体高压一级释放值
    data_save_buffer_write_int(config_get(kDChgHVFstAlarmRelIndex));
    //放电单体高压二级报警值
    data_save_buffer_write_int(config_get(kDChgHVSndAlarmIndex));
    //放电单体高压二级释放值
    data_save_buffer_write_int(config_get(kDChgHVSndAlarmRelIndex));
    //放电单体高压三级报警值
    data_save_buffer_write_int(config_get(kDChgHVTrdAlarmIndex));
    //放电单体高压三级释放值
    data_save_buffer_write_int(config_get(kDChgHVTrdAlarmRelIndex));
    //放电单体低压一级报警值
    data_save_buffer_write_int(config_get(kDChgLVFstAlarmIndex));
    //放电单体低压一级释放值
    data_save_buffer_write_int(config_get(kDChgLVFstAlarmRelIndex));
    //放电单体低压二级报警值
    data_save_buffer_write_int(config_get(kDChgLVSndAlarmIndex));
    //放电单体低压二级释放值
    data_save_buffer_write_int(config_get(kDChgLVSndAlarmRelIndex));
    //放电单体低压三级报警值
    data_save_buffer_write_int(config_get(kDChgLVTrdAlarmIndex));
    //放电单体低压三级释放值
    data_save_buffer_write_int(config_get(kDChgLVTrdAlarmRelIndex));
    //放电单体高温一级报警值
    fdata = TEMPERATURE_TO_C(config_get(kDChgHTFstAlarmIndex));
    data_save_buffer_write_float(fdata, 1);
    //放电单体高温一级释放值
    fdata = TEMPERATURE_TO_C(config_get(kDChgHTFstAlarmRelIndex));
    data_save_buffer_write_float(fdata, 1);
    //放电单体高温二级报警值
    fdata = TEMPERATURE_TO_C(config_get(kDChgHTSndAlarmIndex));
    data_save_buffer_write_float(fdata, 1);
    //放电单体高温二级释放值
    fdata = TEMPERATURE_TO_C(config_get(kDChgHTSndAlarmRelIndex));
    data_save_buffer_write_float(fdata, 1);
    //放电单体高温三级报警值
    fdata = TEMPERATURE_TO_C(config_get(kDChgHTTrdAlarmIndex));
    data_save_buffer_write_float(fdata, 1);
    //放电单体高温三级释放值
    fdata = TEMPERATURE_TO_C(config_get(kDChgHTTrdAlarmRelIndex));
    data_save_buffer_write_float(fdata, 1);
    //放电单体低温一级报警值
    fdata = TEMPERATURE_TO_C(config_get(kDChgLTFstAlarmIndex));
    data_save_buffer_write_float(fdata, 1);
    //放电单体低温一级释放值
    fdata = TEMPERATURE_TO_C(config_get(kDChgLTFstAlarmRelIndex));
    data_save_buffer_write_float(fdata, 1);
    //放电单体低温二级报警值
    fdata = TEMPERATURE_TO_C(config_get(kDChgLTSndAlarmIndex));
    data_save_buffer_write_float(fdata, 1);
    //放电单体低温二级释放值
    fdata = TEMPERATURE_TO_C(config_get(kDChgLTSndAlarmRelIndex));
    data_save_buffer_write_float(fdata, 1);
    //放电单体低温三级报警值
    fdata = TEMPERATURE_TO_C(config_get(kDChgLTTrdAlarmIndex));
    data_save_buffer_write_float(fdata, 1);
    //放电单体低温三级释放值
    fdata = TEMPERATURE_TO_C(config_get(kDChgLTTrdAlarmRelIndex));
    data_save_buffer_write_float(fdata, 1);
    //放电单体温差一级报警值
    fdata = config_get(kDChgDTFstAlarmIndex);
    data_save_buffer_write_float(fdata, 1);
    //放电单体温差一级释放值
    fdata = config_get(kDChgDTFstAlarmRelIndex);
    data_save_buffer_write_float(fdata, 1);
    //放电单体温差二级报警值
    fdata = config_get(kDChgDTSndAlarmIndex);
    data_save_buffer_write_float(fdata, 1);
    //放电单体温差二级释放值
    fdata = config_get(kDChgDTSndAlarmRelIndex);
    data_save_buffer_write_float(fdata, 1);
    //放电单体温差三级报警值
    fdata = config_get(kDChgDTTrdAlarmIndex);
    data_save_buffer_write_float(fdata, 1);
    //放电单体温差三级释放值
    fdata = config_get(kDChgDTTrdAlarmRelIndex);
    data_save_buffer_write_float(fdata, 1);
    //放电单体压差一级报警值
    data_save_buffer_write_int(config_get(kDChgDVFstAlarmIndex));
    //放电单体压差一级释放值
    data_save_buffer_write_int(config_get(kDChgDVFstAlarmRelIndex));
    //放电单体压差二级报警值
    data_save_buffer_write_int(config_get(kDChgDVSndAlarmIndex));
    //放电单体压差二级释放值
    data_save_buffer_write_int(config_get(kDChgDVSndAlarmRelIndex));
    //放电单体压差三级报警值
    data_save_buffer_write_int(config_get(kDChgDVTrdAlarmIndex));
    //放电单体压差三级释放值
    data_save_buffer_write_int(config_get(kDChgDVTrdAlarmRelIndex));
    //放电总压高一级报警值
    data_save_buffer_write_int(config_get(kDChgHTVFstAlarmIndex));
    //放电总压高一级释放值
    data_save_buffer_write_int(config_get(kDChgHTVFstAlarmRelIndex));
    //放电总压高二级报警值
    data_save_buffer_write_int(config_get(kDChgHTVSndAlarmIndex));
    //放电总压高二级释放值
    data_save_buffer_write_int(config_get(kDChgHTVSndAlarmRelIndex));
    //放电总压高三级报警值
    data_save_buffer_write_int(config_get(kDChgHTVTrdAlarmIndex));
    //放电总压高三级释放值
    data_save_buffer_write_int(config_get(kDChgHTVTrdAlarmRelIndex));
    //放电总压低一级报警值
    data_save_buffer_write_int(config_get(kDChgLTVFstAlarmIndex));
    //放电总压低一级释放值
    data_save_buffer_write_int(config_get(kDChgLTVFstAlarmRelIndex));
    //放电总压低二级报警值
    data_save_buffer_write_int(config_get(kDChgLTVSndAlarmIndex));
    //放电总压低二级释放值
    data_save_buffer_write_int(config_get(kDChgLTVSndAlarmRelIndex));
    //放电总压低三级报警值
    data_save_buffer_write_int(config_get(kDChgLTVTrdAlarmIndex));
    //放电总压低三级释放值
    data_save_buffer_write_int(config_get(kDChgLTVTrdAlarmRelIndex));
    //放电过流一级报警值
    data_save_buffer_write_int(CURRENT_100_MA_TO_A(config_get(kDChgOCFstAlarmIndex)));
    //放电过流一级释放值
    fdata = config_get(kDChgOCFstAlarmRelIndex);
    fdata = fdata / 10;
    data_save_buffer_write_float(fdata, 1);
    //放电过流二级报警值
    fdata = config_get(kDChgOCSndAlarmIndex);
    fdata = fdata / 10;
    data_save_buffer_write_float(fdata, 1);
    //放电过流二级释放值
    fdata = config_get(kDChgOCSndAlarmRelIndex);
    fdata = fdata / 10;
    data_save_buffer_write_float(fdata, 1);
    //放电过流三级报警值
    fdata = config_get(kDChgOCTrdAlarmIndex);
    fdata = fdata / 10;
    data_save_buffer_write_float(fdata, 1);
    //放电过流三级释放值
    fdata = config_get(kDChgOCTrdAlarmRelIndex);
    fdata = fdata / 10;
    data_save_buffer_write_float(fdata, 1);
    //充电单体高压一级报警值
    data_save_buffer_write_int(config_get(kChgHVFstAlarmIndex));
    //充电单体高压一级释放值
    data_save_buffer_write_int(config_get(kChgHVFstAlarmRelIndex));
    //充电单体高压二级报警值
    data_save_buffer_write_int(config_get(kChgHVSndAlarmIndex));
    //充电单体高压二级释放值
    data_save_buffer_write_int(config_get(kChgHVSndAlarmRelIndex));
    //充电单体高压三级报警值
    data_save_buffer_write_int(config_get(kChgHVTrdAlarmIndex));
    //充电单体高压三级释放值
    data_save_buffer_write_int(config_get(kChgHVTrdAlarmRelIndex));
    //充电单体低压一级报警值
    data_save_buffer_write_int(config_get(kChgLVFstAlarmIndex));
    //充电单体低压一级释放值
    data_save_buffer_write_int(config_get(kChgLVFstAlarmRelIndex));
    //充电单体低压二级报警值
    data_save_buffer_write_int(config_get(kChgLVSndAlarmIndex));
    //充电单体低压二级释放值
    data_save_buffer_write_int(config_get(kChgLVSndAlarmRelIndex));
    //充电单体低压三级报警值
    data_save_buffer_write_int(config_get(kChgLVTrdAlarmIndex));
    //充电单体低压三级释放值
    data_save_buffer_write_int(config_get(kChgLVTrdAlarmRelIndex));
    //充电单体高温一级报警值
    fdata = TEMPERATURE_TO_C(config_get(kChgHTFstAlarmIndex));
    data_save_buffer_write_float(fdata, 1);
    //充电单体高温一级释放值
    fdata = TEMPERATURE_TO_C(config_get(kChgHTFstAlarmRelIndex));
    data_save_buffer_write_float(fdata, 1);
    //充电单体高温二级报警值
    fdata = TEMPERATURE_TO_C(config_get(kChgHTSndAlarmIndex));
    data_save_buffer_write_float(fdata, 1);
    //充电单体高温二级释放值
    fdata = TEMPERATURE_TO_C(config_get(kChgHTSndAlarmRelIndex));
    data_save_buffer_write_float(fdata, 1);
    //充电单体高温三级报警值
    fdata = TEMPERATURE_TO_C(config_get(kChgHTTrdAlarmIndex));
    data_save_buffer_write_float(fdata, 1);
    //充电单体高温三级释放值
    fdata = TEMPERATURE_TO_C(config_get(kChgHTTrdAlarmRelIndex));
    data_save_buffer_write_float(fdata, 1);
    //充电单体低温一级报警值
    fdata = TEMPERATURE_TO_C(config_get(kChgLTFstAlarmIndex));
    data_save_buffer_write_float(fdata, 1);
    //充电单体低温一级释放值
    fdata = TEMPERATURE_TO_C(config_get(kChgLTFstAlarmRelIndex));
    data_save_buffer_write_float(fdata, 1);
    //充电单体低温二级报警值
    fdata = TEMPERATURE_TO_C(config_get(kChgLTSndAlarmIndex));
    data_save_buffer_write_float(fdata, 1);
    //充电单体低温二级释放值
    fdata = TEMPERATURE_TO_C(config_get(kChgLTSndAlarmRelIndex));
    data_save_buffer_write_float(fdata, 1);
    //充电单体低温三级报警值
    fdata = TEMPERATURE_TO_C(config_get(kChgLTTrdAlarmIndex));
    data_save_buffer_write_float(fdata, 1);
    //充电单体低温三级释放值
    fdata = TEMPERATURE_TO_C(config_get(kChgLTTrdAlarmRelIndex));
    data_save_buffer_write_float(fdata, 1);
    //充电单体温差一级报警值
    fdata = config_get(kChgDTFstAlarmIndex);
    data_save_buffer_write_float(fdata, 1);
    //充电单体温差一级释放值
    fdata = config_get(kChgDTFstAlarmRelIndex);
    data_save_buffer_write_float(fdata, 1);
    //充电单体温差二级报警值
    fdata = config_get(kChgDTSndAlarmIndex);
    data_save_buffer_write_float(fdata, 1);
    //充电单体温差二级释放值
    fdata = config_get(kChgDTSndAlarmRelIndex);
    data_save_buffer_write_float(fdata, 1);
    //充电单体温差三级报警值
    fdata = config_get(kChgDTTrdAlarmIndex);
    data_save_buffer_write_float(fdata, 1);
    //充电单体温差三级释放值
    fdata = config_get(kChgDTTrdAlarmRelIndex);
    data_save_buffer_write_float(fdata, 1);
    //充电单体压差一级报警值
    data_save_buffer_write_int(config_get(kChgDVFstAlarmIndex));
    //充电单体压差一级释放值
    data_save_buffer_write_int(config_get(kChgDVFstAlarmRelIndex));
    //充电单体压差二级报警值
    data_save_buffer_write_int(config_get(kChgDVSndAlarmIndex));
    //充电单体压差二级释放值
    data_save_buffer_write_int(config_get(kChgDVSndAlarmRelIndex));
    //充电单体压差三级报警值
    data_save_buffer_write_int(config_get(kChgDVTrdAlarmIndex));
    //充电单体压差三级释放值
    data_save_buffer_write_int(config_get(kChgDVTrdAlarmRelIndex));
    //充电总压高一级报警值
    data_save_buffer_write_int(config_get(kChgHTVFstAlarmIndex));
    //充电总压高一级释放值
    data_save_buffer_write_int(config_get(kChgHTVFstAlarmRelIndex));
    //充电总压高二级报警值
    data_save_buffer_write_int(config_get(kChgHTVSndAlarmIndex));
    //充电总压高二级释放值
    data_save_buffer_write_int(config_get(kChgHTVSndAlarmRelIndex));
    //充电总压高三级报警值
    data_save_buffer_write_int(config_get(kChgHTVTrdAlarmIndex));
    //充电总压高三级释放值
    data_save_buffer_write_int(config_get(kChgHTVTrdAlarmRelIndex));
    //充电总压低一级报警值
    data_save_buffer_write_int(config_get(kChgLTVFstAlarmIndex));
    //充电总压低一级释放值
    data_save_buffer_write_int(config_get(kChgLTVFstAlarmRelIndex));
    //充电总压低二级报警值
    data_save_buffer_write_int(config_get(kChgLTVSndAlarmIndex));
    //充电总压低二级释放值
    data_save_buffer_write_int(config_get(kChgLTVSndAlarmRelIndex));
    //充电总压低三级报警值
    data_save_buffer_write_int(config_get(kChgLTVTrdAlarmIndex));
    //充电总压低三级释放值
    data_save_buffer_write_int(config_get(kChgLTVTrdAlarmRelIndex));
    //充电过流一级报警值
    fdata = config_get(kChgOCFstAlarmIndex);
    fdata = fdata / 10;
    data_save_buffer_write_float(fdata, 1);
    //充电过流一级释放值
    fdata = config_get(kChgOCFstAlarmRelIndex);
    fdata = fdata / 10;
    data_save_buffer_write_float(fdata, 1);
    //充电过流二级报警值
    fdata = config_get(kChgOCSndAlarmIndex);
    fdata = fdata / 10;
    data_save_buffer_write_float(fdata, 1);
    //充电过流二级释放值
    fdata = config_get(kChgOCSndAlarmRelIndex);
    fdata = fdata / 10;
    data_save_buffer_write_float(fdata, 1);
    //充电过流三级报警值
    fdata = config_get(kChgOCTrdAlarmIndex);
    fdata = fdata / 10;
    data_save_buffer_write_float(fdata, 1);
    //充电过流三级释放值
    fdata = config_get(kChgOCTrdAlarmRelIndex);
    fdata = fdata / 10;
    data_save_buffer_write_float(fdata, 1);
    //制冷开启温度
    fdata = TEMPERATURE_TO_C(config_get(kCoolOnTemperature));
    data_save_buffer_write_float(fdata, 1);
    //制冷关闭温度
    fdata = TEMPERATURE_TO_C(config_get(kCoolOffTemperature));
    data_save_buffer_write_float(fdata, 1);
    // 加热开启温度
    fdata = TEMPERATURE_TO_C(config_get(kHeatOnTemperature)); 
    data_save_buffer_write_float(fdata, 1);
    //加热关闭温度
    fdata = TEMPERATURE_TO_C(config_get(kHeatOffTemperature)); 
    data_save_buffer_write_float(fdata, 1);
    
    data_save_buffer_write_cr();
    data_save_buffer_write_eof();
    rc = F_CLOSE(&g_fo); //关闭文件
    
    return RES_OK;
}

static Result data_save_save_d_file(void)
{
    FRESULT rc;
    char* filename;
    FP64 fdata;
    INT8U i, j, offset;
    INT32U tick;
    INT8U slave_num;
    INT16U index, voltage_num, bat_temperature_num,heat_temperature_num;
    INT8U balance_state, balance_temp;
    INT8U need_write_header = 0;
    RPAGE_INIT();
    
    tick = get_tick_count();
    if (get_interval_by_tick(g_data_save_d_file_last_tick, tick) < config_get(kSysStatusSaveIntervalIndex))
    {
        return RES_OK;
    }
    g_data_save_d_file_last_tick = tick;
    
    RPAGE_RESET();
    rc = data_save_set_operation_exception(data_save_check_disk());
    if(rc != FR_OK)
    {
        return rc;
    }
    bcu_get_system_time(&g_data_save_time);
    //DEBUG("data", "save d class file %d, %X, enter", ++times, &rc);
    slave_num = (INT8U)config_get(kSlaveNumIndex);

    /** 创建目录 */
    filename = data_save_get_file_dir();
    rc = F_MKDIR(filename); //创建目录
    if(rc != FR_OK && rc != FR_EXIST) return rc;
    /** 生成文件名 */
    filename = data_save_get_filename("D");

    rc = F_OPEN(&g_fo, filename, FA_WRITE | FA_READ | FA_OPEN_ALWAYS); //打开文件
    if (rc == FR_OK)
    {
        if(g_fo.fsize == 0) need_write_header = 1;
    }
    
    voltage_num = bcu_get_voltage_num();
    bat_temperature_num = bcu_get_temperature_num();         //电池温感数
    heat_temperature_num = bcu_get_heat_temperature_num();   //加热片温感数
    
    if (need_write_header)
    {
#if BMS_DATASAVE_SUPPORT_ENGLISH
        data_save_buffer_write("Life,Date,Time,SOC(%),SOH(%),Total voltage(V),Current(A),");                     
        data_save_buffer_write("Lead-acid voltage(V),Charger supply voltage(V),Power trigger source,");
        data_save_buffer_write("Onboard temperature1(℃),Onboard temperature2(℃),");
        data_save_buffer_write("Positive resistance(kOhm),Negative resistance(kOhm),");
        data_save_buffer_write("System insulation resistance(kOhm),Charge times,");
        data_save_buffer_write("GBCharger fault num,Charge fault num,Discharge fault num,");
        data_save_buffer_write("BMU number,");

        for (index = 0; index < slave_num; index++)
        {
            data_save_buffer_write(STRING_BMU);
            int_to_bcd_string(index+1, g_data_save_bcd);
            data_save_buffer_write(g_data_save_bcd);
            data_save_buffer_write("battery number,");
        }
        //电池温感数
        for (index = 0; index < slave_num; index++)
        {
            data_save_buffer_write(STRING_BMU);
            int_to_bcd_string(index+1, g_data_save_bcd);
            data_save_buffer_write(g_data_save_bcd);
            data_save_buffer_write("temperature number,");
        }
        //加热片温感数
        for (index = 0; index < slave_num; index++)
        {
            data_save_buffer_write(STRING_BMU);
            int_to_bcd_string(index+1, g_data_save_bcd);
            data_save_buffer_write(g_data_save_bcd);
            data_save_buffer_write("heat temperature number,");
        }
        
        for (index = 0; index < voltage_num; index++)
        {
            data_save_buffer_write("Cell voltage");
            int_to_bcd_string(index + 1, g_data_save_bcd);
            data_save_buffer_write(g_data_save_bcd);
            data_save_buffer_write("(mV),");
        }
        //电池温度 
        for (index = 0; index < temperature_num; index++)
        {
            data_save_buffer_write("Temperature");
            int_to_bcd_string(index + 1, g_data_save_bcd);
            data_save_buffer_write(g_data_save_bcd);
            data_save_buffer_write("(℃),");
        }
        //加热片温度  
        for (index = 0; index < heat_temperature_num; index++)
        {
            data_save_buffer_write("Heat Temperature");
            int_to_bcd_string(index + 1, g_data_save_bcd);
            data_save_buffer_write(g_data_save_bcd);
            data_save_buffer_write("(℃),");
        }
        
        for (index = 0; index < BMS_BMU_BALANCE_NUM; index++)
        {
            data_save_buffer_write("Balancing current");
            int_to_bcd_string(index + 1, g_data_save_bcd);
            data_save_buffer_write(g_data_save_bcd);
            data_save_buffer_write("(A),");
        }
        
        for (index = 0; index < ((voltage_num + 7) / 8); index++)
        {
            sprintf(g_data_save_bcd, "%d-%dBalancing state,", index*8+1, (index*8+8)<=voltage_num?(index*8+8):voltage_num);
            data_save_buffer_write(g_data_save_bcd);
        }
#else
        data_save_buffer_write("编号,日期,时间,");
        data_save_buffer_write("SOC(%),SOH(%),总压(V),电流(A),");
        data_save_buffer_write("铅酸电压(V),充电机供电电压(V),电源触发信号,");
        data_save_buffer_write("板载温度1(℃),板载温度2(℃),");
        data_save_buffer_write("绝缘模块正极阻值(kOhm),绝缘模块负极阻值(kOhm),");
        data_save_buffer_write("系统绝缘阻值(kOhm),充电次数,国标充电故障码,");
        data_save_buffer_write("充电故障码,放电故障码,从机个数,");
        
        for (index = 0; index < slave_num; index++)
        {
            data_save_buffer_write("从机");
            int_to_bcd_string(index+1, g_data_save_bcd);
            data_save_buffer_write(g_data_save_bcd);
            data_save_buffer_write("电池数,");
        }
        //电池温感数
        for (index = 0; index < slave_num; index++)
        {
            data_save_buffer_write("从机");
            int_to_bcd_string(index+1, g_data_save_bcd);
            data_save_buffer_write(g_data_save_bcd);
            data_save_buffer_write("电池温感数,");
        }
        //加热片温感数
        for (index = 0; index < slave_num; index++)
        {
            data_save_buffer_write("从机");
            int_to_bcd_string(index+1, g_data_save_bcd);
            data_save_buffer_write(g_data_save_bcd);
            data_save_buffer_write("加热片温感数,");
        }

        for (index = 0; index < voltage_num; index++)
        {
            data_save_buffer_write("电压");
            int_to_bcd_string(index + 1, g_data_save_bcd);
            data_save_buffer_write(g_data_save_bcd);
            data_save_buffer_write("(mV),");
        }
        //电池温度  
        for (index = 0; index < bat_temperature_num; index++)
        {
            data_save_buffer_write("电池温度");
            int_to_bcd_string(index + 1, g_data_save_bcd);
            data_save_buffer_write(g_data_save_bcd);
            data_save_buffer_write("(℃),");
        }
        //加热片温度 
        for (index = 0; index < heat_temperature_num; index++)
        {
            data_save_buffer_write("加热片温度");
            int_to_bcd_string(index + 1, g_data_save_bcd);
            data_save_buffer_write(g_data_save_bcd);
            data_save_buffer_write("(℃),");
        }

        for (index = 0; index < BMS_BMU_BALANCE_NUM; index++)
        {
            data_save_buffer_write("均衡电流");
            int_to_bcd_string(index + 1, g_data_save_bcd);
            data_save_buffer_write(g_data_save_bcd);
            data_save_buffer_write("(A),");
        }
        
        for (index = 0; index < ((voltage_num + 7) / 8); index++)
        {
            sprintf(g_data_save_bcd, "%d-%d节均衡状态,", index*8+1, (index*8+8)<=voltage_num?(index*8+8):voltage_num);
            data_save_buffer_write(g_data_save_bcd);
        }
#endif
        data_save_buffer_write("\r\n");

        data_save_buffer_write_eof();

    }

    if (rc != FR_OK)
    {
        F_CLOSE(&g_fo); //关闭文件
        //DEBUG("data", "save d class file, leave, cannot write, rc:%d", rc);
        return RES_ERROR;
    }

    if(need_write_header) filename = data_save_get_filename("D");
    rc = data_save_check_file_end(&g_fo, filename);
    if(rc != FR_OK) 
    {
        F_CLOSE(&g_fo); return rc;
    }
    
    rc = F_LSEEK(&g_fo, g_fo.fsize); //将文件指针定位到文件末尾
    if(rc != FR_OK)
    {
        F_CLOSE(&g_fo); return rc;
    }
    //Life
    data_save_buffer_write_int(g_data_save_file_d_life++);
    
    data_save_buffer_write_date(&g_data_save_time);

    data_save_buffer_write_time(&g_data_save_time);

    //SOC
    data_save_buffer_write_int((INT16U) SOC_TO_PERCENT(bcu_get_SOC()));
    
    //SOH
    data_save_buffer_write_int((INT16U) SOC_TO_PERCENT(bcu_get_SOH()));

    //总压
    data_save_buffer_write_int((INT16U) MV_TO_V(bcu_get_total_voltage()));

    //电流
    fdata = bcu_get_current();
    data_save_buffer_write_float(fdata/10, 1);
    
    //铅酸供电电压
    DATA_SAVE_SAFE_GET(fdata, bcu_get_lead_acid_volt());
    if(fdata < BMS_SYSTEM_VOLTAGE_LOW_THRESHOLD) fdata = 0;
    data_save_buffer_write_float(fdata / 1000, 1);
    
    //充电机供电电压
    DATA_SAVE_SAFE_GET(fdata, dc_24V_voltage_get());
    if(fdata < BMS_SYSTEM_VOLTAGE_LOW_THRESHOLD) fdata = 0;
    data_save_buffer_write_float(fdata / 1000, 1);
    
    //电源触发信号
    DATA_SAVE_SAFE_GET(index, bms_get_power_trigger_signal());
    data_save_buffer_write_int(index);
    
    //板载温度1
    DATA_SAVE_SAFE_GET(fdata, TEMPERATURE_TO_C(board_temperature_get()));
    data_save_buffer_write_float(fdata, 1);
    
    //板载温度2
    DATA_SAVE_SAFE_GET(fdata, TEMPERATURE_TO_C(board_temperature2_get()));
    data_save_buffer_write_float(fdata, 1);
    
    //正极绝缘阻值
    data_save_buffer_write_int((INT16U) bcu_get_positive_insulation_resistance() / 10);

    //负极绝缘阻值
    data_save_buffer_write_int((INT16U) bcu_get_negative_insulation_resistance() / 10);

    //系统绝缘阻值
    data_save_buffer_write_int((INT16U) bcu_get_system_insulation_resistance() / 10);

    //充电次数
    data_save_buffer_write_int(config_get(kCycleCntIndex));

    //国标充电故障码
    DATA_SAVE_SAFE_GET(index, guobiao_charger_get_selfcheck_fault_num());
    data_save_buffer_write_int(index);
        
    //充电故障码
    DATA_SAVE_SAFE_GET(index, bms_relay_diagnose_get_fault_num(kRelayTypeCharging));
    data_save_buffer_write_int(index);
    
    //放电故障码
    DATA_SAVE_SAFE_GET(index, bms_relay_diagnose_get_fault_num(kRelayTypeDischarging));
    data_save_buffer_write_int(index);
    
    //从机个数
    data_save_buffer_write_int(slave_num);

    //从机电池数
    for (i=0; i<slave_num; i++)
    {
        data_save_buffer_write_int(bmu_get_voltage_num(i));
    }
    //从机温感数
    for (i=0; i<slave_num; i++)
    {
        data_save_buffer_write_int(bmu_get_temperature_num(i));
    }
    //加热片温度数 
    for (i=0; i<slave_num; i++)
    {
        data_save_buffer_write_int(bmu_get_heat_temperature_num(i));
    }
    //单体电压
    for (i=0; i<slave_num; i++)
    {
        for (j = 0; j < bmu_get_voltage_num(i); j++)
            data_save_buffer_write_int(bmu_get_voltage_item(i, j));
    }
    //电池温度
    for (i=0; i<slave_num; i++)
    {
        for (j = 0; j < bmu_get_temperature_num(i); j++) //单体温度
        {
            fdata = TEMPERATURE_TO_C(bmu_get_temperature_item(i, j));
            data_save_buffer_write_float(fdata, 1);
        }
    }
    //加热片温度
    for (i=0; i<slave_num; i++)
    {
        for (j = 0; j < bmu_get_heat_temperature_num(i); j++) 
        {
            fdata = TEMPERATURE_TO_C(bmu_get_heat_temperature_item(i, j));
            data_save_buffer_write_float(fdata, 1);
        }
    }
    
    for (j = 0; j < BMS_BMU_BALANCE_NUM; j++)
        data_save_buffer_write_int(bmu_get_balance_current_item(0, j));
    
    //均衡状态    
    offset = 0;balance_temp = 0;
    for (i=0; i<slave_num; i++)
    {
        voltage_num = (INT8U)bmu_get_voltage_num(i);
        for(j=0; j<voltage_num; j++)
        {
            if(j % 8 == 0)
            {
                balance_state = bmu_get_balance_state(i, j/8);
            }
            offset++;
            balance_temp >>= 1;
                
            if(balance_state & 0x01) 
            {
                balance_temp |= 0x80;
            }
            balance_state >>= 1;
            if(offset >0 && offset >= 8)
            {
                offset = 0;
                data_save_buffer_write_int(balance_temp);
                balance_temp = 0;
            }
        }
    }
    if(offset)
    {
        for(; offset<8; offset++) balance_temp >>= 1;
        
        data_save_buffer_write_int(balance_temp);
    }
    
    data_save_buffer_write_cr();

    data_save_buffer_write_eof();

    rc = F_CLOSE(&g_fo); //关闭文件
    
    g_data_save_counter++;    
    return RES_OK;
}

static Result data_save_on_status_changed(EventTypeId event_id, void* event_data, void* user_data)
{
    (void) event_id; 
    (void)event_data;
    (void)user_data ;
    g_data_save_status_is_changed = 1;
    return RES_OK;
}

static Result data_save_on_config_changing(ConfigIndex index, INT16U new_value)
{
    (void)index;
    (void)new_value;
    g_data_save_config_is_changed = 1;
    return RES_OK;
}

static FRESULT data_save_set_operation_exception(FRESULT result)
{
    FRESULT res;
    INT32U tick = get_tick_count();
    
    res = result;
    if(result == FR_EXIST) result = FR_OK;
    if(g_data_save_operation_exception != FR_OK && result == FR_OK)
    {
        if(get_interval_by_tick(g_data_save_operation_exception_tick, tick) >= config_get(kSysStatusSaveIntervalIndex)) //保证一个写周期才恢复异常
        {
            g_data_save_operation_exception_tick = tick;
            g_data_save_operation_exception = result;
        }
    }
    else
    {
        g_data_save_operation_exception_tick = tick;
        g_data_save_operation_exception = result;
    }
    return res;
}


//=======================================================

static void private_init(void) {
    g_data_save_rpage = (INT8U)((INT32U)g_data_save_buffer / 0x10000);
    sdcard_init();

    RPAGE_RESET();
    data_save_set_root_dir();
    
    event_observe((EventTypeId)kInfoEventBatteryChargeState, data_save_on_status_changed, NULL);
    event_observe((EventTypeId)kInfoEventBatteryDischargeState, data_save_on_status_changed, NULL);
    event_observe((EventTypeId)kInfoEventBatterySocState, data_save_on_status_changed, NULL);
    event_observe((EventTypeId)kInfoEventBatteryHighTemperatureState, data_save_on_status_changed, NULL);
    event_observe((EventTypeId)kInfoEventBatteryLowTemperatureState, data_save_on_status_changed, NULL);
    event_observe((EventTypeId)kInfoEventBatteryOverChargeCurrentState, data_save_on_status_changed, NULL);
    event_observe((EventTypeId)kInfoEventBatteryOverDischargeCurrentState, data_save_on_status_changed, NULL);
    event_observe((EventTypeId)kInfoEventBatteryInsulationState, data_save_on_status_changed, NULL);
    event_observe((EventTypeId)kInfoEventSlaveCommunicationState, data_save_on_status_changed, NULL);
    event_observe((EventTypeId)kInfoEventVoltageExceptionState, data_save_on_status_changed, NULL);
    event_observe((EventTypeId)kInfoEventTempExceptionState, data_save_on_status_changed, NULL);
    event_observe((EventTypeId)kInfoEventTotalVoltageState, data_save_on_status_changed, NULL);
    event_observe((EventTypeId)kInfoEventDifferenceVoltageState, data_save_on_status_changed, NULL);
    event_observe((EventTypeId)kInfoEventBatteryDifferenceTemperatureState, data_save_on_status_changed, NULL);
    
    config_register_observer(kEepromParamStart, kEepromParamEnd, data_save_on_config_changing);
    job_schedule(MAIN_JOB_GROUP, DATA_SAVE_CHECK_SDCARD_PERIODIC, data_save_task_run_check_sdcard, NULL);
}

static void private_uninit(void) {

}

static void private_start(void) {
    g_data_save_request_stopped = 0;
}

static void private_stop(void) {
    if(g_data_save_request_stopped == 0) g_data_save_request_stopped = 1;
}

static INT8U private_is_stop(void) {
    if(g_data_save_request_stopped == 2 &&
        (g_data_save_config_is_changed == 0 || data_save_is_sd_fault() != FR_OK)) //保证修改的参数被存储
        return TRUE;
    
    return FALSE;
}

static DataSaveStatus private_get_status(void) {
    return g_data_save_status;
}

static INT32U private_get_counter(void) {
    return g_data_save_counter;
}

static INT8U private_is_fault(void) {
    if(g_data_save_status == kDataSaveStatusNotAvaliable) return FR_NOT_READY; //未检测到SD卡
    return (INT8U)g_data_save_operation_exception; //SD卡操作异常
}

static void private_task(void *pdata) {
    #if 1
    INT8U sdcard_status;
    //DEBUG("data", "data_save_task_run_save_data, mount, %X", &sdcard_status);
    
    (void)pdata;
    RPAGE_RESET();
    F_MOUNT(&g_fat_fs, g_fat_path, 1); //文件系统注册，实际上不对任何底层的硬件进行访问
    sleep(3000);
    
    for (;;)
    {
        sdcard_status = (INT8U)sdcard_is_available();

        if (g_data_save_last_sdcard_status != sdcard_status)
        {
            g_data_save_last_sdcard_status = sdcard_status;
            if (sdcard_status) // 重新插卡
            {
                //DEBUG("data", "The sdcard is available");
                //TF_RE_CONNECT();
                g_data_save_status = kDataSaveStatusNormal; //自动清除stopped状态
            }
            else
            {
                //TF_DISCONNECT();
                sleep(TF_CHIP_CONNECT_DELAY);
            }
        }

        /** 检查用户是否请求停止数据记录 */
        if (g_data_save_request_stopped)
        {
            g_data_save_status = kDataSaveStatusStopped;
            g_data_save_request_stopped = 2;
        }
        else if(sdcard_status == 0)
        {
            g_data_save_status = kDataSaveStatusNotAvaliable;
        }
        else
        {
            if(g_data_save_last_sdcard_status != g_data_save_status)
                g_data_save_status = g_data_save_last_sdcard_status;
        }

        /** 记录数据 */
        if (kDataSaveStatusNormal == g_data_save_status)
        {
            data_save_save();
            //DEBUG("sdcard", "data_save_save");
        }

        //DEBUG("data", "sdcard_is_available:%d, save_status:%d", sdcard_is_available(), g_data_save_status);

        sleep(DATA_SAVE_CHECK_SAVE_PERIODIC);
    }
    #endif
}

const struct DataSaveHandler data_save_handler_sdcard = {
    private_task,
    private_init,
    private_uninit,
    private_start,
    private_stop,
    private_is_stop,
    private_get_status,
    private_get_counter,
    private_is_fault,
};
