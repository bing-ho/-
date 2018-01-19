#ifndef __SHELL_CONFIG_H__
#define __SHELL_CONFIG_H__

#include "run_mode.h"

// =========================================================
// 在这下面定义SHELL的行为
// =========================================================
#if RUN_TEST_MODE_ALWAYS
#define SHELL_SUPPORT_HELP 1
#define SHELL_WELCOM_MESSAGE \
"\n\
================================================\n\
Hello, this is a simple shell for BC52B test mode.\n\
type \"help<cr>\" for more information.\n\
================================================\n\
"

#else
#define SHELL_SUPPORT_HELP 0
#endif


#define SHELL_PROMPT   "BC52B# "


// =========================================================
// 在这下面定义SHELL的行为
// =========================================================
#include "shell_platform.h"


SHELL_PROTYPE(echo);
SHELL_PROTYPE(adc);
SHELL_PROTYPE(rs485);
SHELL_PROTYPE(dtu);
SHELL_PROTYPE(ads1015);
SHELL_PROTYPE(octrl);
SHELL_PROTYPE(input);
SHELL_PROTYPE(hv);
//SHELL_PROTYPE(sdcard);
SHELL_PROTYPE(can);
SHELL_PROTYPE(rtc);
SHELL_PROTYPE(flag);
SHELL_PROTYPE(hall);
SHELL_PROTYPE(cppwm);
SHELL_PROTYPE(reset);
SHELL_PROTYPE(ntc);
SHELL_PROTYPE(hcf4052);
SHELL_PROTYPE(insuhv);
SHELL_PROTYPE(bdinfo);
SHELL_PROTYPE(uuid);
SHELL_PROTYPE(nand);
SHELL_PROTYPE(spi);
SHELL_PROTYPE(mlock);
SHELL_PROTYPE(vol);
SHELL_PROTYPE(temp);
SHELL_PROTYPE(sslave); 
SHELL_PROTYPE(current); 



#define SHELL_COMMAND_CUSTOM_LIST \
    SHELL_COMMAND("echo", echo), \
    SHELL_COMMAND("adc", adc), \
    SHELL_COMMAND("rs485", rs485), \
    SHELL_COMMAND("dtu", dtu), \
    SHELL_COMMAND("hv", hv), \
    SHELL_COMMAND("octrl", octrl), \
    SHELL_COMMAND("input", input), \
    SHELL_COMMAND("ads1015", ads1015), \
    SHELL_COMMAND("can", can), \
    SHELL_COMMAND("rtc", rtc), \
    SHELL_COMMAND("flag", flag), \
    SHELL_COMMAND("hall", hall), \
    SHELL_COMMAND("cppwm", cppwm), \
    SHELL_COMMAND("reset", reset), \
    SHELL_COMMAND("ntc", ntc), \
    SHELL_COMMAND("hcf", hcf4052), \
    SHELL_COMMAND("insuhv", insuhv), \
    SHELL_COMMAND("uuid", uuid), \
    SHELL_COMMAND("nand", nand),\
    SHELL_COMMAND("bdinfo", bdinfo), \
    SHELL_COMMAND("spi", spi),\
    SHELL_COMMAND("mlock", mlock),\
    SHELL_COMMAND("vol", vol),\
    SHELL_COMMAND("temp", temp),\
    SHELL_COMMAND("sslave", sslave),\
    SHELL_COMMAND("current", current),\

#endif

