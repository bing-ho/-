#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "stdint.h"
#include "bms_config.h"
#include "ex_string.h"

const char shell_help_uuid[] = "\
    [uuid]\n\
    ÉèÖÃ»ò¶ÁÈ¡UUID\
";

const char shell_summary_uuid[] =
    "UUID";

static uint8_t bt_hex_str_to_byte(const char *hex) {
    char c;
    uint8_t rc;

    c = *hex++;
    if (c >= '0' && c <= '9') {
        rc = c - '0';
    } else if (c >= 'a' && c <= 'f') {
        rc = c - ('a' - 10);
    } else if (c >= 'A' && c <= 'F') {
        rc = c - ('A' - 10);
    }
    rc = rc << 4;


    c = *hex;
    if (c >= '0' && c <= '9') {
        rc += c - '0';
    } else if (c >= 'a' && c <= 'f') {
        rc += c - ('a' - 10);
    } else if (c >= 'A' && c <= 'F') {
        rc += c - ('A' - 10);
    }

    return rc;
}

int save_uuid(const char *dat) {
    uint8_t i;
    uint16_t data;

    for (i = 0; i < 8; ++i) {
        data = ((uint16_t)bt_hex_str_to_byte(&dat[i * 4])) << 8;
        data |=  bt_hex_str_to_byte(&dat[i * 4 + 2]);
        config_save(kUUIDPart1 + i, data);
    }
    return 0;
}


int dump_uuid() {
    uint16_t data;
    uint8_t i;
    for (i = 0; i < 8; ++i) {
        data = config_get(kUUIDPart1 + i);
        printf("%04X", data);
    }

    printf("\n");
    return 0;
}


extern void print_arg_err_hint(char *arg);
int shell_func_uuid(int argc, char **argv) {
    if (argc == 1) {
        return dump_uuid();
    } else if (argc == 2) {
        if (strlen(argv[1]) == 32 && ExString_IsHexString(argv[1])) {
            return save_uuid(argv[1]);
        }
    } else if (argc == 3){
        if(!strcmp("socset",argv[1])){
            config_save(kLeftCapIndex,atoi(argv[2]));
            return 0;
        } 
        else if(!strcmp("soc",argv[1])){
            (void)printf("soc:%d\n", config_get(kLeftCapIndex));
            return 0;
        }        
    }

    print_arg_err_hint(argv[0]);
    return -1;
}
