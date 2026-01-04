#include <stdio.h>
#include <stdlib.h>

#include "debug.h"
#include "yamm.h"
#include "yammTest.h"
#include "testFrame.h"

/************************************************************
 * Function:      yammTestInit
 * Description:   测试yammInit的测试函数
 * Input:         params 表示测试用例操作集
 * Output:        NULL
 * Return:        OK 0 ERROR -1 表示测试是否成功
 ************************************************************/
int yammTestInit(YAMM_TEST_CMD_PARAMS *params)
{
    int expectedRet = params->expectedRetVal;
    size_t initSize = params->input;
    int ret = 0;

    DBG_LOG(DBG_LVL_DEBUG, "yammInit(%ld)", initSize);

    /* 调用函数 */
    ret = yammInit(initSize);

    /* 判断返回值是否一致 */
    if (ret != expectedRet)
    {
        DBG_LOG(DBG_LVL_ERROR, "chunk: actual return(%d), expected return(%d)", ret, expectedRet);
        return ERROR;
    }

    return OK;
}

/************************************************************
 * Function:      yammTestAlloc
 * Description:   测试yammAlloc的测试函数
 * Input:         params 表示测试用例操作集
 * Output:        NULL
 * Return:        OK 0 ERROR -1 表示测试是否成功
 ************************************************************/
int yammTestAlloc(YAMM_TEST_CMD_PARAMS *params)
{
    int expectedRet = params->expectedRetVal;
    size_t allocSize = params->input;
    void *returnPtr = NULL;
    int offset = 0;

    DBG_LOG(DBG_LVL_DEBUG, "yammAlloc(%ld)", allocSize);

    /* 调用函数 */
    returnPtr = yammAlloc(allocSize);

    /* 如果返回值为空 */
    if (!returnPtr)
    {
        offset = -1;
    }
    else
    {
        offset = (char *)returnPtr - (char *)yammGetHeap();
    }

    /* 判断返回值是否一致 */
    if (offset != expectedRet)
    {
        DBG_LOG(DBG_LVL_ERROR, "chunk: actual return(%d), expected return(%d)", offset, expectedRet);
        return ERROR;
    }

    return OK;
}

/************************************************************
 * Function:      yammTestFree
 * Description:   测试yammFree的测试函数
 * Input:         params 表示测试用例操作集
 * Output:        NULL
 * Return:        OK 0 ERROR -1 表示测试是否成功
 ************************************************************/
int yammTestFree(YAMM_TEST_CMD_PARAMS *params)
{
    int expectedRet = params->expectedRetVal;
    int freeAddrOffset = params->input;
    void *freeAddress = NULL;
    int ret = 0;

    /* 如果输入-1,则表示测试释放地址为NULL */
    if (freeAddrOffset >= 0)
    {
        freeAddress = (char *)yammGetHeap() + freeAddrOffset;
    }

    DBG_LOG(DBG_LVL_DEBUG, "yammFree(%p)", freeAddress);

    /* 调用函数 */
    ret = yammFree(freeAddress);
    /* 判断返回值是否一致 */
    if (ret != expectedRet)
    {
        DBG_LOG(DBG_LVL_ERROR, "chunk: actual return(%d), expected return(%d)", ret, expectedRet);
        return ERROR;
    }

    return OK;
}

/************************************************************
 * Function:      yammTestDestroy
 * Description:   测试yammDestroy的测试函数
 * Input:         params 表示测试用例操作集
 * Output:        NULL
 * Return:        OK 0 ERROR -1 表示测试是否成功
 ************************************************************/
int yammTestDestroy(YAMM_TEST_CMD_PARAMS *params)
{
    int expectedRet = params->expectedRetVal;
    int ret = 0;

    DBG_LOG(DBG_LVL_DEBUG, "yammDestroy()");

    /* 调用函数 */
    ret = yammDestroy();

    /* 判断返回值是否一致 */
    if (ret != expectedRet)
    {
        DBG_LOG(DBG_LVL_ERROR, "chunk: actual return(%d), expected return(%d)", ret, expectedRet);
        return ERROR;
    }

    return OK;
}

