#include <stdio.h>
#include <stdlib.h>
#include "linked_list.h"

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

int main(void) {
    linked_list_t* list = NULL;
    linked_list_result_t listCreateResult = linked_list_create(&list, &compareInts, NULL);

    if (listCreateResult == LINKED_LIST_ALLOCATION_FAILURE) {
        printf("Not enough memory...");
        exit(EXIT_FAILURE);
    }

    while (1) {
        int userNumber = 0;

        printf("Enter a number (-1 to stop): ");
        int check = scanf("%d", &userNumber);

        if (!check) {
            printf("Invalid number, try again.");
            continue;
        }

        if (userNumber == -1)
            break;

        linked_list_result_t listAppendResult = linked_list_append(list, &userNumber, sizeof(int));

        if (listAppendResult == LINKED_LIST_ALLOCATION_FAILURE) {
            printf("Not enough memory...");
            linked_list_free(&list);
            exit(EXIT_FAILURE);
        }
    }

    linked_list_sort(list, 1);

    linked_list_iterator_t* iterator = NULL;
    linked_list_iterator_create(&iterator, list);

    while (linked_list_iterator_next(iterator) == LINKED_LIST_SUCCESS) {
        printf("%d\n", *(int*) iterator->node->element->value);
    }

    linked_list_iterator_free(&iterator);
    linked_list_free(&list);

    return EXIT_SUCCESS;
}