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

#define DEFAULT_VECTOR_CAPACITY ((int64_t) 8) // The default vector capacity used if one is not given.

// struct definitions

// define all structs early to avoid errors relating to one of these structs not existing.
typedef struct vector vector_t;
typedef struct vector_element vector_element_t;
typedef struct vector_iterator vector_iterator_t;
typedef enum vector_result vector_result_t;

/**
 * @brief Type definition for a custom equality function for vector elements.
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
typedef int32_t (vector_custom_equality_function_t)(const void* const data1, const void* const data2, const uint64_t size);

/**
 * @brief Type definition for a custom sorting function for a vector.
 *
 * This function type is used to sort the elements of a vector. It should modify the vector in place
 * based on the specified sorting order (ascending or descending).
 *
 * @param vector A pointer to the vector to be sorted.
 * @param ascending A boolean value indicating the sort order.
 * 
 * @return A vector_result_t indicating the outcome of the sorting operation.
 */
typedef vector_result_t (vector_custom_sorting_function_t)(vector_t* const vector, const bool ascending);

/**
 * @brief Represents an element in a vector.
 *
 * This structure stores an element's size and a pointer to its value.
 */
typedef struct vector_element {
    uint64_t size; /* Size of the value in bytes. */
    void* value;   /* Pointer to the data stored in the vector element. */
} vector_element_t;

/**
 * @brief Represents a dynamic vector data structure.
 *
 * This structure maintains a dynamic array of pointers to vector elements,
 * along with metadata for managing size, capacity, and custom behavior.
 */
typedef struct vector {
    int64_t size;                                      /* Current number of elements in the vector. */
    int64_t capacity;                                  /* Total allocated capacity for elements. */
    vector_element_t** items;                            /* Array of pointers to vector elements. */
    vector_custom_equality_function_t* equalityFunction; /* Optional custom equality function for comparing elements. */
    vector_custom_sorting_function_t* sortingFunction;   /* Optional custom sorting function for ordering elements. */
} vector_t;

/**
 * @brief Represents an iterator for a vector.
 * 
 * This structure is used to facilitate iteration over the elements of a
 * vector. It maintains the necessary information to traverse the
 * vector.
 *
 * @param vector A pointer to the vector being iterated over.
 * @param index The current index.
 * @param element A pointer to the current element.
 * 
 * @warning Please do not manually free anything witin this structure as 
 * they are the internal values kept by the linked vector. If you wish 
 * to deallocate memory from this structure please use `vector_iterator_free` to safely do so.
 */
typedef struct vector_iterator {
    vector_t* vector;            /* The vector being iterated through. */
    int64_t index;           /* The index of the current iteration. */
    vector_element_t* element; /* The element of the current iteration. */
} vector_iterator_t;

// enum definitions

/**
 * @brief Represents the result of a vector operation.
 *
 * This enumeration defines various status codes that may be returned by
 * functions operating on vectors. Positive values indicate success, while
 * negative values represent specific error conditions.
 */
typedef enum vector_result {
    /**
     * @brief Completed successfully.
     */
    VECTOR_SUCCESS = 1,

    /**
     * @brief Error: Index is out of range.
     * 
     * This error occurs when an attempt is made to access an
     * element at an index that is not valid.
     */
    VECTOR_INDEX_OUT_OF_RANGE_ERROR = -1,

    /**
     * @brief Error: Element not found in the vector.
     * 
     * This error indicates that the specified element could not
     * be located.
     */
    VECTOR_ELEMENT_NOT_FOUND_ERROR = -2,

    /**
     * @brief Error: List is null.
     * 
     * This error occurs when an operation is attempted on a vector
     * that has not been initialized (i.e., it is null).
     */
    VECTOR_NULL_ERROR = -3,

    /**
     * @brief Error: Invalid parameters provided.
     * 
     * This error indicates that the parameters passed to a vector
     * operation are not valid.
     */
    VECTOR_INVALID_PARAMS_ERROR = -4,

    /**
     * @brief Error: Memory allocation failure.
     * 
     * This error occurs when the system is unable to allocate
     * the necessary memory for the operation.
     */
    VECTOR_ALLOCATION_FAILURE = -5
} vector_result_t;

