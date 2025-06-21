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

linked_list_result_t insertion_sort(linked_list_t* const linkedList, const bool ascending) {
    linked_list_node_t* sorted = NULL;
    linked_list_node_t* current = linkedList->head;

    while (current != NULL) {
        linked_list_node_t* next = current->next;
        current->next = NULL;

        if (sorted == NULL || (ascending && linkedList->equalityFunction(current->element->value, sorted->element->value, current->element->size) < 0) ||
            (!ascending && linkedList->equalityFunction(current->element->value, sorted->element->value, current->element->size) > 0)) {

            current->next = sorted;
            sorted = current;
        } else {

            linked_list_node_t* temp = sorted;
            while (temp->next != NULL && 
                   ((ascending && linkedList->equalityFunction(current->element->value, temp->next->element->value, current->element->size) >= 0) ||
                    (!ascending && linkedList->equalityFunction(current->element->value, temp->next->element->value, current->element->size) <= 0))) {
                temp = temp->next;
            }

            current->next = temp->next;
            temp->next = current;
        }

        current = next;
    }

    linkedList->head = sorted;

    return LINKED_LIST_SUCCESS;
}

int main(void) {
    srand(time(NULL));

    linked_list_t* list;
    linked_list_result_t result = linked_list_create(
        &list, 
        &compareInts, 
        &insertion_sort
    );
    printf("create result: %d\n", result);

    for (int i = 0; i < 10; i++) {
        int num = rand() % 100 + 1;
        linked_list_append(list, &num, sizeof(int));
    }
        
    linked_list_sort(list, true);

    linked_list_iterator_t* iterator = NULL;
    linked_list_iterator_create(&iterator, list);

    while (linked_list_iterator_next(iterator) == LINKED_LIST_SUCCESS) {
        printf("value: %d\n", *(int*) iterator->node->element->value);
    }

    linked_list_iterator_free(&iterator);
    linked_list_free(&list);

    return EXIT_SUCCESS;
}