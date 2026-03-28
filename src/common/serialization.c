#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "serialization.h"
#include "utils.h"
#include "request.h"

ssize_t build_register_req(u8 *buf, req_register *req)
{
    u8 *p = buf;

    u16 req_code = htons(req->req_code);
    memcpy(p, &req_code, sizeof(req_code));
    p += sizeof(req_code);

    size_t len_username = sizeof(req->username);
    memcpy(p, req->username, len_username);
    p += len_username;

    size_t len_public_key = sizeof(req->pub_key);
    memcpy(p, req->pub_key, len_public_key);
    p += len_public_key;

    return (size_t)(p - buf);
}

ssize_t build_group_req(u8 *buf, req_create_group *req)
{
    u8 *p = buf;

    u16 req_code = htons(req->req_code_user_id);
    memcpy(p, &req_code, sizeof(req_code));
    p += sizeof(req_code);

    memcpy(p, &req->name_len, sizeof(req->name_len));
    p += sizeof(req->name_len);

    memcpy(p, req->group_name, req->name_len);
    p += req->name_len;

    return (size_t)(p - buf);
}

ssize_t build_invitation_req(u8 *buf, req_invite_group *req)
{
    u8 *p = buf;

    u16 req_code = htons(req->req_code_user_id);
    memcpy(p, &req_code, sizeof(req_code));
    p += sizeof(req_code);

    u16 group_info = htons(req->group_id_count);
    memcpy(p, &group_info, sizeof(group_info));
    p += sizeof(group_info);

    int nb_guests = (req->group_id_count & MASK_5_BITS);
    for (int i = 0; i < nb_guests; i++)
    {
        u16 invited_user_id = htons(req->invited_ids[i]);
        memcpy(p, &invited_user_id, sizeof(invited_user_id));
        p += sizeof(invited_user_id);
    }

    return (size_t)(p - buf);
}

ssize_t build_waiting_invitation_list_req(u8 *buf, req_list_invitations *req)
{
    u8 *p = buf;

    u16 req_code = htons(req->req_code_user_id);
    memcpy(p, &req_code, sizeof(req_code));
    p += sizeof(req_code);

    return (size_t)(p - buf);
}

ssize_t build_invitation_response_req(u8 *buf, req_group_action *req)
{
    u8 *p = buf;

    u16 req_code = htons(req->req_code_user_id);
    memcpy(p, &req_code, sizeof(req_code));
    p += sizeof(req_code);

    u16 group_action = htons(req->group_id_action);
    memcpy(p, &group_action, sizeof(group_action));
    p += sizeof(group_action);

    return (size_t)(p - buf);
}

ssize_t build_list_members_req(u8 *buf, req_list_members *req)
{
    u8 *p = buf;
    u16 req_code = htons(req->req_code_user_id);
    memcpy(p, &req_code, sizeof(req_code));
    p += sizeof(req_code);

    u16 group_id = htons(req->group_id);
    memcpy(p, &group_id, sizeof(group_id));
    p += sizeof(group_id);
    return (size_t)(p - buf);
}

ssize_t build_post_message_req(u8 *buf, req_post_message *req)
{
    u8 *p = buf;

    u16 req_code = htons(req->req_code_user_id);
    memcpy(p, &req_code, sizeof(req_code));
    p += sizeof(req_code);

    u16 group_id = htons(req->group_id);
    memcpy(p, &group_id, sizeof(group_id));
    p += sizeof(group_id);

    u16 len = htons(req->data_len);
    memcpy(p, &len, sizeof(len));
    p += sizeof(len);

    memcpy(p, req->data, req->data_len);
    p += req->data_len;

    return (size_t)(p - buf);
}

ssize_t build_reply_message_req(u8 *buf, req_reply_message *req)
{
    u8 *p = buf;

    u16 req_code = htons(req->req_code_user_id);
    memcpy(p, &req_code, sizeof(req_code));
    p += sizeof(req_code);

    u16 group_id = htons(req->group_id);
    memcpy(p, &group_id, sizeof(group_id));
    p += sizeof(group_id);

    u16 ticket_id = htons(req->ticket_id);
    memcpy(p, &ticket_id, sizeof(ticket_id));
    p += sizeof(ticket_id);

    u16 len = htons(req->data_len);
    memcpy(p, &len, sizeof(len));
    p += sizeof(len);

    memcpy(p, req->data, req->data_len);
    p += req->data_len;

    return (size_t)(p - buf);
}

ssize_t build_list_messages_req(u8 *buf, req_list_messages *req)
{
    u8 *p = buf;

    u16 req_code = htons(req->req_code_user_id);
    memcpy(p, &req_code, sizeof(req_code));
    p += sizeof(req_code);

    u16 group_id = htons(req->group_id);
    memcpy(p, &group_id, sizeof(group_id));
    p += sizeof(group_id);

    u16 last_ticket_reply_id = htons(req->last_ticket_reply_id);
    memcpy(p, &last_ticket_reply_id, sizeof(last_ticket_reply_id));
    p += sizeof(last_ticket_reply_id);

    return (size_t)(p - buf);
}
