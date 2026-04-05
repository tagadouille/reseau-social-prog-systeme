#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <linux/limits.h>

#include "../../includes/user_storage.h"

/**
 * Permet d'écrire les informations dans le fichier 'path'.
 * ├── 0
 *     ├── key         ici 0 est l'identifiant de l'utilisateur enregistré
 *     ├── name
 *     └── udp_port
 *
 * RETURN VALUE: -1 si problème d'appel système (voir perror), 0 sinon
 */
int store_user(int id, const char *name, int udp_port, const char *key, const char *path)
{
	mkdir(path, 0755); // s'il n'existe pas déjà

	char user_path[PATH_MAX];
	snprintf(user_path, PATH_MAX, "%s/%d", path, id);

	if (mkdir(user_path, 0755) == -1)
	{
		perror("mkdir user dir");
		return -1;
	}

	char name_file_path[PATH_MAX];
	if (snprintf(name_file_path, PATH_MAX, "%s/name", user_path) >= PATH_MAX)
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
	write(fd_name, name, strlen(name));
	close(fd_name);

	char port_file_path[PATH_MAX];
	if (snprintf(port_file_path, PATH_MAX, "%s/udp_port", user_path) >= PATH_MAX)
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

	char key_file_path[PATH_MAX];
	if (snprintf(key_file_path, PATH_MAX, "%s/key", user_path) >= PATH_MAX)
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

/**
 * Permet de déterminer un identifiant non utilisé dans le répertoire 'path'. En admettant que le répertoire
 * contient uniquement des sous-répertoires nommés par des entiers positifs.
 * RETURN VALUE: -1 si aucun id possible, l'id disponible sinon.
 */
int find_id(const char *path)
{
	mkdir(path, 0755); // s'il n'existe pas déjà

	int id = 0;
	char dir_path[PATH_MAX];

	while (1)
	{
		snprintf(dir_path, sizeof(dir_path), "%s/%d", path, id);
		struct stat st;
		if (stat(dir_path, &st) == -1)
			return id;
		id++;
	}
	return -1;
}

int is_user_exists(const char *path, int id)
{
	char dir_path[PATH_MAX];
	snprintf(dir_path, sizeof(dir_path), "%s/%d", path, id);

	struct stat st;
	if (stat(dir_path, &st) == -1)
	{
		if(errno = ENOENT)
		{
			return 0;
		}
	}
	return 1;
}
