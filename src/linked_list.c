/*
This file is part of confetti.

confetti is free software: you can redistribute it and/or modify it 
under the terms of the GNU Lesser General Public License as published 
by the Free Software Foundation, either version 3 of the License, 
or (at your option) any later version.

confetti is distributed in the hope that it will be useful, 
but WITHOUT ANY WARRANTY; without even the implied warranty 
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 

See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with confetti. 
If not, see <https://www.gnu.org/licenses/>.
*/

#include "linked_list.h"

// private function definitions

#pragma region private function definitions

/**
 * @brief Creates a new linked list node.
 *
 * This function allocates and initializes a new linked list node and its associated element.
 * It copies the provided value into a newly allocated memory region of the specified size.
 * The newly created node points to the provided `next` node.
 *
 * @param nodeOut Pointer to where the newly allocated node will be stored.
 * @param value Pointer to the data to be copied into the node's element.
 * @param size Size in bytes of the value to be copied.
 * @param next Pointer to the next node in the list.
 *
 * @return
 * - `LINKED_LIST_SUCCESS` if the node was successfully created.
 * 
 * - `LINKED_LIST_ALLOCATION_FAILURE` if memory allocation failed.
 */
static linked_list_result_t linked_list_node_create(linked_list_node_t** nodeOut, const void* const value, const uint64_t size, linked_list_node_t* const next);

/**
 * @brief Frees the memory associated with a linked list node.
 *
 * @param node Pointer to the node pointer to be freed.
 *
 * @return
 * - `LINKED_LIST_SUCCESS` if the node was successfully freed.
 * 
 * - `LINKED_LIST_INVALID_PARAMS_ERROR` if the node pointer is NULL.
 * 
 * @note sets the node to NULL and node.next to NULL, be aware of this if looping and freeing nodes.
 */
static linked_list_result_t linked_list_node_free(linked_list_node_t** node);

/**
 * @brief Retrieves a node from the linked list at the specified index.
 *
 * @param linkedList Double pointer to the linked list to retrieve the node from.
 * @param nodeOut Double pointer to where the node will be stored.
 * @param index Index of the node to retrieve.
 *
 * @return
 * - `LINKED_LIST_SUCCESS` if the node was successfully retrieved.
 * 
 * - `LINKED_LIST_NULL_ERROR` if the list is NULL.
 * 
 * - `LINKED_LIST_INDEX_OUT_OF_RANGE_ERROR` if the index is invalid.
 */
static linked_list_result_t linked_list_node_get(linked_list_t* const linkedList, linked_list_node_t** nodeOut, const int64_t index);

/**
 * @brief Creates a deep clone of a linked list node.
 *
 * This function allocates memory and creates a new node that is a deep copy of the
 * provided node. The element within the node is also cloned using. 
 *
 * @param node Pointer to the node to be cloned.
 * @param nodeOut Pointer to where the cloned node will be stored.
 *
 * @return
 * - `LINKED_LIST_SUCCESS` if the node was successfully cloned.
 * 
 * - `LINKED_LIST_INVALID_PARAMS_ERROR` if the input node is NULL.
 * 
 * - `LIST_ALLOCATION_FAILURE` if the system couldn't allocate enough memory for the operation.
 */
static linked_list_result_t linked_list_node_clone(linked_list_node_t* const node, linked_list_node_t** nodeOut);

/**
 * @brief Creates a deep clone of a linked list element.
 *
 * This function allocates and initializes a new linked list element by copying the
 * contents and size of the input element. If the element contains a value, a deep copy
 * of the value is also performed.
 *
 * @param element Pointer to the element to clone.
 * @param elementOut Pointer to where the cloned element will be stored.
 *
 * @return
 * - `LINKED_LIST_SUCCESS` if the element was successfully cloned.
 * 
 * - `LIST_ALLOCATION_FAILURE` if the system couldn't allocate enough memory for the operation.
 */
static linked_list_result_t linked_list_element_clone(linked_list_element_t* const element, linked_list_element_t** elementOut);

/**
 * @brief Sets the value of a linked list element.
 *
 * This function assigns a new value to an existing linked list element. If the new
 * value's size differs from the current one, the internal memory is reallocated
 * accordingly. The value is then copied into the element.
 *
 * @param element Pointer to the element whose value will be set.
 * @param value Pointer to the new value to be copied into the element.
 * @param size Size in bytes of the new value.
 *
 * @return
 * - `LINKED_LIST_SUCCESS` if the value was successfully set.
 * 
 * - `LINKED_LIST_ALLOCATION_FAILURE` if memory allocation or reallocation fails.
 */
static linked_list_result_t linked_list_element_set(linked_list_element_t* const element, const void* value, const uint64_t size);

/**
 * @brief Default equality comparison function for memory blocks.
 *
 * This function performs a byte-wise comparison of two memory blocks of the same size.
 * If either pointer is NULL, the function handles it with the following rules:
 * 
 * - If both `data1` and `data2` are NULL, they are considered equal.
 * 
 * - If only one is NULL, the non-null value is considered greater.
 * 
 * - If both are non-null, `memcmp` is used to compare them.
 *
 * @param data1 Pointer to the first data block.
 * @param data2 Pointer to the second data block.
 * @param size Number of bytes to compare.
 *
 * @return
 * - `0` if the blocks are equal.
 * 
 * - A negative value if `data1` is less than `data2`.
 * 
 * - A positive value if `data1` is greater than `data2`.
 */
