#include <string.h>

#include "utils.h"
#include "create_group.h"

/**
 * @brief extrait NOMG du buffer et le renvoie
 * @param buf le buffer de requête qui a été envoyé par le client
 * @param NOMG un char * qui est le nom du groupe qui sera rempli
 * par la fonction
 * @param len la longueur de NOMG
 */
static void read_NOMG(u8* buf, char * NOMG, u8 len) {

    size_t loc = 0;

    for (size_t i = 3; i < 3 + len; i++)
    {
        NOMG[loc] = buf[i];
        loc++;
    }
    NOMG[loc] = '\0';
    
    server_log("Le nom du groupe est : %s", NOMG);
}

void read_create_group(u8 *buf, req_create_group *request)
{
    int current_pos = 2; // Car codereq déja lu

    u8 len = buf[2]; // Récupération de la longueur de NOMG
    server_log("La longueur du nom du groupe est %i ", len);

    char NOMG[len + 1];

    read_NOMG(buf, NOMG, len);
    prepare_group_req(request, read_id(buf), NOMG);
}

void read_rep_create_group(u8 *buf, resp_create_group *response){
    
    int code_req = (buf[0] >> 3) & MASK_5_BITS;

    client_log("[create group] Le code_req vaut %i", code_req);

    if(code_req != 4)
    {
        return;
    }

    int group_id = read_id(buf);

    int mdiff_port = ntohs(buf[2]);

    u8 mdiff_addr[16];
    memcpy(mdiff_addr, buf + 3, 16);
    
    prepare_group_resp(response, group_id, mdiff_port, mdiff_addr);
}