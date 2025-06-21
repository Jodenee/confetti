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

// constant definitions

#define DEFAULT_LIST_CAPACITY ((int64_t) 8) // The default list capacity used if one is not given.

// struct definitions

// define all structs early to avoid errors relating to one of these structs not existing.
typedef struct list list_t;
typedef struct list_element list_element_t;
typedef struct list_iterator list_iterator_t;
typedef enum list_result list_result_t;

/**
 * @brief Type definition for a custom equality function for list elements.
 *
 * This function type is used to compare two data elements for equality. It should return:
 * 
 * - A negative value if the first element is less than the second.
 * 
 * - Zero if they are equal.
 * 
 * - A positive value if the first element is greater than the second.
 *
 * @param data1 A pointer to the first data element to compare.
 * @param data2 A pointer to the second data element to compare.
 * @param size The size of the data elements to be compared.
 * 
 * @return An int32_t indicating the comparison result.
 */
typedef int32_t (list_custom_equality_function_t)(const void* const data1, const void* const data2, const uint64_t size);

/**
 * @brief Type definition for a custom sorting function for a list.
 *
 * This function type is used to sort the elements of a list. It should modify the list in place
 * based on the specified sorting order (ascending or descending).
 *
 * @param list A pointer to the list to be sorted.
 * @param ascending A boolean value indicating the sort order.
 * 
 * @return A list_result_t indicating the outcome of the sorting operation.
 */
typedef list_result_t (list_custom_sorting_function_t)(list_t* const list, const bool ascending);

/**
 * @brief Represents an element in a list.
 *
 * This structure stores an element's size and a pointer to its value.
 */
typedef struct list_element {
    uint64_t size; /* Size of the value in bytes. */
    void* value;   /* Pointer to the data stored in the list element. */
} list_element_t;

/**
 * @brief Represents a dynamic list data structure.
 *
 * This structure maintains a dynamic array of pointers to list elements,
 * along with metadata for managing size, capacity, and custom behavior.
 */
typedef struct list {
    int64_t size;                                      /* Current number of elements in the list. */
    int64_t capacity;                                  /* Total allocated capacity for elements. */
    list_element_t** items;                            /* Array of pointers to list elements. */
    list_custom_equality_function_t* equalityFunction; /* Optional custom equality function for comparing elements. */
    list_custom_sorting_function_t* sortingFunction;   /* Optional custom sorting function for ordering elements. */
} list_t;

/**
 * @brief Represents an iterator for a list.
 * 
 * This structure is used to facilitate iteration over the elements of a
 * list. It maintains the necessary information to traverse the
 * list.
 *
 * @param list A pointer to the list being iterated over.
 * @param index The current index.
 * @param element A pointer to the current element.
 * 
 * @warning Please do not manually free anything witin this structure as 
 * they are the internal values kept by the linked list. If you wish 
 * to deallocate memory from this structure please use `list_iterator_free` to safely do so.
 */
typedef struct list_iterator {
    list_t* list;            /* The list being iterated through. */
    int64_t index;           /* The index of the current iteration. */
    list_element_t* element; /* The element of the current iteration. */
} list_iterator_t;

// enum definitions

/**
 * @brief Represents the result of a list operation.
 *
 * This enumeration defines various status codes that may be returned by
 * functions operating on lists. Positive values indicate success, while
 * negative values represent specific error conditions.
 */
