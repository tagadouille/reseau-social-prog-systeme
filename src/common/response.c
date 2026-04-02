#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "response.h"
#include "protocol.h"

/*
 * ============================================================================
 * RÔLE : Ce fichier contient les fonctions permettant de préparer les structures
 * de réponses envoyées par le SERVEUR aux CLIENTS.
 * IMPORTANT :
 * - Ces fonctions "packent" les bits (ex: 5 bits pour le code, 11 bits pour l'ID).
 * - La conversion en format réseau (Big Endian avec htons) n'est PAS faite ici.
 * Elle devra être réalisée lors de la sérialisation (dans serialization.c).
 * - Certaines fonctions allouent de la mémoire avec malloc(). Le serveur devra
 * obligatoirement libérer (free) cette mémoire après l'envoi du message pour
 * éviter les fuites.
 * ============================================================================
 */

/*
 * Fonction utilitaire interne (privée).
 * Initialise les informations d'un membre d'un groupe.
 * Format : ID de l'utilisateur sur 11 bits (suivi de 5 bits à 0) + Nom sur 10 octets.
 */
static void fill_user_info(resp_user_info *user, int member_id, const u8 *member_name)
{
    if (user == NULL)
        return;

    // On décale l'ID de 5 crans vers la gauche pour réserver 5 bits à 0 à la fin.
    // Visuellement : [ID(11 bits)] devient [ID(11 bits) | 00000].
    user->user_id = (member_id & MASK_11_BITS) << 5;

    // Remplit la zone du nom avec des zéros par précaution
    size_t name_size = sizeof(user->name);
    memset(user->name, 0, name_size);

    if (member_name == NULL)
        return;

    // On copie au maximum 10 octets pour rester compatible avec le format fixe du protocole.
    size_t name_len = strnlen((const char *)member_name, name_size);
    memcpy(user->name, member_name, name_len);
}

/*
 * CODEREQ = 2 : Réponse à l'inscription d'un utilisateur.
 * Le serveur attribue un identifiant unique (user_id) et un port UDP.
 *
 * Contexte protocole : cette réponse est envoyée après la requête d'inscription (code 1).
 */
void prepare_register_resp(resp_register *resp, int user_id, int udp_port)
{
    if (resp == NULL)
        return;

    // On décale CODEREQ de 11 crans vers la gauche pour lui donner les 5 bits de tête.
    // Puis on insère l'ID utilisateur dans les 11 bits restants.
    // Résultat final : [CODEREQ=2 sur 5 bits | user_id sur 11 bits].
    resp->resp_code_user_id = ((RES_REGISTER & MASK_5_BITS) << 11) | (user_id & MASK_11_BITS);
    resp->udp_port = udp_port;

    // TODO: La clé publique du serveur (ED25519) n'est pas gérée dans la version simple.
    // On remplit le champ avec 113 octets de zéros.
    memset(resp->server_pub_key, 0, sizeof(resp->server_pub_key));
}

/*
 * CODEREQ = 4 : Réponse à la création d'un nouveau groupe.
 * Le serveur attribue un identifiant de groupe, un port et une IP de multidiffusion.
 *
 * Contexte protocole : cette réponse est envoyée après une demande de création (code 3).
 */
void prepare_group_resp(resp_create_group *resp, int group_id, int mdiff_port, const u8 *mdiff_addr)
{
    if (resp == NULL)
        return;

    // Pack : CODEREQ 4 (5 bits) | ID Groupe (11 bits)
    resp->resp_code_group_id = ((RES_CREATE_GROUP & MASK_5_BITS) << 11) | (group_id & MASK_11_BITS);
    resp->mdiff_port = mdiff_port;

    if (mdiff_addr != NULL)
    {
        memcpy(resp->mdiff_ipv6, mdiff_addr, sizeof(resp->mdiff_ipv6));
    }
    else
    {
        memset(resp->mdiff_ipv6, 0, sizeof(resp->mdiff_ipv6));
    }
}

/*
 * CODEREQ = 24 : Acquittement simple (ACK).
 * Utilisé par le serveur pour confirmer :
 * - l'envoi des invitations (réponse au Code 5)
 * - le refus d'une invitation (réponse au Code 8, AN=0)
 * - la sortie volontaire d'un groupe (réponse au Code 8, AN=2)
 */
