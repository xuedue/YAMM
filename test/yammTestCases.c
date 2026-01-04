#include <stdio.h>

#include "yammTest.h"
#include "types.h"
#include "testFrame.h"

#define YCS YAMM_CMD_SET_DEFINE
#define YMV YAMM_MID_VAL_DEFINE

/* 测试目的：简单用例，无异常情况 */
static YAMM_TEST_CMD_PARAMS yammCaseBasicFunc[] =
{
    /* 初始化 0x1000 字节的用户内存的 YAMM 实例，预期成功 */
    YCS(yammTestInit, 0x1000, OK, YMV(255, 0, 1, 0, 0x1000)),

    /* 申请 0x100 字节的用户内存 3 次，预期成功 */
    YCS(yammTestAlloc, 0x100, 0x0, YMV(254, 1, 1, 0x100, 0xF00)),
    YCS(yammTestAlloc, 0x100, 0x100, YMV(253, 2, 1, 0x200, 0xE00)),
    YCS(yammTestAlloc, 0x100, 0x200, YMV(252, 3, 1, 0x300, 0xD00)),

    /* 打印内存，预期看到用户内存占满 */
    YCS(yammTestDump, 0, OK, YMV(252, 3, 1, 0x300, 0xD00)),

    /* 释放第 1 块内存，预期成功 */
    YCS(yammTestFree, 0, OK, YMV(252, 2, 2, 0x200, 0xE00)),

    /* 释放第 3 块内存，预期成功 */
    YCS(yammTestFree, 0x200, OK, YMV(253, 1, 2, 0x100, 0xF00)),

    /* 释放第 2 块内存，出现前后合并情况，预期成功 */
    YCS(yammTestFree, 0x100, OK, YMV(255, 0, 1, 0, 0x1000)),

    /* 打印内存，预期看到只有内存尾部一块长度为 0xD00 的内存未释放 */
    YCS(yammTestDump, 0, OK, YMV(255, 0, 1, 0, 0x1000)),

    /* 销毁内存，预期成功 */
    YCS(yammTestDestroy, 0, OK, YMV(0, 0, 0, 0, 0))
};

/* 测试目的：测试和内存管理器初始化相关的异常情况 */
static YAMM_TEST_CMD_PARAMS yammCaseInitError[] =
{
    /* 初始化字节超出边界 */
    YCS(yammTestInit, 0x0, ERROR, YMV(0, 0, 0, 0, 0)),
    YCS(yammTestInit, 0x4001, ERROR, YMV(0, 0, 0, 0, 0)),

    /* 未初始化的情况进行内存操作，预计失败 */
    YCS(yammTestAlloc, 0x100, -1, YMV(0, 0, 0, 0, 0)),
    YCS(yammTestFree, 0, ERROR, YMV(0, 0, 0, 0, 0)),
    YCS(yammTestDump, 0, ERROR, YMV(0, 0, 0, 0, 0)),
    YCS(yammTestDestroy, 0, ERROR, YMV(0, 0, 0, 0, 0)),

    /* 初始化边界检查 */
    YCS(yammTestInit, 0x4000, OK, YMV(1023, 0, 1, 0, 0x4000)),
    YCS(yammTestDestroy, 0, OK, YMV(0, 0, 0, 0, 0)),

    /* 内存初始化对齐检查 */
    YCS(yammTestInit, 0x3fff, OK, YMV(1023, 0, 1, 0, 0x4000)),

    /* 重复初始化 */
    YCS(yammTestInit, 0x1000, ERROR, YMV(1023, 0, 1, 0, 0x4000)),

    /* 销毁内存，预期成功 */
    YCS(yammTestDestroy, 0, OK, YMV(0, 0, 0, 0, 0))
};

/* 测试目的：测试内存分配和内存释放相关的异常情况 */
static YAMM_TEST_CMD_PARAMS yammCaseAllocFree[] =
{
    /* 初始化0x30 */
    YCS(yammTestInit, 0x30, OK, YMV(2, 0, 1, 0, 0x30)),

    /* 对于申请大小为0的情况，返回错误 */
    YCS(yammTestAlloc, 0x0, -1, YMV(2, 0, 1, 0, 0x30)),
    /* 对于申请大小超出可用空间，返回错误 */
    YCS(yammTestAlloc, 0x31, -1, YMV(2, 0, 1, 0, 0x30)),
    /* 申请大小为10的内存，消耗空闲链表 */
    YCS(yammTestAlloc, 0x10, 0x0, YMV(1, 1, 1, 0x10, 0x20)),
    YCS(yammTestAlloc, 0x10, 0x10, YMV(0, 2, 1, 0x20, 0x10)),
    /* 打印内存，最后一块0x10为free */
    YCS(yammTestDump, 0, OK, YMV(0, 2, 1, 0x20, 0x10)),
    /* 空闲链表为空，再次申请内存，若有剩余空间，则预计失败 */
    YCS(yammTestAlloc, 0x5, -1, YMV(0, 2, 1, 0x20, 0x10)),
    /* 空闲链表为空，再次申请内存，若无剩余空间，则预计成功 */
    YCS(yammTestAlloc, 0x10, 0x20, YMV(0, 3, 0, 0x30, 0x0)),
    /* 打印内存,无空闲链表，内存耗尽 */
    YCS(yammTestDump, 0, OK, YMV(0, 3, 0, 0x30, 0x0)),

    /* cmd 10 偏移值为-1,则表示释放内存地址为NULL */
    YCS(yammTestFree, -1, ERROR, YMV(0, 3, 0, 0x30, 0x0)),
    /* 释放第二块内存，无归并 */
    YCS(yammTestFree, 0x10, OK, YMV(0, 2, 1, 0x20, 0x10)),
    /* 释放未分配的内存,预计失败 */
    YCS(yammTestFree, 0x10, ERROR, YMV(0, 2, 1, 0x20, 0x10)),

    /* 还有内存块出于分配状态，预计销毁发生错误 */
    YCS(yammTestDestroy, 0, ERROR, YMV(0, 2, 1, 0x20, 0x10)),

    /* 释放第一块内存，向后归并 */
    YCS(yammTestFree, 0x0, OK, YMV(1, 1, 1, 0x10, 0x20)),
    /* 释放第三块内存，向前归并 */
    YCS(yammTestFree, 0x20, OK, YMV(2, 0, 1, 0x0, 0x30)),
    /* 打印内存 */
    YCS(yammTestDump, 0, OK, YMV(2, 0, 1, 0x0, 0x30)),

    /* 销毁内存，预期成功 */
    YCS(yammTestDestroy, 0, OK, YMV(0, 0, 0, 0, 0))
};

#undef YCS
#undef YMV

/* 将要需要进行的测试项目加入 */
static YAMM_TEST_CASE yammTestCases[] =
{
#define YTC YAMM_TEST_CASE_DEFINE
    YTC(yammCaseBasicFunc),
    YTC(yammCaseInitError),
    YTC(yammCaseAllocFree),
    /* ... */
#undef YTC
};

TEST_SUITE yammTestSuite =
{
    .testCases = yammTestCases,
    .testCaseNum = ARRAY_SIZE(yammTestCases),
    .testPrepare = NULL,
    .execTestFn = yammTestCase,
    .checkTestResult = NULL
};