#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>
#include <errno.h>
#include <linux/limits.h>

#include "storage/group_storage.h"
#include "user_storage.h"
#include "log.h"

#define MDIFF_ADDR_SIZE 16

int store_group(int id_group, const u8 *group_name, int mdiff_port, const u8 mdiff_addr[])
{
    if (mkdir(GROUP_PATH, 0755) < 0) // s'il n'existe pas déjà
    {
        if(errno != EEXIST)
        {
            perror("mkdir store_group");
            return -1;
        }
    }

    /*----------------------CREATION DU REPERTOIRE CONTENANT LE GROUPE---------------------------------*/
    char group_path[PATH_MAX];
    snprintf(group_path, PATH_MAX, "%s/%d", GROUP_PATH, id_group);

    if (mkdir(group_path, 0755) == -1)
    {
        perror("mkdir group dir store_group");
        return -1;
    }

    /*----------------------ECRITURE DU NOM DU GROUPE---------------------------------*/
    char name_file_path[PATH_MAX];
    if (snprintf(name_file_path, PATH_MAX, "%s/group_name", group_path) >= PATH_MAX)
    {
        perror("name file path too long store_group");
        return -1;
    }

    int fd_name = open(name_file_path, O_CREAT | O_EXCL | O_WRONLY, 0755);
    if (fd_name < 0)
    {
        perror("open name file store_group");
        return -1;
    }
    write(fd_name, group_name, strlen((const char *)group_name));
    close(fd_name);

    /*----------------------ECRITURE DU PORT DE MULTIDIFFUSION---------------------------------*/

    char port_file_path[PATH_MAX];
    if (snprintf(port_file_path, PATH_MAX, "%s/mdiff_port", group_path) >= PATH_MAX)
    {
        perror("port file path too long store_group");
        return -1;
    }

    log_server("Broadcast port to write : %d", mdiff_port);

    int fd_port = open(port_file_path, O_CREAT | O_EXCL | O_WRONLY, 0755);
    if (fd_port < 0)
    {
        perror("open port udp file store_group");
        return -1;
    }
    write(fd_port, &mdiff_port, sizeof(int));
    close(fd_port);

    /*----------------------ECRITURE DE L'ADRESSE DE MULTIDIFFUSION---------------------------------*/

    char key_file_path[PATH_MAX];
    if (snprintf(key_file_path, PATH_MAX, "%s/mdiff_addr", group_path) >= PATH_MAX)
    {
        perror("key file path too long store_group");
        return -1;
    }

    int fd_key = open(key_file_path, O_CREAT | O_EXCL | O_WRONLY, 0755);
    if (fd_key < 0)
    {
        perror("open key file store_group");
        return -1;
    }
    write(fd_key, mdiff_addr, MDIFF_ADDR_SIZE);
    close(fd_key);

    return 0;
}

int add_user_group(int user_id, int group_id)
{

    char dir_name[PATH_MAX];

    // Construction du chemin vers le groupe

    if(snprintf(dir_name, PATH_MAX, "%s/%d", GROUP_PATH, group_id) >= PATH_MAX)
    {
        perror("group path too long add_user_group");
        return -1;
    }

    DIR *dir = opendir(dir_name);

    if (dir == NULL)
    {
        if (errno == ENOENT)
        {
            log_server("The group of id %d doesn't exists", group_id);
        }
        perror("opendir add_user_group");
        return -1;
    }
    closedir(dir);

    // Chemin vers le répertoire users

    char users_path[PATH_MAX];
    if(snprintf(users_path, PATH_MAX, "%s/users", dir_name) >= PATH_MAX)
    {
        perror("users path too long add_user_group");
        return -1;
    }

    if (mkdir(users_path, 0755) < 0) // s'il n'existe pas déjà
    {
        if (errno != EEXIST)
        {
            perror("mkdir add_user_group");
            return -1;
        }
    }

    // Création du fichier de l'utilisateur

    if(snprintf(dir_name, PATH_MAX, "%s/%d", users_path, user_id) >= PATH_MAX)
    {
        perror("user file path too long add_user_group");
        return -1;
    }

    int fd = open(dir_name, O_CREAT, 644);

    if (fd < 0)
    {
        if (errno == EEXIST)
        {
            log_server("[add_user_group] The user %d is already in the group %d", user_id, group_id);
            return 0;
        }
        perror("open add user group");
        return -1;
    }
    close(fd);

    log_server("[add_user_group] The user %d was added to the group %d with success", user_id, group_id);

    return 0;
}

