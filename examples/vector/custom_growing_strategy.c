
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include "vector.h"

static vector_result_t custom_growing_strategy(
    int64_t const currentCapacity,
    int64_t *const newCapacityOut)
{
    if (newCapacityOut == NULL)
        return VECTOR_INVALID_PARAMS_ERROR;

    if (currentCapacity > INT64_MAX / 3)
        return VECTOR_SIZE_LIMIT_REACHED;

    int64_t newCapacity = currentCapacity * 3;

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
        fprintf(
            stderr, 
            "Failed to create vector: %s\n",
            vector_result_to_string(result)
        );
        return EXIT_FAILURE;
    }

    int64_t initialCapacity = vector->capacity;

    for (int64_t i = 0; i <= initialCapacity; i++)
    {
        result = vector_append(vector, &i, sizeof(i));

        if (result != VECTOR_SUCCESS)
        {
            fprintf(
                stderr, 
                "Failed to append element: %s\n",
                vector_result_to_string(result)
            );

            vector_free(&vector);
            return EXIT_FAILURE;
        }
    }

    fprintf(stdout, "Initial capacity: %" PRId64 "\n", initialCapacity);
    fprintf(stdout, "Capacity after growth: %" PRId64 "\n", vector->capacity);

    vector_free(&vector);

    return EXIT_SUCCESS;
}
