#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "list.h"
#include "yamm.h"
#include "types.h"
#include "debug.h"
#include "testFrame.h"

typedef struct Mem_Ctrl_Blk
{
    struct List_Head list;
    char *chunkPtr;
    int chunkSize;
    char type;
} MEM_CTRL_BLK;

typedef struct YammContext
{
    char *memoryPoolAddress;
    MEM_CTRL_BLK *mcbPoolAddress;
    struct List_Head *busyListHead;
    struct List_Head *idelListHead;
} YAMMCONTEXT;

YAMMCONTEXT gyammContext;
YAMM_STATS gyammStats;

/************************************************************
 * Function:    yammPrintInitInfo
 * Description: 打印初始化后的内存管理器信息
 * Input:       size 表示内存申请的大小
 * Output:      null
 * Return:      null
 ************************************************************/
void yammPrintInitInfo(size_t size)
{
    size_t mcbTotalCount = 0;
    size_t actualSize = 0;

    if (!gyammContext.memoryPoolAddress)
    {
        DBG_LOG(DBG_LVL_DEBUG, "yamm has not init.");
        return;
    }

    actualSize = (char *)gyammContext.mcbPoolAddress - (char *)gyammContext.memoryPoolAddress;
    yammGetInfo(&gyammStats);
    mcbTotalCount = gyammStats.mcbIdleCount + gyammStats.mcbAllocatedCount + gyammStats.mcbFreeCount;

    DBG_LOG(DBG_LVL_INFO, "Init 0x%lx OK, addr: %p, actual size: 0x%lx", size,
            gyammContext.memoryPoolAddress, actualSize);

    DBG_LOG(DBG_LVL_DEBUG, "Idel: %ld, Allocated: %ld, Free: %ld, Total: %ld", gyammStats.mcbIdleCount,
            gyammStats.mcbAllocatedCount, gyammStats.mcbFreeCount, mcbTotalCount);
}

/************************************************************
 * Function:    yammInit
 * Description: 用于内存管理器初始化
 * Input:       size 表示可分配内存空间大小
 * Output:      null
 * Return:      0 表示分割成功，-1表示分割失败
 ************************************************************/
int yammInit(size_t size)
{
    MEM_CTRL_BLK *mcbPoolPos = NULL;
    char *initalMemory = NULL;
    int initalSize = 0;
    int memorySize = 0;
    int mcbNum = 0;
    int mcbSize = 0;
    int iterateMcb = 0;

    if (gyammContext.memoryPoolAddress)
    {
        DBG_LOG(DBG_LVL_DEBUG, "Yamm has init already.");
        return YAMM_ERROR;
    }

    /* 用户数据内存池，申请内存空间地址对齐 */
    memorySize = CEIL(size, ALIGN_BYTE) * ALIGN_BYTE;

    if (0 == memorySize || MAX_HEAP_SIZE < memorySize)
    {
        DBG_LOG(DBG_LVL_DEBUG, "Init Size is illeage.");
        return YAMM_ERROR;
    }

    mcbNum = CEIL(memorySize, MCB_RATIO); /* 内存管理节点个数（向上取整） */
    mcbSize = mcbNum * sizeof(MEM_CTRL_BLK);    /* 内存管理节点池大小 */
    /* 总的大小（加上两个头指针对应的内存大小） */
    initalSize = memorySize + mcbSize + 2 * sizeof(struct List_Head);

    /* 内存预分配，申请一大段内存 */
    initalMemory = malloc(initalSize);
    if (!initalMemory)
    {
        DBG_LOG(DBG_LVL_DEBUG, "Malloc Failed.");
        return YAMM_ERROR;
    }
    memset(initalMemory, 0, initalSize);

    /* 将预申请的内存划分给每个部分 */
    gyammContext.memoryPoolAddress = initalMemory;
    gyammContext.mcbPoolAddress = (MEM_CTRL_BLK *)(initalMemory + memorySize);
    gyammContext.busyListHead = (struct List_Head *)(initalMemory + memorySize + mcbSize);
    gyammContext.idelListHead = (struct List_Head *)(initalMemory + memorySize + mcbSize + sizeof(struct List_Head));

    /* 初始化空闲和忙碌链表头节点 */
    commonListInit(gyammContext.busyListHead);
    commonListInit(gyammContext.idelListHead);

    /* 初始化忙碌链表，内存块地址指向内存池首地址 */
    gyammContext.mcbPoolAddress->type = TYPE_FREE;
    gyammContext.mcbPoolAddress->chunkPtr = gyammContext.memoryPoolAddress;
    gyammContext.mcbPoolAddress->chunkSize = memorySize;
    commonListAdd(&gyammContext.mcbPoolAddress->list, gyammContext.busyListHead);

    /* 初始化空闲链表 */
    for (iterateMcb = 1; iterateMcb < mcbNum; iterateMcb++)
    {
        mcbPoolPos = gyammContext.mcbPoolAddress + iterateMcb;
        mcbPoolPos->type = TYPE_IDEL;
        mcbPoolPos->chunkPtr = NULL;
        mcbPoolPos->chunkSize = 0;
        commonListAdd(&mcbPoolPos->list, gyammContext.idelListHead);
    }
    yammPrintInitInfo(memorySize);

    return YAMM_OK;
}

