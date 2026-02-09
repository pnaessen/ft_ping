#ifndef NETWORK_H
#define NETWORK_H

#include "ping_types.h"

void init_ping_struct(t_ping *ping);
int setup_socket(t_ping *ping);
int setup_target_and_socket(t_ping *ping);

int resolve_dns(const char *host, struct sockaddr_in *dest);

#endif
