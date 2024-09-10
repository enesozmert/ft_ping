#include "header.h"

int main(int argc, char *argv[])
{
    t_ping *ping;
    int verbose_flag = 0;

    ping = malloc(sizeof(t_ping));
    if (!ping) {
        perror("Failed to allocate memory for t_ping");
        return 0;
    }

    ping->result = malloc(sizeof(t_ping_result));
    if (!ping->result) {
        perror("Failed to allocate memory for t_ping_result");
        free(ping); // Ayırılan belleği serbest bırakıyoruz
        return 0;
    }

    ping->target_addr = (struct sockaddr_ll *)malloc(sizeof(struct sockaddr_ll));
    if (!ping->target_addr)
    {
        perror("Memory Allocation Error for target_addr!");
        exit(1); // Hata durumunda çıkış yapmadan önce programdaki dinamik bellekler serbest bırakılmalı
    }

    ping->ifreq = (struct ifreq *)malloc(sizeof(struct ifreq));
    if (!ping->ifreq) {
        perror("Failed to allocate memory for ifreq");
        close(ping->sock_fd);
        free(ping);
        return 1;
    }

    ping->payload = (t_payload *)malloc(sizeof(t_payload));
    if (!ping->payload) {
        perror("Failed to allocate memory for t_payload");
        close(ping->sock_fd);
        free(ping);
        return 1;
    }

    // Parse command line arguments
    parse_args(argc, argv, &verbose_flag);

    // Get target IP address
    if (optind >= argc)
    {
        fprintf(stderr, "Usage: %s [-v] [-?] <hostname>\n", argv[0]);
        exit(EXIT_FAILURE);
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
    ping->src_ip_addr = get_source_ip_address();

    // Start ping process
    if (create_socket(ping) != 0)
    {
        fprintf(stderr, "ft_ping: Ping request failed.\n");
        return 1;
    }

    // Display results
    printf("\n--- %s ping statistics ---\n", ping->dest_ip_addr);
    printf("%d packets transmitted, %d received, %.2f%% packet loss, time %.2fms\n",
           ping->result->sent_packets, ping->result->received_packets,
           ((ping->result->sent_packets - ping->result->received_packets) / (double)ping->result->sent_packets) * 100,
           ping->result->rtt);

    return 0;
}