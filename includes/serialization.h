#ifndef SERIALIZATION_H
#define SERIALIZATION_H

#include <sys/types.h>
#include <unistd.h>

#include "./request.h"
#include "./response.h"
#include "./utils.h"

/*
 * Ces fonctions sérialisent une requête C dans un buffer réseau.
 * Retour : nombre d'octets écrits dans `buf` (ou -1 en cas d'erreur).
 *
 * Rappel protocole : les `u16` doivent être encodés en ordre réseau (Big Endian).
 */

/*
 * Inscription (CODEREQ = 1).
 * Format encodé :
 *   req_code(5 bits code + 11 bits 0) | username[10] | pub_key[113]
 */
ssize_t build_register_req(u8 *buf, req_register *req);

/*
 * Création de groupe (CODEREQ = 3).
 * Format encodé :
 *   req_code_user_id(5 bits code + 11 bits id user) | name_len | group_name[name_len]
 */
ssize_t build_group_req(u8 *buf, req_create_group *req);

/*
 * Invitation dans un groupe (CODEREQ = 5).
 * Format encodé :
 *   req_code_user_id | group_id_count(11 bits id groupe + 5 bits nb invités)
 *   | invited_ids[nb] (chaque entrée : 11 bits id + 5 bits 0)
 */
ssize_t build_invitation_req(u8 *buf, req_invite_group *req);

/*
 * Demande de liste des invitations en attente (CODEREQ = 6).
 * Format encodé :
 *   req_code_user_id
 */
ssize_t build_waiting_invitation_list_req(u8 *buf, req_list_invitations *req);

/*
 * Réponse à invitation / quitter groupe (CODEREQ = 8).
 * Format encodé :
 *   req_code_user_id | group_id_action(11 bits id groupe + 2 bits action + 3 bits 0)
 */
ssize_t build_invitation_response_req(u8 *buf, req_group_action *req);

/*
 * Liste des membres (CODEREQ = 10).
 * Format encodé :
 *   req_code_user_id | group_id(11 bits id groupe + 5 bits 0)
 *   (si group_id = 0, demande de la liste globale des utilisateurs)
 */
ssize_t build_list_members_req(u8 *buf, req_list_members *req);

/*
 * Publication d'un billet (CODEREQ = 12).
 * Format encodé :
 *   req_code_group_id(5 bits code + 11 bits id groupe)
 *   | user_id_zeros(11 bits id user + 5 bits 0)
 *   | data_len | data[data_len]
 */
ssize_t build_post_message_req(u8 *buf, req_post_message *req);

/*
 * Réponse à un billet (CODEREQ = 14).
 * Format encodé :
 *   req_code_user_id | group_id | ticket_id | data_len | data[data_len]
 *   (group_id et ticket_id : 11 bits valeur + 5 bits 0)
 */
ssize_t build_reply_message_req(u8 *buf, req_reply_message *req);

/*
 * Récupération des derniers billets/réponses (CODEREQ = 16).
 * Format encodé :
 *   req_code_user_id | group_id | ticket_reply_ids(11 bits NUMB + 5 bits NUMR)
 */
ssize_t build_list_messages_req(u8 *buf, req_list_messages *req);


#endif
