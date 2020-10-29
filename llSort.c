// llSort.c

// standard includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>
#include <sys/random.h>

// types
#include <stdint.h>
#include <stdbool.h>
typedef uint8_t   u8;
typedef int8_t    s8;
typedef uint16_t  u16;
typedef int16_t   s16;
typedef uint32_t  u32;
typedef int32_t   s32;
typedef uint64_t  u64;
typedef int64_t   s64;

typedef struct LinkedList LinkedList;

typedef struct LinkedList
{
	LinkedList *next;
	s64         data;
} LinkedList;

static struct PrngType
{
	u64 bytesProduced;
	u8 i, j;
	u8 s[256];
} Prng;

static u64 comparisionCount;

static void
randomnessInit(void)
{
	u32 i;
	u32 res;
	u8  k[256];
	u8  t;
	Prng.j = 0;
	Prng.i = 0;
	do {
		res = getrandom(&k, 256, 0);
	} while (res != 256);
	
	for (i=0; i<256; i++)
	{
		Prng.s[i] = (u8)i;
	}
	
	for (i=0; i<256; i++)
	{
		Prng.j += Prng.s[i] + k[i];
		t = Prng.s[Prng.j];
		Prng.s[Prng.j] = Prng.s[i];
		Prng.s[i] = t;
	}
}

static void
fillWithRandom(void *pBuff, u32 size)
{
	u8 *cursor = pBuff;
	u8  t;
	
	if (Prng.bytesProduced > 0x40000000)
	{
		randomnessInit();
	}
	
	Prng.bytesProduced += size;
	
	do {
		Prng.i++;
		t = Prng.s[Prng.i];
		Prng.j += t;
		Prng.s[Prng.i] = Prng.s[Prng.j];
		Prng.s[Prng.j] = t;
		t += Prng.s[Prng.i];
		*cursor = Prng.s[t];
		cursor++;
		size--;
	} while (size);
}

static LinkedList *
makeLinkedList(s64 sizeOfList)
{
	LinkedList *newNode;
	LinkedList *start;
	LinkedList *list;
	
	if (sizeOfList == 0)
	{
		return 0;
	}
	
	start = malloc(sizeof(LinkedList));
	list = start;
	fillWithRandom(&start->data, 8);
	start->next = 0;
	sizeOfList--;
	
	while(sizeOfList)
	{
		newNode = malloc(sizeof(LinkedList));
		fillWithRandom(&newNode->data, 8);
		newNode->next = 0;
		list->next = newNode;
		list = newNode;
		sizeOfList--;
	}
	return start;
}

static LinkedList *
makeSortedLinkedList(s64 sizeOfList)
{
	LinkedList *newNode;
	LinkedList *start;
	LinkedList *list;
	u64         value = 0;
	
	if (sizeOfList == 0)
	{
		return 0;
	}
	
	start = malloc(sizeof(LinkedList));
	list = start;
	start->data = value;
	value++;
	start->next = 0;
	sizeOfList--;
	
	while(sizeOfList)
	{
		newNode = malloc(sizeof(LinkedList));
		newNode->data = value;
		value++;
		newNode->next = 0;
		list->next = newNode;
		list = newNode;
		sizeOfList--;
	}
	return start;
}

static u64
getListSize(LinkedList *list)
{
	u64 size = 0;
	while(list){
		size++;
		list = list->next;
	}
	return size;
}

static u64
getListSizeRI(LinkedList *list, u64 size)
{
	if (list == 0)
	{
		return size;
	}
	size++;
	getListSizeRI(list, size);
}

static u64
getListSizeR(LinkedList *list)
{
	return getListSizeRI(list, 0);
}

typedef struct HeadTail
{
	LinkedList *head;
	LinkedList *tail;
} HeadTail;

static HeadTail
sortListRecursiveCompact(LinkedList *list, u64 size)
{
	HeadTail     listHeadTail;
	HeadTail     leftHeadTail;
	HeadTail     rightHeadTail;
	LinkedList  *start;
	LinkedList **sorted;
	LinkedList  *left;
	LinkedList  *right;
	u64          leftSize;
	u64          rightSize;
	
	// base case of 1
	if (size <= 1)
	{
		listHeadTail.head = list;
		listHeadTail.tail = list->next;
		list->next = 0;
		return listHeadTail;
	}

	leftSize = size/2;
	rightSize = size - leftSize;
	
	leftHeadTail = sortListRecursiveCompact(list, leftSize);
	rightHeadTail = sortListRecursiveCompact(leftHeadTail.tail, rightSize);
	
	listHeadTail.tail = rightHeadTail.tail;
	
	sorted = &start;
	
	left =  leftHeadTail.head;
	right = rightHeadTail.head;
	
	// merge sort
	while(1)
	{
		comparisionCount++;
		if (left->data <= right->data)
		{
			*sorted = left;
			sorted = &left->next;
			left = left->next;
			if (left==0)
			{
				*sorted = right;
				break;
			}
		} else {
			*sorted = right;
			sorted = &right->next;
			right = right->next;
			if (right==0)
			{
				*sorted = left;
				break;
			}
		}
	}
	// save off new head
	listHeadTail.head = start;
	return listHeadTail;
}

