#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include <string.h>
#include <pthread.h>

#include "server.h"

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

    if(socket_fd == -1) {
        perror("socket server");
        exit(EXIT_FAILURE);
    }

    // Set socket options to allow reuse of the address and port :
    if(setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) == -1) {
        perror("setsockopt server");
        close(socket_fd);
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in6 server_addr;

    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin6_family = AF_INET6;
    server_addr.sin6_addr = in6addr_any;
    server_addr.sin6_port = htons(PORT);

    // Bind the socket to the specified port :
    if(bind(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind server");
        goto error;
    }

    // Listen for incoming connections :
    if(listen(socket_fd, 0) == -1) {
        perror("listen server");
        goto error;
    }

    return socket_fd;

    error:
    close(socket_fd);
    exit(EXIT_FAILURE);
}

int main() {

    int socket_fd = server_config();
    
    short ret = EXIT_FAILURE;

    thread_array_t thread_array = {
        count : 0,
        capacity : 10,
        threads : NULL
    };

    thread_array.threads = malloc(thread_array.capacity * sizeof(pthread_t));

    if(thread_array.threads == NULL) {
        perror("malloc thread array");
        goto error;
    }

    while(1) {

        struct sockaddr_in6 client_addr;
        socklen_t client_addr_len = sizeof(client_addr);

        // Accept an incoming connection :
        int client_fd = accept(socket_fd, (struct sockaddr *) &client_addr, &client_addr_len);

        if(client_fd == -1) {
            perror("accept server");
            goto error;
        }

        // Close the client socket after handling the connection
        close(client_fd);
    }


    ret = EXIT_SUCCESS;

    error:
    close(socket_fd);

    exit(ret);
}