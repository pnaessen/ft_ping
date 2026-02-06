#include "ft_ping.h"
#include <getopt.h>

#ifndef OPT_TTL
#define OPT_TTL 1000
#endif

int parse_args(int argc, char **argv, t_ping *ping)
{
    int opt;
    int option_index = 0;

    static struct option long_options[] = {{"count", required_argument, 0, 'c'},
					   {"type", required_argument, 0, 't'},
					   {"ttl", required_argument, 0, OPT_TTL},
					   {0, 0, 0, 0}};

    while ((opt = getopt_long(argc, argv, "c:t:", long_options, &option_index)) != -1) {

		printf("%d\n", opt);
	switch (opt) {
	case 'h':
	    usage(argv[0]);
	    exit(EXIT_SUCCESS);

	case 'v':
	    ping->verbose = true;
	    break;

	case 'c': {
	    char *endptr;
	    long val = strtol(optarg, &endptr, 10);
	    if (*endptr != '\0' || val <= 0) {
		fprintf(stderr, "ft_ping: bad number of packets to transmit: %s\n", optarg);
		return EXIT_FAILURE;
	    }
	    ping->count = (int)val;
	    break;
	}

	case 't':
	    if (strcmp(optarg, "echo") == 0)
		ping->type = ICMP_ECHO;
	    else if (strcmp(optarg, "timestamp") == 0)
		ping->type = ICMP_TIMESTAMP;
	    else {
		fprintf(stderr, "ft_ping: unsupported packet type: %s\n", optarg);
		return EXIT_FAILURE;
	    }
	    break;

	case OPT_TTL: {
	    char *endptr;
	    long val = strtol(optarg, &endptr, 10);
	    if (*endptr != '\0' || val < 0 || val > 255) {
		fprintf(stderr, "ft_ping: invalid TTL: %s\n", optarg);
		return EXIT_FAILURE;
	    }
	    ping->ttl = (int)val;
	    break;
	}

	case '?':
	    return EXIT_FAILURE;
	}
    }

    if (optind >= argc) {
	fprintf(stderr, "%s: missing host operand\n", argv[0]);
	return EXIT_FAILURE;
    }
    ping->target_host = argv[optind];

    return EXIT_SUCCESS;
}

void usage(const char *exec)
{
    dprintf(2, "Usage: %s  [-t TYPE] [-c NUMBER] <destination>\n", exec);
}
