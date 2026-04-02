#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "client_handler.h"

void *handle(void *arg)
{
    int client_socket = *(int *)arg;
    free(arg);

    // TODO: Handle client communication here

    close(client_socket);
    return NULL;
}