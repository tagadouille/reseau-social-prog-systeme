#ifndef INVITATION_H
#define INVITATION_H

/**
 * @brief Structure représentant une invitation à un groupe
 */
typedef struct invitation
{
    int group_id;
    int user_id;
} invitation_t;

/**
 * @brief permet de créer une invitation à un groupe pour un utilisateur
 * 
 * @param group_id l'id du groupe
 * @param user_id l'id de l'utilisateur
 * 
 * @return un pointeur vers une invitation, NULL si erreur
 */
invitation_t *invitation_create(int group_id, int user_id);

/**
 * @brief permet de détruire une invitation
 * 
 * @param invitation l'invitation à détruire
 */
void invitation_destroy(invitation_t *invitation);

#endif