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

#include "vector.h"

// private function definitions

#pragma region private function definitions

/**
 * @brief Adds two signed 64-bit integers while checking for overflow.
 *
 * Calculates the sum of the two supplied integers and stores the result
 * in the destination only if the addition can be represented by an
 * int64_t value.
 *
 * The destination pointer must reference a valid int64_t object.
 *
 * @param numberOne The first integer operand.
 * @param numberTwo The second integer operand.
 * @param destination Pointer to where the calculated sum will be stored.
 *
 * @return `true` if the addition succeeds without overflow.
 * @return `false` if the addition would cause signed integer overflow.
 *
 * @note The destination is left unchanged if overflow is detected.
 */
static bool safe_add_int64(int64_t numberOne, int64_t numberTwo, int64_t *destination);

/**
 * @brief Creates a vector element containing a copy of the supplied data.
 *
 * Allocates a new vector element and initialises its size and value.
 * When value is not `NULL`, the function allocates separate storage
 * and copies size bytes into it.
 *
 * When value is `NULL`, the element is created with a `NULL` value
 * pointer while retaining the supplied size.
 *
 * The output pointer must initially point to `NULL`.
 *
 * @param value Pointer to the data to copy, or `NULL`.
 * @param size Size of the element data in bytes. Must be greater than zero.
 * @param elementOut Receives the newly created element.
 *
 * @return `VECTOR_SUCCESS` if the element is created successfully.
 * @return `VECTOR_INVALID_PARAMS_ERROR` if elementOut is `NULL`,
 *         *elementOut is not `NULL`, or size is zero.
 * @return `VECTOR_ALLOCATION_FAILURE` if memory allocation fails.
 *
 * @note The returned element owns its allocated value storage.
 */
static vector_result_t vector_element_create(const void *const value, const size_t size, vector_element_t **elementOut);

/**
 * @brief Replaces the value and size of an existing vector element.
 *
 * Updates the specified element with the supplied data.
 *
 * When value is `NULL`, the existing value allocation is freed and
 * the element's value pointer is set to `NULL`.
 *
 * When value is not `NULL`, new storage is allocated and the supplied
 * data is copied before the existing allocation is released. This
 * allows value to refer to data within the element's existing allocation.
 *
 * If allocation fails, the existing element remains unchanged.
 *
 * @param element Pointer to the element to update.
 * @param value Pointer to the replacement data, or `NULL`.
 * @param size Size of the replacement data in bytes. Must be greater than zero.
 *
 * @return `VECTOR_SUCCESS` if the element is updated successfully.
 * @return `VECTOR_INVALID_PARAMS_ERROR` if element is `NULL` or size is zero.
 * @return `VECTOR_ALLOCATION_FAILURE` if memory allocation fails.
 */
static vector_result_t vector_element_set(vector_element_t *const element, const void *const value, const size_t size);

/**
 * @brief Creates an independent copy of an existing vector element.
 *
 * Allocates a new element and copies the source element's size and value.
 * If the source element's value is `NULL`, the cloned element also has
 * a `NULL` value pointer.
 *
 * The cloned element owns its own value allocation and does not share
 * storage with the original element.
 *
 * The output pointer must initially point to `NULL`.
 *
 * @param element Pointer to the element to clone.
 * @param elementOut Receives the newly created element.
 *
 * @return `VECTOR_SUCCESS` if the element is cloned successfully.
 * @return `VECTOR_INVALID_PARAMS_ERROR` if element or elementOut is `NULL`,
 *         or if *elementOut is not `NULL`.
 * @return `VECTOR_ALLOCATION_FAILURE` if memory allocation fails.
 */
static vector_result_t vector_element_clone(vector_element_t *const element, vector_element_t **elementOut);

/**
 * @brief Validates a vector capacity value.
 *
 * Checks that the requested capacity is positive and that allocating
 * an array of vector element pointers of that capacity will not exceed
 * the maximum representable size_t allocation size.
 *
 * @param capacity The capacity to validate.
 *
 * @return `VECTOR_SUCCESS` if the capacity is valid.
 * @return `VECTOR_INVALID_PARAMS_ERROR` if capacity is zero or negative.
 * @return `VECTOR_SIZE_LIMIT_REACHED` if the capacity would exceed
 *         the maximum supported allocation size.
 */
static vector_result_t validate_capacity(int64_t capacity);

/**
 * @brief Increases the allocated capacity of a vector.
 *
 * Allocates a new element-pointer array with the requested capacity,
 * copies the pointers to all active elements into the new array,
 * and initialises the remaining slots to `NULL`.
 *
 * Once the new allocation has been successfully prepared, the previous
 * pointer array is released and the vector's items pointer and capacity
 * are updated.
 *
 * The capacity must not be smaller than the vector's current capacity.
 *
 * @param vector Pointer to the vector whose capacity is to be increased.
 * @param capacity The requested capacity.
 *
 * @return `VECTOR_SUCCESS` if the capacity is updated successfully.
 * @return `VECTOR_INVALID_PARAMS_ERROR` if vector is `NULL` or the
 *         requested capacity is smaller than the current capacity.
 * @return `VECTOR_SIZE_LIMIT_REACHED` if the requested capacity exceeds
 *         the supported allocation limit.
 * @return `VECTOR_ALLOCATION_FAILURE` if memory allocation fails.
 *
 * @note Existing element allocations are not copied or moved. Only their
 *       pointers are transferred to the new array.
 */
