#ifndef PROTOCOL_H
#define PROTOCOL_H

// =========================================================
// ÉCHANGES CLIENT -> SERVEUR (Requêtes)
// =========================================================
#define REQ_REGISTER 0x01     // Client s'inscrit (pseudo + clé publique)
#define REQ_CREATE_GROUP 0x03 // Client crée un nouveau groupe
#define REQ_INVITE 0x05       // Admin invite des utilisateurs dans son groupe
#define REQ_GET_INVITES 0x06  // Client demande la liste de ses invitations en attente
#define REQ_GROUP_ACTION 0x08 // Client accepte/refuse une invitation ou quitte un groupe
#define REQ_GET_MEMBERS 0x0a  // Client demande les membres d'un groupe (ou tous les inscrits si 0)
#define REQ_POST_TICKET 0x0c  // Client poste un nouveau billet sur le fil
#define REQ_REPLY_TICKET 0x0e // Client répond à un billet spécifique
#define REQ_GET_LAST_TICKET 0x10  // Client demande le dernier billet reçu avec le numéro de la dernière réponse à ce billet

// =========================================================
// ÉCHANGES SERVEUR -> CLIENT (Réponses directes)
// =========================================================
#define RES_REGISTER 0x02        // Serveur donne l'ID unique et le port UDP
#define RES_CREATE_GROUP 0x04    // Serveur donne l'ID du groupe et l'adresse IP/Port Multicast
#define RES_GET_INVITES 0x07     // Serveur liste les groupes où le client est invité
#define RES_GROUP_ACTION_OK 0x09 // Serveur confirme l'ajout au groupe (avec liste des membres)
#define RES_GET_MEMBERS 0x0b     // Serveur envoie la liste des membres demandée
#define RES_POST_TICKET 0x0d     // Serveur confirme l'ajout du billet (donne son numéro)
#define RES_REPLY_TICKET 0x0f    // Serveur confirme la réponse (donne son numéro de réponse)
#define RES_GET_TICKETS 0x11     // Serveur envoie la liste des billets demandés

// =========================================================
// ACQUITTEMENTS ET ERREURS (Serveur -> Client)
// =========================================================

// ACK générique (CODEQE=24) utilisé pour :
// - confirmation d'invitation envoyée
// - refus d'invitation
// - départ volontaire d'un groupe
#define RES_ACK 0x18   // Serveur acquitte une action simple (Valeur décimale 24)
#define RES_ERROR 0x1F // Serveur signale un problème (requête mal formée, etc.) (Valeur décimale 31)

// =========================================================
// NOTIFICATIONS DU SERVEUR (Événements spontanés)
// =========================================================
// Multicast (envoyé à tous les membres d'un groupe)
#define NOTIF_NEW_TICKET 0x12   // Un nouveau billet ou réponse a été posté (Valeur 18)
#define NOTIF_USER_JOINED 0x13  // Quelqu'un a accepté une invitation et rejoint le groupe (Valeur 19)
#define NOTIF_USER_LEFT 0x14    // Quelqu'un a quitté le groupe (Valeur 20)
#define NOTIF_GROUP_CLOSED 0x15 // L'admin a fermé le groupe (Valeur 21)

// UDP (envoyé à un seul utilisateur)
#define NOTIF_INVITE_RECV 0x16 // L'utilisateur vient de recevoir une nouvelle invitation (Valeur 22)
#define NOTIF_TICKET_READ 0x17 // Un billet de l'utilisateur a été récupéré ou a reçu une réponse (Valeur 23)

#endif
