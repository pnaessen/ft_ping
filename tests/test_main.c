#include "ft_ping.h"

#include <fcntl.h>
#include <getopt.h>
#include <sys/wait.h>

static int g_failures = 0;

#define CHECK(condition)                                                                         \
    do {                                                                                         \
        if (!(condition)) {                                                                      \
            fprintf(stderr, "FAIL:%s:%d: %s\n", __FILE__, __LINE__, #condition);                \
            g_failures++;                                                                        \
        }                                                                                        \
    } while (0)

static int argv_len(char **argv)
{
    int len = 0;

    while (argv[len] != NULL)
        len++;
    return len;
}

static void reset_getopt_state(void)
{
    optind = 0;
    opterr = 0;
}

static int parse_with_args(char **argv, t_ping *ping)
{
    reset_getopt_state();
    return parse_args(argv_len(argv), argv, ping);
}

static void expect_parse_exit_failure(char **argv)
{
    pid_t pid = fork();

    CHECK(pid >= 0);
    if (pid == 0) {
        int devnull = open("/dev/null", O_WRONLY);

        if (devnull >= 0) {
            dup2(devnull, STDERR_FILENO);
            close(devnull);
        }

        t_ping ping;
        init_ping_struct(&ping);
        reset_getopt_state();
        parse_args(argv_len(argv), argv, &ping);
        _exit(EXIT_SUCCESS);
    }

    int status = 0;
    waitpid(pid, &status, 0);
    CHECK(WIFEXITED(status));
    CHECK(WEXITSTATUS(status) == EXIT_FAILURE);
}

static void test_init_ping_struct_defaults(void)
{
    t_ping ping;

    init_ping_struct(&ping);
    CHECK(ping.target_host == NULL);
    CHECK(ping.ttl == 64);
    CHECK(ping.verbose == false);
    CHECK(ping.count == -1);
    CHECK(ping.type == ICMP_ECHO);
    CHECK(ping.stats.pkts_transmitted == 0);
    CHECK(ping.stats.pkts_received == 0);
    CHECK(ping.stats.min_rtt == DBL_MAX);
    CHECK(ping.stats.max_rtt == 0.0);
}

static void test_parse_args_success(void)
{
    char *argv[] = {"ft_ping", "-v", "-c", "3", "--ttl", "42", "-t", "timestamp", "-w", "5",
                    "-p", "ABCD", "example.com", NULL};
    t_ping ping;

    init_ping_struct(&ping);
    CHECK(parse_with_args(argv, &ping) == EXIT_SUCCESS);
    CHECK(ping.verbose == true);
    CHECK(ping.count == 3);
    CHECK(ping.ttl == 42);
    CHECK(ping.type == ICMP_TIMESTAMP);
    CHECK(ping.deadline == 5);
    CHECK(ping.pattern_set == true);
    CHECK(ping.pattern_len == 2);
    CHECK(ping.pattern[0] == 0xAB);
    CHECK(ping.pattern[1] == 0xCD);
    CHECK(strcmp(ping.target_host, "example.com") == 0);
}

static void test_parse_args_pattern_prefix(void)
{
    char *argv[] = {"ft_ping", "-p", "0xABCD", "example.com", NULL};
    t_ping ping;

    init_ping_struct(&ping);
    CHECK(parse_with_args(argv, &ping) == EXIT_SUCCESS);
    CHECK(ping.pattern_len == 2);
    CHECK(ping.pattern[0] == 0xAB);
    CHECK(ping.pattern[1] == 0xCD);
}

static void test_parse_args_failures(void)
{
    char *missing_host[] = {"ft_ping", "-v", NULL};
    char *bad_count[] = {"ft_ping", "-c", "0", "example.com", NULL};
    char *bad_type[] = {"ft_ping", "-t", "address-mask", "example.com", NULL};
    char *bad_ttl[] = {"ft_ping", "--ttl", "300", "example.com", NULL};
    char *bad_pattern[] = {"ft_ping", "-p", "ABCG", "example.com", NULL};

    t_ping ping;

    init_ping_struct(&ping);
    CHECK(parse_with_args(missing_host, &ping) == EXIT_FAILURE);

    init_ping_struct(&ping);
    CHECK(parse_with_args(bad_count, &ping) == EXIT_FAILURE);

    init_ping_struct(&ping);
    CHECK(parse_with_args(bad_type, &ping) == EXIT_FAILURE);

    expect_parse_exit_failure(bad_ttl);
    expect_parse_exit_failure(bad_pattern);
}

static void test_parse_args_pattern_truncation(void)
{
    char *argv[] = {"ft_ping", "-p", "00112233445566778899AABBCCDDEEFF1234", "example.com", NULL};
    t_ping ping;

    init_ping_struct(&ping);
    CHECK(parse_with_args(argv, &ping) == EXIT_SUCCESS);
    CHECK(ping.pattern_len == MAX_PATTERN_LEN);
    CHECK(ping.pattern[0] == 0x00);
    CHECK(ping.pattern[MAX_PATTERN_LEN - 1] == 0xFF);
}

static void test_is_deadline_reached(void)
{
    t_ping ping;

    init_ping_struct(&ping);
    CHECK(is_deadline_reached(&ping, get_time_now() - 10.0) == false);

    ping.deadline = 1;
    CHECK(is_deadline_reached(&ping, get_time_now() - 2.0) == true);
}

static void test_calculate_checksum(void)
{
    unsigned char even_bytes[] = {0x00, 0x01, 0xF2, 0x03, 0xF4, 0xF5};
    unsigned char odd_bytes[] = {0x01, 0x02, 0x03};

    CHECK(calculate_checksum(even_bytes, sizeof(even_bytes)) == 0x0519);
    CHECK(calculate_checksum(odd_bytes, sizeof(odd_bytes)) == 0xFDFB);
}

static void test_calculate_rtt(void)
{
    const suseconds_t offset_usec = 250000;
    const double min_expected_rtt = 100.0;
    const double max_expected_rtt = 1000.0;
    struct {
        struct icmphdr hdr;
        struct timeval sent_at;
    } packet;
    struct timeval now;
    double rtt;

    memset(&packet, 0, sizeof(packet));
    gettimeofday(&now, NULL);
    packet.sent_at = now;
    if (packet.sent_at.tv_usec < offset_usec) {
        packet.sent_at.tv_sec -= 1;
        packet.sent_at.tv_usec += 1000000;
    }
    packet.sent_at.tv_usec -= offset_usec;

    rtt = calculate_rtt(&packet.hdr);
    CHECK(rtt >= min_expected_rtt);
    CHECK(rtt < max_expected_rtt);
}

static void test_update_stats(void)
{
    t_ping ping;

    init_ping_struct(&ping);
    update_stats(&ping, 10.0);
    update_stats(&ping, 20.0);
    CHECK(ping.stats.pkts_received == 2);
    CHECK(ping.stats.min_rtt == 10.0);
    CHECK(ping.stats.max_rtt == 20.0);
    CHECK(ping.stats.sum_rtt == 30.0);
    CHECK(ping.stats.sum_sq_rtt == 500.0);
}

static void test_init_ping_packet_echo_default_payload(void)
{
    t_ping ping;
    t_ping_packet pkt;
    struct timeval *sent_at;

    init_ping_struct(&ping);
    ping.seq = 7;
    init_ping_packet(&pkt, &ping);

    CHECK(pkt.hdr.type == ICMP_ECHO);
    CHECK(pkt.hdr.code == 0);
    CHECK(ntohs(pkt.hdr.un.echo.id) == (getpid() & 0xFFFF));
    CHECK(ntohs(pkt.hdr.un.echo.sequence) == 7);
    CHECK(pkt.hdr.checksum != 0);

    sent_at = (struct timeval *)pkt.msg;
    CHECK(sent_at->tv_sec > 0);
    for (size_t i = sizeof(struct timeval); i < PING_DATA_S; i++)
        CHECK((unsigned char)pkt.msg[i] == (unsigned char)i);
}

static void test_init_ping_packet_echo_pattern_payload(void)
{
    t_ping ping;
    t_ping_packet pkt;
    unsigned char pattern[] = {0xAA, 0xBB, 0xCC};

    init_ping_struct(&ping);
    ping.seq = 8;
    memcpy(ping.pattern, pattern, sizeof(pattern));
    ping.pattern_len = sizeof(pattern);
    ping.pattern_set = true;

    init_ping_packet(&pkt, &ping);

    for (size_t i = sizeof(struct timeval); i < PING_DATA_S; i++)
        CHECK((unsigned char)pkt.msg[i] ==
              pattern[(i - sizeof(struct timeval)) % sizeof(pattern)]);
}

static void test_init_ping_packet_timestamp_payload(void)
{
    t_ping ping;
    t_ping_packet pkt;
    uint32_t *payload;

    init_ping_struct(&ping);
    ping.type = ICMP_TIMESTAMP;

    init_ping_packet(&pkt, &ping);
    payload = (uint32_t *)pkt.msg;

    CHECK(pkt.hdr.type == ICMP_TIMESTAMP);
    CHECK(ntohl(payload[0]) < 86400000U);
    CHECK(payload[1] == 0);
    CHECK(payload[2] == 0);
}

static void test_resolve_dns_numeric_host(void)
{
    struct sockaddr_in dest;

    memset(&dest, 0, sizeof(dest));
    CHECK(resolve_dns("127.0.0.1", &dest) == SUCCESS);
    CHECK(dest.sin_family == AF_INET);
    CHECK(ntohl(dest.sin_addr.s_addr) == INADDR_LOOPBACK);
}

int main(void)
{
    test_init_ping_struct_defaults();
    test_parse_args_success();
    test_parse_args_pattern_prefix();
    test_parse_args_failures();
    test_parse_args_pattern_truncation();
    test_is_deadline_reached();
    test_calculate_checksum();
    test_calculate_rtt();
    test_update_stats();
    test_init_ping_packet_echo_default_payload();
    test_init_ping_packet_echo_pattern_payload();
    test_init_ping_packet_timestamp_payload();
    test_resolve_dns_numeric_host();

    if (g_failures != 0) {
        fprintf(stderr, "%d test(s) failed\n", g_failures);
        return EXIT_FAILURE;
    }

    printf("All tests passed\n");
    return EXIT_SUCCESS;
}
