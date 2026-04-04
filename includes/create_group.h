#ifndef CREATE_GROUP_H
#define CREATE_GROUP_H

#include "utils.h"
#include "request.h"
#include "response.h"
#include "serialization.h"

/**
 * @brief permet de lire dans le buffer de requête et
 * de remplir la structure de requête de création de groupe
 * 
 * @param buf le buffer de requête
 * @param request la structure de requête à remplir
 */
void read_create_group(u8 *buf, req_create_group *request);

/**
 * @brief permet de lire dans le buffer de réponse du server et
 * de remplir la structure de réponse de création de groupe
 * 
 * @param buf le buffer de réponse
 * @param request la structure de réponse à remplir
 */
void read_rep_create_group(u8 *buf, resp_create_group *response);

#endif