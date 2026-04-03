#ifndef RESPONSE_H
#define RESPONSE_H

#include "utils.h"

/* =========================================================================
 * RÈGLE GLOBALE : Tous les champs numériques (u16) sont sérialisés en
 * Big Endian (ordre réseau) via htons()/ntohs() dans serialization.c.
 * Les champs composites (ex: code + id) sont documentés ci-dessous.
 * ========================================================================= */

/* CODEREQ = 2
 * Réponse d'inscription utilisateur
 * -> renvoie ID utilisateur + port UDP de notifications + clé publique serveur
 */
typedef struct
{
	u16 resp_code_user_id; // 5 bits CODEREQ + 11 bits ID utilisateur
	u16 udp_port;		   // port UDP de réception des notifications
	u8 server_pub_key[113];
} resp_register;

/* CODEREQ = 4
 * Réponse de création de groupe
 * -> renvoie ID du groupe + infos de multidiffusion
 */
typedef struct
{
	u16 resp_code_group_id; // 5 bits CODEREQ + 11 bits ID groupe
	u16 mdiff_port;			// port de multidiffusion
	u8 mdiff_ipv6[16];		// adresse IPv6 de multidiffusion
} resp_create_group;

/* CODEREQ = 24
 * Réponse de validation simple (ACK)
 * Utilisée pour : confirmer une invitation, une sortie de groupe, etc.
 */
typedef struct
{
	u16 resp_header; // 5 bits CODEREQ (24) + 11 bits à 0 (ZEROS)
} resp_generic_ack;

/* Élément invitation groupe
 * -> ID groupe + longueur nom + nom du groupe + nom administrateur
 */
typedef struct
{
	u16 group_id_name_len; // 11 bits IDG + 5 bits LEN (nom du groupe)
	u8 *group_name;		   // nom du groupe (taille LEN)
	u8 admin_name[10];	   // pseudo administrateur (10 octets)
} resp_group_invitation_info;

/* CODEREQ = 7
 * Liste des invitations en attente
 * -> NB invitations + tableau d'invitations
 */
typedef struct
{
	u16 resp_code_count;					 // 5 bits CODEREQ + 11 bits NB
	resp_group_invitation_info *invitations; // tableau de taille NB
} resp_list_invitations;

/* Élément membre de groupe
 * -> ID utilisateur + nom
 * Il faut transmettre cette information à tout les membres d'un groupe et l'admin dès qu'un utilisateur rejoint ou quitte un groupe
 */
typedef struct
{
	u16 user_id; // 11 bits ID + 5 bits à 0
	u8 name[10]; // pseudo utilisateur (10 octets)
} resp_user_info;

/* CODEREQ = 9
 * Invitation acceptée
 * -> infos groupe (multidiffusion) + liste des membres
 */
typedef struct
{
	u16 resp_code_group_id;	 // 5 bits CODEREQ + 11 bits IDG
	u16 mdiff_port;			 // port de multidiffusion
	u8 mdiff_ipv6[16];		 // adresse IPv6 de multidiffusion
	u16 member_count;		 // nombre de membres
	resp_user_info *members; // tableau de taille member_count
} resp_accept_invitation;

/* CODEREQ = 11
 * Liste des membres d'un groupe (ou tous utilisateurs si group_id = 0 côté requête)
 */
typedef struct
{
	u16 resp_code_group_id;	 // 5 bits CODEREQ + 11 bits IDG
	u16 member_count;		 // nombre de membres
	resp_user_info *members; // tableau de taille member_count
} resp_list_members;

/* CODEREQ = 13
 * Accusé de création de billet
 * -> ID groupe + numéro de billet attribué
 */
typedef struct
{
	u16 resp_code_group_id; // 5 bits CODEREQ + 11 bits IDG
	u16 ticket_id;			// 11 bits NUMB + 5 bits à 0
} resp_post_message;

/* CODEREQ = 15
 * Accusé de réponse à un billet
 * -> ID groupe + couple (NUMB, NUMR)
 */
typedef struct
{
	u16 resp_code_group_id; // 5 bits CODEREQ + 11 bits IDG
	u16 ticket_reply_id;	// 11 bits NUMB + 5 bits NUMR
} resp_reply_message;

/* Élément billet/réponse
 * -> auteur + couple (NUMB, NUMR) + longueur + données
 */
typedef struct
{
	u16 author_id;		 // 11 bits ID auteur + 5 bits à 0
	u16 ticket_reply_id; // 11 bits NUMB + 5 bits NUMR (NUMR=0 si billet)
	u16 data_len;		 // longueur de data en octets
	u8 *data;			 // contenu du billet/réponse
} resp_message_info;

/* CODEREQ = 17
 * Historique des billets/réponses d'un groupe
 * -> ID groupe + NB éléments + tableau des billets
 */
typedef struct
{
	u16 resp_code_group_id;				// 5 bits CODEREQ + 11 bits IDG
	u16 message_count;					// nombre d'éléments envoyés
	resp_message_info *message_history; // tableau de taille message_count
} resp_list_messages;

/* CODEREQ = 31
 * Erreur générique
 */
typedef struct
{
	u16 resp_code; // 5 bits CODEREQ + 11 bits à 0
} resp_error;

/* Notifications serveur (CODEREQ = 18..23)
 * 18: nouveau billet/réponse, 19: invitation acceptée,
 * 20: membre quitte, 21: groupe fermé,
 * 22: nouvelle invitation reçue, 23: billet récupéré.
 */
typedef struct
{
	u16 notif_code_group_id; // 5 bits CODEREQ + 11 bits IDG
} resp_group_notification;

/*
 * Taille des reponses du serveur pour savoir combien d'octets on doit lire pour une requête précise
 */

#define SIZE_RESP_REGISTER 117

/* Fonctions de préparation des réponses */
void prepare_register_resp(resp_register *resp, int user_id, int udp_port);

void prepare_group_resp(resp_create_group *resp, int group_id, int mdiff_port, const u8 *mdiff_addr);

void prepare_ack_resp(resp_generic_ack *resp);

/* CODEREQ = 7 : liste des invitations */
void init_list_invitations_resp(resp_list_invitations *resp, int count);

void set_invitation_item_resp(resp_list_invitations *resp, int index, int group_id, u8 name_len, const u8 *group_name, const u8 *admin_name);

/* CODEREQ = 9 : invitation acceptée */
void init_accept_invitation_resp(resp_accept_invitation *resp, int group_id, u16 mdiff_port, const u8 *mdiff_ipv6, int member_count);

void set_accept_invitation_member(resp_accept_invitation *resp, int index, int member_id, const u8 *member_name);

/* CODEREQ = 11 : liste des membres */
void init_list_members_resp(resp_list_members *resp, int group_id, int member_count);

void set_list_member_resp(resp_list_members *resp, int index, int member_id, const u8 *member_name);

/* CODEREQ = 13 et 15 : accusés de billet/réponse */
void prepare_post_message_resp(resp_post_message *resp, int group_id, int ticket_id);

void prepare_reply_message_resp(resp_reply_message *resp, int group_id, int ticket_id, int reply_id);

/* CODEREQ = 17 : historique des billets */
void init_list_messages_resp(resp_list_messages *resp, int group_id, int message_count);

void set_list_message_item(resp_list_messages *resp, int index, int author_id, int ticket_id, int reply_id, const u8 *data, u16 data_len);

/* CODEREQ = 18..23 : notifications serveur */
void prepare_notification_resp(resp_group_notification *resp, int notif_code, int group_id);

#endif
