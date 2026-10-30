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
 * @brief Creates a new vector element and initializes it with a value.
 *
 * This function allocates memory for a new vector element and
 * initializes its value with a copy of the provided data. If the value
 * is NULL, the element's value is set to NULL, and its size is set to
 * the specified value size.
 *
 * @param value A pointer to the value to be assigned to the new element.
 * @param size The size of value.
 * @param elementOut A double pointer to where the element will be stored.
 *
 * @return
 * - `VECTOR_SUCCESS` if the element was created successfully.
 *
 * - `VECTOR_ALLOCATION_FAILURE` if memory allocation fails during the process.
 */
static vector_result_t vector_element_create(const void *const value, const size_t size, vector_element_t **elementOut);

/**
 * @brief Sets the value of an existing vector element.
 *
 * This function updates the value of a specified vector element
 * with a new value. If the size of the new value differs from the current
 * size of the element, it reallocates memory for the element's value.
 *
 * @param element A pointer to the vector element whose value is to be set.
 * @param value A pointer to the new value to be assigned to the element.
 * @param size The size of value.
 *
 * @return
 * - `VECTOR_SUCCESS` if the value was set successfully.
 *
 * - `VECTOR_ALLOCATION_FAILURE` if memory allocation fails during the process.
 */
static vector_result_t vector_element_set(vector_element_t *const element, const void *const value, const size_t size);

/**
 * @brief Clones an existing vector element.
 *
 * This function creates a new vector element that is a copy of
 * the specified existing element. It allocates memory for the new element
 * and copies the value from the original element.
 *
 * @param element A pointer to the vector element to be cloned.
 * @param elementOut A double pointer to where the cloned element will be stored.
 *
 * @return
 * - `VECTOR_SUCCESS` if the element was cloned successfully.
 *
 * - `VECTOR_ALLOCATION_FAILURE` if memory allocation fails during the process.
 */
static vector_result_t vector_element_clone(vector_element_t *const element, vector_element_t **elementOut);

/**
 * @brief Reallocates the capacity of the vector to a new size.
 *
 * This function adjusts the capacity of the specified vector to
 * the new specified capacity. If the new capacity is less than the current
 * capacity, it frees any excess elements and updates the size of the vector
 * if necessary. If the new capacity is greater, it reallocates memory for
 * the items array. If the new capacity is equal to the current capacity,
 * it returns success without making any changes.
 *
 * @param vector A pointer to the vector whose capacity is to be reallocated.
 * @param capacity The new capacity for the vector.
 *
 * @return
 * - `VECTOR_SUCCESS` if the capacity was reallocated successfully.
 *
 * - `VECTOR_INVALID_PARAMS_ERROR` if the new capacity is less than 1.
 *
 * - `VECTOR_ALLOCATION_FAILURE` if memory allocation fails during the process.
 */
static vector_result_t vector_realloc_capacity(vector_t *const vector, const int64_t capacity);

static vector_result_t vector_grow(vector_t* vector);

/**
 * @brief Compares two data elements for compare.
 *
 * This function checks if two data elements are equal by comparing their values. It handles NULL pointers
 * and uses `memcmp` for the actual comparison of non-NULL data.
 *
 * @param dataOne A pointer to the first data element to compare.
 * @param dataTwo A pointer to the second data element to compare.
 * @param size The size of the data elements to be compared.
 *
 * @return
 * - A negative value if the first data element is NULL and the second is not.
 *
 * - A positive value if the first data element is not NULL and the second is.
 *
 * - `0` if both data elements are NULL or equal.
 *
 * - The result of `memcmp` if both data elements are non-NULL.
 *
 * @note This function assumes that the size parameter is valid and corresponds to the size of the data elements.
 */
static int32_t default_compare(
    const void *const dataOne,
    const void *const dataTwo,
    const size_t dataOneSize,
    const size_t dataTwoSize
);

/**
 * @brief Sorts the elements of the vector using a specified sorting order.
 *
 * This function sorts the elements of the specified vector using
 * the `qsort_s` function. The sorting order is determined by the `ascending`
 * parameter, which specifies whether to sort in ascending or descending order.
 * The function uses the provided custom compare function to compare elements
 * during sorting.
 *
 * @param vector A pointer to the vector whose elements are to be sorted.
 * @param ascending A boolean value indicating the sort order; true for
 *                  ascending order and false for descending order.
 */
static vector_result_t default_sort(vector_t *const vector, const bool ascending);

#pragma endregion

// private functions

#pragma region private functions