// public function definitions

#pragma region public function definitions

/**
 * @brief Creates a new vector with the specified capacity and custom functions.
 * 
 * This function allocates memory for a new vector and initializes its
 * properties, including size, capacity, and optional custom equality
 * and sorting functions. The created vector is returned through the output parameter.
 *
 * @param vectorOut A double pointer to where the created vector will be stored.
 * @param capacity The initial capacity of the vector. If less than 1, the `DEFAULT_VECTOR_CAPACITY` will be used.
 * @param customEqualityFunction A pointer to a custom equality function, or NULL to use the default.
 * @param customSortingFunction A pointer to a custom sorting function, or NULL to use the default.
 * 
 * @return 
 * - `VECTOR_SUCCESS` if the vector was created successfully. 
 * 
 * - `VECTOR_ALLOCATION_FAILURE` if the system couldn't allocate enough memory for the operation.
 */
CONFETTI_EXPORT vector_result_t vector_create(
    vector_t** vectorOut,
    const int64_t capacity,
    vector_custom_equality_function_t* const customEqualityFunction,
    vector_custom_sorting_function_t* const customSortingFunction);

/**
 * @brief frees the memory for a `vector_t` and its elements.
 *
 * @param vector A double pointer to the vector to be freed.
 * 
 * @return
 * - `VECTOR_SUCCESS` if the vector was successfully freed.
 *
 * - `VECTOR_NULL_ERROR` if the provided vector is NULL.
 * 
 * @note Sets the vector pointer to NULL after freeing.
 */
CONFETTI_EXPORT vector_result_t vector_free(vector_t** vectorOut);

/**
 * @brief Prints the contents of the vector to the standard output.
 * 
 * This function iterates through all elements of the specified vector
 * and prints their value's address. The function also prints the address 
 * of the vector itself at the end.
 *
 * @param vector A pointer to the vector to be printed.
 * 
 * @return 
 * - `VECTOR_SUCCESS` if the vector was printed successfully. 
 * 
 * - `VECTOR_NULL_ERROR` if the provided vector pointer is NULL.
 */
CONFETTI_EXPORT vector_result_t vector_print(vector_t* const vector);

/**
 * @brief Resizes the vector to the new capacity.
 * 
 * @param vector A pointer to the vector to be resized.
 * @param newCapacity The new capacity for the vector.
 * 
 * @return 
 * - `VECTOR_SUCCESS` if the vector was resized successfully. 
 * 
 * - `VECTOR_NULL_ERROR` if the provided vector pointer is NULL
 * 
 * - `VECTOR_INVALID_PARAMS_ERROR` if the new capacity is less than 1.
 * 
 * @note If the new capacity is the same as the current capacity, 
 *       it returns `VECTOR_SUCCESS` without making any changes.
 */
CONFETTI_EXPORT vector_result_t vector_resize(vector_t* const vector, const int64_t size);

/**
 * @brief Prepends a new element to the beginning of the vector.
 *
 * @param vector A pointer to the vector to which the element will be prepended.
 * @param value A pointer to the value to be prepended.
 * @param size The size of the value being prepended.
 * 
 * @return 
 * - `VECTOR_SUCCESS` if the element was prepended successfully.
 * 
 * - `VECTOR_NULL_ERROR` if the provided vector pointer is NULL.
 * 
 * - `VECTOR_ALLOCATION_FAILURE` if the system couldn't allocate enough memory for the operation.
 */
CONFETTI_EXPORT vector_result_t vector_prepend(vector_t* const vector, void* const value, const uint64_t size);

/**
 * @brief Adds a new element with the specified value to the
 * end of the given vector.
 *
 * @param vector A pointer to the vector to which the element will be appended.
 * @param value A pointer to the value to be appended.
 * @param size The size of the value to be appended.
 * 
 * @return 
 * - `VECTOR_SUCCESS` if the element was appended successfully. 
 * 
 * - `VECTOR_NULL_ERROR` if the provided vector pointer is NULL.
 * 
 * - `VECTOR_ALLOCATION_FAILURE` if the system couldn't allocate enough memory for the operation.
 */
