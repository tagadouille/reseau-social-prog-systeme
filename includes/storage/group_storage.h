#ifndef GROUP_STORAGE_H
#define GROUP_STORAGE_H

#include "utils.h"

#define GROUP_PATH "./grouptest"

/**
 * @brief Wrapper contenant les informations
 * relative à une adresse de multidiffusion
 * et son port associé
 */
typedef struct diff_wrapper {

    int mdiff_port;
    u8* mdiff_addr;
} diff_wrapper_t;

/**
 * @brief Permet d'écrire les informations relatif au groupe pour son enregistrement 
 * dans le fichier 'path' :
 * ├── 0
 *     ├── mdiff_addr
 *     ├── group_name
 *     └── mdiff_port
 *
 * @param id_group l'identifiant du groupe
 * @param group_name le nom du groupe
 * @param mdiff_port le port de multidiffusion
 * @param mdiff_addr l'adresse de multidiffusion
 * 
 * @return -1 si problème d'appel système (voir perror), 0 sinon
 */
int store_group(int id_group, const u8 *group_name, int mdiff_port, const u8 *mdiff_addr);

/**
 * @brief permet de trouver une adresse IPV6 ainsi
 * qu'un port de multidiffusion de libre en cherchant
 * dans les répertoires des groupes
 * 
 * @return le port et l'adresse trouvé dans un diff_wrapper à libérer, 
 * NULL si erreur
 */
diff_wrapper_t * find_free_mdiff_addr_port(); //TODO utiliser le multithreading

#endif