#include "ft_ping.h"

void update_stats(t_ping *ping, double rtt)
{
    ping->stats.pkts_received++;
    ping->stats.sum_rtt += rtt;
    ping->stats.sum_sq_rtt += (rtt * rtt);

    if (rtt < ping->stats.min_rtt)
	ping->stats.min_rtt = rtt;
    if (rtt > ping->stats.max_rtt)
	ping->stats.max_rtt = rtt;
}

void handle_reception(t_ping *ping)
{
    char buffer[512];
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);
    ssize_t bytes;

    while (true) {
	bytes =
	    recvfrom(ping->sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&addr, &addr_len);

	if (bytes < 0) {
	    if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
		return;
	    perror("recvfrom");
	    return;
	}

	struct iphdr *ip = (struct iphdr *)buffer;
	struct icmphdr *icmp = (struct icmphdr *)(buffer + (ip->ihl * 4));

	if (icmp->type == ICMP_ECHOREPLY && ntohs(icmp->un.echo.id) == (getpid() & 0xFFFF)) {
	    double rtt = calculate_rtt(icmp);

	    update_stats(ping, rtt);
	    print_packet_info(ping, ip, icmp, rtt, bytes);
	    return;
	} else if (icmp->type == ICMP_TIME_EXCEEDED) {

	    char sender[INET_ADDRSTRLEN];
	    inet_ntop(AF_INET, &addr.sin_addr, sender, sizeof(sender));
	    printf("From %s: Time to live exceeded\n", sender);

	    return;
	}
    }
}
