/**
  * @file       logic_ctrl_process_start.c
  * @brief      流程启动
  *	@copyright	Ligoo Inc.
  *	@date       2017.03.17
  *	@author     Ligoo软件逻辑组
  */
#include "logic_ctrl_process_start.h"
#include "process_mgmt.h"  
#include "charge_heat_on_process_create.h"
#include "charge_cool_on_process_create.h"
#include "charge_off_process_create.h"
#include "charge_on_process_create.h"
#include "discharge_off_process_create.h"
#include "discharge_on_process_create.h"
#include "precharge_process_create.h"
#include "charge_heat_off_process_create.h"
#include "charge_cool_off_process_create.h"

const  ProcessAddrNameMap  main_process_addr_name_map[MAIN_PROCESS_MAX_NUM] = 
{

     charge_off_process,       "charge_off_process",
     charge_on_process,        "charge_on_process",
     discharge_off_process,    "discharge_off_process",
     discharge_on_process,     "discharge_on_process",
     charge_heat_on_process,   "charge_heat_on_process",
     charge_heat_off_process, "charge_heat_off_process",
     charge_cool_on_process,   "charge_cool_on_process",
     charge_cool_off_process, "charge_cool_off_process",

};
const  ProcessAddrNameMap  sub_process_addr_name_map[SUB_PROCESS_MAX_NUM] = 
{
     precharge_process,       "precharge_process"
};

/**
  * @brief  启动所有流程
  */
void start_all_logic_ctrl_process(void)
{
    start_charge_on_process();
    start_charge_off_process();
    start_discharge_on_process();
    start_discharge_off_process();
    start_heat_on_process();
    start_heat_off_process();
    start_cool_off_process();
    start_cool_on_process();
}

INT32U relay_on_outtime(void)
{
    return 0;
}

INT32U relay_off_outtime(void)
{
    return 0;
}

