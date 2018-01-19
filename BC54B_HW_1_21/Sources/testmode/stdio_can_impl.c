#include "bms_can.h"

#include "stdint.h"

#include "stdio_impl.h"

#include "ring_buffer.h"

#define STDIN_ID  ((INT32U)0x48 <<21)
#define STDOUT_ID ((INT32U)0x49 <<21)
#define STDERR_ID ((INT32U)0x4A <<21)


#if defined(STDIO_USE_CAN) && (STDIO_USE_CAN != 0)

#pragma DATA_SEG __RPAGE_SEG PAGED_RAM_DTU 


static CanInfo stdio_can;
static can_t stdio_context;

static CanMessage stdout_message;
static CanMessage stderr_message;

OS_EVENT *new_stdin_package;
RINGBUFFER_DEFINE_STATIC(stdin_buffer, 100);

void test_mode_heartbeat(void);

static INT16U onStdioCanOnReceive(can_t handle, CanMessage* msg, void* userdata) {
    (void)handle;
    (void)userdata;
    if (msg->id.value == STDIN_ID) {
        if (msg->len == 8 && memcmp(msg->data, "\xFFKeepON", 7) == 0) {
            test_mode_heartbeat();
            return 0;
        }
        (void)RingBuffer_TryWrite(&stdin_buffer, msg->data, msg->len);
        (void)OSSemPost(new_stdin_package);
    }
    return 0;
}

void stdio_impl_init(void) {
    new_stdin_package = OSSemCreate(0);
    stdio_can.dev = kCanDev0;
    stdio_can.bps = kCan250kBps;
    stdio_can.receive_id = (STDIN_ID >> 21);
    stdio_can.mask_id = 0x80;
    stdio_can.mode = kCanStandardMode;
    stdio_can.receive_callback = onStdioCanOnReceive;
    stdio_can.filtemode=CanFilte32;
    stdio_context = can_init(&stdio_can);
    

    stdout_message.id.value = STDOUT_ID;
    stdout_message.mask.value = 0;
    stderr_message.id.value = STDERR_ID;
    stderr_message.mask.value = 0;
}


extern byte can_get_TFLG(CanDev dev);///wllllllllllllll

void can_wait_transmit(can_t can) {
    uint8_t i = 0;
    for (;;) {
        if (++i > 50) break;
        if (can_get_TFLG(can->dev) == 7) {
            break;
        }
        OSTimeDly(4);
    }
}

static void __wait_for_sent(can_t can) {
    can_wait_transmit(can);
}

static void __flush(CanMessage *__FAR msg) {
    OS_CPU_SR cpu_sr = 0;
    OS_ENTER_CRITICAL();
    if (msg->len > 0) {
        (void)can_send(stdio_context, msg);
        msg->len = 0;
        __wait_for_sent(stdio_context);
    }
    OS_EXIT_CRITICAL();
}
//////////////////////////////////////////////////////////
static void __putchar(CanMessage *__FAR msg, char c) {
    OS_CPU_SR cpu_sr = 0;
    OS_ENTER_CRITICAL();
    msg->data[msg->len++] = c;
    if (msg->len >= sizeof(msg->data)) {
        (void)can_send(stdio_context, msg);
        msg->len = 0;
        __wait_for_sent(stdio_context);

    }
    OS_EXIT_CRITICAL();
}




void TERMIO_PutCharStderr(char c) {
    if ('\n' == c)
        __putchar(&stderr_message, '\r');
    __putchar(&stderr_message, c);
}

///////////////////////////////////////////////////////
void TERMIO_PutChar(char c) {
    if ('\n' == c)
        __putchar(&stdout_message, '\r');
    __putchar(&stdout_message, c);
}

int fflush(FILE *f) {
    if (f == stdout) {
        __flush(&stdout_message);
    }

    if (f == stderr) {
        __flush(&stdout_message);
    }
    return 1;
}


int TERMIO_GetChar(void) {
    unsigned char c;
    OS_CPU_SR cpu_sr = 0;
    for (;;) {
        OS_ENTER_CRITICAL();
        if (1 == RingBuffer_TryRead(&stdin_buffer, &c, 1)) {
            OS_EXIT_CRITICAL();
            return c;
        }
        OS_EXIT_CRITICAL();
        OSSemPend(new_stdin_package, 0, &c);
    }
}

int TERMIO_GetCharWithTimeout(uint16_t tick) {
    unsigned char c;
    OS_CPU_SR cpu_sr = 0;
    INT32U now = OSTimeGet();
    while((OSTimeGet() - now) < tick) {
        OS_ENTER_CRITICAL();
        if (1 == RingBuffer_TryRead(&stdin_buffer, &c, 1)) {
            OS_EXIT_CRITICAL();
            return c;
        }
        OS_EXIT_CRITICAL();
        OSSemPend(new_stdin_package, 5, &c);
        if (c == OS_ERR_TIMEOUT) {
            return -1;
        }
    }
}


#endif
