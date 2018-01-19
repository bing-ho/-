#include "shell.h"
#include "bms_base_cfg.h"
#include "app_cfg.h"

#include "bms_upper_computer_j1939.h"


static void shell_task(void *nouse) {
    (void)nouse;
    for(;;) 
    {
        shell_loop();
    }
}

void shell_impl_create_shell_task(void) {
    OSTaskCreate(shell_task, (void *) 0, (OS_STK *) &g_slave_task_stack.stack_all[SLAVE_TX_STK_SIZE + SLAVE_RX_STK_SIZE - 1],
                 SLAVECAN_TX_TASK_PRIO);
}