static vector_result_t vector_realloc_capacity(vector_t *const vector, const int64_t capacity);

/**
 * @brief Increases a vector's capacity using its configured growth strategy.
 *
 * Invokes the vector's growing strategy callback with the current capacity
 * to obtain a proposed new capacity.
 *
 * The proposed capacity is validated and must be greater than the current
 * capacity. If valid, the internal capacity reallocation helper is invoked.
 *
 * @param vector Pointer to the vector whose capacity is to be increased.
 *
 * @return `VECTOR_SUCCESS` if the vector's capacity is increased successfully.
 * @return The non-success result returned by the growing strategy callback,
 *         if the callback fails.
 * @return `VECTOR_INVALID_PARAMS_ERROR` if the proposed capacity is invalid.
 * @return `VECTOR_SIZE_LIMIT_REACHED` if the proposed capacity does not
 *         exceed the current capacity or exceeds the supported size limit.
 * @return `VECTOR_ALLOCATION_FAILURE` if capacity reallocation fails.
 *
 * @note This helper assumes that vector is valid and has a configured
 *       growing strategy callback.
 */
static vector_result_t vector_grow(vector_t *vector);

/**
 * @brief Compares two data values using the default comparison behaviour.
 *
 * Compares the supplied data pointers and their associated sizes.
 *
 * If both pointers are `NULL`, they are considered equal regardless of
 * their associated sizes. If only one pointer is `NULL`, the `NULL`
 * pointer is considered smaller.
 *
 * When both pointers are non-`NULL`, their common byte range is compared
 * using memcmp. If that range is equal, the data sizes are compared to
 * determine the result.
 *
 * @param dataOne Pointer to the first data value.
 * @param dataTwo Pointer to the second data value.
 * @param dataOneSize Size of the first data value in bytes.
 * @param dataTwoSize Size of the second data value in bytes.
 *
 * @return A negative value if the first value is considered smaller.
 * @return Zero if the values are considered equal.
 * @return A positive value if the first value is considered greater.
 *
 * @note This comparison treats data as byte sequences. It does not
 *       interpret the underlying data as particular C types.
 */
static int32_t default_compare(
    const void *const dataOne,
    const void *const dataTwo,
    const size_t dataOneSize,
    const size_t dataTwoSize);

/**
 * @brief Calculates the default increased vector capacity.
 *
 * Calculates a new capacity by doubling the current capacity.
 * The proposed capacity is validated before being written to the output.
 *
 * The function rejects calculations that would overflow an int64_t value
 * or produce a capacity exceeding the maximum supported allocation size.
 *
 * @param currentCapacity The vector's current capacity.
 * @param newCapacityOut Receives the calculated capacity.
 *
 * @return `VECTOR_SUCCESS` if the new capacity is calculated successfully.
 * @return `VECTOR_INVALID_PARAMS_ERROR` if newCapacityOut is `NULL`
 *         or the calculated capacity is invalid.
 * @return `VECTOR_SIZE_LIMIT_REACHED` if doubling the current capacity
 *         would exceed the supported size limit.
 */
static vector_result_t default_growing_strategy(
    int64_t const currentCapacity, int64_t *const newCapacityOut
);

/**
 * @brief Sorts a range of vector elements using the quicksort algorithm.
 *
 * Sorts the elements between the supplied low and high indices,
 * inclusive, using the vector's configured comparison callback.
 *
 * The pivot is selected from the middle of the current range. Elements
 * are partitioned around the pivot according to the requested sort order.
 *
 * The algorithm recursively processes the smaller partition and iterates
 * over the larger partition to limit recursive stack usage.
 *
 * @param vector Pointer to the vector being sorted.
 * @param low First index of the range to sort.
 * @param high Last index of the range to sort.
 * @param ascending `true` for ascending order or `false` for descending order.
 *
 * @pre vector must be valid and have a configured comparison callback.
 * @pre The supplied range must contain valid active element indices.
 *
 * @note This function reorders element pointers without changing the
 *       elements' data, sizes, or ownership.
 */
static void vector_quicksort_range(
    vector_t *vector, int64_t low, int64_t high, bool ascending
);

/**
 * @brief Sorts the vector using the default sorting implementation.
 *
 * Uses vector_quicksort_range() to sort all active vector elements
 * according to the configured comparison callback.
 *
 * Vectors containing zero or one element require no sorting and
 * immediately return success.
 *
 * @param vector Pointer to the vector to sort.
 * @param ascending `true` for ascending order or `false` for descending order.
 *
 * @return `VECTOR_SUCCESS` when sorting completes.
 *
 * @note This function assumes vector is valid and has a configured
 *       comparison callback.
 */
