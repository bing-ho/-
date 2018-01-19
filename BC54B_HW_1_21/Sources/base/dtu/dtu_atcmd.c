/// \file dtu_atcmd.c
/// \brief DTU AT命令的封装实现.
/// \author Xi Qingping, qingping.xi@ligoo.cn
/// \version
/// \date 2015-09-01

#include <string.h>
#include <stdio.h>
#include "dtu_atcmd.h"
#include "ring_buffer.h"
#include "ex_string.h"


///////////////////////////////////// for data safe ///////////////////////////
#include "bms_util.h"
#define memset safe_memset
#define memcmp safe_memcmp
#define memcpy safe_memcpy
#define strcpy safe_strcpy
#define strlen safe_strlen
#define strcmp safe_strcmp

#pragma MESSAGE DISABLE C4001 //Condition always FALSE  //do{}while(0)    
#pragma MESSAGE DISABLE C4002


//#define dprintf printf
#define dprintf


#pragma DATA_SEG __RPAGE_SEG PAGED_RAM_DTU3

#define AT_CMD_REPLY_LINE_MAX_LEN 60
RINGBUFFER_DEFINE_STATIC(mReceivedLines, AT_CMD_REPLY_LINE_MAX_LEN);

static OS_EVENT *mEvent;

#pragma DATA_SEG DEFAULT


/// \brief atcmd_SendBytes 向AT命令的串口发送指定长度的数据.
///
/// \param dat 发送的数据.
/// \param len 数据的长度.
static void (*atcmd_SendBytes)(const unsigned char *__FAR dat, int len);

static void atcmd_SendByte(unsigned char b) {
    atcmd_SendBytes(&b, 1);
}

/// \brief atcmd_ClearReply 清空接收缓冲区.
static void atcmd_ClearReply() {
    INT8U err;
    OS_CPU_SR cpu_sr = 0;
    OS_ENTER_CRITICAL();
    RingBuffer_Clear(&mReceivedLines);
    OS_EXIT_CRITICAL();
    OSSemSet(mEvent, 0, &err);
}



/// \brief atcmd_SendString 向AT命令的串口发送一个字符串.
///
/// \param s 发送的字符串指针.
static void atcmd_SendString(const char *__FAR s) {
    atcmd_SendBytes((const unsigned char *__FAR)s, strlen(s));
}



/// \brief atcmd_Expect 等待接收一个期望字符串开头的应答.
///
/// \param expect 期望字符串结构体
/// \param timeoutTick 超时时间.
///
/// \return TRUE 等到了期望的应答; FALSE 超时.
static Bool atcmd_Expect(const ATCMD_Expect *__FAR expect, INT32U timeoutTick) {
    INT8U err;
    OS_CPU_SR cpu_sr = 0;
    RINGBUFFER_SIZE_TYPE size;

    INT32U now = OSTimeGet();
    INT32U end = now + timeoutTick;
    dprintf("[AT]: Wait for event with %d ticks timeout\n", timeoutTick);

    for (; now < end; now = OSTimeGet()) {
        INT16U thisTimeout = (INT16U)((INT32U)(end - now) > 32767 ? 32767 : (end - now));
        OSSemPend(mEvent, thisTimeout, &err);
        if (err != OS_ERR_NONE) {
            return FALSE;
        }
        size = expect->recvLen;
        OS_ENTER_CRITICAL();
        if (RingBuffer_PopStringIsStartWith(&mReceivedLines,
                                            (const char *__FAR)expect->expect,
                                            (char *__FAR)expect->recvBuffer, &size)) {
            OS_EXIT_CRITICAL();
            dprintf("[AT]< \"%s\"\n", expect->recvLen > 0 ? (char *__FAR)expect->recvBuffer : expect->expect);
            return TRUE;
        }
        OS_EXIT_CRITICAL();
    }
    return FALSE;
}

void ATCMD_SerialRecvLine(unsigned char *__FAR line, INT16U len) {
    //memcpy(mReceivedLine, line, len > sizeof(mReceivedLine) ? sizeof(mReceivedLine) : len);
    (void)RingBuffer_Write(&mReceivedLines, (const unsigned char *__FAR)line, len);
    (void)OSSemPost(mEvent);
}