typedef enum list_result {
    /**
     * @brief Completed successfully.
     */
    LIST_SUCCESS = 1,

    /**
     * @brief Error: Index is out of range.
     * 
     * This error occurs when an attempt is made to access an
     * element at an index that is not valid.
     */
    LIST_INDEX_OUT_OF_RANGE_ERROR = -1,

    /**
     * @brief Error: Element not found in the list.
     * 
     * This error indicates that the specified element could not
     * be located.
     */
    LIST_ELEMENT_NOT_FOUND_ERROR = -2,

    /**
     * @brief Error: List is null.
     * 
     * This error occurs when an operation is attempted on a list
     * that has not been initialized (i.e., it is null).
     */
    LIST_NULL_ERROR = -3,

    /**
     * @brief Error: Invalid parameters provided.
     * 
     * This error indicates that the parameters passed to a list
     * operation are not valid.
     */
    LIST_INVALID_PARAMS_ERROR = -4,

    /**
     * @brief Error: Memory allocation failure.
     * 
     * This error occurs when the system is unable to allocate
     * the necessary memory for the operation.
     */
    LIST_ALLOCATION_FAILURE = -5
} list_result_t;

// public function definitions

#pragma region public function definitions

/**
 * @brief Creates a new list with the specified capacity and custom functions.
 * 
 * This function allocates memory for a new list and initializes its
 * properties, including size, capacity, and optional custom equality
 * and sorting functions. The created list is returned through the output parameter.
 *
 * @param listOut A double pointer to where the created list will be stored.
 * @param capacity The initial capacity of the list. If less than 1, the `DEFAULT_LIST_CAPACITY` will be used.
 * @param customEqualityFunction A pointer to a custom equality function, or NULL to use the default.
 * @param customSortingFunction A pointer to a custom sorting function, or NULL to use the default.
 * 
 * @return 
 * - `LIST_SUCCESS` if the list was created successfully. 
 * 
 * - `LIST_ALLOCATION_FAILURE` if the system couldn't allocate enough memory for the operation.
 */
CONFETTI_EXPORT list_result_t list_create(
    list_t** listOut,
    const int64_t capacity,
    list_custom_equality_function_t* const customEqualityFunction,
    list_custom_sorting_function_t* const customSortingFunction);

/**
 * @brief frees the memory for a `list_t` and its elements.
 *
 * @param list A double pointer to the list to be freed.
 * 
 * @return
 * - `LIST_SUCCESS` if the list was successfully freed.
 *
 * - `LIST_NULL_ERROR` if the provided list is NULL.
 * 
 * @note Sets the list pointer to NULL after freeing.
 */
CONFETTI_EXPORT list_result_t list_free(list_t** listOut);

/**
 * @brief Prints the contents of the list to the standard output.
 * 
 * This function iterates through all elements of the specified list
 * and prints their value's address. The function also prints the address 
 * of the list itself at the end.
 *
 * @param list A pointer to the list to be printed.
 * 
 * @return 
 * - `LIST_SUCCESS` if the list was printed successfully. 
 * 
 * - `LIST_NULL_ERROR` if the provided list pointer is NULL.
 */
CONFETTI_EXPORT list_result_t list_print(list_t* const list);

/**
 * @brief Resizes the list to the new capacity.
 * 
 * @param list A pointer to the list to be resized.
 * @param newCapacity The new capacity for the list.
 * 
 * @return 
 * - `LIST_SUCCESS` if the list was resized successfully. 
 * 
 * - `LIST_NULL_ERROR` if the provided list pointer is NULL
 * 
 * - `LIST_INVALID_PARAMS_ERROR` if the new capacity is less than 1.
 * 
 * @note If the new capacity is the same as the current capacity, 
 *       it returns `LIST_SUCCESS` without making any changes.
 */
CONFETTI_EXPORT list_result_t list_resize(list_t* const list, const int64_t size);

/**
 * @brief Prepends a new element to the beginning of the list.
 *
 * @param list A pointer to the list to which the element will be prepended.
 * @param value A pointer to the value to be prepended.
 * @param size The size of the value being prepended.
 * 
 * @return 
 * - `LIST_SUCCESS` if the element was prepended successfully.
 * 
 * - `LIST_NULL_ERROR` if the provided list pointer is NULL.
 * 
 * - `LIST_ALLOCATION_FAILURE` if the system couldn't allocate enough memory for the operation.
 */
CONFETTI_EXPORT list_result_t list_prepend(list_t* const list, void* const value, const uint64_t size);

