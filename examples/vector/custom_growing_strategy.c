#include <stdio.h>
#include <stdlib.h>

#include "vector.h"

static vector_result_t custom_growing_strategy(int64_t const currentCapacity, int64_t *const newCapacityOut)
{
    if (newCapacityOut == NULL)
        return VECTOR_INVALID_PARAMS_ERROR;
    else if (currentCapacity > INT64_MAX / 2)
        return VECTOR_SIZE_LIMIT_REACHED;

    int64_t newCapacity = currentCapacity * 3;

    vector_result_t validationResult = validate_capacity(newCapacity);

    if (validationResult != VECTOR_SUCCESS)
        return validationResult;

    *newCapacityOut = newCapacity;
    return VECTOR_SUCCESS;
}

int main(void)
{
    vector_t *vector = NULL;

    vector_result_t result = vector_create(
        DEFAULT_VECTOR_CAPACITY,
        custom_growing_strategy,
        NULL,
        NULL,
        NULL,
        &vector
    );

    if (result != VECTOR_SUCCESS)
    {
        fprintf(stderr, "Failed to create vector: %d\n", result);
        return EXIT_FAILURE;
    }

    for (int64_t i = 0; i < vector->capacity + 1; i++) {
        vector_append(vector, &i, sizeof(int64_t));
    }

    fprintf(stdout, "vector capacity after first grow: %d\n", vector->capacity);

    vector_free(&vector);
    return EXIT_SUCCESS;
}