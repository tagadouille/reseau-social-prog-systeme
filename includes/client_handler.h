#ifndef CLIENT_HANDLER_H
#define CLIENT_HANDLER_H

#include <sys/socket.h>
#include "utils.h"

#define PORT_UDP 12580

void * handle(void * arg);

int handle_register(int sock, u8 *buf_header);

#endif
