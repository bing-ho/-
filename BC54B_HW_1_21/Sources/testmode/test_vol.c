#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "stdint.h"

#include "bsu_sample.h"
#include "ltc68041_impl.h" 
#include "mcp3421_intermediate.h" 

const char shell_help_vol[] = "\n\
	输出电池电压,\n\
	vol read all,\n\
";

const char shell_summary_vol[] =
    "电池电压采集";



/*****************************************************************************
 *函数名称:read_vol
 *函数功能:5248D电压采集测试
 *参    数: 
 *返 回 值:无
 *修订信息:
 ******************************************************************************/
void read_vol(void)
{
    INT8U i,j;
    INT16U v_tmp;
    for(j = 0; j < 4; j++) {
      
        for(i = 0; i<12; i++)
        {
            v_tmp = get_bsu_cells_volt(j, i);
            (void)printf("vol_%d_%d:%d\n", j, i, v_tmp);
        }
    }
}

extern void print_arg_err_hint(char *arg);
int shell_func_vol(int argc, char **argv) {
    if (argc == 3) {
        read_vol();
        return 0;
    }
    
    print_arg_err_hint(argv[0]);
    return -1;
}




const char shell_help_temp[] = "\n\
	输出电池电压,\n\
	vol read all,\n\
";

const char shell_summary_temp[] =
    "电池电压采集";



/*****************************************************************************
 *函数名称:read_temp
 *函数功能:5848D温度采集测试
 *参    数: 
 *返 回 值:无
 *修订信息:
 ******************************************************************************/
void read_temp(void)
{
    INT8U i,j;
    INT16U t_tmp;
    for(j=0; j<4; j++) 
    {
        for(i = 0; i<4; i++)
        {
            t_tmp = get_bsu_cells_temp(j, i);
            (void)printf("vol_%d_%d:%d\n", j, i, t_tmp);
        }
    }
    
}
extern void print_arg_err_hint(char *arg);
int shell_func_temp(int argc, char **argv) {
    if (argc == 3) {
        read_temp();
        return 0;
    }
    
    print_arg_err_hint(argv[0]);
    return -1;
}




const char shell_help_sslave[] = "\n\
	输出电池电压,\n\
	vol read all,\n\
";

const char shell_summary_sslave[] =
    "电池电压采集";



/*****************************************************************************
 *函数名称:balance_control
 *函数功能:5248D均衡控制
 *参    数: 
 *返 回 值:无
 *修订信息:
 ******************************************************************************/
void balance_control(INT8U on)
{
    INT8U i;
    if(on){    
        for(i=0;i<4;i++)
        {
            set_ltc6804_balance_ctrl_word(i,0xFFFF);
        }
    }
    if(!on){       
        for(i=0;i<4;i++)
        {
            set_ltc6804_balance_ctrl_word(i,0x0000);
        }
    
    }
}

extern void print_arg_err_hint(char *arg);
int shell_func_sslave(int argc, char **argv) {
    if (argc == 3) {
    if(!strcmp(argv[1],"balance")){
        if(!strcmp(argv[2],"on")){      
            balance_control(1);
            return 0;  
        } 
        if(!strcmp(argv[2],"off")){
            balance_control(0);
            return 0; 
        }    
    } 
    }
    
    
    print_arg_err_hint(argv[0]);
    return -1;
}




const char shell_help_current[] = "\n\
	输出电池电压,\n\
	current read,\n\
";

const char shell_summary_current[] =
    "分流器电流获取";



void current_get(void)
{
    int current;
    Mcp3421_AvrgCurrent_Get(&current);
    (void)printf("current:%d\n", current);
}

extern void print_arg_err_hint(char *arg);
int shell_func_current(int argc, char **argv) {
    if (argc == 2) {
        current_get();
        return 0;
    }
    
    
    print_arg_err_hint(argv[0]);
    return -1;
}

