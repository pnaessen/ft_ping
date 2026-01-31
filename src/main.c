#include "ft_ping.h"

int main(int argc, char const **argv)
{
	if(argc != 2) {
		dprintf(2, "Usage: %s <ip adress>\n", argv[0]);
		return 1;
	}

	return 0;
}
