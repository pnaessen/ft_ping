#include "ft_ping.h"

bool g_signal = false;

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
