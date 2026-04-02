#include <stdint.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>

#include "../../includes/client_handler.h"
#include "../../includes/utils.h"
#include "../../includes/protocol.h"
#include "../../includes/request.h"
#include "../../includes/register.h"
#include "../../includes/user_storage.h"

void *handle(void *arg)
{
	int sock = *(int *)arg;

	u8 buf[1024];
	memset(buf, 0, sizeof(buf));

	int codereq = read_codereq(sock, buf);
	if (codereq < 0)
	{
		perror("Erreur lecture CODEREQ");
		goto cleanup;
	}

	printf("CODEREQ reçu : %d\n", codereq);

	int ret;
	switch (codereq)
	{
        case REQ_REGISTER:
			ret = handle_register(sock, buf);
			break;
		case REQ_CREATE_GROUP:
			ret = handle_create_group(sock, buf);
			break;

        default:
		perror("CODEREQ inconnu");
		ret = -1;
		break;
	}

	if (ret < 0)
		perror("Erreur dans le handler");

cleanup:
	close(sock);
	return NULL;

}

/**
 * Fonction pour gérer le cas d'une requête d'inscription, en deux étapes on lit et on reconstruit la requête dans 'buf_header' sur le serveur,
 * On la traite et on produit une réponse en suivant le protocole pour l'envoyer au client 'sock'
 * RETURN VALUE: -1 si problème de lecture ou construction de la requête et réponse sinon 0.
 */
int handle_register(int sock, u8 *buf_header)
{
	int remaining = SIZE_REQ_REGISTER - 2;

	u8 rest[remaining];
	memset(rest, 0, sizeof(rest));

	if (recv_all(sock, (char *)rest, remaining) < 0)
	{
	        perror("erreur lecture corps register");
		return -1;
	}

	u8 full_buf[SIZE_REQ_REGISTER];
	memcpy(full_buf, buf_header, 2);
	memcpy(full_buf + 2, rest, remaining);

	req_register request;
	memset(&request, 0, sizeof(request));
	read_register(full_buf, &request);

	printf("Inscription de : %s\n", request.username);

	int user_id = find_id(USER_PATH);

	int r = store_user(user_id,(char *) request.username, PORT_UDP, (char *)request.pub_key, USER_PATH);
	if (r == -1)
	{
		perror("error store user");
		return -1;
	}

	resp_register response;
	memset(&response, 0, sizeof(response));
	prepare_register_resp(&response, user_id, PORT_UDP);

	u8 resp_buf[SIZE_RESP_REGISTER];
	memset(resp_buf, 0, sizeof(resp_buf));

	ssize_t len = build_register_resp(resp_buf, &response);
	if (len < 0)
	{
		perror("erreur construction réponse register");
		return -1;
	}

	if (send_all(sock, (char *)resp_buf, len) < 0)
	{
		perror("erreur envoi réponse register");
		return -1;
	}

	printf("Réponse envoyée : ID=%d, UDP_PORT=%d\n", user_id, 12580);
	return 0;
}

/**
 * @brief Fonction pour gérer le cas d'une requête de création de groupe, en deux étapes
 * on lit et on reconstruit la requête dans 'buf_header' sur le serveur,
 * On la traite et on produit une réponse en suivant le protocole pour l'envoyer au client 'sock'
 * RETURN VALUE: -1 si problème de lecture ou construction de la requête et réponse sinon 0.
 */
int handle_create_group(int sock, u8 *buf_header)
{
	int remaining = SIZE_REQ_REGISTER - 2;

	u8 rest[remaining];
	memset(rest, 0, sizeof(rest));

	if (recv_all(sock, (char *)rest, remaining) < 0)
	{
	    perror("erreur lecture corps création groupe");
		return -1;
	}

	u8 full_buf[SIZE_REQ_REGISTER];
	memcpy(full_buf, buf_header, 2);
	memcpy(full_buf + 2, rest, remaining);

	req_register request;
	memset(&request, 0, sizeof(request));
	read_register(full_buf, &request); // TODO CREATE GROUP

	printf("Création du groupe de : %s\n", request.username);

	// Stockage du groupe :

	int user_id = find_id(USER_PATH);

	int r = store_user(user_id,(char *) request.username, PORT_UDP, (char *)request.pub_key, USER_PATH);
	if (r == -1)
	{
		perror("error store user");
		return -1;
	}

	// Préparation de la réponse :

	resp_create_group response;
	memset(&response, 0, sizeof(response));
	prepare_group_resp(&response, int group_id, int mdiff_port, const u8 *mdiff_addr);

	u8 resp_buf[SIZE_RESP_CREATE_GROUP];
	memset(resp_buf, 0, sizeof(resp_buf));

	ssize_t len = build_group_resp(resp_buf, &response);

	if (len < 0)
	{
		perror("erreur construction réponse création groupe");
		return -1;
	}

	if (send_all(sock, (char *)resp_buf, len) < 0)
	{
		perror("erreur envoi réponse création groupe");
		return -1;
	}

	printf("Réponse envoyée : ID=%d, UDP_PORT=%d\n", user_id, 12580);
	return 0;
}