static bool safe_add_int64(int64_t numberOne, int64_t numberTwo, int64_t* destination)
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

    if (element->value == NULL) {
        if (value == NULL) {
            element->value = NULL;
            element->size = size;
            return VECTOR_SUCCESS;
        }
    }

    if (element->value != NULL) {
        if (value == NULL) {
            free(element->value);

            element->value = NULL;
            element->size = size;
            return VECTOR_SUCCESS;
        }
    }

    void *valueCopy = NULL;

    if (element->value == NULL)
        valueCopy = (void *)malloc(size);
    else
        valueCopy = (void *)realloc(element->value, size);

    if (valueCopy == NULL)
        return VECTOR_ALLOCATION_FAILURE;

    element->value = valueCopy;
    element->size = size;

    memcpy(element->value, value, size);

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

    newItems = (vector_element_t**) malloc(newCapacity * sizeof(vector_element_t*));

    if (newItems == NULL)
        return VECTOR_ALLOCATION_FAILURE;

    memcpy(
        newItems,
        vector->items,
        vector->size * sizeof(vector_element_t*)
    );

    for (int64_t i = vector->size; i < newCapacity; ++i)
        newItems[i] = NULL;

    free(vector->items);
    vector->items = newItems;
    vector->capacity = newCapacity;

    return VECTOR_SUCCESS;
}

