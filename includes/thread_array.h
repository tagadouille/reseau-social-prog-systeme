#ifndef THREAD_ARRAY_H
#define THREAD_ARRAY_H

#include <pthread.h>
#include <stdlib.h>

/**
 * @brief A structure to manage an array of threads.
 */
typedef struct thread_array {

    size_t count; // Current number of threads
    size_t capacity; // Capacity of the thread array
    pthread_t * threads; // Dynamic array of thread identifiers

} thread_array_t;

/**
 * @brief Initializes a thread array with a specified initial capacity.
 * 
 * @param initial_capacity The initial capacity of the thread array.
 * @return A pointer to the initialized thread array, or NULL on failure.
 */
thread_array_t * thread_array_init(size_t initial_capacity);

/**
 * @brief Resizes the thread array by doubling its capacity.
 * 
 * @param array A pointer to the thread array to resize.
 * @return A pointer to the resized thread array, or NULL on failure.
 */
thread_array_t * thread_array_resize(thread_array_t * array);

/**
 * @brief Destroys the thread array and frees associated resources.
 * @param array A pointer to the thread array to destroy.
 */
void thread_array_destroy(thread_array_t * array);

#endif