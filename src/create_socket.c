#include "header.h"

int create_socket(const char *ip_addr, t_ping_result *result)
{
    int sockFd;
    unsigned char *packet;
    int icmp_packet_size;
    int packet_len;
    // char packet[1000] = {0};
    unsigned char recvBuff[1024];
    t_ethernet_frame *ethHdr;
    struct iphdr *ipHeader;
    struct icmphdr *icmpHeader;
    struct sockaddr_ll target_addr;
    struct timeval start_time, end_time;
    double elapsed_time;
    fd_set read_fds;
    struct timeval timeout;
    const char *payload = "abcdefghijklmnopqrstuvwabcdefghiabcdefghij123456789*-.!+";
    int payload_size = strlen(payload);
    uint32_t network_interface_index;

    network_interface_index = 0;

    // Create a raw socket
    sockFd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sockFd < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    network_interface_index = get_network_interface_index(sockFd);

    // Find source MAC address
    unsigned char *src_mac = find_src_mac_addr(sockFd);
    if (src_mac == NULL)
    {
        fprintf(stderr, "Source MAC address not found.\n");
        close(sockFd);
        return 1;
    }

    printf("network_interface_index %d \n", network_interface_index);
    unsigned char *dest_mac = resolve_mac_address(ip_addr, sockFd, network_interface_index);
    if (dest_mac == NULL)
    {
        fprintf(stderr, "Failed to resolve destination MAC address.\n");
        close(sockFd);
        return 1;
    }

    icmp_packet_size = sizeof(struct icmphdr) + payload_size;
    packet_len = sizeof(t_ethernet_frame) + sizeof(struct iphdr) + icmp_packet_size;
    packet = malloc(packet_len);
    if (!packet) {
        printf("Memory Allocation Error!\n");
        exit(1);
    }

    memset(packet, 0, packet_len);
    ethHdr = (t_ethernet_frame *)packet;
    ipHeader = (struct iphdr *)(packet + sizeof(t_ethernet_frame));
    icmpHeader = (struct icmphdr *)(packet + sizeof(t_ethernet_frame) + sizeof(struct iphdr));

    // Setup Ethernet header
    memcpy(ethHdr->src_mac, src_mac, 6);
    memcpy(ethHdr->dest_mac, dest_mac, 6); 
    // memcpy(ethHdr->dest_mac, "\xff\xff\xff\xff\xff\xff", 6);

    ethHdr->eth_type = htons(ETH_P_IP);

    // Setup IP header
    ipHeader->version = 4; // IPv4
    ipHeader->ihl = 5;     // IP header length (5 * 32 bits = 20 bytes)
    ipHeader->tos = 0;
    ipHeader->tot_len = htons(sizeof(struct iphdr) + sizeof(struct icmphdr) + payload_size);
    ipHeader->id = htons(get_source_ip_address() & 0xFFFF);
    ipHeader->frag_off = htons(0);
    ipHeader->ttl = 64;
    ipHeader->protocol = IPPROTO_ICMP;
    ipHeader->check = 0;
    ipHeader->saddr = get_source_ip_address();
    ipHeader->daddr = inet_addr(ip_addr);

    // Calculate IP header checksum
    ipHeader->check = checksum((unsigned short *)ipHeader, sizeof(struct iphdr));

    // Setup ICMP header
    icmpHeader->type = ICMP_ECHO;
    icmpHeader->code = 0;
    icmpHeader->checksum = 0;
    icmpHeader->un.echo.id = htons(get_source_ip_address() & 0xFFFF);
    icmpHeader->un.echo.sequence = htons(0);

    if (payload_size > 0 && payload != NULL)
    {
        memcpy((unsigned char *)(icmpHeader + 1), payload, payload_size);
    }

    // Calculate ICMP checksum
    icmpHeader->checksum = checksum((unsigned short *)icmpHeader, sizeof(struct icmphdr) + payload_size);

    // Setup destination address
    memset(&target_addr, 0, sizeof(target_addr));
    target_addr.sll_ifindex = network_interface_index;
    target_addr.sll_halen = ETH_ALEN;
    memcpy(target_addr.sll_addr, ethHdr->dest_mac, ETH_ALEN);

    // Send ping request
    result->sent_packets++;
    gettimeofday(&start_time, NULL);
    printf("Sending ping request...\n");
    if (sendto(sockFd, packet, sizeof(t_ethernet_frame) + ntohs(ipHeader->tot_len), 0, (struct sockaddr *)&target_addr, sizeof(target_addr)) < 0)
    {
        perror("Send failed");
        close(sockFd);
        return 1;
    }
    printf("Ping request sent.\n");

    FD_ZERO(&read_fds);
    FD_SET(sockFd, &read_fds);

    timeout.tv_sec = 1; // 1 second timeout
    timeout.tv_usec = 0;

    int ret = select(sockFd + 1, &read_fds, NULL, NULL, &timeout);
    if (ret > 0)
    {
        // Wait for a reply
        printf("Waiting for ping reply...\n");
        ssize_t len = recvfrom(sockFd, &recvBuff, sizeof(recvBuff), 0, NULL, NULL);
        if (len <= 0)
        {
            perror("Recvfrom error");
        }
        else
        {
            // Parse IP and ICMP headers
            struct iphdr *recv_ip_hdr = (struct iphdr *)(recvBuff + sizeof(t_ethernet_frame));
            struct icmphdr *recv_icmp_hdr = (struct icmphdr *)(recvBuff + sizeof(t_ethernet_frame) + (recv_ip_hdr->ihl * 4));

            printf("Received packet type: %d\n", recv_icmp_hdr->type);
            printf("Expected ICMP type: %d\n", ICMP_ECHOREPLY);
            printf("Received packet ID: %d\n", ntohs(recv_icmp_hdr->un.echo.id));
            printf("Expected packet ID: %d\n", ntohs(icmpHeader->un.echo.id));

            // Check ICMP type and ID
            // if (recv_icmp_hdr->type == ICMP_ECHOREPLY && recv_icmp_hdr->un.echo.id == icmpHeader->un.echo.id)
            // {
                result->received_packets++;
                gettimeofday(&end_time, NULL);
                elapsed_time = (end_time.tv_sec - start_time.tv_sec) * 1000.0;
                elapsed_time += (end_time.tv_usec - start_time.tv_usec) / 1000.0;
                result->rtt += elapsed_time;
                printf("%zd bytes from %s: icmp_seq=%d ttl=%d time=%.2f ms\n",
                       len, ip_addr, ntohs(recv_icmp_hdr->un.echo.sequence), recv_ip_hdr->ttl, elapsed_time);
            // }
            // else
            // {
            //     printf("Received packet is not an ICMP Echo Reply or the ID does not match.\n");
            // }
        }
    }
    else
    {
        printf("Ping request timed out.\n");
    }

    close(sockFd);
    return 0;
}
