#include "ft_ping.h"

void handle_reception(int sockfd) {
    char recv_buffer[128];
    struct sockaddr_in socket_from;
    socklen_t from_len = sizeof(socket_from);

    ssize_t bytes = recvfrom(sockfd, recv_buffer, sizeof(recv_buffer), 0,
                             (struct sockaddr *)&socket_from, &from_len);

    if (bytes < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
			return;
        perror("recvfrom");
        return;
    }

    struct iphdr *ip = (struct iphdr *)recv_buffer;

    // Calcul de l'offset ICMP (IHL est en mots de 32 bits, donc * 4)
    int ip_hdr_len = ip->ihl * 4;
    struct icmphdr *icmp = (struct icmphdr *)(recv_buffer + ip_hdr_len);

    if (icmp->type == ICMP_ECHOREPLY) {
        if (ntohs(icmp->un.echo.id) == (getpid() & 0xFFFF)) {
            print_stats(ip, icmp, bytes);
        }
    } else if (icmp->type == ICMP_TIME_EXCEEDED) {
        printf("Time to live exceeded\n");
    }
}
