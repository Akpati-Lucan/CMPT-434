/*
 * Student Names: Aaron DSouza, Hudson Sundbo
 * NSIDs        : aad921, hrs257
 * Student #s   : 11297151, 11292706
 * CMPT 332 Term 1 2024
 */

#include <list.h>
#include <stdio.h>


/*
 * Makes the first item the current item
 *
 * Param
 *  - LIST *list = list in which to move current item
 *
 * Returns
 *  - Reference to first item
 * */
void *ListFirst(LIST *list) {
    
    if (list == NULL) {
        return NULL;
    }
    else if (list->listSize == 0) {
        return NULL;
    }

    /* Make current node list head */
    list->currentNode = list->head;

    return (list->currentNode->item);

}


/*
 * Makes the last item the current item
 *
 * Param
 *  - LIST *list = list in which to move current item
 *
 * Returns
 *  - Reference to last item
 * */
void *ListLast(LIST *list) {

    if (list == NULL) {
        return NULL;
    }
    else if (list->listSize == 0) {
        return NULL;
    }

    /* Make current node list tail */
    list->currentNode = list->tail;

    return (list->currentNode->item);

}


/*
 * Advances list's current node by one
 *
 * Param
 *  - LIST *list = list in which to move current item
 *
 * Returns
 *  - Reference to new current item
 * */
void *ListNext(LIST *list) {
    
    if (list == NULL) {
        return NULL;
    }
    else if (list->listSize == 0) {
        return NULL;
    }
    else if (list->currentNode == list->tail) {
        return NULL;
    }

    /* Advances current node */    
    list->currentNode = list->currentNode->nextNode;

    return (list->currentNode->item);

}


 /*
 * Backs up list's current node by one
 *
 * Param
 *  - LIST *list = list in which to move current item
 *
 * Returns
 *  - Reference to new current item
 * */

void *ListPrev(LIST *list) {

    if (list == NULL) {
        return NULL;
    }
    else if (list->listSize == 0) {
        return NULL;
    }
    else if (list->currentNode == list->head) {
        return NULL;
    }

    /* Back up current node by one*/
    list->currentNode = list->currentNode->prevNode;

    return (list->currentNode->item);

}


/*
 * Gets current item in the list
 *
 * Param
 *  - LIST *list = list for which to get current item
 *
 * Returns
 *  - Reference to current item
 * */

void *ListCurr(LIST *list) {

    if (list == NULL) {
        return NULL;
    }
    
    /* list is empty */
    else if (list->listSize == 0) {
        return NULL;
    }

    return list->currentNode->item;

}


/*
 * Get passed list's count
 *
 * Param
 *  - LIST *list = list for which to get count
 *
 * Returns
 *  - Count of passed list
 * */

int ListCount(LIST *list) {

    if (list == NULL) {
        return -1;
    }

    return list->listSize;

}


/*
 * Search list for a match to the passed item
 * The cursor is moved to the match if it is found
 * The cursor is moved to the end of the list if a match was not found
 *
 * Param
 *  - LIST *list = list to search
 *  - Comparator comparator = function to be used in in the comparison during
 *                            the search
 *  - void *comparisonArg  = item being searched for in the passed list
 *
 * Returns
 *  - Reference to matching item if it was found in the list
 *  - NULL if a matching item was not found in the list
 * */

void *ListSearch(LIST *list, Comparator comparator, void* comparisonArg) {
    
    if (list == NULL) {
        return NULL;
    }
    else if (comparator == NULL) {
        return NULL;
    }
    else if (comparisonArg == NULL) {
        return NULL;
    }
    else if (list->listSize == 0) {
        return NULL;
    }

    list->currentNode = list->head;

    do {
        
        if (comparator(list->currentNode->item, comparisonArg)) {
            return list->currentNode->item;
        }
        else if (list->currentNode != list->tail) {
            list->currentNode = list->currentNode->nextNode;
        }

    }
    while (list->currentNode != list->tail);

    return NULL;

}

