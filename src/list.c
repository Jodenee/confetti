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

#include "list.h"

// private function definitions

#pragma region private function definitions

/**
 * @brief Creates a new list element and initializes it with a value.
 * 
 * This function allocates memory for a new list element and
 * initializes its value with a copy of the provided data. If the value
 * is NULL, the element's value is set to NULL, and its size is set to
 * the specified value size.
 *
 * @param value A pointer to the value to be assigned to the new element.
 * @param size The size of value.
 * @param elementOut A double pointer to where the element will be stored.
 * 
 * @return 
 * - `LIST_SUCCESS` if the element was created successfully.
 * 
 * - `LIST_ALLOCATION_FAILURE` if memory allocation fails during the process.
 */
static list_result_t list_element_create(const void* const value, const uint64_t size, list_element_t** elementOut);

/**
 * @brief Sets the value of an existing list element.
 * 
 * This function updates the value of a specified list element
 * with a new value. If the size of the new value differs from the current
 * size of the element, it reallocates memory for the element's value.
 *
 * @param element A pointer to the list element whose value is to be set.
 * @param value A pointer to the new value to be assigned to the element.
 * @param size The size of value.
 * 
 * @return 
 * - `LIST_SUCCESS` if the value was set successfully.
 * 
 * - `LIST_ALLOCATION_FAILURE` if memory allocation fails during the process.
 */
static list_result_t list_element_set(list_element_t* const element, const void* const value, const uint64_t size);

/**
 * @brief Clones an existing list element.
 * 
 * This function creates a new list element that is a copy of
 * the specified existing element. It allocates memory for the new element
 * and copies the value from the original element.
 *
 * @param element A pointer to the list element to be cloned.
 * @param elementOut A double pointer to where the cloned element will be stored.
 * 
 * @return
 * - `LIST_SUCCESS` if the element was cloned successfully. 
 * 
 * - `LIST_ALLOCATION_FAILURE` if memory allocation fails during the process.
 */
static list_result_t list_element_clone(list_element_t* const element, list_element_t** elementOut);

/**
 * @brief Reallocates the capacity of the list to a new size.
 * 
 * This function adjusts the capacity of the specified list to
 * the new specified capacity. If the new capacity is less than the current
 * capacity, it frees any excess elements and updates the size of the list
 * if necessary. If the new capacity is greater, it reallocates memory for
 * the items array. If the new capacity is equal to the current capacity,
 * it returns success without making any changes.
 *
 * @param list A pointer to the list whose capacity is to be reallocated.
 * @param capacity The new capacity for the list.
 * 
 * @return 
 * - `LIST_SUCCESS` if the capacity was reallocated successfully. 
 * 
 * - `LIST_INVALID_PARAMS_ERROR` if the new capacity is less than 1.
 * 
 * - `LIST_ALLOCATION_FAILURE` if memory allocation fails during the process.
 */
static list_result_t list_realloc_capacity(list_t* const list, const int64_t capacity);

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
 * @brief Partitions the list for the Median of Three Pivot Quicksort algorithm and returns the pivot.
 *
 * This function partitions the specified portion of the list around a pivot element. 
 * It rearranges the elements in such a way that all elements less than or equal to the pivot 
 * are on one side, and all elements greater than the pivot are on the other side. 
 * The pivot is chosen based on the specified order.
 *
 * @param list A pointer to the list to be partitioned.
 * @param low The starting index of the portion of the list to be partitioned.
 * @param high The ending index of the portion of the list to be partitioned.
 * @param ascending A boolean value indicating the sort order.
 * 
 * @return The index of the pivot element after partitioning.
 * 
 * @note This function assumes that the portion of the list to be partitioned contains at least one element.
 */
static int64_t partition(list_t* const list, const int64_t low, const int64_t high, const bool ascending);

/**
 * @brief Sorts the elements of the list using Median of Three Pivot Quicksort.
 * 
 * This function sorts the elements of the specified list using
 * the `qsort_s` function. The sorting order is determined by the `ascending`
 * parameter, which specifies whether to sort in ascending or descending order.
 * The function uses the provided custom equality function to compare elements
 * during sorting.
 *
 * @param list A pointer to the list whose elements are to be sorted.
 * @param low The starting index of the portion of the list to be sorted.
 * @param high The ending index of the portion of the list to be sorted.
 * @param ascending A boolean value indicating the sort order.
 * 
 * @note The list is sorted in place.
 */
