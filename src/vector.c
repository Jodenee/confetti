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
static vector_result_t vector_element_create(const void* const value, const uint64_t size, vector_element_t** elementOut);

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
static vector_result_t vector_element_set(vector_element_t* const element, const void* const value, const uint64_t size);

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
static vector_result_t vector_element_clone(vector_element_t* const element, vector_element_t** elementOut);

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
static vector_result_t vector_realloc_capacity(vector_t* const vector, const int64_t capacity);

/**
 * @brief Compares two data elements for equality.
 *
 * This function checks if two data elements are equal by comparing their values. It handles NULL pointers
 * and uses `memcmp` for the actual comparison of non-NULL data. 
 *
 * @param data1 A pointer to the first data element to compare.
 * @param data2 A pointer to the second data element to compare.
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
static int32_t default_equals(const void* const data1, const void* const data2, const uint64_t size);

/**
 * @brief Partitions the vector for the Median of Three Pivot Quicksort algorithm and returns the pivot.
 *
 * This function partitions the specified portion of the vector around a pivot element. 
 * It rearranges the elements in such a way that all elements less than or equal to the pivot 
 * are on one side, and all elements greater than the pivot are on the other side. 
 * The pivot is chosen based on the specified order.
 *
 * @param vector A pointer to the vector to be partitioned.
 * @param low The starting index of the portion of the vector to be partitioned.
 * @param high The ending index of the portion of the vector to be partitioned.
 * @param ascending A boolean value indicating the sort order.
 * 
 * @return The index of the pivot element after partitioning.
 * 
 * @note This function assumes that the portion of the vector to be partitioned contains at least one element.
 */
static int64_t partition(vector_t* const vector, const int64_t low, const int64_t high, const bool ascending);

/**
 * @brief Sorts the elements of the vector using Median of Three Pivot Quicksort.
 * 
 * This function sorts the elements of the specified vector using
 * the `qsort_s` function. The sorting order is determined by the `ascending`
 * parameter, which specifies whether to sort in ascending or descending order.
 * The function uses the provided custom equality function to compare elements
 * during sorting.
 *
 * @param vector A pointer to the vector whose elements are to be sorted.
 * @param low The starting index of the portion of the vector to be sorted.
 * @param high The ending index of the portion of the vector to be sorted.
 * @param ascending A boolean value indicating the sort order.
 * 
 * @note The vector is sorted in place.
 */
void quicksort(vector_t* const vector, const int64_t low, const int64_t high, const bool ascending);

/**
 * @brief Sorts the elements of the vector using a specified sorting order.
 * 
 * This function sorts the elements of the specified vector using
 * the `qsort_s` function. The sorting order is determined by the `ascending`
 * parameter, which specifies whether to sort in ascending or descending order.
 * The function uses the provided custom equality function to compare elements
 * during sorting.
 *
 * @param vector A pointer to the vector whose elements are to be sorted.
 * @param ascending A boolean value indicating the sort order; true for
 *                  ascending order and false for descending order.
 */
static vector_result_t default_sort(vector_t* const vector, const bool ascending);

#pragma endregion

// private functions

#pragma region private functions

static vector_result_t vector_element_create(const void* const value, const uint64_t size, vector_element_t** elementOut) {
    vector_element_t* element = (vector_element_t*) malloc(sizeof(vector_element_t));

    if (element == NULL)
        return VECTOR_ALLOCATION_FAILURE;

    element->value = NULL;
    element->size = size;

    if (value != NULL) {
        element->value = (void*) malloc(size);

        if (element->value == NULL) {
            free(element);
            element = NULL;

            return VECTOR_ALLOCATION_FAILURE;
        }

        memcpy(element->value, value, size);
    }

    *elementOut = element;
    return VECTOR_SUCCESS;
}


static vector_result_t vector_element_set(vector_element_t* const element, const void* value, const uint64_t size) {
    if (size != element->size) {
        void* valueCopy = NULL;

        if (element->value == NULL)
            valueCopy = (void*)malloc(size);
        else
            valueCopy = (void*)realloc(element->value, size);

        if (valueCopy == NULL)
            return VECTOR_ALLOCATION_FAILURE;
        
        element->value = valueCopy;
        element->size = size;
    }

    if (value == NULL || size == 0)
        element->value = NULL;
    else
        memcpy(element->value, value, size);

    return VECTOR_SUCCESS;
}


