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

#pragma once

// Headers

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "confetti_export.h"

// definitions

typedef struct linked_list linked_list_t;
typedef struct linked_list_node linked_list_node_t;
typedef struct linked_list_element linked_list_element_t; 
typedef struct linked_list_iterator linked_list_iterator_t;
typedef enum linked_list_result linked_list_result_t; 


/**
 * @brief Function type definition for a custom equality function.
 *
 * @param data1 Pointer to the first memory block.
 * @param data2 Pointer to the second memory block.
 * @param size Size in bytes of the data elements.
 * 
 * @return 
 * - `0` if the elements are considered equal.
 * 
 * - A negative value if the first element is considered less than the second.
 * 
 * - A positive value if the first element is considered greater than the second.
 */
typedef int32_t (linked_list_custom_equality_function_t)(const void* const data1, const void* const data2, const uint64_t size);


/**
 * @brief Function type definition for a custom sorting function for linked lists.
 *
 * @param list A pointer to the linked list to be sorted.
 * @param ascending A boolean value indicating the sorting order. 
 * 
 * @return A linked_list_result_t indicating the outcome of the sorting operation.
 */
typedef linked_list_result_t (linked_list_custom_sorting_function_t)(linked_list_t* const list, const bool ascending);


/**
 * @brief Represents an element in a linked list.
 *
 * This structure holds a value and its size, which can be used by the linked list
 * to store arbitrary data.
 */
typedef struct linked_list_element {
    void* value;   /* Pointer to the data stored in the element. */
    uint64_t size; /* Size of the data in bytes. */
} linked_list_element_t;


/**
 * @brief Represents a node in a linked list.
 *
 * Each node contains a pointer to a linked list element and a pointer to the next node.
 */
typedef struct linked_list_node {
    linked_list_node_t* next;       /* Pointer to the next node in the list. */
    linked_list_element_t* element; /* Pointer to the element contained in this node. */
} linked_list_node_t;


/**
 * @brief Represents a singly linked list.
 *
 * This structure maintains pointers to the head and tail nodes, the total number
 * of elements, an equality function and sorting function.
 */
typedef struct linked_list {
    linked_list_node_t* head;                                 /* Pointer to the first node in the linked list. */
    linked_list_node_t* tail;                                 /* Pointer to the last node in the linked list. */
    int64_t size;                                             /* Number of elements currently in the linked list. */
    linked_list_custom_equality_function_t* equalityFunction; /* Equality function for comparing elements. */
    linked_list_custom_sorting_function_t* sortingFunction;   /* Sorting function for sorting the linked list. */
} linked_list_t;


/**
 * @brief Represents an iterator for a linked list.
 * 
 * This structure is used to facilitate iteration over the nodes of a
 * linked list. It maintains the necessary information to traverse the
 * list, including the current position and the associated node.
 *
 * @param list A pointer to the linked list being iterated over.
 * @param index The index of the current node within the linked list.
 * @param node A pointer to the current node in the iteration.
 * 
 * @warning Please do not manually free anything witin this structure as 
 * they are the internal values kept by the linked list. If you wish 
 * to deallocate memory from this structure please use `linked_list_iterator_free` to safely do so.
 */
typedef struct linked_list_iterator {
    linked_list_t* list;            /* The list being iterated through. */
    int64_t index;                  /* The index of the current iteration. */
    linked_list_node_t* node;       /* The node of the current iteration. */
} linked_list_iterator_t;

// enums

/**
 * @brief Represents the result of a linked list operation.
 *
 * This enumeration defines status codes that indicate the outcome of operations
 * on a linked list. Positive values indicate success, while negative values
 * correspond to specific error conditions.
 */
typedef enum linked_list_result {
    /**
     * @brief Operation completed successfully.
     */
    LINKED_LIST_SUCCESS = 1,

    /**
     * @brief Error: Index is out of range.
     *
     * Returned when an attempt is made to access a node at an invalid index.
     */
    LINKED_LIST_INDEX_OUT_OF_RANGE_ERROR = -1,

    /**
     * @brief Error: Element not found in the linked list.
     *
     * Indicates that the requested element could not be located in the list.
     */
    LINKED_LIST_ELEMENT_NOT_FOUND_ERROR = -2,

    /**
     * @brief Error: Linked list is null.
     *
     * Returned when an operation is attempted on a null list.
     */
    LINKED_LIST_NULL_ERROR = -3,

    /**
     * @brief Error: Invalid parameters provided.
     *
     * Indicates that one or more parameters passed to the function are invalid.
     */
    LINKED_LIST_INVALID_PARAMS_ERROR = -4,

    /**
     * @brief Error: Memory allocation failure.
     *
     * Occurs when the system fails to allocate memory required for the operation.
     */
    LINKED_LIST_ALLOCATION_FAILURE = -5
} linked_list_result_t;

