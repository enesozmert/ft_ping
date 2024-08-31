#include "header.h"

int main(int argc, char *argv[])
{
    int verbose_flag = 0;
    t_ping_result result = {0, 0, 0.0};

    // Parse command line arguments
    parse_args(argc, argv, &verbose_flag);

    // Get target IP address
    if (optind >= argc)
    {
        fprintf(stderr, "Usage: %s [-v] [-?] <hostname>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    const char *ip_addr = argv[optind];

    // Start ping process
    if (create_socket(ip_addr, &result) != 0)
    {
        fprintf(stderr, "Ping request failed.\n");
        return 1;
    }

    // Display results
    printf("\n--- %s ping statistics ---\n", ip_addr);
    printf("%d packets transmitted, %d received, %.2f%% packet loss, time %.2fms\n",
           result.sent_packets, result.received_packets,
           ((result.sent_packets - result.received_packets) / (double)result.sent_packets) * 100,
           result.rtt);

    return 0;
}