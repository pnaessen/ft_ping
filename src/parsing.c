#include "ft_ping.h"
#include <getopt.h>
#define OPT_TTL 1000

int parse_args(int argc, char **argv, t_ping *ping)
{

    int opt;
    // int option_index = 0;

    // static struct option long_options[] = {{"help", no_argument, 0, 'h'},
    // 				   {"count", required_argument, 0, 'c'},
    // 				   {"type", required_argument, 0, 't'},
    // 				   {"ttl", required_argument, 0, OPT_TTL},
    // 				   {0, 0, 0, 0}};

    while ((opt = getopt(argc, argv, "c:t:")) != -1) {

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
	case 't':
	    if (strcmp(optarg, "echo") == 0)
		ping->type = ICMP_ECHO;
	    else if (strcmp(optarg, "timestamp") == 0)
		ping->type = ICMP_TIMESTAMP;
	    else {
		fprintf(stderr, "ft_ping: unsupported packet type: %s\n", optarg);
		return (EXIT_FAILURE);
	    }
	    break;
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
