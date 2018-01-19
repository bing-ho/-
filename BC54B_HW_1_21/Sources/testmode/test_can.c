#include <stdio.h>
#include <stdlib.h>
#include "stdint.h"
#include "bms_can.h"
#include "bms_config.h"
#include "ucos_ii.h"



#pragma DATA_SEG __RPAGE_SEG PAGED_RAM
CanMessage test_can_buffers1[CHARGER_CAN_BUFFER_COUNT];
CanMessage test_can_buffers2[CHARGER_CAN_BUFFER_COUNT];
CanMessage test_can_buffers3[CHARGER_CAN_BUFFER_COUNT];


#pragma DATA_SEG DEFAULT


static CanDev devs[] = { kCanDev1, kCanDev2, kCanDev3 };

struct can_recv_cb_userdata {
    OS_EVENT *sem;
    CanMessage msg;
    uint8_t expect_id;
};

static INT16U can_recv_cb(can_t handle, CanMessage* msg, void* userdata) {
    struct can_recv_cb_userdata *cb_userdata = (struct can_recv_cb_userdata *)userdata;
    (void)handle;
    if (userdata == 0) {
        return 0;
    }
    
    if (((msg->id.value >> 21) & 0xff) != cb_userdata->expect_id) {//only post the msg we wanted
        return 0;
    }

    memcpy(&cb_userdata->msg, msg, sizeof(*msg));
    OSSemPost(cb_userdata->sem);
}

void test_can_init(void) {
    CanInfo info;
    info.bps = kCan250kBps;
    info.receive_id = 0x00;
    info.mask_id = 0xffffffff;
    info.mode = kCanStandardMode;
    info.buffers = test_can_buffers1;
    info.buffer_count = 5;
    info.receive_callback = can_recv_cb;
    info.receive_userdata = 0;
    info.filtemode=CanFilte32;
    info.dev = kCanDev1;
    can_init(&info);
    info.dev = kCanDev2;
    info.buffers = test_can_buffers2;
    info.buffer_count = 5;
    can_init(&info);
    info.dev = kCanDev3;
    info.buffers = test_can_buffers3;
    info.buffer_count = 5;
    can_init(&info);
}


extern int can_wait_transmit(can_t can);

static int __write_to(CanDev dev, uint8_t id, char *s) {
    uint8_t len;
    CanMessage msg;
    can_t ctx = can_get_context(dev);
    
    msg.id.value = (uint32_t)id << 21;
    msg.mask.value = 0;
    
    for (;;) {
        len = (uint8_t)strlen(s);
        if (len == 0) {
            break;
        }
        if (len > 8) {
            len = 8;
        }
        msg.len = len;
        memcpy(msg.data, s, len);
        s += len;
        
        (void)can_send(ctx, &msg);
        if (can_wait_transmit(ctx) == 0) {
            return -1;
        }
    }
    return 0;
}

static void __recv(CanDev dev, uint8_t id, uint16_t timeout) {
    OS_CPU_SR cpu_sr = 0;
    INT32U end;
    INT32U now;
    INT8U err;
    uint8_t i; 
    CanMessage tmp_msg;
    struct can_recv_cb_userdata userdata;
    can_t ctx = can_get_context(dev);
    userdata.sem = OSSemCreate(0);
    userdata.expect_id = id;
    
    OS_ENTER_CRITICAL();
    ctx->info.receive_userdata = &userdata;
    OS_EXIT_CRITICAL();
    
    
    now = OSTimeGet();
    end = now + timeout;
    
    for(; now < end; now = OSTimeGet()) {
        timeout = (uint16_t)(uint32_t)(end - now);
        OSSemPend(userdata.sem, timeout, &err);
        if (OS_ERR_NONE != err) {
            continue;
        }
        
        if (((userdata.msg.id.value >> 21) & 0xff) != id) {
            continue;
        }

        memcpy(&tmp_msg, &userdata.msg, sizeof(userdata.msg));//copy out to avoid still write to userdata.msg when in the loop of putchar
        for (i = 0; i < tmp_msg.len; ++i) {
            putchar(tmp_msg.data[i]);
        }
    }
    OS_ENTER_CRITICAL();
    ctx->info.receive_userdata = 0;
    OS_EXIT_CRITICAL();
    OSSemDel(userdata.sem, OS_DEL_ALWAYS, &err);
}


const char shell_help_can[] = "\
	dev id ops xx\n\
	dev: 设备号, 必须为1,2或3\n\
	id: 标准帧ID(十进制);\n\
	ops xx\n\
		read xx: 从CAN接收数据并打印, xx为超时时间(ms);\n\
		send xx: 通过CAN发送数据, xx为需要发送的数据;\n\
";

const char shell_summary_can[] =
    "CAN操作";

extern void print_arg_err_hint(char *arg);
int shell_func_can(int argc, char **argv) {
    uint8_t dev;
    CanDev can_dev;
    uint8_t msg_id;

    if (argc != 5) {
        goto __error_params;
    }
    if (0 != argv[1][1]) {
        goto __error_params;
    }
    
    dev = argv[1][0] - '1';
    if (dev > 2) {
        goto __error_params;
    }
    
    can_dev = devs[dev];
    msg_id = (uint8_t)atoi(argv[2]);

    if (strcmp(argv[3], "read") == 0) { // read recv buffer
        __recv(can_dev, msg_id, atoi(argv[4]));
        putchar('\n');
        return 0;
    }
    
    if (strcmp(argv[3], "send") == 0) {
        return __write_to(can_dev, msg_id, argv[4]);
    }

__error_params:
    print_arg_err_hint(argv[0]);
    return -1;

}
