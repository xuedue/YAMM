#ifndef __TESTFRAME_H_
#define __TESTFRAME_H_

#define YAMM_MID_VAL_DEFINE(nIdle, nAlloc, nFree, allocated, free) {     .mcbIdleCount = nIdle,     .mcbAllocatedCount = nAlloc,     .mcbFreeCount = nFree,     .allocatedSize = allocated,     .freeSize = free }

#define YAMM_CMD_SET_DEFINE(             function, inputArg, expectedRetValue, expectedMidValue) {     .func = function,     .input = inputArg,     .expectedRetVal  = expectedRetValue,     .expectedMidVal  = expectedMidValue  }

#define YAMM_TEST_CASE_DEFINE(testCase) {     .cmdSets = testCase,     .cmdSetsNum = sizeof(testCase) / sizeof(YAMM_TEST_CMD_PARAMS) }

typedef int (*funcUnitTest)(void *data);

typedef struct _TEST_SUITE
{
	void *testCases;
	int testCaseNum;

	int (*testPrepare)(void);

	funcUnitTest execTestFn;
	funcUnitTest checkTestResult;
} TEST_SUITE;

typedef struct _YAMM_STATS
{
    size_t mcbIdleCount;        /* 空闲的 MCB 管理结点数目 */
    size_t mcbAllocatedCount;     /* 指向已分配内存的 MCB 管理结点数目 */
    size_t mcbFreeCount;         /* 指向未分配内存的 MCB 管理结点数目 */

    size_t allocatedSize;        /* 堆中的已分配的用户内存大小 */
    size_t freeSize;            /* 堆中的未分配的用户内存大小 */
} YAMM_STATS;

/* YAMM 的操作命令结构定义 */
typedef struct _YAMM_TEST_CMD_PARAMS
{
    int (*func)(struct _YAMM_TEST_CMD_PARAMS *params);        /* 操作函数 */
    int input;                                                /* 函数入参 */
    int expectedRetVal;                                        /* 预期返回值 */
    YAMM_STATS expectedMidVal;                                /* 预期中间临时变量状态 */
} YAMM_TEST_CMD_PARAMS;

/* YAMM 的测试用例结构定义 */
typedef struct _YAMM_TEST_CASE
{
    YAMM_TEST_CMD_PARAMS *cmdSets;        /* 操作集 */
    int cmdSetsNum;                        /* 操作数量 */
} YAMM_TEST_CASE;

int yammTest(TEST_SUITE *testSuite);

#endif /* __TESTFRAME_H_ */