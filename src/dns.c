#include "ft_ping.h"

void print_addrinfo_list(struct addrinfo *result)
{
    int i = 0;
    char ip_str[INET6_ADDRSTRLEN];  // Support IPv4/IPv6

    for (struct addrinfo *ai = result; ai != NULL; ai = ai->ai_next) {
        printf("=== Nœud %d ===\n", ++i);
        printf("ai_flags: 0x%x\n", ai->ai_flags);
        printf("ai_family: %d (%s)\n", ai->ai_family,
               ai->ai_family == AF_INET ? "AF_INET" :
               ai->ai_family == AF_INET6 ? "AF_INET6" : "Autre");
        printf("ai_socktype: %d (%s)\n", ai->ai_socktype,
               ai->ai_socktype == SOCK_STREAM ? "SOCK_STREAM" :
               ai->ai_socktype == SOCK_DGRAM ? "SOCK_DGRAM" :
               ai->ai_socktype == SOCK_RAW ? "SOCK_RAW" : "Autre");
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
		return 1;
    }
	// print_addrinfo_list(result);
	struct sockaddr_in *addr_in = (struct sockaddr_in *)result->ai_addr;
    dest->sin_addr = addr_in->sin_addr;
	freeaddrinfo(result);
	return 0;
}
