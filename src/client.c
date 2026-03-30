#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>

#include "../includes/utils.h"
#include "../includes/server.h"
#include "../includes/register.h"

req_register *prepare_request(u8 name[10], u8 key[113]);

int main()
{
	int sock;

	sock = socket(PF_INET6, SOCK_STREAM, 0);
	if (sock < 0) exit(1);

	struct sockaddr_in6 address_sock;
	memset(&address_sock, 0,sizeof(address_sock));
	address_sock.sin6_family = AF_INET6;
	address_sock.sin6_port = htons(PORT);
	inet_pton(AF_INET6, ADRESSE_IPV6, &address_sock.sin6_addr);

	int r;
	r = connect(sock,(struct sockaddr *) &address_sock, sizeof(address_sock));
	if(r == -1)
	{
		perror("echec de la connexion");
		close(sock);
		exit(1);
	}

	
	char name[10];
	snprintf(name, 10, "testuser");

	u8 *buf;
	buf = malloc(SIZE_REQ_REGISTER);
	if (!buf)
	{
		perror("Malloc buf");
		exit(1);
	}

	req_register *request;
	request = malloc(sizeof(req_register));
	if (!request)
	{
		perror("malloc request");
		exit(1);
	}
	
	prepare_register_req(request, name);
        ssize_t len = build_register_req(buf, request);
	if (len < 0)
	{
		perror("Request prep");
		close(sock);
		exit(1);
	}

	printf("Requête envoyée:AAA %s\n", buf);
	send_all(sock, (char *)buf, len);

	recv_all(sock, (char *)buf, SIZE_RESP_REGISTER);

	//printf("Réponse reçu (size %d): %s\n",n, buf);

	close(sock);
	return 0;
}
