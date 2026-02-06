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