void quicksort(list_t* const list, const int64_t low, const int64_t high, const bool ascending);

/**
 * @brief Sorts the elements of the list using a specified sorting order.
 * 
 * This function sorts the elements of the specified list using
 * the `qsort_s` function. The sorting order is determined by the `ascending`
 * parameter, which specifies whether to sort in ascending or descending order.
 * The function uses the provided custom equality function to compare elements
 * during sorting.
 *
 * @param list A pointer to the list whose elements are to be sorted.
 * @param ascending A boolean value indicating the sort order; true for
 *                  ascending order and false for descending order.
 */
static list_result_t default_sort(list_t* const list, const bool ascending);

#pragma endregion

// private functions

#pragma region private functions

static list_result_t list_element_create(const void* const value, const uint64_t size, list_element_t** elementOut) {
    list_element_t* element = (list_element_t*) malloc(sizeof(list_element_t));

    if (element == NULL)
        return LIST_ALLOCATION_FAILURE;

    element->value = NULL;
    element->size = size;

    if (value != NULL) {
        element->value = (void*) malloc(size);

        if (element->value == NULL) {
            free(element);
            element = NULL;

            return LIST_ALLOCATION_FAILURE;
        }

        memcpy(element->value, value, size);
    }

    *elementOut = element;
    return LIST_SUCCESS;
}


static list_result_t list_element_set(list_element_t* const element, const void* value, const uint64_t size) {
    if (size != element->size) {
        if (element->value == NULL)
            element->value = (void*)malloc(size);
        else
            element->value = (void*)realloc(element->value, size);

        element->size = size;

        if (element->value == NULL)
            return LIST_ALLOCATION_FAILURE;
    }

    memcpy(element->value, value, size);
    return LIST_SUCCESS;
}


static list_result_t list_element_clone(list_element_t* const element, list_element_t** elementOut) {
    list_element_t* elementClone = (list_element_t*)malloc(sizeof(list_element_t));

    if (elementClone == NULL)
        return LIST_ALLOCATION_FAILURE;

    elementClone->value = NULL;
    elementClone->size = element->size;

    if (element->value != NULL) {
        elementClone->value = (void*)malloc(element->size);

        if (elementClone->value == NULL)
            return LIST_ALLOCATION_FAILURE;

        memcpy(elementClone->value, element->value, element->size);
    }

    *elementOut = elementClone;
    return LIST_SUCCESS;
}


