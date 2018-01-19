#if 1
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "stdint.h"
//#include "spi.h"
#include "ucos_ii.h"
#include "spi_interface.h"
#include "nandflash_hardware.h"
#include "nandflash_interface.h"
#include "nandflash_interface_platform.h"
//#include "storage_al.h" 
#include "nandflash_intermediate.h"       
#include "bms_bcu.h"


#pragma push
#pragma DATA_SEG __RPAGE_SEG  PAGED_RAM_NAND// +: shell not show; -: normal mode bank overflow
static const struct hcs12_gpio cs_pin = {&PTIS, &PTS, &DDRS, 1 << 7 };

static const struct hcs12_spi_platform this_hcs12_spi = {
    (struct hcs12_spi_regs *)0xD8,  // SPI1
    (0 << 4) | (0 << 0), //baud_reg_value
    1,
    &cs_pin,
};

#define this_spi this_hcs12_spi
#define this_spi_ops hcs12_spi_ops

static const struct spi_bus this_spi_bus = 
{
  &this_spi, &this_spi_ops
};

static struct spi_nand_ctx nand_ctx = {
    0,
    SPI_FLAG_CLK_IDLE_LOW | SPI_FLAG_CLK_FIRST_EDGE,
    NULL
};

static const struct spi_flash_platform nand_platform = {
    &nand_ctx,
    &this_spi_bus,
};
static const struct spi_flash nand = {
    NULL,
    &spi_nand_ops,
    &nand_platform
};

static uint8_t cache[512];
static uint8_t spare[4];
static struct StorageContext storage_ctx = { 
    cache,
    spare,
};
static const struct StorageObject storage_obj = {
    &nand,
    64,
    1023,
    &storage_ctx
};

#pragma pop


    
static int dump_info(void) {
    const struct spi_nand_info *__FAR info = spi_flash_get_chipinfo(&nand);
    if (info == NULL) {
        return -1;
    }

    (void)printf("SPI NAND %s:\n", info->part_num);
    (void)printf("  id %08X\n", info->id);
    (void)printf("  bytes_per_page %d\n", info->bytes_per_page);
    (void)printf("  bytes_per_spare %d\n", info->bytes_per_spare);
    (void)printf("  pages_per_block %d\n", info->pages_per_block);
    (void)printf("  blocks_per_chip %d\n", info->blocks_per_chip);
    return 0;
}
     
#pragma push
#pragma DATA_SEG __RPAGE_SEG PAGED_RAM_NAND
static uint8_t page[2112]; 
#pragma pop

static int dump_page(uint32_t page_addr) {
    const struct spi_nand_info *__FAR info = spi_flash_get_chipinfo(&nand);
    uint16_t i;

    if(spi_flash_read_page(&nand, page_addr)) {
        return -1;
    }
    if (info->bytes_per_page  != spi_flash_read_page_data(&nand, 0, page, info->bytes_per_page)) {
        return -1;
    }

    for (i = 0; i < info->bytes_per_page;) {
        (void)printf("%02X ", page[i++]);
        if((i & 0xF) == 0) {
            (void)printf("\n");
        }
    }
    if(info->bytes_per_spare != spi_flash_read_page_data(&nand, info->bytes_per_page, page, info->bytes_per_spare)) {
        return -1;
    }
    for(i = 0; i < info->bytes_per_spare;) {
        (void)printf("%02X ", page[i++]);
        if((i & 0xF) == 0) {
            (void)printf("\n");
        }        
    }

    (void)printf("\n");
    return 0;
}

static int dump_test_data(uint32_t page_addr) {
    const struct spi_nand_info *__FAR info = spi_flash_get_chipinfo(&nand);
    uint16_t i;

    if(spi_flash_read_page(&nand, page_addr)) {
        return -1;
    }
    if ((info->bytes_per_page+64)!= spi_flash_read_page_data(&nand, 0, page, (info->bytes_per_page+64))) {
        return -1;
    }
if((page[2048]==(uint8_t)0xFF)&&(page[2049]==(uint8_t)0xFF)) {    
    for (i = 0; i < 256;) {
        (void)printf("%02X ", page[i++]);
        if((i & 0xF) == 0) {
            (void)printf("\n");
        }
    } 
}
else 
{    
   (void)printf("bad block\n");
        
}
  //(void)printf("%02X ", page[2048]);
  //(void)printf("%02X ", page[2049]);
    /*if(info->bytes_per_spare != spi_flash_read_page_data(&nand, info->bytes_per_page, page, info->bytes_per_spare)) {
        return -1;
    }
    for(i = 0; i < info->bytes_per_spare;) {
        (void)printf("%02X ", page[i++]);
        if((i & 0xF) == 0) {
            (void)printf("\n");
        }        
    }*/

    (void)printf("\n");
    return 0;
}