static int32_t default_equals(const void* const data1, const void* const data2, const uint64_t size);

/**
 * @brief Merges two sorted linked list sublists into a single sorted list.
 *
 * This function performs a stable merge of two sorted singly-linked sublists,
 * using the provided equality function and sort order. The merged list maintains
 * sort order as defined by the `ascending` flag. It also outputs a pointer to
 * the last node of the merged list.
 *
 * @param firstHalf Pointer to the head of the first sorted sublist.
 * @param secondHalf Pointer to the head of the second sorted sublist.
 * @param ascending If `true`, the merge will be in ascending order; otherwise, descending.
 * @param equalityFunction Pointer to a function that compares two element values.
 * @param tailOut Pointer to where the tail of the merged list will be stored.
 *
 * @return Pointer to the head of the newly merged linked list.
 */
static linked_list_node_t* merge(
    linked_list_node_t* firstHalf, 
    linked_list_node_t* secondHalf, 
    const bool ascending,
    const linked_list_custom_equality_function_t* equalityFunction,
    linked_list_node_t** tailOut
);

/**
 * @brief Splits a linked list into two halves.
 *
 * This function takes a source linked list and splits it into two sublists,
 * assigning the heads of the two sublists to the provided pointers. The split
 * is done in such a way that the first half contains the first half of the
 * nodes, and the second half contains the remaining nodes.
 *
 * @param source Pointer to the head of the source linked list to be split.
 * @param firstHalfOut Double pointer to where the head of the first half will be stored.
 * @param secondHalfOut Double pointer to where the head of the second half will be stored.
 */
static void merge_split(linked_list_node_t* const source, linked_list_node_t** firstHalfOut, linked_list_node_t** secondHalfOut);

/**
 * @brief Sorts a linked list using the merge sort algorithm.
 *
 * This function recursively sorts a linked list by dividing it into two halves,
 * sorting each half, and then merging the sorted halves back together. The sort
 * order is determined by the `ascending` flag, and the provided equality function
 * is used to compare element values. The function also outputs a pointer to the
 * last node of the sorted list.
 *
 * @param head Pointer to the head of the linked list to be sorted.
 * @param ascending If `true`, the sort will be in ascending order; otherwise, descending.
 * @param equalityFunction Pointer to a function that compares two element values.
 * @param tailOut Pointer to where the tail of the sorted list will be stored.
 *
 * @return Pointer to the head of the newly sorted linked list.
 */
static linked_list_node_t* merge_sort(
    linked_list_node_t* const head, 
    const bool ascending,
    const linked_list_custom_equality_function_t* equalityFunction,
    linked_list_node_t** tailOut
);

/**
 * @brief Sorts a linked list using the default sorting method.
 *
 * This function sorts the linked list by calling the `merge_sort` function.
 * It updates the head and tail pointers of the linked list to reflect the
 * newly sorted order. The sort order is determined by the `ascending` flag.
 *
 * @param linkedList Pointer to the linked list to be sorted.
 * @param ascending If `true`, the sort will be in ascending order; otherwise, descending.
 *
 * @return 
 * - `LINKED_LIST_SUCCESS` if the linked list was successfully sorted.
 */
static linked_list_result_t default_sort(linked_list_t* const linkedList, const bool ascending);

#pragma endregion

// private functions

#pragma region private functions

static linked_list_result_t linked_list_node_create(linked_list_node_t** nodeOut, const void* const value, const uint64_t size, linked_list_node_t* const next) {
    linked_list_element_t* element = (linked_list_element_t*) malloc(sizeof(linked_list_element_t));

    if (element == NULL)
        return LINKED_LIST_ALLOCATION_FAILURE;

    element->size = size;
    element->value = NULL;

    if (value != NULL) {
        element->value = (void*) malloc(size);

        if (element->value == NULL) {
            free(element);
            element = NULL;

            return LINKED_LIST_ALLOCATION_FAILURE;
        }

        memcpy(element->value, value, size);
    }

    linked_list_node_t* node = (linked_list_node_t*) malloc(sizeof(linked_list_node_t));

    if (node == NULL) {
        free(element->value);
        element->value = NULL;

        free(element);
        element = NULL;
        
        return LINKED_LIST_ALLOCATION_FAILURE;
    }   

    node->next = next;
    node->element = element;

    *nodeOut = node;
    return LINKED_LIST_SUCCESS;
}


static linked_list_result_t linked_list_node_free(linked_list_node_t** node) {
    if (*node == NULL)
        return LINKED_LIST_INVALID_PARAMS_ERROR;

    linked_list_element_free(&(*node)->element);
    (*node)->next = NULL;
        
    free((*node));
    (*node) = NULL;

    return LINKED_LIST_SUCCESS;
}


static linked_list_result_t linked_list_element_clone(linked_list_element_t* const element, linked_list_element_t** elementOut) {
    linked_list_element_t* elementClone = (linked_list_element_t*)malloc(sizeof(linked_list_element_t));

    if (elementClone == NULL)
        return LINKED_LIST_ALLOCATION_FAILURE;

    elementClone->value = NULL;
    elementClone->size = element->size;

    if (element->value != NULL) {
        elementClone->value = (void*)malloc(element->size);

        if (elementClone->value == NULL)
            return LINKED_LIST_ALLOCATION_FAILURE;

        memcpy(elementClone->value, element->value, element->size);
    }

    *elementOut = elementClone;
    return LINKED_LIST_SUCCESS;
}