int add_admin_group(int admin_id, int group_id)
{
    char dir_name[PATH_MAX];

    // Construction du chemin vers le groupe

    if(snprintf(dir_name, PATH_MAX, "%s/%d", GROUP_PATH, group_id) >= PATH_MAX)
    {
        perror("group path too long add_admin_group");
        return -1;
    }

    DIR *dir = opendir(dir_name);

    if (dir == NULL)
    {
        if (errno == ENOENT)
        {
            log_server("The group of id %d doesn't exists", group_id);
        }
        perror("opendir find_free_mdiff_addr_helper");
        return -1;
    }
    closedir(dir);

    char admin_file_path[PATH_MAX];
    if(snprintf(admin_file_path, PATH_MAX, "%s/admin_id", dir_name) >= PATH_MAX)
    {
        perror("admin file path too long add_admin_group");
        return -1;
    }

    int fd = open(admin_file_path, O_CREAT | O_WRONLY, 0755);

    if (fd < 0)
    {
        perror("open add admin group");
        return -1;
    }
    if (write(fd, admin_id, sizeof(admin_id)) != sizeof(admin_id))
    {
        perror("write add admin");
        close(fd);
        return -1;
    }

    log_server("The admin %d was added to the group %d with success", admin_id, group_id);
    close(fd);

    return 0;
}

int delete_group(int group_id)
{
    char dir_path[PATH_MAX];

    snprintf(dir_path, PATH_MAX, "%s/%d", GROUP_PATH, group_id);

    int r = delete_directory(dir_path);

    if (r < 0)
    {
        log_server("The suppression of the group %d failed", group_id);
    }

    return r;
}

int delete_user_group(int user_id, int group_id)
{
    char dir_name[PATH_MAX];

    snprintf(dir_name, PATH_MAX, "%s/%d/users/%d", GROUP_PATH, group_id, user_id);

    if (unlink(dir_name) < 0)
    {
        perror("unlink");
        return -1;
    }

    log_server("The user %d was deleted of the group %d with success");

    return 0;
}

/**
 * @brief fonction helper pour find_free_mdiff_addr_port qui
 * permet de trouver une adresse de multidiffusion et un port de libre
 * en cherchant si dans les fichiers un autre groupe ne l'occupe pas déjà
 *
 * @param diff_wrapper un wrapper contenant le port et l'adresse,
 * La fonction peut mettre l'un des champs à 0, si c'est le cas c'est que le champ
 * a déjà été pris
 *
 * @param is_addr_good 1 si une adresse libre a déja été trouvé
 * @param is_port_good  si un port libre a déja été trouvé
 *
 * @return 0 si le port et l'adresse sont libres, 1 si ce n'est pas le cas, -1
 * si il y a eu une erreur
 */
