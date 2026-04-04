/*
 * Student Names: Aaron DSouza, Hudson Sundbo
 * NSIDs        : aad921, hrs257
 * Student #s   : 11297151, 11292706
 * CMPT 332 Term 1 2024
 */

#include <list.h>
#include <stdio.h>
#include <stdlib.h>

/* Global variables */
LIST *lists = NULL;
NODE *nodes = NULL;
NODE *freedNodes = NULL;
int freedListsIndices[MAX_LISTS];
int availableNodes = MAX_NODES;
int availableLists = MAX_LISTS;
int freedListsCount = 0;

/*
 * Makes a new, empty list
 * 
 * Returns:
 *  reference to created list on success
 *  NULL on failure
 * */
LIST *ListCreate() {

    LIST *createdList;
    int freedListIndex = 0;

    /* Initialized pool of lists and nodes */
    if (lists == NULL || nodes == NULL) {
        
        lists = (LIST*) malloc(MAX_LISTS * sizeof(LIST));
        if (lists == NULL) {
            printf("Failed to allocate memory pool for lists\n");
            return NULL;
        }

        nodes = (NODE*) malloc(MAX_NODES * sizeof(NODE));
        if (nodes == NULL) {
            printf("Failed to allocate memory pool for nodes\n");
            return NULL;
        }
    }

    /* Check if all lists are used up */
    if (availableLists == 0) {
        return NULL;
    }

    /* Check if any freed lists can be reused */
    else if (freedListsCount > 0) { 
        freedListIndex = freedListsIndices[freedListsCount - 1];
        createdList = &lists[freedListIndex];
        freedListsCount --;
    }
    else {
        createdList = &lists[MAX_LISTS - availableLists];
    }

    availableLists--;

    /* Initialize members of empty list */
    createdList->head = NULL;
    createdList->tail = NULL;
    createdList->currentNode = NULL;
    createdList->listSize = 0;

    /*printf("Got to procedure ListCreate()\n");*/

    return createdList;

}


/*
 * Adds new item to list directly after current item
 * The new item becomes the current item
 *
 * Params
 *  LIST *list = Reference to list
 *  void *item = New item to be added to the passed list
 *
 * Returns
 *  - Success: 0
 *  - Failure: -1
 * */
int ListAdd(LIST *list, void *item) {

    NODE *createdNode, *currentNode;

    /* Check parameters for errors */
    if (list == NULL) {
        /* printf("Error in procedure ListAdd(): invalid parameter *list\n");*/
        return -1;
    }
    else if (item == NULL) {
        /*printf("Error in procedure ListAdd(): invalid parameter *item\n");*/
        return -1;
    }

    /* Check if all lists are used up */
    if (availableNodes == 0) {
        return -1;
    }

    /* Check if there is a freedNode that can be reused */
    else if (freedNodes != NULL) {
        createdNode = freedNodes;
        freedNodes = freedNodes->nextNode;
        createdNode->nextNode = NULL;
    }

    else {
        createdNode = &nodes[MAX_NODES - availableNodes];
    } 

    /* Initialize new node with item */
    createdNode->item = item;

    currentNode = list->currentNode;

    /* When the list is empty */
    if (list->listSize == 0) {
        createdNode->nextNode = NULL;
        createdNode->prevNode = NULL;

        list->head = createdNode;
        list->tail = createdNode;
    }

    /* When the currentNode is the last item in the list */
    else if (currentNode->nextNode == NULL){
        
        createdNode->nextNode = NULL;
        createdNode->prevNode = currentNode;

        currentNode->nextNode = createdNode;

        list->tail = createdNode;
    }

    /* When the currentNode is in the middle of the list */
    else{
        createdNode->nextNode = currentNode->nextNode;
        createdNode->prevNode = currentNode;

        currentNode->nextNode->prevNode = createdNode;
        currentNode->nextNode = createdNode;
    }

    /* Make added node the currentNode */
    list->currentNode = createdNode;

    list->listSize ++;
    availableNodes --;

    /*printf("Got to procedure ListAdd()\n");*/

    return (0);
}


/*
 * Adds new item to list directly before current item
 * The new item becomes the current item
 *
 * Params
 *  LIST *list = Reference to list
 *  void *item = New item to be added to the passed list
 *
 * Returns
 *  - Success: 0
 *  - Failure: -1
 * */
int ListInsert(LIST *list, void *item) {
    
    NODE *createdNode, *currentNode;

    /* Check parameters for errors */
    if (list == NULL) {
        /*printf("Error in procedure ListAdd(): invalid parameter *list\n");*/
        return -1;
    }
    else if (item == NULL) {
        /*printf("Error in procedure ListAdd(): invalid parameter *item\n");*/
        return -1;
    }

    /* Check if all lists are used up */
    if (availableNodes == 0) {
        return -1;
    }

    /* Check if there is a freedNode that can be reused */
    else if (freedNodes != NULL) {
        createdNode = freedNodes;
        freedNodes = freedNodes->nextNode;
        createdNode->nextNode = NULL;
    }

    else {
        createdNode = &nodes[MAX_NODES - availableNodes];
    } 

    /* Initialize new node with item */
    createdNode->item = item;

    currentNode = list->currentNode;

    /* When the list is empty */
    if (list->listSize == 0) {
        createdNode->nextNode = NULL;
        createdNode->prevNode = NULL;

        list->head = createdNode;
        list->tail = createdNode;
    }

    /* When the currentNode is the first item in the list */
    else if (currentNode->prevNode == NULL){
        
        createdNode->prevNode = NULL;
        createdNode->nextNode = currentNode;
        
        currentNode->prevNode = createdNode;
        list->head = createdNode;
    }

    /* When the currentNode is in the middle of the list */
    else{
        createdNode->prevNode = currentNode->prevNode;
        createdNode->nextNode = currentNode;

        currentNode->prevNode->nextNode = createdNode;
        currentNode->prevNode = createdNode;
    }

    /* Make added node the currentNode */
    list->currentNode = createdNode;

    list->listSize ++;
    availableNodes --;
    

    return (0);
    
}


