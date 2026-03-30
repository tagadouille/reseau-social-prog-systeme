#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>

#include "../../includes/client_handler.h"
#include "../../includes/utils.h"
#include "../../includes/protocol.h"
#include "../../includes/request.h"
#include "../../includes/register.h"

#define PORT_UDP 12580

void handle_register(handle_wrapper_t wrapper, u16 buf_code);

void *handle(void *arg)
{
    handle_wrapper_t wrapper = *(handle_wrapper_t *)arg;
    free(arg);

    int client_sock = wrapper.client_sock;

    u16 buf_code;
    recv(client_sock, &buf_code, 2, 0);

    switch ((buf_code & MASK_5_BITS) << 11)
    {
    case REQ_REGISTER : handle_register(wrapper, buf_code);
    }
    
    close(client_sock);
    return NULL;
}

void handle_register(handle_wrapper_t wrapper, u16 buf_code)
{
	int client_socket = wrapper.client_sock;
	
	u8 *buf;
	buf = malloc(SIZE_REQ_REGISTER);
	if (!buf)
	{
		perror("malloc buf");
		return;
	}

	memcpy(buf, &buf_code, 2);
	
	recv_all(client_socket, (char *)buf, SIZE_REQ_REGISTER);
	req_register *request;
	request = malloc(sizeof(req_register));
	if (!request)
	{
		perror("malloc request");
		return;
	}
	
	printf("Message du client: %s\n", buf);
	read_register(buf, request);

	resp_register *response;
	response = malloc(sizeof(resp_register));
	if (!response)
	{
		perror("malloc response");
		return;
	}
	
	prepare_register_resp(response, 0, PORT_UDP);
	memset(buf, 0, SIZE_REQ_REGISTER);
	build_register_resp(buf, response);

	printf("Message envoyé: %s\n", buf);
	
	sendto(client_socket,
	       buf,
	       SIZE_RESP_REGISTER,
	       0,
	       (struct sockaddr *)wrapper.client_addr,
	       wrapper.len);
}
