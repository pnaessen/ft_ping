#include "ft_ping.h"

int parse_args(int argc, char **argv, t_ping *ping)
{

    int opt;

    while ((opt = getopt(argc, argv, "c:")) != -1) {

	printf("opt = %d\n", opt);
	switch (opt) {
	case 'c': {
	    char *endptr;
	    long val = strtol(optarg, &endptr, 10);
	    if (*endptr != '\0' || val <= 0) {
		fprintf(stderr, "ft_ping: bad number of packet send %s\n", optarg);
		return EXIT_FAILURE;
	    }
	    ping->count = val;
	    printf("count = %d\n", ping->count);
	    break;
	}
	case '?':
	    return EXIT_FAILURE;
	}
    }
    ping->target_host = argv[optind];
    return EXIT_SUCCESS;
}

void usage(const char *exec)
{
    dprintf(2, "Usage: %s [-v] [-t TYPE] [-c NUMBER] <destination>\n", exec);
}