CONFETTI_EXPORT vector_result_t vector_append(vector_t* const vector, void* const value, const uint64_t size);

/**
 * @brief Inserts a new element at a specified index in the vector.
 *
 * @param vector A pointer to the vector where the element will be inserted.
 * @param index The index at which the new element will be inserted.
 * @param value A pointer to the value to be inserted.
 * @param size The size of the value to be inserted.
 * 
 * @return 
 * - `VECTOR_SUCCESS` if the element was inserted successfully. 
 * 
 * - `VECTOR_NULL_ERROR` if the provided vector pointer is NULL.
 * 
 * - `VECTOR_INDEX_OUT_OF_RANGE_ERROR` if the index is out of range.
 */
CONFETTI_EXPORT vector_result_t vector_insert(vector_t* const vector, const int64_t index, void* const value, const uint64_t size);

/**
 * @brief Retrieves an element from the vector at the specified index.
 * 
 * This function fetches the element at the given index from the specified
 * vector and returns a clone of that element through the output parameter.
 *
 * @param vector A pointer to the vector from which to retrieve the element.
 * @param elementOut A double pointer to where the cloned element will be set.
 * @param index The index of the vector to retrieve.
 * 
 * @return 
 * - `VECTOR_SUCCESS` if the element was retrieved successfully.
 * 
 * - `VECTOR_NULL_ERROR` if the provided vector pointer is NULL.
 * 
 * - `VECTOR_INDEX_OUT_OF_RANGE_ERROR` if the given index is out of range.
 * 
 * @note The outputted `vector_element_t` is a clone that is allocated separately from
 * the one inside the vector, so freeing this element is your responsibility, it is 
 * recomended to use `vector_element_free` for this.
 */
CONFETTI_EXPORT vector_result_t vector_get(vector_t* const vector, vector_element_t** elementOut, const int64_t index);

/**
 * @brief Sets the value of an element in the vector at the specified index.
 * 
 * @param vector A pointer to the vector in which to set the element.
 * @param index The index of the element to set.
 * @param value A pointer to the new value to be assigned to the element.
 * @param size The size of the new value.
 * 
 * @return 
 * - `VECTOR_SUCCESS` if the value was set successfully. 
 * 
 * - `VECTOR_NULL_ERROR` if the provided vector pointer is NULL.
 * 
 * - `VECTOR_INDEX_OUT_OF_RANGE_ERROR` if the index is out of range.
 * 
 * - `VECTOR_INVALID_PARAMS_ERROR` if the value size is zero.
 */
CONFETTI_EXPORT vector_result_t vector_set(vector_t* const vector, const int64_t index, void* const value, const uint64_t size);

/**
 * @brief Removes an element from the vector at the specified index.
 *
 * @param vector A pointer to the vector from which the element will be removed.
 * @param index The index of the element to remove.
 * 
 * @return 
 * - `VECTOR_SUCCESS` if the element was removed successfully. 
 * 
 * - `VECTOR_NULL_ERROR` if the provided vector pointer is NULL.
 *  
 * - `VECTOR_INDEX_OUT_OF_RANGE_ERROR` if the index is out of range.
 */
CONFETTI_EXPORT vector_result_t vector_remove(vector_t* const vector, const int64_t index);

/**
 * @brief Pops an element from the vector at the specified index.
 *
 * @param vector A pointer to the vector from which the element will be popped.
 * @param index The index of the element to pop.
 * @param elementOut A double pointer to where the popped element will be stored.
 * 
 * @return 
 * - `VECTOR_SUCCESS` if the element was popped successfully.
 * 
 * - `VECTOR_NULL_ERROR` if the provided vector pointer is NULL.
 * 
 * - `VECTOR_INDEX_OUT_OF_RANGE_ERROR` if the index is out of range.
 * 
 * @note The outputted `vector_element_t` is a clone that is allocated separately from
 * the one inside the vector, so freeing this element is your responsibility, it is 
 * recomended to use `vector_element_free` for this.
 */