void ATCMD_RuntimeInit(void (*send_bytes)(const unsigned char *__FAR bytes, int len)) {
    atcmd_SendBytes = send_bytes;
    mEvent = OSSemCreate(0);
}

Bool ATCMD_ExecCommand(const char *__FAR cmd, const ATCMD_Expect *__FAR expect, INT32U timeoutTick) {
    dprintf("[AT]> \"%s\"\n", cmd ? cmd : "(none)");
    if (cmd != NULL) {
        atcmd_ClearReply();
        atcmd_SendString(cmd);
        if ((*cmd) && (strcmp(cmd, "+++") != 0)) {
            atcmd_SendByte('\r');
        }
    }

    if (expect == NULL || expect->expect == NULL) {
        while (timeoutTick > 0) {
            INT16U thisTimeout = (INT16U)(timeoutTick > 32767 ? 32767 : timeoutTick);
            OSTimeDly(thisTimeout);
            timeoutTick -= thisTimeout;
        }
        return TRUE;
    }

    return atcmd_Expect(expect, timeoutTick);
}

Bool ATCMD_RetryUntilExpect(const char *__FAR atcmd, const char *__FAR expect, INT8U times, INT16U timeout) {
    ATCMD_Expect exp;

    exp.expect = expect;
    exp.recvBuffer = NULL;
    exp.recvLen = 0;

    while (times--) {
        if (ATCMD_ExecCommand(atcmd, &exp, timeout)) {
            return TRUE;
        }
    }
    return FALSE;
}

Bool ATCMD_RetryUntilExpects(const char *__FAR atcmd, const char *__FAR const *__FAR expects, INT16U times, INT16U timeout) {
    ATCMD_Expect exp;
    char buf[24];

    exp.expect = "";
    exp.recvBuffer = buf;
    exp.recvLen = sizeof(buf);

    while (times--) {
        volatile INT32U now;
        const char *__FAR const *__FAR expect;
        INT32U end = OSTimeGet() + timeout;
        (void)ATCMD_ExecCommand(atcmd, NULL, 0);
        for (now = OSTimeGet(); now < end; now = OSTimeGet()) {
            if (!ATCMD_ExecCommand(NULL, &exp, end - now)) {
                continue;
            }

            for (expect = expects; *expect != 0; ++expect) {
                if (memcmp((unsigned char *__FAR)buf, (unsigned char *__FAR)(*expect), strlen(*expect)) == 0) {
                    return TRUE;
                }
            }
        }
    }
    return FALSE;
}

Bool ATCMD_GetCIMI(char *buf, INT8U len) {
    INT8U i;

    ATCMD_Expect expect;
    expect.expect = "";
    expect.recvBuffer = buf;
    expect.recvLen = len;

    for (i = 0; i < 5; ++i) {
        volatile INT32U now;
        INT32U end = OSTimeGet() + OS_TICKS_PER_SEC;
        (void)ATCMD_ExecCommand("AT+CCID", NULL, 0);
        for (now = OSTimeGet(); now < end;  now = OSTimeGet()) {
            if (!ATCMD_ExecCommand(NULL, &expect, end - now)) {
                continue;
            }
            if (strlen(buf)>18 && strlen(buf)<30) {
                return TRUE;
            }
        }
    } 

    return FALSE;
}
Bool ATCMD_GetIMEI(char *buf, INT8U len) {
    INT8U i;

    ATCMD_Expect expect;
    expect.expect = "";
    expect.recvBuffer = buf;
    expect.recvLen = len;

    for (i = 0; i < 5; ++i) {
        volatile INT32U now;
        INT32U end = OSTimeGet() + OS_TICKS_PER_SEC;
        (void)ATCMD_ExecCommand("AT+GSN", NULL, 0);
        for (now = OSTimeGet(); now < end;  now = OSTimeGet()) {
            if (!ATCMD_ExecCommand(NULL, &expect, end - now)) {
                continue;
            }
            if (ExString_IsHexString(buf)) {
                return TRUE;
            }
        }
    }

    return FALSE;
}