static vector_result_t default_sort(vector_t *const vector, const bool ascending);

/**
 * @brief Prints the default representation of a vector element.
 *
 * Prints the literal `NULL` when value is `NULL`. Otherwise, prints
 * the memory address represented by value.
 *
 * The supplied stream is used as the output destination and remains
 * owned by the caller.
 *
 * @param value Pointer to the element data.
 * @param size Size of the element data in bytes. Unused by this callback.
 * @param stream Stream to which the representation is written.
 * 
 * @pre stream must be a valid, open output stream.
 *
 * @return `VECTOR_SUCCESS` if the output operation succeeds.
 * @return `VECTOR_IO_ERROR` if fprintf() reports an output error.
 *
 * @note This callback intentionally prints the data address rather than
 *       interpreting or formatting the contents of the data.
 */
static vector_result_t default_print(
    const void *value, size_t size, FILE *stream
);

#pragma endregion

// private functions

#pragma region private functions

static bool safe_add_int64(int64_t numberOne, int64_t numberTwo, int64_t *destination)
{
    if (numberTwo > 0 && numberOne > INT64_MAX - numberTwo)
        return false;
    else if (numberTwo < 0 && numberOne < INT64_MIN - numberTwo)
        return false;

    *destination = numberOne + numberTwo;
    return true;
}

static vector_result_t vector_element_create(const void *const value, const size_t size, vector_element_t **elementOut)
{
    if (elementOut == NULL || *elementOut != NULL)
        return VECTOR_INVALID_PARAMS_ERROR;
    else if (size == 0)
        return VECTOR_INVALID_PARAMS_ERROR;

    vector_element_t *element = (vector_element_t *)malloc(sizeof(vector_element_t));

    if (element == NULL)
        return VECTOR_ALLOCATION_FAILURE;

    element->value = NULL;
    element->size = size;

    if (value != NULL)
    {
        element->value = (void *)malloc(size);

        if (element->value == NULL)
        {
            free(element);
            element = NULL;

            return VECTOR_ALLOCATION_FAILURE;
        }

        memcpy(element->value, value, size);
    }

    *elementOut = element;
    return VECTOR_SUCCESS;
}

static vector_result_t vector_element_set(vector_element_t *const element, const void *value, const size_t size)
{
    if (element == NULL)
        return VECTOR_INVALID_PARAMS_ERROR;
    else if (size == 0)
        return VECTOR_INVALID_PARAMS_ERROR;

    if (element->value == NULL)
    {
        if (value == NULL)
        {
            element->value = NULL;
            element->size = size;
            return VECTOR_SUCCESS;
        }
    }

    if (element->value != NULL)
    {
        if (value == NULL)
        {
            free(element->value);

            element->value = NULL;
            element->size = size;
            return VECTOR_SUCCESS;
        }
    }

    void *valueCopy = (void*) malloc(size);

    if (valueCopy == NULL)
        return VECTOR_ALLOCATION_FAILURE;

    memcpy(valueCopy, value, size);

    free(element->value);

    element->value = valueCopy;
    element->size = size;

    return VECTOR_SUCCESS;
}

static vector_result_t vector_element_clone(vector_element_t *const element, vector_element_t **elementOut)
{
    if (element == NULL)
        return VECTOR_INVALID_PARAMS_ERROR;
    if (elementOut == NULL || *elementOut != NULL)
        return VECTOR_INVALID_PARAMS_ERROR;

    vector_element_t *elementClone = (vector_element_t *)malloc(sizeof(vector_element_t));

    if (elementClone == NULL)
        return VECTOR_ALLOCATION_FAILURE;

    elementClone->value = NULL;
    elementClone->size = element->size;

    if (element->value != NULL)
    {
        elementClone->value = (void *)malloc(element->size);

        if (elementClone->value == NULL)
        {
            free(elementClone);
            return VECTOR_ALLOCATION_FAILURE;
        }

        memcpy(elementClone->value, element->value, element->size);
    }

    *elementOut = elementClone;
    return VECTOR_SUCCESS;
}

static vector_result_t validate_capacity(int64_t capacity)
{
    if (capacity <= 0)
        return VECTOR_INVALID_PARAMS_ERROR;

    if ((uintmax_t)capacity > (uintmax_t)(SIZE_MAX / sizeof(vector_element_t *)))
        return VECTOR_SIZE_LIMIT_REACHED;

    return VECTOR_SUCCESS;
}

