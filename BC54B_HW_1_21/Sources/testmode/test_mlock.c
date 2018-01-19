/***************************************************
*
*				新增 mlock检测 2016/11/17  
*
***************************************************/
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>


#include "stdint.h"
#include "bms_mlock.h"

const char shell_help_mlock[]	= "\
 	ops \n\
		1 电磁锁锁定;\n\
		0 电磁锁解锁;\n\
";

const char shell_summary_mlock[]="\
	mlock 操作相关";

extern void print_arg_err_hint(char *arg);
int shell_func_mlock(int argc, char **argv) {
    ChgLock_ConfigType config;
    Result result;
    config.lock_time=400;
    config.unlock_time=400;
    //config.feedback_state=0;
    config.mlocktype=kLock_Type1;
    config.lock_func=mlock_drv8802_lock;
    config.unlock_func=mlock_drv8802_unlock;
    config.lock_state_feedback=mlock_drv8802_get_status; 
    config.lock_uninit=mlock_drv8802_uninit;
	result=chglock_config_set(config);
	mlock_uninit();
    if (argc == 1) {
        print_arg_err_hint(argv[0]);
        return 0;
    } else if (argc > 2) {
        print_arg_err_hint(argv[0]);
        return 0;
    } else if (argc == 2) {
		if(0 == atoi(argv[1])){
        	mlock_lock();
        	return 0;
		}else if(1 == atoi(argv[1])){
        	mlock_unlock();
        	return 0;
		}else {
			print_arg_err_hint(argv[0]);
			return 0;
		} 
    }
    print_arg_err_hint(argv[0]);
    return -1;
}


