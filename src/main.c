#include "ft_ping.h"

bool g_signal = false;

void init_ping_struct(t_ping *ping)
{
    memset(ping, 0, sizeof(t_ping));
    ping->target_host = NULL;
    ping->stats.min_rtt = DBL_MAX;
    ping->stats.max_rtt = 0.0;

    ping->ttl = 64;
    ping->verbose = false;
    ping->count = -1;
    ping->type = ICMP_ECHO;
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

    if (setsockopt(ping->sockfd, IPPROTO_IP, IP_TTL, &ping->ttl, sizeof(ping->ttl)) < 0) {
	perror("setsockopt ttl");
	close(ping->sockfd);
	return -1;
    }
    return 0;
}

int setup_target_and_socket(t_ping *ping)
{
    if (setup_socket(ping) < 0)
	return ERR_SOCKET;

    if (resolve_dns(ping->target_host, &ping->dest_addr) != 0) {
	close(ping->sockfd);
	return ERR_DNS;
    }

    inet_ntop(AF_INET, &ping->dest_addr.sin_addr, ping->target_ip, sizeof(ping->target_ip));

    return SUCCESS;
}

int main(int argc, char **argv)
{
    t_ping ping;
    double start_time;

    signal(SIGINT, signalHandler);
    init_ping_struct(&ping);

    if (parse_args(argc, argv, &ping) != 0) {
	usage(argv[0]);
	return EXIT_FAILURE;
    }

    if (setup_target_and_socket(&ping) != SUCCESS)
	return EXIT_FAILURE;

    print_ping_header(&ping);

    start_time = get_time_now();

    while (!g_signal) {
	if (is_deadline_reached(&ping, start_time))
	    break;

	if (send_ping(&ping) > 0) {
	    ping.stats.pkts_transmitted++;
	    ping.seq++;
	} else {
	    // fprintf(stderr, "sendto error\n");
	}

	handle_reception(&ping);

	if (ping.count > 0 && ping.stats.pkts_transmitted >= ping.count)
	    break;

	if (is_deadline_reached(&ping, start_time))
	    break;
	usleep(PING_INTERVAL);
    }

    print_final_stats(&ping);
    close(ping.sockfd);

    return SUCCESS;
}
