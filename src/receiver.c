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

	printf("%ld bytes from %s:  icmp_seq=%d\n", bytes, sender, ntohs(icmp->un.echo.sequence));
	printf("icmp_otime = %u\n", otime);
	printf("icmp_rtime = %u\n", rtime);
	printf("icmp_ttime = %u\n", ttime);
	if (ping->verbose) {
	    long long diff = (long long)rtime - (long long)otime;
	    printf("difference = %lld ms\n", diff);
	}
    } else {
	printf("Malformed Timestamp Reply received\n");
    }

    ping->stats.pkts_received++;
}

// static void process_time_exceeded(struct sockaddr_in *addr)
// {
//     char sender[INET_ADDRSTRLEN];
//     inet_ntop(AF_INET, &addr->sin_addr, sender, sizeof(sender));
//     // TODO: ADD BYTES and from _gateway
//     printf("From %s: Time to live exceeded\n", sender);
// }

int process_error_icmp(t_ping *ping, struct sockaddr_in *addr, struct icmphdr *icmp, ssize_t bytes)
{
    if (bytes < (ssize_t)(sizeof(struct icmphdr) + sizeof(struct iphdr) + 8))
	return 0;

    struct iphdr *inner_ip = (struct iphdr *)(icmp + 1);
    struct icmphdr *inner_icmp = (struct icmphdr *)((char *)inner_ip + (inner_ip->ihl * 4));

    if (ntohs(inner_icmp->un.echo.id) != (uint16_t)getpid())
	return 0;

    char sender[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &addr->sin_addr, sender, sizeof(sender));

    char host[NI_MAXHOST];
    int s = getnameinfo((struct sockaddr *)addr, sizeof(struct sockaddr_in), host, sizeof(host),
			NULL, 0, NI_NAMEREQD);
    if (s != 0)
	memcpy(host, "can't resolve dns", 18);
    printf("%ld bytes From %s (%s): ", bytes - sizeof(struct iphdr), host, sender);

    if (icmp->type == ICMP_TIME_EXCEEDED)
	printf("Time to live exceeded\n");

    if (ping->verbose) {

	if (icmp->type == ICMP_DEST_UNREACH)
	    printf("Destination Host Unreachable\n");
	else if (icmp->type == ICMP_TIME_EXCEEDED) {
	    print_packet_debug(inner_ip, inner_icmp);
	}
    }

    return 1;
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
	} else if (icmp->type == ICMP_TIME_EXCEEDED || icmp->type == ICMP_DEST_UNREACH) {
	    if (process_error_icmp(ping, &addr, icmp, bytes)) {
		return;
	    }
	}
	// else {
	// if (is_valid_id(icmp)) {
	//     process_error_icmp(ping, &addr, icmp, bytes);
	// 	return;
	// }
	// }
    }
}

int print_ip_dump(struct iphdr *ip)
{
    int hlen = ip->ihl * 4;
    unsigned short *ptr = (unsigned short *)ip;

    printf("IP Hdr Dump:\n ");
    for (int i = 0; i < hlen / 2; i++) {
	printf("%04x ", ntohs(ptr[i]));
    }
    printf("\n");
    return hlen;
}

void print_packet_debug(struct iphdr *ip, struct icmphdr *icmp)
{
    int ip_header_len = print_ip_dump(ip);

    uint16_t val_frag = ntohs(ip->frag_off);
    unsigned int flags = (val_frag >> 13) & 0x07;
    unsigned int offset = val_frag & 0x1FFF;
    char src[INET_ADDRSTRLEN], dst[INET_ADDRSTRLEN];

    inet_ntop(AF_INET, &ip->saddr, src, sizeof(src));
    inet_ntop(AF_INET, &ip->daddr, dst, sizeof(dst));

    printf("Vr HL TOS  Len   ID Flg  off TTL Pro  cks      Src      Dst     Data\n");
    printf(" %1x  %1x  %02x %04x %04x   %1x %04x  %02x  %02x %04x %-15s %-15s\n", ip->version,
	   ip->ihl, ip->tos, ntohs(ip->tot_len), ntohs(ip->id), flags, offset, ip->ttl,
	   ip->protocol, ntohs(ip->check), src, dst);

    int icmp_size = ntohs(ip->tot_len) - ip_header_len;

    printf("ICMP: type %u, code %u, size %u, id 0x%04x, seq 0x%04x\n", icmp->type, icmp->code,
	   icmp_size, ntohs(icmp->un.echo.id), ntohs(icmp->un.echo.sequence));
}