/************************************************************
 * Function:    mcbSplit
 * Description: 分割内存块，从空闲链表中取尾部节点指向剩余空间，并加
 *              入到忙碌链表相应位置
 * Input:       allocatedNode 表示申请内存块的对应节点
 *              mallocSize 表示内存管理器分配空间大小(!=用户输入大小)
 * Output:      null
 * Return:      1 表示分割成功，0表示分割失败
 * Calls:       mcbSplit 用于内存块分割
 ************************************************************/
int mcbSplit(struct List_Head *allocatedNode, int mallocSize)
{
    MEM_CTRL_BLK *moveMCB = NULL;
    MEM_CTRL_BLK *allocatedMCB = NULL;
    struct List_Head *moveNode = NULL;
    int restSize = 0;

    allocatedMCB = commonListEntry(allocatedNode, MEM_CTRL_BLK, list);
    restSize = allocatedMCB->chunkSize - mallocSize;

    /* 如果申请内存正好符合节点指向的内存块大小，就不需要进行内存块分割 */
    if (restSize == 0)
    {
        return TRUE;
    }
    /* 如果剩余空间不为0,且空闲链表为空，表示没有多余的节点可以用来指向剩余空间 */
    else if (commonListEmtpy(gyammContext.idelListHead))
    {
        DBG_LOG(DBG_LVL_DEBUG, "idel list has no node.");
        return FALSE;
    }
    else /* 将空闲链表中节点移动到忙碌链表中 */
    {
        moveNode = gyammContext.idelListHead->prev;
        moveMCB = commonListEntry(moveNode, MEM_CTRL_BLK, list);
        /* moveNode从原空闲链表中删除 */
        commonListDel(moveNode);
        moveMCB->type = TYPE_FREE;
        moveMCB->chunkPtr = allocatedMCB->chunkPtr + mallocSize;
        moveMCB->chunkSize = restSize;
        commonListAdd(moveNode, allocatedNode);
        return TRUE;
    }

    DEBUG_ERROR_POSITION;
    return FALSE;
}

/************************************************************
 * Function:    yammAlloc
 * Description: 用于内存管理器的内存申请
 * Input:       size 表示用户要申请的内存大小
 * Output:      null
 * Return:      返回分配后的内存的首地址
 * Calls:       mcbSplit 用于内存块分割
 ************************************************************/
void *yammAlloc(size_t size)
{
    struct List_Head *iteratePos = NULL;
    struct List_Head *iterateNext = NULL;
    MEM_CTRL_BLK *iterateMCB = NULL;
    int mallocSize = 0;

    if (!gyammContext.memoryPoolAddress)
    {
        DBG_LOG(DBG_LVL_DEBUG, "YAMM has not init or has destroy.");
        return NULL;
    }

    if (0 == size)
    {
        DBG_LOG(DBG_LVL_DEBUG, "can't not alloc a zero byte memory block.");
        return NULL;
    }

    /* 申请内存要求8字节对齐，向上取整 */
    mallocSize = (int)((size + ALIGN_BYTE - 1) / ALIGN_BYTE) * ALIGN_BYTE;

    /* 遍历忙碌链表，首次适应算法，然后修改节点信息并进行内存块分割 */
    commonListIterateSafe(iteratePos, iterateNext, gyammContext.busyListHead)
    {
        iterateMCB = commonListEntry(iteratePos, MEM_CTRL_BLK, list);

        /* 如果找到满足条件的节点且内存块分割成功 */
        if (iterateMCB->type == TYPE_FREE &&
            iterateMCB->chunkSize >= mallocSize &&
            mcbSplit(iteratePos, mallocSize))
        {
            iterateMCB->type = TYPE_ALLOC;
            iterateMCB->chunkSize = mallocSize;
            DBG_LOG(DBG_LVL_INFO, "Alloc  0x%lx success, addr: %p.", size, iterateMCB->chunkPtr);
            return iterateMCB->chunkPtr;
        }
    }

    DBG_LOG(DBG_LVL_DEBUG, "Yamm can't find fit memory block.");

    return NULL;
}

