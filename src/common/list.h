#ifndef __LIST_H_
#define __LIST_H_

/* 遍历链表，修改节点可能导致循环出错 */
#define commonListIterate(pos, head) \
	for (pos = (head)->next; pos != (head); pos = pos->next)

/* 遍历链表，删除修改节点不会导致循环出错 */
#define commonListIterateSafe(pos, n, head) \
	for (pos = (head)->next, n = pos->next; pos != (head); \
		pos = n, n = pos->next)

/* 获取TYPE中MEMBER的偏移量，0地址指向的成员的地址就是该成员的偏移量 */
#define offsetof(TYPE, MEMBER) ((size_t)&((TYPE *)0)->MEMBER)

/* 获取成员变量指针的类型,指向成员变量地址ptr
 * 利用成员变量地址 - 成员变量偏移量，得到结构体首地址
 */
#define container_of(ptr, type, member)({ \
		const typeof(((type *) 0)->member) * mptr = (ptr); \
		(type *)((char *)mptr - offsetof(type, member)); })

#define commonListEntry(ptr, type, member) container_of(ptr, type, member)

struct List_Head
{
	struct List_Head *next;
	struct List_Head *prev;
};

void commonListInit(struct List_Head *list);
void commonListAdd(struct List_Head *new, struct List_Head *head);
void commonListAddTail(struct List_Head *new, struct List_Head *head);
void commonListDel(struct List_Head *entry);
void commonListDelInit(struct List_Head *entry);
int commonListEmtpy(const struct List_Head *head);
unsigned int commonListLength(struct List_Head *head);

#endif /* __LIST_H_ */