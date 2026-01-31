#ifndef FT_PING_HPP
# define FT_PING_HPP

# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>


typedef struct s_ipHeader
{
	/* data */
}				t_ipHeader;


typedef struct s_icmp_packet
{
	uint8_t  type;
    uint8_t  code;
    uint16_t checksum;
    uint16_t id;
    uint16_t sequence;
}			   t_icmp_packet;

int resolve_dns(const char *host, struct sockaddr_in *dest);

#endif /* FT_PING_H */