// public function definitions

/**
 * @brief Creates a new linked list.
 *
 * @param linkedListOut A double pointer to where the 
 *                      linked list will be stored.
 * @param customEqualityFunction A pointer to a custom equality function
 *                               for comparing elements in the linked list.
 *                               If NULL, a default equality function is used.
 * @param customSortingFunction A pointer to a custom sorting function
 *                              for sorting elements in the linked list.
 *                              If NULL, a default sorting function is used.
 * 
 * @return 
 * `LINKED_LIST_SUCCESS` if the linked list was created successfully.
 * 
 * `LINKED_LIST_ALLOCATION_FAILURE` if the system couldn't allocate enough memory for the operation.
 */
CONFETTI_EXPORT linked_list_result_t linked_list_create(
    linked_list_t** linkedListOut, 
    linked_list_custom_equality_function_t* const customEqualityFunction, 
    linked_list_custom_sorting_function_t* const customSortingFunction);

/**
 * @brief Frees the memory allocated for a linked list.
 *
 * This function deallocates all nodes in the linked list and frees the
 * memory associated with the list itself.
 *
 * @param linkedList A double pointer to the linked list to be freed.
 * 
 * @return 
 * - `LINKED_LIST_SUCCESS` if the linked list was freed successfully.
 * 
 * - `LINKED_LIST_NULL_ERROR` if the provided linked list pointer is NULL.
 * 
 * @note Sets the element pointer to NULL after freeing.
 */
CONFETTI_EXPORT linked_list_result_t linked_list_free(linked_list_t** linkedList);

/**
 * @brief Prints the elements of a linked list.
 *
 * This function iterates through the linked list and prints the values
 * of its elements. It displays the elements in a formatted manner, 
 * including a pointer to the linked list itself at the end.
 *
 * @param linkedList A pointer to the linked list to be printed.
 * 
 * @return 
 * - `LINKED_LIST_SUCCESS` if the linked list was printed successfully.
 * 
 * - `LINKED_LIST_NULL_ERROR` if the provided linked list pointer is NULL.
 */
CONFETTI_EXPORT linked_list_result_t linked_list_print(linked_list_t* const linkedList);

/**
 * @brief Retrieves an element from the linked list at a specified index.
 *
 * This function accesses the linked list and retrieves the element
 * at the given index. It clones the element to ensure that the caller
 * receives a separate copy.
 *
 * @param linkedList A pointer to the linked list from which to retrieve the element.
 * @param elementOut A double pointer where the cloned element will be stored.
 * @param index The index of the element to retrieve.
 * 
 * @return 
 * - `LINKED_LIST_SUCCESS` if the element was retrieved successfully.
 * 
 * - `LINKED_LIST_NULL_ERROR` if the provided linked list pointer is NULL.
 * 
 * - `LINKED_LIST_INDEX_OUT_OF_RANGE_ERROR` if the specified index is out of range.
 * 
 * - `LINKED_LIST_ALLOCATION_FAILURE` if the system couldn't allocate enough memory for the operation.
 * 
 * @note The outputted `linked_list_element_t` is a clone that is allocated separately from
 * the one inside the list, so freeing this element is your responsibility, it is 
 * recomended to use `linked_list_element_free` for this.
 */
CONFETTI_EXPORT linked_list_result_t linked_list_get(linked_list_t* const linkedList, linked_list_element_t** elementOut, const int64_t index);