CONFETTI_EXPORT vector_result_t vector_pop(vector_t* const vector, vector_element_t** elementOut, const int64_t index);

/**
 * @brief Reverses the order of elements inside the vector in-place.
 *
 * @param vector A pointer to the vector to be reversed.
 * 
 * @return 
 * - `VECTOR_SUCCESS` if the vector was reversed successfully. 
 * 
 * - `VECTOR_NULL_ERROR` if the provided vector pointer is NULL.
 */
CONFETTI_EXPORT vector_result_t vector_reverse(vector_t* const vector);

/**
 * @brief Clones the specified vector, creating a new copy.
 * 
 * This function allocates memory for a new vector and copies the elements
 * from the original vector into the new vector. 
 *
 * @param vector A pointer to the vector to be cloned.
 * @param vectorOut A pointer to a pointer where the cloned vector will be stored.
 * 
 * @return 
 * - `VECTOR_SUCCESS` if the vector was cloned successfully. 
 * 
 * - `VECTOR_NULL_ERROR` if the provided vector pointer is NULL.
 * 
 * - `VECTOR_ALLOCATION_FAILURE` if memory allocation fails during the process.
 * 
 * @note Each element is cloned to ensure that the vector clone contains independent copies of the elements.
 */
CONFETTI_EXPORT vector_result_t vector_clone(vector_t* const vector, vector_t** vectorOut);

/**
 * @brief Clears all elements from the vector.
 * 
 * This function deallocates the memory associated with each element in
 * the specified vector and resets the size of the vector to zero.
 *
 * @param vector A pointer to the vector to be cleared.
 * 
 * @return 
 * - `VECTOR_SUCCESS` if the vector was cleared successfully. 
 * 
 * - `VECTOR_NULL_ERROR` if the provided vector pointer is NULL.
 */
CONFETTI_EXPORT vector_result_t vector_clear(vector_t* const vector);

/**
 * @brief Joins two vectors into a new vector.
 * 
 * This function creates a new vector that contains all elements from both
 * specified vectors, `vector1` and `vector2`. Each element from both vectors are cloned to ensure
 * that the new vector is independent from `vector1` and `vector2`.
 *
 * @param vector1 A pointer to the first vector to be joined.
 * @param vector2 A pointer to the second vector to be joined.
 * @param vectorOut A pointer to a pointer where the joined vector will be stored.
 * 
 * @return 
 * - `VECTOR_SUCCESS` if the vectors were joined successfully.
 * 
 * - `VECTOR_NULL_ERROR` if either of the provided vector pointers is NULL.
 * 
 * - `VECTOR_ALLOCATION_FAILURE` if the system couldn't allocate enough memory for the operation.
 */
CONFETTI_EXPORT vector_result_t vector_join(vector_t* const vector1, vector_t* const vector2, vector_t** vectorOut);

/**
 * @brief Checks if the vector includes a specific value.
 * 
 * This function searches through the specified vector to determine if it
 * contains an element that matches the provided value. It uses the vector's
 * equality function to check for a match.
 *
 * @param vector A pointer to the vector to be searched.
 * @param value A pointer to the value to search for in the vector.
 * @param size The size of the value to be compared.
 * 
 * @return 
 * - `VECTOR_SUCCESS` if the value is found in the vector.
 * 
 * - `VECTOR_NULL_ERROR` if the provided vector pointer is NULL.
 * 
 * - `VECTOR_ELEMENT_NOT_FOUND_ERROR` if the value is not found.
 */
CONFETTI_EXPORT vector_result_t vector_includes(vector_t* const vector, void* const value, const uint64_t size);

