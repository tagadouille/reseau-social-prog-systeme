#ifndef USER_STORAGE_H
#define USER_STORAGE_H

#define USER_PATH "./usertest"

int store_user(int id, const char *name, int udp_port, const char *key, const char *path);

int find_id(const char *path);

/**
 * @brief permet de déterminer si un utilisateur à
 * partir de son id est inscrit
 * 
 * @param path le chemin où sont les utilisateurs
 * @param id l'id de l'utilisateur
 * 
 * @return 1 si l'utilisateur est inscrit, 0 sinon
 */
int is_user_exists(const char *path, int id);

#endif
