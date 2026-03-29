#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "serialization.h"
#include "utils.h"
#include "request.h"

/*
 * Sérialise l'inscription (CODEREQ = 1)
 */
ssize_t build_register_req(u8 *buf, req_register *req)
{
    // On crée un curseur 'p' qui va se déplacer dans notre buffer 'buf'.
    // Ça permet d'écrire les données à la suite sans écraser le début.
    u8 *p = buf;

    // 1. On prépare le code de la requête.
    // La fonction htons() est obligatoire ici car le protocole exige le
    // format Big-Endian (BE) pour les nombres sur 16 bits.
    u16 req_code = htons(req->req_code);
    memcpy(p, &req_code, sizeof(req_code));

    // On fait avancer notre curseur de 2 octets (la taille d'un u16) pour la suite.
    p += sizeof(req_code);

    // 2. On écrit le pseudo.
    // On sait qu'il fait exactement 10 octets (définis dans la structure).
    size_t len_username = sizeof(req->username);
    memcpy(p, req->username, len_username);
    p += len_username; // Le curseur avance de 10 octets.

    // 3. On écrit la clé publique.
    // Même logique, la taille est fixe (113 octets).
    size_t len_public_key = sizeof(req->pub_key);
    memcpy(p, req->pub_key, len_public_key);
    p += len_public_key;

    // On calcule la distance entre la position finale du curseur et son point
    // de départ pour connaître la taille totale exacte du message à envoyer.
    return (size_t)(p - buf);
}

/*
 * Sérialise la création de groupe (CODEREQ = 3)
 */
ssize_t build_group_req(u8 *buf, req_create_group *req)
{
    u8 *p = buf;

    // 1. On écrit l'entête contenant le code et l'ID utilisateur (16 bits en BE).
    u16 req_code = htons(req->req_code_user_id);
    memcpy(p, &req_code, sizeof(req_code));
    p += sizeof(req_code);

    // 2. On écrit la longueur du nom du groupe.
    // Ici on n'utilise pas htons() car 'name_len' est sur un seul octet (u8).
    // Il n'y a pas d'ordre des octets à inverser pour un octet unique.
    memcpy(p, &req->name_len, sizeof(req->name_len));
    p += sizeof(req->name_len);

    // 3. On écrit les lettres du nom du groupe.
    // On utilise directement la longueur qu'on vient d'envoyer.
    memcpy(p, req->group_name, req->name_len);
    p += req->name_len;

    return (size_t)(p - buf);
}

/*
 * Sérialise l'invitation dans un groupe (CODEREQ = 5)
 */
ssize_t build_invitation_req(u8 *buf, req_invite_group *req)
{
    u8 *p = buf;

    // 1. Entête standard : Code + ID (16 bits en BE).
    u16 req_code = htons(req->req_code_user_id);
    memcpy(p, &req_code, sizeof(req_code));
    p += sizeof(req_code);

    // 2. On écrit le bloc qui combine l'ID du groupe et le nombre d'invités.
    // Tout ce bloc de 16 bits doit passer en Big-Endian.
    u16 group_info = htons(req->group_id_count);
    memcpy(p, &group_info, sizeof(group_info));
    p += sizeof(group_info);

    // 3. Pour faire notre boucle, on a besoin de connaître le nombre d'invités.
    // On utilise un masque MASK_5_BITS pour effacer l'ID du groupe et ne garder
    // que les 5 derniers bits qui correspondent au compteur d'invités.
    int nb_guests = (req->group_id_count & MASK_5_BITS);

    // 4. On écrit l'identifiant de chaque invité un par un.
    for (int i = 0; i < nb_guests; i++)
    {
        // Chaque identifiant est sur 16 bits (ID + ZEROS), donc htons() est requis.
        u16 invited_user_id = htons(req->invited_ids[i]);
        memcpy(p, &invited_user_id, sizeof(invited_user_id));
        p += sizeof(invited_user_id); // Le curseur avance de 2 octets par invité.
    }

    return (size_t)(p - buf);
}

/*
 * Sérialise la demande des invitations en attente (CODEREQ = 6)
 */
ssize_t build_waiting_invitation_list_req(u8 *buf, req_list_invitations *req)
{
    u8 *p = buf;

    // On envoie juste les 16 premiers bits.
    u16 req_code = htons(req->req_code_user_id);
    memcpy(p, &req_code, sizeof(req_code));
    p += sizeof(req_code);

    return (size_t)(p - buf);
}

/*
 * Sérialise l'action sur un groupe (CODEREQ = 8)
 */
ssize_t build_invitation_response_req(u8 *buf, req_group_action *req)
{
    u8 *p = buf;

    // 1. Entête standard.
    u16 req_code = htons(req->req_code_user_id);
    memcpy(p, &req_code, sizeof(req_code));
    p += sizeof(req_code);

    // 2. On écrit le bloc qui contient l'ID du groupe et l'action (accepter/refuser/quitter).
    u16 group_action = htons(req->group_id_action);
    memcpy(p, &group_action, sizeof(group_action));
    p += sizeof(group_action);

    return (size_t)(p - buf);
}

