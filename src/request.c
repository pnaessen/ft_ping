#include "ft_ping.h"

ssize_t send_ping(t_ping *ping)
{
    t_ping_packet pkt;
    int packet_size;

    init_ping_packet(&pkt, ping->seq, ping->type);

    if (ping->type == ICMP_TIMESTAMP) {
	packet_size = sizeof(struct icmphdr) + 12;
    } else {
	packet_size = sizeof(pkt);
    }

    int bytes_send = sendto(ping->sockfd, &pkt, packet_size, 0, (struct sockaddr *)&ping->dest_addr,
			    sizeof(ping->dest_addr));

    if (bytes_send < 0) {
	perror("Error bytes send");
	return -1;
    }
    return bytes_send;
}

void init_ping_packet(struct s_ping_packet *pkt, uint16_t seq, int type)
{
    memset(pkt, 0, sizeof(*pkt));

    pkt->hdr.type = type;
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
