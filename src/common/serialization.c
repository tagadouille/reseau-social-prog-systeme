#include <sys/types.h>
#include <unistd.h>

#include "serialization.h"

ssize_t build_register_req(u8 *buf, req_register *req)
{
    u8 *p = buf;
    
}

ssize_t build_group_req(u8 *buf, req_create_group *req);

ssize_t build_invitation_req(u8 *buf, req_invite_group *req);

ssize_t build_waiting_invitation_list_req(u8 *buf, req_list_invitations *req);

ssize_t build_invitation_response_req(u8 *buf, req_group_action *req);

ssize_t build_list_members_req(u8 *buf, req_list_members *req);

ssize_t build_post_message_req(u8 *buf, req_post_message *req);

ssize_t build_reply_message_req(u8 *buf, req_reply_message *req);

ssize_t build_list_messages_req(u8 *buf, req_list_messages *req);
