#include "ft_ping.h"

double calculate_rtt(struct icmphdr *icmp)
{
    struct timeval end_time;
    struct timeval *start_time;

    gettimeofday(&end_time, NULL);
    start_time = (struct timeval *)((char *)icmp + sizeof(struct icmphdr));

    return ((end_time.tv_sec - start_time->tv_sec) * 1000.0) +
	   ((end_time.tv_usec - start_time->tv_usec) / 1000.0);
}

void print_packet_info(t_ping *ping, struct iphdr *ip, struct icmphdr *icmp, double rtt,
		       ssize_t bytes)
{
    printf("%ld bytes from %s: icmp_seq=%d ttl=%d time=%.3f ms\n", bytes - (ip->ihl * 4),
	   ping->target_ip, ntohs(icmp->un.echo.sequence), ip->ttl, rtt);
}

void print_final_stats(t_ping *ping)
{
    printf("\n--- %s ping statistics ---\n", ping->target_host);

    long lost = ping->stats.pkts_transmitted - ping->stats.pkts_received;
    int loss_percent = (ping->stats.pkts_transmitted == 0)
			   ? 0
			   : (int)((lost * 100) / ping->stats.pkts_transmitted);

    printf("%ld packets transmitted, %ld packets received, %d%% packet loss\n",
	   ping->stats.pkts_transmitted, ping->stats.pkts_received, loss_percent);

    if (ping->stats.pkts_received > 0 && ping->type == ICMP_ECHO) {
	double avg = ping->stats.sum_rtt / ping->stats.pkts_received;

	double variance = (ping->stats.sum_sq_rtt / ping->stats.pkts_received) - (avg * avg);
	double mdev = sqrt(variance);

	printf("round-trip min/avg/max/stddev = %.3f/%.3f/%.3f/%.3f ms\n", ping->stats.min_rtt, avg,
	       ping->stats.max_rtt, mdev);
    }
}

void print_help()
{

    printf("\n Usage: ping [OPTION...] HOST ...\n");
    printf("Send ICMP ECHO_REQUEST packets to network hosts.\n\n");
    printf("Options controlling ICMP request types:\n");
    printf("    --timestamp     send ICMP_TIMESTAMP packets\n");
    printf("    --echo          send ICMP_ECHO packets (default)\n");
    printf("-t, --type=TYPE     send TYPE packets\n\n");
    printf("Options valide for all request types:\n\n");
    printf("-c, --count=NUMBER         stop after sending NUMBER packets\n");
    printf("-w, --timeout=N            stop after N seconds\n");
    printf("-p, --pattern=PATTERN      fill ICMP packet with given pattern (hex)\n");
    printf("-v, --verbose              verbose output\n");
    printf("--ttl, -ttl                set IP headears to N ttl\n");
    printf("-?, --help                 give this help list\n\n");
    printf("Mandatory or optional arguments to long options are also mandatory or optional\n");
    printf("for any corresponding short options.\n\n");
    printf("Options marked with (root only) are available only to superuser.\n\n");
    printf("Report bugs to <bocal@42.lyon.fr>.");

}

void print_ping_header(t_ping *ping)
{
    if (ping->type == ICMP_TIMESTAMP) {
	printf("PING %s (%s): sending timestamp requests", ping->target_host, ping->target_ip);
    } else {
	printf("PING %s (%s): %ld data bytes", ping->target_host, ping->target_ip, PING_DATA_S);
    }
    if(ping->verbose && ping->type == ICMP_ECHO) {
        printf("id 0x%4x = %d", getpid(), getpid());
    }
    printf("\n");
}
