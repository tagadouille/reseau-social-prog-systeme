#include <string.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <sys/socket.h>
#include <errno.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../../includes/utils.h"
#include <linux/limits.h>

/*
 * Lit exactement 'len' octets sur le réseau.
 * On utilise une boucle car TCP coupe parfois les messages en plusieurs morceaux.
 * Retourne le nombre d'octets lus, ou -1 si erreur/déconnexion.
 */
int recv_all(int sock, char *buf, int len)
{
    int received = 0;
    while (received < len)
    {
        // On décale le pointeur (buf + received) pour remplir la suite du tableau
        // On demande uniquement les octets manquants (len - received)
        int n = recv(sock, buf + received, len - received, 0);

        if (n < 0)
        {
            perror("Erreur recv");
            return -1;
        }
        if (n == 0)
        {
            printf("Connexion coupée par le client.\n");
            return -1;
        }
        received += n;
    }
    return received;
}

/*
 * Envoie exactement 'len' octets sur le réseau.
 * On boucle pour garantir que tout le buffer part sur le réseau.
 */
int send_all(int sock, const char *buf, int len)
{
    int sent = 0;
    while (sent < len)
    {
        int n = send(sock, buf + sent, len - sent, 0);

        if (n < 0)
        {
            perror("Erreur send");
            return -1;
        }
        if (n == 0)
        {
            return -1;
        }
        sent += n;
    }
    return sent;
}

/**
 * Permet de recevoir le codereq d'une requête venant de 'sock' en utilisant 'buf_header'
 * RETURN VALUE: -1 si problème dans recv_all, codereq sinon
 */
int read_codereq(int sock, u8 *buf_header)
{
	if (recv_all(sock, (char *)buf_header, 2) < 0)
		return -1;

	uint16_t code;
	memcpy(&code, buf_header, 2);
	code = ntohs(code);

	return (code >> 11) & MASK_5_BITS; 
}

int read_id(u8 * buffer) {

    u16 value = (buffer[0] << 8) | buffer[1];

    int id = value & MASK_11_BITS;

    return id;
}


char *IPV6_addr_to_string(uint8_t *addr) {
    
    char *str = malloc(INET6_ADDRSTRLEN);
    if (!str) return NULL;

    if (inet_ntop(AF_INET6, addr, str, INET6_ADDRSTRLEN) == NULL) {
        free(str);
        return NULL;
    }

    return str;
}

/**
 * @brief Supprime récursivement tous les fichiers et répertoires
 * d'un chemin donné, puis supprime le répertoire lui-même.
 * 
 * @param path le chemin vers le répertoire à supprimer
 * @return 0 if success, -1 if error
 */
static int remove_directory_contents(const char *path) {

    DIR *dir;
    struct dirent *entry;

    struct stat statbuf;

    char full_path[PATH_MAX];

    int result = 0;
    
    if (!path) {
        dprintf(2, "Error : the path can't be null\n");
        return -1;
    }
    
    dir = opendir(path);
    if (!dir) {
        perror("opendir");
        return -1;
    }
    
    // Read all the entries :
    while ((entry = readdir(dir)) != NULL && result == 0) {

        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        // Make the full path :
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
        
        if (lstat(full_path, &statbuf) == -1) {
            dprintf(2, "Error : lstat fail for '%s': %s\n", full_path, strerror(errno));
            continue;
        }
        
        // Process according to the file type :

        // Directory
        if (S_ISDIR(statbuf.st_mode)) {
            
            // Delete the content :
            if (remove_directory_contents(full_path) == -1) {
                dprintf(2, "Error : deletion of the sub-directory failed '%s'\n", full_path);
                result = -1;
                break;
            }
            
            // Delete the directory :
            if (rmdir(full_path) == -1) {
                dprintf(2, "Error: failed to delete the directory '%s': %s\n", full_path, strerror(errno));
                result = -1;
                break;
            }
            
        } else {
            // If it's a file or a symbolic link :
            if (unlink(full_path) == -1) {
                result = -1;
                dprintf(2, "Error : failed to delete the file '%s' : %s\n", full_path, strerror(errno));
                break;
            }
        }
    }
    
    if (closedir(dir) == -1) {
        dprintf(2, "Error : Error when closing the directory '%s': %s\n", path, strerror(errno));
    }
    
    if (result == 0) {
        dprintf(1, "Success: Content of '%s' completely delete\n", path);
    } else {
        dprintf(2, "Failure : Error while delete the content of '%s'\n", path);
    }
    
    return result;
}

int delete_directory(const char *path) {

    struct stat statbuf;
    
    if (!path) {
        dprintf(2, "Error : the path can't be null\n");
        return -1;
    }
    
    // Some verifications :
    if (access(path, F_OK) == -1) {
        dprintf(2, "Error : The path'%s' doesn't exist\n", path);
        return -1;
    }
    
    if (stat(path, &statbuf) == -1) {
        dprintf(2, "Error: lstat failed '%s': %s\n", path, strerror(errno));
        return -1;
    }
    
    if (!S_ISDIR(statbuf.st_mode)) {
        dprintf(2, "Errorr: '%s' is not a directory\n", path);
        return -1;
    }
    
    // delete
    int result = remove_directory_contents(path);

    if (rmdir(path) == -1) {
        dprintf(2, "Error: failed to delete the directory '%s': %s\n", path, strerror(errno));
        result = -1;
    }
    
    return result;
}