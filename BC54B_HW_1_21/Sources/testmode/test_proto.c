#include "test_proto.h"
#include "test_sd.h"
#include "test_rtc.h"

#define memcpy safe_memcpy

/// 协议版本
#define TEST_PROTO_VERSION 0x0000
#define TEST_PROTO_MESSAGE_PC_TO_DEVICE 0
#define TEST_PROTO_MESSAGE_DEVICE_TO_PC 1
#define TEST_PROTO_MESSAGE_SN_SET 0
#define TEST_PROTO_MESSAGE_SN_INC 1


enum {
    // protocol
    TEST_PROTO_TYPE_TEST_ENSURE,
    TEST_PROTO_TYPE_ECHO,

    // sdcard
    TEST_PROTO_TYPE_GET_SDINFO,
    TEST_PROTO_TYPE_SD_WRITE_SECTOR,
    TEST_PROTO_TYPE_SD_READ_SECTOR,

    // rtc
    TEST_PROTO_TYPE_SET_RTC,
    TEST_PROTO_TYPE_GET_RTC,
};

static unsigned short sn = 0;
struct test_packet {
    /// test数据包版本.
    unsigned short version;
    /// test数据包序列号
    unsigned short sn;
    /// test数据类型.
    unsigned short type;
    struct {
        /// 序列号操作; 0表示序列号自增, 1表示设置序列号.
        unsigned short sn_op: 1;
        /// 消息方向; 0表示上位机到BC52B或工装, 1表示BC52B或工装到上位机.
        unsigned short direction: 1;
    } flags;
    unsigned char dat[1]; // 测试数据内容.
};

#define TEST_PACKET_HEADER_SIZE (sizeof(struct test_packet) - 1)


typedef char (*handle_func)(void *__FAR in_data, unsigned short in_len, void *__FAR out_data, unsigned short *out_len);

struct type_handle_map {
    unsigned short type;
    handle_func func;
};

static char test_echo(const void *__FAR in_data, unsigned short in_len, void *__FAR out_data, unsigned short *out_len);

const static struct type_handle_map type_handle_maps[] = {
    {TEST_PROTO_TYPE_ECHO, test_echo},

    // sdcard
    {TEST_PROTO_TYPE_GET_SDINFO, test_sd_get_sdinfo},
    {TEST_PROTO_TYPE_SD_WRITE_SECTOR, test_sd_write_sector},
    {TEST_PROTO_TYPE_SD_READ_SECTOR, test_sd_read_sector},
    // rtc
    { TEST_PROTO_TYPE_SET_RTC, test_rtc_set_time},
    { TEST_PROTO_TYPE_GET_RTC, test_rtc_get_time},


    // end
    {0, (handle_func)0}
};


char test_proto_is_test_ensure(const struct test_packet *__FAR in_packet, unsigned short in_len) {
    if (in_packet == 0) return 0;
    if (in_len < TEST_PACKET_HEADER_SIZE) return 0;
    // 检查数据包的数据有效性

    if (in_packet->version != TEST_PROTO_VERSION) return 0;
    if (in_packet->flags.direction != TEST_PROTO_MESSAGE_PC_TO_DEVICE) return 0;

    return in_packet->type == TEST_PROTO_TYPE_TEST_ENSURE;
}


char test_proto_handle_message(
    const struct test_packet *__FAR in_packet,
    unsigned short in_len,
    struct test_packet *__FAR out_packet,
    unsigned short *out_len) {
    char ret;
    unsigned short len;
    handle_func handle;
    const struct type_handle_map *map;

    // 检查参数有消息.
    if (in_packet == 0) return 0;
    if (in_len < TEST_PACKET_HEADER_SIZE) return 0;
    if (out_packet == 0 || out_len == 0) return 0;
    if (*out_len < sizeof(struct test_packet)) return 0;

    // 检查数据包的数据有效性
    if (in_packet->version != TEST_PROTO_VERSION) return 0;
    if (in_packet->flags.direction != TEST_PROTO_MESSAGE_PC_TO_DEVICE) return 0;

    // 找到处理函数
    handle = 0;
    for (map = type_handle_maps; map->func; ++map) {
        if (in_packet->type == map->type) {
            handle = map->func;
            break;
        }
    }

    if (handle == 0) {
        return 0;
    }

    len = *out_len - TEST_PACKET_HEADER_SIZE;
    ret = handle(in_packet->dat, in_len - TEST_PACKET_HEADER_SIZE, out_packet->dat, &len);

    if (ret == 0) return 0;

    // 更新SN
    if (in_packet->flags.sn_op == TEST_PROTO_MESSAGE_SN_SET) {
        sn = in_packet->sn;
    } else {
        ++sn;
    }

    // 应答数据包.
    out_packet->version == TEST_PROTO_VERSION;
    out_packet->sn = sn;
    out_packet->flags.direction = TEST_PROTO_MESSAGE_DEVICE_TO_PC;
    *out_len = len + TEST_PACKET_HEADER_SIZE;

    return 1;
}

static char test_echo(
    const void *__FAR in_data,
    unsigned short in_len,
    void *__FAR out_data,
    unsigned short *out_len) {
    memcpy(out_data, in_data, in_len);
    *out_len = in_len;
    return 1;
}