static linked_list_result_t linked_list_element_set(linked_list_element_t* const element, const void* value, const uint64_t size) {
    if (size != element->size) {
        if (element->value == NULL)
            element->value = (void*)malloc(size);
        else
            element->value = (void*)realloc(element->value, size);

        element->size = size;

        if (element->value == NULL)
            return LINKED_LIST_ALLOCATION_FAILURE;
    }

    memcpy(element->value, value, size);
    return LINKED_LIST_SUCCESS;
}


static linked_list_result_t linked_list_node_get(linked_list_t* const linkedList, linked_list_node_t** nodeOut, int64_t index) {
    if (linkedList == NULL)
        return LINKED_LIST_NULL_ERROR;
    else if (index >= linkedList->size || index < 0) 
        return LINKED_LIST_INDEX_OUT_OF_RANGE_ERROR;

    int64_t currentIndex = 0;

    for (linked_list_node_t* node = linkedList->head; node != NULL; node = node->next) {
        if (currentIndex == index) {
            *nodeOut = node;
            return LINKED_LIST_SUCCESS;
        }
            
        currentIndex++;
    }
}


static linked_list_result_t linked_list_node_clone(linked_list_node_t* const node, linked_list_node_t** nodeOut) {
    if (node == NULL)
        return LINKED_LIST_INVALID_PARAMS_ERROR;

    linked_list_node_t* nodeClone = (linked_list_node_t*) malloc(sizeof(linked_list_node_t));

    if (nodeClone == NULL)
        return LINKED_LIST_ALLOCATION_FAILURE;

    linked_list_result_t elementCloneResult = linked_list_element_clone(node->element, &nodeClone->element);

    if (elementCloneResult != LINKED_LIST_SUCCESS)
        return elementCloneResult;

    nodeClone->next = NULL;

    *nodeOut = nodeClone;
    return LINKED_LIST_SUCCESS;
}


static int32_t default_equals(const void* data1, const void* data2, uint64_t size) {
    if (data1 == NULL && data2 != NULL)
        return -1;
    else if (data1 != NULL && data2 == NULL) 
        return 1;
    else if (data1 == NULL && data2 == NULL)
        return 0;

    return memcmp(data1, data2, size);
}


static linked_list_node_t* merge(
    linked_list_node_t* firstHalf, 
    linked_list_node_t* secondHalf, 
    const bool ascending,
    const linked_list_custom_equality_function_t* equalityFunction,
    linked_list_node_t** tailOut
) {
    if (firstHalf == NULL) {
        linked_list_node_t* current = secondHalf;

        while (current != NULL && current->next != NULL) {
            current = current->next;
        }

        *tailOut = current;
        return secondHalf;
    }
    else if (secondHalf == NULL) {
        linked_list_node_t* current = firstHalf;

        while (current != NULL && current->next != NULL) {
            current = current->next;
        }

        *tailOut = current;
        return firstHalf;
    }

    linked_list_node_t* newHead = NULL;
    linked_list_node_t* lastMergedNode = NULL;

    int8_t equality = equalityFunction(firstHalf->element->value, secondHalf->element->value, firstHalf->element->size);
    bool takeFromFirstHalf = ascending
        ? (equality <= 0)
        : (equality >= 0);

    if (takeFromFirstHalf) {
        newHead = firstHalf;
        firstHalf = firstHalf->next;
    } else {
        newHead = secondHalf;
        secondHalf = secondHalf->next;
    }
    lastMergedNode = newHead;


    while (firstHalf != NULL && secondHalf != NULL) {
        equality = equalityFunction(firstHalf->element->value, secondHalf->element->value, firstHalf->element->size);
        takeFromFirstHalf = ascending
            ? (equality <= 0)
            : (equality >= 0);

        if (takeFromFirstHalf) {
            lastMergedNode->next = firstHalf;
            firstHalf = firstHalf->next;
        } else {
            lastMergedNode->next = secondHalf;
            secondHalf = secondHalf->next;
        }

        lastMergedNode = lastMergedNode->next;
    }

    if (firstHalf == NULL) {
        lastMergedNode->next = secondHalf;
    } 
    else if (secondHalf == NULL) {
        lastMergedNode->next = firstHalf;
    }

    while (lastMergedNode->next != NULL) {
        lastMergedNode = lastMergedNode->next;
    }
    *tailOut = lastMergedNode;
    
    return newHead;
}


static void merge_split(linked_list_node_t* const source, linked_list_node_t** firstHalfOut, linked_list_node_t** secondHalfOut) {
    if (source == NULL || source->next == NULL) {
        *firstHalfOut = source;
        *secondHalfOut = NULL;
        return;
    }

    linked_list_node_t* singleStep = source;
    linked_list_node_t* doubleStep = source->next;

    while (doubleStep != NULL && doubleStep->next != NULL) {
        singleStep = singleStep->next;
        doubleStep = doubleStep->next->next;
    }

    *firstHalfOut = source;
    *secondHalfOut = singleStep->next;
    singleStep->next = NULL;
}


