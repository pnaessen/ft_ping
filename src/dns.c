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

    return SUCCESS;
}
