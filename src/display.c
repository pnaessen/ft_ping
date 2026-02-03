#include "ft_ping.h"

void print_stats(struct iphdr *ip, struct icmphdr *icmp, ssize_t bytes)
{

    (void)ip;
    (void)icmp;
    (void)bytes;
}

void print_addrinfo_list(struct addrinfo *result)
{
    int i = 0;
    char ip_str[INET6_ADDRSTRLEN];

    for (struct addrinfo *ai = result; ai != NULL; ai = ai->ai_next) {
	printf("=== Nœud %d ===\n", ++i);
	printf("ai_flags: 0x%x\n", ai->ai_flags);
	printf("ai_family: %d (%s)\n", ai->ai_family,
	       ai->ai_family == AF_INET	   ? "AF_INET"
	       : ai->ai_family == AF_INET6 ? "AF_INET6"
					   : "Autre");
	printf("ai_socktype: %d (%s)\n", ai->ai_socktype,
	       ai->ai_socktype == SOCK_STREAM  ? "SOCK_STREAM"
	       : ai->ai_socktype == SOCK_DGRAM ? "SOCK_DGRAM"
	       : ai->ai_socktype == SOCK_RAW   ? "SOCK_RAW"
					       : "Autre");
	printf("ai_protocol: %d\n", ai->ai_protocol);
	printf("ai_addrlen: %d\n", (int)ai->ai_addrlen);

	if (ai->ai_canonname) {
	    printf("ai_canonname: %s\n", ai->ai_canonname);
	} else {
	    printf("ai_canonname: NULL\n");
	}

	if (ai->ai_addr) {
	    inet_ntop(ai->ai_family, ai->ai_addr, ip_str, sizeof(ip_str));
	    printf("ai_addr: %s\n", ip_str);
	} else {
	    printf("ai_addr: NULL\n");
	}
	printf("\n");
    }
}

void printData(struct s_ping_packet pkt)
{
    struct timeval *tv_ptr = (struct timeval *)pkt.msg;
    printf("Timestamp : %ld.%06ld | Data func : ", tv_ptr->tv_sec, tv_ptr->tv_usec);
    for (size_t i = sizeof(struct timeval); i < PING_DATA_S && i < sizeof(pkt.msg); i++) {
	printf("%c", pkt.msg[i]);
    }
    printf("\n");
}
