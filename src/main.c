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

	printf("Loop\n");
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
    return SUCCESS;
}
