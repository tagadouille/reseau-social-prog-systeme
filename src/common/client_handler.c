#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
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
#include "communication/create_group.h"
#include "communication/group_struct.h"
#include "storage/group_storage.h"
#include "log.h"

/**
 * @brief Fonction pour gérer le cas d'une requête de création de groupe, en deux étapes
 * on lit et on reconstruit la requête dans 'buf_header' sur le serveur,
 * On la traite et on produit une réponse en suivant le protocole pour l'envoyer au client 'sock'
 * @return -1 si problème de lecture ou construction de la requête et réponse sinon 0.
 */
static int handle_create_group(int sock, u8 *buf_header)
{
	log_server("[handle_create_group]");

	// Création de la requete de création de groupe
	req_create_group request;
	memset(&request, 0, sizeof(request));

	// Structure group utiliser pour les logs et avoir l'admin
	group_t *group = read_create_group(sock, &request, buf_header);

	if (group == NULL)
	{
		return -1;
	}

	log_server("[handle_create_group] Creation of the group of the user %d will start", group->id_admin);

	// Stockage du groupe :

	// Trouver un id de groupe libre :
	int group_id = find_id(GROUP_PATH);

	log_server("[handle_create_group] Group id found : %d, let's find a free port and address", group_id);

	// Trouver un port et une adresse de libre :
	diff_wrapper_t *wrapper = find_free_mdiff_addr_port();

	if (wrapper == NULL)
	{
		group_struct_destroy(group);
		return -1;
	}

	int mdiff_port = wrapper->mdiff_port;
	u8 *mdiff_addr = wrapper->mdiff_addr;

	free(wrapper);

	log_server("[handle_create_group] Group id found : %d, let's store the group", group_id);

	int r = store_group(group_id, request.group_name, mdiff_port, mdiff_addr);
	if (r == -1)
	{
		perror("error store user");
		group_struct_destroy(group);
		return -1;
	}

	log_server("[handle_create_group] Storing the group finish. Storing the admin");

	r = add_admin_group(group->id_admin, group_id);

	if (r < 0)
	{
		delete_group(group_id);
		log_server("Adding the user failed, deleting the group..");
		group_struct_destroy(group);
		return -1;
	}
	group_struct_destroy(group);

	log_server("[handle_create_group] Storing of the admin finish, create response");

	// Préparation de la réponse :

	resp_create_group response;
	memset(&response, 0, sizeof(response));
	prepare_group_resp(&response, group_id, mdiff_port, mdiff_addr);

	u8 resp_buf[SIZE_RESP_CREATE_GROUP];
	memset(resp_buf, 0, sizeof(resp_buf));

	ssize_t len = build_group_resp(resp_buf, &response);

	if (len < 0)
	{
		perror("erreur construction réponse création groupe");
		return -1;
	}

	log_server("[handle_create_group] Sending the response");

	if (send_all(sock, (char *)resp_buf, len) < 0)
	{
		perror("erreur envoi réponse création groupe");
		return -1;
	}

	char *addr = IPV6_addr_to_string(mdiff_addr);
	log_server("[handle_create_group]  Réponse envoyée : GROUP_ID=%d, PORT=%d ADDR=%s\n", group_id, mdiff_port, addr);
	free(addr);
	return 0;
}

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

	log_server("CODEREQ reçu : %d\n", codereq);

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

	int r = store_user(user_id, (char *)request.username, PORT_UDP, (char *)request.pub_key, USER_PATH);
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