void prepare_ack_resp(resp_generic_ack *resp)
{
    if (resp == NULL)
        return;

    // Format : [CODEREQ=24 | 00000000000]
    resp->resp_header = ((RES_ACK & MASK_5_BITS) << 11);
}

/*
 * CODEREQ = 31 : Le serveur signale une erreur au client (requête mal formée, groupe inconnu...).
 *
 * Cette réponse est générique : elle ne contient pas de détail d'erreur, uniquement le code 31.
 */
void prepare_error_resp(resp_error *resp)
{
    if (resp == NULL)
        return;
    // Pack : CODEREQ 31 (5 bits) | 11 bits à 0
    resp->resp_code = ((RES_ERROR & MASK_5_BITS) << 11);
}

/*
 * CODEREQ = 7 : Réponse à la demande des invitations en attente.
 * ÉTAPE 1/2 : Initialise l'en-tête et alloue le tableau d'invitations.
 *
 * count = nombre d'invitations à envoyer (0 si aucune invitation).
 */
void init_list_invitations_resp(resp_list_invitations *resp, int count)
{
    if (resp == NULL)
        return;

    // On place CODEREQ=7 à gauche (<< 11), puis count à droite (11 bits).
    // Format : [CODEREQ=7 | NB invitations]
    resp->resp_code_count = ((RES_GET_INVITES & MASK_5_BITS) << 11) | (count & MASK_11_BITS);

    if (count <= 0)
    {
        resp->invitations = NULL;
        return;
    }

    // [ATTENTION MÉMOIRE] Allocation d'un tableau dynamique d'invitations.
    // L'appelant devra faire free(resp->invitations) après envoi/sérialisation.
    resp->invitations = malloc(count * sizeof(resp_group_invitation_info));
}

/*
 * CODEREQ = 7 : Réponse à la demande des invitations en attente.
 * ÉTAPE 2/2 : Remplit une invitation spécifique dans le tableau précédemment alloué.
 *
 * index = position dans le tableau (entre 0 et count-1).
 * name_len = longueur du nom de groupe sur 5 bits (donc max 31).
 */
void set_invitation_item_resp(resp_list_invitations *resp, int index, int group_id, u8 name_len, const u8 *group_name, const u8 *admin_name)
{
    if (resp == NULL || resp->invitations == NULL || index < 0)
        return;

    resp_group_invitation_info *group_invitation = &(resp->invitations[index]);

    // Ici c'est l'inverse de d'habitude :
    // - ID groupe est placé dans les 11 bits de gauche (<< 5),
    // - name_len occupe les 5 bits de droite.
    // Format : [IDG(11 bits) | LEN(5 bits)].
    group_invitation->group_id_name_len = ((group_id & MASK_11_BITS) << 5) | (name_len & MASK_5_BITS);

    // [ATTENTION MÉMOIRE] Le nom du groupe est alloué dynamiquement (taille variable).
    // L'appelant devra free(group_invitation->group_name) pour chaque invitation non NULL.
    group_invitation->group_name = NULL;
    if (name_len > 0)
    {
        group_invitation->group_name = malloc(name_len);
        if (group_invitation->group_name != NULL && group_name != NULL)
        {
            memcpy(group_invitation->group_name, group_name, name_len);
        }
    }

    // L'administrateur a un nom de taille fixe (10 octets), complété par des zéros.
    size_t admin_name_size = sizeof(group_invitation->admin_name);
    memset(group_invitation->admin_name, 0, admin_name_size);

    if (admin_name != NULL)
    {
        size_t admin_len = strnlen((const char *)admin_name, admin_name_size);
        memcpy(group_invitation->admin_name, admin_name, admin_len);
    }
}

/*
 * CODEREQ = 9 : Réponse quand un utilisateur ACCEPTE une invitation.
 * ÉTAPE 1/2 : Initialise l'en-tête (infos multidiffusion) et alloue le tableau des membres.
 *
 * member_count = nombre de membres du groupe.
 * Rappel protocole : le membre à l'index 0 devra être l'administrateur.
 */