/************************************************************
 * Function:    mcbMerge
 * Description: 内存块归并，先判断后节点，再判断前节点
 * Input:       freeNode 表示释放内存的节点
 * Output:      修改MCB节点的状态，若存在归并，就将目标节点移动到空闲链表中
 * Return:      void
 ************************************************************/
void mcbMerge(struct List_Head *freeNode)
{
    MEM_CTRL_BLK *freeMCB = NULL;
    MEM_CTRL_BLK *prevMCB = NULL;
    MEM_CTRL_BLK *nextMCB = NULL;
    struct List_Head *moveNode = NULL;

    freeMCB = commonListEntry(freeNode, MEM_CTRL_BLK, list);

    /* 如果忙碌链表除了freeNode还有其他节点的话 */
    if (freeNode->prev != gyammContext.busyListHead)
    {
        prevMCB = commonListEntry(freeNode->prev, MEM_CTRL_BLK, list);
    }
    if (freeNode->next != gyammContext.busyListHead)
    {
        nextMCB = commonListEntry(freeNode->next, MEM_CTRL_BLK, list);
    }

    /* 向后归并 */
    if (nextMCB && (TYPE_FREE == nextMCB->type))
    {
        freeMCB->chunkSize += nextMCB->chunkSize;
        nextMCB->type = TYPE_IDEL;
        nextMCB->chunkPtr = NULL;
        nextMCB->chunkSize = 0;
        moveNode = freeNode->next;
        commonListDel(moveNode);
        commonListAdd(moveNode, gyammContext.idelListHead);
        DBG_LOG(DBG_LVL_DEBUG, "Combine with next fragment.");
    }

    /* 向前归并 */
    if (prevMCB && (TYPE_FREE == prevMCB->type))
    {
        prevMCB->chunkSize += freeMCB->chunkSize;
        freeMCB->type = TYPE_IDEL;
        freeMCB->chunkPtr = NULL;
        freeMCB->chunkSize = 0;
        moveNode = freeNode;
        commonListDel(moveNode);
        commonListAdd(moveNode, gyammContext.idelListHead);
        DBG_LOG(DBG_LVL_DEBUG, "Combine with previous fragment.");
    }
}

/************************************************************
 * Function:    yammFree
 * Description: 内存空间释放，并进行前后内存块归并
 * Input:       ptr 表示释放内存的地址
 * Output:      null
 * Return:      0 for success, -1 for print failed
 * Calls:       调用 mcbMerge 函数用于归并操作
 ************************************************************/
int yammFree(void *ptr)
{
    struct List_Head *iteratePos = NULL;
    struct List_Head *iterateNext = NULL;
    MEM_CTRL_BLK *iterateMCB = NULL;
    MEM_CTRL_BLK *freeMCB = NULL;

    if (!ptr)
    {
        DBG_LOG(DBG_LVL_DEBUG, "Free address is NULL.");
        return YAMM_ERROR;
    }

    if (!gyammContext.memoryPoolAddress)
    {
        DBG_LOG(DBG_LVL_DEBUG, "YAMM has not init or has destroy.");
        return YAMM_ERROR;
    }

    /* 遍历忙碌链表，寻找对应的地址 */
    commonListIterateSafe(iteratePos, iterateNext, gyammContext.busyListHead)
    {
        iterateMCB = commonListEntry(iteratePos, MEM_CTRL_BLK, list);

        /* 找到对应地址
         * 直接 break/return 是因为不可以申请内存大小为0的内存空间
         */
        if (ptr == iterateMCB->chunkPtr)
        {
            freeMCB = iterateMCB;
            break;
        }
    }

    /* 如果在忙碌链表中找不到对应的内存块 */
    if (!freeMCB)
    {
        DBG_LOG(DBG_LVL_DEBUG, "Free Address is illeagel.");
        return YAMM_ERROR;
    }

    /* 找到对应地址并且为分配状态才进行内存释放，即修改状态码，清空内存块内容*/
    if (TYPE_ALLOC == freeMCB->type)
    {
        freeMCB->type = TYPE_FREE;
        memset(freeMCB->chunkPtr, 0, freeMCB->chunkSize);
        /* 进行归并操作 */
        mcbMerge(&freeMCB->list);
        DBG_LOG(DBG_LVL_INFO, "free %p bytes success.", ptr);
        return YAMM_OK;
    }
    else
    {
        DBG_LOG(DBG_LVL_DEBUG, "Try to free not allocated memory.");
        return YAMM_ERROR;
    }

    return YAMM_ERROR;
}

