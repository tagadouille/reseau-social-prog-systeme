#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>

#include "storage/group_storage.h"
#include "user_storage.h"

int store_group(int id_group, const char *group_name, int mdiff_port, const u8 *mdiff_addr, const char *path)
{
    mkdir(path, 0755); // s'il n'existe pas déjà

    /*----------------------CREATION DU REPERTOIRE CONTENANT LE GROUPE---------------------------------*/
    char group_path[MAX_LEN_PATH];
    snprintf(group_path, MAX_LEN_PATH, "%s/%d", path, id_group);

    if (mkdir(group_path, 0755) == -1)
    {
        perror("mkdir user dir");
        return -1;
    }

    /*----------------------ECRITURE DU NOM DU GROUPE---------------------------------*/
    char name_file_path[MAX_LEN_PATH];
    if (snprintf(name_file_path, MAX_LEN_PATH, "%s/group_name", group_path) >= MAX_LEN_PATH)
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
    write(fd_name, group_name, strlen(group_name));
    close(fd_name);

    /*----------------------ECRITURE DU PORT DE MULTIDIFFUSION---------------------------------*/

    char port_file_path[MAX_LEN_PATH];
    if (snprintf(port_file_path, MAX_LEN_PATH, "%s/mdiff_port", group_path) >= MAX_LEN_PATH)
    {
        perror("port file path too long");
        return -1;
    }

    server_log("port de multidiffusion à écrire : %d", mdiff_port);

    int fd_port = open(port_file_path, O_CREAT | O_EXCL | O_WRONLY, 0755);
    if (fd_port < 0)
    {
        perror("open port udp file");
        return -1;
    }
    write(fd_port, &mdiff_port, sizeof(int));
    close(fd_port);

    /*----------------------ECRITURE DE L'ADRESSE DE MULTIDIFFUSION---------------------------------*/

    char key_file_path[MAX_LEN_PATH];
    if (snprintf(key_file_path, MAX_LEN_PATH, "%s/mdiff_addr", group_path) >= MAX_LEN_PATH)
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
    write(fd_key, mdiff_addr, sizeof(mdiff_addr));
    close(fd_key);

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
static int find_free_mdiff_addr_helper(diff_wrapper_t *diff_wrapper, int is_addr_good, int is_port_good)
{
    DIR *dir;
    struct dirent *entry;

    dir = opendir(GROUP_PATH);

    if (dir == NULL)
    {

        if (errno == ENOENT)
        {
            return 0;
        }
        perror("opendir find_free_mdiff_addr_helper");
        return -1;
    }

    int ret = 0;

    int is_addr_free = 1; // Si l'adresse est libre
    int is_port_free = 1; // Si le port est libre

    // Parcourt les entrées du répertoire de groupe
    while ((entry = readdir(dir)) != NULL)
    {

        if (is_addr_free == 0 && is_port_free == 0)
        {
            break;
        }

        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }

        char dir_path[MAX_LEN_PATH];
        snprintf(dir_path, sizeof(dir_path), "%s/%s", GROUP_PATH, entry->d_name);

        if (strcmp(entry->d_name, "mdiff_addr") == 0 && is_addr_free == 1 && is_addr_good == 0)
        {
            int fd = open(dir_path, O_RDONLY, 0);

            if (fd < 0)
            {
                perror("open mdiff file at find_free_mdiff_addr_helper");
                server_log("[find_free_mdiff_addr_helper] Le fichier à l'emplacement %s ne semble pas existé, cependant la recherche d'une adresse libre continue", dir_path);
                continue;
            }

            u8 addr[16];

            ssize_t r = read(fd, addr, 16 * sizeof(u8));

            if (r < 0)
            {
                perror("read find_free_mdiff_addr_helper");
            }

            if (r != 16 * sizeof(u8))
            {
                server_log("[find_free_mdiff_addr_helper] Le contenu du fichier à l'emplacement %s ne semble pas être valide, la longueur n'est pas la bonne, cependant la recherche d'une adresse libre continue", dir_path);
                continue;
            }

            if (memcmp(addr, diff_wrapper->mdiff_addr, 16 * sizeof(u8)) == 0)
            {
                memset(diff_wrapper->mdiff_addr, 0, 16 * sizeof(u8));
                server_log("[find_free_mdiff_addr_helper] L'adresse de multidiffusion est déjà utilisée par un groupe, elle est donc marquée comme non libre");
                is_addr_free = 0;
                ret = 1;
            }
        }
        else if (strcmp(entry->d_name, "mdiff_port") == 0 && is_port_free == 1 && is_port_good == 0)
        {
            int fd = open(dir_path, O_RDONLY, 0);

            if (fd < 0)
            {
                perror("open mdiff file at find_free_mdiff_addr_helper");
                server_log("[find_free_mdiff_addr_helper] Le fichier à l'emplacement %s ne semble pas existé, cependant la recherche d'une adresse libre continue", dir_path);
                continue;
            }

            int file_port;

            ssize_t r = read(fd, &file_port, sizeof(file_port));

            if (r < 0)
            {
                perror("read find_free_mdiff_addr_helper");
            }

            if (r != sizeof(file_port))
            {
                server_log("[find_free_mdiff_addr_helper] Le contenu du fichier à l'emplacement %s ne semble pas être valide, la longueur n'est pas la bonne, cependant la recherche d'une adresse libre continue", dir_path);
                continue;
            }

            if (file_port == diff_wrapper->mdiff_port)
            {
                diff_wrapper->mdiff_port = 0;
                server_log("[find_free_mdiff_addr_helper] Le port est déjà utilisée par un groupe, il est donc marquée comme non libre");
                is_port_free = 0;
                ret = 1;
            }
        }
    }
    closedir(dir);

    return ret;
}

diff_wrapper_t *find_free_mdiff_addr_port()
{
    diff_wrapper_t *ret = malloc(sizeof(diff_wrapper_t));

    if (ret == NULL)
    {
        perror("malloc diff wrapper find_free_mdiff_addr_port");
    }

    int free_msg;
    int is_addr_good = 0; // Si l'adresse est libre
    int is_port_good = 0; // Si le port est libre

    do
    {
        // Générer un port et une adresse multicast aléatoire :
        if (is_addr_good == 0)
        {
            u8 addr[16];
            srand(time(NULL));

            // Préfixe de multicast
            addr[0] = 0xFF;

            // Flags = 0x00, Scope = 0x0E (global)
            addr[1] = 0x0E;

            // Group ID aléatoire (112 bits restants)
            for (int i = 2; i < 16; i++) // TODO faire selon l'id du groupe ??
            {
                addr[i] = rand() % 256;
            }

            ret->mdiff_addr = addr;
        }

        if (is_port_good == 0)
        {
            ret->mdiff_port = 5000 + rand() % 1000;
        }

        free_msg = find_free_mdiff_addr_helper(ret, is_addr_good, is_port_good);

        // Vérifier si le port est libre :
        if(ret -> mdiff_port != 0)
        {
            is_port_good = 1;
        }

        // Vérifier si l'adresse est libre :
        u8 addr_cmp[16];

        memset(addr_cmp, 0, 16 * sizeof(u8));

        if(memcmp(ret -> mdiff_addr, addr_cmp, 16 * sizeof(u8)) != 0) 
        {
            is_addr_good = 1;
        }
        
    } while (free_msg != 0);

    return ret;
}