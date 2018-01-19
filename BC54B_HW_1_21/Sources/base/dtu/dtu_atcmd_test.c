#include "dtu_atcmd.h"
#include "unity_fixture.h"
#include "bms_job.h"

#pragma MESSAGE DISABLE C4001 //Condition always FALSE (do... while(0))

unsigned char cmd_buffer[100];
unsigned char cmd_buffer_index = 0;


TEST_GROUP(dtu_atcmd);

static void send_byte(unsigned char c) {
    if (cmd_buffer_index < sizeof(cmd_buffer)) {
        cmd_buffer[cmd_buffer_index++] = c;
    }
}

void send_bytes(const unsigned char *__FAR bytes, int len) {
    while(len-- > 0)send_byte(*bytes++);
}

#define CHECK_ATCMD_SENT(expect) \
    do { \
        TEST_ASSERT_EQUAL_INT8_ARRAY(expect, cmd_buffer, strlen(expect)); \
        TEST_ASSERT_EQUAL(strlen(expect), cmd_buffer_index); \
    } while(0)


TEST_SETUP(dtu_atcmd) {
    (void)memset(cmd_buffer, 0, sizeof(cmd_buffer));
    cmd_buffer_index = 0;
}


TEST_TEAR_DOWN(dtu_atcmd) {
}

extern void DTUM35_RecvByte(unsigned char b);

static void commit_at_reply_line(const char *__FAR line) {
    while (*line) {
        DTUM35_RecvByte(*line++);
    }
    DTUM35_RecvByte('\r');
    DTUM35_RecvByte('\n');
}

static void commit_at_reply(void *pp) {
    const char *__FAR const *__FAR p = (const char *__FAR const * __FAR)pp;
    if (!p) {
        return;
    }
    while (*p) {
        commit_at_reply_line(*p);
        ++p;
    }
}

TEST(dtu_atcmd, get_signal_quality) {
    JobId id;
    Bool ret;

    ATCMD_Expect exp;
    char buf[24];
    unsigned char rssi = 0;
    unsigned char ber = 0;
    static const char *__FAR const commits[] = { (const char *__FAR)"+CSQ: 28,2", (const char *__FAR)"OK", (const char *__FAR)0};

    exp.expect = "+CSQ";
    exp.recvBuffer = buf;
    exp.recvLen = sizeof(buf);

    id = job_schedule(MAIN_JOB_GROUP, BCU_UPDATE_JOB_PERIODIC, commit_at_reply, (void *)commits);
    ret = ATCMD_GetSignalQuality(&rssi, &ber);
    job_cancel(MAIN_JOB_GROUP, id);
    CHECK_ATCMD_SENT("AT+CSQ\r");
    TEST_ASSERT(ret);
    TEST_ASSERT_EQUAL(28, rssi);
    TEST_ASSERT_EQUAL(2, ber);
}

TEST(dtu_atcmd, get_cimi) {
    JobId id;
    Bool ret;
    char buf[24];
    static const char *__FAR const commits[] = {(const char *__FAR)"121212121212", (const char *__FAR)"OK", (const char *__FAR)0};

    id = job_schedule(MAIN_JOB_GROUP, BCU_UPDATE_JOB_PERIODIC, commit_at_reply, (void *)commits);
    ret = ATCMD_GetCIMI(buf, sizeof(buf));
    CHECK_ATCMD_SENT("AT+CIMI\r");
    job_cancel(MAIN_JOB_GROUP, id);
    TEST_ASSERT(ret);
    TEST_ASSERT_EQUAL_STRING(&buf[0], commits[0]);
}


TEST(dtu_atcmd, get_data_send_info) {
    JobId id;
    Bool ret;
    unsigned long sent = 0;
    unsigned long acked = 0;
    unsigned long unacked = 0;
    static const char *__FAR const commits[] = {(const char *__FAR)"+QISACK: 101, 67, 34", (const char *__FAR)"OK", (const char *__FAR)0};

    id = job_schedule(MAIN_JOB_GROUP, BCU_UPDATE_JOB_PERIODIC, commit_at_reply, (void *)commits);
    ret = ATCMD_GetDataSentInfo(&sent, &acked, &unacked);
    CHECK_ATCMD_SENT("AT+QISACK\r");
    job_cancel(MAIN_JOB_GROUP, id);
    TEST_ASSERT(ret);
    TEST_ASSERT_EQUAL(sent, 101);
    TEST_ASSERT_EQUAL(acked, 67);
    TEST_ASSERT_EQUAL(unacked, 34);
}

TEST(dtu_atcmd, get_lac_ci) {
    JobId id;
    Bool ret;
    unsigned short lac_ci[2];
    static const char *__FAR const commits[] = {(const char *__FAR)"+CREG: 2,1,\"550B\",\"5A65\"", (const char *__FAR)0};

    id = job_schedule(MAIN_JOB_GROUP, BCU_UPDATE_JOB_PERIODIC, commit_at_reply, (void *)commits);
    ret = ATCMD_GetLacCi(lac_ci);
    CHECK_ATCMD_SENT("AT+CREG?\r");
    job_cancel(MAIN_JOB_GROUP, id);
    TEST_ASSERT(ret);
    TEST_ASSERT_EQUAL(lac_ci[0], 0x550B);
    TEST_ASSERT_EQUAL(lac_ci[1], 0x5A65);
}

TEST_GROUP_RUNNER(dtu_atcmd) {
    ATCMD_RuntimeInit(send_bytes);
    RUN_TEST_CASE(dtu_atcmd, get_signal_quality);
    RUN_TEST_CASE(dtu_atcmd, get_cimi);
    RUN_TEST_CASE(dtu_atcmd, get_data_send_info);
    RUN_TEST_CASE(dtu_atcmd, get_lac_ci);
}

