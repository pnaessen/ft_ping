#include "ft_ping.h"

int main(int argc, char const **argv)
{
	if(argc != 2) {
		//TODO: better check args for flags
		dprintf(2, "Usage: %s <ip adress>\n", argv[0]);
		return 1;
	}



	if(resolve_dns(argv[1], ) != 0) {
		dprintf(2, "Error in dns resolution\n");
		return 1;
	}

	struct sockaddr_in sockaddr;
	struct addrinfo *addinfo;

	int status = getaddrinfo(argv[1], NULL, NULL, &addinfo);
	if(status == 0) {
		struct sockaddr_in* addr_in = (struct sockaddr_in*)addinfo->ai_addr;
		sockaddr.sin_addr.s_addr = addr_in->sin_addr.s_addr;
	}

	freeaddrinfo(addinfo);
	return 0;
}
