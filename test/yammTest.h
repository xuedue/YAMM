#ifndef __YAMMTEST_H_
#define __YAMMTEST_H_

#include "testFrame.h"

#define OK 0
#define ERROR -1

/* 用于测试YAMM每个接口函数 */
int yammTestInit(YAMM_TEST_CMD_PARAMS *params);
int yammTestAlloc(YAMM_TEST_CMD_PARAMS *params);
int yammTestFree(YAMM_TEST_CMD_PARAMS *params);
int yammTestDestroy(YAMM_TEST_CMD_PARAMS *params);
int yammTestDump(YAMM_TEST_CMD_PARAMS *params);

/* 用于测试每次操作内存后的内存状态是否一致 */
int yammTestMemStats(YAMM_STATS *expectedMidValue);
/* 用于测试单个测试用例 */
int yammTestCase(void *testCaseCmd);

#endif /* __YAMMTEST_H_ */