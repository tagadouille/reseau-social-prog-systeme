#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "response.h"
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
void prepare_register_resp(resp_register *resp, int ID, int UDP_PORT)
{
    resp->resp_code_user_id = ((RES_REGISTER & MASK_5_BITS) << 11) | (ID & MASK_11_BITS);
    resp->udp_port = UDP_PORT;

    // TODO: S'occuper plus tard de la clé publique du serveur
    memset(resp->server_pub_key, 0, sizeof(resp->server_pub_key));
}

/*
 * Prépare la requête de création de groupe.
 * Le champ req_code_user_id packe : [CODEREQ sur 5 bits | ID user sur 11 bits].
 *
 * Mémoire : group_name est alloué ici, il devra être libéré par l'appelant.
 */
void prepare_group_resp(resp_create_group *resp, int ID_group, int MDIFF_PORT, const u8 *MDIFF_ADDR)
{
    resp->resp_code_group_id = ((RES_CREATE_GROUP & MASK_5_BITS) << 11) | (ID_group & MASK_11_BITS);
    resp->mdiff_port = MDIFF_PORT;

    memcpy(resp->mdiff_ipv6, MDIFF_ADDR, sizeof(resp->mdiff_ipv6));
}

/*
 * Prépare la requête d'invitation d'utilisateurs dans un groupe.
 * - req_code_user_id : [code 5 bits | id user 11 bits]
 * - group_id_count   : [id groupe 11 bits | nb invités 5 bits]
 * - invited_ids[i]   : [id invité 11 bits | 5 bits à 0]
 *
 * Mémoire : invited_ids est alloué ici, à libérer plus tard.
 */
void prepare_invitation_resp(resp_generic_ack *resp)
{
    resp->resp_header = ((RES_ACK & MASK_5_BITS) << 11);
}

void init_reponse_invitations(resp_list_invitations *resp, int count)
{
    // CODEREQ = 7 (5 bits) | NB = count (11 bits)
    resp->resp_code_count = ((RES_GET_INVITES & MASK_5_BITS) << 11) | (count & MASK_11_BITS);

    if (count == 0)
    {
        resp->invitations = NULL;
        return;
    }

    // Allocation : on multiplie par la taille de la STRUCTURE
    resp->invitations = malloc(count * sizeof(resp_group_invitation_info));
}

void remplir_une_invitation(resp_list_invitations *resp, int index, int id_g, u8 len, const u8 *nom, const u8 *admin)
{
    // On récupère l'adresse de la case spécifique
    resp_group_invitation_info *group_invitation = &(resp->invitations[index]);

    // Format : IDG (11 bits) | LEN (5 bits)
    group_invitation->group_id_name_len = ((id_g & MASK_11_BITS) << 5) | (len & MASK_5_BITS);

    // Allocation pour le nom du groupe (taille variable LEN)
    group_invitation->group_name = malloc(len);
    memcpy(group_invitation->group_name, nom, len);

    // Nom de l'admin en taille fixe (complété par des zéros)
    size_t admin_name_size = sizeof(group_invitation->admin_name);
    memset(group_invitation->admin_name, 0, admin_name_size);

    size_t admin_len = strnlen((const char *)admin, admin_name_size);
    memcpy(group_invitation->admin_name, admin, admin_len);
}

void prepare_invitation_response_resp()
{
}

void prepare_list_members_resp()
{
}

void prepare_post_message_resp()
{
}

void prepare_reply_message_resp()
{
}

void prepare_list_messages_resp()
{
}