/**
 * @brief Adds a new element with the specified value to the
 * end of the given list.
 *
 * @param list A pointer to the list to which the element will be appended.
 * @param value A pointer to the value to be appended.
 * @param size The size of the value to be appended.
 * 
 * @return 
 * - `LIST_SUCCESS` if the element was appended successfully. 
 * 
 * - `LIST_NULL_ERROR` if the provided list pointer is NULL.
 * 
 * - `LIST_ALLOCATION_FAILURE` if the system couldn't allocate enough memory for the operation.
 */
CONFETTI_EXPORT list_result_t list_append(list_t* const list, void* const value, const uint64_t size);

/**
 * @brief Inserts a new element at a specified index in the list.
 *
 * @param list A pointer to the list where the element will be inserted.
 * @param index The index at which the new element will be inserted.
 * @param value A pointer to the value to be inserted.
 * @param size The size of the value to be inserted.
 * 
 * @return 
 * - `LIST_SUCCESS` if the element was inserted successfully. 
 * 
 * - `LIST_NULL_ERROR` if the provided list pointer is NULL.
 * 
 * - `LIST_INDEX_OUT_OF_RANGE_ERROR` if the index is out of range.
 */
CONFETTI_EXPORT list_result_t list_insert(list_t* const list, const int64_t index, void* const value, const uint64_t size);

/**
 * @brief Retrieves an element from the list at the specified index.
 * 
 * This function fetches the element at the given index from the specified
 * list and returns a clone of that element through the output parameter.
 *
 * @param list A pointer to the list from which to retrieve the element.
 * @param elementOut A double pointer to where the cloned element will be set.
 * @param index The index of the list to retrieve.
 * 
 * @return 
 * - `LIST_SUCCESS` if the element was retrieved successfully.
 * 
 * - `LIST_NULL_ERROR` if the provided list pointer is NULL.
 * 
 * - `LIST_INDEX_OUT_OF_RANGE_ERROR` if the given index is out of range.
 * 
 * @note The outputted `list_element_t` is a clone that is allocated separately from
 * the one inside the list, so freeing this element is your responsibility, it is 
 * recomended to use `list_element_free` for this.
 */
CONFETTI_EXPORT list_result_t list_get(list_t* const list, list_element_t** elementOut, const int64_t index);

/**
 * @brief Sets the value of an element in the list at the specified index.
 * 
 * @param list A pointer to the list in which to set the element.
 * @param index The index of the element to set.
 * @param value A pointer to the new value to be assigned to the element.
 * @param size The size of the new value.
 * 
 * @return 
 * - `LIST_SUCCESS` if the value was set successfully. 
 * 
 * - `LIST_NULL_ERROR` if the provided list pointer is NULL.
 * 
 * - `LIST_INDEX_OUT_OF_RANGE_ERROR` if the index is out of range.
 * 
 * - `LIST_INVALID_PARAMS_ERROR` if the value size is zero.
 */
CONFETTI_EXPORT list_result_t list_set(list_t* const list, const int64_t index, void* const value, const uint64_t size);

/**
 * @brief Removes an element from the list at the specified index.
 *
 * @param list A pointer to the list from which the element will be removed.
 * @param index The index of the element to remove.
 * 
 * @return 
 * - `LIST_SUCCESS` if the element was removed successfully. 
 * 
 * - `LIST_NULL_ERROR` if the provided list pointer is NULL.
 *  
 * - `LIST_INDEX_OUT_OF_RANGE_ERROR` if the index is out of range.
 */
CONFETTI_EXPORT list_result_t list_remove(list_t* const list, const int64_t index);

