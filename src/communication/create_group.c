#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>

#include "utils.h"
#include "communication/create_group.h"
#include "log.h"
#include "user_storage.h"

/**
 * @brief extrait NOMG du buffer et le renvoie
 * @param buf le buffer de requête qui a été envoyé par le client
 * @param NOMG un char * qui est le nom du groupe qui sera rempli
 * par la fonction
 * @param len la longueur de NOMG
 */
static void read_NOMG(u8 *buf, char *NOMG, u8 len)
{
    for (size_t i = 0; i < (size_t)len; i++)
    {
        NOMG[i] = buf[i];
    }
    NOMG[len] = '\0';

    log_server("[read_NOMG] The name of the group is : %s", NOMG);
}

group_t *read_create_group(int sock, req_create_group *request, u8 *buf_header)
{
    int user_id = read_id(buf_header);

    if (is_user_exists(USER_PATH, user_id) == 0)
    {
        log_server("The user with id %d doesn't exists");
        return NULL;
    }

    u8 buffer[256];

    ssize_t re = read(sock, buffer, 256);

    if (re < 0)
    {
        perror("read read create group 1");
        return NULL;
    }
    u8 len = buffer[1]; // Récupération de la longueur de NOMG
    log_server("[read_create_group] The length of the group name is : %i ", len);

    char NOMG[len + 1];

    read_NOMG(buffer, NOMG, len);
    prepare_group_req(request, user_id, NOMG);

    return group_initialize(-1, (const u8 *)NOMG, user_id);
}

void read_rep_create_group(u8 *buf, resp_create_group *response)
{

    int code_req = (buf[0] >> 3) & MASK_5_BITS;

    log_client("[create group] the code req is : %i", code_req);

    if (code_req != 4)
    {
        return;
    }

    int group_id = read_id(buf);

    int mdiff_port = ntohs(buf[2]);

    u8 mdiff_addr[16];
    memcpy(mdiff_addr, buf + 3, 16);

    prepare_group_resp(response, group_id, mdiff_port, mdiff_addr);
}