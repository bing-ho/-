
#include "bms_data_save_impl.h"

#pragma MESSAGE DISABLE C1825 // Indirection to different types
#pragma MESSAGE DISABLE C1860 // Pointer conversion: possible loss of data

#define DATA_SAVE_MIN_DISK_SECTORS      256 //1M ������С������
#define DATA_SAVE_REL_MIN_DISK_SECTORS  1280 //5M �ͷ���С������
#define MAX_BATTERY_PER_CLUSTER_COUNT   16

#define TF_POWER_ON_INTREVAL    1000
#define TF_POWER_OFF_INTERVAL   2000  //��⵽USB����Ͱγ�����ʱ�Ͽ��������ʱ��
#define TF_POWER_ON_DELAY       1000 //�������ϵ��ȴ��ȶ�ʱ��
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
 * �ļ�����ΪAʱ���洢A���������ֻ��BMS��һ������ʱ���������������У�����
 * @return
 */
static Result data_save_save_a_file(void);
/**
 * �ļ�����ΪBʱ���洢B�������״̬����������ĳһ״̬�ı�ʱ�ʹ洢һ�Σ����ı��򲻴洢
 * @return
 */
static Result data_save_save_b_file(void);
/**
 * �ļ�����ΪCʱ���洢C���������������������һ����洢һ��
 * @return
 */
static Result data_save_save_c_file(void);
/**
 * �ļ�����ΪDʱ���洢D�������ʵʱ��Ϣ���������Թ̶����ڣ�20s���洢���������ļ������Ƕ��٣�ʵʱ��ʱ�䡢���ڣ������ࣩ��Ӧ�洢
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
 * �ⲿ��ʹ��,���ܸ�Ϊstatic����
 */
FATFS g_fat_fs;
static FIL  g_fo;
static TCHAR g_fat_path[5] = {0};

static INT8U g_data_save_request_stopped = 0;
static INT8U g_data_save_status = kDataSaveStatusNotAvaliable;
static INT8U g_data_save_last_sdcard_status = 0;

const char g_old_root_dir[] = {"BMS"};
static char g_root_dir[25] = {0}; //VRU��Ϣ�������һ���ļ������� ע�⣺����Ϊ��

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
        rc = F_GETFREEPROOF(g_fat_path, &free_clustuters, &fs); //��ȡ���̾����Ϣ�Ϳ��дش�С
        data_save_check_disk(); //���¼��Ӳ��״̬
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
        case 0: data_save_buffer_write("����,"); break;
        case 1: data_save_buffer_write("һ������,"); break;
        case 2: data_save_buffer_write("һ�������ͷ�,"); break;
        case 3: data_save_buffer_write("��������,"); break;
        case 4: data_save_buffer_write("���������ͷ�,"); break;
        case 5: data_save_buffer_write("��������,"); break;
        case 6: data_save_buffer_write("���������ͷ�,"); break;
#if BMS_FORTH_ALARM_SUPPORT
        case 7: data_save_buffer_write("�ļ�����,"); break;
        case 8: data_save_buffer_write("�ļ������ͷ�,"); break;
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
    
    if(file->fsize < 2) //�����ļ��������ٴ�������ͷ(header)
    {
        data_save_set_operation_exception(FR_INVALID_OBJECT);
        F_UNLINK(path); return FR_INVALID_OBJECT;
    }
    
    res = F_LSEEK(file, file->fsize-2);                                        // ��ȡƫ��ָ��
    if(res != FR_OK) return res;
    
    res = F_READ(file, buf, 2, &br);
    if(res != FR_OK) return res;
    
    if ((buf[0] != '\r') || (buf[1] != '\n'))          // β�ж�
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
        rc = F_READDIR(dp, fno); //���ɨ��Ŀ¼�µ��ļ���Ϣ
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
    rc = F_OPENDIR(&dir, path); //�������ļ�Ŀ¼
    if (rc != FR_OK) return rc;

    oldfno.fdate = 0xFFFF;
    oldfno.ftime = 0xFFFF;
    for (;;) //ͳ����־Ŀ¼����־�ļ�����Ŀ���������紴�����ļ�
    {
        rc = data_save_find_file_in_dir(&dir, &fno);
        if (rc != FR_OK) break;

        if (oldfno.fdate > fno.fdate) //�������紴������־�ļ�
            oldfno = fno;
    }
    
    if (oldfno.fdate == 0xFFFF) return FR_NO_FILE;
    
    /** ƴд�ļ��� */
    strcat(path, "/");
    strcat(path, oldfno.fname); //��λ���紴�����ļ�
    
    rc = F_CLOSEDIR(&dir); //�ر������ļ�Ŀ¼
    return rc;
}

