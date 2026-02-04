#include "ft_ping.h"

bool g_signal = false;

void init_ping_struct(t_ping *ping, const char *host)
{
    memset(ping, 0, sizeof(t_ping));
    ping->target_host = (char *)host;
    ping->stats.min_rtt = DBL_MAX;
    ping->stats.max_rtt = 0.0;
}

int setup_socket(t_ping *ping)
{
    struct timeval tv = {1, 0};

    ping->sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (ping->sockfd < 0) {
	perror("socket");
	return -1;
    }

    if (setsockopt(ping->sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
	perror("setsockopt");
	close(ping->sockfd);
	return -1;
    }

    return 0;
}

int main(int argc, char const **argv)
{
    t_ping ping;

    if (argc != 2) {
	dprintf(2, "Usage: %s <host>\n", argv[0]);
	return 1;
    }

    signal(SIGINT, signalHandler);
    init_ping_struct(&ping, argv[1]);

    if (resolve_dns(ping.target_host, &ping.dest_addr) != 0)
	return ERR_DNS;

    inet_ntop(AF_INET, &ping.dest_addr.sin_addr, ping.target_ip, sizeof(ping.target_ip));

    if (setup_socket(&ping) < 0)
	return ERR_SOCKET;

    while (!g_signal) {
	if (send_ping(ping.sockfd, &ping.dest_addr, ping.seq) > 0) {
	    ping.stats.pkts_transmitted++;
	} else {
	    // TODO: print error or maybe not
	}

	handle_reception(&ping);
	ping.seq++;
	usleep(PING_INTERVAL);
    }

    print_final_stats(&ping);
    close(ping.sockfd);

    return SUCCESS;
}
