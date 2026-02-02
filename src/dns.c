#include "ft_ping.h"

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

int resolve_dns(const char *host, struct sockaddr_in *dest)
{

    struct addrinfo hints;
    struct addrinfo *result;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_RAW;
    dest->sin_port = 0;
    dest->sin_family = AF_INET;

    int err = getaddrinfo(host, NULL, &hints, &result);
    if (err != 0) {
	dprintf(2, "Error getaddrinfo: %s\n", gai_strerror(err));
	return ERR_DNS;
    }
    // print_addrinfo_list(result);
    struct sockaddr_in *addr_in = (struct sockaddr_in *)result->ai_addr;
    dest->sin_addr = addr_in->sin_addr;
    freeaddrinfo(result);
    return SUCCESS;
}

uint16_t calculate_checksum(void *addr, int len)
{
    uint32_t sum = 0;
    uint16_t *ptr = addr;

    while (len > 1) {
	sum += *ptr++;
	len -= 2;
    }

    if (len == 1) {
	sum += *(uint8_t *)ptr;
    }

    while (sum >> 16) {
	sum = (sum & 0xffff) + (sum >> 16);
    }

    return (uint16_t)~sum;
}

void init_ping_packet(struct s_ping_packet *pkt, uint16_t  seq) {
    memset(pkt, 0, sizeof(*pkt));

    pkt->hdr.type = ICMP_ECHO;
    pkt->hdr.code = 0;
    pkt->hdr.un.echo.id = htons(getpid() & 0xFFFF);
    pkt->hdr.un.echo.sequence = htons(seq);

    struct timeval tv;
    gettimeofday(&tv, NULL);
    memcpy(pkt->msg, &tv, sizeof(tv));

    pkt->hdr.checksum = 0;
    pkt->hdr.checksum = calculate_checksum(pkt, sizeof(*pkt));
}
