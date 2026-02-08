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

static bool is_valid_id(struct icmphdr *icmp)
{
    return ntohs(icmp->un.echo.id) == (getpid() & 0xFFFF);
}

static void process_echo_reply(t_ping *ping, struct iphdr *ip, struct icmphdr *icmp, ssize_t bytes)
{
    double rtt = calculate_rtt(icmp);

    update_stats(ping, rtt);
    print_packet_info(ping, ip, icmp, rtt, bytes);
}

static void process_timestamp_reply(t_ping *ping, struct iphdr *ip, struct icmphdr *icmp,
				    ssize_t bytes, struct sockaddr_in *addr)
{
    char sender[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &addr->sin_addr, sender, sizeof(sender));

    int data_len = bytes - (ip->ihl * 4) - sizeof(struct icmphdr);

    if (data_len >= PAYLOAD_TIMESTAMP) {
	uint32_t *ptr = (uint32_t *)((char *)icmp + sizeof(struct icmphdr));
	// uint32_t *ptr = (uint32_t *)(icmp + 1);

	uint32_t otime = ntohl(ptr[0]);
	uint32_t rtime = ntohl(ptr[1]);
	uint32_t ttime = ntohl(ptr[2]);

	printf("%ld bytes from %s: type=TIMESTAMP_REPLY icmp_seq=%d\n", bytes, sender,
	       ntohs(icmp->un.echo.sequence));
	printf("icmp_otime = %u\n", otime);
	printf("icmp_rtime = %u\n", rtime);
	printf("icmp_ttime = %u\n", ttime);
    } else {
	printf("Malformed Timestamp Reply received\n");
    }

    ping->stats.pkts_received++;
}

static void process_time_exceeded(struct sockaddr_in *addr)
{
    char sender[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &addr->sin_addr, sender, sizeof(sender));
	//TODO: ADD BYTES and from _gateway
    printf("From %s: Time to live exceeded\n", sender);
}

void handle_reception(t_ping *ping)
{
    char buffer[IP_MAXPACKET];
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
	    if (is_valid_id(icmp)) {
		process_echo_reply(ping, ip, icmp, bytes);
		return;
	    }
	} else if (icmp->type == ICMP_TIMESTAMPREPLY) {
	    if (is_valid_id(icmp)) {
		process_timestamp_reply(ping, ip, icmp, bytes, &addr);
		return;
	    }
	} else if (icmp->type == ICMP_TIME_EXCEEDED) {
	    process_time_exceeded(&addr);
	    return;
	}
    }
}