static linked_list_node_t* merge_sort(
    linked_list_node_t* const head, 
    const bool ascending,
    const linked_list_custom_equality_function_t* equalityFunction,
    linked_list_node_t** tailOut
) {
    if (head == NULL || head->next == NULL) {
        *tailOut = head;
        return head;
    }
    
    linked_list_node_t* firstHalf = NULL;
    linked_list_node_t* secondHalf = NULL;

    merge_split(head, &firstHalf, &secondHalf);

    linked_list_node_t* firstHalfTail = NULL;
    linked_list_node_t* secondHalfTail = NULL;

    firstHalf = merge_sort(firstHalf, ascending, equalityFunction, &firstHalfTail);
    secondHalf = merge_sort(secondHalf, ascending, equalityFunction, &secondHalfTail);

    return merge(firstHalf, secondHalf, ascending, equalityFunction, tailOut);
}


static linked_list_result_t default_sort(linked_list_t* const linkedList, const bool ascending) {
    linked_list_node_t* newHead = NULL;
    linked_list_node_t* newTail = NULL;

    newHead = merge_sort(linkedList->head, ascending, (const linked_list_custom_equality_function_t*) linkedList->equalityFunction, &newTail);
    linkedList->head = newHead;
    linkedList->tail = newTail;

    return LINKED_LIST_SUCCESS;
}

#pragma endregion

// public functions

#pragma region public functions

linked_list_result_t linked_list_create(
    linked_list_t** linkedListOut, 
    linked_list_custom_equality_function_t* const customEqualityFunction, 
    linked_list_custom_sorting_function_t* const customSortingFunction
) {
    linked_list_t* const linkedList = (linked_list_t*) malloc(sizeof(linked_list_t));

    if (linkedList == NULL)
        return LINKED_LIST_ALLOCATION_FAILURE;

    linkedList->head = NULL;
    linkedList->tail = NULL;
    linkedList->size = 0;
    linkedList->equalityFunction = customEqualityFunction == NULL 
        ? (linked_list_custom_equality_function_t*) &default_equals 
        : customEqualityFunction;
    linkedList->sortingFunction = customSortingFunction == NULL 
        ? (linked_list_custom_sorting_function_t*) &default_sort 
        : customSortingFunction;

    *linkedListOut = linkedList;
    return LINKED_LIST_SUCCESS;
}


linked_list_result_t linked_list_free(linked_list_t** linkedList) {
    if (*linkedList == NULL)
        return LINKED_LIST_NULL_ERROR;

    linked_list_node_t* previousNode = NULL;

    for (linked_list_node_t* node = (*linkedList)->head; node != NULL; node = node->next) {
        if (previousNode != NULL) {
            linked_list_result_t nodeFreeResult = linked_list_node_free(&previousNode);

            if (nodeFreeResult != LINKED_LIST_SUCCESS)
                return nodeFreeResult;
        }

        previousNode = node;
    }

    (*linkedList)->head = NULL;
    (*linkedList)->tail = NULL;
    (*linkedList)->size = 0;

    free(*linkedList);
    *linkedList = NULL;

    return LINKED_LIST_SUCCESS;
}


linked_list_result_t linked_list_print(linked_list_t* const linkedList) {
    if (linkedList == NULL)
        return LINKED_LIST_NULL_ERROR;

    printf("[ ");

    for (linked_list_node_t* node = linkedList->head; node != NULL; node = node->next) {
        if (node->element->value != NULL)
            printf(node->next != NULL ? "%p, " : "%p", node->element->value);
        else
            printf(node->next != NULL ? "NULL, " : "NULL");
    }

    printf(" ] -> %p\n", linkedList);
    
    return LINKED_LIST_SUCCESS;
}


linked_list_result_t linked_list_get(linked_list_t* const linkedList, linked_list_element_t** elementOut, const int64_t index) {
    if (linkedList == NULL)
        return LINKED_LIST_NULL_ERROR;
    else if (index >= linkedList->size || index < 0) 
        return LINKED_LIST_INDEX_OUT_OF_RANGE_ERROR;

    int64_t currentIndex = 0;

    for (linked_list_node_t* node = linkedList->head; node != NULL; node = node->next) {
        if (currentIndex == index) {
            linked_list_element_t* elementClone = NULL;
            linked_list_result_t cloneResult = linked_list_element_clone(node->element, &elementClone);

            if (cloneResult != LINKED_LIST_SUCCESS)
                return cloneResult;

            *elementOut = elementClone;
            return LINKED_LIST_SUCCESS;
        }
            
        currentIndex++;
    }
}


linked_list_result_t linked_list_set(linked_list_t* const linkedList, const int64_t index, void* const value, const uint64_t size) {
    if (linkedList == NULL)
        return LINKED_LIST_NULL_ERROR;
    else if (index >= linkedList->size || index < 0) 
        return LINKED_LIST_INDEX_OUT_OF_RANGE_ERROR;

    int64_t currentIndex = 0;

    for (linked_list_node_t* node = linkedList->head; node != NULL; node = node->next) {
        if (currentIndex == index) {
            linked_list_result_t setResult = linked_list_element_set(node->element, value, size);

            if (setResult != LINKED_LIST_SUCCESS)
                return setResult;

            break;
        }   

        currentIndex++;
    }

    return LINKED_LIST_SUCCESS;
}


linked_list_result_t linked_list_prepend(linked_list_t* const linkedList, void* const value, const uint64_t size) {
    if (linkedList == NULL)
        return LINKED_LIST_NULL_ERROR;

    linked_list_node_t* node = NULL;
    linked_list_result_t nodeCreateResult = linked_list_node_create(&node, value, size, linkedList->head);

    if (nodeCreateResult != LINKED_LIST_SUCCESS)
        return nodeCreateResult;

    if (linkedList->head == NULL) {
        linkedList->head = node;
        linkedList->tail = node;
    }
    else 
        linkedList->head = node;

    linkedList->size++;
    return LINKED_LIST_SUCCESS;
}


