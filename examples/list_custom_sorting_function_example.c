#include <stdio.h>
#include <stdlib.h>
#include "list.h"

int32_t compareInts(const void* const data1, const void* const data2, const uint64_t size) {
    if (data1 == NULL && data2 != NULL)
        return -1;
    else if (data1 != NULL && data2 == NULL) 
        return 1;
    else if (data1 == NULL && data2 == NULL)
        return 0;

    int num1 = *(int*) data1;
    int num2 = *(int*) data2;

    return num1 > num2 
        ? 1 
        : num1 == num2 
            ? 0 
            : -1;
}

list_result_t bubble_sort(list_t* const list, const bool ascending) {
    for (int64_t i = 0; i < list->size - 1; i++) {
        for (int64_t j = 0; j < list->size - i - 1; j++) {
            list_element_t* left = list->items[j];
            list_element_t* right = list->items[j + 1];

            int32_t comparisonResult;
            if (list->equalityFunction) {
                comparisonResult = list->equalityFunction(left->value, right->value, left->size);
            } else {
                comparisonResult = (*(int32_t*)left->value - *(int32_t*)right->value);
            }

            if ((ascending && comparisonResult > 0) || (!ascending && comparisonResult < 0)) {
                list_element_t* temp = list->items[j];
                list->items[j] = list->items[j + 1];
                list->items[j + 1] = temp;
            }
        }
    }
    
    return LIST_SUCCESS;
}

int main(void) {
    srand(time(NULL));

    list_t* list;
    list_result_t result = list_create(
        &list, 
        (int64_t) NULL, 
        (list_custom_equality_function_t*) NULL, 
        (list_custom_sorting_function_t*) &bubble_sort
    );
    printf("create result: %d\n", result);

    for (int i = 0; i < 10; i++) {
        int num = rand() % 100 + 1;
        list_append(list, &num, sizeof(int));
    }
        
    list_sort(list, true);

    list_iterator_t* iterator = NULL;
    list_iterator_create(&iterator, list);

    while (list_iterator_next(iterator) == LIST_SUCCESS) {
        printf("value: %d\n", *(int*) iterator->element->value);
    }

    list_iterator_free(&iterator);
    list_free(&list);

    return EXIT_SUCCESS;
}