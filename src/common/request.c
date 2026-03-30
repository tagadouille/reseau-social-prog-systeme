#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "request.h"
#include "protocol.h"

/*
 * Ce fichier prépare les structures de requêtes côté client.
 *
 * Important : ici on "compose" les champs (code + ids + tailles),
 * mais la conversion en ordre réseau (htons) sera faite au moment
 * de la sérialisation dans serialization.c.
 */

/*
 * Prépare la requête d'inscription.
 * - req_code : 5 bits de code, 11 bits à 0
 * - username : pseudo sur 10 octets max
 * - pub_key : pour l'instant mise à zéro (clé non gérée ici)
 */
void prepare_register_req(req_register *req, const char *username)
{
    // 5 bits CODEREQ + 11 bits à 0
    req->req_code = (REQ_REGISTER & MASK_5_BITS) << 11;

    // Copie du pseudo, le reste est complété par des zéros automatiquement grâce à strncpy
    strncpy((char *)req->username, username, sizeof(req->username));

    // TODO: S'occuper de la clé plus tard
    memset(req->pub_key, 0, sizeof(req->pub_key));
}

/*
 * Prépare la requête de création de groupe.
 * Le champ req_code_user_id packe : [CODEREQ sur 5 bits | ID user sur 11 bits].
 *
 * Mémoire : group_name est alloué ici, il devra être libéré par l'appelant.
 */
void prepare_group_req(req_create_group *req, int ID, const char *NOMG)
{
    // Masque de sécurité : on garde seulement 11 bits pour l'ID utilisateur.
    req->req_code_user_id = ((REQ_CREATE_GROUP & MASK_5_BITS) << 11) | (ID & MASK_11_BITS);

    req->name_len = (u8)strlen(NOMG);

    req->group_name = malloc(req->name_len); // TODO: Ne pas oublier le free()
    if (req->group_name == NULL)
    {
        perror("Memory Allocation failed in prepare_group_req function.");
        return;
    }

    memcpy(req->group_name, NOMG, req->name_len);
}

/*
 * Prépare la requête d'invitation d'utilisateurs dans un groupe.
 * - req_code_user_id : [code 5 bits | id user 11 bits]
 * - group_id_count   : [id groupe 11 bits | nb invités 5 bits]
 * - invited_ids[i]   : [id invité 11 bits | 5 bits à 0]
 *
 * Mémoire : invited_ids est alloué ici, à libérer plus tard.
 */
void prepare_invitation_req(req_invite_group *req, int ID, int ID_group, int guest_number, u16 *guest_ids)
{
    req->req_code_user_id = ((REQ_INVITE & MASK_5_BITS) << 11) | (ID & MASK_11_BITS);

    req->group_id_count = ((ID_group & MASK_11_BITS) << 5) | (guest_number & MASK_5_BITS);

    req->invited_ids = malloc(sizeof(u16) * guest_number); // TODO: Ne pas oublier le free()
    if (req->invited_ids == NULL)
    {
        perror("Memory Allocation failed in prepare_invitation_req function.");
        return;
    }

    for (int i = 0; i < guest_number; i++)
    {
        req->invited_ids[i] = ((guest_ids[i] & MASK_11_BITS) << 5);
    }
}

/*
 * Prépare la requête qui demande les invitations en attente d'un utilisateur.
 * Format : [code 5 bits | id user 11 bits]
 */
void prepare_waiting_invitation_list_req(req_list_invitations *req, int ID)
{
    req->req_code_user_id = ((REQ_GET_INVITES & MASK_5_BITS) << 11) | (ID & MASK_11_BITS);
}

/*
 * Prépare la requête d'action sur invitation.
 * answer est codé sur 2 bits :
 * - 1 : accepter
 * - 0 : refuser
 * - 2 : quitter
 *
 * group_id_action : [id groupe 11 bits | answer 2 bits | 3 bits à 0]
 */
void prepare_invitation_response_req(req_group_action *req, int ID, int ID_group, int answer)
{
    req->req_code_user_id = ((REQ_GROUP_ACTION & MASK_5_BITS) << 11) | (ID & MASK_11_BITS);
    req->group_id_action = ((ID_group & MASK_11_BITS) << 5) | ((answer & MASK_2_BITS) << 3);
}

/*
 * Prépare la requête de liste des membres d'un groupe.
 * Si ID_group vaut 0, le protocole peut être interprété comme "liste globale".
 */
void prepare_list_members_req(req_list_members *req, int ID, int ID_group)
{
    req->req_code_user_id = ((REQ_GET_MEMBERS & MASK_5_BITS) << 11) | (ID & MASK_11_BITS);
    req->group_id = ((ID_group & MASK_11_BITS) << 5);
}

/*
 * Prépare la requête de publication d'un billet.
 * - data_len est la longueur exacte du message (sans '\0').
 * - data contient les octets du message.
 *
 * Mémoire : data est alloué ici, à libérer par l'appelant.
 */
void prepare_post_message_req(req_post_message *req, int ID, int ID_group, const char *msg)
{
    req->req_code_user_id = ((REQ_POST_TICKET & MASK_5_BITS) << 11) | (ID & MASK_11_BITS);
    req->group_id = ((ID_group & MASK_11_BITS) << 5);
    req->data_len = (u16)strlen(msg);
    req->data = malloc(req->data_len); // TODO: Ne pas oublier le free()
    if (req->data == NULL)
    {
        perror("Memory Allocation failed in prepare_post_message_req function.");
        return;
    }

    memcpy(req->data, msg, req->data_len);
}

/*
 * Prépare la requête de réponse à un billet existant.
 * ticket_id encode le numéro du billet ciblé sur 11 bits.
 *
 * Mémoire : data est alloué ici, à libérer par l'appelant.
 */
void prepare_reply_message_req(req_reply_message *req, int ID, int ID_group, int ID_ticket, const char *msg)
{
    req->req_code_user_id = ((REQ_REPLY_TICKET & MASK_5_BITS) << 11) | (ID & MASK_11_BITS);
    req->group_id = ((ID_group & MASK_11_BITS) << 5);
    req->ticket_id = ((ID_ticket & MASK_11_BITS) << 5);

    req->data_len = (u16)strlen(msg);
    req->data = malloc(req->data_len); // TODO: Ne pas oublier le free()
    if (req->data == NULL)
    {
        perror("Memory Allocation failed in prepare_reply_message_req function.");
        return;
    }

    memcpy(req->data, msg, req->data_len);
}

/*
 * Prépare la requête de récupération des derniers billets/réponses.
 * last_ticket_reply_id : [numéro dernier billet 11 bits | numéro dernière réponse 5 bits].
 */
void prepare_list_messages_req(req_list_messages *req, int ID, int ID_group, int ID_last_ticket, int ID_last_reply)
{
    req->req_code_user_id = ((REQ_GET_LAST_TICKET & MASK_5_BITS) << 11) | (ID & MASK_11_BITS);
    req->group_id = ((ID_group & MASK_11_BITS) << 5);
    req->last_ticket_reply_id = ((ID_last_ticket & MASK_11_BITS) << 5) | (ID_last_reply & MASK_5_BITS);
}