/**
 * @brief Finds the first occurrence of a specific value in the vector.
 * 
 * This function searches the specified vector for the first element that
 * matches the provided value, starting from a given index. It uses the
 * vector's equality function to compare the elements.
 *
 * @param vector A pointer to the vector to be searched.
 * @param indexOut A pointer to an integer where the index of the found element
 *                 will be stored.
 * @param startIndex The index from which to start the search.
 * @param value A pointer to a value to search for in the vector.
 * @param size The size of the value to be searched.
 * 
 * @return 
 * - `VECTOR_SUCCESS` if the value is found.
 * 
 * - `VECTOR_NULL_ERROR` if the provided vector pointer is NULL.
 * 
 * - `VECTOR_INVALID_PARAMS_ERROR` if the start index or size is invalid.
 * 
 * - `VECTOR_ELEMENT_NOT_FOUND_ERROR` if the value is not found.
 * 
 * @note On error indexOut will be set to -1.
 */
CONFETTI_EXPORT vector_result_t vector_find_first(
    vector_t* const vector, 
    int64_t* const indexOut, 
    const int64_t startIndex,
    void* const value, 
    const uint64_t size);

/**
 * @brief Finds the last occurrence of a specific value in the vector.
 * 
 * This function searches the specified vector for the last element that
 * matches the provided value, starting from a given index. It uses the
 * vector's equality function to compare the elements.
 *
 * @param vector A pointer to the vector to be searched.
 * @param indexOut A pointer to an integer where the index of the found element
 *                 will be stored.
 * @param startIndex The index from which to start the search.
 * @param value A pointer to a value to search for in the vector.
 * @param size The size of the value to be searched.
 * 
 * @return
 * - `VECTOR_SUCCESS` if the value is found.
 * 
 * - `VECTOR_NULL_ERROR` if the provided vector pointer is NULL.
 * 
 * - `VECTOR_INDEX_OUT_OF_RANGE_ERROR` if the start index is out of range.
 * 
 * - `VECTOR_INVALID_PARAMS_ERROR` if the size is less than or equal to 0.
 * 
 * - `VECTOR_ELEMENT_NOT_FOUND_ERROR` if the value is not found.
 */
CONFETTI_EXPORT vector_result_t vector_find_last(
    vector_t* const vector, 
    int64_t* const indexOut, 
    const int64_t startIndex, 
    void* const value, 
    const uint64_t size);

/**
 * @brief Sorts the elements of the vector.
 * 
 * This function sorts the elements of the specified vector using the
 * provided sorting function. The order of sorting is determined by the
 * `ascending` parameter.
 * 
 * @param vector A pointer to the vector to be sorted.
 * @param ascending A boolean value indicating the sort order; true for
 *                  ascending order and false for descending order.
 * 
 * @return 
 * - `VECTOR_SUCCESS` if the vector was sorted successfully.
 * 
 * - `VECTOR_INDEX_OUT_OF_RANGE_ERROR` if the sorting function encounters an index out of range.
 * 
 * - `VECTOR_ELEMENT_NOT_FOUND_ERROR` if the sorting function cannot find an element.
 * 
 * - `VECTOR_NULL_ERROR` if the provided vector pointer is NULL.
 * 
 * - `VECTOR_INVALID_PARAMS_ERROR` if invalid parameters are provided to the sorting function.
 * 
 * - `VECTOR_ALLOCATION_FAILURE` if the sorting function fails to allocate memory.
 * 
 * @warning The actual errors returned depend on the implementation of the sorting function.
 * 
 * @note If the default sorting function is used only `VECTOR_SUCCESS` should be returned.
 * @note If the vector is empty or contains only one element,
 *       the function returns success without performing any operations.
 */
CONFETTI_EXPORT vector_result_t vector_sort(vector_t* const vector, const bool ascending);

/**
 * @brief Fills the vector with a specified value up to its capacity.
 *
 * @param vector A pointer to the vector to be filled.
 * @param value A pointer to the value to be assigned.
 * @param size The size of the value to be assigned.
 * 
 * @return 
 * - `VECTOR_SUCCESS` if the vector was filled successfully. 
 * 
 * - `VECTOR_NULL_ERROR` if the provided vector pointer is NULL.
 * 
 * - `VECTOR_ALLOCATION_FAILURE` if the system couldn't allocate enough memory for the operation. 
 */
CONFETTI_EXPORT vector_result_t vector_fill(vector_t* const vector, void* const value, const uint64_t size);

