#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include <string.h>
#include <pthread.h>

#include "../includes/server.h"
#include "../includes/thread_array.h"
#include "../includes/client_handler.h"

/**
 * @brief Configures the server by creating a socket,
 * setting socket options, binding to a port,
 * and listening for incoming connections.
 *
 * @return int The file descriptor for the server socket.
 */
static int server_config()
{
    // Create a socket for the server
    int socket_fd = socket(AF_INET6, SOCK_STREAM, 0);

    if (socket_fd == -1)
    {
        perror("socket server");
        exit(EXIT_FAILURE);
    }

    // Set socket options to allow reuse of the address and port :
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) == -1)
    {
        perror("setsockopt server");
        goto error;
    }

    struct sockaddr_in6 server_addr;

    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin6_family = AF_INET6;
    server_addr.sin6_addr = in6addr_any;
    server_addr.sin6_port = htons(PORT);

    // Bind the socket to the specified port :
    if (bind(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("bind server");
        goto error;
    }

    // Listen for incoming connections :
    if (listen(socket_fd, 0) == -1)
    {
        perror("listen server");
        goto error;
    }

    return socket_fd;

error:
    close(socket_fd);
    exit(EXIT_FAILURE);
}

int main()
{

    int socket_fd = server_config();

    short ret = EXIT_FAILURE;

    thread_array_t *thread_array = thread_array_init(5);

    if (thread_array == NULL)
    {
        goto error;
    }

    size_t nb_client = 0;

    while (1)
    {

        // resize the thread array :
        if (nb_client >= thread_array->capacity)
        {
            if (thread_array_resize(thread_array) == NULL)
            {
                goto error;
            }
        }

        struct sockaddr_in6 client_addr;
        socklen_t client_addr_len = sizeof(client_addr);

        int *client_fd = malloc(sizeof(int));

        if (client_fd == NULL)
        {
            perror("malloc client fd");
            goto error;
        }

        // Accept an incoming connection :
        *client_fd = accept(socket_fd, (struct sockaddr *)&client_addr, &client_addr_len);

        if (*client_fd == -1)
        {
            perror("accept server");
            goto error;
        }

        if (pthread_create(&thread_array->threads[nb_client], NULL, handle, client_fd) != 0)
        {
            perror("pthread_create server");
            close(*client_fd);
            free(client_fd);
            goto error;
        }

        nb_client++;
    }

    for (size_t i = 0; i < thread_array->count; i++)
    {
        pthread_join(thread_array->threads[i], NULL);
    }

    ret = EXIT_SUCCESS;

error:   
    thread_array_destroy(thread_array);
    close(socket_fd);

    exit(ret);
}
