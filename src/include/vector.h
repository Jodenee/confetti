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

/**
 * @file vector.h
 * @brief Public interface for the Confetti dynamic vector implementation.
 *
 * Provides a dynamically allocated vector capable of storing elements of
 * arbitrary byte sizes.
 *
 * Elements are copied into vector-owned memory when inserted. The vector
 * supports custom comparison, sorting, printing and capacity growth
 * callbacks.
 *
 * Element retrieval and removal operations that return an element provide
 * independently allocated copies which must be released by the caller.
 *
 * @note The vector manages the memory associated with its stored elements.
 *       Callers remain responsible for managing the lifetime of their
 *       original input data.
 */

#pragma once

// Headers

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "confetti_export.h"

#pragma region Constants And Types Definitions

/**
 * @brief The default initial capacity used when creating a vector.
 *
 * This constant provides the recommended initial capacity for a new vector.
 *
 * @note Passing this value to vector_create() explicitly is required when
 *       the default capacity is desired.
 */
#define DEFAULT_VECTOR_CAPACITY ((int64_t)8)

typedef struct vector vector_t;
typedef struct vector_element vector_element_t;
typedef struct vector_iterator vector_iterator_t;
typedef enum vector_result vector_result_t;

/**
 * @brief Calculates a new capacity when a vector becomes full.
 *
 * This callback is invoked when an insertion operation requires additional
 * capacity. It is responsible for calculating and providing a proposed
 * capacity greater than the vector's current capacity.
 *
 * The proposed capacity must be greater than zero and within the supported
 * allocation limit. The vector validates the proposed capacity before
 * attempting to reallocate its internal storage.
 *
 * The callback must write the proposed capacity to newCapacityOut before
 * returning VECTOR_SUCCESS. It must not rely on the initial value of
 * newCapacityOut.
 *
 * @param currentCapacity The vector's current allocated capacity.
 * @param newCapacityOut Receives the proposed new capacity. Must not be NULL.
 *
 * @return VECTOR_SUCCESS if a new capacity was calculated successfully.
 * @return VECTOR_INVALID_PARAMS_ERROR if the callback receives invalid
 *         parameters.
 * @return VECTOR_SIZE_LIMIT_REACHED if a valid capacity increase cannot
 *         be provided.
 * @return Any other non-success result returned by the callback.
 *
 * @note The proposed capacity must be greater than currentCapacity.
 * @note The vector performs its own validation of the proposed capacity.
 *
 * @warning The callback must not modify the vector's internal storage.
 */
typedef vector_result_t (*vector_growing_strategy_function_t)(
    const int64_t currentCapacity,
    int64_t *const newCapacityOut);

/**
 * @brief Compares two vector elements.
 *
 * The comparison function determines element equality and ordering.
 *
 * It must return:
 * - A negative value if the first element is less than the second.
 * - Zero if both elements are considered equal.
 * - A positive value if the first element is greater than the second.
 *
 * The elements can have different sizes. The callback is responsible
 * for handling differing sizes appropriately.
 *
 * @param dataOne Pointer to the first element's data.
 * @param dataTwo Pointer to the second element's data.
 * @param dataOneSize Size of the first element in bytes.
 * @param dataTwoSize Size of the second element in bytes.
 *
 * @return A negative, zero, or positive comparison result.
 *
 * @note The default comparison compares the common byte prefix using
 *       memcmp and uses the element sizes to break ties.
 */
typedef int32_t (*vector_compare_function_t)(
    const void *const dataOne,
    const void *const dataTwo,
    const size_t dataOneSize,
    const size_t dataTwoSize);

/**
 * @brief Sorts a vector using a custom sorting implementation.
 *
 * The callback must sort the supplied vector in place.
 *
 * @param vector The vector to sort.
 * @param ascending True for ascending order, false for descending order.
 *
 * @return A vector_result_t describing the result of the operation.
 */
