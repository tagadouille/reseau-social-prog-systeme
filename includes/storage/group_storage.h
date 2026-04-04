#ifndef GROUP_STORAGE_H
#define GROUP_STORAGE_H

#include "utils.h"

#define GROUP_PATH "./grouptest"
#define MAX_LEN_PATH 256

/**
 * @brief Permet d'écrire les informations relatif au groupe pour son enregistrement 
 * dans le fichier 'path' :
 * ├── 0
 *     ├── key         ici 0 est l'identifiant de l'utilisateur enregistré
 *     ├── name
 *     └── udp_port
 *
 * @param id_group l'identifiant du groupe
 * @param group_name le nom du groupe
 * @param mdiff_port le port de multidiffusion
 * @param mdiff_addr l'adresse de multidiffusion
 * @param path le chemin de stockage
 * @return -1 si problème d'appel système (voir perror), 0 sinon
 */
int store_group(int id_group, const char *group_name, int mdiff_port, const u8 *mdiff_addr, const char *path);

/**
 * @brief permet de trouver une adresse de multidiffusion libre
 * en cherchant si dans les fichiers un autre groupe ne l'occupe pas déjà
 */
u8 find_free_mdiff_addr();

/**
 * @brief permet de trouver un port de multidiffusion libre
 * en cherchant si dans les fichiers un autre groupe ne l'occupe pas déjà
 */
int find_free_mdiff_port();

#endif