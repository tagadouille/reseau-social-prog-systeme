#ifndef CREATE_GROUP_H
#define CREATE_GROUP_H

#include "utils.h"
#include "request.h"
#include "response.h"
#include "serialization.h"

void read_create_group(u8 *buf, req_create_group *request);
void read_rep_create_group(u8 *buf, resp_create_group *response);

#endif