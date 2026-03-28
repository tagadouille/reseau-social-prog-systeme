#ifndef RESPONSE_H
#define RESPONSE_H

#include "utils.h"

/**
   CODEREQ = 2,
ID est l’identifiant unique de l’utilisateur·ice attribué par le serveur,
PORTUDP est le numéro de port sur lequel le client recevra les notifications UDP du serveur,
CLE correspond à la clef publique du serveur sous forme d’une chaîne de caractères
 */
typedef struct response_inscription
{
	u16 codereq_id; // codereq sur 5bits le reste pour l'id, A METTRE EN BE
	u16 port_udp;	// A METTRE EN BE
	u8 cle[113];
} resp_inscription;

/**
   CODEREQ = 4,
IDG est l’identifiant unique strictement positif du groupe attribué par le serveur,
PORTMDIFF est le numéro de port de multidiffusion du groupe attribué par le serveur,
IPMDIFF est un entier de 16 octets au format réseaux correspondant à l’adresse IPv6 de
multidiffusion unique du groupe attribuée par le serveur.
 */
typedef struct response_new_grp
{
	u16 codereq_id; // codereq sur 5bits le reste pour l'id, A METTRE EN BE
	u16 portmdiff;	// A METTRE EN BE
	u8 ipmdiff[16];
} resp_new_grp;

/**
   CODEREQ = 24
 */
typedef struct response_inv_grp
{
	u16 codereq; // codereq sur 5bits le reste à 0, A METTRE EN BE
} resp_inv_grp;

/**
   identifiant du groupe (IDG), la longueur en
octet du nom du groupe (LEN), le nom du groupe (NOMG) et le nom de l’administrateur·ice
du groupe (NOM).
 */
typedef struct info_group
{
	u16 idg_len; // idg sur 11bits, le reste pour len, A METTRE EN BE
	u8 *nom_grp; // taille len
	u8 nom[10];	 // admin du groupe
} info_group;

/**
   CODEREQ = 7,
NB est le nombre d’invitations envoyées au client (qui peut valoir 0),
puis pour chaque invitation sont envoyées, l’identifiant du groupe (IDGi), la longueur en
octet du nom du groupe (LENi), le nom du groupe (NOMGi) et le nom de l’administrateur·ice
du groupe (NOMi).
 */
typedef struct response_list_inv_grp
{
	u16 codereq_nb;	  // codereq sur 5bits le reste pour nb, A METTRE EN BE
	info_group *grps; // taille nb
} resp_list_inv;

/**
   identifiant (ID) et son nom sur 10octets (NOM).
 */
typedef struct info_person
{
	u16 id; // id sur 11 bits le rest à 0, A METTRE EN BE
	u8 nom[10];
} info_person;

/**
   CODEREQ = 9,
IDG est l’identifiant du groupe,
PORTMDIFF est le numéro du port de multidiffusion du groupe,
IPMDIFF est un entier de 16 octets au format réseaux correspondant à l’adresse IPv6 de
multidiffusion,
NB est le nombre de membres du groupe,
puis pour chacun de ces membres sont envoyés, son identifiant (IDi) et son nom sur 10
octets (NOMi). Les premiers identifiant et nom doivent être celui de l’administrateur·ice du
groupe.
 */
typedef struct response_inv_accept_grp
{
	u16 codereq_idg;		  // codereq sur 5bits le reste pour l'idg, A METTRE EN BE
	u16 portmdiff;			  // A METTRE EN BE
	u8 ipmdiff[16];			  // A METTRE EN BE
	u16 nb_personne;		  // A METTRE EN BE
	info_person *list_person; // taille nb_personne
} resp_inv_accept_grp;

/**
   CODEREQ = 24
 */
typedef struct response_quit_decline_grp
{
	u16 codereq; // codereq sur 5bits le reste à 0, A METTRE EN BE
} resp_quit_decl_grp;

/**
   CODEREQ = 11,
IDG est l’identifiant du groupe,
NB est le nombre de membres du groupe,
puis pour chacun de ces membre sont envoyés, son identifiant (IDi) et son nom sur 10
octets (NOMi). Les premiers identifiant et nom doivent être celui de l’administrateur·ice du
groupe.
 */