linked_list_result_t linked_list_append(linked_list_t* const linkedList, void* const value, const uint64_t size) {
    if (linkedList == NULL)
        return LINKED_LIST_NULL_ERROR;

    linked_list_node_t* node = NULL;
    linked_list_result_t nodeCreateResult = linked_list_node_create(&node, value, size, NULL);

    if (nodeCreateResult != LINKED_LIST_SUCCESS)
        return nodeCreateResult;

    if (linkedList->head == NULL) {
        linkedList->head = node;
        linkedList->tail = node;
    }
    else {
        linkedList->tail->next = node;
        linkedList->tail = node;
    }

    linkedList->size++;
    return LINKED_LIST_SUCCESS;
}


linked_list_result_t linked_list_insert(linked_list_t* const linkedList, const int64_t index, void* value, const uint64_t size) {
    if (linkedList == NULL)
        return LINKED_LIST_NULL_ERROR;
    else if (index > linkedList->size || index < 0) 
        return LINKED_LIST_INDEX_OUT_OF_RANGE_ERROR;

    linked_list_node_t* newNode = NULL;
    linked_list_result_t nodeCreateResult = linked_list_node_create(&newNode, value, size, NULL);

    if (nodeCreateResult != LINKED_LIST_SUCCESS)
        return nodeCreateResult;

    if (linkedList->head == NULL) {
        linkedList->head = newNode;
        linkedList->tail = newNode;
        linkedList->size++;

        return LINKED_LIST_SUCCESS;
    }

    if (index == 0) {
        newNode->next = linkedList->head;
        linkedList->head = newNode;
        linkedList->size++;

        return LINKED_LIST_SUCCESS;
    }
    else if (index == linkedList->size) {
        linkedList->tail->next = newNode;
        linkedList->tail = newNode;
        linkedList->size++;

        return LINKED_LIST_SUCCESS;
    }

    linked_list_node_t* previousNode = NULL;
    linked_list_result_t getNodeResult = linked_list_node_get(linkedList, &previousNode, index - 1);

    if (getNodeResult != LINKED_LIST_SUCCESS)
        return getNodeResult;

    newNode->next = previousNode->next;
    previousNode->next = newNode;

    linkedList->size++;
    return LINKED_LIST_SUCCESS;
}


linked_list_result_t linked_list_remove(linked_list_t* const linkedList, const uint64_t index) {
    if (linkedList == NULL)
        return LINKED_LIST_NULL_ERROR;
    else if (index >= linkedList->size || index < 0) 
        return LINKED_LIST_INDEX_OUT_OF_RANGE_ERROR;

    if (linkedList->size == 1) {
        linked_list_result_t freeResult = linked_list_node_free(&linkedList->head);

        if (freeResult != LINKED_LIST_SUCCESS)
            return freeResult;

        linkedList->size--;
        linkedList->head = NULL;
        linkedList->tail = NULL;
        return LINKED_LIST_SUCCESS;
    }

    if (index == 0) {
        linked_list_node_t* previousHead = linkedList->head;

        linkedList->head = previousHead->next;
        linkedList->size--;

        linked_list_result_t freeResult = linked_list_node_free(&previousHead);

        if (freeResult != LINKED_LIST_SUCCESS)
            return freeResult;

        return LINKED_LIST_SUCCESS;
    }
    else if (index == linkedList->size - 1) {
        linked_list_node_t* previousTail = linkedList->tail;
        linked_list_node_t* newTail = NULL;
        linked_list_result_t getResult = linked_list_node_get(linkedList, &newTail, index - 1);

        if (getResult != LINKED_LIST_SUCCESS)
            return getResult;

        newTail->next = NULL;
        linkedList->tail = newTail;
        linkedList->size--;

        linked_list_result_t freeResult = linked_list_node_free(&previousTail);

        if (freeResult != LINKED_LIST_SUCCESS)
            return freeResult;

        return LINKED_LIST_SUCCESS;
    }

    int64_t currentIndex = 0;
    linked_list_node_t* lastNode = NULL;

    for (linked_list_node_t* current = linkedList->head; current != NULL; current = current->next) {
        if (currentIndex == index) {
            lastNode->next = current->next;
            linkedList->size--;

            linked_list_result_t freeResult = linked_list_node_free(&current);

            if (freeResult != LINKED_LIST_SUCCESS)
                return freeResult;

            return LINKED_LIST_SUCCESS;
        }

        currentIndex++;
        lastNode = current;
    }
}