/************************************************************
 * Function:      yammTestDump
 * Description:   测试yammDump的测试函数
 * Input:         params 表示测试用例操作集
 * Output:        NULL
 * Return:        OK 0 ERROR -1 表示测试是否成功
 ************************************************************/
int yammTestDump(YAMM_TEST_CMD_PARAMS *params)
{
    int expectedRet = params->expectedRetVal;
    int ret = 0;

    DBG_LOG(DBG_LVL_DEBUG, "yammDump()");

    /* 调用函数 */
    ret = yammDump();

    /* 判断返回值是否一致 */
    if (ret != expectedRet)
    {
        DBG_LOG(DBG_LVL_ERROR, "chunk: actual return(%d), expected return(%d)", ret, expectedRet);
        return ERROR;
    }

    return OK;
}

/************************************************************
 * Function:      yammTestMemStats
 * Description:   用于测试每次操作后的内存状态是否一致
 * Input:         expectedMidValue 表示内存操作时中间临时变量
 * Output:        NULL
 * Return:        OK 0 ERROR -1 表示测试是否成功
 ************************************************************/
int yammTestMemStats(YAMM_STATS *expectedMidValue)
{
    YAMM_STATS *yammStats = NULL;

    if (!expectedMidValue)
    {
        DBG_LOG(DBG_LVL_ERROR, "expectedMidValue ptr is NULL.");
        return ERROR;
    }

    yammStats = malloc(sizeof(YAMM_STATS));
    if (!yammStats)
    {
        DBG_LOG(DBG_LVL_ERROR, "malloc failed.");
        return ERROR;
    }

    yammGetInfo(yammStats);

    if (expectedMidValue->mcbIdleCount == yammStats->mcbIdleCount &&
        expectedMidValue->mcbAllocatedCount == yammStats->mcbAllocatedCount &&
        expectedMidValue->mcbFreeCount == yammStats->mcbFreeCount &&
        expectedMidValue->allocatedSize == yammStats->allocatedSize &&
        expectedMidValue->freeSize == yammStats->freeSize)
    {
        free(yammStats);
        return OK;
    }

    free(yammStats);

    return ERROR;
}

/************************************************************
 * Function:      yammTestCase
 * Description:   用于依次测试单个测试用例中的每条操作
 * Input:         testCaseCmd 表示每个测试用例
 * Output:        NULL
 * Return:        OK 0 ERROR -1 表示测试是否成功
 ************************************************************/
int yammTestCase(void *testCaseCmd)
{
    int cmdIndex = 0;
    int ret = 0;
    YAMM_TEST_CASE *testCase = (YAMM_TEST_CASE *)testCaseCmd;
    YAMM_TEST_CMD_PARAMS *cmdSet = NULL;

    /* 对每一个测试用例，依次执行操作集中的每一个操作 */
    for (cmdIndex = 0; cmdIndex < testCase->cmdSetsNum; cmdIndex++)
    {
        DBG_LOG(DBG_LVL_INFO, MINUS_STR);
        DBG_LOG(DBG_LVL_INFO, "Cmd [%d/%d] begin.", cmdIndex+1, testCase->cmdSetsNum);

        cmdSet = &testCase->cmdSets[cmdIndex];

        /* 调用操作函数 */
        ret = cmdSet->func(cmdSet);
        if (ret != OK)
        {
            DBG_LOG(DBG_LVL_ERROR, "Cmd [%d/%d] failed. Function Check Failed.",
                    cmdIndex+1, testCase->cmdSetsNum);
            return ERROR;
        }

        /* 检查内存信息 */
        ret = yammTestMemStats(&cmdSet->expectedMidVal);
        if (ret != OK)
        {
            DBG_LOG(DBG_LVL_ERROR, "Cmd [%d/%d] failed. MemStates Check Failed.",
                    cmdIndex+1, testCase->cmdSetsNum);
            return ERROR;
        }

        DBG_LOG(DBG_LVL_INFO, "Cmd [%d/%d] success.", cmdIndex+1, testCase->cmdSetsNum);
    }
    return OK;
}