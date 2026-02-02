#include "ft_ping.h"

int main(int argc, char const **argv)
{
    if (argc != 2) {
	// TODO: better check args for flags
	dprintf(2, "Usage: %s <ip adress>\n", argv[0]);
	return 1;
    }

    struct sockaddr_in sockaddr;

    if (resolve_dns(argv[1], &sockaddr) != 0) {
	dprintf(2, "Error in dns resolution\n");
	return 1;
    }

    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd < 0) {
		dprintf(2, "Error creat socket\n");
		return 1;
    }
	struct icmphdr test;

    printf("Pas de segfault\n");
    return 0;
}
