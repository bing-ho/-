/// \file ex_string.c
/// \brief  string扩展操作的实现
/// \author Xi Qingping, qingping.xi@ligoo.cn
/// \version
/// \date 2015-09-06

#include "ex_string.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>


char ExString_NIsDigitString(const char *__FAR s, int len) {
    if (!s) {
        return 0;
    }

    while (len-- > 0) {
        if (!isdigit(*s)) {
            return 0;
        }
        ++s;
    }
    return 1;
}

char ExString_IsDigitString(const char *__FAR s) {
    if (!s) {
        return 0;
    }

    while (*s) {
        if (!isdigit(*s)) {
            return 0;
        }
        ++s;
    }
    return 1;
}

char ExString_IsHexString(const char *__FAR s) {
    if (!s) {
        return 0;
    }

    if ((s[0] == '0') && (s[1] == 'X' || s[1] == 'x')) {
        s += 2;
    }

    while (*s) {
        if (!isxdigit(*s)) {
            return 0;
        }
        ++s;
    }
    return 1;
}

long ExString_StrToLDecade(const char *__FAR s, const char *__FAR *left) {
    long ret = 0;

    if (!s) {
        goto __ret;
    }

    while (isdigit(*s)) {
        ret = ret * 10 + (*s++) - '0';
    }

__ret:
    if (left) {
        *left = s;
    }
    return ret;
}



long ExString_StrToLHex(const char *__FAR s, const char *__FAR *left) {
    long ret = 0;

    if (!s) {
        goto __ret;
    }

    if ((s[0] == '0') && (s[1] == 'X' || s[1] == 'x')) {
        s += 2;
    }

    for(;;) {
        char x = *s;
        if (x >= '0' && x <= '9') {
            x -= '0';
        } else if (x >= 'A' && x <= 'F') {
            x -= ('A' - 10);
        } else if (x >= 'a' && x <= 'f') {
            x -= ('a' - 10);
        } else {
            goto __ret;
        }
        ++s;
        ret = (ret << 4) | x;
    }
__ret:
    if (left) {
        *left = s;
    }
    return ret;
}


unsigned char ExString_IsIPString(const char *__FAR s) {
    long ipseg;
    const char *_FAR p = s;

    if (!s) {
        return 0;
    }

    ipseg = ExString_StrToLDecade(p, &p);
    if (ipseg < 0 || ipseg > 255 || *p != '.') {
        return 0;
    }


    ipseg = ExString_StrToLDecade(p + 1, &p);
    if (ipseg < 0 || ipseg > 255 || *p != '.') {
        return 0;
    }

    ipseg = ExString_StrToLDecade(p + 1, &p);
    if (ipseg < 0 || ipseg > 255 || *p != '.') {
        return 0;
    }

    ipseg = ExString_StrToLDecade(p + 1, &p);
    if (ipseg < 0 || ipseg > 255 || *p != 0) {
        return 0;
    }

    return 1;

}


long ExString_Atol(const char *__FAR s) {
    long ret = 0;
    while (isdigit(*s)) {
        ret = ret * 10 + (*s++) - '0';
    }

    return ret;
}


long ExString_AtolSkipBankPrefix(const char *__FAR s) {
    long ret = 0;
    if (!s) {
        return -1;
    }

    while (*s == ' ' || *s == '\t') {
        ++s;
    }
    while (isdigit(*s)) {
        ret = ret * 10 + (*s++) - '0';
    }

    return ret;
}
