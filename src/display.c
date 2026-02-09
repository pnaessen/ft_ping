#include "ft_ping.h"

void print_packet_info(t_ping *ping, struct iphdr *ip, struct icmphdr *icmp, double rtt,
		       ssize_t bytes)
{
    printf("%ld bytes from %s: icmp_seq=%d ttl=%d time=%.3f ms\n", bytes - (ip->ihl * 4),
	   ping->target_ip, ntohs(icmp->un.echo.sequence), ip->ttl, rtt);
}

void print_final_stats(t_ping *ping)
{
    printf("\n--- %s ping statistics ---\n", ping->target_host);

    long lost = ping->stats.pkts_transmitted - ping->stats.pkts_received;
    int loss_percent = (ping->stats.pkts_transmitted == 0)
			   ? 0
			   : (int)((lost * 100) / ping->stats.pkts_transmitted);

    printf("%ld packets transmitted, %ld packets received, %d%% packet loss\n",
	   ping->stats.pkts_transmitted, ping->stats.pkts_received, loss_percent);

    if (ping->stats.pkts_received > 0 && ping->type == ICMP_ECHO) {
	double avg = ping->stats.sum_rtt / ping->stats.pkts_received;

	double variance = (ping->stats.sum_sq_rtt / ping->stats.pkts_received) - (avg * avg);
	double mdev = sqrt(variance);

	printf("round-trip min/avg/max/stddev = %.3f/%.3f/%.3f/%.3f ms\n", ping->stats.min_rtt, avg,
	       ping->stats.max_rtt, mdev);
    }
}

void print_help()
{

    printf("\n Usage: ping [OPTION...] HOST ...\n");
    printf("Send ICMP ECHO_REQUEST packets to network hosts.\n\n");
    printf("Options controlling ICMP request types:\n");
    printf("    --timestamp     send ICMP_TIMESTAMP packets\n");
    printf("    --echo          send ICMP_ECHO packets (default)\n");
    printf("-t, --type=TYPE     send TYPE packets\n\n");
    printf("Options valide for all request types:\n\n");
    printf("-c, --count=NUMBER         stop after sending NUMBER packets\n");
    printf("-w, --timeout=N            stop after N seconds\n");
    printf("-p, --pattern=PATTERN      fill ICMP packet with given pattern (hex)\n");
    printf("-v, --verbose              verbose output\n");
    printf("--ttl, -ttl                set IP headears to N ttl\n");
    printf("-?, --help                 give this help list\n\n");
    printf("Mandatory or optional arguments to long options are also mandatory or optional\n");
    printf("for any corresponding short options.\n\n");
    printf("Options marked with (root only) are available only to superuser.\n\n");
    printf("Report bugs to <bocal@42.lyon.fr>.");
}

void print_ping_header(t_ping *ping)
{
    if (ping->type == ICMP_TIMESTAMP) {
	printf("PING %s (%s): sending timestamp requests", ping->target_host, ping->target_ip);
    } else {
	printf("PING %s (%s): %ld data bytes", ping->target_host, ping->target_ip, PING_DATA_S);
    }
    if (ping->verbose && ping->type == ICMP_ECHO) {
	printf("id 0x%4x = %d", getpid(), getpid());
    }
    printf("\n");
}

void usage(const char *exec)
{
    dprintf(2,
	    "Usage: %s [-p HEXA_PATTERN] [-w NUMBER] [--help] [-t TYPE] [-c NUMBER] [--ttl NUMBER] "
	    "<destination>\n",
	    exec);
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
