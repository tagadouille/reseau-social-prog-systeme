#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <linux/limits.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>

#include "communication/group_struct.h"
#include "storage/group_storage.h"
#include "log.h"

group_t *group_initialize(int group_id, const u8 *group_name, int id_admin)
{
    return group_fill(group_id, group_name, id_admin, NULL, 0, 0, (u8[16]){0});
}

group_t *group_fill(int group_id, const u8 *group_name, int id_admin, int *members_id, size_t members_id_len, int mdiff_port, u8 mdiff_addr[16])
{
    group_t *group = malloc(sizeof(group_t));

    if (group == NULL)
    {
        perror("malloc group fill");
        return NULL;
    }

    group->group_id = group_id;

    group->group_name = malloc(strlen((char *)group_name) + 1);

    if(group->group_name == NULL)
    {
        perror("malloc group_name group fill");
        free(group);
        return NULL;
    }
    memcpy(group->group_name, group_name, strlen((char *)group_name) + 1);

    group->id_admin = id_admin;

    group->members_id = members_id;
    group->members_id_len = members_id_len;

    group->mdiff_port = mdiff_port;
    memcpy(group->mdiff_addr, mdiff_addr, 16);

    return group;
}

/**
 * @brief wrapper contenant la liste des membres du groupe
 * et sa longueur, utilisé uniquement pour la fonction 
 * extract_members_id
 */
typedef struct members_wrapper {

    int* members_id;
    size_t members_id_len;
} members_wrapper_t;

/**
 * @brief permet de récupérer l'ensemble des membres du groupes
 * 
 * @param path le chemin où sont stocké les membres du groupes
 * 
 * @return un pointeur vers un members_wrapper, NULL si erreur
 */
static members_wrapper_t* extract_members_id(const char * path)
{
    DIR *dir = opendir(path);

    if (dir == NULL)
    {
        perror("opendir extract_members_id");
        return NULL;
    }

    struct dirent *entry;

    size_t members_id_capacity = 10;
    int *members_id = malloc(members_id_capacity * sizeof(int));

    if(members_id == NULL)
    {
        perror("malloc members id extract_members_id");
        closedir(dir);
        return NULL;
    }

    size_t members_id_len = 0;

    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }

        char member_file_path[PATH_MAX];
        if (snprintf(member_file_path, PATH_MAX, "%s/%s", path, entry->d_name) >= PATH_MAX)
        {
            perror("member file path too long");
            closedir(dir);
            free(members_id);
            return NULL;
        }

        struct stat path_stat;
        stat(member_file_path, &path_stat);
        if (!S_ISREG(path_stat.st_mode))
        {
            continue;
        }

        int member_id = atoi(entry->d_name);

        if(members_id_len >= members_id_capacity)
        {
            size_t new_capacity = members_id_capacity * 2;
            int *new_members_id = realloc(members_id, new_capacity * sizeof(int));

            if (new_members_id == NULL)
            {
                perror("realloc members id extract_members_id");
                free(members_id);
                closedir(dir);
                return NULL;
            }

            members_id = new_members_id;
            members_id_capacity = new_capacity;
        }
        members_id[members_id_len] = member_id;
        members_id_len++;
    }
    closedir(dir);

    members_wrapper_t *wrapper = malloc(sizeof(members_wrapper_t));

    if (wrapper == NULL)
    {
        perror("malloc members wrapper extract_members_id");
        free(members_id);
        return NULL;
    }

    wrapper->members_id = members_id;
    wrapper->members_id_len = members_id_len;

    return wrapper;
}

group_t *group_fill_from_files(int group_id)
{

    // Chemin vers le répertoire du groupe
    char dir_path[PATH_MAX];

    snprintf(dir_path, PATH_MAX, "%s/%d", GROUP_PATH, group_id);

    // Extraction du nom du groupe :
    char name_file_path[PATH_MAX];
    if (snprintf(name_file_path, PATH_MAX, "%s/group_name", dir_path) >= PATH_MAX)
    {
        perror("name file path too long");
        return NULL;
    }

    int fd_name = open(name_file_path, O_RDONLY);

    if (fd_name < 0)    {
        perror("open name file");
        return NULL;
    }

    u8 group_name[256];
    ssize_t re = read(fd_name, group_name, sizeof(u8) * 256);

    if (re < 0)    {
        perror("read name file");
        close(fd_name);
        return NULL;
    }
    group_name[re] = '\0';
    close(fd_name);

    // Extraction de l'admin :
    char admin_file_path[PATH_MAX];
    if (snprintf(admin_file_path, PATH_MAX, "%s/admin_id", dir_path) >= PATH_MAX)
    {
        perror("admin file path too long");
        return NULL;
    }

    int fd_admin = open(admin_file_path, O_RDONLY);

    if (fd_admin < 0)
    {
        perror("open admin file");
        return NULL;
    }

    int id_admin;
    if (read(fd_admin, &id_admin, sizeof(id_admin)) != sizeof(id_admin))
    {
        perror("read admin file");
        close(fd_admin);
        return NULL;
    }
    close(fd_admin);

    // Extraction des membres :
    char members_path[PATH_MAX];

    if (snprintf(members_path, PATH_MAX, "%s/users", dir_path) >= PATH_MAX)
    {
        perror("port file path too long");
        return NULL;
    }

    members_wrapper_t *members_wrapper = extract_members_id(members_path);

    if(members_wrapper == NULL)
    {
        log_server("[group_fill_from_files] Echec de extract_members_id");
        return NULL;
    }

    int *members_id = members_wrapper -> members_id;
    size_t members_id_len = members_wrapper -> members_id_len;

    free(members_wrapper);

    // Extraction du port :
    char port_file_path[PATH_MAX];

    if (snprintf(port_file_path, PATH_MAX, "%s/mdiff_port", dir_path) >= PATH_MAX)
    {
        perror("port file path too long");
        return NULL;
    }

    int fd_port = open(port_file_path, O_RDONLY);

    if (fd_port < 0)
    {
        perror("open port file");
        return NULL;
    }

    int mdiff_port;
    if (read(fd_port, &mdiff_port, sizeof(mdiff_port)) != sizeof(mdiff_port))
    {
        perror("read port file");
        close(fd_port);
        return NULL;
    }
    close(fd_port);

    // Extraction de l'adresse :
    char addr_file_path[PATH_MAX];

    if (snprintf(addr_file_path, PATH_MAX, "%s/mdiff_addr", dir_path) >= PATH_MAX)
    {
        perror("addr file path too long");
        return NULL;
    }

    int fd_addr = open(addr_file_path, O_RDONLY);

    if (fd_addr < 0)
    {
        perror("open addr file");
        return NULL;
    }

    u8 mdiff_addr[16];

    if (read(fd_addr, mdiff_addr, 16) != 16)
    {
        perror("read addr file");
        close(fd_addr);
        return NULL;
    }
    close(fd_addr);

    return group_fill(group_id, group_name, id_admin, members_id, members_id_len, mdiff_port, mdiff_addr);
}

void group_struct_destroy(group_t *group)
{
    if (group != NULL)
    {
        if (group->group_name != NULL)
        {
            free(group->group_name);
        }

        if (group->members_id != NULL)
        {
            free(group->members_id);
        }
        free(group);
    }
}