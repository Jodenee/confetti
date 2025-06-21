#include <stdio.h>
#include <stdlib.h>
#include "list.h"

int main(void) {
    list_t* list;
    list_result_t result = list_create(
        &list, 
        (int64_t) NULL, 
        (list_custom_equality_function_t*) NULL, 
        (list_custom_sorting_function_t*) NULL
    );
    printf("create result: %d\n", result);

    for (int i = 0; i < 100; i++)
        list_append(list, &i, sizeof(int));

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