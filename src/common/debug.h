#ifndef __DEBUG_H_
#define __DEBUG_H_

#define EQUAL_STR "=============================================="
#define MINUS_STR "----------------------------------------------"

#define DEBUG 1

#if DEBUG == 1
#define DEBUG_INFO(format, ...) printf(format, ##__VA_ARGS__)
#define DEBUG_ERROR_POSITION printf("Error:[file %s],[line %d],[function %s]\n", \
                                    __FILE__, __LINE__, __func__)
#define DBG_LVL_INFO printf("[INFO]     [%s]-[%s:%d] ", __FILE__, __func__, __LINE__)
#define DBG_LVL_DEBUG printf("[DEBUG]    [%s]-[%s:%d] ", __FILE__, __func__, __LINE__)
#define DBG_LVL_ERROR printf("[ERROR]    [%s]-[%s:%d] ", __FILE__, __func__, __LINE__)
#define DBG_LOG(dbg_lvl, format, ...) { dbg_lvl; printf(format, ##__VA_ARGS__); printf("\n"); }
#else
#define DEBUG_INFO(format, ...)
#define DEBUG_ERROR_POSITION
#define DBG_LVL_DEBUG
#define DBG_LVL_ERROR
#define DBG_LOG
#endif

#endif /* __DEBUG_H_ */
