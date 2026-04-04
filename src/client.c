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
#include "../includes/create_group.h"

req_register *prepare_request(u8 name[10], u8 key[113]);
ssize_t generate_register_request(u8 *buf, char name[10]);
void test_register(int sock);
ssize_t generate_create_group_request(u8 *buf, int ID, const char* NOMG);

int main(int argc, char *argv[])
{
	// Prise en charge temporaire des arguments :
	if(argc != 2)
	{
		dprintf(2, "Il faut mettre un argument. Soit g pour créer un groupe, soit u pour un utilisateur\n");
	}
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
	
	if(strcmp(argv[1], "u") == 0)
	{
		test_register(sock);
	}
	else if(strcmp(argv[1], "g") == 0)
	{
		test_create_group(sock);
	}
	else{
		dprintf(2, "Argument invalide, c'est soit g pour créer un groupe, soit u pour un utilisateur.\n");
	}
	
	close(sock);
	return 0;
}

/**
 * Permet de créer la requête pour qu'un client s'inscrive, 'buf' est la chaine ou on met la requête et 'name' le nom de l'utilisateur.
 * RETURN VALUE : -1 si problème de malloc ou build_register_req sinon renvoit la taille du message.
 */
ssize_t generate_register_request(u8 *buf, char name[10])
{
        /* Création de la requête */

	req_register *request = malloc(sizeof(req_register));

	if (!request)
	{
		perror("malloc request");
		return -1;
	}

	prepare_register_req(request, name);

	ssize_t len = build_register_req(buf, request);
	if (len < 0)
	{
		perror("build_register_req");
		free(buf);
		free(request);
		return -1;
	}

	/* Log pour voir la requête envoyé */
	int fd = open("./bin/output_inscription.bin", O_CREAT | O_WRONLY | O_TRUNC, 0666);
	write(fd, buf, SIZE_REQ_REGISTER);
	close(fd);
	return len;
}

/**
 * Permet de créer la requête pour qu'un client créer un groupe, 'buf' est la chaine ou on met la requête et 'name' le nom de l'utilisateur.
 * RETURN VALUE : -1 si problème de malloc ou build_register_req sinon renvoit la taille du message.
 */
ssize_t generate_create_group_request(u8 *buf, int ID, const char* NOMG)
{
    /* Création de la requête */

	req_create_group *request = malloc(sizeof(req_create_group));

	if (!request)
	{
		perror("malloc request");
		return -1;
	}

	prepare_group_req(request, ID, NOMG);

	ssize_t len = build_group_req(buf, request);
	if (len < 0)
	{
		perror("build_group_request_req");
		free(buf);
		free(request);
		return -1;
	}

	/* Log pour voir la requête envoyé */
	int fd = open("./bin/output_group_create.bin", O_CREAT | O_WRONLY | O_TRUNC, 0666);
	write(fd, buf, len);
	close(fd);
	return len;
}

/**
 * @brief test temporaire de l'inscription d'un client
 */
void test_register(int sock)
{
	char name[10];
	snprintf(name, sizeof(name), "testuser");

	u8 *send_buf = malloc(SIZE_REQ_REGISTER);
	if (!send_buf) {
		perror("malloc send_buf");
		exit(1);
	}

	ssize_t len = generate_register_request(send_buf, name);

	printf("Requete : %s\n", send_buf);
	printf("Envoi de la requête d'inscription (%zd octets)\n", len);
	send_all(sock, (char *)send_buf, len);
	free(send_buf);

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
		perror("recv_all");
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
	printf("  ID attribuée  : %d\n", user_id);
	printf("  Port UDP     : %d\n", udp_port);
	free(response);
	free(recv_buf);
}

void test_create_group(int sock)
{
	int ID = 1;

	u8 *send_buf = malloc(SIZE_REQ_CREATE_GROUP);

	if (!send_buf) {
		perror("malloc send_buf");
		exit(1);
	}

	ssize_t len = generate_create_group_request(send_buf, ID, "testgroup");

	printf("Requete : %s\n", send_buf);
	printf("Envoi de la requête d'inscription (%zd octets)\n", len);
	send_all(sock, (char *)send_buf, len);
	free(send_buf);

	/* Reception et affichage de la réponse du serveur */
	u8 *recv_buf = malloc(SIZE_RESP_CREATE_GROUP);
	if (!recv_buf) {
		perror("malloc recv_buf");
		close(sock);
		exit(1);
	}
	memset(recv_buf, 0, SIZE_RESP_CREATE_GROUP);

	printf("En attente de la réponse du serveur...\n");
	if (recv_all(sock, (char *)recv_buf, SIZE_RESP_CREATE_GROUP) < 0)
	{
		perror("recv_all");
		free(recv_buf);
		close(sock);
		exit(1);
	}

	resp_create_group *response = malloc(sizeof(resp_create_group));
	if (!response) {
		perror("malloc response");
		free(recv_buf);
		close(sock);
		exit(1);
	}

	read_rep_create_group(recv_buf, response);

	int group_id  = response->resp_code_group_id & MASK_11_BITS;
	int mdiff_port = response->mdiff_port;

	printf("=== Réponse du serveur ===\n");
	printf("  ID de groupe attribuée  : %d\n", group_id);
	printf("  Port de MULTIDIFFUSION     : %d\n", mdiff_port);
	free(response);
	free(recv_buf);
}