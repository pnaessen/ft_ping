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
    char buffer[65536];
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

	if (icmp->type == ICMP_ECHOREPLY) {

	    if (ntohs(icmp->un.echo.id) == (getpid() & 0xFFFF)) {

		double rtt = calculate_rtt(icmp);

		update_stats(ping, rtt);

		if (ping->flood) {
		    write(1, "\b \b", 3);
		} else {
		    print_packet_info(ping, ip, icmp, rtt, bytes);
		}
		return;
	    }
	}

	else if (icmp->type == ICMP_TIME_EXCEEDED || icmp->type == ICMP_DEST_UNREACH) {

	    struct iphdr *inner_ip = (struct iphdr *)((char *)icmp + 8);

	    struct icmphdr *inner_icmp = (struct icmphdr *)((char *)inner_ip + (inner_ip->ihl * 4));

	    if (ntohs(inner_icmp->un.echo.id) == (getpid() & 0xFFFF)) {

		if (ping->verbose) {
		    char sender[INET_ADDRSTRLEN];
		    inet_ntop(AF_INET, &addr.sin_addr, sender, sizeof(sender));

		    if (icmp->type == ICMP_TIME_EXCEEDED)
			printf("From %s: Time to live exceeded\n", sender);
		    else if (icmp->type == ICMP_DEST_UNREACH)
			printf("From %s: Destination Unreachable (Code %d)\n", sender, icmp->code);
		}

		return;
	    }

	}

	else if (ping->verbose) {

	    printf("Ignored ICMP packet type=%d code=%d from unknown source\n", icmp->type,
		   icmp->code);
	}
    }
}
