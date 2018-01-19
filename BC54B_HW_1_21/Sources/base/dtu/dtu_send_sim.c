#include "dtu_m35.h"

#if 0


#pragma DATA_SEG DEFAULT

//////////////////////////////////////////////////////////////////////////
// CRC MODBUS 效验
// 输入参数: pDataIn: 数据地址
//           iLenIn: 数据长度
// 输出参数: pCRCOut: 2字节校验值


static INT16U CalcCRCModBus(char cDataIn, INT16U wCRCIn) {
    int i;
    INT16U wCheck = 0;
    wCRCIn = wCRCIn ^ cDataIn;

    for (i = 0; i < 8; i++) {
        wCheck = wCRCIn & 1;
        wCRCIn = wCRCIn >> 1;
        wCRCIn = wCRCIn & 0x7fff;

        if (wCheck == 1) {
            wCRCIn = wCRCIn ^ 0xa001;
        }
        wCRCIn = wCRCIn & 0xffff;
    }

    return wCRCIn;
}

static void CheckCRCModBus(const char *pDataIn, int iLenIn, INT16U *pCRCOut) {
    INT16U wHi = 0;
    INT16U wLo = 0;
    INT16U wCRC;
    int i;
    wCRC = 0xFFFF;


    for (i = 0; i < iLenIn; i++) {
        wCRC = CalcCRCModBus(*pDataIn, wCRC);
        pDataIn++;
    }

    wHi = wCRC / 256;
    wLo = wCRC % 256;
    wCRC = (wHi << 8) | wLo;

    *pCRCOut = wCRC;
}



#define MASTER_LEN  (54)
#define SLAVE_LEN   (11)

static char mMasterBuffer[MASTER_LEN + 2] = {
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x01, 0xFA, 0x00, 0x00,
    0x04, 0x00, 0x64, 0x00, 0x64, 0x00, 0x00,
    0x7D, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0x00, 0xFA, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static char mSlaveBuffer[SLAVE_LEN + 2] = {
    0x01, 0x00, 0x0B, 0x00, 0x00, 0x02, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

extern INT16U crc_check_bt(PINT8U msg, INT16U len);
extern INT16U crc_check(PINT8U msg, INT16U len);

void aaaa_task(void *nouse) {
    INT16U crc = 0;

    mMasterBuffer[1] = 0x00;
    mMasterBuffer[2] = 0x00;
    mMasterBuffer[3] = 0x00;
    mMasterBuffer[4] = 0x00;
    mMasterBuffer[5] = 0x00;
    mMasterBuffer[6] = 0x00;
    mMasterBuffer[7] = 0x00;
    //eb2804ef7867488e0  0000000000002
    mMasterBuffer[8] = 0x0e;
    mMasterBuffer[9] = 0xb2;
    mMasterBuffer[10] = 0x80;
    mMasterBuffer[11] = 0x4e;
    mMasterBuffer[12] = 0xf7;
    mMasterBuffer[13] = 0x86;
    mMasterBuffer[14] = 0x74;
    mMasterBuffer[15] = 0x88;
    mMasterBuffer[16] = 0xe0;


    //crc = crc_check(mMasterBuffer, MASTER_LEN);
    CheckCRCModBus(mMasterBuffer, MASTER_LEN, &crc);
    mMasterBuffer[MASTER_LEN] = (crc & 0xff00) >> 8;
    mMasterBuffer[MASTER_LEN + 1] = crc & 0x00ff;

    //crc = crc_check(mSlaveBuffer, SLAVE_LEN);
    CheckCRCModBus(mSlaveBuffer, SLAVE_LEN, &crc);
    mSlaveBuffer[SLAVE_LEN] = (crc & 0xff00) >> 8;
    mSlaveBuffer[SLAVE_LEN + 1] = crc & 0x00ff;


    while (1) {
        OSTimeDly(OS_TICKS_PER_SEC * 28);
        DTUM35_SendData(mMasterBuffer, MASTER_LEN + 2, 1);
        OSTimeDly(OS_TICKS_PER_SEC * 2);
        DTUM35_SendData(mSlaveBuffer, SLAVE_LEN + 2, 0);
    }
}

static unsigned char mTaskStack[256];

void DTU_SIMSendInit(void) {
    memset(mTaskStack, 0xFF, sizeof(mTaskStack));
    OSTaskCreate(aaaa_task, (void *)0, (OS_STK *)&mTaskStack[sizeof(mTaskStack)], SEND_SIM_TASK_PRIO);
}

#endif
