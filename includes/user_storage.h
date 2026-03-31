#ifndef USER_STORAGE_H
#define USER_STORAGE_H

#define USER_PATH "./usertest"

int store_user(int id, const char *name, int udp_port, const char *key, const char *path);
int find_id(const char *path);

#endif