Bool ATCMD_SetCPIMode(INT8U cpi_mode) {

    if(0 == cpi_mode) 
    { 
        if (!ATCMD_RetryUntilExpect("AT+CIPMODE=0", "OK", 1, 5*OS_TICKS_PER_SEC)) {
            return FALSE;
        }	
    } 
    else if(1 == cpi_mode)
    {
        if (!ATCMD_RetryUntilExpect("AT+CIPMODE=1", "OK", 1, 5*OS_TICKS_PER_SEC)) {
            return FALSE;
        }	
    }
    else if(2 == cpi_mode)
    {
        if (!ATCMD_RetryUntilExpect("+++", "OK", 1, 5*OS_TICKS_PER_SEC)) {
            return FALSE;
        }	
    }
    else if(3 == cpi_mode)
    {
        if (!ATCMD_RetryUntilExpect("ATO", "CONNECT", 1, 10*OS_TICKS_PER_SEC)) {
            return FALSE;
        }	
    }
    return TRUE;
}



Bool ATCMD_ConnectTcpServer(const char *__FAR addr, INT16U port) {
   // if (!ATCMD_RetryUntilExpect(ExString_IsIPString(addr) ? "AT+QIDNSIP=0" : "AT+QIDNSIP=1", "OK", 5, OS_TICKS_PER_SEC)) {
    //    return FALSE;
   // }
    if (addr && (port != 0)) {
        ATCMD_Expect expect;
        char line[48];
        expect.recvBuffer = line;
        expect.recvLen = sizeof(line);
        
        (void)ATCMD_RetryUntilExpect("AT+CIPCLOSE=1", "CLOSE OK", 1, 10*OS_TICKS_PER_SEC);
        
        
      //  if (!ATCMD_RetryUntilExpect("AT+CIPMUX=0", "OK", 1, 5*OS_TICKS_PER_SEC)) {
       //     return FALSE;
      //  }	//设置连接为单连接
        
        
        if (!ATCMD_RetryUntilExpect("AT+CIPSHUT", "SHUT OK", 1, 65*OS_TICKS_PER_SEC)) {
            return FALSE;
        }	//关闭移动场景
        if (!ATCMD_RetryUntilExpect("AT+CIPQSEND = 0", "OK", 1, 5*OS_TICKS_PER_SEC)) {
            return FALSE;
        }	//
     
     
        if (!ATCMD_SetCPIMode(1)) {
            return FALSE;
        }	//设置连接为非透传模式
        
     
        if (!ATCMD_RetryUntilExpect("AT+CGCLASS=\"B\"", "OK", 1, 5*OS_TICKS_PER_SEC)) {
            return FALSE;
        }	//设置GPRS移动平台为B

        if (!ATCMD_RetryUntilExpect("AT+CGDCONT=1,\"IP\",\"CMNET\"", "OK", 1, 5*OS_TICKS_PER_SEC)) {
            return FALSE;
        }	//设置PDP上下文

        OSTimeDly(OS_TICKS_PER_SEC);

        if (!ATCMD_RetryUntilExpect("AT+CGATT=1", "OK", 5, 10*OS_TICKS_PER_SEC)) {
            return FALSE;
        }	//附着GPRS业务

        if (!ATCMD_RetryUntilExpect("AT+CIPCSGP=1,\"CMNET\"", "OK", 1, 5*OS_TICKS_PER_SEC)) {
            return FALSE;
        }	//设置为GPRS模式	

        if (!ATCMD_RetryUntilExpect("AT+CLPORT=\"TCP\",\"0\"", "OK", 1, 65*OS_TICKS_PER_SEC)) {
            return FALSE;
        }	//设置本地端口号	

        atcmd_SendString("AT+CIPSTART=\"TCP\",\"");
        atcmd_SendString(addr);
        SAFE_CALL(sprintf(line, "\",\"%d\"", port));
        expect.expect = "OK";
        if (!ATCMD_ExecCommand(line, &expect, 160UL * OS_TICKS_PER_SEC)) {
            return FALSE;
        }

        if (strcmp(line, "ALREADY CONNECT") == 0) {
            return TRUE;
        }
       // if (strcmp(line, "OK") != 0) {
       //     return FALSE;
      //  }

        expect.expect = "CONNECT";
        if (!ATCMD_ExecCommand(NULL, &expect, 160UL * OS_TICKS_PER_SEC)) {
            return FALSE;
        }
        if (strcmp(line, "CONNECT") == 0) {
            return TRUE;
        }
        return FALSE;
    }
    return FALSE;
}