#define SKIP_MERGE_IF_SORTED

static HeadTail
sortListRecursive(LinkedList *list, u64 size)
{
	HeadTail    listHeadTail;
	HeadTail    leftHeadTail;
	HeadTail    rightHeadTail;
	LinkedList *sorted;
	LinkedList *left;
	LinkedList *right;
	LinkedList *tailNext;
	u64         leftSize;
	u64         rightSize;
	
	// base case of 0 or 1
	if (size <= 1)
	{
		listHeadTail.head = list;
		listHeadTail.tail = list;
		return listHeadTail;
	}

	leftSize = size/2;
	rightSize = size - leftSize;
	
	leftHeadTail = sortListRecursive(list, leftSize);
	rightHeadTail = sortListRecursive(leftHeadTail.tail->next, rightSize);
	
	#ifdef SKIP_MERGE_IF_SORTED

	comparisionCount++;
	if (leftHeadTail.tail->data <= rightHeadTail.head->data)
	{
		leftHeadTail.tail->next = rightHeadTail.head;
		listHeadTail.head = leftHeadTail.head;
		listHeadTail.tail = rightHeadTail.tail;
		return listHeadTail;
	}

	#endif
	
	tailNext = rightHeadTail.tail->next;
	
	left =  leftHeadTail.head;
	right = rightHeadTail.head;
	
	// sort first nodes
	// this initializes the sorted variable and listHeadTail.head correctly
	comparisionCount++;
	if (left->data <= right->data)
	{
		sorted = left;
		listHeadTail.head = left;
		left = left->next;
		leftSize--;
		if (leftSize==0)
		{
			sorted->next = right;
			listHeadTail.tail = rightHeadTail.tail;
			return listHeadTail;
		}
	} else {
		sorted = right;
		listHeadTail.head = right;
		right = right->next;
		rightSize--;
		if (rightSize==0)
		{
			sorted->next = left;
			listHeadTail.tail = leftHeadTail.tail;
			listHeadTail.tail->next = tailNext;
			return listHeadTail;
		}
	}
	
	// sort the rest
	while(1)
	{
		comparisionCount++;
		if (left->data <= right->data)
		{
			sorted->next = left;
			sorted = left;
			left = left->next;
			leftSize--;
			if (leftSize==0)
			{
				sorted->next = right;
				listHeadTail.tail = rightHeadTail.tail;
				return listHeadTail;
			}
		} else {
			sorted->next = right;
			sorted = right;
			right = right->next;
			rightSize--;
			if (rightSize==0)
			{
				sorted->next = left;
				listHeadTail.tail = leftHeadTail.tail;
				listHeadTail.tail->next = tailNext;
				return listHeadTail;
			}
		}
	}
}

/*******************************************************************************
 * handels special cases:
 * 0    0 = 0
 * 0    1 = 0
 * list 0 = list
 * list 1 = list
 * will break if there is a list but size is wrong
*******************************************************************************/
static LinkedList *
sortList(LinkedList *list, u64 size)
{
	return sortListRecursive(list, size).head;
}

/*******************************************************************************
 * handels special cases:
 * 0    0 = NEEDS GUARD OR WOULD CRASH
 * 0    1 = NEEDS GUARD OR WOULD CRASH
 * list 0 = list
 * list 1 = list
 * will break if there is a list but size is wrong
*******************************************************************************/
static LinkedList *
sortListCompact(LinkedList *list, u64 size)
{
	if (list == 0)
	{
		return 0;
	}
	
	return sortListRecursiveCompact(list, size).head;
}

static void
sortedVerify(LinkedList *list)
{
	while (list)
	{
		if (list->next)
		{
			if (list->data > list->next->data)
			{
				printf("ERROR: NOT SORTED CORRECTLY!!!\n");
			}
		}
		list = list->next;
	}
}

#define LIST_SIZE 34000000

int main(int argc, char **argv)
{
	randomnessInit();
	LinkedList *list;
	list = makeLinkedList(LIST_SIZE);
	//list = makeSortedLinkedList(LIST_SIZE);
	u64 listSize = getListSize(list);
	
	printf("list size = %ld.\n", listSize);
	
	list = sortList(list, listSize);
	//list = sortListCompact(list, listSize);
	
	sortedVerify(list);
	
	listSize = getListSize(list);
	
	printf("list size after sort = %ld.\n", listSize);
	
	printf("Comparision count = %ld.\n", comparisionCount);
	
	return 0;
}

