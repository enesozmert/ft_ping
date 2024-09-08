#include "header.h"

int create_socket(t_ping *ping)
{
    create_raw_socket(ping);
    ping->src_mac = find_src_mac_addr(ping->sock_fd);
    if (ping->src_mac == NULL)
    {
        fprintf(stderr, "Source MAC address not found.\n");
        close(ping->sock_fd);
        return 1;
    }

    int result1 = get_network_default_gateway(ping);
    if (result1 == -1) {
        perror("Failed to get default gateway");
        free(ping->ifreq);
        close(ping->sock_fd);
        free(ping);
        return 1;
    }

    int result2 = get_network_gateway_mac_address(ping);
    if (result2 == -1) {
        perror("Failed to get_network_gateway_mac_address");
        free(ping->ifreq);
        close(ping->sock_fd);
        free(ping);
        return 1;
    }

    int result3 = get_network_interface_name(ping);
    if (result3 == -1) {
        perror("Failed to get_network_interface_name");
        free(ping);
        return 1;
    }

    int result4 = get_network_interface_index(ping);
    if (result4 == -1) {
        perror("Failed to get_network_interface_name");
        free(ping);
        return 1;
    }

    ping_create_func_t *functions = ping_create_functions();
    run_ping_create_functions(ping, functions, 8);

    // create_packet(ping);
    // create_ethernet_frame(ping);
    // create_payload(ping);
    // create_ip_header(ping);
    // create_icmp_header(ping);
    // create_sockaddr(ping);

    // Send ping request
    // ping->result->sent_packets++;
    // gettimeofday(&ping->time.start_time, NULL);
    // printf("Sending ping request...\n");
    // if (sendto(ping->sock_fd, ping->packet.packet, sizeof(t_ethernet_frame) + ntohs(ping->ip_header->tot_len), 0, (struct sockaddr *)&ping->target_addr, sizeof(ping->target_addr)) < 0)
    // {
    //     perror("Send failed");
    //     close(ping->sock_fd);
    //     return 1;
    // }
    // printf("Ping request sent.\n");

    // FD_ZERO(&ping->read_fds);
    // FD_SET(ping->sock_fd, &ping->read_fds);

    ping->time.timeout.tv_sec = 1;
    ping->time.timeout.tv_usec = 0;

    int ret = select(ping->sock_fd + 1, &ping->read_fds, NULL, NULL, &ping->time.timeout);
    if (ret > 0)
    {
        printf("Waiting for ping reply...\n");
        unsigned char recvBuff[1024];
        ssize_t len = recvfrom(ping->sock_fd, &recvBuff, sizeof(recvBuff), 0, NULL, NULL);
        if (len <= 0)
        {
            perror("Recvfrom error");
        }
        else
        {
            struct iphdr *recv_ip_hdr = (struct iphdr *)(recvBuff + sizeof(t_ethernet_frame));
            struct icmphdr *recv_icmp_hdr = (struct icmphdr *)(recvBuff + sizeof(t_ethernet_frame) + (recv_ip_hdr->ihl * 4));

            // if (recv_icmp_hdr->type == ICMP_ECHOREPLY && recv_icmp_hdr->un.echo.id == ping->icmp_header->un.echo.id)
            // {
                ping->result->received_packets++;
                gettimeofday(&ping->time.end_time, NULL);
                ping->time.elapsed_time = (ping->time.end_time.tv_sec - ping->time.start_time.tv_sec) * 1000.0;
                ping->time.elapsed_time += (ping->time.end_time.tv_usec - ping->time.start_time.tv_usec) / 1000.0;
                ping->result->rtt += ping->time.elapsed_time;
                printf("%zd bytes from %s: icmp_seq=%d ttl=%d time=%.2f ms\n", len, ping->ip_addr, ntohs(recv_icmp_hdr->un.echo.sequence), recv_ip_hdr->ttl, ping->time.elapsed_time);
            // }
        }
    }
    else
    {
        printf("Ping request timed out.\n");
    }

    close(ping->sock_fd);
    return 0;
}