static vector_result_t vector_realloc_capacity(
    vector_t *const vector,
    const int64_t newCapacity)
{
    if (vector == NULL)
        return VECTOR_INVALID_PARAMS_ERROR;
    else if (newCapacity < vector->capacity)
        return VECTOR_INVALID_PARAMS_ERROR;

    vector_result_t validationResult = validate_capacity(newCapacity);

    if (validationResult != VECTOR_SUCCESS)
        return validationResult;

    vector_element_t **newItems = NULL;

    newItems = (vector_element_t **)malloc(newCapacity * sizeof(vector_element_t *));

    if (newItems == NULL)
        return VECTOR_ALLOCATION_FAILURE;

    memcpy(
        newItems,
        vector->items,
        vector->size * sizeof(vector_element_t *));

    for (int64_t i = vector->size; i < newCapacity; ++i)
        newItems[i] = NULL;

    free(vector->items);
    vector->items = newItems;
    vector->capacity = newCapacity;

    return VECTOR_SUCCESS;
}

static vector_result_t vector_grow(vector_t *vector)
{
    int64_t newCapacity = 0;
    vector_result_t growResult = vector->growingStrategy(vector->capacity, &newCapacity);
    vector_result_t validationResult = validate_capacity(newCapacity);

    if (growResult != VECTOR_SUCCESS)
        return growResult;
    if (validationResult != VECTOR_SUCCESS)
        return validationResult;
    else if (newCapacity <= vector->capacity)
        return VECTOR_SIZE_LIMIT_REACHED;

    vector_result_t reallocResult = vector_realloc_capacity(vector, newCapacity);

    if (reallocResult != VECTOR_SUCCESS)
        return reallocResult;

    return VECTOR_SUCCESS;
}

static int32_t default_compare(
    const void *const dataOne,
    const void *const dataTwo,
    const size_t dataOneSize,
    const size_t dataTwoSize)
{
    if (dataOne == NULL && dataTwo == NULL)
        return 0;

    if (dataOne == NULL)
        return -1;

    if (dataTwo == NULL)
        return 1;

    size_t smallestSize = dataOneSize < dataTwoSize
                              ? dataOneSize
                              : dataTwoSize;

    int result = memcmp(dataOne, dataTwo, smallestSize);

    if (result != 0)
        return result < 0 ? -1 : 1;

    if (dataOneSize < dataTwoSize)
        return -1;

    if (dataOneSize > dataTwoSize)
        return 1;

    return 0;
}

static vector_result_t default_growing_strategy(int64_t const currentCapacity, int64_t *const newCapacityOut)
{
    if (newCapacityOut == NULL)
        return VECTOR_INVALID_PARAMS_ERROR;
    else if (currentCapacity > INT64_MAX / 2)
        return VECTOR_SIZE_LIMIT_REACHED;

    int64_t newCapacity = currentCapacity * 2;

    vector_result_t validationResult = validate_capacity(newCapacity);

    if (validationResult != VECTOR_SUCCESS)
        return validationResult;

    *newCapacityOut = newCapacity;
    return VECTOR_SUCCESS;
}

static void vector_quicksort_range(
    vector_t *vector,
    int64_t low,
    int64_t high,
    bool ascending)
{
    while (low < high)
    {
        int64_t left = low;
        int64_t right = high;
        int64_t pivot_index = low + (high - low) / 2;

        vector_element_t *pivot = vector->items[pivot_index];

        while (left <= right)
        {
            int comparison;

            while (left <= high)
            {
                vector_element_t *left_element = vector->items[left];

                comparison = vector->compare(
                    left_element->value,
                    pivot->value,
                    left_element->size,
                    pivot->size);

                if (!ascending)
                {
                    comparison = comparison < 0   ? 1
                                 : comparison > 0 ? -1
                                                  : 0;
                }

                if (comparison >= 0)
                {
                    break;
                }

                ++left;
            }

            while (right >= low)
            {
                vector_element_t *rightElement = vector->items[right];

                comparison = vector->compare(
                    rightElement->value,
                    pivot->value,
                    rightElement->size,
                    pivot->size);

                if (!ascending)
                {
                    comparison = comparison < 0   ? 1
                                 : comparison > 0 ? -1
                                                  : 0;
                }

                if (comparison <= 0)
                {
                    break;
                }

                /*
                 * Avoid unsigned underflow.
                 */
                if (right == 0)
                {
                    break;
                }

                --right;
            }

            if (left > right)
            {
                break;
            }

            if (left != right)
            {
                vector_swap(
                    vector,
                    left,
                    right);
            }

            ++left;

            if (right == 0)
            {
                break;
            }

            --right;
        }

        if (right >= low && right - low < high - left + 1)
        {
            if (low < right)
            {
                vector_quicksort_range(
                    vector, low, right, ascending);
            }

            low = left;
        }
        else
        {
            if (left < high)
            {
                vector_quicksort_range(
                    vector, left, high, ascending);
            }

            if (right < low || right == 0)
            {
                break;
            }

            high = right;
        }
    }
}

static vector_result_t default_sort(vector_t *vector, bool ascending)
{
    if (vector->size <= 1)
        return VECTOR_SUCCESS;

    vector_quicksort_range(
        vector,
        0,
        vector->size - 1,
        ascending);

    return VECTOR_SUCCESS;
}

