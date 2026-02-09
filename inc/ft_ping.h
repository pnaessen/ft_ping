#define _GNU_SOURCE

#ifndef FT_PING_HPP
#define FT_PING_HPP

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

#define ERR_DNS -1
#define ERR_SOCKET -2
#define SUCCESS 0

#define MAX_PATTERN_LEN 16
#define PING_PKT_S 64
#define PAYLOAD_TIMESTAMP 12
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
    int ttl;	  //  --ttl
    int type;	  // -t
    int count;	  //  -c
    int deadline; // -w
    unsigned char pattern[MAX_PATTERN_LEN];
    int pattern_len;
    bool pattern_set;
} t_ping;

int resolve_dns(const char *host, struct sockaddr_in *dest);

void init_ping_packet(t_ping_packet *pkt, t_ping *ping);
uint16_t calculate_checksum(void *addr, int len);
ssize_t send_ping(t_ping *ping);

void handle_reception(t_ping *ping);

double calculate_rtt(struct icmphdr *icmp);

void signalHandler(int signum);
void print_final_stats(t_ping *ping);
void update_stats(t_ping *ping, double rtt);
void print_packet_info(t_ping *ping, struct iphdr *ip, struct icmphdr *icmp, double rtt,
		       ssize_t bytes);

int parse_args(int argc, char **argv, t_ping *ping);
void print_packet_debug(struct iphdr *ip, struct icmphdr *icmp);
void usage(const char *exec);
void print_help();
void print_ping_header(t_ping *ping);

bool is_deadline_reached(t_ping *ping, double start_time);
double get_time_now();

#endif /* FT_PING_H */
