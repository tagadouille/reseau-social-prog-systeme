#ifndef REQUEST_H
#define REQUEST_H

#include "./utils.h"

// Masques pour limiter la taille des données (le nombre de bits à 1)
#define MASK_11_BITS 0x07FF // Binaire: 0000 0111 1111 1111

#define MASK_5_BITS 0x1F // Binaire: 0000 0000 0001 1111

/* =========================================================================
 * RÈGLE GLOBALE : Tous les champs numériques (u16) doivent être convertis
 * en Big Endian (ordre réseau) avec htons() avant d'être copiés dans le buffer.
 * Ce travail est censé être réalisé dans le fichier serialization.c
 * ========================================================================= */

/* CODEREQ = 1
 * Inscription utilisateur
 * -> envoie pseudo (10 max) + clé publique (113 octets)
 */
typedef struct
{
   u16 req_code;    // 5 bits CODEREQ + 11 bits à 0
   u8 username[10]; // pseudo (complété avec '\0' si <10)
   u8 pub_key[113]; // clé publique (format PEM)
} req_register;

/* CODEREQ = 3
 * Création d’un groupe
 * -> envoie le nom du groupe (taille variable)
 */
typedef struct
{
   u16 req_code_user_id; // 5 bits CODEREQ + 11 bits ID user
   u8 name_len;          // longueur du nom
   u8 *group_name;       // nom du groupe (taille name_len)
} req_create_group;

/* CODEREQ = 5
 * Invitation dans un groupe
 * -> ID du groupe + liste des utilisateurs invités
 */
typedef struct
{
   u16 req_code_user_id; // 5 bits CODEREQ + 11 bits ID user
   u16 group_id_count;   // 11 bits ID groupe + 5 bits nb invités
   u16 *invited_ids;     // tableau de nb IDs (11 bits ID + 5 bits à 0 chacun)
} req_invite_group;

/* CODEREQ = 6
 * Demande des invitations reçues
 * -> récupère les invitations en attente de l'utilisateur
 */
typedef struct
{
   u16 req_code_user_id; // 5 bits CODEREQ + 11 bits ID user
} req_list_invitations;

/* CODEREQ = 8
 * Répondre à une invitation / quitter un groupe
 * -> action : 1 = accepter, 0 = refuser, 2 = quitter
 */
typedef struct
{
   u16 req_code_user_id; // 5 bits CODEREQ + 11 bits ID user
   u16 group_id_action;  // 11 bits ID groupe + 2 bits action + 3 bits à 0
} req_group_action;

/* CODEREQ = 10
 * Liste des membres
 * -> si group_id = 0 : tous les utilisateurs
 */
typedef struct
{
   u16 req_code_user_id; // 5 bits CODEREQ + 11 bits ID user
   u16 group_id;         // 11 bits ID groupe + 5 bits à 0
} req_list_members;

/* CODEREQ = 12
 * Poster un billet
 * -> envoie un message dans un groupe
 */
typedef struct
{
   u16 req_code_group_id; // 5 bits CODEREQ + 11 bits ID GROUPE
   u16 user_id_zeros;     // 11 bits ID user + 5 bits à 0
   u16 data_len;          // taille du message
   u8 *data;              // contenu du billet
} req_post_message;

/* CODEREQ = 14
 * Répondre à un billet
 * -> réponse à un message existant
 */
typedef struct
{
   u16 req_code_user_id; // 5 bits CODEREQ + 11 bits ID user
   u16 group_id;         // 11 bits ID groupe + 5 bits à 0
   u16 ticket_id;        // 11 bits numéro du billet + 5 bits à 0
   u16 data_len;         // taille de la réponse
   u8 *data;             // contenu de la réponse
} req_reply_message;

/* CODEREQ = 16
 * Récupérer les billets récents
 * -> à partir du dernier billet/réponse reçus
 */
typedef struct
{
   u16 req_code_user_id; // 5 bits CODEREQ + 11 bits ID user
   u16 group_id;         // 11 bits ID groupe + 5 bits à 0
   u16 ticket_reply_ids; // 11 bits NUMB + 5 bits NUMR
} req_list_messages;

void prepare_register_req(req_register *req, const char *username);

void prepare_group_req(req_create_group *req, int ID, const char *NOMG);

void prepare_invitation_req(req_invite_group *req, int ID, int ID_group, int guest_number, u16 *guest_ids);

void prepare_waiting_invitation_list_req(req_list_invitations *req);

void prepare_invitation_response_req(req_group_action *req);

void prepare_list_members_req(req_list_members *req);

void prepare_post_message_req(req_post_message *req);

void prepare_reply_message_req(req_reply_message *req);

void prepare_list_messages_req(req_list_messages *req);

#endif