static vector_result_t default_print(
    const void *value,
    size_t size,
    FILE *stream)
{
    int streamPrintResult;

    if (value == NULL)
        streamPrintResult = fprintf(stream, "NULL");
    else
        streamPrintResult = fprintf(stream, "%p", value);

    if (streamPrintResult < 0)
        return VECTOR_IO_ERROR;

    return VECTOR_SUCCESS;
}

#pragma endregion

// public functions

#pragma region public functions

vector_result_t vector_create(
    const int64_t capacity,
    vector_growing_strategy_function_t const growingStrategy,
    vector_compare_function_t const compare,
    vector_sorting_function_t const sort,
    vector_print_function_t const print,
    vector_t **vectorOut)
{
    if (vectorOut == NULL || *vectorOut != NULL)
        return VECTOR_INVALID_PARAMS_ERROR;

    vector_result_t validationResult = validate_capacity(capacity);

    if (validationResult != VECTOR_SUCCESS)
        return validationResult;

    vector_t *vector = (vector_t *)malloc(sizeof(vector_t));

    if (vector == NULL)
        return VECTOR_ALLOCATION_FAILURE;

    vector->size = 0;
    vector->capacity = capacity;
    vector->items = (vector_element_t **)calloc(vector->capacity, sizeof(vector_element_t *));
    vector->compare = compare == NULL ? default_compare : compare;
    vector->sort = sort == NULL ? default_sort : sort;
    vector->print = print == NULL ? default_print : print;
    vector->growingStrategy = growingStrategy == NULL ? default_growing_strategy : growingStrategy;

    if (vector->items == NULL)
    {
        free(vector);
        return VECTOR_ALLOCATION_FAILURE;
    }

    *vectorOut = vector;
    return VECTOR_SUCCESS;
}

vector_result_t vector_free(vector_t **vector)
{
    if (vector == NULL || *vector == NULL)
        return VECTOR_NULL_ERROR;

    for (int64_t i = 0; i < (*vector)->size; i++)
    {
        vector_element_t *element = (*vector)->items[i];

        if (element == NULL)
            continue;

        free(element->value);
        element->value = NULL;
        element->size = 0;

        free(element);
        (*vector)->items[i] = NULL;
    }

    free((*vector)->items);
    (*vector)->items = NULL;

    free(*vector);
    *vector = NULL;

    return VECTOR_SUCCESS;
}

vector_result_t vector_print(vector_t *vector, FILE *stream)
{
    if (vector == NULL)
        return VECTOR_NULL_ERROR;

    int streamPrintResult;
    FILE *outputStream = stream == NULL ? stdout : stream;

    if (vector->size == 0)
    {
        streamPrintResult = fprintf(outputStream, "[ ] -> %p\n", vector);

        if (streamPrintResult < 0)
            return VECTOR_IO_ERROR;


        return VECTOR_SUCCESS;
    }

    streamPrintResult = fprintf(outputStream, "[");

    if (streamPrintResult < 0)
        return VECTOR_IO_ERROR;

    for (int64_t i = 0LL; i < vector->size; i++)
    {
        vector_element_t *element = vector->items[i];

        vector_result_t printResult = vector->print(element->value, element->size, outputStream);

        if (printResult != VECTOR_SUCCESS)
            return printResult;

        if (i < vector->size - 1) {
            streamPrintResult = fprintf(outputStream, ", ");

            if (streamPrintResult < 0)
                return VECTOR_IO_ERROR;
        }
    }

    streamPrintResult = fprintf(outputStream, "] -> %p\n", vector);

    if (streamPrintResult < 0)
        return VECTOR_IO_ERROR;

    return VECTOR_SUCCESS;
}

vector_result_t vector_resize_capacity(vector_t *vector, const int64_t capacity)
{
    if (vector == NULL)
        return VECTOR_NULL_ERROR;
    else if (capacity == vector->capacity)
        return VECTOR_SUCCESS;
    else if (capacity < vector->capacity)
        return VECTOR_INVALID_PARAMS_ERROR;

    vector_result_t result = vector_realloc_capacity(vector, capacity);

    if (result != VECTOR_SUCCESS)
        return result;

    return VECTOR_SUCCESS;
}

vector_result_t vector_prepend(vector_t *vector, const void *value, const size_t size)
{
    if (vector == NULL)
        return VECTOR_NULL_ERROR;
    else if (size == 0)
        return VECTOR_INVALID_PARAMS_ERROR;

    if (vector->size == vector->capacity)
    {
        vector_result_t growResult = vector_grow(vector);

        if (growResult != VECTOR_SUCCESS)
            return growResult;
    }

    vector_element_t *element = NULL;
    vector_result_t result = vector_element_create(value, size, &element);

    if (result != VECTOR_SUCCESS)
        return result;

    for (int64_t i = vector->size - 1; i >= 0; i--)
        vector->items[i + 1] = vector->items[i];

    vector->items[0] = element;
    vector->size++;

    return VECTOR_SUCCESS;
}

