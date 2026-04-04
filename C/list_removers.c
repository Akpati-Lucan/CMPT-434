/*
 * Student Names: Aaron DSouza, Hudson Sundbo
 * NSIDs        : aad921, hrs257
 * Student #s   : 11297151, 11292706
 * CMPT 332 Term 1 2024
 */

#include <list.h>
#include <stdio.h>

extern LIST *lists;
extern NODE *nodes;
extern NODE *freedNodes;
extern int freedListsIndices[MAX_LISTS];
extern int availableNodes;
extern int availableLists;
extern int freedListsCount;

/*
 * Remove current item from the list
 * Make the next item the current one
 *
 * Param
 *  - LIST *list: List from which to remove current item
 *
 * Returns
 *  - Item removed from list
 * */
void *ListRemove(LIST *list) {
    
    NODE *removedNode;
    void *removedItem;

    if (list == NULL) {
        return NULL;
    }
    else if (list->listSize == 0) {
        return NULL;
    }

    if (list->listSize == 1) {
        list->head = NULL;
        list->tail = NULL;
    }
    else if (list->currentNode == list->head) {
        
        list->head = list->currentNode->nextNode;
        list->head->prevNode = NULL;

    }
    else if (list->currentNode == list->tail) {

        list->tail = list->currentNode->prevNode;
        list->tail->nextNode = NULL;
    }
    else {

        list->currentNode->prevNode->nextNode = list->currentNode->nextNode;
        list->currentNode->nextNode->prevNode = list->currentNode->prevNode;
    }

    removedNode = list->currentNode;

    if (list->currentNode->nextNode != NULL) {
        list->currentNode = list->currentNode->nextNode;
    }
    else {
        list->currentNode = list->currentNode->prevNode;
    }

    removedNode->nextNode = freedNodes;
    removedNode->prevNode = NULL;
    freedNodes = removedNode;
    
    removedItem = removedNode->item;
    removedNode->item = NULL;

    availableNodes++;
    list->listSize--;

    return removedItem;
}


/*
 * Delete passed list
 *
 * Param
 *  - LIST *list: List to delete
 *  - ItemFree itemFree: function to free each item in the list
 *
 * */
void ListFree(LIST *list, ItemFree itemFree){

    NODE *currentNode;
    NODE *nextNode;

    if (list == NULL) {
        return;
    }
    else if (itemFree == NULL) {
        return;
    }

    currentNode = list->head;

    while (currentNode != NULL) {
    
        nextNode = currentNode->nextNode;

        (*itemFree)(currentNode->item);

        currentNode->nextNode = freedNodes;
        currentNode->prevNode = NULL;
        currentNode->item = NULL;
        freedNodes = currentNode;

        availableNodes++;

        currentNode = nextNode;
        
    }

    list->head = NULL;
    list->tail = NULL;
    list->currentNode = NULL;
    list->listSize = 0;

    freedListsIndices[freedListsCount] = list - lists;
    freedListsCount++;
    availableLists++;

}

/*
 * Takes out the last item in the passed list
 * The current item will be the new last item in the list
 *
 * Params
 *  - LIST *list: List to trim
 *
 * Returns:
 *  - Item removed from the list
 * */
void *ListTrim(LIST *list) {
    
    NODE *removedNode;
    void *removedItem;

    if (list == NULL) {
        return NULL;
    }
    else if (list->listSize == 0) {
        return NULL;
    }

    removedNode = list->tail;

    if (list->listSize == 1) {
        list->head = NULL;
        list->tail = NULL;
    }
    else {

        list->tail = list->tail->prevNode;
        list->tail->nextNode = NULL;
    }

    removedNode->nextNode = freedNodes;
    removedNode->prevNode = NULL;
    freedNodes = removedNode;
    
    removedItem = removedNode->item;
    removedNode->item = NULL;

    list->currentNode = list->tail;

    availableNodes++;
    list->listSize--;

    return removedItem;

}