static FRESULT data_save_remove_directory(char *path)
{
    FRESULT rc=0;
    DIR dir;
    FILINFO fno;

    fno.lfname = g_long_file_name_buffer;
    fno.lfsize = DATA_SAVE_FILE_NAME_MAX;
    rc = F_OPENDIR(&dir, path); //�������ļ�Ŀ¼
    if (rc != FR_OK) return rc;

    for (;;) //ͳ����־Ŀ¼����־�ļ�����Ŀ��ȫ��ɾ��
    {
        INT8U len = 0;
        
        rc = F_READDIR(&dir, &fno); //���ɨ��Ŀ¼�µ��ļ���Ϣ
        if ((rc != FR_OK) || (fno.fname[0] == 0)) break;

        if (fno.fname[0] == '.') continue;

        /** ƴд�ļ��� */
        strcat(path, "/");
        strcat(path, fno.fname); //��λ���紴�����ļ�

        rc = F_UNLINK(path); //ɾ���ļ�
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
    rc = F_CLOSEDIR(&dir); //�ر������ļ�Ŀ¼
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
    
    //�ɴ洢��ʽ
    g_data_save_buffer[0] = 0x00;
    strcat(g_data_save_buffer, g_old_root_dir);
    rc = F_OPENDIR(&dir, g_data_save_buffer); //�򿪾������ļ�Ŀ¼
    if(rc == FR_OK)
    {
        rc = data_save_find_file_in_dir(&dir, &fno);
        if (rc == FR_OK)
        {
            strcat(g_data_save_buffer, "/");
            strcat(g_data_save_buffer, fno.fname); //��λ���紴�����ļ�
        }
        rc = F_CLOSEDIR(&dir); //�ر������ļ�Ŀ¼
        
        if (rc != FR_OK) return rc;
        
        rc = F_UNLINK(g_data_save_buffer);
    }
    else
    {
        //�´洢��ʽ
        /** ƴд�ļ��� */
        g_data_save_buffer[0] = 0x00;
        strcat(g_data_save_buffer, g_root_dir);
        rc = data_save_find_oldest_in_dir(g_data_save_buffer);
        if (rc != FR_OK) return rc;
        rc = data_save_remove_directory(g_data_save_buffer); //ɾ���ļ���
    }
    return rc;
}

static FRESULT data_save_check_disk(void)
{
    // TODO: f_getfree��sdcard�γ�ʱ���������
    FRESULT rc=0;
    FATFS *fs = NULL;
    INT8U unlink_cnt = 0;
    INT32U free_clustuters = 0; //���дش�С
    INT32U free_sectors = 0; //����������С
    RPAGE_INIT();
    
    /** ����Ŀ¼ */
    rc = F_MKDIR(g_root_dir); //����Ŀ¼
    
    /** �����̿ռ� */
    free_sectors = 0;
    rc = F_GETFREE(g_fat_path, &free_clustuters, &fs); //��ȡ���̾����Ϣ�Ϳ��дش�С
    if (rc != FR_OK) return rc;
    
    /** ������̿ռ䲻�㣬ɾ�������ļ� */
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
    
    /** �����ļ��� */
    g_root_dir[0] = '\0';
    strcat(g_root_dir, DATA_SAVE_PATH);
    strcat(g_root_dir, "_");
    for(i=0; i<5; i++) //17���ַ�
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
    //����������������־�ļ���
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
    //����������������־�ļ�����
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

/****************************A���ļ�����************************/
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

    /** ����Ŀ¼ */
    filename = data_save_get_file_dir();
    rc = F_MKDIR(filename); //����Ŀ¼
    if(rc != FR_OK && rc != FR_EXIST) return rc;
    
    /** �����ļ��� */
    filename = data_save_get_filename("A");
    rc = F_OPEN(&g_fo, filename, FA_WRITE | FA_READ | FA_CREATE_NEW); //���ļ�
    if (rc == FR_EXIST)
    {
        F_CLOSE(&g_fo);
    }

    if (rc == FR_OK)
    {
        rc = F_LSEEK(&g_fo, g_fo.fsize); //���ļ�ָ�붨λ���ļ�ĩβ
        if(rc != FR_OK)
        {
            F_CLOSE(&g_fo); return rc;
        }

        /* write the header */
        //һ��д���ַ���������С��DATA_SAVE_MAX_LINE_SIZE 
#if BMS_DATASAVE_SUPPORT_ENGLISH
        data_save_buffer_write("Date,Time,Unique code of BCU,Production Date,");  
#else
        data_save_buffer_write("��ǰ����,��ǰʱ��,����ģ��Ψһ��,ģ������ʱ��,");
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

        rc = F_CLOSE(&g_fo); //�ر��ļ�
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
    /** ����Ŀ¼ */
    filename = data_save_get_file_dir();
    rc = F_MKDIR(filename); //����Ŀ¼
    if(rc != FR_OK && rc != FR_EXIST) return rc;
    /** �����ļ��� */
    filename = data_save_get_filename("B");
    rc = F_OPEN(&g_fo, filename, FA_WRITE | FA_READ | FA_OPEN_ALWAYS); //���ļ�
    if (rc == FR_OK)
    {
        if(g_fo.fsize == 0) need_write_header = 1;
    }

    if (need_write_header)
    {//һ��д���ַ���������С��DATA_SAVE_MAX_LINE_SIZE 
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
        data_save_buffer_write("����,ʱ��,����״̬,����״̬,soc״̬,����״̬,");
        data_save_buffer_write("����״̬,������״̬,�ŵ����״̬,©��״̬,");
        data_save_buffer_write("ͨ���ж�״̬,��ѹ����쳣״̬,�¸��쳣״̬,");
        data_save_buffer_write("��ѹ��״̬,��ѹ��״̬,ѹ��״̬,�²�״̬");
#endif
        data_save_buffer_write_cr();
        data_save_buffer_write_eof();
    }

    if (rc == FR_OK) //�ɹ�����������Ҫ���µ��ļ�
    {
        if(need_write_header) filename = data_save_get_filename("B");
        rc = data_save_check_file_end(&g_fo, filename);
        if(rc != FR_OK) 
        {
            F_CLOSE(&g_fo); return rc;
        }
        
        rc = F_LSEEK(&g_fo, g_fo.fsize); //���ļ�ָ�붨λ���ļ�ĩβ
        if(rc != FR_OK)
        {
            F_CLOSE(&g_fo); return rc;
        }

        data_save_buffer_write_date(&g_data_save_time);
        data_save_buffer_write_time(&g_data_save_time);
        data_save_buffer_write_alarm(bcu_get_charge_state()); //����״̬
        data_save_buffer_write_alarm(bcu_get_discharge_state()); //����״̬
        data_save_buffer_write_alarm(bcu_get_low_soc_state()); //SOC״̬
        data_save_buffer_write_alarm(bcu_get_high_temperature_state()); //������
        data_save_buffer_write_alarm(bcu_get_low_temperature_state()); //������
        data_save_buffer_write_alarm(bcu_get_chg_oc_state()); //������
        data_save_buffer_write_alarm(bcu_get_dchg_oc_state()); //������
        data_save_buffer_write_alarm(bcu_get_battery_insulation_state()); //��Ե״̬
        data_save_buffer_write_alarm(bcu_get_slave_communication_state()); //ͨ���ж�״̬
        data_save_buffer_write_alarm(bcu_get_voltage_exception_state()); //��������쳣״̬
        data_save_buffer_write_alarm(bcu_get_temp_exception_state()); //�¸��쳣״̬
        data_save_buffer_write_alarm(bcu_get_high_total_volt_state()); //��ѹ״̬
        data_save_buffer_write_alarm(bcu_get_low_total_volt_state()); //��ѹ״̬
        data_save_buffer_write_alarm(bcu_get_delta_voltage_state()); //ѹ��״̬
        data_save_buffer_write_alarm(bcu_get_delta_temperature_state()); //�²�״̬
        data_save_buffer_write_cr();

        data_save_buffer_write_eof();

        g_data_save_status_is_changed = 0;
    }

    rc = F_CLOSE(&g_fo); //�ر��ļ�
    
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
    /** ����Ŀ¼ */
    filename = data_save_get_file_dir();
    rc = F_MKDIR(filename); //����Ŀ¼
    if(rc != FR_OK && rc != FR_EXIST) return rc;
    /** �����ļ��� */
    filename = data_save_get_filename("C");
    //memset(&g_fo, 0, sizeof(g_fo));
    rc = F_OPEN(&g_fo, filename, FA_WRITE | FA_READ | FA_OPEN_ALWAYS); //���ļ�
    if (rc == FR_OK)
    {
        if(g_fo.fsize == 0) need_write_header = 1;
    }

    if (need_write_header)
    {//һ��д���ַ���������С��DATA_SAVE_MAX_LINE_SIZE 
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
        data_save_buffer_write("Dchg 1st high temperature(��),Dchg 1st high temperature release(��),");
        data_save_buffer_write("Dchg 2st high temperature(��),Dchg 2st high temperautre release(��),");
        data_save_buffer_write("Dchg 3rd high temperature(��),Dchg 3rd high temperautre release(��),");
        data_save_buffer_write("Dchg 1st low temperature(��),Dchg 1st low temperature release(��),");
        data_save_buffer_write("Dchg 2st low temperature(��),Dchg 2st low temperautre release(��),");
        data_save_buffer_write("Dchg 3rd low temperature(��),Dchg 3rd low temperautre release(��),");
        data_save_buffer_write("Dchg 1st delta temperature(��),Dchg 1st delta temperature release(��),");
        data_save_buffer_write("Dchg 2st delta temperature(��),Dchg 2st delta temperautre release(��),");
        data_save_buffer_write("Dchg 3rd delta temperature(��),Dchg 3rd delta temperautre release(��),");
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
        data_save_buffer_write("Chg 1st high temperature(��),Chg 1st high temperature release(��),");
        data_save_buffer_write("Chg 2st high temperature(��),Chg 2st high temperautre release(��),");
        data_save_buffer_write("Chg 3rd high temperature(��),Chg 3rd high temperautre release(��),");
        data_save_buffer_write("Chg 1st low temperature(��),Chg 1st low temperature release(��),");
        data_save_buffer_write("Chg 2st low temperature(��),Chg 2st low temperautre release(��),");
        data_save_buffer_write("Chg 3rd low temperature(��),Chg 3rd low temperautre release(��),");
        data_save_buffer_write("Chg 1st delta temperature(��),Chg 1st delta temperature release(��),");
        data_save_buffer_write("Chg 2st delta temperature(��),Chg 2st delta temperautre release(��),");
        data_save_buffer_write("Chg 3rd delta temperature(��),Chg 3rd delta temperautre release(��),");
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
        data_save_buffer_write("Refrigeration starting temperature(��),");
        data_save_buffer_write("Refrigeration stopping temperature(��),");
        data_save_buffer_write("Heating starting temperature(��),");
        data_save_buffer_write("Heating stopping temperature(��)");
#else
        data_save_buffer_write("����,ʱ��,�����(AH),������(AH),ʣ������(AH),");
        data_save_buffer_write("����У׼ֵ,��������,���ŵ����,�����ͷ���ѹ,");
        data_save_buffer_write("�ŵ絥���ѹһ������ֵ(mV),�ŵ絥���ѹһ���ͷ�ֵ(mV),");
        data_save_buffer_write("�ŵ絥���ѹ��������ֵ(mV),�ŵ絥���ѹ�����ͷ�ֵ(mV),");
        data_save_buffer_write("�ŵ絥���ѹ��������ֵ(mV),�ŵ絥���ѹ�����ͷ�ֵ(mV),");
        data_save_buffer_write("�ŵ絥���ѹһ������ֵ(mV),�ŵ絥���ѹһ���ͷ�ֵ(mV),");
        data_save_buffer_write("�ŵ絥���ѹ��������ֵ(mV),�ŵ絥���ѹ�����ͷ�ֵ(mV),");
        data_save_buffer_write("�ŵ絥���ѹ��������ֵ(mV),�ŵ絥���ѹ�����ͷ�ֵ(mV),");
        data_save_buffer_write("�ŵ絥�����һ������ֵ(��),�ŵ絥�����һ���ͷ�ֵ(��),");
        data_save_buffer_write("�ŵ絥����¶�������ֵ(��),�ŵ絥����¶����ͷ�ֵ(��),");
        data_save_buffer_write("�ŵ絥�������������ֵ(��),�ŵ絥����������ͷ�ֵ(��),");
        data_save_buffer_write("�ŵ絥�����һ������ֵ(��),�ŵ絥�����һ���ͷ�ֵ(��),");
        data_save_buffer_write("�ŵ絥����¶�������ֵ(��),�ŵ絥����¶����ͷ�ֵ(��),");
        data_save_buffer_write("�ŵ絥�������������ֵ(��),�ŵ絥����������ͷ�ֵ(��),");
        data_save_buffer_write("�ŵ絥���²�һ������ֵ(��),�ŵ絥���²�һ���ͷ�ֵ(��),");
        data_save_buffer_write("�ŵ絥���²��������ֵ(��),�ŵ絥���²�����ͷ�ֵ(��),");
        data_save_buffer_write("�ŵ絥���²���������ֵ(��),�ŵ絥���²������ͷ�ֵ(��),");
        data_save_buffer_write("�ŵ絥��ѹ��һ������ֵ(mV),�ŵ絥��ѹ��һ���ͷ�ֵ(mV),");
        data_save_buffer_write("�ŵ絥��ѹ���������ֵ(mV),�ŵ絥��ѹ������ͷ�ֵ(mV),");
        data_save_buffer_write("�ŵ絥��ѹ����������ֵ(mV),�ŵ絥��ѹ�������ͷ�ֵ(mV),");
        data_save_buffer_write("�ŵ���ѹ��һ������ֵ(ƽ����ѹ��mV),�ŵ���ѹ��һ���ͷ�ֵ(mV),");
        data_save_buffer_write("�ŵ���ѹ�߶�������ֵ(mV),�ŵ���ѹ�߶����ͷ�ֵ(mV),");
        data_save_buffer_write("�ŵ���ѹ����������ֵ(mV),�ŵ���ѹ�������ͷ�ֵ(mV),");
        data_save_buffer_write("�ŵ���ѹ��һ������ֵ(ƽ����ѹ��mV),�ŵ���ѹ��һ���ͷ�ֵ(mV),");
        data_save_buffer_write("�ŵ���ѹ�Ͷ�������ֵ(mV),�ŵ���ѹ�Ͷ����ͷ�ֵ(mV),");
        data_save_buffer_write("�ŵ���ѹ����������ֵ(mV),�ŵ���ѹ�������ͷ�ֵ(mV),");
        data_save_buffer_write("�ŵ����һ������ֵ(A),�ŵ����һ���ͷ�ֵ(A),");
        data_save_buffer_write("�ŵ������������ֵ(A),�ŵ���������ͷ�ֵ(A),");
        data_save_buffer_write("�ŵ������������ֵ(A),�ŵ���������ͷ�ֵ(A),");
        data_save_buffer_write("��絥���ѹһ������ֵ(mV),��絥���ѹһ���ͷ�ֵ(mV),");
        data_save_buffer_write("��絥���ѹ��������ֵ(mV),��絥���ѹ�����ͷ�ֵ(mV),");
        data_save_buffer_write("��絥���ѹ��������ֵ(mV),��絥���ѹ�����ͷ�ֵ(mV),");
        data_save_buffer_write("��絥���ѹһ������ֵ(mV),��絥���ѹһ���ͷ�ֵ(mV),");
        data_save_buffer_write("��絥���ѹ��������ֵ(mV),��絥���ѹ�����ͷ�ֵ(mV),");
        data_save_buffer_write("��絥���ѹ��������ֵ(mV),��絥���ѹ�����ͷ�ֵ(mV),");
        data_save_buffer_write("��絥�����һ������ֵ(��),��絥�����һ���ͷ�ֵ(��),");
        data_save_buffer_write("��絥����¶�������ֵ(��),��絥����¶����ͷ�ֵ(��),");
        data_save_buffer_write("��絥�������������ֵ(��),��絥����������ͷ�ֵ(��),");
        data_save_buffer_write("��絥�����һ������ֵ(��),��絥�����һ���ͷ�ֵ(��),");
        data_save_buffer_write("��絥����¶�������ֵ(��),��絥����¶����ͷ�ֵ(��),");
        data_save_buffer_write("��絥�������������ֵ(��),��絥����������ͷ�ֵ(��),");
        data_save_buffer_write("��絥���²�һ������ֵ(��),��絥���²�һ���ͷ�ֵ(��),");
        data_save_buffer_write("��絥���²��������ֵ(��),��絥���²�����ͷ�ֵ(��),");
        data_save_buffer_write("��絥���²���������ֵ(��),��絥���²������ͷ�ֵ(��),");
        data_save_buffer_write("��絥��ѹ��һ������ֵ(mV),��絥��ѹ��һ���ͷ�ֵ(mV),");
        data_save_buffer_write("��絥��ѹ���������ֵ(mV),��絥��ѹ������ͷ�ֵ(mV),");
        data_save_buffer_write("��絥��ѹ����������ֵ(mV),��絥��ѹ�������ͷ�ֵ(mV),");
        data_save_buffer_write("�����ѹ��һ������ֵ(ƽ����ѹ��mV),�����ѹ��һ���ͷ�ֵ(mV),");
        data_save_buffer_write("�����ѹ�߶�������ֵ(mV),�����ѹ�߶����ͷ�ֵ(mV),");
        data_save_buffer_write("�����ѹ����������ֵ(mV),�����ѹ�������ͷ�ֵ(mV),");
        data_save_buffer_write("�����ѹ��һ������ֵ(ƽ����ѹ��mV),�����ѹ��һ���ͷ�ֵ(mV),");
        data_save_buffer_write("�����ѹ�Ͷ�������ֵ(mV),�����ѹ�Ͷ����ͷ�ֵ(mV),");
        data_save_buffer_write("�����ѹ����������ֵ(mV),�����ѹ�������ͷ�ֵ(mV),");
        data_save_buffer_write("������һ������ֵ(A),������һ���ͷ�ֵ(A),");
        data_save_buffer_write("��������������ֵ(A),�����������ͷ�ֵ(A),");
        data_save_buffer_write("��������������ֵ(A),�����������ͷ�ֵ(A),");
        data_save_buffer_write("���俪���¶�(��),����ر��¶�(��),");
        data_save_buffer_write("���ȿ����¶�(��),���ȹر��¶�(��)");
#endif
        data_save_buffer_write_cr();
        data_save_buffer_write_eof();
    }
    if (rc != FR_OK)
    {
        F_CLOSE(&g_fo); //�ر��ļ�
        return RES_ERROR;
    }

    g_data_save_config_is_changed = 0;
    
    if(need_write_header) filename = data_save_get_filename("C");
    rc = data_save_check_file_end(&g_fo, filename);
    if(rc != FR_OK) 
    {
        F_CLOSE(&g_fo); return rc;
    }
    
    //�ɹ�����������Ҫ���µ��ļ�
    rc = F_LSEEK(&g_fo, g_fo.fsize); //���ļ�ָ�붨λ���ļ�ĩβ
    if(rc != FR_OK)
    {
        F_CLOSE(&g_fo); return rc;
    }

    data_save_buffer_write_date(&g_data_save_time);
    data_save_buffer_write_time(&g_data_save_time);
    //�����
    data_save_buffer_write_int(config_get(kNominalCapIndex));
    //������
    data_save_buffer_write_int(config_get(kTotalCapIndex));
    //ʣ������
    data_save_buffer_write_int(config_get(kLeftCapIndex));
    //����У׼ֵ
    fdata = (INT16S)config_get(kCchkIndex);
    fdata = fdata / 10;
    data_save_buffer_write_float(fdata, 3);
    //��������
    fdata = config_get(kChgOCSndAlarmIndex);
    fdata = fdata / 10;
    data_save_buffer_write_float(fdata, 1);
    //���ŵ����
    fdata = config_get(kDChgOCSndAlarmIndex);
    fdata = fdata / 10;
    data_save_buffer_write_float(fdata, 1);
    //�����ͷ���ѹ
    data_save_buffer_write_float((float)config_get(kChgFullChgTVReleaseIndex) / 10, 1);
    //�ŵ絥���ѹһ������ֵ
    data_save_buffer_write_int(config_get(kDChgHVFstAlarmIndex));
    //�ŵ絥���ѹһ���ͷ�ֵ
    data_save_buffer_write_int(config_get(kDChgHVFstAlarmRelIndex));
    //�ŵ絥���ѹ��������ֵ
    data_save_buffer_write_int(config_get(kDChgHVSndAlarmIndex));
    //�ŵ絥���ѹ�����ͷ�ֵ
    data_save_buffer_write_int(config_get(kDChgHVSndAlarmRelIndex));
    //�ŵ絥���ѹ��������ֵ
    data_save_buffer_write_int(config_get(kDChgHVTrdAlarmIndex));
    //�ŵ絥���ѹ�����ͷ�ֵ
    data_save_buffer_write_int(config_get(kDChgHVTrdAlarmRelIndex));
    //�ŵ絥���ѹһ������ֵ
    data_save_buffer_write_int(config_get(kDChgLVFstAlarmIndex));
    //�ŵ絥���ѹһ���ͷ�ֵ
    data_save_buffer_write_int(config_get(kDChgLVFstAlarmRelIndex));
    //�ŵ絥���ѹ��������ֵ
    data_save_buffer_write_int(config_get(kDChgLVSndAlarmIndex));
    //�ŵ絥���ѹ�����ͷ�ֵ
    data_save_buffer_write_int(config_get(kDChgLVSndAlarmRelIndex));
    //�ŵ絥���ѹ��������ֵ
    data_save_buffer_write_int(config_get(kDChgLVTrdAlarmIndex));
    //�ŵ絥���ѹ�����ͷ�ֵ
    data_save_buffer_write_int(config_get(kDChgLVTrdAlarmRelIndex));
    //�ŵ絥�����һ������ֵ
    fdata = TEMPERATURE_TO_C(config_get(kDChgHTFstAlarmIndex));
    data_save_buffer_write_float(fdata, 1);
    //�ŵ絥�����һ���ͷ�ֵ
    fdata = TEMPERATURE_TO_C(config_get(kDChgHTFstAlarmRelIndex));
    data_save_buffer_write_float(fdata, 1);
    //�ŵ絥����¶�������ֵ
    fdata = TEMPERATURE_TO_C(config_get(kDChgHTSndAlarmIndex));
    data_save_buffer_write_float(fdata, 1);
    //�ŵ絥����¶����ͷ�ֵ
    fdata = TEMPERATURE_TO_C(config_get(kDChgHTSndAlarmRelIndex));
    data_save_buffer_write_float(fdata, 1);
    //�ŵ絥�������������ֵ
    fdata = TEMPERATURE_TO_C(config_get(kDChgHTTrdAlarmIndex));
    data_save_buffer_write_float(fdata, 1);
    //�ŵ絥����������ͷ�ֵ
    fdata = TEMPERATURE_TO_C(config_get(kDChgHTTrdAlarmRelIndex));
    data_save_buffer_write_float(fdata, 1);
    //�ŵ絥�����һ������ֵ
    fdata = TEMPERATURE_TO_C(config_get(kDChgLTFstAlarmIndex));
    data_save_buffer_write_float(fdata, 1);
    //�ŵ絥�����һ���ͷ�ֵ
    fdata = TEMPERATURE_TO_C(config_get(kDChgLTFstAlarmRelIndex));
    data_save_buffer_write_float(fdata, 1);
    //�ŵ絥����¶�������ֵ
    fdata = TEMPERATURE_TO_C(config_get(kDChgLTSndAlarmIndex));
    data_save_buffer_write_float(fdata, 1);
    //�ŵ絥����¶����ͷ�ֵ
    fdata = TEMPERATURE_TO_C(config_get(kDChgLTSndAlarmRelIndex));
    data_save_buffer_write_float(fdata, 1);
    //�ŵ絥�������������ֵ
    fdata = TEMPERATURE_TO_C(config_get(kDChgLTTrdAlarmIndex));
    data_save_buffer_write_float(fdata, 1);
    //�ŵ絥����������ͷ�ֵ
    fdata = TEMPERATURE_TO_C(config_get(kDChgLTTrdAlarmRelIndex));
    data_save_buffer_write_float(fdata, 1);
    //�ŵ絥���²�һ������ֵ
    fdata = config_get(kDChgDTFstAlarmIndex);
    data_save_buffer_write_float(fdata, 1);
    //�ŵ絥���²�һ���ͷ�ֵ
    fdata = config_get(kDChgDTFstAlarmRelIndex);
    data_save_buffer_write_float(fdata, 1);
    //�ŵ絥���²��������ֵ
    fdata = config_get(kDChgDTSndAlarmIndex);
    data_save_buffer_write_float(fdata, 1);
    //�ŵ絥���²�����ͷ�ֵ
    fdata = config_get(kDChgDTSndAlarmRelIndex);
    data_save_buffer_write_float(fdata, 1);
    //�ŵ絥���²���������ֵ
    fdata = config_get(kDChgDTTrdAlarmIndex);
    data_save_buffer_write_float(fdata, 1);
    //�ŵ絥���²������ͷ�ֵ
    fdata = config_get(kDChgDTTrdAlarmRelIndex);
    data_save_buffer_write_float(fdata, 1);
    //�ŵ絥��ѹ��һ������ֵ
    data_save_buffer_write_int(config_get(kDChgDVFstAlarmIndex));
    //�ŵ絥��ѹ��һ���ͷ�ֵ
    data_save_buffer_write_int(config_get(kDChgDVFstAlarmRelIndex));
    //�ŵ絥��ѹ���������ֵ
    data_save_buffer_write_int(config_get(kDChgDVSndAlarmIndex));
    //�ŵ絥��ѹ������ͷ�ֵ
    data_save_buffer_write_int(config_get(kDChgDVSndAlarmRelIndex));
    //�ŵ絥��ѹ����������ֵ
    data_save_buffer_write_int(config_get(kDChgDVTrdAlarmIndex));
    //�ŵ絥��ѹ�������ͷ�ֵ
    data_save_buffer_write_int(config_get(kDChgDVTrdAlarmRelIndex));
    //�ŵ���ѹ��һ������ֵ
    data_save_buffer_write_int(config_get(kDChgHTVFstAlarmIndex));
    //�ŵ���ѹ��һ���ͷ�ֵ
    data_save_buffer_write_int(config_get(kDChgHTVFstAlarmRelIndex));
    //�ŵ���ѹ�߶�������ֵ
    data_save_buffer_write_int(config_get(kDChgHTVSndAlarmIndex));
    //�ŵ���ѹ�߶����ͷ�ֵ
    data_save_buffer_write_int(config_get(kDChgHTVSndAlarmRelIndex));
    //�ŵ���ѹ����������ֵ
    data_save_buffer_write_int(config_get(kDChgHTVTrdAlarmIndex));
    //�ŵ���ѹ�������ͷ�ֵ
    data_save_buffer_write_int(config_get(kDChgHTVTrdAlarmRelIndex));
    //�ŵ���ѹ��һ������ֵ
    data_save_buffer_write_int(config_get(kDChgLTVFstAlarmIndex));
    //�ŵ���ѹ��һ���ͷ�ֵ
    data_save_buffer_write_int(config_get(kDChgLTVFstAlarmRelIndex));
    //�ŵ���ѹ�Ͷ�������ֵ
    data_save_buffer_write_int(config_get(kDChgLTVSndAlarmIndex));
    //�ŵ���ѹ�Ͷ����ͷ�ֵ
    data_save_buffer_write_int(config_get(kDChgLTVSndAlarmRelIndex));
    //�ŵ���ѹ����������ֵ
    data_save_buffer_write_int(config_get(kDChgLTVTrdAlarmIndex));
    //�ŵ���ѹ�������ͷ�ֵ
    data_save_buffer_write_int(config_get(kDChgLTVTrdAlarmRelIndex));
    //�ŵ����һ������ֵ
    data_save_buffer_write_int(CURRENT_100_MA_TO_A(config_get(kDChgOCFstAlarmIndex)));
    //�ŵ����һ���ͷ�ֵ
    fdata = config_get(kDChgOCFstAlarmRelIndex);
    fdata = fdata / 10;
    data_save_buffer_write_float(fdata, 1);
    //�ŵ������������ֵ
    fdata = config_get(kDChgOCSndAlarmIndex);
    fdata = fdata / 10;
    data_save_buffer_write_float(fdata, 1);
    //�ŵ���������ͷ�ֵ
    fdata = config_get(kDChgOCSndAlarmRelIndex);
    fdata = fdata / 10;
    data_save_buffer_write_float(fdata, 1);
    //�ŵ������������ֵ
    fdata = config_get(kDChgOCTrdAlarmIndex);
    fdata = fdata / 10;
    data_save_buffer_write_float(fdata, 1);
    //�ŵ���������ͷ�ֵ
    fdata = config_get(kDChgOCTrdAlarmRelIndex);
    fdata = fdata / 10;
    data_save_buffer_write_float(fdata, 1);
    //��絥���ѹһ������ֵ
    data_save_buffer_write_int(config_get(kChgHVFstAlarmIndex));
    //��絥���ѹһ���ͷ�ֵ
    data_save_buffer_write_int(config_get(kChgHVFstAlarmRelIndex));
    //��絥���ѹ��������ֵ
    data_save_buffer_write_int(config_get(kChgHVSndAlarmIndex));
    //��絥���ѹ�����ͷ�ֵ
    data_save_buffer_write_int(config_get(kChgHVSndAlarmRelIndex));
    //��絥���ѹ��������ֵ
    data_save_buffer_write_int(config_get(kChgHVTrdAlarmIndex));
    //��絥���ѹ�����ͷ�ֵ
    data_save_buffer_write_int(config_get(kChgHVTrdAlarmRelIndex));
    //��絥���ѹһ������ֵ
    data_save_buffer_write_int(config_get(kChgLVFstAlarmIndex));
    //��絥���ѹһ���ͷ�ֵ
    data_save_buffer_write_int(config_get(kChgLVFstAlarmRelIndex));
    //��絥���ѹ��������ֵ
    data_save_buffer_write_int(config_get(kChgLVSndAlarmIndex));
    //��絥���ѹ�����ͷ�ֵ
    data_save_buffer_write_int(config_get(kChgLVSndAlarmRelIndex));
    //��絥���ѹ��������ֵ
    data_save_buffer_write_int(config_get(kChgLVTrdAlarmIndex));
    //��絥���ѹ�����ͷ�ֵ
    data_save_buffer_write_int(config_get(kChgLVTrdAlarmRelIndex));
    //��絥�����һ������ֵ
    fdata = TEMPERATURE_TO_C(config_get(kChgHTFstAlarmIndex));
    data_save_buffer_write_float(fdata, 1);
    //��絥�����һ���ͷ�ֵ
    fdata = TEMPERATURE_TO_C(config_get(kChgHTFstAlarmRelIndex));
    data_save_buffer_write_float(fdata, 1);
    //��絥����¶�������ֵ
    fdata = TEMPERATURE_TO_C(config_get(kChgHTSndAlarmIndex));
    data_save_buffer_write_float(fdata, 1);
    //��絥����¶����ͷ�ֵ
    fdata = TEMPERATURE_TO_C(config_get(kChgHTSndAlarmRelIndex));
    data_save_buffer_write_float(fdata, 1);
    //��絥�������������ֵ
    fdata = TEMPERATURE_TO_C(config_get(kChgHTTrdAlarmIndex));
    data_save_buffer_write_float(fdata, 1);
    //��絥����������ͷ�ֵ
    fdata = TEMPERATURE_TO_C(config_get(kChgHTTrdAlarmRelIndex));
    data_save_buffer_write_float(fdata, 1);
    //��絥�����һ������ֵ
    fdata = TEMPERATURE_TO_C(config_get(kChgLTFstAlarmIndex));
    data_save_buffer_write_float(fdata, 1);
    //��絥�����һ���ͷ�ֵ
    fdata = TEMPERATURE_TO_C(config_get(kChgLTFstAlarmRelIndex));
    data_save_buffer_write_float(fdata, 1);
    //��絥����¶�������ֵ
    fdata = TEMPERATURE_TO_C(config_get(kChgLTSndAlarmIndex));
    data_save_buffer_write_float(fdata, 1);
    //��絥����¶����ͷ�ֵ
    fdata = TEMPERATURE_TO_C(config_get(kChgLTSndAlarmRelIndex));
    data_save_buffer_write_float(fdata, 1);
    //��絥�������������ֵ
    fdata = TEMPERATURE_TO_C(config_get(kChgLTTrdAlarmIndex));
    data_save_buffer_write_float(fdata, 1);
    //��絥����������ͷ�ֵ
    fdata = TEMPERATURE_TO_C(config_get(kChgLTTrdAlarmRelIndex));
    data_save_buffer_write_float(fdata, 1);
    //��絥���²�һ������ֵ
    fdata = config_get(kChgDTFstAlarmIndex);
    data_save_buffer_write_float(fdata, 1);
    //��絥���²�һ���ͷ�ֵ
    fdata = config_get(kChgDTFstAlarmRelIndex);
    data_save_buffer_write_float(fdata, 1);
    //��絥���²��������ֵ
    fdata = config_get(kChgDTSndAlarmIndex);
    data_save_buffer_write_float(fdata, 1);
    //��絥���²�����ͷ�ֵ
    fdata = config_get(kChgDTSndAlarmRelIndex);
    data_save_buffer_write_float(fdata, 1);
    //��絥���²���������ֵ
    fdata = config_get(kChgDTTrdAlarmIndex);
    data_save_buffer_write_float(fdata, 1);
    //��絥���²������ͷ�ֵ
    fdata = config_get(kChgDTTrdAlarmRelIndex);
    data_save_buffer_write_float(fdata, 1);
    //��絥��ѹ��һ������ֵ
    data_save_buffer_write_int(config_get(kChgDVFstAlarmIndex));
    //��絥��ѹ��һ���ͷ�ֵ
    data_save_buffer_write_int(config_get(kChgDVFstAlarmRelIndex));
    //��絥��ѹ���������ֵ
    data_save_buffer_write_int(config_get(kChgDVSndAlarmIndex));
    //��絥��ѹ������ͷ�ֵ
    data_save_buffer_write_int(config_get(kChgDVSndAlarmRelIndex));
    //��絥��ѹ����������ֵ
    data_save_buffer_write_int(config_get(kChgDVTrdAlarmIndex));
    //��絥��ѹ�������ͷ�ֵ
    data_save_buffer_write_int(config_get(kChgDVTrdAlarmRelIndex));
    //�����ѹ��һ������ֵ
    data_save_buffer_write_int(config_get(kChgHTVFstAlarmIndex));
    //�����ѹ��һ���ͷ�ֵ
    data_save_buffer_write_int(config_get(kChgHTVFstAlarmRelIndex));
    //�����ѹ�߶�������ֵ
    data_save_buffer_write_int(config_get(kChgHTVSndAlarmIndex));
    //�����ѹ�߶����ͷ�ֵ
    data_save_buffer_write_int(config_get(kChgHTVSndAlarmRelIndex));
    //�����ѹ����������ֵ
    data_save_buffer_write_int(config_get(kChgHTVTrdAlarmIndex));
    //�����ѹ�������ͷ�ֵ
    data_save_buffer_write_int(config_get(kChgHTVTrdAlarmRelIndex));
    //�����ѹ��һ������ֵ
    data_save_buffer_write_int(config_get(kChgLTVFstAlarmIndex));
    //�����ѹ��һ���ͷ�ֵ
    data_save_buffer_write_int(config_get(kChgLTVFstAlarmRelIndex));
    //�����ѹ�Ͷ�������ֵ
    data_save_buffer_write_int(config_get(kChgLTVSndAlarmIndex));
    //�����ѹ�Ͷ����ͷ�ֵ
    data_save_buffer_write_int(config_get(kChgLTVSndAlarmRelIndex));
    //�����ѹ����������ֵ
    data_save_buffer_write_int(config_get(kChgLTVTrdAlarmIndex));
    //�����ѹ�������ͷ�ֵ
    data_save_buffer_write_int(config_get(kChgLTVTrdAlarmRelIndex));
    //������һ������ֵ
    fdata = config_get(kChgOCFstAlarmIndex);
    fdata = fdata / 10;
    data_save_buffer_write_float(fdata, 1);
    //������һ���ͷ�ֵ
    fdata = config_get(kChgOCFstAlarmRelIndex);
    fdata = fdata / 10;
    data_save_buffer_write_float(fdata, 1);
    //��������������ֵ
    fdata = config_get(kChgOCSndAlarmIndex);
    fdata = fdata / 10;
    data_save_buffer_write_float(fdata, 1);
    //�����������ͷ�ֵ
    fdata = config_get(kChgOCSndAlarmRelIndex);
    fdata = fdata / 10;
    data_save_buffer_write_float(fdata, 1);
    //��������������ֵ
    fdata = config_get(kChgOCTrdAlarmIndex);
    fdata = fdata / 10;
    data_save_buffer_write_float(fdata, 1);
    //�����������ͷ�ֵ
    fdata = config_get(kChgOCTrdAlarmRelIndex);
    fdata = fdata / 10;
    data_save_buffer_write_float(fdata, 1);
    //���俪���¶�
    fdata = TEMPERATURE_TO_C(config_get(kCoolOnTemperature));
    data_save_buffer_write_float(fdata, 1);
    //����ر��¶�
    fdata = TEMPERATURE_TO_C(config_get(kCoolOffTemperature));
    data_save_buffer_write_float(fdata, 1);
    // ���ȿ����¶�
    fdata = TEMPERATURE_TO_C(config_get(kHeatOnTemperature)); 
    data_save_buffer_write_float(fdata, 1);
    //���ȹر��¶�
    fdata = TEMPERATURE_TO_C(config_get(kHeatOffTemperature)); 
    data_save_buffer_write_float(fdata, 1);
    
    data_save_buffer_write_cr();
    data_save_buffer_write_eof();
    rc = F_CLOSE(&g_fo); //�ر��ļ�
    
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

    /** ����Ŀ¼ */
    filename = data_save_get_file_dir();
    rc = F_MKDIR(filename); //����Ŀ¼
    if(rc != FR_OK && rc != FR_EXIST) return rc;
    /** �����ļ��� */
    filename = data_save_get_filename("D");

    rc = F_OPEN(&g_fo, filename, FA_WRITE | FA_READ | FA_OPEN_ALWAYS); //���ļ�
    if (rc == FR_OK)
    {
        if(g_fo.fsize == 0) need_write_header = 1;
    }
    
    voltage_num = bcu_get_voltage_num();
    bat_temperature_num = bcu_get_temperature_num();         //����¸���
    heat_temperature_num = bcu_get_heat_temperature_num();   //����Ƭ�¸���
    
    if (need_write_header)
    {
#if BMS_DATASAVE_SUPPORT_ENGLISH
        data_save_buffer_write("Life,Date,Time,SOC(%),SOH(%),Total voltage(V),Current(A),");                     
        data_save_buffer_write("Lead-acid voltage(V),Charger supply voltage(V),Power trigger source,");
        data_save_buffer_write("Onboard temperature1(��),Onboard temperature2(��),");
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
        //����¸���
        for (index = 0; index < slave_num; index++)
        {
            data_save_buffer_write(STRING_BMU);
            int_to_bcd_string(index+1, g_data_save_bcd);
            data_save_buffer_write(g_data_save_bcd);
            data_save_buffer_write("temperature number,");
        }
        //����Ƭ�¸���
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
        //����¶� 
        for (index = 0; index < temperature_num; index++)
        {
            data_save_buffer_write("Temperature");
            int_to_bcd_string(index + 1, g_data_save_bcd);
            data_save_buffer_write(g_data_save_bcd);
            data_save_buffer_write("(��),");
        }
        //����Ƭ�¶�  
        for (index = 0; index < heat_temperature_num; index++)
        {
            data_save_buffer_write("Heat Temperature");
            int_to_bcd_string(index + 1, g_data_save_bcd);
            data_save_buffer_write(g_data_save_bcd);
            data_save_buffer_write("(��),");
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
        data_save_buffer_write("���,����,ʱ��,");
        data_save_buffer_write("SOC(%),SOH(%),��ѹ(V),����(A),");
        data_save_buffer_write("Ǧ���ѹ(V),���������ѹ(V),��Դ�����ź�,");
        data_save_buffer_write("�����¶�1(��),�����¶�2(��),");
        data_save_buffer_write("��Եģ��������ֵ(kOhm),��Եģ�鸺����ֵ(kOhm),");
        data_save_buffer_write("ϵͳ��Ե��ֵ(kOhm),������,�����������,");
        data_save_buffer_write("��������,�ŵ������,�ӻ�����,");
        
        for (index = 0; index < slave_num; index++)
        {
            data_save_buffer_write("�ӻ�");
            int_to_bcd_string(index+1, g_data_save_bcd);
            data_save_buffer_write(g_data_save_bcd);
            data_save_buffer_write("�����,");
        }
        //����¸���
        for (index = 0; index < slave_num; index++)
        {
            data_save_buffer_write("�ӻ�");
            int_to_bcd_string(index+1, g_data_save_bcd);
            data_save_buffer_write(g_data_save_bcd);
            data_save_buffer_write("����¸���,");
        }
        //����Ƭ�¸���
        for (index = 0; index < slave_num; index++)
        {
            data_save_buffer_write("�ӻ�");
            int_to_bcd_string(index+1, g_data_save_bcd);
            data_save_buffer_write(g_data_save_bcd);
            data_save_buffer_write("����Ƭ�¸���,");
        }

        for (index = 0; index < voltage_num; index++)
        {
            data_save_buffer_write("��ѹ");
            int_to_bcd_string(index + 1, g_data_save_bcd);
            data_save_buffer_write(g_data_save_bcd);
            data_save_buffer_write("(mV),");
        }
        //����¶�  
        for (index = 0; index < bat_temperature_num; index++)
        {
            data_save_buffer_write("����¶�");
            int_to_bcd_string(index + 1, g_data_save_bcd);
            data_save_buffer_write(g_data_save_bcd);
            data_save_buffer_write("(��),");
        }
        //����Ƭ�¶� 
        for (index = 0; index < heat_temperature_num; index++)
        {
            data_save_buffer_write("����Ƭ�¶�");
            int_to_bcd_string(index + 1, g_data_save_bcd);
            data_save_buffer_write(g_data_save_bcd);
            data_save_buffer_write("(��),");
        }

        for (index = 0; index < BMS_BMU_BALANCE_NUM; index++)
        {
            data_save_buffer_write("�������");
            int_to_bcd_string(index + 1, g_data_save_bcd);
            data_save_buffer_write(g_data_save_bcd);
            data_save_buffer_write("(A),");
        }
        
        for (index = 0; index < ((voltage_num + 7) / 8); index++)
        {
            sprintf(g_data_save_bcd, "%d-%d�ھ���״̬,", index*8+1, (index*8+8)<=voltage_num?(index*8+8):voltage_num);
            data_save_buffer_write(g_data_save_bcd);
        }
#endif
        data_save_buffer_write("\r\n");

        data_save_buffer_write_eof();

    }

    if (rc != FR_OK)
    {
        F_CLOSE(&g_fo); //�ر��ļ�
        //DEBUG("data", "save d class file, leave, cannot write, rc:%d", rc);
        return RES_ERROR;
    }

    if(need_write_header) filename = data_save_get_filename("D");
    rc = data_save_check_file_end(&g_fo, filename);
    if(rc != FR_OK) 
    {
        F_CLOSE(&g_fo); return rc;
    }
    
    rc = F_LSEEK(&g_fo, g_fo.fsize); //���ļ�ָ�붨λ���ļ�ĩβ
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

    //��ѹ
    data_save_buffer_write_int((INT16U) MV_TO_V(bcu_get_total_voltage()));

    //����
    fdata = bcu_get_current();
    data_save_buffer_write_float(fdata/10, 1);
    
    //Ǧ�ṩ���ѹ
    DATA_SAVE_SAFE_GET(fdata, bcu_get_lead_acid_volt());
    if(fdata < BMS_SYSTEM_VOLTAGE_LOW_THRESHOLD) fdata = 0;
    data_save_buffer_write_float(fdata / 1000, 1);
    
    //���������ѹ
    DATA_SAVE_SAFE_GET(fdata, dc_24V_voltage_get());
    if(fdata < BMS_SYSTEM_VOLTAGE_LOW_THRESHOLD) fdata = 0;
    data_save_buffer_write_float(fdata / 1000, 1);
    
    //��Դ�����ź�
    DATA_SAVE_SAFE_GET(index, bms_get_power_trigger_signal());
    data_save_buffer_write_int(index);
    
    //�����¶�1
    DATA_SAVE_SAFE_GET(fdata, TEMPERATURE_TO_C(board_temperature_get()));
    data_save_buffer_write_float(fdata, 1);
    
    //�����¶�2
    DATA_SAVE_SAFE_GET(fdata, TEMPERATURE_TO_C(board_temperature2_get()));
    data_save_buffer_write_float(fdata, 1);
    
    //������Ե��ֵ
    data_save_buffer_write_int((INT16U) bcu_get_positive_insulation_resistance() / 10);

    //������Ե��ֵ
    data_save_buffer_write_int((INT16U) bcu_get_negative_insulation_resistance() / 10);

    //ϵͳ��Ե��ֵ
    data_save_buffer_write_int((INT16U) bcu_get_system_insulation_resistance() / 10);

    //������
    data_save_buffer_write_int(config_get(kCycleCntIndex));

    //�����������
    DATA_SAVE_SAFE_GET(index, guobiao_charger_get_selfcheck_fault_num());
    data_save_buffer_write_int(index);
        
    //��������
    DATA_SAVE_SAFE_GET(index, bms_relay_diagnose_get_fault_num(kRelayTypeCharging));
    data_save_buffer_write_int(index);
    
    //�ŵ������
    DATA_SAVE_SAFE_GET(index, bms_relay_diagnose_get_fault_num(kRelayTypeDischarging));
    data_save_buffer_write_int(index);
    
    //�ӻ�����
    data_save_buffer_write_int(slave_num);

    //�ӻ������
    for (i=0; i<slave_num; i++)
    {
        data_save_buffer_write_int(bmu_get_voltage_num(i));
    }
    //�ӻ��¸���
    for (i=0; i<slave_num; i++)
    {
        data_save_buffer_write_int(bmu_get_temperature_num(i));
    }
    //����Ƭ�¶��� 
    for (i=0; i<slave_num; i++)
    {
        data_save_buffer_write_int(bmu_get_heat_temperature_num(i));
    }
    //�����ѹ
    for (i=0; i<slave_num; i++)
    {
        for (j = 0; j < bmu_get_voltage_num(i); j++)
            data_save_buffer_write_int(bmu_get_voltage_item(i, j));
    }
    //����¶�
    for (i=0; i<slave_num; i++)
    {
        for (j = 0; j < bmu_get_temperature_num(i); j++) //�����¶�
        {
            fdata = TEMPERATURE_TO_C(bmu_get_temperature_item(i, j));
            data_save_buffer_write_float(fdata, 1);
        }
    }
    //����Ƭ�¶�
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
    
    //����״̬    
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

    rc = F_CLOSE(&g_fo); //�ر��ļ�
    
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
        if(get_interval_by_tick(g_data_save_operation_exception_tick, tick) >= config_get(kSysStatusSaveIntervalIndex)) //��֤һ��д���ڲŻָ��쳣
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
        (g_data_save_config_is_changed == 0 || data_save_is_sd_fault() != FR_OK)) //��֤�޸ĵĲ������洢
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
    if(g_data_save_status == kDataSaveStatusNotAvaliable) return FR_NOT_READY; //δ��⵽SD��
    return (INT8U)g_data_save_operation_exception; //SD�������쳣
}

static void private_task(void *pdata) {
    #if 1
    INT8U sdcard_status;
    //DEBUG("data", "data_save_task_run_save_data, mount, %X", &sdcard_status);
    
    (void)pdata;
    RPAGE_RESET();
    F_MOUNT(&g_fat_fs, g_fat_path, 1); //�ļ�ϵͳע�ᣬʵ���ϲ����κεײ��Ӳ�����з���
    sleep(3000);
    
    for (;;)
    {
        sdcard_status = (INT8U)sdcard_is_available();

        if (g_data_save_last_sdcard_status != sdcard_status)
        {
            g_data_save_last_sdcard_status = sdcard_status;
            if (sdcard_status) // ���²忨
            {
                //DEBUG("data", "The sdcard is available");
                //TF_RE_CONNECT();
                g_data_save_status = kDataSaveStatusNormal; //�Զ����stopped״̬
            }
            else
            {
                //TF_DISCONNECT();
                sleep(TF_CHIP_CONNECT_DELAY);
            }
        }

        /** ����û��Ƿ�����ֹͣ���ݼ�¼ */
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

        /** ��¼���� */
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