/*
 * Sérialise la demande de la liste des membres (CODEREQ = 10)
 */
ssize_t build_list_members_req(u8 *buf, req_list_members *req)
{
    u8 *p = buf;

    // 1. Entête standard.
    u16 req_code = htons(req->req_code_user_id);
    memcpy(p, &req_code, sizeof(req_code));
    p += sizeof(req_code);

    // 2. On envoie l'ID du groupe demandé (ou 0 pour avoir tout le monde).
    u16 group_id = htons(req->group_id);
    memcpy(p, &group_id, sizeof(group_id));
    p += sizeof(group_id);

    return (size_t)(p - buf);
}

/*
 * Sérialise le postage d'un billet (CODEREQ = 12)
 */
ssize_t build_post_message_req(u8 *buf, req_post_message *req)
{
    u8 *p = buf;

    // 1. Code + ID user.
    u16 req_code = htons(req->req_code_user_id);
    memcpy(p, &req_code, sizeof(req_code));
    p += sizeof(req_code);

    // 2. ID du groupe ciblé.
    u16 group_id = htons(req->group_id);
    memcpy(p, &group_id, sizeof(group_id));
    p += sizeof(group_id);

    // 3. La taille du texte.
    // Contrairement à la création de groupe, la taille est ici sur 16 bits.
    // On doit donc obligatoirement utiliser htons() pour l'envoyer sur le réseau.
    u16 len = htons(req->data_len);
    memcpy(p, &len, sizeof(len));
    p += sizeof(len);

    // 4. Le texte du billet.
    memcpy(p, req->data, req->data_len);
    p += req->data_len;

    return (size_t)(p - buf);
}

/*
 * Sérialise la réponse à un billet (CODEREQ = 14)
 */
ssize_t build_reply_message_req(u8 *buf, req_reply_message *req)
{
    u8 *p = buf;

    // 1. Code + ID user.
    u16 req_code = htons(req->req_code_user_id);
    memcpy(p, &req_code, sizeof(req_code));
    p += sizeof(req_code);

    // 2. ID du groupe.
    u16 group_id = htons(req->group_id);
    memcpy(p, &group_id, sizeof(group_id));
    p += sizeof(group_id);

    // 3. ID du billet auquel on veut répondre.
    u16 ticket_id = htons(req->ticket_id);
    memcpy(p, &ticket_id, sizeof(ticket_id));
    p += sizeof(ticket_id);

    // 4. Taille de notre réponse (sur 16 bits BE).
    u16 len = htons(req->data_len);
    memcpy(p, &len, sizeof(len));
    p += sizeof(len);

    // 5. Texte de la réponse.
    memcpy(p, req->data, req->data_len);
    p += req->data_len;

    return (size_t)(p - buf);
}

/*
 * Sérialise la demande des derniers billets (CODEREQ = 16)
 */
ssize_t build_list_messages_req(u8 *buf, req_list_messages *req)
{
    u8 *p = buf;

    // 1. Code + ID user.
    u16 req_code = htons(req->req_code_user_id);
    memcpy(p, &req_code, sizeof(req_code));
    p += sizeof(req_code);

    // 2. ID du groupe.
    u16 group_id = htons(req->group_id);
    memcpy(p, &group_id, sizeof(group_id));
    p += sizeof(group_id);

    // 3. On envoie nos "points de sauvegarde" au serveur :
    // - NUMB : le numéro du dernier billet qu'on a reçu
    // - NUMR : le numéro de la dernière réponse reçue
    // Le serveur utilisera ces numéros pour envoyer uniquement les nouveaux messages.
    u16 last_ticket_reply_id = htons(req->last_ticket_reply_id);
    memcpy(p, &last_ticket_reply_id, sizeof(last_ticket_reply_id));
    p += sizeof(last_ticket_reply_id);

    return (size_t)(p - buf);
}

// ----------------------- RESPONSES ---------------------------------------

ssize_t build_register_resp(u8 *buf, resp_register *resp);

ssize_t build_group_resp(u8 *buf, resp_create_group *resp);

ssize_t build_generic_ack_resp(u8 *buf, resp_generic_ack *resp);

ssize_t build_error_resp(u8 *buf, resp_error *resp);

ssize_t build_list_invitations_resp(u8 *buf, resp_list_invitations *resp);

ssize_t build_accept_invitation_resp(u8 *buf, resp_accept_invitation *resp);

ssize_t build_list_members_resp(u8 *buf, resp_list_members *resp);

ssize_t build_post_message_resp(u8 *buf, resp_post_message *resp);

ssize_t build_reply_message_resp(u8 *buf, resp_reply_message *resp);

ssize_t build_list_messages_resp(u8 *buf, resp_list_messages *resp);

ssize_t build_group_notification_resp(u8 *buf, resp_group_notification *resp);