#include <stdio.h>
#include <stdlib.h>

#include "vector.h"

typedef struct
{
    uint64_t id;
    char name[64];
    uint8_t age;
} person_t;

vector_result_t print_person(const void *value, size_t size, FILE *stream) {
    int streamPrintResult;

    if (value == NULL)
        streamPrintResult = fprintf(stream, "NULL");
    else {
        person_t* person = (person_t*) value;
        streamPrintResult = fprintf(stream, "{ .id=%d .name=%s, .age=%d }", person->id, person->name, person->age);
    }
        
    if (streamPrintResult < 0)
        return VECTOR_IO_ERROR;

    return VECTOR_SUCCESS;
}

int32_t person_compare(
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

    if (dataOneSize > dataTwoSize)
        return 1;
    else if (dataOneSize < dataTwoSize)
        return -1;

    const person_t *const personOne = (const person_t *const) dataOne;
    const person_t *const personTwo = (const person_t *const) dataTwo;

    bool isEqual = personOne->id == personTwo->id;

    if (isEqual)
        return 0;
    else {
        if (personOne->age > personTwo->age)
            return 1;
        else if (personOne->age < personTwo->age) 
            return -1;
    }

    if (dataOneSize < dataTwoSize)
        return -1;

    if (dataOneSize > dataTwoSize)
        return 1;

    return 0;
}

int main(void)
{
    vector_t *people = NULL;

    vector_result_t result = vector_create(
        DEFAULT_VECTOR_CAPACITY,
        NULL,
        person_compare,
        NULL,
        print_person,
        &people
    );

    if (result != VECTOR_SUCCESS)
    {
        fprintf(stderr, "Failed to create vector: %d\n", result);
        return EXIT_FAILURE;
    }

    person_t personOne = {
        .id = 1,
        .name = "Crusnr Colven",
        .age = 25
    };
    person_t personTwo = {
        .id = 2,
        .name = "C'venash Vishneri",
        .age = 23
    };

    // adding custom objects

    vector_append(people, &personOne, sizeof(personOne));
    vector_insert(people, 0, &personTwo, sizeof(personTwo));

    // finding custom objects

    vector_result_t includesResult = vector_includes(people, &personOne, sizeof(personOne));

    if (includesResult == VECTOR_SUCCESS)
        fprintf(stdout, "personOne successfully found inside people vector\n", result);
    else
        fprintf(stderr, "personOne was not successfully find inside people vector\n", result);

    // get a custom object

    vector_element_t* element = NULL;
    vector_result_t getResult = vector_get(people, 0, &element);

    person_t* personTwoClone = (person_t*) element->value;    

    fprintf(
        stdout, 
        "retrieved personTwo from vector: {.id=%d, .name=%s, .age=%d}\n", 
        personTwoClone->id,
        personTwoClone->name,
        personTwoClone->age
    );

    vector_element_free(&element);

    // sorting and printing custom objects

    vector_sort(people, true);
    vector_print(people, stdout);

    vector_sort(people, false);
    vector_print(people, stdout);

    vector_free(&people);
    return EXIT_SUCCESS;
}