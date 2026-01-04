#ifndef __YAMM_H_
#define __YAMM_H_

#include <stdio.h>
#include "testFrame.h"

#define MAX_HEAP_SIZE 0x4000
#define MCB_RATIO 16
#define ALIGN_BYTE 8

#define YAMM_OK 0
#define YAMM_ERROR -1
#define TRUE 1
#define FALSE 0

/* 向上取整，整除不加1,非整除加1 */
#define CEIL(len, size) ((int)((len) + (size) - 1) / (size))

typedef enum _MCB_TYPE
{
    TYPE_ALLOC = '2',
    TYPE_FREE = '1',
    TYPE_IDEL = '0',
} MCB_TYPE;

/* 功能接口 */
int yammInit(size_t size);
void *yammAlloc(size_t size);
int yammFree(void *ptr);
int yammDestroy(void);
int yammDump(void);

/* 测试接口 */
int yammForceDestroy(void);
void yammGetInfo(YAMM_STATS *stats);
void *yammGetHeap(void);

#endif /* __YAMM_H_ */