static vector_result_t vector_element_clone(vector_element_t* const element, vector_element_t** elementOut) {
    vector_element_t* elementClone = (vector_element_t*)malloc(sizeof(vector_element_t));

    if (elementClone == NULL)
        return VECTOR_ALLOCATION_FAILURE;

    elementClone->value = NULL;
    elementClone->size = element->size;

    if (element->value != NULL) {
        elementClone->value = (void*)malloc(element->size);

        if (elementClone->value == NULL) {
            free(elementClone);
            return VECTOR_ALLOCATION_FAILURE;
        }

        memcpy(elementClone->value, element->value, element->size);
    }

    *elementOut = elementClone;
    return VECTOR_SUCCESS;
}


static vector_result_t vector_realloc_capacity(vector_t* const vector, const int64_t capacity) {
    if (capacity < 1)
        return VECTOR_INVALID_PARAMS_ERROR;

    int64_t oldCapacity = vector->capacity;

    if (capacity == oldCapacity)
        return VECTOR_SUCCESS;

    if (capacity < oldCapacity) {
        for (int64_t i = oldCapacity - 1; i > capacity - 1; i--) {
            if (vector->items[i] != NULL) {
                vector_element_t* element = vector->items[i];

                vector_element_free(&element);
            }

            vector->items[i] = NULL;
        }

        vector_element_t** reallocatedItems = (vector_element_t**) realloc(vector->items, sizeof(vector_element_t*) * capacity);

        if (reallocatedItems == NULL)
            return VECTOR_ALLOCATION_FAILURE;

        vector->capacity = capacity;
        vector->items = reallocatedItems;

        if (vector->size > capacity)
            vector->size = capacity;
    }
    else {
        vector_element_t** reallocatedItems = (vector_element_t**) realloc(vector->items, sizeof(vector_element_t*) * capacity);

        if (reallocatedItems == NULL)
            return VECTOR_ALLOCATION_FAILURE;

        vector->capacity = capacity;
        vector->items = reallocatedItems;

        for (int64_t i = oldCapacity; i < capacity; i++) {
            vector->items[i] = NULL;
        }
    }

    return VECTOR_SUCCESS;
}


static int32_t default_equals(const void* const data1, const void* const data2, const uint64_t size)
{
    if (data1 == NULL && data2 != NULL)
        return -1;
    else if (data1 != NULL && data2 == NULL) 
        return 1;
    else if (data1 == NULL && data2 == NULL)
        return 0;

    return memcmp(data1, data2, size);
}


static int64_t partition(vector_t* const vector, const int64_t low, const int64_t high, const bool ascending) {
    if (high - low + 1 >= 3) {
        int64_t mid = low + (high - low) / 2;

        if (ascending) {
            if (vector->equalityFunction(vector->items[low]->value, vector->items[mid]->value, vector->items[low]->size) > 0) {
                vector_swap(vector, low, mid);
            }

            if (vector->equalityFunction(vector->items[low]->value, vector->items[high]->value, vector->items[low]->size) > 0) {
                vector_swap(vector, low, high);
            }

            if (vector->equalityFunction(vector->items[mid]->value, vector->items[high]->value, vector->items[mid]->size) > 0) {
                vector_swap(vector, mid, high);
            }
        }
        else {
            if (vector->equalityFunction(vector->items[low]->value, vector->items[mid]->value, vector->items[low]->size) < 0) {
                vector_swap(vector, low, mid);
            }

            if (vector->equalityFunction(vector->items[low]->value, vector->items[high]->value, vector->items[low]->size) < 0) {
                vector_swap(vector, low, high);
            }

            if (vector->equalityFunction(vector->items[mid]->value, vector->items[high]->value, vector->items[mid]->size) < 0) {
                vector_swap(vector, mid, high);
            }
        }

        vector_swap(vector, mid, high - 1);
    }

    vector_element_t* pivot_element = vector->items[high - 1];
    void* pivot_value = pivot_element->value;
    uint64_t pivot_size = pivot_element->size;

    int64_t i = (low - 1);

    for (int64_t j = low; j <= high - 2; j++) {
        int32_t comparison_result = vector->equalityFunction(vector->items[j]->value, pivot_value, pivot_size);

        bool condition = ascending 
            ? (comparison_result <= 0) 
            : (comparison_result >= 0);

        if (condition) {
            i++;
            vector_swap(vector, i, j);
        }
    }

    vector_swap(vector, i + 1, high - 1);
    return (i + 1);
}