/*
 * Adds new item to end of the list
 * The new item becomes the current item
 *
 * Params
 *  LIST *list = Reference to list
 *  void *item = New item to be added to the passed list
 *
 * Returns
 *  - Success: 0
 *  - Failure: -1
 * */
int ListAppend(LIST *list, void *item) {

    NODE *createdNode;

    /* Check parameters for errors */
    if (list == NULL) {
        /*printf("Error in procedure ListAdd(): invalid parameter *list\n");*/
        return -1;
    }
    else if (item == NULL) {
        /*printf("Error in procedure ListAdd(): invalid parameter *item\n");*/
        return -1;
    }

    /* Check if all lists are used up */
    if (availableNodes == 0) {
        return -1;
    }

    /* Check if there is a freedNode that can be reused */
    else if (freedNodes != NULL) {
        createdNode = freedNodes;
        freedNodes = freedNodes->nextNode;
        createdNode->nextNode = NULL;
    }

    else {
        createdNode = &nodes[MAX_NODES - availableNodes];
    } 

    /* Initialize new node with item */
    createdNode->item = item;

    /* When the list is empty */
    if (list->listSize == 0) {
        createdNode->nextNode = NULL;
        createdNode->prevNode = NULL;

        list->head = createdNode;
    }
    else {
        createdNode->nextNode = NULL;
        createdNode->prevNode = list->tail;

        list->tail->nextNode = createdNode;
    }

    list->tail = createdNode;

    /* Make added node the currentNode */
    list->currentNode = createdNode;

    list->listSize ++;
    availableNodes --;

    /*printf("Got to procedure ListAppend()\n");*/
    
    return (0);
}


/*
 * Adds new item to front of the list
 * The new item becomes the current item
 *
 * Params
 *  LIST *list = Reference to list
 *  void *item = New item to be added to the passed list
 *
 * Returns
 *  - Success: 0
 *  - Failure: -1
 * */
int ListPrepend(LIST *list, void *item) {

    NODE *createdNode;

    /* Check parameters for errors */
    if (list == NULL) {
        /*printf("Error in procedure ListAdd(): invalid parameter *list\n");*/
        return -1;
    }
    else if (item == NULL) {
        /*printf("Error in procedure ListAdd(): invalid parameter *item\n");*/
        return -1;
    }

    /* Check if all lists are used up */
    if (availableNodes == 0) {
        return -1;
    }

    /* Check if there is a freedNode that can be reused */
    else if (freedNodes != NULL) {
        createdNode = freedNodes;
        freedNodes = freedNodes->nextNode;
        createdNode->nextNode = NULL;
    }

    else {
        createdNode = &nodes[MAX_NODES - availableNodes];
    } 

    /* Initialize new node with item */
    createdNode->item = item;

    /* When the list is empty */
    if (list->listSize == 0) {
        createdNode->nextNode = NULL;
        createdNode->prevNode = NULL;

        list->tail = createdNode;
    }
    else {
        createdNode->nextNode = list->head;
        createdNode->prevNode = NULL;

        list->head->prevNode = createdNode;
    }

    list->head = createdNode;

    /* Make added node the currentNode */
    list->currentNode = createdNode;

    list->listSize ++;
    availableNodes --;

    /*printf("Got to procedure ListPrepend()\n");*/
    
    return (0);
}


/*
 * Adds list2 to the end of list1 
 * list2 no longer exists after this operation
 *
 * Params
 *  LIST *list1 = Reference to list that will added onto
 *  LIST *list2 = Reference to list that will be added to list1
 *
 * */
void ListConcat(LIST *list1, LIST *list2) {

    if (list1 == NULL) {
        printf("Error in procedure ListConcat(): invalid parameter *list1\n");
        return;
    }
    else if (list2 == NULL) {
        printf("Error in procedure ListConcat(): invalid parameter *list2\n");
        return;
    }
    /* List1 is empty */
    else if (list1->listSize == 0 && list2->listSize > 0) {
        list1->head = list2->head;
        list1->tail = list2->tail;
        list1->currentNode = list2->currentNode;
    }
    /* Both lists have items */
    else if (list1->listSize > 0 && list2->listSize > 0) {

        list1->tail->nextNode = list2->head;
        list2->head->prevNode = list1->tail;
        list1->tail = list2->tail;
    }

    list1->listSize += list2->listSize;


    /* Free list2 */
    list2->head = NULL;
    list2->tail = NULL;
    list2->currentNode = NULL;
    list2->listSize = 0;

    freedListsIndices[freedListsCount] = list2 - lists;
    freedListsCount++;
    availableLists++;

    /*printf("Got to procedure ListConcat()\n");*/
    
}

