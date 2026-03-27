#ifndef SERVER_H
#define SERVER_H

#define PORT 4321


/**
 * @brief A structure to manage an array of threads.
 */
typedef struct thread_array {

    size_t count; // Current number of threads
    size_t capacity; // Capacity of the thread array
    pthread_t * threads; // Dynamic array of thread identifiers

} thread_array_t;


#endif