linked_list_result_t linked_list_pop(linked_list_t* const linkedList, linked_list_element_t** elementOut, const uint64_t index) {
    if (linkedList == NULL)
        return LINKED_LIST_NULL_ERROR;
    else if (index >= linkedList->size || index < 0) 
        return LINKED_LIST_INDEX_OUT_OF_RANGE_ERROR;

    if (linkedList->size == 1) {
        linked_list_element_t* elementClone = NULL;
        linked_list_result_t cloneResult = linked_list_element_clone(linkedList->head->element, &elementClone);

        if (cloneResult != LINKED_LIST_SUCCESS)
            return cloneResult;

        linked_list_result_t freeResult = linked_list_element_free(&linkedList->head->element);

        if (freeResult != LINKED_LIST_SUCCESS)
            return freeResult;

        free(linkedList->head);

        linkedList->size--;
        linkedList->head = NULL;
        linkedList->tail = NULL;

        *elementOut = elementClone;
        return LINKED_LIST_SUCCESS;
    }

    if (index == 0) {
        linked_list_element_t* elementClone = NULL;
        linked_list_result_t cloneResult = linked_list_element_clone(linkedList->head->element, &elementClone);

        if (cloneResult != LINKED_LIST_SUCCESS)
            return cloneResult;

        linked_list_node_t* previousHead = linkedList->head;

        linkedList->head = previousHead->next;
        linkedList->size--;

        linked_list_result_t freeResult = linked_list_element_free(&previousHead->element);

        if (freeResult != LINKED_LIST_SUCCESS)
            return freeResult;

        free(previousHead);

        *elementOut = elementClone;
        return LINKED_LIST_SUCCESS;
    }
    else if (index == linkedList->size - 1) {
        linked_list_element_t* elementClone = NULL;
        linked_list_result_t cloneResult = linked_list_element_clone(linkedList->tail->element, &elementClone);

        if (cloneResult != LINKED_LIST_SUCCESS)
            return cloneResult;

        linked_list_node_t* previousTail = linkedList->tail;
        linked_list_node_t* newTail = NULL;
        linked_list_result_t getResult = linked_list_node_get(linkedList, &newTail, index - 1);

        if (getResult != LINKED_LIST_SUCCESS)
            return getResult;

        newTail->next = NULL;
        linkedList->tail = newTail;
        linkedList->size--;

        linked_list_result_t freeResult = linked_list_element_free(&previousTail->element);

        if (freeResult != LINKED_LIST_SUCCESS)
            return freeResult;

        free(previousTail);

        *elementOut = elementClone;
        return LINKED_LIST_SUCCESS;
    }

    int64_t currentIndex = 0;
    linked_list_node_t* lastNode = NULL;

    for (linked_list_node_t* current = linkedList->head; current != NULL; current = current->next) {
        if (currentIndex == index) {
            linked_list_element_t* elementClone = NULL;
            linked_list_result_t cloneResult = linked_list_element_clone(current->element, &elementClone);

            if (cloneResult != LINKED_LIST_SUCCESS)
                return cloneResult;

            lastNode->next = current->next;
            linkedList->size--;

            linked_list_result_t freeResult = linked_list_element_free(&current->element);

            if (freeResult != LINKED_LIST_SUCCESS)
                return freeResult;

            free(current);
            *elementOut = elementClone;
            return LINKED_LIST_SUCCESS;
        }

        currentIndex++;
        lastNode = current;
    }
}


linked_list_result_t linked_list_clear(linked_list_t* const linkedList) {
    if (linkedList == NULL)
        return LINKED_LIST_NULL_ERROR;

    linked_list_node_t* previousNode = NULL;

    for (linked_list_node_t* node = linkedList->head; node != NULL; node = node->next) {       
        if (previousNode != NULL) {
            linked_list_result_t nodeFreeResult = linked_list_node_free(&previousNode);

            if (nodeFreeResult != LINKED_LIST_SUCCESS)
                return nodeFreeResult;
        }

        previousNode = node;
    }

    linkedList->head = NULL;
    linkedList->tail = NULL;
    linkedList->size = 0;

    return LINKED_LIST_SUCCESS;
}


linked_list_result_t linked_list_clone(linked_list_t* const linkedList, linked_list_t** linkedListOut) {
    if (linkedList == NULL)
        return LINKED_LIST_NULL_ERROR;

    linked_list_t* const linkedListClone = (linked_list_t*) malloc(sizeof(linked_list_t));

    if (linkedListClone == NULL)
        return LINKED_LIST_ALLOCATION_FAILURE;

    linkedListClone->head = NULL;
    linkedListClone->tail = NULL;
    linkedListClone->size = linkedList->size;

    if (linkedList->size > 0) {
        linked_list_node_t* previousNodeClone = NULL;

        for (linked_list_node_t* node = linkedList->head; node != NULL; node = node->next) {
            linked_list_node_t* nodeClone = NULL;
            linked_list_result_t nodeCloneResult = linked_list_node_clone(node, &nodeClone);

            if (nodeCloneResult != LINKED_LIST_SUCCESS) 
                return nodeCloneResult;

            if (linkedListClone->head == NULL) 
                linkedListClone->head = nodeClone;
            else 
                previousNodeClone->next = nodeClone;
            
            previousNodeClone = nodeClone;
        }

        linkedListClone->tail = previousNodeClone;
    }

    *linkedListOut = linkedListClone;
    return LINKED_LIST_SUCCESS;
}


