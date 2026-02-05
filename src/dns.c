#include "ft_ping.h"

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
    struct sockaddr_in *addr_in = (struct sockaddr_in *)result->ai_addr;
    dest->sin_addr = addr_in->sin_addr;
    freeaddrinfo(result);

    char src_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &dest->sin_addr, src_ip, sizeof(src_ip));
    printf("PING %s (%s): %ld data bytes\n", host, src_ip, PING_DATA_S);
    return SUCCESS;
}
