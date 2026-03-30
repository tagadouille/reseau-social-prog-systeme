#include "../../includes/register.h"
#include <netinet/in.h>
#include <string.h>

void read_register(u8 *buf, req_register *request)
{
	int current_pos = 2; // 2 car codereq déjà lu

	char username[10];
	memset(username, 0, sizeof(username));
	memcpy(username, buf+current_pos, 10);
	current_pos += 10;

	u8 key[113];
	memset(key, 0, sizeof(key));
	memcpy(key, buf+current_pos, 113);
    
	prepare_register_req(request, username);
}

void read_rep_register(u8 *buf, resp_register *response)
{
	int current_pos = 0;
	uint16_t id;
	memcpy(&id, buf, 2);
	id = ntohs(id);
	id = (id & MASK_11_BITS);
	current_pos += 2;

	// On reçoit le port udp en BE donc on le reconstruit en LE pour le coté client
	u16 udp_port;
	memcpy(&udp_port, buf+current_pos, 2);
	udp_port = ntohs(udp_port);
	current_pos += 2;

	u8 key[113];
	memset(key, 0, sizeof(key));
	memcpy(key, buf+current_pos, 113);

	prepare_register_resp(response, id, udp_port);
}
