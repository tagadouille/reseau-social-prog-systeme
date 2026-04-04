#ifndef CREATE_GROUP_H
#define CREATE_GROUP_H

#include "utils.h"
#include "request.h"
#include "response.h"
#include "serialization.h"

/**
 * @brief permet de lire la requête de création de groupe
 * que le client a envoyé et de remplir la structure de requête de création de groupe
 * 
 * @param sock la socket du client
 * @param request la structure de requête à remplir
 * @param buf_header le buffer contenant l'entete de la requete de taille 2
 * 
 * @return 0 en cas de succès, -1 si il y a eu une erreur
 */
int read_create_group(int sock, req_create_group *request, u8* buf_header);

/**
 * @brief permet de lire dans le buffer de réponse du server et
 * de remplir la structure de réponse de création de groupe
 * 
 * @param buf le buffer de réponse
 * @param request la structure de réponse à remplir
 */
void read_rep_create_group(u8 *buf, resp_create_group *response);

#endif