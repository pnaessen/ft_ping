#ifndef UTILS_H
#define UTILS_H

#include "ping_types.h"

int parse_args(int argc, char **argv, t_ping *ping);

void print_ping_header(t_ping *ping);
void print_packet_info(t_ping *ping, struct iphdr *ip, struct icmphdr *icmp, double rtt,
		       ssize_t bytes);
void print_final_stats(t_ping *ping);
void print_packet_debug(struct iphdr *ip, struct icmphdr *icmp);
void print_help(void);
void usage(const char *exec);

double get_time_now(void);
bool is_deadline_reached(t_ping *ping, double start_time);
double calculate_rtt(struct icmphdr *icmp);
uint16_t calculate_checksum(void *addr, int len);
void update_stats(t_ping *ping, double rtt);

void signalHandler(int signum);

#endif