static int find_free_mdiff_addr_helper(diff_wrapper_t *diff_wrapper, int *is_addr_good, int *is_port_good)
{
    DIR *dir;
    struct dirent *entry;

    dir = opendir(GROUP_PATH);

    if (dir == NULL)
    {
        if (errno == ENOENT)
        {
            mkdir(GROUP_PATH, 0755);
            return 0; // Le répertoire n'existait pas, donc l'adresse et le port sont libres
        }
        perror("opendir find_free_mdiff_addr_helper");
        return -1;
    }

    // Parcourt les sous-répertoires de groupe (ex: 0, 1, 2...)
    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }

        char group_dir_path[PATH_MAX];
        snprintf(group_dir_path, sizeof(group_dir_path), "%s/%s", GROUP_PATH, entry->d_name);

        struct stat path_stat;
        stat(group_dir_path, &path_stat);
        if (!S_ISDIR(path_stat.st_mode))
        {
            continue;
        }

        // Vérifier l'adresse si on n'en a pas encore trouvé de bonne
        if (!(*is_addr_good))
        {
            char addr_file_path[PATH_MAX];
            int len = snprintf(addr_file_path, sizeof(addr_file_path), "%s/mdiff_addr", group_dir_path);

            if (len < 0 || (size_t)len >= sizeof(addr_file_path))
            {
                log_server("Error: file path for mdiff_addr is too long.");
                continue;
            }

            int fd = open(addr_file_path, O_RDONLY);
            if (fd >= 0)
            {
                u8 addr[16];
                if (read(fd, addr, MDIFF_ADDR_SIZE) == MDIFF_ADDR_SIZE)
                {
                    if (memcmp(addr, diff_wrapper->mdiff_addr, MDIFF_ADDR_SIZE) == 0)
                    {
                        log_server("[find_free_mdiff_addr_helper] L'adresse de multidiffusion est déjà utilisée.");
                        closedir(dir);
                        return 1; // Adresse déjà utilisée
                    }
                }
                close(fd);
            }
        }

        // Vérifier le port si on n'en a pas encore trouvé de bon
        if (!(*is_port_good))
        {
            char port_file_path[PATH_MAX];
            int len = snprintf(port_file_path, sizeof(port_file_path), "%s/mdiff_port", group_dir_path);

            if (len < 0 || (size_t)len >= sizeof(port_file_path))
            {
                log_server("Error: file path for mdiff_port is too long.");
                continue;
            }

            int fd = open(port_file_path, O_RDONLY);
            if (fd >= 0)
            {
                int file_port;
                if (read(fd, &file_port, sizeof(file_port)) == sizeof(file_port))
                {
                    if (file_port == diff_wrapper->mdiff_port)
                    {
                        log_server("[find_free_mdiff_addr_helper] Le port est déjà utilisé.");
                        closedir(dir);
                        return 1; // Port déjà utilisé
                    }
                }
                close(fd);
            }
        }
    }
    closedir(dir);

    // Si on arrive ici, c'est que l'adresse et/ou le port testé n'ont pas été trouvés
    *is_addr_good = 1;
    *is_port_good = 1;
    return 0; // Libre
}

diff_wrapper_t *find_free_mdiff_addr_port()
{
    diff_wrapper_t *ret = malloc(sizeof(diff_wrapper_t));

    if (ret == NULL)
    {
        perror("malloc diff wrapper find_free_mdiff_addr_port");
        return NULL;
    }

    int is_addr_good = 0; // Si une adresse libre a été trouvée
    int is_port_good = 0; // Si un port libre a été trouvé

    do
    {
        // Générer une nouvelle adresse si nécessaire
        if (!is_addr_good)
        {
            // Préfixe de multicast IPv6
            ret->mdiff_addr[0] = 0xFF;
            // Flags = 0x0E (transient, global scope)
            ret->mdiff_addr[1] = 0x0E;
            // Group ID aléatoire (112 bits)
            for (int i = 2; i < 16; i++)
            {
                ret->mdiff_addr[i] = rand() % 256;
            }
        }

        // Générer un nouveau port si nécessaire
        if (!is_port_good)
        {
            ret->mdiff_port = 5000 + rand() % 1000;
        }

        int find_res = find_free_mdiff_addr_helper(ret, &is_addr_good, &is_port_good);

        if (find_res < 0)
        {
            free(ret);
            return NULL; // Erreur
        }

        if (find_res == 0)
        {
            // L'adresse et le port sont libres
            log_server("[find_free_mdiff_addr_port] Free address and broadcast port found.");
            break;
        }

        // Si find_res == 1, au moins un des deux est utilisé, la boucle continue
        // en ne régénérant que ce qui est nécessaire.
    } while (1);

    return ret;
}