/**
 * @brief Pops an element from the list at the specified index.
 *
 * @param list A pointer to the list from which the element will be popped.
 * @param index The index of the element to pop.
 * @param elementOut A double pointer to where the popped element will be stored.
 * 
 * @return 
 * - `LIST_SUCCESS` if the element was popped successfully.
 * 
 * - `LIST_NULL_ERROR` if the provided list pointer is NULL.
 * 
 * - `LIST_INDEX_OUT_OF_RANGE_ERROR` if the index is out of range.
 * 
 * @note The outputted `list_element_t` is a clone that is allocated separately from
 * the one inside the list, so freeing this element is your responsibility, it is 
 * recomended to use `list_element_free` for this.
 */
CONFETTI_EXPORT list_result_t list_pop(list_t* const list, list_element_t** elementOut, const int64_t index);

/**
 * @brief Reverses the order of elements inside the list in-place.
 *
 * @param list A pointer to the list to be reversed.
 * 
 * @return 
 * - `LIST_SUCCESS` if the list was reversed successfully. 
 * 
 * - `LIST_NULL_ERROR` if the provided list pointer is NULL.
 */
CONFETTI_EXPORT list_result_t list_reverse(list_t* const list);

/**
 * @brief Clones the specified list, creating a new copy.
 * 
 * This function allocates memory for a new list and copies the elements
 * from the original list into the new list. 
 *
 * @param list A pointer to the list to be cloned.
 * @param listOut A pointer to a pointer where the cloned list will be stored.
 * 
 * @return 
 * - `LIST_SUCCESS` if the list was cloned successfully. 
 * 
 * - `LIST_NULL_ERROR` if the provided list pointer is NULL.
 * 
 * - `LIST_ALLOCATION_FAILURE` if memory allocation fails during the process.
 * 
 * @note Each element is cloned to ensure that the list clone contains independent copies of the elements.
 */
CONFETTI_EXPORT list_result_t list_clone(list_t* const list, list_t** listOut);

/**
 * @brief Clears all elements from the list.
 * 
 * This function deallocates the memory associated with each element in
 * the specified list and resets the size of the list to zero.
 *
 * @param list A pointer to the list to be cleared.
 * 
 * @return 
 * - `LIST_SUCCESS` if the list was cleared successfully. 
 * 
 * - `LIST_NULL_ERROR` if the provided list pointer is NULL.
 */
CONFETTI_EXPORT list_result_t list_clear(list_t* const list);

/**
 * @brief Joins two lists into a new list.
 * 
 * This function creates a new list that contains all elements from both
 * specified lists, `list1` and `list2`. Each element from both lists are cloned to ensure
 * that the new list is independent from `list1` and `list2`.
 *
 * @param list1 A pointer to the first list to be joined.
 * @param list2 A pointer to the second list to be joined.
 * @param listOut A pointer to a pointer where the joined list will be stored.
 * 
 * @return 
 * - `LIST_SUCCESS` if the lists were joined successfully.
 * 
 * - `LIST_NULL_ERROR` if either of the provided list pointers is NULL.
 * 
 * - `LIST_ALLOCATION_FAILURE` if the system couldn't allocate enough memory for the operation.
 */
CONFETTI_EXPORT list_result_t list_join(list_t* const list1, list_t* const list2, list_t** listOut);

/**
 * @brief Checks if the list includes a specific value.
 * 
 * This function searches through the specified list to determine if it
 * contains an element that matches the provided value. It uses the list's
 * equality function to check for a match.
 *
 * @param list A pointer to the list to be searched.
 * @param value A pointer to the value to search for in the list.
 * @param size The size of the value to be compared.
 * 
 * @return 
 * - `LIST_SUCCESS` if the value is found in the list.
 * 
 * - `LIST_NULL_ERROR` if the provided list pointer is NULL.
 * 
 * - `LIST_ELEMENT_NOT_FOUND_ERROR` if the value is not found.
 */
CONFETTI_EXPORT list_result_t list_includes(list_t* const list, void* const value, const uint64_t size);