void init_accept_invitation_resp(resp_accept_invitation *resp, int group_id, u16 mdiff_port, const u8 *mdiff_ipv6, int member_count)
{
    if (resp == NULL)
        return;

    // Format en-tête : [CODEREQ=9 | ID groupe].
    resp->resp_code_group_id = ((RES_GROUP_ACTION_OK & MASK_5_BITS) << 11) | (group_id & MASK_11_BITS);
    resp->mdiff_port = mdiff_port;

    if (mdiff_ipv6 != NULL)
    {
        memcpy(resp->mdiff_ipv6, mdiff_ipv6, sizeof(resp->mdiff_ipv6));
    }
    else
    {
        memset(resp->mdiff_ipv6, 0, sizeof(resp->mdiff_ipv6));
    }

    if (member_count <= 0)
    {
        resp->member_count = 0;
        resp->members = NULL;
        return;
    }

    resp->member_count = (u16)member_count;
    // [ATTENTION MÉMOIRE] Allocation du tableau des membres.
    // L'appelant devra faire free(resp->members).
    resp->members = malloc((size_t)resp->member_count * sizeof(resp_user_info));
    if (resp->members == NULL)
    {
        resp->member_count = 0;
    }
}

/*
 * CODEREQ = 9 : Réponse quand un utilisateur ACCEPTE une invitation.
 * ÉTAPE 2/2 : Ajoute un membre au tableau.
 * IMPORTANT : Le premier membre (index 0) doit toujours être l'administrateur du groupe.
 */
void set_accept_invitation_member(resp_accept_invitation *resp, int index, int member_id, const u8 *member_name)
{
    if (resp == NULL || resp->members == NULL || index < 0)
        return;
    if ((u16)index >= resp->member_count)
        return;

    fill_user_info(&(resp->members[index]), member_id, member_name);
}

/*
 * CODEREQ = 11 : Réponse donnant la liste des membres d'un groupe (ou tous les inscrits).
 * ÉTAPE 1/2 : Initialise l'en-tête et alloue le tableau des membres.
 *
 * group_id peut valoir 0 selon le protocole côté requête (liste globale).
 */
void init_list_members_resp(resp_list_members *resp, int group_id, int member_count)
{
    if (resp == NULL)
        return;

    // Format en-tête : [CODEREQ=11 | ID groupe].
    resp->resp_code_group_id = ((RES_GET_MEMBERS & MASK_5_BITS) << 11) | (group_id & MASK_11_BITS);

    if (member_count <= 0)
    {
        resp->member_count = 0;
        resp->members = NULL;
        return;
    }

    resp->member_count = (u16)member_count;
    // [ATTENTION MÉMOIRE] Allocation du tableau des membres.
    // L'appelant devra faire free(resp->members).
    resp->members = malloc((size_t)resp->member_count * sizeof(resp_user_info));
    if (resp->members == NULL)
    {
        resp->member_count = 0;
    }
}

/*
 * CODEREQ = 11 : Réponse donnant la liste des membres d'un groupe (ou tous les inscrits).
 * ÉTAPE 2/2 : Ajoute un membre au tableau.
 */
void set_list_member_resp(resp_list_members *resp, int index, int member_id, const u8 *member_name)
{
    if (resp == NULL || resp->members == NULL || index < 0)
        return;
    if ((u16)index >= resp->member_count)
        return;

    fill_user_info(&(resp->members[index]), member_id, member_name);
}

/*
 * CODEREQ = 13 : Accusé de réception après avoir posté un NOUVEAU billet.
 * Le serveur confirme que le billet a été ajouté et lui attribue un numéro.
 *
 * ticket_id correspond à NUMB (11 bits), les 5 bits de droite restent à 0.
 */
void prepare_post_message_resp(resp_post_message *resp, int group_id, int ticket_id)
{
    if (resp == NULL)
        return;

    // Pack : CODEREQ 13 (5 bits) | ID Groupe (11 bits)
    resp->resp_code_group_id = ((RES_POST_TICKET & MASK_5_BITS) << 11) | (group_id & MASK_11_BITS);
    // On décale NUMB de 5 crans pour obtenir : [NUMB(11 bits) | 00000].
    resp->ticket_id = (ticket_id & MASK_11_BITS) << 5;
}

/*
 * CODEREQ = 15 : Accusé de réception après avoir RÉPONDU à un billet.
 * Le serveur confirme l'ajout et renvoie le numéro du billet original (NUMB) et le numéro de la réponse (NUMR).
 *
 * reply_id est codé sur 5 bits.
 * Rappel : dans l'historique (code 17), reply_id = 0 signifie "billet original" (pas une réponse).
 */