/**
 * @brief Swaps two elements in the vector at the specified indices.
 *
 * This function exchanges the positions of two elements in the vector based on their indices. 
 *
 * @param vector A pointer to the vector containing the elements to be swapped.
 * @param index1 The index of the first element to swap.
 * @param index2 The index of the second element to swap.
 * 
 * @return 
 * - `VECTOR_SUCCESS` if the elements were successfully swapped.
 * 
 * - `VECTOR_NULL_ERROR` if the provided vector pointer is NULL.
 * 
 * - `VECTOR_INDEX_OUT_OF_RANGE_ERROR` if either index is out of the valid range of the vector.
 * 
 * @note If index1 and index2 are the same `VECTOR_SUCCESS` is returned without modifying anything.
 */
CONFETTI_EXPORT vector_result_t vector_swap(vector_t* const vector, const int64_t index1, const int64_t index2);

/**
 * @brief frees the memory allocated for a `vector_element_t`.
 *
 * @param element A double pointer to the vector element to be freed.
 * 
 * @return 
 * `VECTOR_SUCCESS` if the element was successfully freed. 
 * 
 * `VECTOR_INVALID_PARAMS_ERROR` if the provided element pointer is NULL.
 * 
 * @note Sets the element pointer to NULL after freeing.
 */
CONFETTI_EXPORT vector_result_t vector_element_free(vector_element_t** element);

/**
 * @brief Creates a new vector iterator.
 *
 *
 * @param iteratorOut A double pointer where the created iterator will be stored.
 * @param vector A pointer to the vector to be iterated over.
 * 
 * @return 
 * - `VECTOR_SUCCESS` if the iterator was successfully created.
 * 
 * - `VECTOR_NULL_ERROR` if the provided vector pointer is NULL.
 */
CONFETTI_EXPORT vector_result_t vector_iterator_create(vector_iterator_t** iteratorOut, vector_t* const vector);

/**
 * @brief Advances the iterator to the next element in the vector.
 *
 * @param iterator A pointer to the vector iterator to be advanced.
 * 
 * @return 
 * - `VECTOR_SUCCESS` if the iterator was successfully advanced to the next element.
 * 
 * - `VECTOR_INVALID_PARAMS_ERROR` if the provided iterator pointer is NULL.
 * 
 * - `VECTOR_INDEX_OUT_OF_RANGE_ERROR` if the iterator is already at the end of the vector.
 * 
 * @note When the iterator reaches the end of a vector it will rewind itself to avoid 
 *       lots of calls to `vector_iterator_rewind`.
 */
CONFETTI_EXPORT vector_result_t vector_iterator_next(vector_iterator_t* const iterator);

/**
 * @brief Resets the iterator to its initial state.
 *
 * This function rewinds the iterator, setting it to an invalid state with no current element. 
 * The index is reset to -1, and the element pointer is set to NULL.
 *
 * @param iterator A pointer to the vector iterator to be reset.
 * 
 * @return 
 * - `VECTOR_SUCCESS` if the iterator was successfully reset.
 * 
 * - `VECTOR_INVALID_PARAMS_ERROR` if the iterator is NULL.
 */
CONFETTI_EXPORT vector_result_t vector_iterator_rewind(vector_iterator_t* const iterator);

/**
 * @brief Frees the memory allocated for a vector iterator.
 *
 * This function deallocates the memory used by a vector iterator and resets its fields to NULL
 * or zero. After freeing the iterator, it sets the double pointer to NULL to avoid dangling pointers.
 *
 * @param iterator A double pointer to the vector iterator to be freed.
 * 
 * @return 
 * - `VECTOR_SUCCESS` if the iterator was successfully freed.
 * 
 * - `VECTOR_INVALID_PARAMS_ERROR` if the provided iterator pointer is NULL.
 * 
 * @note Sets the iterator pointer to NULL after freeing.
 * @note Only the iterator is freed, the vector used does not get freed.
 */
CONFETTI_EXPORT vector_result_t vector_iterator_free(vector_iterator_t** iterator);

#pragma endregion