void quicksort(vector_t* const vector, const int64_t low, const int64_t high, const bool ascending) {
    if (low < high) {
        int32_t p = partition(vector, low, high, ascending);

        quicksort(vector, low, p - 1, ascending);
        quicksort(vector, p + 1, high, ascending);
    }
}


static vector_result_t default_sort(vector_t* const vector, const bool ascending)
{
    quicksort(vector, 0, vector->size - 1, ascending);

    return VECTOR_SUCCESS;
}

#pragma endregion

// public functions

#pragma region public functions

vector_result_t vector_create(
    vector_t** vectorOut,
    const int64_t capacity,
    vector_custom_equality_function_t* const customEqualityFunction,
    vector_custom_sorting_function_t* const customSortingFunction
) {
    vector_t* vector = (vector_t*) malloc(sizeof(vector_t));

    if (vector == NULL)
        return VECTOR_ALLOCATION_FAILURE;

    vector->size = 0;
    vector->capacity = capacity < 1 ? DEFAULT_VECTOR_CAPACITY : capacity;
    vector->items = (vector_element_t**) calloc(vector->capacity, sizeof(vector_element_t*));
    vector->equalityFunction = customEqualityFunction == NULL ? (vector_custom_equality_function_t*) &default_equals : customEqualityFunction;
    vector->sortingFunction = customSortingFunction == NULL ? (vector_custom_sorting_function_t*) &default_sort : customSortingFunction;

    if (vector->items == NULL) {
        free(vector);
        return VECTOR_ALLOCATION_FAILURE;
    }

    *vectorOut = vector;
    return VECTOR_SUCCESS;
}


