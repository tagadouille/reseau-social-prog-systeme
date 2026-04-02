#include <stdio.h>
#include <stdlib.h>

#include "thread_array.h"

thread_array_t *thread_array_init(size_t initial_capacity)
{

    thread_array_t *array = malloc(sizeof(thread_array_t));

    if (array == NULL)
    {
        perror("malloc thread array");
        return NULL;
    }

    array->count = 0;
    array->capacity = initial_capacity;
    array->threads = malloc(initial_capacity * sizeof(pthread_t));

    if (array->threads == NULL)
    {
        perror("malloc thread array threads");
        free(array);
        return NULL;
    }
    return array;
}

thread_array_t *thread_array_resize(thread_array_t *array)
{

    size_t new_capacity = array->capacity * 2;

    pthread_t *new_threads = realloc(array->threads, new_capacity * sizeof(pthread_t));
    if (new_threads == NULL)
    {
        perror("realloc thread array threads");
        return NULL;
    }

    array->threads = new_threads;
    array->capacity = new_capacity;
    return array;
}

void thread_array_destroy(thread_array_t *array)
{
    if (array)
    {
        free(array->threads);
        free(array);
    }
}