#ifndef PING_TYPES_H
#define PING_TYPES_H

#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <netdb.h>
#include <netinet/ip_icmp.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

/* --- Constants & Macros --- */
#define ERR_DNS -1
#define ERR_SOCKET -2
#define SUCCESS 0

#define MAX_PATTERN_LEN 16
#define PING_PKT_S 64
#define PAYLOAD_TIMESTAMP 12
#define PING_DATA_S (PING_PKT_S - sizeof(struct icmphdr))
#define PING_INTERVAL 1000000

/* --- Structures --- */

typedef struct s_ping_packet {
    struct icmphdr hdr;
    char msg[PING_DATA_S];
} t_ping_packet;

typedef struct s_stats {
    long pkts_transmitted;
    long pkts_received;
    double min_rtt;
    double max_rtt;
    double sum_rtt;
    double sum_sq_rtt;
} t_stats;

typedef struct s_ping {
    int sockfd;
    struct sockaddr_in dest_addr;
    char *target_host;
    char target_ip[INET_ADDRSTRLEN];
    uint16_t seq;
    t_stats stats;

    bool verbose; //  -v
    int ttl;	  //  --ttl
    int type;	  // -t
    int count;	  //  -c
    int deadline; // -w
    unsigned char pattern[MAX_PATTERN_LEN];
    int pattern_len;
    bool pattern_set;
} t_ping;

#endif /* PING_TYPES_H */
