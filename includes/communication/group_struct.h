#ifndef GROUP_STRUCT_H
#define GROUP_STRUCT_H

#include "utils.h"

/**
 * @brief Structure représentant un groupe
 * 
 * Elle contient divers information comme le nom du groupe
 * son id, son port et IPV6 multicast, l'id de l'administrateur
 * ainsi que l'id des membres
 * 
 * La liste des membres peut être NULL si il n'y a aucun membre
 * mais juste l'admin
 */
typedef struct group
{
    int group_id;
    u8 *group_name;

    int id_admin;

    int *members_id;
    size_t members_id_len;

    int mdiff_port;
    u8 mdiff_addr[16];
} group_t;

/**
 * @brief permet de créer et de renvoyer une structure group_t
 * 
 * @param group_id l'id du groupe
 * @param group_name le nom du groupe
 * @param id_admin l'id de l'administrateur
 * 
 * @return la structure group_t, NULL si erreur
 */
group_t * group_initialize(int group_id, const u8 *group_name, int id_admin);

/**
 * @brief permet de créer et de renvoyer une structure group_t
 * 
 * @param group_id l'id du groupe
 * @param group_name le nom du groupe
 * @param id_admin l'id de l'administrateur
 * @param members_id le tableau contenant l'ensemble des membres du groupe
 * @param members_id_len la longueur du tableau contenant l'ensemble des membres du groupe
 * @param mdiff_port le port de multidiffusion
 * @param mdiff_addr l'adresse de multidiffusion
 * 
 * @return la structure group_t, NULL si erreur
 */
group_t * group_fill(int group_id, const u8 *group_name, int id_admin, int *members_id, size_t members_id_len, int mdiff_port, u8 mdiff_addr[16]);

/**
 * @brief permet de créer et de renvoyer une structure group_t
 * en lisant les fichiers contenant les informations relatif au groupe.
 * 
 * La fonction echoue si toutes les informations sauf la liste des membres
 * sont introuvables, elle informera de ça dans les logs du server
 * 
 * @param group_id
 * 
 * @return la structure group_t, NULL si erreur
 */
group_t *group_fill_from_files(int group_id);

/**
 * @brief libére la structure de groupe
 * 
 * @param group la structure de group à libérer
 */
void group_struct_destroy(group_t* group);

#endif