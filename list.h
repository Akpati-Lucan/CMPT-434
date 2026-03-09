/*
 * Student Names: Aaron DSouza, Hudson Sundbo
 * NSIDs        : aad921, hrs257
 * Student #s   : 11297151, 11292706
 * CMPT 332 Term 1 2024
 */

#ifndef __LIST_H__
#define __LIST_H__

/* Constants */
#define MAX_LISTS 10
#define MAX_NODES 1000

/* Structure Definitions */
typedef struct _node {
    void *item;
    struct _node *prevNode;
    struct _node *nextNode;
} NODE;

typedef struct _list {
    NODE *head;
    NODE *tail;
    NODE *currentNode;
    int listSize;
} LIST;

/* Function Pointer Defs */
typedef int (*Comparator)(void *item, void* comparisonArg);
typedef void (*ItemFree)(void *itemToBeFreed);

/* Function Prototypes*/
LIST *ListCreate();
int ListAdd(LIST *list, void *item);
int ListInsert(LIST *list, void *item);
int ListAppend(LIST *list, void *item);
int ListPrepend(LIST *list, void *item);
void ListConcat(LIST *list1, LIST *list2);
void *ListFirst(LIST *list);
void *ListLast(LIST *list);
void *ListNext(LIST *list);
void *ListPrev(LIST *list);
void *ListCurr(LIST *list);
int ListCount(LIST *list);
void *ListSearch(LIST *list, Comparator comparator, void *comparisonArg);
void *ListRemove(LIST *list);
void ListFree(LIST *list, ItemFree itemFree);
void *ListTrim(LIST *list);

#endif
