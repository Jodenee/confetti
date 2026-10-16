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

#pragma once

// Headers

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "linked_list.h"

#include "confetti_export.h"

// definitions

typedef struct hashtable hashtable_t;
typedef struct hashtable_node hashtable_node_t;
typedef enum hashtable_result hashtable_result_t; 

typedef struct hashtable {
    linked_list_t* items; /* Storage of key value pairs */
    int64_t size;         /* Number of key value pairs currently in the hashtable. */
} hashtable_t;

typedef struct hashtable_node {
    void* key;     /* Unique key identifier. */
    void* value;   /* Pointer to the value. */
    uint64_t size; /* The size of the value in memory. */
} hashtable_node_t;

typedef enum hashtable_result {
    HASHTABLE_SUCCESS = 1,
    HASHTABLE_KEY_DOES_NOT_EXIST_ERROR = -1,
    HASHTABLE_NULL_ERROR = -2,
    HASHTABLE_INVALID_PARAMS_ERROR = -3,
    HASHTABLE_ALLOCATION_FAILURE = -4
} hashtable_result_t;

// public function definitions

CONFETTI_EXPORT hashtable_result_t hashtable_create(hashtable_t** hashtableOut);

CONFETTI_EXPORT hashtable_result_t hashtable_free(hashtable_t** hashtable);

CONFETTI_EXPORT hashtable_result_t hashtable_print(hashtable_t* const hashtable);

CONFETTI_EXPORT hashtable_result_t hashtable_insert(hashtable_t** const void* key, const void* value, const uint64_t size);

CONFETTI_EXPORT hashtable_result_t hashtable_get(hashtable_t** const void* key, linked_list_element_t** elementOut);

CONFETTI_EXPORT hashtable_result_t hashtable_update(hashtable_t** const void* key, const void* value, const uint64_t size);

CONFETTI_EXPORT hashtable_result_t hashtable_remove(hashtable_t** hashtable, const void* key);

CONFETTI_EXPORT hashtable_result_t hashtable_pop(hashtable_t** hashtable, const void* key, linked_list_element_t** elementOut);

CONFETTI_EXPORT hashtable_result_t hashtable_key_exists(hashtable_t** hashtable, const void* key);

CONFETTI_EXPORT hashtable_result_t hashtable_clear(hashtable_t** hashtable);

CONFETTI_EXPORT hashtable_result_t hashtable_copy(hashtable_t** hashtable, hashtable_t** const hashtableOut);

CONFETTI_EXPORT hashtable_result_t hashtable_merge(hashtable_t** hashtableOne, hashtable_t** hashtableTwo, hashtable_t** const hashtableOut);

CONFETTI_EXPORT hashtable_result_t hashtable_keys(hashtable_t** hashtable, void** const keysOut);

CONFETTI_EXPORT hashtable_result_t hashtable_values(hashtable_t** hashtable, void** const valuesOut);
