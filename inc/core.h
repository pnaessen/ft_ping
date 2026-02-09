#ifndef CORE_H
#define CORE_H

#include "ping_types.h"

void init_ping_packet(t_ping_packet *pkt, t_ping *ping);
ssize_t send_ping(t_ping *ping);

void handle_reception(t_ping *ping);

int process_error_icmp(t_ping *ping, struct sockaddr_in *addr, struct icmphdr *icmp, ssize_t bytes);

#endif
