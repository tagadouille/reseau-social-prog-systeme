#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../includes/serialization.h"
#include "../../includes/utils.h"
#include "../../includes/request.h"
#include "../../includes/protocol.h"

/*
 * Convention de sécurité dans ce fichier :
 * - Si un pointeur obligatoire est NULL, on retourne -1.
 * - Si une longueur > 0 mais le pointeur associé est NULL, on retourne -1.
 *
 * Important : toutes les fonctions retournent le nombre total d'octets écrits
 * dans le buffer. Cette taille est ensuite utilisée par l'appelant pour send().
 */

/*
 * Sérialise la requête d'inscription (CODEREQ = 1).
 *
 * Arguments :
 * - buf : buffer binaire de sortie (message réseau final).
 * - req : structure C contenant req_code + username + pub_key.
 *
 * Retour :
 * - nombre d'octets écrits (à passer à send()),
 * - ou -1 en cas d'entrée invalide (protection anti-crash).
 */
ssize_t build_register_req(u8 *buf, req_register *req)
{
    // Sécurité : si buf ou req est NULL, on évite un Segmentation Fault.
    if (buf == NULL || req == NULL)
        return -1;

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
 * Sérialise la requête de création de groupe (CODEREQ = 3).
 *
 * Arguments :
 * - buf : buffer de sortie.
 * - req : contient req_code_user_id, name_len, group_name.
 *
 * Retour : taille totale du message (en octets) pour send(), ou -1.
 */
ssize_t build_group_req(u8 *buf, req_create_group *req)
{
    // Sécurité : évite un crash si on nous passe un pointeur vide.
    if (buf == NULL || req == NULL)
        return -1;

    // Si name_len > 0, on doit impérativement avoir un pointeur valide.
    if (req->name_len > 0 && req->group_name == NULL)
        return -1;

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
 * Sérialise la requête d'invitation dans un groupe (CODEREQ = 5).
 *
 * Arguments :
 * - buf : buffer binaire de sortie.
 * - req : contient l'en-tête et la liste des IDs invités.
 *
 * Retour : taille totale encodée (octets), ou -1.
 */
ssize_t build_invitation_req(u8 *buf, req_invite_group *req)
{
    // Sécurité : contrôle de base pour éviter un accès mémoire invalide.
    if (buf == NULL || req == NULL)
        return -1;

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

    // S'il y a des invités à écrire, le tableau doit exister.
    if (nb_guests > 0 && req->invited_ids == NULL)
        return -1;

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
 * Sérialise la demande de liste des invitations en attente (CODEREQ = 6).
 *
 * Arguments :
 * - buf : destination binaire.
 * - req : contient seulement req_code_user_id.
 *
 * Retour : taille en octets du message prêt à envoyer, ou -1.
 */
ssize_t build_waiting_invitation_list_req(u8 *buf, req_list_invitations *req)
{
    // Sécurité : protège le serveur si l'appelant a oublié d'initialiser req.
    if (buf == NULL || req == NULL)
        return -1;

    u8 *p = buf;

    // On envoie juste les 16 premiers bits.
    u16 req_code = htons(req->req_code_user_id);
    memcpy(p, &req_code, sizeof(req_code));
    p += sizeof(req_code);

    return (size_t)(p - buf);
}

/*
 * Sérialise une action sur invitation/groupe (CODEREQ = 8).
 *
 * Arguments :
 * - buf : buffer réseau de sortie.
 * - req : contient req_code_user_id et group_id_action.
 *
 * Retour : taille du message en octets (pour send()), ou -1.
 */
ssize_t build_invitation_response_req(u8 *buf, req_group_action *req)
{
    // Sécurité : évite les déréférencements de pointeurs NULL.
    if (buf == NULL || req == NULL)
        return -1;

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
 * Sérialise la demande de liste des membres (CODEREQ = 10).
 *
 * Arguments :
 * - buf : buffer binaire à remplir.
 * - req : contient req_code_user_id + group_id (0 possible pour liste globale).
 *
 * Retour : taille écrite dans buf (octets), ou -1.
 */
ssize_t build_list_members_req(u8 *buf, req_list_members *req)
{
    // Sécurité : on stoppe tôt si les entrées sont invalides.
    if (buf == NULL || req == NULL)
        return -1;

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
 * Sérialise l'envoi d'un nouveau billet (CODEREQ = 12).
 *
 * Arguments :
 * - buf : buffer de sortie.
 * - req : contient code+id, group_id, data_len, data.
 *
 * Retour : taille totale du paquet binaire, ou -1.
 */
ssize_t build_post_message_req(u8 *buf, req_post_message *req)
{
    // Sécurité : évite un crash si la structure n'est pas fournie.
    if (buf == NULL || req == NULL)
        return -1;

    // Si on annonce des données, le pointeur data doit être valide.
    if (req->data_len > 0 && req->data == NULL)
        return -1;

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
 * Sérialise la réponse à un billet existant (CODEREQ = 14).
 *
 * Arguments :
 * - buf : destination réseau.
 * - req : contient code+id, group_id, ticket_id, data_len, data.
 *
 * Retour : nombre d'octets écrits (utile pour send), ou -1.
 */
ssize_t build_reply_message_req(u8 *buf, req_reply_message *req)
{
    // Sécurité : protège contre les pointeurs invalides.
    if (buf == NULL || req == NULL)
        return -1;

    if (req->data_len > 0 && req->data == NULL)
        return -1;

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
 * Sérialise la demande d'historique incrémental (CODEREQ = 16).
 *
 * Arguments :
 * - buf : buffer de sortie.
 * - req : contient code+id, group_id, et last_ticket_reply_id (NUMB+NUMR).
 *
 * Retour : taille finale du message binaire, ou -1.
 */
ssize_t build_list_messages_req(u8 *buf, req_list_messages *req)
{
    // Sécurité : return -1 plutôt que crash si req est absent.
    if (buf == NULL || req == NULL)
        return -1;

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

ssize_t build_register_resp(u8 *buf, resp_register *resp)
{
    /*
     * Réponse inscription (CODEREQ = 2).
     * Arguments : buf (sortie), resp (code+id, port UDP, clé publique).
     * Retour : taille totale en octets pour send(), ou -1.
     */
    // Sécurité : empêche un Segmentation Fault si un pointeur est NULL.
    if (buf == NULL || resp == NULL)
        return -1;

    u8 *p = buf;

    // 1. Écriture de l'en-tête [CODEREQ|ID] en Big Endian.
    u16 resp_code = htons(resp->resp_code_user_id);
    memcpy(p, &resp_code, sizeof(resp_code));
    // On avance de 2 octets pour écrire le champ suivant sans écraser.
    p += sizeof(resp_code);

    // 2. Écriture du port UDP en Big Endian.
    u16 udp_port = htons(resp->udp_port);
    memcpy(p, &udp_port, sizeof(udp_port));
    // Même idée : on décale le curseur après copie.
    p += sizeof(udp_port);

    // 3. Écriture de la clé publique (taille fixe protocolaire).
    size_t len_public_key = sizeof(resp->server_pub_key);
    memcpy(p, resp->server_pub_key, len_public_key);
    p += len_public_key;

    return (size_t)(p - buf);
}

ssize_t build_group_resp(u8 *buf, resp_create_group *resp)
{
    /*
     * Réponse création groupe (CODEREQ = 4).
     * Arguments : buf (sortie), resp (code+id groupe, port mdiff, IPv6 mdiff).
     * Retour : taille totale en octets, ou -1.
     */
    // Sécurité basique contre les pointeurs invalides.
    if (buf == NULL || resp == NULL)
        return -1;

    u8 *p = buf;

    // 1. En-tête [CODEREQ|IDG] en ordre réseau.
    u16 resp_code = htons(resp->resp_code_group_id);
    memcpy(p, &resp_code, sizeof(resp_code));
    // On avance le curseur de 2 octets.
    p += sizeof(resp_code);

    // 2. Port de multidiffusion en Big Endian.
    u16 mdiff_port = htons(resp->mdiff_port);
    memcpy(p, &mdiff_port, sizeof(mdiff_port));
    // On avance encore pour préparer l'écriture IPv6.
    p += sizeof(mdiff_port);

    // 3. Adresse IPv6 de multidiffusion (16 octets fixes).
    size_t len_mdiff_ipv6 = sizeof(resp->mdiff_ipv6);
    memcpy(p, resp->mdiff_ipv6, len_mdiff_ipv6);
    p += len_mdiff_ipv6;

    return (size_t)(p - buf);
}

ssize_t build_generic_ack_resp(u8 *buf, resp_generic_ack *resp)
{
    /*
     * Réponse ACK simple (CODEREQ = 24).
     * Arguments : buf (sortie), resp (champ d'en-tête ACK).
     * Retour : taille en octets (2), ou -1.
     */
    // Sécurité : coupe court si entrée invalide.
    if (buf == NULL || resp == NULL)
        return -1;

    u8 *p = buf;

    // 1. Écriture du champ ACK en Big Endian.
    u16 ack = htons(resp->resp_header);
    memcpy(p, &ack, sizeof(ack));
    // 2. Avance du curseur pour calculer la taille finale.
    p += sizeof(ack);

    return (size_t)(p - buf);
}

ssize_t build_error_resp(u8 *buf, resp_error *resp)
{
    /*
     * Réponse erreur (CODEREQ = 31).
     * Arguments : buf (sortie), resp (champ resp_code).
     * Retour : taille totale (2 octets), ou -1.
     */
    // Sécurité : évite les crashes sur pointeurs NULL.
    if (buf == NULL || resp == NULL)
        return -1;

    u8 *p = buf;

    // 1. Écriture du code d'erreur en format réseau.
    u16 err = htons(resp->resp_code);
    memcpy(p, &err, sizeof(err));
    // 2. Déplacement du curseur.
    p += sizeof(err);

    return (size_t)(p - buf);
}

ssize_t build_list_invitations_resp(u8 *buf, resp_list_invitations *resp)
{
    /*
     * Réponse liste des invitations (CODEREQ = 7).
     * Arguments :
     * - buf : buffer de sortie
     * - resp : en-tête + tableau d'invitations
     * Retour : taille totale sérialisée, ou -1.
     */
    // Sécurité : garde-fou contre pointeurs invalides.
    if (buf == NULL || resp == NULL)
        return -1;

    u8 *p = buf;

    // 1. Écriture de l'en-tête [CODEREQ|NB].
    u16 resp_code = htons(resp->resp_code_count);
    memcpy(p, &resp_code, sizeof(resp_code));
    p += sizeof(resp_code);

    // 2. Extraction de NB : on garde uniquement les 11 bits de droite.
    // On utilise MASK_11_BITS pour ignorer le CODEREQ stocké dans les 5 bits de gauche.
    int count = (resp->resp_code_count & MASK_11_BITS);

    // Cohérence minimale : count > 0 implique un tableau alloué.
    if (count > 0 && resp->invitations == NULL)
        return -1;

    // 3. Boucle d'écriture de chaque invitation.
    for (int i = 0; i < count; i++)
    {
        resp_group_invitation_info *invitations = &(resp->invitations[i]);

        // 3.1 Écriture du couple [IDG|LEN] en Big Endian.
        u16 group_id_name_len = htons(invitations->group_id_name_len);
        memcpy(p, &group_id_name_len, sizeof(group_id_name_len));
        p += sizeof(group_id_name_len);

        // 3.2 LEN est codé sur les 5 bits de droite de group_id_name_len.
        size_t len_group_name = (invitations->group_id_name_len & MASK_5_BITS);

        if (len_group_name > 0 && invitations->group_name == NULL)
            return -1;

        // 3.3 Écriture du nom de groupe (taille variable LEN).
        memcpy(p, invitations->group_name, len_group_name);
        p += len_group_name;

        // 3.4 Écriture du nom admin (taille fixe 10 octets).
        size_t len_admin_name = sizeof(invitations->admin_name);
        memcpy(p, invitations->admin_name, len_admin_name);
        p += len_admin_name;
    }

    return (size_t)(p - buf);
}

ssize_t build_accept_invitation_resp(u8 *buf, resp_accept_invitation *resp)
{
    /*
     * Réponse invitation acceptée (CODEREQ = 9).
     * Arguments : buf (sortie), resp (en-tête groupe + infos mdiff + membres).
     * Retour : taille totale du message, ou -1.
     */
    // Sécurité : protège contre les pointeurs NULL.
    if (buf == NULL || resp == NULL)
        return -1;

    if (resp->member_count > 0 && resp->members == NULL)
        return -1;

    u8 *p = buf;

    // 1. Écriture de l'en-tête [CODEREQ|IDG].
    u16 resp_code = htons(resp->resp_code_group_id);
    memcpy(p, &resp_code, sizeof(resp_code));
    p += sizeof(resp_code);

    // 2. Écriture du port de multidiffusion.
    u16 mdiff_port = htons(resp->mdiff_port);
    memcpy(p, &mdiff_port, sizeof(mdiff_port));
    // On avance pour écrire l'adresse IPv6.
    p += sizeof(mdiff_port);

    // 3. Écriture de l'IPv6 de multidiffusion (16 octets).
    size_t len_mdiff_ipv6 = sizeof(resp->mdiff_ipv6);
    memcpy(p, resp->mdiff_ipv6, len_mdiff_ipv6);
    p += len_mdiff_ipv6;

    // 4. Écriture du nombre de membres (u16 réseau).
    u16 member_count = htons(resp->member_count);
    memcpy(p, &member_count, sizeof(member_count));
    p += sizeof(member_count);

    // 5. Boucle d'écriture des membres (ID + nom[10]).
    for (size_t i = 0; i < resp->member_count; i++)
    {
        resp_user_info *member = &(resp->members[i]);

        // 5.1 ID membre en Big Endian.
        u16 user_id = htons(member->user_id);
        memcpy(p, &user_id, sizeof(user_id));
        p += sizeof(user_id);

        // 5.2 Nom fixe sur 10 octets.
        size_t len_name = sizeof(member->name);
        memcpy(p, member->name, len_name);
        p += len_name;
    }

    return (size_t)(p - buf);
}

ssize_t build_list_members_resp(u8 *buf, resp_list_members *resp)
{
    /*
     * Réponse liste des membres (CODEREQ = 11).
     * Arguments : buf (sortie), resp (en-tête + tableau members).
     * Retour : taille écrite (octets), ou -1.
     */
    // Sécurité : évite un crash sur structure vide.
    if (buf == NULL || resp == NULL)
        return -1;

    if (resp->member_count > 0 && resp->members == NULL)
        return -1;

    u8 *p = buf;

    // 1. En-tête [CODEREQ|IDG] en ordre réseau.
    u16 resp_code = htons(resp->resp_code_group_id);
    memcpy(p, &resp_code, sizeof(resp_code));
    p += sizeof(resp_code);

    // 2. Nombre de membres.
    u16 member_count = htons(resp->member_count);
    memcpy(p, &member_count, sizeof(member_count));
    p += sizeof(member_count);

    // 3. Écriture séquentielle de tous les membres.
    for (size_t i = 0; i < resp->member_count; i++)
    {
        resp_user_info *member = &(resp->members[i]);

        // 3.1 ID membre.
        u16 user_id = htons(member->user_id);
        memcpy(p, &user_id, sizeof(user_id));
        p += sizeof(user_id);

        // 3.2 Nom membre (10 octets).
        size_t len_name = sizeof(member->name);
        memcpy(p, member->name, len_name);
        p += len_name;
    }

    return (size_t)(p - buf);
}

ssize_t build_post_message_resp(u8 *buf, resp_post_message *resp)
{
    /*
     * Accusé création billet (CODEREQ = 13).
     * Arguments : buf (sortie), resp (en-tête groupe + ticket_id).
     * Retour : taille totale encodée (octets), ou -1.
     */
    // Sécurité : protège contre les pointeurs invalides.
    if (buf == NULL || resp == NULL)
        return -1;

    u8 *p = buf;

    // 1. En-tête [CODEREQ|IDG].
    u16 resp_code = htons(resp->resp_code_group_id);
    memcpy(p, &resp_code, sizeof(resp_code));
    p += sizeof(resp_code);

    // 2. Numéro de billet (NUMB) packé côté préparation, converti ici en BE.
    u16 ticket_id = htons(resp->ticket_id);
    memcpy(p, &ticket_id, sizeof(ticket_id));
    p += sizeof(ticket_id);

    return (size_t)(p - buf);
}

ssize_t build_reply_message_resp(u8 *buf, resp_reply_message *resp)
{
    /*
     * Accusé réponse à billet (CODEREQ = 15).
     * Arguments : buf (sortie), resp (en-tête groupe + ticket_reply_id).
     * Retour : taille finale en octets, ou -1.
     */
    // Sécurité : évite un Segmentation Fault si resp est NULL.
    if (buf == NULL || resp == NULL)
        return -1;

    u8 *p = buf;

    // 1. En-tête [CODEREQ|IDG].
    u16 resp_code = htons(resp->resp_code_group_id);
    memcpy(p, &resp_code, sizeof(resp_code));
    p += sizeof(resp_code);

    // 2. Couple [NUMB|NUMR] déjà packé, converti en ordre réseau.
    u16 ticket_id = htons(resp->ticket_reply_id);
    memcpy(p, &ticket_id, sizeof(ticket_id));
    p += sizeof(ticket_id);

    return (size_t)(p - buf);
}

ssize_t build_list_messages_resp(u8 *buf, resp_list_messages *resp)
{
    /*
     * Réponse historique des messages (CODEREQ = 17).
     * Arguments :
     * - buf : destination binaire
     * - resp : en-tête groupe + compteur + tableau de messages
     * Retour : taille totale encodée, ou -1.
     */
    // Sécurité : ces gardes évitent un crash du serveur sur entrée invalide.
    if (buf == NULL || resp == NULL)
        return -1;

    if (resp->message_count > 0 && resp->message_history == NULL)
        return -1;

    u8 *p = buf;

    // 1. En-tête [CODEREQ|IDG].
    u16 resp_code = htons(resp->resp_code_group_id);
    memcpy(p, &resp_code, sizeof(resp_code));
    p += sizeof(resp_code);

    // 2. Nombre total d'éléments de l'historique.
    u16 message_count = htons(resp->message_count);
    memcpy(p, &message_count, sizeof(message_count));
    p += sizeof(message_count);

    // 3. Boucle sur chaque message (billet ou réponse).
    for (size_t i = 0; i < resp->message_count; i++)
    {
        resp_message_info *message = &(resp->message_history[i]);

        // 3.1 ID auteur.
        u16 author_id = htons(message->author_id);
        memcpy(p, &author_id, sizeof(author_id));
        p += sizeof(author_id);

        // 3.2 Couple [NUMB|NUMR].
        u16 ticket_reply_id = htons(message->ticket_reply_id);
        memcpy(p, &ticket_reply_id, sizeof(ticket_reply_id));
        p += sizeof(ticket_reply_id);

        // 3.3 Taille du contenu texte.
        u16 data_len = htons(message->data_len);
        memcpy(p, &data_len, sizeof(data_len));
        p += sizeof(data_len);

        if (message->data_len > 0 && message->data == NULL)
            return -1;

        // 3.4 Contenu du message (longueur variable).
        memcpy(p, message->data, message->data_len);
        p += message->data_len;
    }

    return (size_t)(p - buf);
}

ssize_t build_group_notification_resp(u8 *buf, resp_group_notification *resp)
{
    /*
     * Réponse notification groupe (CODEREQ = 18..23).
     * Arguments :
     * - buf : buffer de sortie
     * - resp : champ notif_code_group_id (code notification + id groupe)
     * Retour : taille en octets (2), ou -1.
     */
    // Sécurité : protège contre les pointeurs invalides.
    if (buf == NULL || resp == NULL)
        return -1;

    // 1. On extrait CODEREQ (5 bits de gauche) avec >> 11.
    // Puis on applique MASK_5_BITS pour ne garder que ces 5 bits.
    // Vérification protocole : valeurs autorisées ici = 18 à 23.
    u16 notif_code = (resp->notif_code_group_id >> 11) & MASK_5_BITS;
    if (notif_code < NOTIF_NEW_TICKET || notif_code > NOTIF_TICKET_READ)
        return -1;

    u8 *p = buf;

    // 2. Écriture du champ notification en Big Endian.
    u16 resp_code = htons(resp->notif_code_group_id);
    memcpy(p, &resp_code, sizeof(resp_code));
    // 3. Avance du curseur pour calculer la taille finale.
    p += sizeof(resp_code);

    return (size_t)(p - buf);
}
