#ifndef GROUP_STORAGE_H
#define GROUP_STORAGE_H

#include "utils.h"

#define GROUP_PATH "./grouptest"

/**
 * STRUCTURE DE L'ARBORESCENCE DES GROUPES :
 * 
 * ├── 0
 *     ├── mdiff_addr
 *     ├── group_name
 *     ├── mdiff_port
 *     ├── admin_id
 *     ├__ users/
 *
 */

 
/**
 * @brief Wrapper contenant les informations
 * relative à une adresse de multidiffusion
 * et son port associé
 */
typedef struct diff_wrapper {

    int mdiff_port;
    u8 mdiff_addr[16];
} diff_wrapper_t;

/**
 * @brief Permet d'écrire les informations relatif au groupe pour son enregistrement 
 * dans le fichier 'path'
 * @param id_group l'identifiant du groupe
 * @param group_name le nom du groupe
 * @param mdiff_port le port de multidiffusion
 * @param mdiff_addr l'adresse de multidiffusion
 * 
 * @return -1 si problème d'appel système (voir perror), 0 sinon
 */
int store_group(int id_group, const u8 *group_name, int mdiff_port, const u8 *mdiff_addr);

/**
 * @brief Permet d'ajouter un utilisateur à un groupe
 * en fonction de son id en l'enregistrant dans le
 * répertoire des groupes.
 * 
 * Le répertoire des utilisateurs à l'intérieur d'un groupe est de cette forme:
 *     |__users/
 *             |__ 0
 *             |__ 1
 *             |__ 2
 *             ...
 * 
 * @param user_id l'id de l'utilisateur
 * @param group_id l'id du groupe
 * 
 * @return 0 si succès, -1 si erreur
 */
int add_user_group(int user_id, int group_id);

/**
 * @brief Ajoute l'admin du groupe dans le répertoire du groupe
 * 
 * @param admin_id l'id de l'admin
 * @param group_id l'id du groupe en question
 */
int add_admin_group(int admin_id, int group_id);

/**
 * @brief permet de supprimer un group
 * selon son id
 * 
 * @param group_id l'id du groupe à supprimer
 * 
 * @return 0 si succès, -1 si erreur
 */
int delete_group(int group_id);

/**
 * @brief Permet de supprimer un utilisateur d'un groupe
 * en fonction de son id en l'enregistrant dans le
 * répertoire des groupes
 * 
 * @param user_id l'id de l'utilisateur
 * @param group_id l'id du groupe
 * 
 * @return 0 si succès, -1 si erreur
 */
int delete_user_group(int user_id, int group_id);

/**
 * @brief permet de trouver une adresse IPV6 ainsi
 * qu'un port de multidiffusion de libre en cherchant
 * dans les répertoires des groupes
 * 
 * @return le port et l'adresse trouvé dans un diff_wrapper à libérer, 
 * NULL si erreur
 */
diff_wrapper_t * find_free_mdiff_addr_port(); //TODO utiliser le multithreading

/**
 * @brief Permet de déterminer un identifiant non utilisé. En admettant que le répertoire
 * contient uniquement des sous-répertoires nommés par des entiers positifs.
 * @return -1 si aucun id possible, l'id disponible sinon.
 */
int find_group_id();

#endif