vector_result_t vector_free(vector_t** vector) {
    if (*vector == NULL)
        return VECTOR_NULL_ERROR;

    for (int64_t i = 0; i < (*vector)->size; i++) {
        vector_element_t* element = (*vector)->items[i];

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


vector_result_t vector_print(vector_t* vector) {
    if (vector == NULL)
        return VECTOR_NULL_ERROR;

    printf("[");

    for (int64_t i = 0LL; i < vector->size; i++) {
        vector_element_t* element = vector->items[i];

        if (element->value != NULL)
            printf(i == vector->size - 1 ? "%p" : "%p, ", element->value);
        else
            printf(i == vector->size - 1 ? "NULL" : "NULL, ", element->value);
    }

    printf("] -> %p\n", vector);

    return VECTOR_SUCCESS;
}


vector_result_t vector_resize(vector_t* vector, const int64_t capacity) {
    if (vector == NULL)
        return VECTOR_NULL_ERROR;
    else if (capacity < 1)
        return VECTOR_INVALID_PARAMS_ERROR;
    else if (capacity == vector->capacity)
        return VECTOR_SUCCESS;

    vector_result_t result;
    result = vector_realloc_capacity(vector, capacity);

    if (result != VECTOR_SUCCESS) 
        return result;

    return VECTOR_SUCCESS;
}


vector_result_t vector_prepend(vector_t* vector, void* value, const uint64_t size) {
    if (vector == NULL)
        return VECTOR_NULL_ERROR;

    if (vector->size == vector->capacity) {
        vector_result_t result = vector_realloc_capacity(vector, vector->capacity * 2);

        if (result != VECTOR_SUCCESS) 
            return result;
    }
    
    vector_element_t* element = NULL;
    vector_result_t result = vector_element_create(value, size, &element);

    if (result != VECTOR_SUCCESS)
        return result;

    for (int64_t i = vector->size - 1; i >= 0; i--)
        vector->items[i + 1] = vector->items[i];

    vector->items[0] = element;
    vector->size++;

    return VECTOR_SUCCESS;
}


vector_result_t vector_append(vector_t* vector, void* const value, const uint64_t size) {
    if (vector == NULL)
        return VECTOR_NULL_ERROR;

    if (vector->size == vector->capacity) {
        vector_result_t result = vector_realloc_capacity(vector, vector->capacity * 2);

        if (result != VECTOR_SUCCESS) 
            return result;
    }

    vector_element_t* element;
    vector_result_t result = vector_element_create(value, size, &element);

    if (result != VECTOR_SUCCESS)
        return result;

    vector->items[vector->size++] = element;
    return VECTOR_SUCCESS;
}


vector_result_t vector_insert(vector_t* vector, const int64_t index, void* const value, const uint64_t size) {
    if (vector == NULL)
        return VECTOR_NULL_ERROR;
    else if (index > vector->size || index < 0)
        return VECTOR_INDEX_OUT_OF_RANGE_ERROR;

    if (vector->size == vector->capacity) {
        vector_result_t result = vector_realloc_capacity(vector, vector->capacity * 2);

        if (result != VECTOR_SUCCESS) return result;
    }

    vector_element_t* element = NULL;
    vector_result_t createResult = vector_element_create(value, size, &element);

    if (createResult != VECTOR_SUCCESS)
        return createResult;

    for (int i = vector->size - 1; i >= index; i--)
        vector->items[i + 1] = vector->items[i];
    
    vector->items[index] = element;
    vector->size++;

    return VECTOR_SUCCESS;
}


vector_result_t vector_get(vector_t* vector,  vector_element_t** elementOut, const int64_t index) {
    if (vector == NULL)
        return VECTOR_NULL_ERROR;
    else if (index >= vector->size || index < 0)
        return VECTOR_INDEX_OUT_OF_RANGE_ERROR;

    vector_element_t* element;
    vector_result_t result = vector_element_clone(vector->items[index], &element);

    if (result != VECTOR_SUCCESS) {
        *elementOut = NULL;
        return result;
    }

    *elementOut = element;
    return VECTOR_SUCCESS;
}


vector_result_t vector_set(vector_t* vector, const int64_t index, void* const value, const uint64_t size) {
    if (vector == NULL)
        return VECTOR_NULL_ERROR;
    else if (index >= vector->size || index < 0)
        return VECTOR_INDEX_OUT_OF_RANGE_ERROR;
    else if (size == 0)
        return VECTOR_INVALID_PARAMS_ERROR;

    if (vector->items[index] == NULL) {
        vector_element_t* element;
        vector_result_t result = vector_element_create(value, size, &element);

        if (result == VECTOR_SUCCESS)
            vector->items[index] = element;
        else
            return result;
    }
    else {
        vector_result_t result = vector_element_set(vector->items[index], value, size);

        if (result != VECTOR_SUCCESS)
            return result;
    }

    return VECTOR_SUCCESS;
}


vector_result_t vector_remove(vector_t* vector, const int64_t index) {
    if (vector == NULL)
        return VECTOR_NULL_ERROR;
    else if (index < 0 || index >= vector->size)
        return VECTOR_INDEX_OUT_OF_RANGE_ERROR;

    vector_result_t freeResult = vector_element_free(&vector->items[index]);

    if (freeResult != VECTOR_SUCCESS)
        return freeResult;

    for (int64_t i = index; i < vector->size - 1; i++)
        vector->items[i] = vector->items[i + 1];

    vector->size--;
    return VECTOR_SUCCESS;
}


vector_result_t vector_pop(vector_t* const vector, vector_element_t** elementOut, const int64_t index) {
    if (vector == NULL)
        return VECTOR_NULL_ERROR;
    else if (index < 0 || index >= vector->size)
        return VECTOR_INDEX_OUT_OF_RANGE_ERROR;

    vector_result_t cloneResult = vector_element_clone(vector->items[index], elementOut);
    
    if (cloneResult != VECTOR_SUCCESS)
        return cloneResult;
    
    vector_result_t freeResult = vector_element_free(&vector->items[index]);

    if (freeResult != VECTOR_SUCCESS)
        return freeResult;

    for (int64_t i = index; i < vector->size - 1; i++)
        vector->items[i] = vector->items[i + 1];

    vector->size--;
    return VECTOR_SUCCESS;
}


vector_result_t vector_reverse(vector_t* const vector) {
    if (vector == NULL)
        return VECTOR_NULL_ERROR;

    vector_element_t** start = vector->items;
    vector_element_t** end = vector->items + vector->size - 1;
    int64_t mid = vector->size / 2LL;

    for (int64_t i = 0LL; i < mid; i++) {
        vector_element_t* leftElement = *start;
        vector_element_t* rightElement = *end;

        *start = rightElement;
        *end = leftElement;

        start++;
        end--;
    }

    return VECTOR_SUCCESS;
}


vector_result_t vector_clone(vector_t* const vector, vector_t** vectorOut) {
    if (vector == NULL)
        return VECTOR_NULL_ERROR;

    if (*vectorOut == NULL) 
        return VECTOR_INVALID_PARAMS_ERROR;

    vector_t* vectorClone = (vector_t*) malloc(sizeof(vector_t));

    if (vectorClone == NULL)
        return VECTOR_ALLOCATION_FAILURE;

    vectorClone->size = vector->size;
    vectorClone->capacity = vector->capacity;
    vectorClone->items = (vector_element_t**) calloc(vectorClone->capacity, sizeof(vector_element_t*));

    if (vectorClone->items == NULL)
        free(vectorClone);
        return VECTOR_ALLOCATION_FAILURE;

    for (int64_t i = 0LL; i < vector->size; i++) {
        vector_element_t* elementClone;
        vector_result_t result = vector_element_clone(vector->items[i], &elementClone);

        if (result != VECTOR_SUCCESS) {
            for (int64_t i = 0LL; i < i; i++) {
                vector_element_free(&vector->items[i]);
            }

            vector_free(&vectorClone);
            return result;
        }

        vectorClone->items[i] = elementClone;
    }
        
    *vectorOut = vectorClone;
    return VECTOR_SUCCESS;
}


vector_result_t vector_clear(vector_t* const vector) {
    if (vector == NULL)
        return VECTOR_NULL_ERROR;

    for (int64_t i = 0; i < vector->size; i++) {
        vector_element_free(&vector->items[i]);
    }

    vector->size = 0;
    return VECTOR_SUCCESS;
}


vector_result_t vector_join(vector_t* const vector1, vector_t* const vector2, vector_t** vectorOut) {
    if (vector1 == NULL)
        return VECTOR_NULL_ERROR;
    else if (vector2 == NULL)
        return VECTOR_NULL_ERROR;

    vector_t* joinList;
    vector_result_t result = vector_create(&joinList, (vector1->size + vector2->size), NULL, NULL);

    if (result != VECTOR_SUCCESS)
        return result;

    vector_element_t** tempJoinListPointer = joinList->items;

    for (int64_t i = 0; i < vector1->size; i++) {
        vector_element_t* element;
        vector_element_clone(vector1->items[i], &element);

        *tempJoinListPointer = element;
        tempJoinListPointer++;
    }

    for (int64_t i = 0; i < vector2->size; i++) {
        vector_element_t* element;
        vector_element_clone(vector2->items[i], &element);

        *tempJoinListPointer = element;
        tempJoinListPointer++;
    }

    joinList->size = joinList->capacity;
    *vectorOut = joinList;
    return VECTOR_SUCCESS;
}


vector_result_t vector_includes(vector_t* const vector, void* value, const uint64_t size) {
    if (vector == NULL)
        return VECTOR_NULL_ERROR;

    for (int64_t i = 0LL; i < vector->size; i++)
    {
        vector_element_t* element = vector->items[i];

        if (element->size != size)
            continue;

        if (vector->equalityFunction(element->value, value, element->size) == 0)
            return VECTOR_SUCCESS;
    }

    return VECTOR_ELEMENT_NOT_FOUND_ERROR;
}


vector_result_t vector_find_first(
    vector_t* const vector, 
    int64_t* const indexOut, 
    const int64_t startIndex, 
    void* const value, 
    const uint64_t size
) {
    if (vector == NULL) {
        *indexOut = -1;
        return VECTOR_NULL_ERROR;
    }
    else if (startIndex >= vector->size || startIndex < 0) {
        *indexOut = -1;
        return VECTOR_INVALID_PARAMS_ERROR;
    }
    else if (size <= 0) {
        *indexOut = -1;
        return VECTOR_INVALID_PARAMS_ERROR;
    }

    int64_t index = startIndex > 0LL ? startIndex : 0LL;

    for (int64_t i = index; i < vector->size; i++) {
        vector_element_t* element = vector->items[i];

        if (element->size != size)
            continue;

        if (vector->equalityFunction(element->value, value, element->size) == 0) {
            *indexOut = i;
            return VECTOR_SUCCESS;
        }
    }

    *indexOut = -1;
    return VECTOR_ELEMENT_NOT_FOUND_ERROR;
}


vector_result_t vector_find_last(vector_t* const vector, int64_t* indexOut, const int64_t startIndex, void* value, const uint64_t size) {
    if (vector == NULL) {
        *indexOut = -1;
        return VECTOR_NULL_ERROR;
    }
    else if (startIndex >= vector->size || startIndex < 0) {
        *indexOut = -1;
        return VECTOR_INDEX_OUT_OF_RANGE_ERROR;
    }
    else if (size <= 0) {
        *indexOut = -1;
        return VECTOR_INVALID_PARAMS_ERROR;
    }

    int64_t index = -1;

    for (int64_t i = startIndex; i < vector->size; i++) {
        vector_element_t* element = vector->items[i];

        if (element->size != size)
            continue;

        if (vector->equalityFunction(element->value, value, element->size) == 0) 
            index = i;
    }

    *indexOut = index;
    return index != -1 ? VECTOR_SUCCESS : VECTOR_ELEMENT_NOT_FOUND_ERROR;
}


vector_result_t vector_sort(vector_t* const vector, const bool ascending) {
    if (vector == NULL) 
        return VECTOR_NULL_ERROR;
    else if (vector->size == 0 || vector->size == 1) 
        return VECTOR_SUCCESS;
    
    return vector->sortingFunction(vector, ascending);
}


vector_result_t vector_fill(vector_t* const vector, void* const value, const uint64_t size) {
    if (vector == NULL)
        return VECTOR_NULL_ERROR;

    for (int64_t i = vector->size; i < vector->capacity; i++) {
        vector_element_t* element;
        vector_result_t result = vector_element_create(value, size, &element);

        if (result != VECTOR_SUCCESS)
            return result;

        vector->items[i] = element;
    }

    vector->size = vector->capacity;
    return VECTOR_SUCCESS;
}


vector_result_t vector_swap(vector_t* const vector, const int64_t index1, const int64_t index2) {
    if (vector == NULL)
        return VECTOR_NULL_ERROR;
    else if (index1 >= vector->size || index1 < 0)
        return VECTOR_INDEX_OUT_OF_RANGE_ERROR;
    else if (index2 >= vector->size || index2 < 0)
        return VECTOR_INDEX_OUT_OF_RANGE_ERROR;

    if (index1 == index2)
        return VECTOR_SUCCESS;

    vector_element_t* temp = vector->items[index1];

    vector->items[index1] = vector->items[index2];
    vector->items[index2] = temp;

    return VECTOR_SUCCESS;
}


vector_result_t vector_element_free(vector_element_t** element) {
    if (*element == NULL)
        return VECTOR_INVALID_PARAMS_ERROR;

    free((*element)->value);
    (*element)->value = NULL;
    (*element)->size = 0;

    free(*element);
    *element = NULL;

    return VECTOR_SUCCESS;
}


vector_result_t vector_iterator_create(vector_iterator_t** iteratorOut, vector_t* vector) {
    if (vector == NULL)
        return VECTOR_NULL_ERROR;

    vector_iterator_t* iterator = (vector_iterator_t*) malloc(sizeof(vector_iterator_t));

    iterator->vector = vector;
    iterator->index = -1;
    iterator->element = NULL;

    *iteratorOut = iterator;
    return VECTOR_SUCCESS;
}


vector_result_t vector_iterator_next(vector_iterator_t* iterator) {
    if (iterator == NULL)
        return VECTOR_INVALID_PARAMS_ERROR;

    if (iterator->index == -1) {
        iterator->index = 0;
        iterator->element = iterator->vector->items[iterator->index];
    } 
    else if (iterator->index + 1 >= iterator->vector->size) {
        iterator->index = -1;
        iterator->element = NULL;

        return VECTOR_INDEX_OUT_OF_RANGE_ERROR;
    }
    else {
        iterator->index++;
        iterator->element = iterator->vector->items[iterator->index];
    }

    return VECTOR_SUCCESS;
}


vector_result_t vector_iterator_rewind(vector_iterator_t* const iterator) {
    if (iterator == NULL)
        return VECTOR_INVALID_PARAMS_ERROR;

    iterator->index = -1;
    iterator->element = NULL;

    return VECTOR_SUCCESS;
}


vector_result_t vector_iterator_free(vector_iterator_t** iterator) {
    if (iterator == NULL)
        return VECTOR_INVALID_PARAMS_ERROR;

    (*iterator)->vector = NULL;
    (*iterator)->index = 0;
    (*iterator)->element = NULL;

    free((*iterator));
    *iterator = NULL;
    return VECTOR_SUCCESS;
}

#pragma endregion
