#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#include "request.h"
#include "protocol.h"

void prepare_register_req(req_register *req, const char *username)
{
    // 5 bits CODEREQ + 11 bits à 0
    req->req_code = (REQ_REGISTER & MASK_5_BITS) << 11;

    // Copie du pseudo, le reste est complété par des zéros automatiquement grâce à strncpy
    strncpy((char *)req->username, username, sizeof(req->username));

    // TODO: S'occuper de la clé plus tard
    memset(req->pub_key, 0, sizeof(req->pub_key));
}

void prepare_group_req(req_create_group *req, int ID, const char *NOMG)
{
    // On ajoute "(ID & 0x07FF)".
    // Pourquoi : Le nombre 0x07FF correspond à 11 bits remplis de "1".
    // Le symbole "&" va agir comme un filtre : il coupe tout ce qui dépasse 11 bits.
    // C'est une sécurité au cas où l'ID fourni serait un nombre immense,
    // pour être sûr qu'il ne déborde pas sur le code de création (REQ_CREATE_GROUP).
    req->req_code_user_id = ((REQ_CREATE_GROUP & MASK_5_BITS) << 11) | (ID & MASK_11_BITS);

    req->name_len = (u8)strlen(NOMG);

    req->group_name = malloc(req->name_len);
    memcpy(req->group_name, NOMG, req->name_len);
}

void prepare_invitation_req(req_invite_group *req, int ID, int ID_group, int guest_number, u16 *guest_ids)
{
    req->req_code_user_id = ((REQ_INVITE & MASK_5_BITS) << 11) | (ID & MASK_11_BITS);

    req->group_id_count = ((ID_group & MASK_11_BITS) << 5) | (guest_number & MASK_5_BITS);

    req->invited_ids = malloc(sizeof(u16) * guest_number); // TODO: Ne pas oublier le free()
    if (req->invited_ids == NULL)
    {
        perror("Memory Allocation failed in prepare_invitation_req function.");
        return;
    }

    for (size_t i = 0; i < guest_number; i++)
    {
        req->invited_ids[i] = ((guest_ids[i] & MASK_11_BITS) << 5);
    }
}

void prepare_waiting_invitation_list_req(req_list_invitations *req, int ID)
{
    req->req_code_user_id = ((REQ_GET_INVITES & MASK_5_BITS) << 11) | (ID & MASK_11_BITS);
}

void prepare_invitation_response_req(req_group_action *req, int ID, int ID_group, int answer)
{
    req->req_code_user_id = ((REQ_GROUP_ACTION & MASK_5_BITS) << 11) | (ID & MASK_11_BITS);
    req->group_id_action = ((ID_group & MASK_11_BITS) << 5) | ((answer & MASK_2_BITS) << 3);
}

void prepare_list_members_req(req_list_members *req, int ID, int ID_group)
{
    req->req_code_user_id = ((REQ_GET_MEMBERS & MASK_5_BITS) << 11) | (ID & MASK_11_BITS);
    req->group_id = ((ID_group & MASK_11_BITS) << 5);
}

void prepare_post_message_req(req_post_message *req)
{
    
}

void prepare_reply_message_req(req_reply_message *req);

void prepare_list_messages_req(req_list_messages *req);
