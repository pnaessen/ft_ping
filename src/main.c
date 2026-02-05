#include "ft_ping.h"

bool g_signal = false;

void init_ping_struct(t_ping *ping)
{
    memset(ping, 0, sizeof(t_ping));

    ping->target_host = NULL;
    ping->ttl = 64;
    ping->packet_size = 56;
    ping->count = -1;
    ping->timeout = -1;
    ping->interval = 1.0;
    ping->verbose = false;
    ping->flood = false;

    ping->stats.min_rtt = DBL_MAX;
    ping->stats.max_rtt = 0.0;
    ping->stats.pkts_transmitted = 0;
    ping->stats.pkts_received = 0;
}

int setup_socket(t_ping *ping)
{
    struct timeval tv = {1, 0};

    ping->sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (ping->sockfd < 0) {
	perror("socket");
	return -1;
    }

    if (setsockopt(ping->sockfd, IPPROTO_IP, IP_TTL, &ping->ttl, sizeof(ping->ttl)) < 0) {
	perror("setsockopt IP_TTL");
	close(ping->sockfd);
	return -1;
    }

    if (setsockopt(ping->sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
	perror("setsockopt SO_RCVTIMEO");
	close(ping->sockfd);
	return -1;
    }

    return 0;
}

int main(int argc, char **argv)
{
    t_ping ping;

    if (getuid() != 0) {
	fprintf(stderr, "ft_ping: Lacking privilege for raw socket.\n");
	return (EXIT_FAILURE);
    }

    init_ping_struct(&ping);

    if (parse_args(argc, argv, &ping) != 0) {
	// usage();
	return (EXIT_FAILURE);
    }

    signal(SIGINT, signalHandler);

    if (ping.timeout > 0) {
	signal(SIGALRM, signalHandler);
	alarm(ping.timeout);
    }

    if (resolve_dns(ping.target_host, &ping.dest_addr) != 0) {
	return (ERR_DNS);
    }

    inet_ntop(AF_INET, &ping.dest_addr.sin_addr, ping.target_ip, sizeof(ping.target_ip));

    if (setup_socket(&ping) < 0) {
	return (ERR_SOCKET);
    }

    while (!g_signal) {
	if (send_ping(ping.sockfd, &ping.dest_addr, &ping) > 0) {
	    ping.stats.pkts_transmitted++;
	} else {
	    // Error
	}

	handle_reception(&ping);

	if (ping.count > 0 && ping.stats.pkts_transmitted >= ping.count) {
	    break;
	}
	ping.seq++;

	if (!ping.flood && !g_signal) {
	    usleep((useconds_t)(ping.interval * 1000000));
	}
    }

    print_final_stats(&ping);
    close(ping.sockfd);

    return (SUCCESS);
}