linked_list_result_t linked_list_join(linked_list_t* const linkedList1, linked_list_t* const linkedList2, linked_list_t** linkedListOut) {
    if (linkedList1 == NULL || linkedList2 == NULL)
        return LINKED_LIST_NULL_ERROR;

    linked_list_t* const linkedListClone = (linked_list_t*) malloc(sizeof(linked_list_t));

    if (linkedListClone == NULL)
        return LINKED_LIST_ALLOCATION_FAILURE;

    linkedListClone->head = NULL;
    linkedListClone->tail = NULL;
    linkedListClone->size = linkedList1->size + linkedList2->size;

    linked_list_node_t* previousNodeClone = NULL;

    for (linked_list_node_t* node = linkedList1->head; node != NULL; node = node->next) {
        linked_list_node_t* nodeClone = NULL;
        linked_list_result_t nodeCloneResult = linked_list_node_clone(node, &nodeClone);

        if (nodeCloneResult != LINKED_LIST_SUCCESS) 
            return nodeCloneResult;

        if (linkedListClone->head == NULL) 
            linkedListClone->head = nodeClone;
        else 
            previousNodeClone->next = nodeClone;
        
        previousNodeClone = nodeClone;
    }

    for (linked_list_node_t* node = linkedList2->head; node != NULL; node = node->next) {
        linked_list_node_t* nodeClone = NULL;
        linked_list_result_t nodeCloneResult = linked_list_node_clone(node, &nodeClone);

        if (nodeCloneResult != LINKED_LIST_SUCCESS) 
            return nodeCloneResult;

        if (linkedListClone->head == NULL) 
            linkedListClone->head = nodeClone;
        else 
            previousNodeClone->next = nodeClone;
        
        previousNodeClone = nodeClone;
    }

    linkedListClone->tail = previousNodeClone;

    *linkedListOut = linkedListClone;
    return LINKED_LIST_SUCCESS;
}


linked_list_result_t linked_list_resize(linked_list_t* const linkedList, uint64_t size) {
    if (linkedList == NULL)
        return LINKED_LIST_NULL_ERROR;

    if (size == linkedList->size) 
        return LINKED_LIST_SUCCESS;

    if (size == 0) {
        linked_list_node_t* previousNode = NULL;

        for (linked_list_node_t* node = linkedList->head; node != NULL; node = node->next) {
            if (previousNode != NULL) {
                linked_list_result_t nodeFreeResult = linked_list_node_free(&node);

                if (nodeFreeResult != LINKED_LIST_SUCCESS)
                    return nodeFreeResult;
            }

            previousNode = node;
        }

        linkedList->head = NULL;
        linkedList->tail = NULL;
    }
    else if (size > linkedList->size) {
        if (linkedList->head == NULL) {
            linked_list_node_t* node = NULL;
            linked_list_result_t nodeCreateResult = linked_list_node_create(&node, NULL, sizeof(NULL), NULL);

            linkedList->head = node;
            linkedList->tail = node;
        }

        linked_list_node_t* previousNode = linkedList->tail;

        for (int i = 0; i < size - linkedList->size; i++) {
            linked_list_node_t* node = NULL;
            linked_list_result_t nodeCreateResult = linked_list_node_create(&node, NULL, sizeof(void*), NULL);

            if (nodeCreateResult != LINKED_LIST_SUCCESS) 
                return nodeCreateResult;

            previousNode->next = node;
            previousNode = node;
        }

        linkedList->tail = previousNode;
    }
    else if (size < linkedList->size) {
        linked_list_node_t* startingNode;
        linked_list_result_t getNodeResult = size > 0 
            ? linked_list_node_get(linkedList, &startingNode, size - 1) 
            : linked_list_node_get(linkedList, &startingNode, size);

        if (getNodeResult != LINKED_LIST_SUCCESS)
            return getNodeResult;

        linked_list_node_t* previousNode = NULL;

        for (linked_list_node_t* node = startingNode->next; node != NULL; node = node->next) {
            if (previousNode != NULL) {
                linked_list_result_t nodeFreeResult = linked_list_node_free(&node);

                if (nodeFreeResult != LINKED_LIST_SUCCESS)
                    return nodeFreeResult;
            }

            previousNode = node;
        }

        startingNode->next = NULL;
    }

    linkedList->size = size;
    return LINKED_LIST_SUCCESS; 
}


linked_list_result_t linked_list_reverse(linked_list_t* const linkedList) {
    if (linkedList == NULL)
        return LINKED_LIST_NULL_ERROR;
    
    if (linkedList->head == NULL) 
        return LINKED_LIST_SUCCESS;

    linked_list_node_t* previousNode = NULL;
    linked_list_node_t* currentNode = linkedList->head;
    linked_list_node_t* nextNode = NULL;

    while (currentNode != NULL) {
        nextNode = currentNode->next;

        currentNode->next = previousNode;

        previousNode = currentNode;
        currentNode = nextNode;
    }

    linkedList->tail = linkedList->head;
    linkedList->head = previousNode;

    return LINKED_LIST_SUCCESS;
}


linked_list_result_t linked_list_includes(linked_list_t* const linkedList, void* const value, const uint64_t size) {
    if (linkedList == NULL)
        return LINKED_LIST_NULL_ERROR;

    for (linked_list_node_t* node = linkedList->head; node != NULL; node = node->next) {
        if (linkedList->equalityFunction(node->element->value, value, size) == 0) {
            return LINKED_LIST_SUCCESS;
        }
    }
    
    return LINKED_LIST_ELEMENT_NOT_FOUND_ERROR;
}


linked_list_result_t linked_list_find_first(
    linked_list_t* const linkedList, 
    int64_t* const indexOut, 
    const int64_t startFromIndex, 
    void* const value, 
    const uint64_t size
) {
    if (linkedList == NULL)
        return LINKED_LIST_NULL_ERROR;
    else if (startFromIndex >= linkedList->size || startFromIndex < 0) 
        return LINKED_LIST_INDEX_OUT_OF_RANGE_ERROR;

    int64_t index = 0;

    for (linked_list_node_t* node = linkedList->head; node != NULL; node = node->next) {
        if (index < startFromIndex) {
            index++;
            continue;
        }

        if (linkedList->equalityFunction(node->element->value, value, size) == 0) {
            *indexOut = index;
            return LINKED_LIST_SUCCESS;
        }
        index++;
    }

    *indexOut = -1;
    return LINKED_LIST_ELEMENT_NOT_FOUND_ERROR;
}