/**
 * @brief Sets the value of an element in the linked list at a specified index.
 *
 * @param linkedList A pointer to the linked list in which to set the element.
 * @param index The index of the element to update.
 * @param value A pointer to the new value to be assigned to the element.
 * @param size The size of the value to be assigned.
 * 
 * @return 
 * - `LINKED_LIST_SUCCESS` if the element was updated successfully.
 * 
 * - `LINKED_LIST_NULL_ERROR` if the provided linked list pointer is NULL.
 * 
 * - `LINKED_LIST_INDEX_OUT_OF_RANGE_ERROR` if the specified index is out of range.
 * 
 * - `LINKED_LIST_ALLOCATION_FAILURE` if the system couldn't allocate enough memory for the operation.
 */
CONFETTI_EXPORT linked_list_result_t linked_list_set(linked_list_t* const linkedList, const int64_t index, void* const value, const uint64_t size);

/**
 * @brief Prepends a new element to the beginning of the linked list.
 *
 * @param linkedList A pointer to the linked list to which the element will be prepended.
 * @param value A pointer to the value to be assigned to the new element.
 * @param size The size of the value to be assigned.
 * 
 * @return 
 * - `LINKED_LIST_SUCCESS` if the element was prepended successfully.
 * 
 * - `LINKED_LIST_NULL_ERROR` if the provided linked list pointer is NULL.
 * 
 * - `LINKED_LIST_ALLOCATION_FAILURE` if the system couldn't allocate enough memory for the operation.
 */
CONFETTI_EXPORT linked_list_result_t linked_list_prepend(linked_list_t* const linkedList, void* const value, const uint64_t size);

/**
 * @brief Appends a new element to the end of the linked list.
 *
 * @param linkedList A pointer to the linked list to which the element will be appended.
 * @param value A pointer to the value to be assigned to the new element.
 * @param size The size of the value to be assigned.
 * 
 * @return 
 * - `LINKED_LIST_SUCCESS` if the element was appended successfully.
 * 
 * - `LINKED_LIST_NULL_ERROR` if the provided linked list pointer is NULL.
 * 
 * - `LINKED_LIST_ALLOCATION_FAILURE` if the system couldn't allocate enough memory for the operation.
 */
CONFETTI_EXPORT linked_list_result_t linked_list_append(linked_list_t* const linkedList, void* const value, const uint64_t size);

/**
 * @brief Inserts a new element at a specified index in the linked list.
 *
 * @param linkedList A pointer to the linked list where the element will be inserted.
 * @param index The index at which to insert the new element.
 * @param value A pointer to the value to be assigned to the new element.
 * @param size The size of the value to be assigned.
 * 
 * @return 
 * - `LINKED_LIST_SUCCESS` if the element was inserted successfully.
 * 
 * - `LINKED_LIST_NULL_ERROR` if the provided linked list pointer is NULL.
 * 
 * - `LINKED_LIST_INDEX_OUT_OF_RANGE_ERROR` if the specified index is out of range.
 */
CONFETTI_EXPORT linked_list_result_t linked_list_insert(linked_list_t* const linkedList, const int64_t index, void* value, const uint64_t size);

/**
 * @brief Removes an element from the linked list at a specified index.
 *
 * @param linkedList A pointer to the linked list from which the element will be removed.
 * @param index The index of the element to remove.
 * 
 * @return 
 * - `LINKED_LIST_SUCCESS` if the element was removed successfully.
 * 
 * - `LINKED_LIST_NULL_ERROR` if the provided linked list pointer is NULL.
 * 
 * - `LINKED_LIST_INDEX_OUT_OF_RANGE_ERROR` if the specified index is out of range.
 */
CONFETTI_EXPORT linked_list_result_t linked_list_remove(linked_list_t* const linkedList, const uint64_t index);

/**
 * @brief Removes and retrieves an element from the linked list at a specified index.
 *
 * @param linkedList A pointer to the linked list from which the element will be removed.
 * @param elementOut A double pointer where the cloned element will be stored.
 * @param index The index of the element to remove and retrieve.
 * 
 * @return 
 * - `LINKED_LIST_SUCCESS` if the element was removed and retrieved successfully.
 * 
 * - `LINKED_LIST_NULL_ERROR` if the provided linked list pointer is NULL.
 * 
 * - `LINKED_LIST_INDEX_OUT_OF_RANGE_ERROR` if the specified index is out of range.
 * 
 * @note The outputted `linked_list_element_t` is a clone that is allocated separately from
 * the one inside the list, so freeing this element is your responsibility, it is 
 * recomended to use `linked_list_element_free` for this.
 */