typedef vector_result_t (*vector_sorting_function_t)(vector_t *const vector, const bool ascending);

/**
 * @brief Prints one vector element to a stream.
 *
 * @param value Pointer to the element's data. May be NULL.
 * @param size Size of the element in bytes.
 * @param stream Stream to which the element should be printed.
 *
 * @return VECTOR_SUCCESS if printing succeeds.
 *
 * @note The callback is responsible for deciding how the element's
 *       contents should be represented.
 */
typedef vector_result_t (*vector_print_function_t)(
    const void *value,
    size_t size,
    FILE *stream);

#pragma endregion

#pragma region Public Structures

/**
 * @brief Represents an element in a vector.
 *
 * This structure stores an element's size and a pointer to its value.
 */
typedef struct vector_element
{
    size_t size; /* Size of the value in bytes. */
    void *value; /* Pointer to the data stored in the vector element. */
} vector_element_t;

/**
 * @brief Represents a dynamic vector data structure.
 *
 * This structure maintains a dynamic array of pointers to vector elements,
 * along with metadata for managing size, capacity, and custom behavior.
 */
typedef struct vector
{
    int64_t size;                                       /* Current number of elements in the vector. */
    int64_t capacity;                                   /* Total allocated capacity for elements. */
    vector_element_t **items;                           /* Array of pointers to vector elements. */
    vector_compare_function_t compare;                  /* function for comparing elements. */
    vector_sorting_function_t sort;                     /* function for ordering elements. */
    vector_print_function_t print;                      /* function for displaying elements with vector_print. */
    vector_growing_strategy_function_t growingStrategy; /* function for calculating the vector's new capacity when full. */
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
 * @warning Please do not manually free anything within this structure as
 * they are the internal values kept by the vector. If you wish
 * to deallocate memory from this structure please use `vector_iterator_free` to safely do so.
 */
typedef struct vector_iterator
{
    vector_t *vector;          /* The vector being iterated through. */
    int64_t index;             /* The index of the current iteration. */
    vector_element_t *element; /* The element of the current iteration. */
} vector_iterator_t;

#pragma endregion

#pragma region Enumerations

/**
 * @brief Represents the result of a vector operation.
 *
 * This enumeration defines various status codes that may be returned by
 * functions operating on vectors. Positive values indicate success, while
 * negative values represent specific error conditions.
 */
typedef enum vector_result
{
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
     * @brief Error: Vector is null.
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
    VECTOR_ALLOCATION_FAILURE = -5,

    VECTOR_SIZE_LIMIT_REACHED = -6,
} vector_result_t;

#pragma endregion

#pragma region Public Functions

/**
 * @brief Creates and initializes a new vector.
 *
 * Allocates a vector with the requested initial capacity and initializes
 * its size to zero.
 *
 * Custom callbacks can be provided to control capacity growth, element
 * comparison, sorting and printing. Any callback supplied as NULL is
 * replaced with its corresponding default implementation.
 *
 * @param capacity Initial capacity. Must be greater than zero.
 *        Use DEFAULT_VECTOR_CAPACITY to use the library's default value.
 * @param grow Custom capacity growth strategy, or NULL for the default.
 * @param compare Custom element comparison function, or NULL for the default.
 * @param sort Custom sorting function, or NULL for the default.
 * @param print Custom element printing function, or NULL for the default.
 * @param vectorOut Receives the newly created vector. Must point to NULL.
 *
 * @return VECTOR_SUCCESS if the vector was created successfully.
 * @return VECTOR_INVALID_PARAMS_ERROR if vectorOut is NULL, already contains
 *         a vector, or capacity is invalid.
 * @return VECTOR_SIZE_LIMIT_REACHED if the requested capacity exceeds
 *         the supported allocation limit.
 * @return VECTOR_ALLOCATION_FAILURE if memory allocation fails.
 */
CONFETTI_EXPORT vector_result_t vector_create(
    const int64_t capacity,
    vector_growing_strategy_function_t const grow,
    vector_compare_function_t const compare,
    vector_sorting_function_t const sort,
    vector_print_function_t const print,
    vector_t **vectorOut);

/**
 * @brief Frees a vector and all elements stored within it.
 *
 * Releases the memory allocated for every active element, the internal
 * element pointer array and the vector itself.
 *
 * After successful deallocation, the caller's vector pointer is set to NULL.
 *
 * @param vectorOut Pointer to the vector pointer to be freed.
 *
 * @return VECTOR_SUCCESS if the vector was successfully freed.
 * @return VECTOR_NULL_ERROR if vectorOut or *vectorOut is NULL.
 *
 * @note This operation permanently destroys the vector and all elements
 *       stored within it.
 */
CONFETTI_EXPORT vector_result_t vector_free(vector_t **vectorOut);

/**
 * @brief Prints the contents of a vector.
 *
 * Prints the vector's active elements using its configured printing callback.
 * The elements are displayed in their current order, followed by the
 * address of the vector itself.
 *
 * If stream is NULL, stdout is used as the destination stream.
 *
 * The default printing callback displays each element's data pointer
 * rather than interpreting or printing its stored bytes.
 *
 * @param vector Vector whose contents are to be printed.
 * @param stream Destination output stream, or NULL to use stdout.
 *
 * @return VECTOR_SUCCESS if the printing operation completes successfully.
 * @return VECTOR_NULL_ERROR if vector is NULL.
 *
 * @note Errors returned by the configured element-printing callback are
 *       propagated to the caller.
 */
CONFETTI_EXPORT vector_result_t vector_print(vector_t *const vector, FILE *stream);

/**
 * @brief Increases the allocated capacity of a vector.
 *
 * Reallocates the internal element pointer array to accommodate the
 * requested capacity. Existing elements remain unchanged.
 *
 * This operation only supports increasing capacity. Requests to reduce
 * the current capacity are rejected.
 *
 * @param vector Vector whose capacity is to be increased.
 * @param capacity Requested capacity. Must not be smaller than the
 *                 current capacity.
 *
 * @return VECTOR_SUCCESS if the capacity was successfully updated or
 *         already equals the requested capacity.
 * @return VECTOR_NULL_ERROR if vector is NULL.
 * @return VECTOR_INVALID_PARAMS_ERROR if capacity is smaller than the
 *         current capacity or is otherwise invalid.
 * @return VECTOR_SIZE_LIMIT_REACHED if the requested capacity exceeds
 *         the supported allocation limit.
 * @return VECTOR_ALLOCATION_FAILURE if memory allocation fails.
 */
CONFETTI_EXPORT vector_result_t vector_resize(vector_t *const vector, const int64_t capacity);

/**
 * @brief Inserts an element at the beginning of a vector.
 *
 * Creates an independent copy of the supplied data and inserts it at index
 * zero. Existing elements are shifted one position towards the end.
 *
 * If the vector is full, its configured growing strategy is invoked before
 * the element is inserted.
 *
 * @param vector Vector into which the element is to be inserted.
 * @param value Pointer to the data to be copied. May be NULL.
 * @param size Size of the supplied data in bytes. Must be greater than zero.
 *
 * @return VECTOR_SUCCESS if the element was inserted successfully.
 * @return VECTOR_NULL_ERROR if vector is NULL.
 * @return VECTOR_INVALID_PARAMS_ERROR if size is zero.
 * @return VECTOR_ALLOCATION_FAILURE if memory allocation fails.
 * @return VECTOR_SIZE_LIMIT_REACHED if the vector cannot increase its capacity.
 *
 * @note The vector stores its own copy of the supplied data. The caller
 *       retains ownership of the original data.
 */
CONFETTI_EXPORT vector_result_t vector_prepend(vector_t *const vector, void *const value, const size_t size);

/**
 * @brief Appends an element to the end of a vector.
 *
 * Creates an independent copy of the supplied data and inserts it at the
 * next available index.
 *
 * If the vector is full, its configured growing strategy is invoked before
 * the element is inserted.
 *
 * @param vector Vector to which the element is to be appended.
 * @param value Pointer to the data to be copied. May be NULL.
 * @param size Size of the supplied data in bytes. Must be greater than zero.
 *
 * @return VECTOR_SUCCESS if the element was appended successfully.
 * @return VECTOR_NULL_ERROR if vector is NULL.
 * @return VECTOR_INVALID_PARAMS_ERROR if size is zero.
 * @return VECTOR_ALLOCATION_FAILURE if memory allocation fails.
 * @return VECTOR_SIZE_LIMIT_REACHED if the vector cannot increase its capacity.
 *
 * @note The vector stores its own copy of the supplied data. The caller
 *       retains ownership of the original data.
 */
CONFETTI_EXPORT vector_result_t vector_append(vector_t *const vector, void *const value, const size_t size);

/**
 * @brief Inserts an element at a specified index in a vector.
 *
 * Creates an independent copy of the supplied data and inserts it at the
 * requested index. Existing elements at and after that index are shifted
 * one position towards the end.
 *
 * The index may range from zero to the vector's current size, inclusive.
 * An index equal to the current size appends the element to the end.
 *
 * If the vector is full, its configured growing strategy is invoked before
 * the element is inserted.
 *
 * @param vector Vector into which the element is to be inserted.
 * @param index Position at which the element is to be inserted.
 * @param value Pointer to the data to be copied. May be NULL.
 * @param size Size of the supplied data in bytes. Must be greater than zero.
 *
 * @return VECTOR_SUCCESS if the element was inserted successfully.
 * @return VECTOR_NULL_ERROR if vector is NULL.
 * @return VECTOR_INDEX_OUT_OF_RANGE_ERROR if index is smaller than zero
 *         or greater than the vector's current size.
 * @return VECTOR_INVALID_PARAMS_ERROR if size is zero.
 * @return VECTOR_ALLOCATION_FAILURE if memory allocation fails.
 * @return VECTOR_SIZE_LIMIT_REACHED if the vector cannot increase its capacity.
 *
 * @note The vector stores its own copy of the supplied data. The caller
 *       retains ownership of the original data.
 */
CONFETTI_EXPORT vector_result_t vector_insert(vector_t *const vector, const int64_t index, void *const value, const size_t size);

/**
 * @brief Retrieves an independent copy of an element.
 *
 * Creates a separate copy of the element stored at the specified index
 * and assigns it to elementOut.
 *
 * The returned element is not owned by the vector. The caller is responsible
 * for releasing it using vector_element_free().
 *
 * @param vector Vector from which the element is to be retrieved.
 * @param index Index of the element to retrieve.
 * @param elementOut Receives the cloned element. Must point to NULL.
 *
 * @return VECTOR_SUCCESS if the element was retrieved successfully.
 * @return VECTOR_NULL_ERROR if vector is NULL.
 * @return VECTOR_INDEX_OUT_OF_RANGE_ERROR if index is outside the valid range.
 * @return VECTOR_INVALID_PARAMS_ERROR if elementOut is NULL or *elementOut
 *         is not NULL.
 * @return VECTOR_ALLOCATION_FAILURE if memory allocation fails.
 *
 * @note Modifying or freeing the returned element does not modify the
 *       element stored in the original vector.
 */
CONFETTI_EXPORT vector_result_t vector_get(vector_t *vector, const int64_t index, vector_element_t **elementOut);

/**
 * @brief Replaces the data stored at a specified index.
 *
 * Replaces the contents of the element at index with a copy of the supplied
 * data.
 *
 * The existing element is retained as a vector element, but its stored data
 * and size are updated.
 *
 * @param vector Vector containing the element to be updated.
 * @param index Index of the element to update.
 * @param value Pointer to the replacement data. May be NULL.
 * @param size Size of the replacement data in bytes. Must be greater than zero.
 *
 * @return VECTOR_SUCCESS if the element was updated successfully.
 * @return VECTOR_NULL_ERROR if vector is NULL.
 * @return VECTOR_INDEX_OUT_OF_RANGE_ERROR if index is outside the valid range.
 * @return VECTOR_INVALID_PARAMS_ERROR if size is zero.
 * @return VECTOR_ALLOCATION_FAILURE if memory allocation fails.
 *
 * @note The vector stores its own copy of the supplied data. The caller
 *       retains ownership of the original data.
 */
CONFETTI_EXPORT vector_result_t vector_set(vector_t *const vector, const int64_t index, void *const value, const size_t size);

/**
 * @brief Removes an element from a vector.
 *
 * Frees the element stored at the specified index and shifts all subsequent
 * elements one position towards the beginning of the vector.
 *
 * The vector's capacity remains unchanged.
 *
 * @param vector Vector from which the element is to be removed.
 * @param index Index of the element to remove.
 *
 * @return VECTOR_SUCCESS if the element was removed successfully.
 * @return VECTOR_NULL_ERROR if vector is NULL.
 * @return VECTOR_INDEX_OUT_OF_RANGE_ERROR if index is outside the valid range.
 *
 * @note The removed element and its associated data are deallocated.
 */
CONFETTI_EXPORT vector_result_t vector_remove(vector_t *const vector, const int64_t index);

/**
 * @brief Removes an element and returns an independent copy of it.
 *
 * Creates a copy of the element at the specified index, removes and frees
 * the original element, and assigns the copy to elementOut.
 *
 * The returned element is independently allocated and is owned by the caller.
 * It must be released using vector_element_free().
 *
 * @param vector Vector from which the element is to be removed.
 * @param index Index of the element to remove.
 * @param elementOut Receives the cloned element. Must point to NULL.
 *
 * @return VECTOR_SUCCESS if the element was removed successfully.
 * @return VECTOR_NULL_ERROR if vector is NULL.
 * @return VECTOR_INDEX_OUT_OF_RANGE_ERROR if index is outside the valid range.
 * @return VECTOR_INVALID_PARAMS_ERROR if elementOut is NULL or *elementOut
 *         is not NULL.
 * @return VECTOR_ALLOCATION_FAILURE if cloning the element fails.
 *
 * @note The vector's capacity remains unchanged.
 */
CONFETTI_EXPORT vector_result_t vector_pop(vector_t *const vector, const int64_t index, vector_element_t **elementOut);

/**
 * @brief Reverses the order of elements in a vector.
 *
 * Reverses the positions of all active elements in place.
 *
 * The stored data is not copied or modified, and the vector's size and
 * capacity remain unchanged.
 *
 * @param vector Vector whose element order is to be reversed.
 *
 * @return VECTOR_SUCCESS if the vector was reversed successfully.
 * @return VECTOR_NULL_ERROR if vector is NULL.
 *
 * @note Vectors containing zero or one element are left unchanged.
 */
CONFETTI_EXPORT vector_result_t vector_reverse(vector_t *const vector);

/**
 * @brief Creates an independent copy of a vector.
 *
 * Creates a new vector with the same capacity and callback configuration
 * as the source vector.
 *
 * Every active element is cloned, including its stored data, so the new
 * vector does not share element allocations with the original vector.
 *
 * @param vector Vector to be cloned.
 * @param vectorOut Receives the newly created vector. Must point to NULL.
 *
 * @return VECTOR_SUCCESS if the vector was cloned successfully.
 * @return VECTOR_NULL_ERROR if vector is NULL.
 * @return VECTOR_INVALID_PARAMS_ERROR if vectorOut is NULL or *vectorOut
 *         is not NULL.
 * @return VECTOR_ALLOCATION_FAILURE if memory allocation fails.
 * @return VECTOR_SIZE_LIMIT_REACHED if the vector cannot be created within
 *         the supported capacity limits.
 *
 * @note The cloned vector inherits the callback configuration of the
 *       original vector.
 * @note The caller is responsible for releasing the cloned vector using
 *       vector_free().
 */
CONFETTI_EXPORT vector_result_t vector_clone(vector_t *const vector, vector_t **vectorOut);

/**
 * @brief Removes all elements from a vector.
 *
 * Frees every active element and its associated data, then resets the
 * vector's size to zero.
 *
 * The vector itself is retained, along with its allocated capacity and
 * callback configuration.
 *
 * @param vector Vector to be cleared.
 *
 * @return VECTOR_SUCCESS if the vector was cleared successfully.
 * @return VECTOR_NULL_ERROR if vector is NULL.
 *
 * @note This operation does not deallocate the vector or its internal
 *       element pointer array.
 */
CONFETTI_EXPORT vector_result_t vector_clear(vector_t *const vector);

/**
 * @brief Joins two vectors into a new vector.
 *
 * Creates a new vector containing independent copies of all active elements
 * from vector1, followed by all active elements from vector2.
 *
 * The resulting vector's capacity is the sum of the capacities of the
 * two source vectors.
 *
 * The new vector inherits the callback configuration of vector1.
 *
 * @param vector1 First vector whose elements are to be copied.
 * @param vector2 Second vector whose elements are to be copied.
 * @param vectorOut Receives the newly created vector. Must point to NULL.
 *
 * @return VECTOR_SUCCESS if the vectors were joined successfully.
 * @return VECTOR_NULL_ERROR if either source vector is NULL.
 * @return VECTOR_INVALID_PARAMS_ERROR if vectorOut is NULL, *vectorOut
 *         is not NULL, or the combined capacity cannot be represented.
 * @return VECTOR_SIZE_LIMIT_REACHED if the combined capacity exceeds the
 *         supported allocation limit.
 * @return VECTOR_ALLOCATION_FAILURE if memory allocation fails.
 *
 * @note The source vectors and their elements remain unchanged.
 * @note The resulting vector is independent of both source vectors.
 */
CONFETTI_EXPORT vector_result_t vector_join(vector_t *const vector1, vector_t *const vector2, vector_t **vectorOut);

/**
 * @brief Checks whether a vector contains a matching element.
 *
 * Searches the vector for an element whose size matches the supplied size
 * and whose contents are considered equal by the vector's comparison
 * callback.
 *
 * @param vector Vector to search.
 * @param value Pointer to the data to search for. May be NULL.
 * @param size Size of the supplied data in bytes. Must be greater than zero.
 *
 * @return VECTOR_SUCCESS if a matching element is found.
 * @return VECTOR_NULL_ERROR if vector is NULL.
 * @return VECTOR_INVALID_PARAMS_ERROR if size is zero.
 * @return VECTOR_ELEMENT_NOT_FOUND_ERROR if no matching element is found.
 *
 * @note The vector's configured comparison callback determines whether
 *       two elements are considered equal.
 */
CONFETTI_EXPORT vector_result_t vector_includes(
    vector_t *const vector,
    void *const value,
    const size_t size);

/**
 * @brief Finds the first matching element in a vector.
 *
 * Searches forwards through the vector, beginning at startIndex, and
 * returns the index of the first matching element encountered.
 *
 * An element is considered a match when its size matches the supplied
 * size and the vector's comparison callback considers its data equal
 * to value.
 *
 * @param vector Vector to search.
 * @param startIndex Index at which the forward search begins. Must be
 *                   greater than or equal to zero and less than the
 *                   vector's size.
 * @param value Pointer to the data to search for. May be NULL if supported
 *              by the vector's comparison callback.
 * @param size Size of the supplied data in bytes. Must be greater than zero.
 * @param indexOut Receives the index of the first matching element.
 *
 * @return VECTOR_SUCCESS if a matching element is found.
 * @return VECTOR_NULL_ERROR if vector is NULL.
 * @return VECTOR_INVALID_PARAMS_ERROR if indexOut is NULL, startIndex is
 *         outside the permitted range, or size is zero.
 * @return VECTOR_ELEMENT_NOT_FOUND_ERROR if no matching element is found.
 *
 * @note On failure, indexOut is set to -1 when the output pointer is valid.
 */
CONFETTI_EXPORT vector_result_t vector_find_first(
    vector_t *const vector,
    const int64_t startIndex,
    void *const value,
    const size_t size,
    int64_t *const indexOut);

/**
 * @brief Finds the last matching element in a vector.
 *
 * Searches backwards through the vector, beginning at startIndex, and
 * returns the index of the first matching element encountered during
 * the reverse search.
 *
 * An element is considered a match when its size matches the supplied
 * size and the vector's comparison callback considers its data equal
 * to value.
 *
 * @param vector Vector to search.
 * @param startIndex Index at which the reverse search begins. Must be
 *                   greater than or equal to zero and less than the
 *                   vector's size.
 * @param value Pointer to the data to search for. May be NULL if supported
 *              by the vector's comparison callback.
 * @param size Size of the supplied data in bytes. Must be greater than zero.
 * @param indexOut Receives the index of the last matching element.
 *
 * @return VECTOR_SUCCESS if a matching element is found.
 * @return VECTOR_NULL_ERROR if vector is NULL.
 * @return VECTOR_INVALID_PARAMS_ERROR if indexOut is NULL, startIndex is
 *         outside the permitted range, or size is zero.
 * @return VECTOR_ELEMENT_NOT_FOUND_ERROR if no matching element is found.
 *
 * @note On failure, indexOut is set to -1 when the output pointer is valid.
 */
CONFETTI_EXPORT vector_result_t vector_find_last(
    vector_t *const vector,
    const int64_t startIndex,
    void *const value,
    const size_t size,
    int64_t *const indexOut);

/**
 * @brief Sorts the elements of a vector.
 *
 * Sorts the vector's active elements in place using its configured sorting
 * callback.
 *
 * The vector's comparison callback is available to the sorting
 * implementation to determine the relative ordering of elements.
 *
 * @param vector Vector whose elements are to be sorted.
 * @param ascending True to sort in ascending order, or false to sort in
 *                  descending order.
 *
 * @return VECTOR_SUCCESS if the sorting operation completes successfully.
 * @return VECTOR_NULL_ERROR if vector is NULL.
 * @return Any non-success result returned by the configured sorting callback.
 *
 * @note When the default sorting callback is used, vectors containing zero
 *       or one element are left unchanged and return VECTOR_SUCCESS.
 *
 * @warning The errors returned by this function depend on the configured
 *          sorting callback.
 */
CONFETTI_EXPORT vector_result_t vector_sort(vector_t *const vector, const bool ascending);

/**
 * @brief Swaps the positions of two elements in a vector.
 *
 * Exchanges the element pointers stored at index1 and index2.
 *
 * The elements' data, sizes and allocations are not modified. Only their
 * positions within the vector are exchanged.
 *
 * @param vector Vector containing the elements to be swapped.
 * @param index1 Index of the first element.
 * @param index2 Index of the second element.
 *
 * @return VECTOR_SUCCESS if the elements were swapped successfully.
 * @return VECTOR_NULL_ERROR if vector is NULL.
 * @return VECTOR_INDEX_OUT_OF_RANGE_ERROR if either index is outside
 *         the valid range.
 *
 * @note If both indices are equal, the operation succeeds without making
 *       any changes.
 */
CONFETTI_EXPORT vector_result_t vector_swap(vector_t *const vector, const int64_t index1, const int64_t index2);

/**
 * @brief Frees a vector element and its associated data.
 *
 * Releases the memory allocated for the element's stored data and the
 * vector_element_t structure itself.
 *
 * After successful deallocation, the caller's element pointer is set to NULL.
 *
 * @param element Pointer to the element pointer to be freed.
 *
 * @return VECTOR_SUCCESS if the element was freed successfully.
 * @return VECTOR_INVALID_PARAMS_ERROR if element or *element is NULL.
 *
 * @note This function should be used to release elements returned by
 *       vector_get() and vector_pop().
 * @note Do not use this function to free an element that is still owned
 *       by a vector.
 */
CONFETTI_EXPORT vector_result_t vector_element_free(vector_element_t **element);

/**
 * @brief Creates an iterator for a vector.
 *
 * Allocates and initialises an iterator associated with the supplied vector.
 *
 * The iterator starts before the first element, with its index set to -1
 * and its current element pointer set to NULL.
 *
 * @param vector Vector to be traversed.
 * @param iteratorOut Receives the newly created iterator. Must point to NULL.
 *
 * @return VECTOR_SUCCESS if the iterator was created successfully.
 * @return VECTOR_NULL_ERROR if vector is NULL.
 * @return VECTOR_INVALID_PARAMS_ERROR if iteratorOut is NULL or *iteratorOut
 *         is not NULL.
 * @return VECTOR_ALLOCATION_FAILURE if memory allocation fails.
 *
 * @note The iterator does not own the vector. The vector must remain valid
 *       while the iterator is in use.
 */
CONFETTI_EXPORT vector_result_t vector_iterator_create(vector_t *const vector, vector_iterator_t **iteratorOut);

/**
 * @brief Advances an iterator to the next element.
 *
 * Advances the iterator's index and updates its current element pointer
 * to reference the next element in the associated vector.
 *
 * If the vector is empty or the iterator has reached its final element,
 * the current element pointer is set to NULL and
 * VECTOR_INDEX_OUT_OF_RANGE_ERROR is returned.
 *
 * When the iterator is exhausted, its index remains unchanged. Repeated
 * calls continue returning VECTOR_INDEX_OUT_OF_RANGE_ERROR until the
 * iterator is rewound.
 *
 * @param iterator Iterator to advance. Must not be NULL and must reference
 *                 a valid vector.
 *
 * @return VECTOR_SUCCESS if the iterator advances successfully.
 * @return VECTOR_INVALID_PARAMS_ERROR if iterator is NULL.
 * @return VECTOR_INDEX_OUT_OF_RANGE_ERROR if the vector is empty or
 *         the iterator has reached its final element.
 *
 * @note The iterator must remain associated with a valid vector throughout
 *       its lifetime.
 * @note Call vector_iterator_rewind() to restart iteration.
 */
CONFETTI_EXPORT vector_result_t vector_iterator_next(vector_iterator_t *const iterator);

/**
 * @brief Resets an iterator to its initial state.
 *
 * Sets the iterator's index to -1 and its current element pointer to NULL.
 *
 * The next call to vector_iterator_next() advances the iterator to the
 * first element of the associated vector.
 *
 * @param iterator Iterator to reset.
 *
 * @return VECTOR_SUCCESS if the iterator was reset successfully.
 * @return VECTOR_INVALID_PARAMS_ERROR if iterator is NULL.
 */
CONFETTI_EXPORT vector_result_t vector_iterator_rewind(vector_iterator_t *const iterator);

/**
 * @brief Frees an iterator.
 *
 * Releases the memory allocated for the iterator and sets the caller's
 * iterator pointer to NULL.
 *
 * The vector associated with the iterator is not freed or modified.
 *
 * @param iterator Pointer to the iterator pointer to be freed.
 *
 * @return VECTOR_SUCCESS if the iterator was freed successfully.
 * @return VECTOR_INVALID_PARAMS_ERROR if iterator or *iterator is NULL.
 *
 * @note The associated vector remains valid after the iterator is freed.
 */
CONFETTI_EXPORT vector_result_t vector_iterator_free(vector_iterator_t **iterator);

#pragma endregion