static int rspeed_test(uint32_t page_num) 
{
    int rc = 0;
    uint32_t i;
    uint32_t start, stop;//, diff;
    uint32_t total_time = 0;
    
    const struct spi_nand_info *__FAR info = spi_flash_get_chipinfo(&nand);
start = OSTimeGet();
     for (i = 0; i < page_num; ++i) 
     {
            if(spi_flash_read_page(&nand, i)) {
                goto __out;
            }
            if (info->bytes_per_page  != spi_flash_read_page_data(&nand, 0, page, info->bytes_per_page)) {
                rc = -1;             
                goto __out;
        }
    }
    for (i = 0; i < page_num; ++i) {
            if(spi_flash_read_page(&nand, i)) {
                goto __out;
            }
            if (info->bytes_per_spare  != spi_flash_read_page_data(&nand, info->bytes_per_page, page, info->bytes_per_spare)) {
                rc = -1;
                goto __out;
        }
    }
stop = OSTimeGet();
total_time = stop - start; 
(void)printf("read page %ld %ldms\n", page_num, total_time);
__out:
    
    return rc;
}

static int wspeed_test(uint32_t page_num) {
    int rc = 0;
    uint32_t i;
    uint16_t bytes;
    uint32_t start, stop;//, diff;
    uint32_t total_time = 0;

    const struct spi_nand_info *__FAR info = spi_flash_get_chipinfo(&nand);
    for (i = 0; i < sizeof(page); ++i) {
        page[i] = (uint8_t)i;
    }
start = OSTimeGet();
     for (i = 0; i < page_num; ++i) {
            if ((i % info->pages_per_block) == 0) {
                if (spi_flash_erase_block(&nand, i)) {
                    rc = -1;
                    return -1;               
                }
            }
            bytes = spi_flash_write_page_data(&nand, 0, page, info->bytes_per_page);
            if(spi_flash_write_page(&nand, i)) {
                return -2;
            }
            if (info->bytes_per_page != bytes) {
                rc = -1;
                return -1;
        }
    }
stop = OSTimeGet();
total_time = stop - start;
(void)printf("write page %ld %ldms\n", page_num, total_time);
    return rc;
}

static int write_record(const char *pdata) {
    return storage_save(&storage_obj, 1, (uint8_t *__FAR )pdata, strlen(pdata));
}

static int sync_record(void) {
    return storage_flush(&storage_obj);
}

static int init_record(void) {
    return storage_init(&storage_obj);
}

const char shell_help_nand[] = "\
    info\n\
        打印NAND相关信息\n\
    dump page_addr\n\
        输出一页的内容\n\
    rspeed page_num\n\
       写入速度测试\n\
    wspeed page_num\n\
        读取速度测试\n\
    erase  all\n\
        整片擦除\n\
    erase page_num\n\
        选页擦除\n\
";

const char shell_summary_nand[] = "SPI NAND相关";

extern void print_arg_err_hint(char *arg);
int shell_func_nand(int argc, char **argv) {
    if (argc == 2) {
        if (strcmp(argv[1], "info") == 0) {
            return dump_info();
        } else if (strcmp(argv[1], "sync") == 0) {
            return sync_record();
        } else if (strcmp(argv[1], "init") == 0) {
            return init_record();
        }
    } else if (argc == 3) {
        if (strcmp(argv[1], "dump") == 0) {
            return dump_page(atoi(argv[2]));
        } 
        else if (strcmp(argv[1], "dump_test") == 0) {
            return dump_test_data(atoi(argv[2]));
        } 
        else if (strcmp(argv[1], "rspeed") == 0) {
            return rspeed_test(atoi(argv[2]));
        } else if (strcmp(argv[1], "wspeed") == 0) {
            return wspeed_test(atoi(argv[2]));
        } else if (strcmp(argv[1], "write") == 0) {
            return write_record(argv[2]);
        } else if (strcmp(argv[1], "erase") == 0) {
            uint16_t i;
            uint16_t first_block = storage_obj.first_block;
            uint16_t last_block = storage_obj.last_block;
            struct spi_nand_info *__FAR info = (struct spi_nand_info *__FAR)spi_flash_get_chipinfo(storage_obj.flash);
            if (strcmp(argv[2], "all") == 0) {
                // nothing
            } else {
                uint16_t tmp = (uint16_t)strtol(argv[2], NULL, 0);
                if (tmp < first_block) {
                    return -1;
                }
                if(tmp > last_block) {
                    return -2;
                }
                first_block = 0;
                last_block = tmp;
            }
            for (i = first_block; i <= last_block; i++) {
                (void)spi_flash_erase_block(storage_obj.flash, i * info->pages_per_block);
            }
            return 0;
        }
    }

    print_arg_err_hint(argv[0]);
    return -1;
}

#endif


void test_nand_init(void) 
{
    if(hardware_io_revision_get() >= HW_VER_116)
    {
        (void)spi_flash_init(&nand);
    }
    else
    {
        (void)printf("NO nand support!\n");
    }
}