static list_result_t list_realloc_capacity(list_t* const list, const int64_t capacity) {
    if (capacity < 1)
        return LIST_INVALID_PARAMS_ERROR;

    int64_t oldCapacity = list->capacity;

    if (capacity == oldCapacity)
        return LIST_SUCCESS;

    if (capacity < oldCapacity) {
        for (int64_t i = oldCapacity - 1; i > capacity - 1; i--) {
            if (list->items[i] != NULL) {
                list_element_t* element = list->items[i];

                list_element_free(&element);
            }

            list->items[i] = NULL;
        }

        if (list->size > capacity)
            list->size = capacity;
    }

    list->capacity = capacity;
    list->items = (list_element_t**) realloc(list->items, sizeof(list_element_t*) * capacity);

    if (list->items == NULL)
        return LIST_ALLOCATION_FAILURE;

    if (capacity > oldCapacity) {
        for (int64_t i = oldCapacity; i < capacity - 1; i++) {
            list->items[i] = NULL;
        }
    }

    return LIST_SUCCESS;
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


static int64_t partition(list_t* const list, const int64_t low, const int64_t high, const bool ascending) {
    if (high - low + 1 >= 3) {
        int64_t mid = low + (high - low) / 2;

        if (ascending) {
            if (list->equalityFunction(list->items[low]->value, list->items[mid]->value, list->items[low]->size) > 0) {
                list_swap(list, low, mid);
            }

            if (list->equalityFunction(list->items[low]->value, list->items[high]->value, list->items[low]->size) > 0) {
                list_swap(list, low, high);
            }

            if (list->equalityFunction(list->items[mid]->value, list->items[high]->value, list->items[mid]->size) > 0) {
                list_swap(list, mid, high);
            }
        }
        else {
            if (list->equalityFunction(list->items[low]->value, list->items[mid]->value, list->items[low]->size) < 0) {
                list_swap(list, low, mid);
            }

            if (list->equalityFunction(list->items[low]->value, list->items[high]->value, list->items[low]->size) < 0) {
                list_swap(list, low, high);
            }

            if (list->equalityFunction(list->items[mid]->value, list->items[high]->value, list->items[mid]->size) < 0) {
                list_swap(list, mid, high);
            }
        }

        list_swap(list, mid, high - 1);
    }

    list_element_t* pivot_element = list->items[high - 1];
    void* pivot_value = pivot_element->value;
    uint64_t pivot_size = pivot_element->size;

    int64_t i = (low - 1);

    for (int64_t j = low; j <= high - 2; j++) {
        int32_t comparison_result = list->equalityFunction(list->items[j]->value, pivot_value, pivot_size);

        bool condition = ascending 
            ? (comparison_result <= 0) 
            : (comparison_result >= 0);

        if (condition) {
            i++;
            list_swap(list, i, j);
        }
    }

    list_swap(list, i + 1, high - 1);
    return (i + 1);
}


void quicksort(list_t* const list, const int64_t low, const int64_t high, const bool ascending) {
    if (low < high) {
        int32_t p = partition(list, low, high, ascending);

        quicksort(list, low, p - 1, ascending);
        quicksort(list, p + 1, high, ascending);
    }
}


static list_result_t default_sort(list_t* const list, const bool ascending)
{
    quicksort(list, 0, list->size - 1, ascending);

    return LIST_SUCCESS;
}

#pragma endregion

// public functions

#pragma region public functions

list_result_t list_create(
    list_t** listOut,
    const int64_t capacity,
    list_custom_equality_function_t* const customEqualityFunction,
    list_custom_sorting_function_t* const customSortingFunction
) {
    list_t* list = (list_t*) malloc(sizeof(list_t));

    if (list == NULL)
        return LIST_ALLOCATION_FAILURE;

    list->size = 0;
    list->capacity = capacity < 1 ? DEFAULT_LIST_CAPACITY : capacity;
    list->items = (list_element_t**) calloc(list->capacity, sizeof(list_element_t*));
    list->equalityFunction = customEqualityFunction == NULL ? (list_custom_equality_function_t*) &default_equals : customEqualityFunction;
    list->sortingFunction = customSortingFunction == NULL ? (list_custom_sorting_function_t*) &default_sort : customSortingFunction;

    if (list->items == NULL) {
        free(list);
        return LIST_ALLOCATION_FAILURE;
    }

    *listOut = list;
    return LIST_SUCCESS;
}


list_result_t list_free(list_t** list) {
    if (*list == NULL)
        return LIST_NULL_ERROR;

    for (int64_t i = 0; i < (*list)->size; i++) {
        list_element_t* element = (*list)->items[i];

        if (element == NULL)
            continue;

        free(element->value);
        element->value = NULL;
        element->size = 0;

        free(element);
        (*list)->items[i] = NULL;
    }

    free((*list)->items);
    (*list)->items = NULL;

    free(*list);
    *list = NULL;

    return LIST_SUCCESS;
}


list_result_t list_print(list_t* list) {
    if (list == NULL)
        return LIST_NULL_ERROR;

    printf("[");

    for (int64_t i = 0LL; i < list->size; i++) {
        list_element_t* element = list->items[i];

        if (element->value != NULL)
            printf(i == list->size - 1 ? "%p" : "%p, ", element->value);
        else
            printf(i == list->size - 1 ? "NULL" : "NULL, ", element->value);
    }

    printf("] -> %p\n", list);

    return LIST_SUCCESS;
}


list_result_t list_resize(list_t* list, const int64_t capacity) {
    if (list == NULL)
        return LIST_NULL_ERROR;
    else if (capacity < 1)
        return LIST_INVALID_PARAMS_ERROR;
    else if (capacity == list->capacity)
        return LIST_SUCCESS;

    list_result_t result;
    result = list_realloc_capacity(list, capacity);

    if (result != LIST_SUCCESS) 
        return result;

    return LIST_SUCCESS;
}


list_result_t list_prepend(list_t* list, void* value, const uint64_t size) {
    if (list == NULL)
        return LIST_NULL_ERROR;

    if (list->size == list->capacity) {
        list_result_t result = list_realloc_capacity(list, list->capacity * 2);

        if (result != LIST_SUCCESS) 
            return result;
    }

    for (int64_t i = list->size - 1; i >= 0; i--)
        list->items[i + 1] = list->items[i];
    
    list_element_t* element = (list_element_t*) malloc(sizeof(list_element_t));
    list_result_t result = list_element_create(value, size, &element);

    if (result != LIST_SUCCESS)
        return result;

    list->items[0] = element;
    list->size++;

    return LIST_SUCCESS;
}


list_result_t list_append(list_t* list, void* const value, const uint64_t size) {
    if (list == NULL)
        return LIST_NULL_ERROR;

    if (list->size == list->capacity) {
        list_result_t result = list_realloc_capacity(list, list->capacity * 2);

        if (result != LIST_SUCCESS) 
            return result;
    }

    list_element_t* element;
    list_result_t result = list_element_create(value, size, &element);

    if (result != LIST_SUCCESS)
        return result;

    list->items[list->size++] = element;
    return LIST_SUCCESS;
}


list_result_t list_insert(list_t* list, const int64_t index, void* const value, const uint64_t size) {
    if (list == NULL)
        return LIST_NULL_ERROR;
    else if (index > list->size || index < 0)
        return LIST_INDEX_OUT_OF_RANGE_ERROR;

    if (list->size == list->capacity) {
        list_result_t result = list_realloc_capacity(list, list->capacity * 2);

        if (result != LIST_SUCCESS) return result;
    }

    for (int i = list->size - 1; i >= index; i--)
        list->items[i + 1] = list->items[i];

    list_result_t createResult = list_element_create(value, size, &list->items[index]);
    list->size++;

    return LIST_SUCCESS;
}


list_result_t list_get(list_t* list,  list_element_t** elementOut, const int64_t index) {
    if (list == NULL)
        return LIST_NULL_ERROR;
    else if (index >= list->size || index < 0)
        return LIST_INDEX_OUT_OF_RANGE_ERROR;

    list_element_t* element;
    list_result_t result = list_element_clone(list->items[index], &element);

    if (result != LIST_SUCCESS) {
        *elementOut = NULL;
        return result;
    }

    *elementOut = element;
    return LIST_SUCCESS;
}


list_result_t list_set(list_t* list, const int64_t index, void* const value, const uint64_t size) {
    if (list == NULL)
        return LIST_NULL_ERROR;
    else if (index >= list->size || index < 0)
        return LIST_INDEX_OUT_OF_RANGE_ERROR;
    else if (size == 0)
        return LIST_INVALID_PARAMS_ERROR;

    if (list->items[index] == NULL) {
        list_element_t* element;
        list_result_t result = list_element_create(value, size, &element);

        if (result == LIST_SUCCESS)
            list->items[index] = element;
        else
            return result;
    }
    else {
        list_result_t result = list_element_set(list->items[index], value, size);

        if (result != LIST_SUCCESS)
            return result;
    }

    return LIST_SUCCESS;
}


list_result_t list_remove(list_t* list, const int64_t index) {
    if (list == NULL)
        return LIST_NULL_ERROR;
    else if (index >= list->capacity || index < 0)
        return LIST_INDEX_OUT_OF_RANGE_ERROR;

    list_result_t freeResult = list_element_free(&list->items[index]);

    if (freeResult != LIST_SUCCESS)
        return freeResult;

    for (int64_t i = index; i < list->size - 1; i++)
        list->items[i] = list->items[i + 1];

    list->size--;
    return LIST_SUCCESS;
}


list_result_t list_pop(list_t* const list, list_element_t** elementOut, const int64_t index) {
    if (list == NULL)
        return LIST_NULL_ERROR;
    else if (index >= list->capacity || index < 0)
        return LIST_INDEX_OUT_OF_RANGE_ERROR;

    list_result_t cloneResult = list_element_clone(list->items[index], elementOut);
    
    if (cloneResult != LIST_SUCCESS)
        return cloneResult;
    
    list_result_t freeResult = list_element_free(&list->items[index]);

    if (freeResult != LIST_SUCCESS)
        return freeResult;

    for (int64_t i = index; i < list->size - 1; i++)
        list->items[i] = list->items[i + 1];

    list->size--;
    return LIST_SUCCESS;
}


list_result_t list_reverse(list_t* const list) {
    if (list == NULL)
        return LIST_NULL_ERROR;

    list_element_t** start = list->items;
    list_element_t** end = list->items + list->size - 1;
    int64_t mid = list->size / 2LL;

    for (int64_t i = 0LL; i < mid; i++) {
        list_element_t* leftElement = *start;
        list_element_t* rightElement = *end;

        *start = rightElement;
        *end = leftElement;

        start++;
        end--;
    }

    return LIST_SUCCESS;
}


list_result_t list_clone(list_t* const list, list_t** listOut) {
    if (list == NULL)
        return LIST_NULL_ERROR;

    list_t* const listClone = (list_t*) malloc(sizeof(list_t));

    if (list == NULL)
        return LIST_ALLOCATION_FAILURE;

    listClone->size = list->size;
    listClone->capacity = list->capacity;
    listClone->items = (list_element_t**) calloc(listClone->capacity, sizeof(list_element_t*));

    if (listClone->items == NULL)
        return LIST_ALLOCATION_FAILURE;

    for (int64_t i = 0LL; i < list->size; i++) {
        list_element_t* elementClone;
        list_result_t result = list_element_clone(list->items[i], &elementClone);

        listClone->items[i] = elementClone;
    }
        
    *listOut = listClone;
    return LIST_SUCCESS;
}


list_result_t list_clear(list_t* const list) {
    if (list == NULL)
        return LIST_NULL_ERROR;

    for (int64_t i = 0; i < list->size; i++) {
        list_element_free(&list->items[i]);
    }

    list->size = 0;
    return LIST_SUCCESS;
}


list_result_t list_join(list_t* const list1, list_t* const list2, list_t** listOut) {
    if (list1 == NULL)
        return LIST_NULL_ERROR;
    else if (list2 == NULL)
        return LIST_NULL_ERROR;

    list_t* joinList;
    list_result_t result = list_create(&joinList, (list1->size + list2->size), NULL, NULL);

    if (result != LIST_SUCCESS)
        return result;

    list_element_t** tempJoinListPointer = joinList->items;

    for (int64_t i = 0; i < list1->size; i++) {
        list_element_t* element;
        list_element_clone(list1->items[i], &element);

        *tempJoinListPointer = element;
        tempJoinListPointer++;
    }

    for (int64_t i = 0; i < list2->size; i++) {
        list_element_t* element;
        list_element_clone(list2->items[i], &element);

        *tempJoinListPointer = element;
        tempJoinListPointer++;
    }

    joinList->size = joinList->capacity;
    *listOut = joinList;
    return LIST_SUCCESS;
}


list_result_t list_includes(list_t* const list, void* value, const uint64_t size) {
    if (list == NULL)
        return LIST_NULL_ERROR;

    for (int64_t i = 0LL; i < list->size; i++)
    {
        list_element_t* element = list->items[i];

        if (element->size != size)
            continue;

        if (list->equalityFunction(element->value, value, element->size) == 0)
            return LIST_SUCCESS;
    }

    return LIST_ELEMENT_NOT_FOUND_ERROR;
}


list_result_t list_find_first(
    list_t* const list, 
    int64_t* const indexOut, 
    const int64_t startIndex, 
    void* const value, 
    const uint64_t size
) {
    if (list == NULL) {
        *indexOut = -1;
        return LIST_NULL_ERROR;
    }
    else if (startIndex >= list->size || startIndex < 0) {
        *indexOut = -1;
        return LIST_INVALID_PARAMS_ERROR;
    }
    else if (size <= 0) {
        *indexOut = -1;
        return LIST_INVALID_PARAMS_ERROR;
    }

    int64_t index = startIndex > 0LL ? startIndex : 0LL;

    for (int64_t i = index; i < list->size; i++) {
        list_element_t* element = list->items[i];

        if (element->size != size)
            continue;

        if (list->equalityFunction(element->value, value, element->size) == 0) {
            *indexOut = i;
            return LIST_SUCCESS;
        }
    }

    *indexOut = -1;
    return LIST_ELEMENT_NOT_FOUND_ERROR;
}


list_result_t list_find_last(list_t* const list, int64_t* indexOut, const int64_t startIndex, void* value, const uint64_t size) {
    if (list == NULL) {
        *indexOut = -1;
        return LIST_NULL_ERROR;
    }
    else if (startIndex >= list->size || startIndex < 0) {
        *indexOut = -1;
        return LIST_INDEX_OUT_OF_RANGE_ERROR;
    }
    else if (size <= 0) {
        *indexOut = -1;
        return LIST_INVALID_PARAMS_ERROR;
    }

    int64_t index = -1;

    for (int64_t i = startIndex; i < list->size; i++) {
        list_element_t* element = list->items[i];

        if (element->size != size)
            continue;

        if (list->equalityFunction(element->value, value, element->size) == 0) 
            index = i;
    }

    *indexOut = index;
    return index != -1 ? LIST_SUCCESS : LIST_ELEMENT_NOT_FOUND_ERROR;
}


list_result_t list_sort(list_t* const list, const bool ascending) {
    if (list == NULL) 
        return LIST_NULL_ERROR;
    else if (list->size == 0 || list->size == 1) 
        return LIST_SUCCESS;
    
    return list->sortingFunction(list, ascending);
}


list_result_t list_fill(list_t* const list, void* const value, const uint64_t size) {
    if (list == NULL)
        return LIST_NULL_ERROR;

    for (int64_t i = list->size; i < list->capacity; i++) {
        list_element_t* element;
        list_result_t result = list_element_create(value, size, &element);

        if (result != LIST_SUCCESS)
            return result;

        list->items[i] = element;
    }

    list->size = list->capacity;
    return LIST_SUCCESS;
}


list_result_t list_swap(list_t* const list, const int64_t index1, const int64_t index2) {
    if (list == NULL)
        return LIST_NULL_ERROR;
    else if (index1 >= list->size || index1 < 0)
        return LIST_INDEX_OUT_OF_RANGE_ERROR;
    else if (index2 >= list->size || index2 < 0)
        return LIST_INDEX_OUT_OF_RANGE_ERROR;

    if (index1 == index2)
        return LIST_SUCCESS;

    list_element_t* temp = list->items[index1];

    list->items[index1] = list->items[index2];
    list->items[index2] = temp;

    return LIST_SUCCESS;
}


list_result_t list_element_free(list_element_t** element) {
    if (*element == NULL)
        return LIST_INVALID_PARAMS_ERROR;

    free((*element)->value);
    free(*element);

    (*element)->value = NULL;
    (*element)->size = 0;
    *element = NULL;

    return LIST_SUCCESS;
}


list_result_t list_iterator_create(list_iterator_t** iteratorOut, list_t* list) {
    if (list == NULL)
        return LIST_NULL_ERROR;

    list_iterator_t* iterator = (list_iterator_t*) malloc(sizeof(list_iterator_t));

    iterator->list = list;
    iterator->index = -1;
    iterator->element = NULL;

    *iteratorOut = iterator;
    return LIST_SUCCESS;
}


list_result_t list_iterator_next(list_iterator_t* iterator) {
    if (iterator == NULL)
        return LIST_INVALID_PARAMS_ERROR;

    if (iterator->index == -1) {
        iterator->index = 0;
        iterator->element = iterator->list->items[iterator->index];
    } 
    else if (iterator->index + 1 >= iterator->list->size) {
        iterator->index = -1;
        iterator->element = NULL;

        return LIST_INDEX_OUT_OF_RANGE_ERROR;
    }
    else {
        iterator->index++;
        iterator->element = iterator->list->items[iterator->index];
    }

    return LIST_SUCCESS;
}


list_result_t list_iterator_rewind(list_iterator_t* const iterator) {
    if (iterator == NULL)
        return LIST_INVALID_PARAMS_ERROR;

    iterator->index = -1;
    iterator->element = NULL;

    return LIST_SUCCESS;
}


list_result_t list_iterator_free(list_iterator_t** iterator) {
    if (iterator == NULL)
        return LIST_INVALID_PARAMS_ERROR;

    (*iterator)->list = NULL;
    (*iterator)->index = 0;
    (*iterator)->element = NULL;

    free((*iterator));
    *iterator = NULL;
    return LIST_SUCCESS;
}

#pragma endregion