linked_list_result_t linked_list_find_last(
    linked_list_t* const linkedList, 
    int64_t* const indexOut,
    const int64_t startFromIndex, 
    void* const value, 
    const uint64_t size
) {
    if (linkedList == NULL)
        return LINKED_LIST_NULL_ERROR;
    else if (startFromIndex >= linkedList->size || startFromIndex < 0) 
        return LINKED_LIST_INDEX_OUT_OF_RANGE_ERROR;

    int64_t lastFoundIndex = -1;
    int64_t index = 0;

    for (linked_list_node_t* node = linkedList->head; node != NULL; node = node->next) {
        if (index < startFromIndex) {
            index++;
            continue;
        }

        if (linkedList->equalityFunction(node->element->value, value, size) == 0) 
            lastFoundIndex = index;

        index++;
    }

    *indexOut = lastFoundIndex;
    return lastFoundIndex == -1 ? LINKED_LIST_ELEMENT_NOT_FOUND_ERROR : LINKED_LIST_SUCCESS;
}


linked_list_result_t linked_list_sort(linked_list_t* const linkedList, const bool ascending) {
    if (linkedList == NULL) 
        return LINKED_LIST_NULL_ERROR;
    
    if (linkedList->size == 0 || linkedList->size == 1) 
        return LINKED_LIST_SUCCESS;
     
    return linkedList->sortingFunction(linkedList, ascending);
}


linked_list_result_t linked_list_swap(linked_list_t* const linkedList, const int64_t index1, const int64_t index2) {
    if (linkedList == NULL) 
        return LINKED_LIST_NULL_ERROR;
    else if (index1 >= linkedList->size || index1 < 0) 
        return LINKED_LIST_INDEX_OUT_OF_RANGE_ERROR;
    else if (index2 >= linkedList->size || index2 < 0) 
        return LINKED_LIST_INDEX_OUT_OF_RANGE_ERROR;

    if (index1 == index2) 
        return LINKED_LIST_SUCCESS;
    
    linked_list_node_t* node1Previous = NULL; 
    linked_list_node_t* node1 = linkedList->head;

    for (int i = 0; i < index1; i++) {
        node1Previous = node1;
        node1 = node1->next;
    }

    linked_list_node_t* node2Previous = NULL;
    linked_list_node_t* node2 = linkedList->head;

    for (int i = 0; i < index2; i++) {
        node2Previous = node2;
        node2 = node2->next;
    }

    if (node1Previous != NULL)
        node1Previous->next = node2;
    else 
        linkedList->head = node2;


    if (node2Previous != NULL)
        node2Previous->next = node1;
    else 
        linkedList->head = node1;

    linked_list_node_t* temp = node2->next;
    node2->next = node1->next;
    node1->next = temp;

    if (node1 == linkedList->tail)
        linkedList->tail = node2;
    else if (node2 == linkedList->tail)
        linkedList->tail = node1;
    
    return LINKED_LIST_SUCCESS;
}


linked_list_result_t linked_list_element_free(linked_list_element_t** element) {
    if (*element == NULL)
        return LINKED_LIST_INVALID_PARAMS_ERROR;

    free((*element)->value);
    (*element)->size = 0;
    (*element)->value = NULL;

    free(*element);
    (*element) = NULL;

    return LINKED_LIST_SUCCESS;
}


linked_list_result_t linked_list_iterator_create(linked_list_iterator_t** iteratorOut, linked_list_t* const list) {
    if (list == NULL)
        return LINKED_LIST_NULL_ERROR;

    linked_list_iterator_t* iterator = (linked_list_iterator_t*) malloc(sizeof(linked_list_iterator_t));

    iterator->list = list;
    iterator->node = NULL;
    iterator->index = -1;

    *iteratorOut = iterator;
    return LINKED_LIST_SUCCESS;
}


linked_list_result_t linked_list_iterator_next(linked_list_iterator_t* const iterator) {
    if (iterator == NULL)
        return LINKED_LIST_INVALID_PARAMS_ERROR;

    if (iterator->node == NULL) {
        iterator->node = iterator->list->head;
        iterator->index = 0;
    } 
    else if (iterator->node->next == NULL) {
        iterator->node = NULL;
        iterator->index = -1;

        return LINKED_LIST_INDEX_OUT_OF_RANGE_ERROR;
    }
    else {
        iterator->node = iterator->node->next;
        iterator->index++;
    }

    return LINKED_LIST_SUCCESS;
}


linked_list_result_t linked_list_iterator_rewind(linked_list_iterator_t* const iterator) {
    if (iterator == NULL)
        return LINKED_LIST_INVALID_PARAMS_ERROR;

    iterator->node = NULL;
    iterator->index = -1;

    return LINKED_LIST_SUCCESS;
}


linked_list_result_t linked_list_iterator_free(linked_list_iterator_t** iterator) {
    if (iterator == NULL)
        return LINKED_LIST_INVALID_PARAMS_ERROR;

    (*iterator)->list = NULL;
    (*iterator)->node = NULL;
    (*iterator)->index = 0;

    free((*iterator));
    *iterator = NULL;
    return LINKED_LIST_SUCCESS;
}

#pragma endregion