Bool ATCMD_SendSMS(const char *__FAR dest, const char *__FAR msg) {
    ATCMD_Expect expect;
    char line[10];

    if (!dest || !msg) {
        return FALSE;
    }

    atcmd_SendString("AT+CMGS=\"");
    atcmd_SendString(dest);
    atcmd_SendString("\"\r");
    OSTimeDly(OS_TICKS_PER_SEC / 3);
    atcmd_ClearReply();
    //atcmd_SendString("WHOAMI:");
    atcmd_SendString(msg);
    atcmd_SendByte(0x1A);

    expect.expect = "+CMGS:";
    expect.recvLen = sizeof(line);
    expect.recvBuffer = line;
    return ATCMD_ExecCommand(NULL, &expect, OS_TICKS_PER_SEC * 10);
}

Bool ATCMD_SendDataViaGPRSCallback(void *__FAR private_data, ATCMD_GetSentData getSendData, INT16U total_len) {
    INT16U sent = 0;
    char buf[16] = {0x30,0x0,0x0,0x0,0x0b,'t','e','s','t','/','u','p', 'd','a','t','e'};
    char buf2[15] = {0x30,0x0,0x0,0x0b,'t','e','s','t','/','u','p', 'd','a','t','e'};

    if (!getSendData || total_len <= 0) {
        return RES_ERR;
    }
    
    if(total_len > 110) 
    {
        buf[1] = 0x80 | ((total_len + 13)%128);
        buf[2] = (INT8U)((total_len + 13)/128);
        atcmd_SendBytes((unsigned char *__FAR)buf, 16);
    } 
    else
    {
        buf2[1] = (INT8U)total_len + 13;
        atcmd_SendBytes((unsigned char *__FAR)buf2, 15);    
    }
 
        
    while(sent < total_len) {
        INT16U this_len = total_len - sent;
        if (this_len > 1320) {
            this_len = 1320;
        }        
        sent += this_len;
       
        
        do {
            INT16U i;
            i = getSendData(private_data, buf, (this_len > sizeof(buf)) ? sizeof(buf) : this_len);
            if (i == 0) {
                break;
            }
            atcmd_SendBytes((unsigned char *__FAR)buf, i);
            this_len -= i;
        } while (this_len > 0);
        
        /*数据不够补0*/
        while (this_len-- > 0) {
            atcmd_SendByte(0x00);
        }
        
    }
    
    return RES_OK;
}



Bool ATCMD_SendDataViaGPRS(const unsigned char *__FAR dat, INT16U len) {
    ATCMD_Expect expect;
    char buf[20];
    //INT32U timeout;
    //INT32U t;
    INT16U sent = 0;

    if (!dat || len <= 0) {
        return FALSE;
    }      

    expect.expect = "";
    expect.recvBuffer = buf;
    expect.recvLen = sizeof(buf);
        

    
    atcmd_SendBytes(dat,len);

        /*收到SEND OK表示发送成功*/
      //  timeout = OSTimeGet() + 20*OS_TICKS_PER_SEC;
       // for (t = OSTimeGet(); t < timeout; t = OSTimeGet()) {
        //    if (TRUE != ATCMD_ExecCommand(NULL, &expect, timeout - t )) {
        //        return FALSE;
        //    }

         //   if (strcmp("SEND OK", (char *)buf) == 0) {
         //       break;
          //  }
            
          //  if (strcmp("SEND FAIL", (char *)buf) == 0) {
           //     return FALSE;
           // }
       // }


    return TRUE;
}

