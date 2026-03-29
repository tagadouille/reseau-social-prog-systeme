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

// ----------------------- RESPONSES ---------------------------------------

ssize_t build_register_resp(u8 *buf, resp_register *resp)
{
    u8 *p = buf;

    u16 resp_code = htons(resp->resp_code_user_id);
    memcpy(p, &resp_code, sizeof(resp_code));

    p += sizeof(resp_code);

    u16 udp_port = htons(resp->udp_port);
    memcpy(p, &udp_port, sizeof(udp_port));

    p += sizeof(udp_port);

    size_t len_public_key = sizeof(resp->server_pub_key);
    memcpy(p, resp->server_pub_key, len_public_key);
    p += len_public_key;

    return (size_t)(p - buf);
}

ssize_t build_group_resp(u8 *buf, resp_create_group *resp)
{
    u8 *p = buf;

    u16 resp_code = htons(resp->resp_code_group_id);
    memcpy(p, &resp_code, sizeof(resp_code));

    p += sizeof(resp_code);

    u16 mdiff_port = htons(resp->mdiff_port);
    memcpy(p, &mdiff_port, sizeof(mdiff_port));

    p += sizeof(mdiff_port);

    size_t len_mdiff_ipv6 = sizeof(resp->mdiff_ipv6);
    memcpy(p, resp->mdiff_ipv6, len_mdiff_ipv6);
    p += len_mdiff_ipv6;

    return (size_t)(p - buf);
}

ssize_t build_generic_ack_resp(u8 *buf, resp_generic_ack *resp)
{
    u8 *p = buf;

    return (size_t)(p - buf);
}

ssize_t build_error_resp(u8 *buf, resp_error *resp)
{
    u8 *p = buf;

    return (size_t)(p - buf);
}

ssize_t build_list_invitations_resp(u8 *buf, resp_list_invitations *resp)
{
    u8 *p = buf;

    return (size_t)(p - buf);
}

ssize_t build_accept_invitation_resp(u8 *buf, resp_accept_invitation *resp)
{
    u8 *p = buf;

    return (size_t)(p - buf);
}

ssize_t build_list_members_resp(u8 *buf, resp_list_members *resp)
{
    u8 *p = buf;

    return (size_t)(p - buf);
}

ssize_t build_post_message_resp(u8 *buf, resp_post_message *resp)
{
    u8 *p = buf;

    return (size_t)(p - buf);
}

ssize_t build_reply_message_resp(u8 *buf, resp_reply_message *resp)
{
    u8 *p = buf;

    return (size_t)(p - buf);
}

ssize_t build_list_messages_resp(u8 *buf, resp_list_messages *resp)
{
    u8 *p = buf;

    return (size_t)(p - buf);
}

ssize_t build_group_notification_resp(u8 *buf, resp_group_notification *resp)
{
    u8 *p = buf;

    return (size_t)(p - buf);
}

#endif
