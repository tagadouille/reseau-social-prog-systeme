#ifndef REQUEST_H
#define REQUEST_H

#include "./utils.h"

/**
   CODEREQ = 1,
   NOM est le pseudo de l’utilisateur·ice de 10 caractères maximum, avec complétion par des
   caractères nuls si le pseudo fait moins de 10 caractères,
   CLE correspond à la clef publique de l’utilisateur·ice sous forme d’une chaîne de caractères
*/
typedef struct inscription
{
	u16 codereq; // sur 5bits le reste à 0, A METTRE EN BE
	u8 nom[10];
	u8 cle[118]; // Vide si pas de securite
} req_inscription;

/**
   CODEREQ = 3,
   NOMG est le nom du groupe avec LEN son nombre de caractères.
*/
typedef struct creation_groupe
{
	u16 codereq_id; // codereq sur 5bits le reste pour l'id, A METTRE EN BE
	u8 len;
	u8 *nomg; // de taille len
} req_new_grp;

/**
   CODEREQ = 5,
   IDG est l’identifiant du groupe,
   NB est le nombre d’invitations suivi des NB identifiants des utilisateur·ices invité·es et ZEROS
   (ID1 et ZEROS, ID2 et ZEROS...)
*/
typedef struct invitation_grp
{
	u16 codereq_id; // codereq sur 5bits le reste pour l'id, A METTRE EN BE
	u16 idg_nb; // idg sur 11bits le reste pour nb, A METTRE EN BE
	u16 *id_personne; // taille nb, 5 dernier bits à 0, A METTRE EN BE
} req_inv_grp;

/**
   CODEREQ = 6
*/
typedef struct list_invitation_grp
{
	u16 codereq_id; // codereq sur 5bits le reste pour l'id, A METTRE EN BE	
} req_list_inv;

/**
   CODEREQ = 8,
   IDG est l’identifiant du groupe,
   AN vaut 1 si une invitation est acceptée, 0 si elle refusée et 2 si l’utilisateur·ice demande à
   quitter le groupe
*/
typedef struct request_rej_quit_grp
{
	u16 codereq_id; // codereq sur 5bits le reste pour l'id, A METTRE EN BE
	u16 idg_an; // idg sur 11bits, 2 bits pour AN, reste à 0, A METTRE EN BE
} req_rej_quit;

/**
   CODEREQ = 10,
   IDG est l’identifiant du groupe si la requête concerne les membres du groupe, et vaut 0 si
   elle concerne toustes les utilisateur·ices inscrit·es.
*/
typedef struct list_membre
{
	u16 codereq_id; // codereq sur 5bits le reste pour l'id, A METTRE EN BE
	u16 idg; // idg sur 11bits, reste à 0, A METTRE EN BE	
} req_list_mb;

/**
   CODEREQ = 12,
   IDG est l’identifiant du groupe,
   LEN est la longueur du billet en octets,
   DATA est le billet.
*/
typedef struct billet
{
	u16 codereq_id; // codereq sur 5bits le reste pour l'id, A METTRE EN BE
	u16 idg; // idg sur 11bits, reste à 0, A METTRE EN BE
	u16 len; // A METTRE EN BE
	u8 *data; // taille len
} req_billet;

/**
   CODEREQ = 14,
   IDG est l’identifiant du groupe,
   NUMB est le numéro du billet auquel l’utilisateur·ice répond,
   LEN est la longueur de la réponse en octets,
   DATA est la réponse.
*/
typedef struct reponse_billet
{
	u16 codereq_id; // codereq sur 5bits le reste pour l'id, A METTRE EN BE
	u16 idg; // idg sur 11bits, reste à 0, A METTRE EN BE
	u16 numb; // numb sur 11bits, reste à 0, A METTRE EN BE
	u16 len; // A METTRE EN BE
	u8 *data; // taille len
} req_rep_billet;

/**
   CODEREQ = 16,
   IDG est l’identifiant du groupe,
   NUMB est le numéro du dernier billet reçu avec le numéro de la dernière réponse à ce billet
   NUMR reçue.
*/
typedef struct list_billet
{
	u16 codereq_id; // codereq sur 5bits le reste pour l'id, A METTRE EN BE
	u16 idg; // idg sur 11bits, reste à 0, A METTRE EN BE
	u16 numb_numr; // numb sur 11bits, reste pour numr, A METTRE EN BE
} req_list_billet;


#endif