vector_result_t vector_append(vector_t *vector, const void *value, const size_t size)
{
    if (vector == NULL)
        return VECTOR_NULL_ERROR;
    else if (size == 0)
        return VECTOR_INVALID_PARAMS_ERROR;

    if (vector->size == vector->capacity)
    {
        vector_result_t growResult = vector_grow(vector);

        if (growResult != VECTOR_SUCCESS)
            return growResult;
    }

    vector_element_t *element = NULL;
    vector_result_t result = vector_element_create(value, size, &element);

    if (result != VECTOR_SUCCESS)
        return result;

    vector->items[vector->size++] = element;
    return VECTOR_SUCCESS;
}

vector_result_t vector_insert(vector_t *vector, const int64_t index, const void *value, const size_t size)
{
    if (vector == NULL)
        return VECTOR_NULL_ERROR;
    else if (index > vector->size || index < 0)
        return VECTOR_INDEX_OUT_OF_RANGE_ERROR;
    else if (size == 0)
        return VECTOR_INVALID_PARAMS_ERROR;

    if (vector->size == vector->capacity)
    {
        vector_result_t growResult = vector_grow(vector);

        if (growResult != VECTOR_SUCCESS)
            return growResult;
    }

    vector_element_t *element = NULL;
    vector_result_t createResult = vector_element_create(value, size, &element);

    if (createResult != VECTOR_SUCCESS)
        return createResult;

    for (int64_t i = vector->size - 1; i >= index; i--)
        vector->items[i + 1] = vector->items[i];

    vector->items[index] = element;
    vector->size++;

    return VECTOR_SUCCESS;
}

vector_result_t vector_get(vector_t *vector, const int64_t index, vector_element_t **elementOut)
{
    if (vector == NULL)
        return VECTOR_NULL_ERROR;
    else if (index >= vector->size || index < 0)
        return VECTOR_INDEX_OUT_OF_RANGE_ERROR;
    else if (elementOut == NULL || *elementOut != NULL)
        return VECTOR_INVALID_PARAMS_ERROR;

    vector_element_t *element = NULL;
    vector_result_t result = vector_element_clone(vector->items[index], &element);

    if (result != VECTOR_SUCCESS)
    {
        *elementOut = NULL;
        return result;
    }

    *elementOut = element;
    return VECTOR_SUCCESS;
}

vector_result_t vector_set(vector_t *vector, const int64_t index, const void *value, const size_t size)
{
    if (vector == NULL)
        return VECTOR_NULL_ERROR;
    else if (index >= vector->size || index < 0)
        return VECTOR_INDEX_OUT_OF_RANGE_ERROR;
    else if (size == 0)
        return VECTOR_INVALID_PARAMS_ERROR;

    if (vector->items[index] == NULL)
    {
        vector_element_t *element = NULL;
        vector_result_t result = vector_element_create(value, size, &element);

        if (result == VECTOR_SUCCESS)
            vector->items[index] = element;
        else
            return result;
    }
    else
    {
        vector_element_t *element = vector->items[index];
        vector_result_t result = vector_element_set(element, value, size);

        if (result == VECTOR_SUCCESS)
            vector->items[index] = element;
        else
            return result;
    }

    return VECTOR_SUCCESS;
}

vector_result_t vector_remove(vector_t *vector, const int64_t index)
{
    if (vector == NULL)
        return VECTOR_NULL_ERROR;
    else if (index < 0 || index >= vector->size)
        return VECTOR_INDEX_OUT_OF_RANGE_ERROR;

    vector_result_t freeResult = vector_element_free(&vector->items[index]);

    if (freeResult != VECTOR_SUCCESS)
        return freeResult;

    for (int64_t i = index; i < vector->size - 1; i++)
        vector->items[i] = vector->items[i + 1];

    vector->items[vector->size - 1] = NULL;
    vector->size--;
    return VECTOR_SUCCESS;
}

vector_result_t vector_pop(vector_t *const vector, const int64_t index, vector_element_t **elementOut)
{
    if (vector == NULL)
        return VECTOR_NULL_ERROR;
    else if (index < 0 || index >= vector->size)
        return VECTOR_INDEX_OUT_OF_RANGE_ERROR;
    else if (elementOut == NULL || *elementOut != NULL)
        return VECTOR_INVALID_PARAMS_ERROR;

    vector_result_t cloneResult = vector_element_clone(vector->items[index], elementOut);

    if (cloneResult != VECTOR_SUCCESS)
        return cloneResult;

    vector_result_t freeResult = vector_element_free(&vector->items[index]);

    if (freeResult != VECTOR_SUCCESS)
        return freeResult;

    for (int64_t i = index; i < vector->size - 1; i++)
        vector->items[i] = vector->items[i + 1];

    vector->items[vector->size - 1] = NULL;
    vector->size--;
    return VECTOR_SUCCESS;
}

