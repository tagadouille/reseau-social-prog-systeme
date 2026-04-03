#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>

#include "server.h"
#include "thread_array.h"
#include "client_handler.h"
#include "log.h"

void sig_handler(int signum);

int is_terminated = 0;

static void signal_hijack()
{
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sig_handler;

    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGQUIT, &sa, NULL);
    sigaction(SIGHUP, &sa, NULL);
}
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

    log_server("\n\n-----------SERVER STARTED----------\n server socket configuring...");
    int socket_fd = server_config();
    signal_hijack();
    log_server("Server configured, waiting for connections...");

    short ret = EXIT_FAILURE;

    thread_array_t *thread_array = thread_array_init(5);

    if (thread_array == NULL)
    {
        goto error;
    }
    log_server("Thread array initialized with capacity %zu", thread_array->capacity);

    size_t nb_client = 0;

    while (1)
    {

        // resize the thread array :
        if (nb_client >= thread_array->capacity)
        {
            log_server("Thread array resized with capacity %zu", thread_array->capacity);
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

        log_server("Accepting a new client..");

        // Accept an incoming connection :
        *client_fd = accept(socket_fd, (struct sockaddr *)&client_addr, &client_addr_len);

        if (*client_fd == -1)
        {
            if(is_terminated) {
                log_server("Server is shutting down, stopping accept loop.");
                free(client_fd);
                break;
            }
            perror("accept server");
            goto error;
        }

        log_server("Client accepted with socket ID %d\n thread in creation..", *client_fd);

        if (pthread_create(&thread_array->threads[nb_client], NULL, handle, client_fd) != 0)
        {
            if(is_terminated) {
                log_server("Server is shutting down, stopping thread creation.");
                ret = EXIT_SUCCESS;
            }
            else
            {
                perror("pthread_create server");
            }
            close(*client_fd);
            free(client_fd);
            goto thread_clean;
        }

        nb_client++;
    }

    ret = EXIT_SUCCESS;

    thread_clean:
    for (size_t i = 0; i < thread_array->count; i++)
    {
        pthread_join(thread_array->threads[i], NULL);
    }

    error:   
    thread_array_destroy(thread_array);
    close(socket_fd);

    exit(ret);
}

void sig_handler(int signum)
{
    log_server("Signal %d received, shutting down server...", signum);
    is_terminated = 1;
}