CONFETTI_EXPORT linked_list_result_t linked_list_pop(linked_list_t* const linkedList, linked_list_element_t** elementOut, const uint64_t index);

/**
 * @brief Clears all elements from the linked list.
 *
 * @param linkedList A pointer to the linked list to be cleared.
 * 
 * @return 
 * - `LINKED_LIST_SUCCESS` if the linked list was cleared successfully.
 * 
 * - `LINKED_LIST_NULL_ERROR` if the provided linked list pointer is NULL.
 */
CONFETTI_EXPORT linked_list_result_t linked_list_clear(linked_list_t* const linkedList);

/**
 * @brief Creates a clone of a linked list.
 *
 * This function creates a new linked list that is a copy of the provided
 * linked list. It allocates memory for the new list and clones each node
 * from the original list.
 *
 * @param linkedList A pointer to the linked list to be cloned.
 * @param linkedListOut A double pointer where the address of the cloned linked list will be stored.
 * 
 * @return 
 * - `LINKED_LIST_SUCCESS` if the linked list was cloned successfully.
 * 
 * - `LINKED_LIST_NULL_ERROR` if the provided linked list pointer is NULL.
 * 
 * - `LINKED_LIST_ALLOCATION_FAILURE` if the system couldn't allocate enough memory for the operation.
 * 
 * @note Each element is cloned to ensure that the list clone contains independent copies of the elements.
 */
CONFETTI_EXPORT linked_list_result_t linked_list_clone(linked_list_t* const linkedList, linked_list_t** linkedListOut);

/**
 * @brief Joins two linked lists into a new linked list.
 *
 * This function creates a new linked list that contains all elements from
 * the first linked list followed by all elements from the second linked list.
 *
 * @param linkedList1 A pointer to the first linked list to be joined.
 * @param linkedList2 A pointer to the second linked list to be joined.
 * @param linkedListOut A double pointer to where the new linked list will be stored.
 * 
 * @return 
 * - `LINKED_LIST_SUCCESS` if the linked lists were joined successfully.
 * 
 * - `LINKED_LIST_NULL_ERROR` if either of the provided linked list pointers is NULL.
 * 
 * - `LINKED_LIST_ALLOCATION_FAILURE` if the system couldn't allocate enough memory for the operation.
 * 
 * @note Each element is cloned to ensure that the list clone contains independent copies of the elements
 * from each list.
 */
CONFETTI_EXPORT linked_list_result_t linked_list_join(linked_list_t* const linkedList1, linked_list_t* const linkedList2, linked_list_t** linkedListOut);

/**
 * @brief Resizes the linked list to a specified new size.
 *
 * This function adjusts the size of the linked list. If the new size is
 * greater than the current size, new nodes are added. If the new size is
 * smaller, excess nodes are removed. If the new size is zero, all nodes
 * are freed.
 *
 * @param linkedList A pointer to the linked list to be resized.
 * @param size The new size for the linked list.
 * 
 * @return 
 * - `LINKED_LIST_SUCCESS` if the linked list was resized successfully.
 * 
 * - `LINKED_LIST_NULL_ERROR` if the provided linked list pointer is NULL.
 * 
 * - `LINKED_LIST_ALLOCATION_FAILURE` if the system couldn't allocate enough memory for the operation.
 */
CONFETTI_EXPORT linked_list_result_t linked_list_resize(linked_list_t* const linkedList, const uint64_t size);

/**
 * @brief Reverses the order of elements in the linked list.
 *
 * This function reverses the linked list in place. After the operation,
 * the head will point to the last element, and the tail will point to
 * the first element.
 *
 * @param linkedList A pointer to the linked list to be reversed.
 * 
 * @return 
 * - `LINKED_LIST_SUCCESS` if the linked list was reversed successfully.
 * 
 * - `LINKED_LIST_NULL_ERROR` if the provided linked list pointer is NULL.
 * 
 * @note If the linked list is empty, the function returnssuccessfully 
 *       without making any changes.
 */
CONFETTI_EXPORT linked_list_result_t linked_list_reverse(linked_list_t* const linkedList);

