#include "ft_ping.h"

ssize_t send_ping(int sockfd, struct sockaddr_in *dest, uint16_t seq)
{
    t_ping_packet pkt;
    init_ping_packet(&pkt, seq);
    int bytes_send = sendto(sockfd, &pkt, sizeof(pkt), 0, (struct sockaddr *)dest, sizeof(*dest));
    if (bytes_send < 0) {
	perror("Error bytes send\n");
	return -1;
    }
    return bytes_send;
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
