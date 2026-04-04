#ifndef CREATE_GROUP_H
#define CREATE_GROUP_H

#include "utils.h"
#include "request.h"
#include "response.h"
#include "serialization.h"

/**
 * @brief Wrapper contenant les informations
 * relative à une adresse de multidiffusion
 * et son port associé
 */
typedef struct diff_wrapper {

    int mdiff_port;
    u8 mdiff_addr;
} diff_wrapper_t;

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

/**
 * @brief Cherche un port et une adresse IPV6 libre
 * pour la multidiffusion du groupe
 */
multi_wrapper_t multicast_group_find();

#endif