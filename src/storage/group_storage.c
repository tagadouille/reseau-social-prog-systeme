#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include "storage/group_storage.h"

int store_group(int id_group, const char *group_name, int mdiff_port, const u8 *mdiff_addr, const char *path)
{
    mkdir(path, 0755); // s'il n'existe pas déjà

    // Création du dossier du groupe :
    char group_path[MAX_LEN_PATH];
    snprintf(group_path, MAX_LEN_PATH, "%s/%d", path, id_group);

    if (mkdir(group_path, 0755) == -1)
    {
        perror("mkdir user dir");
        return -1;
    }

    // Ecriture du nom du groupe :
    char name_file_path[MAX_LEN_PATH];
    if (snprintf(name_file_path, MAX_LEN_PATH, "%s/name", group_path) >= MAX_LEN_PATH)
    {
        perror("name file path too long");
        return -1;
    }

    int fd_name = open(name_file_path, O_CREAT | O_EXCL | O_WRONLY, 0755);
    if (fd_name < 0)
    {
        perror("open name file");
        return -1;
    }
    write(fd_name, group_name strlen(group_name));
    close(fd_name);

    char port_file_path[MAX_LEN_PATH];
    if (snprintf(port_file_path, MAX_LEN_PATH, "%s/udp_port", group_path) >= MAX_LEN_PATH)
    {
        perror("port file path too long");
        return -1;
    }

    printf("port : %d", udp_port);

    int fd_port = open(port_file_path, O_CREAT | O_EXCL | O_WRONLY, 0755);
    if (fd_port < 0)
    {
        perror("open port udp file");
        return -1;
    }
    write(fd_port, &udp_port, sizeof(int));
    close(fd_port);

    char key_file_path[MAX_LEN_PATH];
    if (snprintf(key_file_path, MAX_LEN_PATH, "%s/key", group_path) >= MAX_LEN_PATH)
    {
        perror("key file path too long");
        return -1;
    }

    int fd_key = open(key_file_path, O_CREAT | O_EXCL | O_WRONLY, 0755);
    if (fd_key < 0)
    {
        perror("open key file");
        return -1;
    }
    write(fd_key, key, strlen(key));
    close(fd_key);

    return 0;
}

u8 find_free_mdiff_addr()
{
    // TODO
}

int find_free_mdiff_port()
{
    // TODO
}