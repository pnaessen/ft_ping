#include "ft_ping.h"
#include <getopt.h>

#ifndef OPT_TTL
#define OPT_TTL 1000
#endif

static void parse_pattern_arg(const char *str, t_ping *ping)
{
    int len = strlen(str);
    int i = 0;
    int byte_idx = 0;

    if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X'))
	i = 2;

    while (str[i] && byte_idx < MAX_PATTERN_LEN) {
	unsigned int val;

	if (!isxdigit(str[i])) {
	    fprintf(stderr, "ft_ping: error: non-hex character in pattern\n");
	    exit(EXIT_FAILURE);
	}

	if (sscanf(&str[i], "%2x", &val) != 1)
	    break;

	ping->pattern[byte_idx] = (unsigned char)val;

	i += 2;

	if (i > len)
	    i = len;

	byte_idx++;
    }

    ping->pattern_len = byte_idx;
    ping->pattern_set = true;
}

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

int parse_args(int argc, char **argv, t_ping *ping)
{
    int opt;
    int option_index = 0;

    static struct option long_options[] = {{"count", required_argument, 0, 'c'},
					   {"pattern", required_argument, 0, 'p'},
					   {"help", no_argument, 0, 'h'},
					   {"type", required_argument, 0, 't'},
					   {"ttl", required_argument, 0, OPT_TTL},
					   {"timeout", required_argument, 0, 'w'},
					   {"verbose", no_argument, 0, 'v'},
					   {"help", no_argument, 0, '?'},
					   {0, 0, 0, 0}};

    while ((opt = getopt_long(argc, argv, "V?w:hc:t:p:", long_options, &option_index)) != -1) {

	switch (opt) {
	case 'h':
	    usage(argv[0]);
	    exit(EXIT_SUCCESS);

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
	    ping->ttl = get_int_arg(optarg, 0, 255, "--ttl");
	    break;
	}
	case 'w': {
	    ping->deadline = get_int_arg(optarg, 0, INT_MAX, "-w (timeout)");
	    break;
	}
	case 'p':
	    parse_pattern_arg(optarg, ping);
	    break;
	case 'v':
	    ping->verbose = true;
	    break;
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
    dprintf(2, "Usage: %s  [-t TYPE] [-c NUMBER] [--ttl NUMBER] <destination>\n", exec);
}