/************************************************************
 * Function:    yammDestroy
 * Description: 内存管理器销毁,销毁链表以及释放内存空间
 * Input:       void
 * Output:      void
 * Return:      0 for success, -1 for print failed
 ************************************************************/
int yammDestroy(void)
{
    struct List_Head *iteratePos = NULL;
    struct List_Head *iterateNext = NULL;
    MEM_CTRL_BLK *iterateMCB = NULL;

    if (!gyammContext.memoryPoolAddress)
    {
        DBG_LOG(DBG_LVL_DEBUG, "YAMM has not init or has destroy.");
        return YAMM_ERROR;
    }

    /* 判断是否还有内存块处于申请状态 */
    commonListIterate(iteratePos, gyammContext.busyListHead)
    {
        iterateMCB = commonListEntry(iteratePos, MEM_CTRL_BLK, list);
        if (iterateMCB->type == TYPE_ALLOC)
        {
            DBG_LOG(DBG_LVL_DEBUG, "some mcb is still allocated.");
            return YAMM_ERROR;
        }
    }

    /* 清空忙碌链表和空闲链表 */
    commonListIterateSafe(iteratePos, iterateNext, gyammContext.busyListHead)
    {
        commonListDelInit(iteratePos);
    }
    commonListIterateSafe(iteratePos, iterateNext, gyammContext.idelListHead)
    {
        commonListDelInit(iteratePos);
    }

    /* 释放整个内存空间 */
    free(gyammContext.memoryPoolAddress);

    /* 避免野指针 */
    gyammContext.memoryPoolAddress = NULL;
    gyammContext.mcbPoolAddress = NULL;
    gyammContext.busyListHead = NULL;
    gyammContext.idelListHead = NULL;

    return YAMM_OK;
}

/************************************************************
 * Function:    yammDump
 * Description: 打印用户数据内存块的状态，按照地址排序输出表格
 * Input:       void
 * Output:      void
 * Return:      0 for success, -1 for print failed
 ************************************************************/
