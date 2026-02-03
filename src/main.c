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
    printf("Timestamp : %ld.%06ld | Data func : ", tv_ptr->tv_sec, tv_ptr->tv_usec);
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
	perror("Error in dns resolution\n");
	return ERR_DNS;
    }

    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd < 0) {
	perror("Error creat socket\n");
	return ERR_SOCKET;
    }

    while (!g_signal) {
	init_ping_packet(&pkt, seq);
	printf("Loop\n");
	// printData(pkt);
	int bytes_send =
	    sendto(sockfd, &pkt, sizeof(pkt), 0, (struct sockaddr *)&sockaddr, sizeof(sockaddr));
	if (bytes_send < 0) {
	    perror("Error bytes send\n");
	    break;
	}
	char recv_buffer[128];
	struct sockaddr_in recv;
	socklen_t from_len = sizeof(recv);
	int bytes_recv = recvfrom(sockfd, recv_buffer, sizeof(recv_buffer), 0,
				  (struct sockaddr *)&recv, &from_len);
	if (bytes_recv < 0) {
	    perror("Error bytes recv\n");
	    break;
	}
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