vector_result_t vector_reverse(vector_t *const vector)
{
    if (vector == NULL)
        return VECTOR_NULL_ERROR;

    if (vector->size <= 1)
        return VECTOR_SUCCESS;

    vector_element_t **start = vector->items;
    vector_element_t **end = vector->items + vector->size - 1;
    int64_t mid = vector->size / 2LL;

    for (int64_t i = 0LL; i < mid; i++)
    {
        vector_element_t *leftElement = *start;
        vector_element_t *rightElement = *end;

        *start = rightElement;
        *end = leftElement;

        start++;
        end--;
    }

    return VECTOR_SUCCESS;
}

vector_result_t vector_clone(vector_t *const vector, vector_t **vectorOut)
{
    if (vector == NULL)
        return VECTOR_NULL_ERROR;

    if (vectorOut == NULL || *vectorOut != NULL)
        return VECTOR_INVALID_PARAMS_ERROR;

    vector_t *vectorClone = NULL;
    vector_result_t vectorCloneResult = vector_create(
        vector->capacity,
        vector->growingStrategy,
        vector->compare,
        vector->sort,
        vector->print,
        &vectorClone);

    if (vectorCloneResult != VECTOR_SUCCESS)
        return vectorCloneResult;

    for (int64_t i = 0LL; i < vector->size; i++)
    {
        vector_element_t *element = vector->items[i];
        vector_result_t insertResult = vector_insert(vectorClone, i, element->value, element->size);

        if (insertResult != VECTOR_SUCCESS)
        {
            vector_free(&vectorClone);
            return insertResult;
        }
    }

    *vectorOut = vectorClone;
    return VECTOR_SUCCESS;
}

vector_result_t vector_clear(vector_t *const vector)
{
    if (vector == NULL)
        return VECTOR_NULL_ERROR;

    for (int64_t i = vector->size - 1; i >= 0; i--)
    {
        vector_element_free(&vector->items[i]);
    }

    vector->size = 0;
    return VECTOR_SUCCESS;
}

vector_result_t vector_join(vector_t *const vector1, vector_t *const vector2, vector_t **vectorOut)
{
    if (vector1 == NULL || vector2 == NULL)
        return VECTOR_NULL_ERROR;

    if (vectorOut == NULL || *vectorOut != NULL)
        return VECTOR_INVALID_PARAMS_ERROR;

    int64_t joinedCapacity = 0;

    if (!safe_add_int64(vector1->capacity, vector2->capacity, &joinedCapacity))
        return VECTOR_INVALID_PARAMS_ERROR;

    vector_result_t validationResult = validate_capacity(joinedCapacity);

    if (validationResult != VECTOR_SUCCESS)
        return validationResult;

    vector_t *joinVector = NULL;
    vector_result_t result = vector_create(
        joinedCapacity,
        vector1->growingStrategy,
        vector1->compare,
        vector1->sort,
        vector1->print,
        &joinVector);

    if (result != VECTOR_SUCCESS)
        return result;

    vector_element_t **tempJoinVectorPointer = joinVector->items;

    for (int64_t i = 0; i < vector1->size; i++)
    {
        vector_element_t *element = NULL;
        vector_result_t cloneResult = vector_element_clone(vector1->items[i], &element);

        if (cloneResult != VECTOR_SUCCESS)
        {
            vector_free(&joinVector);

            return cloneResult;
        }

        *tempJoinVectorPointer = element;
        tempJoinVectorPointer++;
        joinVector->size++;
    }

    for (int64_t i = 0; i < vector2->size; i++)
    {
        vector_element_t *element = NULL;
        vector_result_t cloneResult = vector_element_clone(vector2->items[i], &element);

        if (cloneResult != VECTOR_SUCCESS)
        {
            vector_free(&joinVector);

            return cloneResult;
        }

        *tempJoinVectorPointer = element;
        tempJoinVectorPointer++;
        joinVector->size++;
    }

    *vectorOut = joinVector;
    return VECTOR_SUCCESS;
}

vector_result_t vector_includes(vector_t *const vector, const void *value, const size_t size)
{
    if (vector == NULL)
        return VECTOR_NULL_ERROR;
    else if (size == 0)
        return VECTOR_INVALID_PARAMS_ERROR;

    for (int64_t i = 0LL; i < vector->size; i++)
    {
        vector_element_t *element = vector->items[i];

        if (vector->compare(element->value, value, element->size, size) == 0)
            return VECTOR_SUCCESS;
    }

    return VECTOR_ELEMENT_NOT_FOUND_ERROR;
}

