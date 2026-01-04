#include <stdio.h>
#include <stdlib.h>

#include "debug.h"
#include "testFrame.h"

/************************************************************
 * Function:      yammTest
 * Description:   用于依次测试每个测试用例
 * Input:         testSuite 表示测试对象
 * Output:        NULL
 * Return:        0 表示测试成功 -1 表示测试失败
 ************************************************************/
int yammTest(TEST_SUITE *testSuite)
{
    int ret = 0;
    int caseIndex = 0;
    YAMM_TEST_CASE *testCase = NULL;
    YAMM_TEST_CASE *testCases = testSuite->testCases;
    int testCaseNum = testSuite->testCaseNum;

    for (caseIndex = 0; caseIndex < testCaseNum; caseIndex++)
    {
        DBG_LOG(DBG_LVL_INFO, EQUAL_STR);
        DBG_LOG(DBG_LVL_INFO, "Executing: case [%d] test basic func", caseIndex);

        testCase = testCases + caseIndex;
        ret = testSuite->execTestFn(testCase);

        if (ret)
        {
            DBG_LOG(DBG_LVL_ERROR, "error in test suite #%d, process terminated.", caseIndex);
            return -1;
        }
    }

    return 0;
}