/**
 * @brief Finds the first occurrence of a specific value in the list.
 * 
 * This function searches the specified list for the first element that
 * matches the provided value, starting from a given index. It uses the
 * list's equality function to compare the elements.
 *
 * @param list A pointer to the list to be searched.
 * @param indexOut A pointer to an integer where the index of the found element
 *                 will be stored.
 * @param startIndex The index from which to start the search.
 * @param value A pointer to a value to search for in the list.
 * @param size The size of the value to be searched.
 * 
 * @return 
 * - `LIST_SUCCESS` if the value is found.
 * 
 * - `LIST_NULL_ERROR` if the provided list pointer is NULL.
 * 
 * - `LIST_INVALID_PARAMS_ERROR` if the start index or size is invalid.
 * 
 * - `LIST_ELEMENT_NOT_FOUND_ERROR` if the value is not found.
 * 
 * @note On error indexOut will be set to -1.
 */
CONFETTI_EXPORT list_result_t list_find_first(
    list_t* const list, 
    int64_t* const indexOut, 
    const int64_t startIndex,
    void* const value, 
    const uint64_t size);

/**
 * @brief Finds the last occurrence of a specific value in the list.
 * 
 * This function searches the specified list for the last element that
 * matches the provided value, starting from a given index. It uses the
 * list's equality function to compare the elements.
 *
 * @param list A pointer to the list to be searched.
 * @param indexOut A pointer to an integer where the index of the found element
 *                 will be stored.
 * @param startIndex The index from which to start the search.
 * @param value A pointer to a value to search for in the list.
 * @param size The size of the value to be searched.
 * 
 * @return
 * - `LIST_SUCCESS` if the value is found.
 * 
 * - `LIST_NULL_ERROR` if the provided list pointer is NULL.
 * 
 * - `LIST_INDEX_OUT_OF_RANGE_ERROR` if the start index is out of range.
 * 
 * - `LIST_INVALID_PARAMS_ERROR` if the size is less than or equal to 0.
 * 
 * - `LIST_ELEMENT_NOT_FOUND_ERROR` if the value is not found.
 */
CONFETTI_EXPORT list_result_t list_find_last(
    list_t* const list, 
    int64_t* const indexOut, 
    const int64_t startIndex, 
    void* const value, 
    const uint64_t size);

/**
 * @brief Sorts the elements of the list.
 * 
 * This function sorts the elements of the specified list using the
 * provided sorting function. The order of sorting is determined by the
 * `ascending` parameter.
 * 
 * @param list A pointer to the list to be sorted.
 * @param ascending A boolean value indicating the sort order; true for
 *                  ascending order and false for descending order.
 * 
 * @return 
 * - `LIST_SUCCESS` if the list was sorted successfully.
 * 
 * - `LIST_INDEX_OUT_OF_RANGE_ERROR` if the sorting function encounters an index out of range.
 * 
 * - `LIST_ELEMENT_NOT_FOUND_ERROR` if the sorting function cannot find an element.
 * 
 * - `LIST_NULL_ERROR` if the provided list pointer is NULL.
 * 
 * - `LIST_INVALID_PARAMS_ERROR` if invalid parameters are provided to the sorting function.
 * 
 * - `LIST_ALLOCATION_FAILURE` if the sorting function fails to allocate memory.
 * 
 * @warning The actual errors returned depend on the implementation of the sorting function.
 * 
 * @note If the default sorting function is used only `LIST_SUCCESS` should be returned.
 * @note If the list is empty or contains only one element,
 *       the function returns success without performing any operations.
 */
CONFETTI_EXPORT list_result_t list_sort(list_t* const list, const bool ascending);

/**
 * @brief Fills the list with a specified value up to its capacity.
 *
 * @param list A pointer to the list to be filled.
 * @param value A pointer to the value to be assigned.
 * @param size The size of the value to be assigned.
 * 
 * @return 
 * - `LIST_SUCCESS` if the list was filled successfully. 
 * 
 * - `LIST_NULL_ERROR` if the provided list pointer is NULL.
 * 
 * - `LIST_ALLOCATION_FAILURE` if the system couldn't allocate enough memory for the operation. 
 */