int yammDump(void)
{
    char *typeString[3] = {"idel", "free", "allocated"};
    struct List_Head *iteratePos = NULL;
    MEM_CTRL_BLK *iterateMCB = NULL;
    size_t mcbTotalCount = 0;
    size_t heapTotalSize = 0;
    size_t addrOffset = 0;
    size_t id = 0;

    if (!gyammContext.memoryPoolAddress)
    {
        DBG_LOG(DBG_LVL_DEBUG, "YAMM has not init or has destroy.");
        return YAMM_ERROR;
    }

    DEBUG_INFO("	------------------------------------------
");
    DEBUG_INFO("	[Heap]
");
    DEBUG_INFO("	%-10s %-10s %-10s %-10s %-10s %-10s
",
                "ID", "Address", "Offset", "Size", "Type", "MCB Address");

    id = 0;
    commonListIterate(iteratePos, gyammContext.busyListHead)
    {
        iterateMCB = commonListEntry(iteratePos, MEM_CTRL_BLK, list);
        addrOffset = iterateMCB->chunkPtr - gyammContext.memoryPoolAddress;
        /* 格式化输出 */
        DEBUG_INFO("	%-10ld %-10p 0x%-8lx 0x%-8x %-10s %-10p
", id, iterateMCB->chunkPtr, addrOffset,
                iterateMCB->chunkSize, typeString[iterateMCB->type - '0'], iterateMCB);
        id++;
    }

    yammGetInfo(&gyammStats);

    mcbTotalCount = gyammStats.mcbIdleCount + gyammStats.mcbAllocatedCount + gyammStats.mcbFreeCount;
    heapTotalSize = gyammStats.allocatedSize + gyammStats.freeSize;

    DEBUG_INFO("	Allocated:0x%lx, Free:0x%lx, Total:0x%lx
",
                gyammStats.allocatedSize, gyammStats.freeSize, heapTotalSize);
    DEBUG_INFO("	------------------------------------------
");
    DEBUG_INFO("	[MCB Count]
");
    DEBUG_INFO("	Idle:%ld, Allocated:%ld, Free:%ld, Total:%ld
", gyammStats.mcbIdleCount,
                gyammStats.mcbAllocatedCount, gyammStats.mcbFreeCount, mcbTotalCount);
    DEBUG_INFO("	------------------------------------------
");

    return YAMM_OK;
}

/************************************************************
 * Function:    yammForceDestroy
 * Description: YAMM 强制销毁函数
 * Input:       NULL
 * Output:      NULL
 * Return:      OK / ERROR
 * Others:      和 yammDestroy 类似，但无条件强制释放 YAMM 动态创建
 *              的资源。用于多样例测试情景，单用例测试失败可以强制清理
 *                资源重新下一用例。
 ************************************************************/
int yammForceDestroy(void)
{
    struct List_Head *iteratePos = NULL;
    struct List_Head *iterateNext = NULL;

    if (!gyammContext.memoryPoolAddress)
    {
        DBG_LOG(DBG_LVL_DEBUG, "YAMM is uninit.");
        return YAMM_ERROR;
    }
    /* 清空忙碌链表和空闲链表 */
    commonListIterateSafe(iteratePos, iterateNext, gyammContext.busyListHead)
    {
        commonListDelInit(iteratePos);
    }
    commonListIterateSafe(iteratePos, iterateNext, gyammContext.idelListHead)
    {
        commonListDelInit(iteratePos);
    }

    /* 释放整个内存空间 */
    free(gyammContext.memoryPoolAddress);

    /* 避免野指针 */
    gyammContext.memoryPoolAddress = NULL;
    gyammContext.mcbPoolAddress = NULL;
    gyammContext.busyListHead = NULL;
    gyammContext.idelListHead = NULL;

    return YAMM_OK;
}

/************************************************************
 * Function:      yammGetInfo
 * Description:   YAMM 用户数据内存状况反馈，用于测试
 * Input:         NULL
 * Output:        stats:    内存管理结点以及用户内存使用状况统计
 * Return:        NULL
 * Others:        如果 YAMM 还未创建，则 stats 所有结构体成员返回0
 ************************************************************/
void yammGetInfo(YAMM_STATS *stats)
{
    struct List_Head *iteratePos = NULL;
    MEM_CTRL_BLK *iterateMCB = NULL;
    size_t mcbIdleCount = 0;
    size_t mcbAllocatedCount = 0;
    size_t mcbFreeCount = 0;
    size_t allocatedSize = 0;
    size_t freeSize = 0;

    if (!stats)
    {
        DBG_LOG(DBG_LVL_DEBUG, "YAMM stats ptr is NULL.");
        return;
    }

    /* 内存管理器已经被初始化 */
    if (NULL != gyammContext.mcbPoolAddress)
    {
        mcbIdleCount = commonListLength(gyammContext.idelListHead);
        commonListIterate(iteratePos, gyammContext.busyListHead)
        {
            iterateMCB = commonListEntry(iteratePos, MEM_CTRL_BLK, list);
            if (TYPE_ALLOC == iterateMCB->type)
            {
                mcbAllocatedCount++;
                allocatedSize += iterateMCB->chunkSize;
            }
            else
            {
                mcbFreeCount++;
                freeSize += iterateMCB->chunkSize;
            }
        }
    }

    stats->mcbIdleCount = mcbIdleCount;
    stats->mcbAllocatedCount = mcbAllocatedCount;
    stats->mcbFreeCount = mcbFreeCount;
    stats->allocatedSize = allocatedSize;
    stats->freeSize = freeSize;
}

/************************************************************
 * Function:      yammGetHeap
 * Description:   获取 YAMM 管理用户数据内存段即堆的首地址
 * Input:         NULL
 * Output:        NULL
 * Return:        YAMM 管理用户数据内存段即堆的首地址
 * Others:        该函数主要用于编写测试数据时，可以使用相对偏移地址，
 *                实际测试时再跟进转化成绝对地址。
 ************************************************************/
void *yammGetHeap(void)
{
    return gyammContext.memoryPoolAddress;
}
    