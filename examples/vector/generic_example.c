#include <stdio.h>
#include <stdlib.h>
#include "vector.h"

int main(void)
{
    char exitWord[] = "EXIT";
    vector_t *words = NULL;

    vector_result_t result = vector_create(
        DEFAULT_VECTOR_CAPACITY,
        NULL,
        NULL,
        NULL,
        NULL,
        &words
    );

    if (result != VECTOR_SUCCESS)
    {
        fprintf(stderr, "Failed to create vector: %d\n", result);
        return EXIT_FAILURE;
    }

    while (true) {
        char buffer[256];

        fprintf(stdout, "Enter a word (EXIT to stop): ");
        int read = scanf("%255s", buffer);

        if (read <= 0)
            continue;
        else if (strcmp(buffer, exitWord) == 0) {
            break;
        }

        vector_append(words, buffer, sizeof(char) * strlen(buffer));
    }

    vector_print(words, stdout);
    vector_free(&words);
    return EXIT_SUCCESS;
}