CONFETTI_EXPORT list_result_t list_fill(list_t* const list, void* const value, const uint64_t size);

/**
 * @brief Swaps two elements in the list at the specified indices.
 *
 * This function exchanges the positions of two elements in the list based on their indices. 
 *
 * @param list A pointer to the list containing the elements to be swapped.
 * @param index1 The index of the first element to swap.
 * @param index2 The index of the second element to swap.
 * 
 * @return 
 * - `LIST_SUCCESS` if the elements were successfully swapped.
 * 
 * - `LIST_NULL_ERROR` if the provided list pointer is NULL.
 * 
 * - `LIST_INDEX_OUT_OF_RANGE_ERROR` if either index is out of the valid range of the list.
 * 
 * @note If index1 and index2 are the same `LIST_SUCCESS` is returned without modifying anything.
 */
CONFETTI_EXPORT list_result_t list_swap(list_t* const list, const int64_t index1, const int64_t index2);

/**
 * @brief frees the memory allocated for a `list_element_t`.
 *
 * @param element A double pointer to the list element to be freed.
 * 
 * @return 
 * `LIST_SUCCESS` if the element was successfully freed. 
 * 
 * `LIST_INVALID_PARAMS_ERROR` if the provided element pointer is NULL.
 * 
 * @note Sets the element pointer to NULL after freeing.
 */
CONFETTI_EXPORT list_result_t list_element_free(list_element_t** element);

/**
 * @brief Creates a new list iterator.
 *
 *
 * @param iteratorOut A double pointer where the created iterator will be stored.
 * @param list A pointer to the list to be iterated over.
 * 
 * @return 
 * - `LIST_SUCCESS` if the iterator was successfully created.
 * 
 * - `LIST_NULL_ERROR` if the provided list pointer is NULL.
 */
CONFETTI_EXPORT list_result_t list_iterator_create(list_iterator_t** iteratorOut, list_t* const list);

/**
 * @brief Advances the iterator to the next element in the list.
 *
 * @param iterator A pointer to the list iterator to be advanced.
 * 
 * @return 
 * - `LIST_SUCCESS` if the iterator was successfully advanced to the next element.
 * 
 * - `LIST_INVALID_PARAMS_ERROR` if the provided iterator pointer is NULL.
 * 
 * - `LIST_INDEX_OUT_OF_RANGE_ERROR` if the iterator is already at the end of the list.
 * 
 * @note When the iterator reaches the end of a list it will rewind itself to avoid 
 *       lots of calls to `list_iterator_rewind`.
 */
CONFETTI_EXPORT list_result_t list_iterator_next(list_iterator_t* const iterator);

/**
 * @brief Resets the iterator to its initial state.
 *
 * This function rewinds the iterator, setting it to an invalid state with no current element. 
 * The index is reset to -1, and the element pointer is set to NULL.
 *
 * @param iterator A pointer to the list iterator to be reset.
 * 
 * @return 
 * - `LIST_SUCCESS` if the iterator was successfully reset.
 * 
 * - `LIST_INVALID_PARAMS_ERROR` if the iterator is NULL.
 */
CONFETTI_EXPORT list_result_t list_iterator_rewind(list_iterator_t* const iterator);

/**
 * @brief Frees the memory allocated for a list iterator.
 *
 * This function deallocates the memory used by a list iterator and resets its fields to NULL
 * or zero. After freeing the iterator, it sets the double pointer to NULL to avoid dangling pointers.
 *
 * @param iterator A double pointer to the list iterator to be freed.
 * 
 * @return 
 * - `LIST_SUCCESS` if the iterator was successfully freed.
 * 
 * - `LIST_INVALID_PARAMS_ERROR` if the provided iterator pointer is NULL.
 * 
 * @note Sets the iterator pointer to NULL after freeing.
 * @note Only the iterator is freed, the list used does not get freed.
 */
CONFETTI_EXPORT list_result_t list_iterator_free(list_iterator_t** iterator);

#pragma endregion
