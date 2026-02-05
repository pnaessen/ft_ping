#include "ft_ping.h"

ssize_t send_ping(int sockfd, struct sockaddr_in *dest, t_ping *ping)
{

    uint8_t packet[IP_MAXPACKET];
    struct icmphdr *icmp;

    size_t total_size = sizeof(struct icmphdr) + ping->packet_size;

    if (total_size > sizeof(packet)) {
	fprintf(stderr, "ft_ping: packet size too large\n");
	return -1;
    }

    icmp = (struct icmphdr *)packet;

    icmp->type = ICMP_ECHO;
    icmp->code = 0;
    icmp->un.echo.id = htons(getpid() & 0xFFFF);
    icmp->un.echo.sequence = htons(ping->seq);
    icmp->checksum = 0;

    char *data = (char *)(packet + sizeof(struct icmphdr));

    for (int i = 0; i < ping->packet_size; i++) {
	data[i] = i % 256;
    }

    if (ping->packet_size >= (int)sizeof(struct timeval)) {
	struct timeval now;
	gettimeofday(&now, NULL);
	memcpy(data, &now, sizeof(now));
    }

    icmp->checksum = calculate_checksum(packet, total_size);

    ssize_t bytes_sent =
	sendto(sockfd, packet, total_size, 0, (struct sockaddr *)dest, sizeof(*dest));

    if (bytes_sent < 0) {
	if (ping->verbose)
	    perror("ft_ping: sendto");
	return -1;
    }

    return bytes_sent;
}

void init_ping_packet(struct s_ping_packet *pkt, uint16_t seq)
{
    memset(pkt, 0, sizeof(*pkt));

    pkt->hdr.type = ICMP_ECHO;
    pkt->hdr.code = 0;
    pkt->hdr.un.echo.id = htons(getpid() & 0xFFFF);
    pkt->hdr.un.echo.sequence = htons(seq);

    struct timeval tv;
    gettimeofday(&tv, NULL);
    memcpy(pkt->msg, &tv, sizeof(tv));

    for (size_t i = sizeof(struct timeval); i < PING_DATA_S; i++) {
	pkt->msg[i] = i;
    }

    pkt->hdr.checksum = 0;
    pkt->hdr.checksum = calculate_checksum(pkt, sizeof(*pkt));
}

uint16_t calculate_checksum(void *addr, int len)
{
    uint32_t sum = 0;
    uint16_t *ptr = addr;

    while (len > 1) {
	sum += *ptr++;
	len -= 2;
    }

    if (len == 1) {
	sum += *(uint8_t *)ptr;
    }

    while (sum >> 16) {
	sum = (sum & 0xffff) + (sum >> 16);
    }

    return (uint16_t)~sum;
}