vector_result_t vector_find_first(
    vector_t *const vector,
    const int64_t startIndex,
    const void *value,
    const size_t size,
    int64_t *const indexOut)
{
    if (vector == NULL)
    {
        if (indexOut != NULL)
            *indexOut = -1;

        return VECTOR_NULL_ERROR;
    }
    else if (startIndex >= vector->size || startIndex < 0)
    {
        *indexOut = -1;
        return VECTOR_INDEX_OUT_OF_RANGE_ERROR;
    }
    else if (indexOut == NULL)
    {
        return VECTOR_INVALID_PARAMS_ERROR;
    }
    else if (size == 0)
    {
        *indexOut = -1;
        return VECTOR_INVALID_PARAMS_ERROR;
    }

    for (int64_t i = startIndex; i < vector->size; i++)
    {
        vector_element_t *element = vector->items[i];

        if (vector->compare(element->value, value, element->size, size) == 0)
        {
            *indexOut = i;
            return VECTOR_SUCCESS;
        }
    }

    *indexOut = -1;
    return VECTOR_ELEMENT_NOT_FOUND_ERROR;
}

vector_result_t vector_find_last(
    vector_t *const vector,
    const int64_t startIndex,
    const void *value,
    const size_t size,
    int64_t *indexOut)
{
    if (vector == NULL)
    {
        if (indexOut != NULL)
            *indexOut = -1;

        return VECTOR_NULL_ERROR;
    }
    else if (indexOut == NULL)
    {
        return VECTOR_INVALID_PARAMS_ERROR;
    }
    else if (startIndex >= vector->size || startIndex < 0)
    {
        *indexOut = -1;
        return VECTOR_INDEX_OUT_OF_RANGE_ERROR;
    }
    else if (size == 0)
    {
        *indexOut = -1;
        return VECTOR_INVALID_PARAMS_ERROR;
    }

    for (int64_t i = startIndex; i >= 0; i--)
    {
        vector_element_t *element = vector->items[i];

        if (vector->compare(element->value, value, element->size, size) == 0)
        {
            *indexOut = i;
            return VECTOR_SUCCESS;
        }
    }

    *indexOut = -1;
    return VECTOR_ELEMENT_NOT_FOUND_ERROR;
}

vector_result_t vector_sort(vector_t *const vector, const bool ascending)
{
    if (vector == NULL)
        return VECTOR_NULL_ERROR;

    return vector->sort(vector, ascending);
}

vector_result_t vector_swap(vector_t *const vector, const int64_t index1, const int64_t index2)
{
    if (vector == NULL)
        return VECTOR_NULL_ERROR;
    else if (index1 >= vector->size || index1 < 0)
        return VECTOR_INDEX_OUT_OF_RANGE_ERROR;
    else if (index2 >= vector->size || index2 < 0)
        return VECTOR_INDEX_OUT_OF_RANGE_ERROR;

    if (index1 == index2)
        return VECTOR_SUCCESS;

    vector_element_t *temp = vector->items[index1];

    vector->items[index1] = vector->items[index2];
    vector->items[index2] = temp;

    return VECTOR_SUCCESS;
}

vector_result_t vector_element_free(vector_element_t **element)
{
    if (element == NULL || *element == NULL)
        return VECTOR_INVALID_PARAMS_ERROR;

    free((*element)->value);
    (*element)->value = NULL;
    (*element)->size = 0;

    free(*element);
    *element = NULL;

    return VECTOR_SUCCESS;
}

vector_result_t vector_iterator_create(vector_t *vector, vector_iterator_t **iteratorOut)
{
    if (vector == NULL)
        return VECTOR_NULL_ERROR;

    if (iteratorOut == NULL || *iteratorOut != NULL)
        return VECTOR_INVALID_PARAMS_ERROR;

    vector_iterator_t *iterator = (vector_iterator_t *)malloc(sizeof(vector_iterator_t));

    if (iterator == NULL)
        return VECTOR_ALLOCATION_FAILURE;

    iterator->vector = vector;
    iterator->index = -1;
    iterator->element = NULL;

    *iteratorOut = iterator;
    return VECTOR_SUCCESS;
}

vector_result_t vector_iterator_next(vector_iterator_t *iterator)
{
    if (iterator == NULL)
        return VECTOR_INVALID_PARAMS_ERROR;
    else if (iterator->vector->size == 0) {
        iterator->element = NULL;
        return VECTOR_INDEX_OUT_OF_RANGE_ERROR;
    }
    else if (iterator->index >= iterator->vector->size - 1)
    {
        iterator->element = NULL;
        return VECTOR_INDEX_OUT_OF_RANGE_ERROR;
    }

    iterator->index++;
    iterator->element = iterator->vector->items[iterator->index];

    return VECTOR_SUCCESS;
}

vector_result_t vector_iterator_rewind(vector_iterator_t *const iterator)
{
    if (iterator == NULL)
        return VECTOR_INVALID_PARAMS_ERROR;

    iterator->index = -1;
    iterator->element = NULL;

    return VECTOR_SUCCESS;
}

vector_result_t vector_iterator_free(vector_iterator_t **iterator)
{
    if (iterator == NULL || *iterator == NULL)
        return VECTOR_INVALID_PARAMS_ERROR;

    (*iterator)->vector = NULL;
    (*iterator)->index = 0;
    (*iterator)->element = NULL;

    free((*iterator));
    *iterator = NULL;
    return VECTOR_SUCCESS;
}

#pragma endregion
