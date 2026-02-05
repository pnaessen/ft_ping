#include "ft_ping.h"
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static int get_int_arg(const char *str, int min, int max, const char *flag_name)
{
    char *endptr;
    long val;

    errno = 0;
    val = strtol(str, &endptr, 10);

    if (errno != 0 || *endptr != '\0' || str == endptr) {
	fprintf(stderr, "ft_ping: invalid argument: '%s'\n", str);
	exit(EXIT_FAILURE);
    }

    if (val < min || val > max) {
	fprintf(stderr, "ft_ping: invalid value for %s: %ld out of range [%d, %d]\n", flag_name,
		val, min, max);
	exit(EXIT_FAILURE);
    }
    return (int)val;
}

int parse_args(int ac, char **av, t_ping *ping)
{
    int opt;

    while ((opt = getopt(ac, av, "vfrc:t:s:w:")) != -1) {
	switch (opt) {
	case 'v':
	    ping->verbose = true;
	    break;

	case 'f':
	    ping->flood = true;
	    ping->interval = 0.0;
	    break;

	case 'r':
	    ping->record_route = true;
	    break;

	case 'c': {
	    char *endptr;
	    long val = strtol(optarg, &endptr, 10);
	    if (*endptr != '\0' || val <= 0) {
		fprintf(stderr, "ft_ping: bad number of packets to transmit: %s\n", optarg);
		return (EXIT_FAILURE);
	    }
	    ping->count = val;
	} break;

	case 't':
	    ping->ttl = get_int_arg(optarg, 0, 255, "-t (ttl)");
	    break;

	case 's':

	    ping->packet_size = get_int_arg(optarg, 0, 65507, "-s (size)");
	    break;

	case 'w':
	    ping->timeout = get_int_arg(optarg, 0, INT_MAX, "-w (deadline)");
	    break;

	case '?':
	    // usage()
	    return (EXIT_FAILURE);
	}
    }

    if (optind == ac) {
	fprintf(stderr, "ft_ping: usage error: destination address required\n");
	return (EXIT_FAILURE);
    }

    ping->target_host = av[optind];

    return (EXIT_SUCCESS);
}

void usage(const char *exec)
{
    dprintf(2, "Usage: %s [-v] [-t TYPE] [-c NUMBER] <destination>\n", exec);
}
