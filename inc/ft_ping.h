#define _GNU_SOURCE

#ifndef FT_PING_HPP
#define FT_PING_HPP

#include <arpa/inet.h>
#include <errno.h>
#include <float.h>
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
    int ttl;	  //  -t
    int count;	  //  -c
} t_ping;

int resolve_dns(const char *host, struct sockaddr_in *dest);

void init_ping_packet(struct s_ping_packet *pkt, uint16_t seq);
uint16_t calculate_checksum(void *addr, int len);
ssize_t send_ping(int sockfd, struct sockaddr_in *dest, uint16_t seq);

void handle_reception(t_ping *ping);

double calculate_rtt(struct icmphdr *icmp);

void signalHandler(int signum);
void print_final_stats(t_ping *ping);
void update_stats(t_ping *ping, double rtt);
void print_packet_info(t_ping *ping, struct iphdr *ip, struct icmphdr *icmp, double rtt,
		       ssize_t bytes);

int parse_args(int argc, char **argv, t_ping *ping);
void usage(const char *exec);
#endif /* FT_PING_H */