/**
 * @brief Checks if a value is included in the linked list.
 *
 * @param linkedList A pointer to the linked list to be searched.
 * @param value A pointer to the value to search for in the linked list.
 * @param size The size of the value to be compared.
 * 
 * @return 
 * - `LINKED_LIST_SUCCESS` if the value was found in the linked list.
 * 
 * - `LINKED_LIST_NULL_ERROR` if the provided linked list pointer is NULL.
 * 
 * - `LINKED_LIST_ELEMENT_NOT_FOUND_ERROR` if the value was not found in the linked list.
 * 
 * @note It uses the linked list's equality function to compare elements.
 */
CONFETTI_EXPORT linked_list_result_t linked_list_includes(linked_list_t* const linkedList, void* const value, const uint64_t size);

/**
 * @brief Finds the first occurrence of a value in the linked list starting from a specified index.
 *
 * @param linkedList A pointer to the linked list to be searched.
 * @param indexOut A pointer to an integer where the index of the found value will be stored.
 * @param startFromIndex The index from which to start the search.
 * @param value A pointer to the value to search for in the linked list.
 * @param size The size of the value to be compared.
 * 
 * @return 
 * - `LINKED_LIST_SUCCESS` if the value was found in the linked list.
 * 
 * - `LINKED_LIST_NULL_ERROR` if the provided linked list pointer is NULL.
 * 
 * - `LINKED_LIST_INDEX_OUT_OF_RANGE_ERROR` if the starting index is out of range.
 * 
 * - `LINKED_LIST_ELEMENT_NOT_FOUND_ERROR` if the value was not found in the linked list.
 * 
 * @note It uses the linked list's equality function to compare elements.
 */
CONFETTI_EXPORT linked_list_result_t linked_list_find_first(
    linked_list_t* const linkedList, 
    int64_t* const indexOut, 
    const int64_t startFromIndex, 
    void* const value, 
    const uint64_t size);

/**
 * @brief Finds the last occurrence of a value in the linked list starting from a specified index.
 *
 * This function searches the linked list for the last occurrence of a specified value, beginning the search
 * from a given starting index. If the value is found, the index of its last occurrence is returned.
 *
 * @param linkedList A pointer to the linked list to be searched.
 * @param indexOut A pointer to an integer where the index of the last found value will be stored.
 * @param startFromIndex The index from which to start the search.
 * @param value A pointer to the value to search for in the linked list.
 * @param size The size of the value to be compared.
 * 
 * @return 
 * - `LINKED_LIST_SUCCESS` if the value was found in the linked list.
 * 
 * - `LINKED_LIST_NULL_ERROR` if the provided linked list pointer is NULL.
 * 
 * - `LINKED_LIST_INDEX_OUT_OF_RANGE_ERROR` if the starting index is out of range.
 * 
 * - `LINKED_LIST_ELEMENT_NOT_FOUND_ERROR` if the value was not found in the linked list.
 * 
 * @note It uses the linked list's equality function to compare elements.
 */
CONFETTI_EXPORT linked_list_result_t linked_list_find_last(
    linked_list_t* const linkedList, 
    int64_t* const indexOut,
    const int64_t startFromIndex, 
    void* const value, 
    const uint64_t size);

/**
 * @brief Sorts the linked list using the specified sorting function.
 *
 * @param linkedList A pointer to the linked list to be sorted.
 * @param ascending A boolean value indicating the sort order.
 * 
 * @return 
 * - `LINKED_LIST_SUCCESS` if the sorting operation completed successfully.
 * 
 * - `LINKED_LIST_NULL_ERROR` if the provided linked list pointer is NULL.
 * 
 * - `LINKED_LIST_ELEMENT_NOT_FOUND_ERROR` if the sorting function cannot find an element.
 * 
 * - `LINKED_LIST_INDEX_OUT_OF_RANGE_ERROR` if the sorting function encounters an index out of range.
 * 
 * - `LINKED_LIST_INVALID_PARAMS_ERROR` if invalid parameters are provided to the sorting function.
 * 
 * - `LINKED_LIST_ALLOCATION_FAILURE` if the sorting function fails to allocate memory.
 * 
 * @warning The actual errors returned depend on the implementation of the sorting function. 
 * 
 * @note If the default sorting function is used only `LINKED_LIST_SUCCESS` should be returned.
 * @note If the linked list is empty or contains only one element,
 *       the function returns success without performing any operations.
 */
CONFETTI_EXPORT linked_list_result_t linked_list_sort(linked_list_t* const linkedList, const bool ascending);

