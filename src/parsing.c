#include "ft_ping.h"

int parse_args(int argc, char **argv, t_ping *ping) {

	int opt;

	ping->ttl = 64;
	ping->verbose = false;
	ping->count = 0;

	while ((opt = getopt(argc, argv, "vt:c:")) < 0) {

	}

	return 1;
}

void usage(const char *exec) {
	dprintf(2, "Usage: %s [-v] [-t TYPE] [-c NUMBER] <destination>\n", exec);
}
