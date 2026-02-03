#include "ft_ping.h"

bool g_signal = false;

int main(int argc, char const **argv)
{
    if (argc != 2) {
	// TODO: better check args for flags
	dprintf(2, "Usage: %s <ip adress>\n", argv[0]);
	return 1;
    }

    signal(SIGINT, signalHandler);
    signal(SIGQUIT, signalHandler);

    struct sockaddr_in sockaddr;
    uint16_t seq = 0;

    if (resolve_dns(argv[1], &sockaddr) != 0) {
	perror("Error in dns resolution\n");
	return ERR_DNS;
    }

    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd < 0) {
	perror("Error creat socket\n");
	return ERR_SOCKET;
    }

    while (!g_signal) {
	if (send_ping(sockfd, &sockaddr, seq) > 0) {
	}
	handle_reception(sockfd);
	seq++;
	usleep(PING_INTERVAL);
    }

    close(sockfd);
    return SUCCESS;
}
