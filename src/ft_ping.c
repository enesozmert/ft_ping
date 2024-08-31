#include "header.h"

uint32_t get_source_ip_address()
{
    struct ifaddrs *ifaddr, *ifa;
    int family, s;
    char host[NI_MAXHOST];
    uint32_t ip_address = 0;

    if (getifaddrs(&ifaddr) == -1)
    {
        perror("getifaddrs");
        exit(EXIT_FAILURE);
    }

    // Iterate through all network interfaces
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr == NULL)
            continue;

        family = ifa->ifa_addr->sa_family;

        // Find an IPv4 address
        if (family == AF_INET)
        {
            s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in),
                            host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
            if (s != 0)
            {
                printf("getnameinfo() failed: %s\n", gai_strerror(s));
                exit(EXIT_FAILURE);
            }

            // Get the first non-loopback IPv4 address
            if (strcmp(ifa->ifa_name, "lo") != 0)
            {
                ip_address = ((struct sockaddr_in *)ifa->ifa_addr)->sin_addr.s_addr;
                break;
            }
        }
    }

    freeifaddrs(ifaddr);

    if (ip_address == 0)
    {
        fprintf(stderr, "Could not determine local IP address\n");
        exit(EXIT_FAILURE);
    }

    return ip_address;
}

unsigned char *find_src_mac_addr(int sockfd)
{
    struct ifreq ifr;
    struct ifconf ifc;
    char buffer[1024];

    ifc.ifc_len = sizeof(buffer);
    ifc.ifc_buf = buffer;
    if (ioctl(sockfd, SIOCGIFCONF, &ifc) == -1)
    {
        perror("ioctl");
        return NULL;
    }

    struct ifreq *it = ifc.ifc_req;
    const struct ifreq *const end = it + (ifc.ifc_len / sizeof(struct ifreq));

    for (; it != end; ++it)
    {
        strcpy(ifr.ifr_name, it->ifr_name);
        if (ioctl(sockfd, SIOCGIFFLAGS, &ifr) == 0)
        {
            if (!(ifr.ifr_flags & IFF_LOOPBACK))
            {
                if (ioctl(sockfd, SIOCGIFHWADDR, &ifr) == 0)
                {
                    unsigned char *mac = (unsigned char *)ifr.ifr_hwaddr.sa_data;
                    return mac;
                }
            }
        }
        else
        {
            perror("ioctl");
        }
    }

    return NULL;
}

int get_network_interface_index(int sockfd)
{
    struct ifreq ifr;
    struct ifconf ifc;
    char buf[1024];
    struct ifreq *it;
    struct ifreq *end;

    ifc.ifc_len = sizeof(buf);
    ifc.ifc_buf = buf;
    if (ioctl(sockfd, SIOCGIFCONF, &ifc) == -1)
    {
        perror("ioctl SIOCGIFCONF");
        return -1;
    }

    it = ifc.ifc_req;
    end = it + (ifc.ifc_len / sizeof(struct ifreq));

    for (; it != end; ++it)
    {
        strcpy(ifr.ifr_name, it->ifr_name);
        if (ioctl(sockfd, SIOCGIFFLAGS, &ifr) == 0)
        {
            if (!(ifr.ifr_flags & IFF_LOOPBACK))
            {
                return if_nametoindex(it->ifr_name);
            }
        }
        else
        {
            perror("ioctl SIOCGIFFLAGS");
        }
    }

    fprintf(stderr, "No suitable network interface found.\n");
    return -1;
}

unsigned short calculate_checksum(void *buffer, int length)
{
    unsigned short *data = buffer;
    unsigned long sum = 0;
    for (sum = 0; length > 1; length -= 2)
    {
        sum += *data++;
    }
    if (length == 1)
    {
        sum += *(unsigned char *)data;
    }
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    return (unsigned short)(~sum);
}

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
    const char *payload = "Hello, this is a custom payload!";
    int payload_size = strlen(payload);

    // Create a raw socket
    sockFd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sockFd < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    // Find source MAC address
    unsigned char *src_mac = find_src_mac_addr(sockFd);
    if (src_mac == NULL)
    {
        fprintf(stderr, "Source MAC address not found.\n");
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
    memcpy(ethHdr->dest_mac, "\xff\xff\xff\xff\xff\xff", 6); // Broadcast
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
    ipHeader->check = calculate_checksum((unsigned short *)ipHeader, sizeof(struct iphdr));

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
    icmpHeader->checksum = calculate_checksum((unsigned short *)icmpHeader, sizeof(struct icmphdr) + payload_size);

    // Setup destination address
    memset(&target_addr, 0, sizeof(target_addr));
    target_addr.sll_ifindex = get_network_interface_index(sockFd);
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
