#include "ft_ping.h"

double get_time_now()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + (tv.tv_usec / 1000000.0);
}

bool is_deadline_reached(t_ping *ping, double start_time)
{
    if (ping->deadline > 0) {
	double elapsed = get_time_now() - start_time;
	if (elapsed >= ping->deadline)
	    return true;
    }
    return false;
}

double calculate_rtt(struct icmphdr *icmp)
{
    struct timeval end_time;
    struct timeval *start_time;

    gettimeofday(&end_time, NULL);
    start_time = (struct timeval *)((char *)icmp + sizeof(struct icmphdr));

    return ((end_time.tv_sec - start_time->tv_sec) * 1000.0) +
	   ((end_time.tv_usec - start_time->tv_usec) / 1000.0);
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
