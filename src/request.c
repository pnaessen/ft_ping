#include "ft_ping.h"

ssize_t send_ping(t_ping *ping)
{
    t_ping_packet pkt;
    int packet_size;

    init_ping_packet(&pkt, ping->seq, ping->type);

    if (ping->type == ICMP_TIMESTAMP) {
	packet_size = sizeof(struct icmphdr) + PAYLOAD_TIMESTAMP;
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

static void fill_timestamp_payload(t_ping_packet *pkt)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);

    uint32_t ms_since_midnight = (tv.tv_sec % 86400) * 1000 + (tv.tv_usec / 1000);

    uint32_t *ptr = (uint32_t *)pkt->msg;
    ptr[0] = htonl(ms_since_midnight);
    ptr[1] = 0;
    ptr[2] = 0;
}

static void fill_echo_payload(t_ping_packet *pkt)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);

    memcpy(pkt->msg, &tv, sizeof(tv));

    for (size_t i = sizeof(struct timeval); i < PING_DATA_S; i++) {
	pkt->msg[i] = (char)i;
    }
}

void init_ping_packet(struct s_ping_packet *pkt, uint16_t seq, int type)
{
    memset(pkt, 0, sizeof(*pkt));

    pkt->hdr.type = type;
    pkt->hdr.code = 0;
    pkt->hdr.un.echo.id = htons(getpid() & 0xFFFF);
    pkt->hdr.un.echo.sequence = htons(seq);

    size_t total_size;

    if (type == ICMP_TIMESTAMP) {
	fill_timestamp_payload(pkt);
	total_size = sizeof(struct icmphdr) + PAYLOAD_TIMESTAMP;
    } else {
	fill_echo_payload(pkt);
	total_size = sizeof(*pkt);
    }

    pkt->hdr.checksum = calculate_checksum(pkt, total_size);
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