typedef struct response_list_grp
{
	u16 codereq_idg;		  // codereq sur 5bits le reste pour l'idg, A METTRE EN BE
	u16 nb_personne;		  // A METTRE EN BE
	info_person *list_person; // taille nb_personne
} resp_list_grp;

/**
   CODEREQ = 13,
IDG est l’identifiant du groupe,
NUMB est le numéro du billet attribué par le serveur. Les billets d’un groupe sont numérotées
de façon incrémentale à partir de 0.
 */
typedef struct response_billet
{
	u16 codereq_idg; // codereq sur 5bits le reste pour l'idg, A METTRE EN BE
	u16 numb;		 // numb sur 11 bits, le reste à 0, A METTRE EN BE
} resp_billet;

/**
   CODEREQ = 15,
IDG est l’identifiant du groupe,
NUMB est le numéro du billet auquel l’utilisateur·ice répond,
NUMR est le numéro de la réponse. Les réponses à un billet sont numérotées de façon
incrémentale à partir de 1
 */
typedef struct response_rep_billet
{
	u16 codereq_idg; // codereq sur 5bits le reste pour l'idg, A METTRE EN BE
	u16 numb_numr;	 // numb sur 11 bits, le reste pour numr, A METTRE EN BE
} resp_rep_billet;

/**
   identifiant de l’auteur du billet ou de la réponse
(ID), le numéro du billet (NUMB), le numéro de la réponse (NUMR) qui vaut zéro si ce n’est
pas une réponse, la longueur en octets du billet ou de la réponse (LEN) et le billet ou la
réponse (DATA).
 */
typedef struct info_billet
{
	u16 id;		   // id sur 11bits le reste à 0, A METTRE EN BE
	u16 numb_numr; // numb sur 11 bits, le reste pour numr, A METTRE EN BE
	u16 len;	   // A METTRE EN BE
	u8 *data;	   // taille len
} info_billet;

/**
   CODEREQ = 17,
IDG est l’identifiant du groupe concerné,
NB est le nombre de billets qui vont être envoyés au client,
puis pour chaque billet sont envoyées, l’identifiant de l’auteur du billet ou de la réponse
(IDi), le numéro du billet (NUMBi), le numéro de la réponse (NUMRi) qui vaut zéro si ce n’est
pas une réponse, la longueur en octets du billet ou de la réponse (LENi) et le billet ou la
réponse (DATAi).
 */
typedef struct response_history_billet
{
	u16 codereq_idg;		  // codereq sur 5bits le reste pour l'idg, A METTRE EN BE
	u16 nb;					  // A METTRE EN BE
	info_billet *list_billet; // taille nb
} resp_history_billet;

/**
   CODEREQ = 31.
 */
typedef struct response_error
{
	u16 codereq; // codereq sur 5bits le reste à 0, A METTRE EN BE
} resp_error;

/**
- Lorsque le serveur reçoit un nouveau message ou une réponse à poster sur le fil du groupe,
il notifie les utilisateur·ices du groupe avec CODEREQ = 18,
– lorsqu’un·e utilisateur·ice accepte une invitation, le serveur notifie les membres du groupe
avec CODEREQ = 19,
- lorsqu’un utilisateur·ice quitte le groupe, le serveur notifie les membres du groupe avec
CODEREQ = 20,
- lorsque l’administrateur·ice ferme le groupe, le serveur notifie les membres du groupe avec
CODEREQ = 21.
– lorsqu’il reçoit une invitation destinée à un·e utilisateur·ice, il envoie une notification d’in-
vitation à cet·te utilisateur·ice avec CODEREQ = 22,
– lorsqu’un utilisateur·ice récupère un billet, l’utilisateur·ice qui a écrit le billet est notifié·e
avec CODEREQ = 23,
 */
typedef struct notif_groupe
{
	u16 codereq_idg; // codereq sur 5bits le reste pour l'idg, A METTRE EN BE
} notif_grp;

#endif
