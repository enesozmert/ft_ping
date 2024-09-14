#include "header.h"

t_ping *g_ping;

int main(int argc, char *argv[])
{
    t_ping *ping;
    int verbose_flag;
    int allocates_error;

    ping = NULL;
    g_ping = NULL;
    verbose_flag = 0;
    // Parse command line arguments
    parse_args(argc, argv, &verbose_flag);

    // Get target IP address
    if (optind >= argc)
    {
        fprintf(stderr, "Usage: %s [-v] [-?] <hostname>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    allocates_error = ping_create_allocates(&ping);

    if (allocates_error < 0){
        return -1;
    }

    char *hostname = argv[optind];
    char ip_addr[INET_ADDRSTRLEN];

    // Resolve hostname
    if (resolve_hostname(hostname, ip_addr, sizeof(ip_addr)) != 0) {
        return 1;
    }

    ping->sock_fd = 0;
    ping->dest_ip_addr = ip_addr;
    ping->dest_hostname = hostname;

    // Start ping process
    if (create_socket(ping) != 0)
    {
        fprintf(stderr, "ft_ping: Ping request failed.\n");
        return 1;
    }

    g_ping = ping;
    signal(SIGINT, interrupt_handler);

    return 0;
}