Bool ATCMD_GetSignalQuality(unsigned char *rssi, unsigned char *ber) {
    char line[20];
    char *p;
    ATCMD_Expect expect;
    unsigned char thisRssi;
    unsigned char thisBer;

    expect.expect = "+CSQ:";
    expect.recvBuffer = line;
    expect.recvLen = sizeof(line);

    if (!ATCMD_ExecCommand("AT+CSQ", &expect, OS_TICKS_PER_SEC / 2)) {
        return FALSE;
    }

    // "+CSQ: 28, 0"
    p = &line[5];
    thisRssi = (INT8S)ExString_AtolSkipBankPrefix(p);
    if (thisRssi > 31 && thisRssi != 99) {
        return FALSE;
    }


    p = strchr(p + 1, ',');
    if (!p) {
        return FALSE;
    }

    thisBer = (INT8S)ExString_AtolSkipBankPrefix(++p);
    if (thisBer > 7 && thisBer != 99) {
        return FALSE;
    }
    dprintf("[ATCMD] Signal quality %d:%d\n", thisRssi, ber);
    if (rssi) {
        *rssi = thisRssi;
    }
    if (ber) {
        *ber = thisBer;
    }
    return TRUE;
}

Bool ATCMD_GetDataSentInfo(unsigned long *__FAR sent, unsigned long *__FAR acked, unsigned long *__FAR unacked) {
    char line[48];
    char *p;
    ATCMD_Expect expect;
    long thisSent;
    long thisAcked;
    long thisUnacked;

    //+QISACK: <sent>, <acked>, <nAcked>

    expect.recvBuffer = line;
    expect.recvLen = sizeof(line);

    expect.expect = "+QISACK:";
    if (!ATCMD_ExecCommand("AT+QISACK", &expect, 1000)) {
        return FALSE;
    }

    p = &line[8];
    thisSent = ExString_AtolSkipBankPrefix(p);
    if (thisSent == -1) {
        return FALSE;
    }
    p = strchr(p, ',');
    if (!p) {
        return FALSE;
    }

    thisAcked = ExString_AtolSkipBankPrefix(++p);
    if (thisAcked == -1) {
        return FALSE;
    }

    p = strchr(p, ',');
    if (!p) return FALSE;

    thisUnacked = ExString_AtolSkipBankPrefix(++p);
    if (thisUnacked == -1) return FALSE;

    if (sent) {
        *sent = thisSent;
    }
    if (acked) {
        *acked = thisAcked;
    }
    if (unacked) {
        *unacked = thisUnacked;
    }

    return TRUE;

}


Bool ATCMD_GetOPS(char *buf, INT8U len) {
    ATCMD_Expect expect;
    expect.expect = "+COPS:";
    expect.recvBuffer = buf;
    expect.recvLen = len;

    if (!ATCMD_ExecCommand("AT+COPS?", &expect, OS_TICKS_PER_SEC)) {
        return FALSE;
    } else {
        char *ops = strchr(buf, '\"');
        if (!ops) {
            return FALSE;
        }
        ++ops;

        while (*ops && *ops != '\"' && len > 1) {
            *buf++ = *ops++;
        }
        *buf = 0;
        return TRUE;
    }
}

Bool ATCMD_GetLacCi(unsigned short lac_ci[2]) {
    char buf[28];
    ATCMD_Expect expect;

    if (!lac_ci) {
        return FALSE;
    }
    expect.expect = "+CREG:";
    expect.recvBuffer = buf;
    expect.recvLen = sizeof(buf);

    if (!ATCMD_ExecCommand("AT+CREG?", &expect, OS_TICKS_PER_SEC)) {
        return FALSE;
    } else {
        char *ops = strchr(buf, '\"');
        if (!ops) {
            return FALSE;
        }

        lac_ci[0] = (unsigned short)strtol(ops + 1, &ops, 16);
        if (!ops || *ops != '\"') {
            return FALSE;
        }

        ops = strchr(ops + 1, '\"');
        if (!ops) {
            return FALSE;
        }
        lac_ci[1] = (unsigned short)strtol(ops + 1, &ops, 16);
        if (!ops || *ops != '\"') {
            return FALSE;
        }

        return TRUE;
    }
}

