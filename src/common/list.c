#include <stdio.h>

#include "debug.h"
#include "list.h"

void commonListInit(struct List_Head *list)
{
	list->prev = list;
	list->next = list;
}

/* 将new节点添加到prev和next节点之间 */
static inline void commonListAddAction(struct List_Head *new, struct List_Head *prev, struct List_Head *next)
{
	next->prev = new;
	new->next = next;
	new->prev = prev;
	prev->next = new;
}

/* 将new节点添加到head节点后面 */
void commonListAdd(struct List_Head *new, struct List_Head *head)
{
	commonListAddAction(new, head, head->next);
}

/* 将new节点添加到head节点前面 */
void commonListAddTail(struct List_Head *new, struct List_Head *head)
{
	commonListAddAction(new, head->prev, head);
}

/* prev 是某节点的前继节点，next 是某节点的后继节点 */
static inline void commonListDelAction(struct List_Head *prev, struct List_Head *next)
{
	prev->next = next;
	next->prev = prev;
}

/* 删除entry节点 */
void commonListDel(struct List_Head *entry)
{
	commonListDelAction(entry->prev, entry->next);
}

/* 删除entry节点，并将其前继指针和后继指针都指向它本身 */
void commonListDelInit(struct List_Head *entry)
{
	commonListDelAction(entry->prev, entry->next);
	commonListInit(entry);
}

/* 链表非空判断 */
int commonListEmtpy(const struct List_Head *head)
{
	return head->next == head;
}

/* 求双向链表长度 */
unsigned int commonListLength(struct List_Head *head)
{
	struct List_Head *pos = NULL;
	unsigned int length = 0;

	commonListIterate(pos, head)
	{
		length++;
	}
	return length;
}

/* 销毁链表 */
void commonListDestroy(struct List_Head *head)
{
	struct List_Head *pos = NULL;
	struct List_Head *next = NULL;

	commonListIterateSafe(pos, next, head)
	{
		commonListDelInit(pos);
	}
}

/* 以下代码用于测试通用双向链表 */
#if 0
typedef struct Person
{
	int age;
	char name[20];
	struct List_Head list;
} PERSON;

int main(void)
{
	PERSON *personPtr = NULL;
	PERSON personHead;
	struct List_Head *pos = NULL;
	struct List_Head *next = NULL;
	int i = 0;

	commonListInit(&personHead.list);

	for (i = 0; i < 5; i++)
	{
		personPtr = (PERSON *)malloc(sizeof(PERSON));
		personPtr->age = i+10;
		snprintf(personPtr->name, 5, "abc%d", i);
		commonListAdd(&personPtr->list, &personHead.list);
	}
	printf("list iterator 1st
");
	commonListIterate(pos, &personHead.list)
	{
		personPtr = commonListEntry(pos, PERSON, list);
		printf("person age: %d, person name: %s
", personPtr->age, personPtr->name);
	}

	/* 删除节点中年龄为12岁的人 */
	commonListIterateSafe(pos, next, &personHead.list)
	{
		personPtr = commonListEntry(pos, PERSON, list);
		if (12 == personPtr->age)
		{
			commonListDelInit(pos);
			free(personPtr);
		}
	}

	printf("list iterator 2nd
");
	commonListIterate(pos, &personHead.list)
	{
		personPtr = commonListEntry(pos, PERSON, list);
		printf("person age%d, person name%s
", personPtr->age, personPtr->name);
	}

	/* 销毁链表 */
	commonListIterateSafe(pos, next, &personHead.list)
	{
		personPtr = commonListEntry(pos, PERSON, list);
		commonListDelInit(pos);
		free(personPtr);
	}

	printf("is empty:%d
", commonListEmtpy(&personHead.list));
	printf("list length:%d
", commonListLength(&personHead.list));

    return 0;
}
#endif