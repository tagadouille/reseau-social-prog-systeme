#ifndef CLIENT_HANDLER_H
#define CLIENT_HANDLER_H


#include <sys/socket.h>
typedef struct handle_wrapper
{
	struct sockaddr_in6 *client_addr;
	int client_sock;
	socklen_t len;
} handle_wrapper_t;

void * handle(void * arg);

#endif
