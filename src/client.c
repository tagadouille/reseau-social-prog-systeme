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
	/* Initialisation de la socket client */
	int sock;
	sock = socket(PF_INET6, SOCK_STREAM, 0);
	if (sock < 0) exit(1);

	struct sockaddr_in6 address_sock;
	memset(&address_sock, 0, sizeof(address_sock));
	address_sock.sin6_family = AF_INET6;
	address_sock.sin6_port = htons(PORT);
	inet_pton(AF_INET6, ADRESSE_IPV6, &address_sock.sin6_addr);

	if (connect(sock, (struct sockaddr *)&address_sock, sizeof(address_sock)) == -1)
	{
		perror("echec de la connexion");
		close(sock);
		exit(1);
	}

        /* Création de la requête */
	char name[10];
	snprintf(name, sizeof(name), "testuser");

	req_register *request = malloc(sizeof(req_register));
	if (!request)
	{
		perror("malloc request");
		exit(1);
	}

	prepare_register_req(request, name);

	u8 *send_buf = malloc(SIZE_REQ_REGISTER);
	if (!send_buf) {
		perror("malloc send_buf");
		free(request);
		exit(1);
	}

	ssize_t len = build_register_req(send_buf, request);
	if (len < 0)
	{
		perror("build_register_req");
		free(send_buf);
		free(request);
		close(sock);
		exit(1);
	}

	/* Log pour voir la requête envoyé */
	int fd = open("./bin/output_inscription.bin", O_CREAT | O_WRONLY | O_TRUNC, 0666);
	write(fd, send_buf, SIZE_REQ_REGISTER);
	close(fd);

	printf("Requete : %s\n", send_buf);
	printf("Envoi de la requête d'inscription (%zd octets)\n", len);
	send_all(sock, (char *)send_buf, len);
	free(send_buf);
	free(request);

	/* Reception et affichage de la réponse du serveur */
	u8 *recv_buf = malloc(SIZE_RESP_REGISTER);
	if (!recv_buf) {
		perror("malloc recv_buf");
		close(sock);
		exit(1);
	}
	memset(recv_buf, 0, SIZE_RESP_REGISTER);

	printf("En attente de la réponse du serveur...\n");
	if (recv_all(sock, (char *)recv_buf, SIZE_RESP_REGISTER) < 0)
	{
		fprintf(stderr, "Erreur lors de la réception\n");
		free(recv_buf);
		close(sock);
		exit(1);
	}

	resp_register *response = malloc(sizeof(resp_register));
	if (!response) {
		perror("malloc response");
		free(recv_buf);
		close(sock);
		exit(1);
	}

	read_rep_register(recv_buf, response);

	int user_id  = response->resp_code_user_id & MASK_11_BITS;
	int udp_port = response->udp_port;

	printf("=== Réponse du serveur ===\n");
	printf("  ID attribué  : %d\n", user_id);
	printf("  Port UDP     : %d\n", udp_port);

	free(response);
	free(recv_buf);
	close(sock);
	return 0;
}