/**
 * @brief Swaps two nodes in the list at the specified indices.
 *
 * This function exchanges the positions of two nodes in the linked list list based on their indices. 
 *
 * @param list A pointer to the linked list containing the nodes to be swapped.
 * @param index1 The index of the first node to swap.
 * @param index2 The index of the second node to swap.
 * 
 * @return 
 * - `LINKED_LIST_SUCCESS` if the nodes were successfully swapped.
 * 
 * - `LINKED_LIST_NULL_ERROR` if the provided list pointer is NULL.
 * 
 * - `LINKED_LIST_INDEX_OUT_OF_RANGE_ERROR` if either index is out of the valid range of the list.
 * 
 * @note If index1 and index2 are the same `LINKED_LIST_SUCCESS` is returned without modifying anything.
 */
CONFETTI_EXPORT linked_list_result_t linked_list_swap(linked_list_t* const linkedList, const int64_t index1, const int64_t index2);

/**
 * @brief Frees the memory allocated for a linked list element.
 *
 * @param element A pointer to a pointer to the linked list element to be freed.
 * 
 * @return 
 * - `LINKED_LIST_SUCCESS` if the element was successfully freed.
 * 
 * - `LINKED_LIST_INVALID_PARAMS_ERROR` if the provided element pointer is NULL.
 * 
 * @note Sets the element pointer to NULL after freeing.
 */
CONFETTI_EXPORT linked_list_result_t linked_list_element_free(linked_list_element_t** element);

/**
 * @brief Creates a new linked list iterator.
 *
 * @param iteratorOut A double pointer where the created iterator will be stored.
 * @param list A pointer to the linked list to be iterated over.
 * 
 * @return 
 * - `LINKED_LIST_SUCCESS` if the iterator was successfully created.
 * 
 * - `LINKED_LIST_NULL_ERROR` if the provided linked list pointer is NULL.
 */
CONFETTI_EXPORT linked_list_result_t linked_list_iterator_create(linked_list_iterator_t** iteratorOut, linked_list_t* const list);

/**
 * @brief Advances the iterator to the next element in the linked list.
 *
 * @param iterator A pointer to the linked list iterator to be advanced.
 * 
 * @return 
 * - `LINKED_LIST_SUCCESS` if the iterator was successfully advanced to the next element.
 * 
 * - `LINKED_LIST_INVALID_PARAMS_ERROR` if the provided iterator pointer is NULL.
 * 
 * - `LINKED_LIST_INDEX_OUT_OF_RANGE_ERROR` if the iterator is already at the end of the list.
 * 
 * @note When the iterator reaches the end of a linked list it will rewind itself to avoid 
 *       lots of calls to `linked_list_iterator_rewind`.
 */
CONFETTI_EXPORT linked_list_result_t linked_list_iterator_next(linked_list_iterator_t* const iterator);

/**
 * @brief Resets the iterator to its initial state.
 *
 * This function rewinds the iterator, setting it to an invalid state with no current node. 
 * The index is reset to -1, and the element pointer is set to NULL.
 *
 * @param iterator A pointer to the linked list iterator to be reset.
 * 
 * @return 
 * - `LINKED_LIST_SUCCESS` if the iterator was successfully reset.
 * 
 * - `LINKED_LIST_INVALID_PARAMS_ERROR` if the iterator is NULL.
 */
CONFETTI_EXPORT linked_list_result_t linked_list_iterator_rewind(linked_list_iterator_t* const iterator);

/**
 * @brief Frees the memory allocated for a linked list iterator.
 *
 * This function deallocates the memory used by a linked list iterator and resets its fields to NULL
 * or zero. After freeing the iterator, it sets the double pointer to NULL to avoid dangling pointers.
 *
 * @param iterator A double pointer to the linked list iterator to be freed.
 * 
 * @return 
 * - `LINKED_LIST_SUCCESS` if the iterator was successfully freed.
 * 
 * - `LINKED_LIST_INVALID_PARAMS_ERROR` if the provided iterator pointer is NULL.
 * 
 * @note Sets the iterator pointer to NULL after freeing.
 * @note Only the iterator is freed, the linked list used does not get freed.
 */
CONFETTI_EXPORT linked_list_result_t linked_list_iterator_free(linked_list_iterator_t** iterator);
