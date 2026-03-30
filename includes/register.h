#ifndef INSCRIPTION_H
#define INSCRIPTION_H

#include "utils.h"
#include "request.h"
#include "response.h"
#include "serialization.h"
#include "protocol.h"

void read_register(u8 *buf, req_register *request);
void read_rep_register(u8 *buf, resp_register *response);

#endif