static vector_result_t vector_grow(vector_t* vector) {
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
    const size_t dataTwoSize
) {
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

static vector_result_t default_growing_strategy(int64_t const currentCapacity, int64_t* const newCapacityOut)
{
    if (newCapacityOut == NULL || *newCapacityOut != NULL)
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
    bool ascending
) {
    while (low < high) {
        int64_t left = low;
        int64_t right = high;
        int64_t pivot_index = low + (high - low) / 2;

        vector_element_t *pivot = vector->items[pivot_index];

        while (left <= right) {
            int comparison;

            while (left <= high) {
                vector_element_t* left_element = vector->items[left];

                comparison = vector->compare(
                    left_element->value,
                    pivot->value,
                    left_element->size,
                    pivot->size
                );

                if (!ascending) {
                    comparison = comparison < 0 ? 1
                        : comparison > 0 ? -1
                        : 0;
                }

                if (comparison >= 0) {
                    break;
                }

                ++left;
            }

            while (right >= low) {
                vector_element_t* rightElement = vector->items[right];

                comparison = vector->compare(
                    rightElement->value,
                    pivot->value,
                    rightElement->size,
                    pivot->size
                );

                if (!ascending) {
                    comparison = comparison < 0 ? 1
                        : comparison > 0 ? -1
                        : 0;
                }

                if (comparison <= 0) {
                    break;
                }

                /*
                 * Avoid unsigned underflow.
                 */
                if (right == 0) {
                    break;
                }

                --right;
            }

            if (left > right) {
                break;
            }

            if (left != right) {
                vector_swap(
                    vector,
                    left,
                    right
                );
            }

            ++left;

            if (right == 0) {
                break;
            }

            --right;
        }

        /*
         * Recurse into the smaller partition and iterate over
         * the larger one to limit stack usage.
         */
        if (right >= low && right - low < high - left + 1) {
            if (low < right) {
                vector_quicksort_range(
                    vector, low, right, ascending
                );
            }

            low = left;
        } else {
            if (left < high) {
                vector_quicksort_range(
                    vector, left, high, ascending
                );
            }

            if (right < low || right == 0) {
                break;
            }

            high = right;
        }
    }
}

static vector_result_t default_sort(vector_t *vector, bool ascending) {
    if (vector->size <= 1)
        return VECTOR_SUCCESS;

    vector_quicksort_range(
        vector,
        0,
        vector->size - 1,
        ascending
    );

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
        return VECTOR_INVALID_PARAMS_ERROR;

    return VECTOR_SUCCESS;
}

static vector_result_t validate_capacity(int64_t capacity)
{
    if (capacity <= 0)
        return VECTOR_INVALID_PARAMS_ERROR;

    if ((uintmax_t)capacity > (uintmax_t)(SIZE_MAX / sizeof(vector_element_t*)))
        return VECTOR_SIZE_LIMIT_REACHED;

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
    vector_t** vectorOut
) {
    if (vectorOut == NULL || *vectorOut != NULL)
        return VECTOR_INVALID_PARAMS_ERROR;

    vector_result_t validationResult = validate_capacity(capacity);

    if (validationResult != VECTOR_SUCCESS)
        return validationResult;

    vector_t *vector = (vector_t *)malloc(sizeof(vector_t));

    if (vector == NULL)
        return VECTOR_ALLOCATION_FAILURE;

    vector->size = 0;
    vector->capacity = capacity < 1 ? DEFAULT_VECTOR_CAPACITY : capacity;
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

    FILE *outputStream = stream == NULL ? stdout : stream;

    if (vector->size == 0) {
        fprintf(outputStream, "[ ] -> %p\n", vector);
        return VECTOR_SUCCESS;
    }

    fprintf(outputStream, "[");

    for (int64_t i = 0LL; i < vector->size; i++)
    {
        vector_element_t *element = vector->items[i];

        vector_result_t printResult = vector->print(element->value, element->size, outputStream);

        if (printResult != VECTOR_SUCCESS)
            return printResult;

        if (i < vector->size - 1)
            fprintf(outputStream, ", ");
    }

    fprintf(outputStream, "] -> %p\n", vector);

    return VECTOR_SUCCESS;
}

vector_result_t vector_resize(vector_t *vector, const int64_t capacity)
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

vector_result_t vector_prepend(vector_t *vector, void *value, const size_t size)
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

vector_result_t vector_append(vector_t *vector, void *const value, const size_t size)
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

vector_result_t vector_insert(vector_t *vector, const int64_t index, void *const value, const size_t size)
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

vector_result_t vector_set(vector_t *vector, const int64_t index, void *const value, const size_t size)
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
        vector_result_t result = vector_element_set(vector->items[index], value, size);

        if (result != VECTOR_SUCCESS)
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
        &vectorClone
    );

    if (vectorCloneResult != VECTOR_SUCCESS)
        return vectorCloneResult;

    for (int64_t i = 0LL; i < vector->size; i++)
    {
        vector_element_t* element = vector->items[i];
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

    for (int64_t i = vector->size-1; i >= 0; i--)
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

vector_result_t vector_includes(vector_t *const vector, void *value, const size_t size)
{
    if (vector == NULL)
        return VECTOR_NULL_ERROR;
    else if (size == 0)
        return VECTOR_INVALID_PARAMS_ERROR;

    for (int64_t i = 0LL; i < vector->size; i++)
    {
        vector_element_t *element = vector->items[i];

        if (element->size != size)
            continue;

        if (vector->compare(element->value, value, element->size, size) == 0)
            return VECTOR_SUCCESS;
    }

    return VECTOR_ELEMENT_NOT_FOUND_ERROR;
}

vector_result_t vector_find_first(
    vector_t *const vector,
    const int64_t startIndex,
    void *const value,
    const size_t size,
    int64_t *const indexOut)
{
    if (indexOut == NULL)
    {
        return VECTOR_INVALID_PARAMS_ERROR;
    }
    else if (vector == NULL)
    {
        *indexOut = -1;
        return VECTOR_NULL_ERROR;
    }
    else if (startIndex > vector->size || startIndex < 0)
    {
        *indexOut = -1;
        return VECTOR_INVALID_PARAMS_ERROR;
    }
    else if (size == 0) {
        *indexOut = -1;
        return VECTOR_INVALID_PARAMS_ERROR;
    }

    int64_t index = startIndex > 0LL ? startIndex : 0LL;

    for (int64_t i = index; i < vector->size; i++)
    {
        vector_element_t *element = vector->items[i];

        if (element->size != size)
            continue;

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
    void *value,
    const size_t size,
    int64_t *indexOut
) {
    if (indexOut == NULL)
    {
        return VECTOR_INVALID_PARAMS_ERROR;
    }
    else if (vector == NULL)
    {
        *indexOut = -1;
        return VECTOR_NULL_ERROR;
    }
    else if (startIndex > vector->size || startIndex < 0)
    {
        *indexOut = -1;
        return VECTOR_INVALID_PARAMS_ERROR;
    }
    else if (size == 0) {
        *indexOut = -1;
        return VECTOR_INVALID_PARAMS_ERROR;
    }
        
    int64_t index = -1;

    for (int64_t i = startIndex; i < vector->size; i++)
    {
        vector_element_t *element = vector->items[i];

        if (element->size != size)
            continue;

        if (vector->compare(element->value, value, element->size, size) == 0)
            index = i;
    }

    *indexOut = index;
    return index != -1 ? VECTOR_SUCCESS : VECTOR_ELEMENT_NOT_FOUND_ERROR;
}

vector_result_t vector_sort(vector_t *const vector, const bool ascending)
{
    if (vector == NULL)
        return VECTOR_NULL_ERROR;

    return vector->sort(vector, ascending);
}

vector_result_t vector_fill(vector_t *const vector, void *const value, const size_t size)
{
    if (vector == NULL)
        return VECTOR_NULL_ERROR;
    else if (vector->size == vector->capacity)
        return VECTOR_SUCCESS;
    else if (size == 0)
        return VECTOR_INVALID_PARAMS_ERROR;

    int64_t elementsToCreate = vector->capacity - vector->size;

    if (elementsToCreate > INT64_MAX / sizeof(vector_element_t*))
        return VECTOR_SIZE_LIMIT_REACHED;

    vector_element_t** temp = (vector_element_t** ) malloc(elementsToCreate * sizeof(vector_element_t*));

    if (temp == NULL)
        return VECTOR_ALLOCATION_FAILURE;

    for (int64_t i = 0; i < elementsToCreate; i++)
    {
        vector_element_t *element = NULL;
        vector_result_t result = vector_element_create(value, size, &element);

        if (result != VECTOR_SUCCESS) {
            for (int64_t j = 0; j < i; j++){
                vector_element_free(&temp[j]);
            }

            free(temp);
            return result;
        }

        temp[i] = element;
    }

    for (int64_t i = 0; i < elementsToCreate; i++) {
        vector->items[vector->size + i] = temp[i];
    }

    vector->size = vector->capacity;
    free(temp);
    return VECTOR_SUCCESS;
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

vector_result_t vector_element_free(vector_element_t **element) {
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
    if (iterator == NULL) {
        return VECTOR_INVALID_PARAMS_ERROR;
    }
    else if (iterator->vector->size == 0) {
        iterator->element = NULL;
        return VECTOR_INDEX_OUT_OF_RANGE_ERROR;
    }
    else if (iterator->index + 1 >= iterator->vector->size)
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
