#define _GNU_SOURCE

#ifndef FT_PING_HPP
#define FT_PING_HPP

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/ip_icmp.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
# include <signal.h>

#define ERR_DNS -1
#define ERR_SOCKET -2
#define SUCCESS 0

#define PING_PKT_S 64
#define PING_DATA_S (PING_PKT_S - sizeof(struct icmphdr))
#define PING_INTERVAL 1000000

typedef struct s_ping_packet {
    struct icmphdr hdr;
    char msg[PING_DATA_S];
} t_ping_packet;

int resolve_dns(const char *host, struct sockaddr_in *dest);
void init_ping_packet(struct s_ping_packet *pkt, uint16_t seq);
uint16_t calculate_checksum(void *addr, int len);
#endif /* FT_PING_H */
