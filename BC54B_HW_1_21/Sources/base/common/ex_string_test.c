#include "ex_string.h"
#include "unity_fixture.h"


TEST_GROUP(ex_string);

TEST_SETUP(ex_string) {
}

TEST_TEAR_DOWN(ex_string) {
}

TEST(ex_string, NIsDigitString) {
    TEST_ASSERT(ExString_NIsDigitString("123", 3));
    TEST_ASSERT(ExString_NIsDigitString("123a", 3));
    TEST_ASSERT(!ExString_NIsDigitString("123b", 4));
    TEST_ASSERT(ExString_NIsDigitString("a123b", 0));
}

TEST(ex_string, IsDigitString) {
    TEST_ASSERT(ExString_IsDigitString("123"));
    TEST_ASSERT(!ExString_IsDigitString("123a"));
    TEST_ASSERT(!ExString_IsDigitString("123b"));
    TEST_ASSERT(!ExString_IsDigitString("a123b"));
}

TEST(ex_string, IsHexString) {
    TEST_ASSERT(ExString_IsHexString("123"));
    TEST_ASSERT(ExString_IsHexString("123a"));
    TEST_ASSERT(ExString_IsHexString("0x123b"));
    TEST_ASSERT(ExString_IsHexString("0x123"));
    TEST_ASSERT(!ExString_IsHexString("a123bj"));
}


TEST(ex_string, IsIPString) {
    TEST_ASSERT(ExString_IsIPString("192.168.9.1"));
    TEST_ASSERT(ExString_IsIPString("192.228.9.1"));
    TEST_ASSERT(!ExString_IsIPString("192.292.9.1"));
    TEST_ASSERT(!ExString_IsIPString("google.cn.1"));
}


TEST(ex_string, Atol) {
    TEST_ASSERT_EQUAL(1234789, ExString_Atol("1234789"));
    TEST_ASSERT_EQUAL(1234, ExString_Atol("1234a"));
}

TEST(ex_string, AtolSkipBankPrefix) {
    TEST_ASSERT_EQUAL(1234789, ExString_AtolSkipBankPrefix("  \t1234789"));
    TEST_ASSERT_EQUAL(1234, ExString_AtolSkipBankPrefix("\t\t1234a"));
    TEST_ASSERT_EQUAL(1234, ExString_AtolSkipBankPrefix(" 1234a"));
    TEST_ASSERT_EQUAL(1234789, ExString_AtolSkipBankPrefix("\t1234789"));
    TEST_ASSERT_EQUAL(0, ExString_AtolSkipBankPrefix("ab1234a"));
    TEST_ASSERT_EQUAL(0, ExString_AtolSkipBankPrefix("\t ab1234a"));
    TEST_ASSERT_EQUAL(0, ExString_AtolSkipBankPrefix(" ab1234a"));
}

TEST(ex_string, StrToLDecade) {
    const char *__FAR a;
    const char *__FAR b;

    a = "1231231avc";
    TEST_ASSERT_EQUAL(1231231, ExString_StrToLDecade(a, &b));
    TEST_ASSERT_EQUAL(a + 7, b);

    a = "7890";
    TEST_ASSERT_EQUAL(7890, ExString_StrToLDecade(a, &b));
    TEST_ASSERT_EQUAL(a + 4, b);

    a = "abc789a";
    TEST_ASSERT_EQUAL(0, ExString_StrToLDecade(a, &b));
    TEST_ASSERT_EQUAL(a, b);
}

TEST(ex_string, StrToLHex) {
    const char *__FAR a;
    const char *__FAR b;

    a = "1231231avc";
    TEST_ASSERT_EQUAL(0x1231231a, ExString_StrToLHex(a, &b));
    TEST_ASSERT_EQUAL(a + 8, b);

    a = "7890";
    TEST_ASSERT_EQUAL(0x7890, ExString_StrToLHex(a, &b));
    TEST_ASSERT_EQUAL(a + 4, b);

    a = "kkabc789a";
    TEST_ASSERT_EQUAL(0x0, ExString_StrToLHex(a, &b));
    TEST_ASSERT_EQUAL(a, b);
    a = "0xabc7";
    TEST_ASSERT_EQUAL(0xabc7, ExString_StrToLHex(a, &b));
    TEST_ASSERT_EQUAL(a+6, b);
}


TEST_GROUP_RUNNER(ex_string) {
    RUN_TEST_CASE(ex_string, NIsDigitString);
    RUN_TEST_CASE(ex_string, IsDigitString);
    RUN_TEST_CASE(ex_string, IsHexString);
    RUN_TEST_CASE(ex_string, IsIPString);
    RUN_TEST_CASE(ex_string, Atol);
    RUN_TEST_CASE(ex_string, AtolSkipBankPrefix);
    RUN_TEST_CASE(ex_string, StrToLDecade);
    RUN_TEST_CASE(ex_string, StrToLHex);
}