void prepare_reply_message_resp(resp_reply_message *resp, int group_id, int ticket_id, int reply_id)
{
    if (resp == NULL)
        return;

    // Pack : CODEREQ 15 (5 bits) | ID Groupe (11 bits)
    resp->resp_code_group_id = ((RES_REPLY_TICKET & MASK_5_BITS) << 11) | (group_id & MASK_11_BITS);
    // Pack : Numéro du billet NUMB (11 bits) | Numéro de la réponse NUMR (5 bits)
    resp->ticket_reply_id = ((ticket_id & MASK_11_BITS) << 5) | (reply_id & MASK_5_BITS);
}

/*
 * CODEREQ = 17 : Réponse contenant l'historique des billets du fil d'un groupe.
 * ÉTAPE 1/2 : Initialise l'en-tête et alloue le tableau des messages.
 *
 * message_count = nombre total d'éléments (billets + réponses) envoyés.
 */
void init_list_messages_resp(resp_list_messages *resp, int group_id, int message_count)
{
    if (resp == NULL)
        return;

    // Pack : CODEREQ 17 (5 bits) | ID Groupe (11 bits)
    resp->resp_code_group_id = ((RES_GET_TICKETS & MASK_5_BITS) << 11) | (group_id & MASK_11_BITS);

    if (message_count <= 0)
    {
        resp->message_count = 0;
        resp->message_history = NULL;
        return;
    }

    resp->message_count = (u16)message_count;
    // [ATTENTION MÉMOIRE] Allocation du tableau des messages d'historique.
    // L'appelant devra faire free(resp->message_history).
    resp->message_history = malloc((size_t)resp->message_count * sizeof(resp_message_info));
    if (resp->message_history == NULL)
    {
        resp->message_count = 0;
    }
}

/*
 * CODEREQ = 17 : Réponse contenant l'historique des billets du fil d'un groupe.
 * ÉTAPE 2/2 : Ajoute un billet ou une réponse au tableau.
 *
 * reply_id :
 * - 0  => billet original
 * - >0 => réponse numéro reply_id au billet ticket_id
 */
void set_list_message_item(resp_list_messages *resp, int index, int author_id, int ticket_id, int reply_id, const u8 *data, u16 data_len)
{
    if (resp == NULL || resp->message_history == NULL || index < 0)
        return;
    if ((u16)index >= resp->message_count)
        return;

    resp_message_info *item = &(resp->message_history[index]);

    // Pack : ID de l'auteur (11 bits) | 5 bits à 0
    item->author_id = (author_id & MASK_11_BITS) << 5;
    // Pack : Numéro du billet NUMB (11 bits) | Numéro de la réponse NUMR (5 bits) (NUMR=0 si c'est un billet original)
    item->ticket_reply_id = ((ticket_id & MASK_11_BITS) << 5) | (reply_id & MASK_5_BITS);
    item->data_len = data_len;

    // [ATTENTION MÉMOIRE] Le contenu data est alloué dynamiquement pour cet item.
    // L'appelant devra faire free(item->data) pour chaque item non NULL.
    item->data = NULL;
    if (data_len > 0)
    {
        item->data = malloc(data_len);
        if (item->data != NULL && data != NULL)
        {
            memcpy(item->data, data, data_len);
        }
    }
}

/*
 * CODEREQ = 18 à 23 : Notifications envoyées par le serveur de façon spontanée.
 * Prépare l'en-tête d'une notification (multidiffusion ou UDP selon le code).
 *
 * notif_code attendu :
 * - 18 : nouveau billet/réponse
 * - 19 : utilisateur ayant accepté une invitation
 * - 20 : utilisateur ayant quitté le groupe
 * - 21 : groupe fermé par l'admin
 * - 22 : nouvelle invitation reçue
 * - 23 : billet lu/récupéré
 */
void prepare_notification_resp(resp_group_notification *resp, int notif_code, int group_id)
{
    if (resp == NULL)
        return;

    // Pack : CODEREQ de la notification (5 bits) | ID Groupe (11 bits)
    resp->notif_code_group_id = ((notif_code & MASK_5_BITS) << 11) | (group_id & MASK_11_BITS);
}