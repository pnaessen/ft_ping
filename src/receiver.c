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
	} else if (icmp->type == ICMP_TIMESTAMP) {

	    if (ntohs(icmp->un.echo.id) != (getpid() & 0xFFFF))
		continue;

	    char sender[INET_ADDRSTRLEN];
	    inet_ntop(AF_INET, &addr.sin_addr, sender, sizeof(sender));

	    int data_len = bytes - (ip->ihl * 4) - sizeof(struct icmphdr);

	    if (data_len >= 12) {
		uint32_t *ptr = (uint32_t *)((char *)icmp + sizeof(struct icmphdr));

		uint32_t otime = ntohl(ptr[0]);
		uint32_t rtime = ntohl(ptr[1]);
		uint32_t ttime = ntohl(ptr[2]);

		printf("%ld bytes from %s: type=TIMESTAMP_REPLY\n", bytes - (ip->ihl * 4), sender);
		printf("icmp_otime = %u\n", otime);
		printf("icmp_rtime = %u\n", rtime);
		printf("icmp_ttime = %u\n", ttime);
	    } else {
		printf("Malformed Timestamp Reply received\n");
	    }

	    ping->stats.pkts_received++;
	    return;

	} else if (icmp->type == ICMP_TIME_EXCEEDED) {

	    char sender[INET_ADDRSTRLEN];
	    inet_ntop(AF_INET, &addr.sin_addr, sender, sizeof(sender));
	    printf("From %s: Time to live exceeded\n", sender);

	    return;
	}
	printf("icmp type resp:%d\n ",icmp->type);
    }
}
