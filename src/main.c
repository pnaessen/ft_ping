#include "ft_ping.h"

bool g_signal = false;

void signalHandler(int signum)
{

    (void)signum;
    g_signal = true;
}

void printData(struct s_ping_packet pkt)
{

    struct timeval *tv_ptr = (struct timeval *)pkt.msg;
    printf("Timestamp : %ld.%06ld | Data : ", tv_ptr->tv_sec, tv_ptr->tv_usec);
    for (size_t i = sizeof(struct timeval); i < PING_DATA_S && i < sizeof(pkt.msg); i++) {
	printf("%c", pkt.msg[i]);
    }
    printf("\n");
}

int main(int argc, char const **argv)
{
    if (argc != 2) {
	// TODO: better check args for flags
	dprintf(2, "Usage: %s <ip adress>\n", argv[0]);
	return 1;
    }

    signal(SIGINT, signalHandler);
    signal(SIGQUIT, signalHandler);

    t_ping_packet pkt;
    struct sockaddr_in sockaddr;
    uint16_t seq = 0;

    if (resolve_dns(argv[1], &sockaddr) != 0) {
	dprintf(2, "Error in dns resolution\n");
	return ERR_DNS;
    }

    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd < 0) {
	dprintf(2, "Error creat socket\n");
	return ERR_SOCKET;
    }

    while (g_signal != true) {
	init_ping_packet(&pkt, seq);
	printData(pkt);
	// sendto()
	// recvfrom()
	seq++;
	usleep(PING_INTERVAL);
    }

    close(sockfd);
    // printf("checksum %d\n", icmp_req.checksum);
    // printf("type %d\n", icmp_req.type);
    // printf("code %d\n", icmp_req.code);
    // printf("id %d\n", icmp_req.un.echo.id);

    printf("Pas de segfault\n");
    return SUCCESS;
}
