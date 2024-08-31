#include <sys/socket.h>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h> // IP PROTO ICMP
#include <netinet/ip.h> // IP HEADER
#include <netinet/ip_icmp.h> // ICMP HEADER
#include <arpa/inet.h> // inet_addr
#include <ifaddrs.h> // struct ifaddrs
#include <netdb.h> // NI_MAXHOST, NI_NUMERICHOST
#include <errno.h>
#include <linux/if_ether.h> // ETH_P_ALL
#include <net/if.h> // struct ifreq
#include <sys/ioctl.h>
#include <linux/if_packet.h>  // sockaddr_ll ve ilgili yapılar
#include <sys/time.h> // gettimeofday

struct ipAndIf {
    char *ip;
    char *ifName;
};

struct ethernet_frame {
    unsigned char dest_mac[6];
    unsigned char src_mac[6];
    unsigned short eth_type;
};

int createSocket() {
    int sockFd;
    sockFd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sockFd < 0) {
        perror("Socket Create Error");
        exit(1);
    }
    return sockFd;
}

struct ipAndIf getIpAddr() {
    struct ifaddrs *ifaddr, *ifa;
    char host[NI_MAXHOST];
    struct ipAndIf data = {NULL, NULL};

    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        exit(1);
    }

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL) {
            continue;
        }

        if (ifa->ifa_addr->sa_family == AF_INET) {
            int s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in),
                                host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
            if (s != 0) {
                printf("getnameinfo() failed: %s\n", gai_strerror(s));
                exit(1);
            }
            if (strcmp(ifa->ifa_name, "lo") != 0) {
                data.ip = strdup(host);
                data.ifName = strdup(ifa->ifa_name);
                break;
            }
        }
    }

    freeifaddrs(ifaddr);
    return data;
}

uint16_t calculate_checksum(uint16_t *buf, int len) {
    uint32_t sum = 0;

    while (len > 1) {
        sum += *buf++;
        len -= 2;
    }

    if (len == 1) {
        sum += *(uint8_t *)buf;
    }

    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }

    return (uint16_t)(~sum);
}

unsigned char *craftPacket(char *destIp, int sockFd, int *packet_len, int ttl, int seq_num, const char *payload, int payload_size) {
    struct ethernet_frame *ethFrame;
    struct iphdr *ipHdr;
    struct icmphdr *icmpHdr;
    unsigned char *packet;
    struct ipAndIf data;
    struct ifreq ifr;
    int icmp_packet_size;

    // Kaynak IP ve arayüz ismini al
    data = getIpAddr();
    if (!data.ip || !data.ifName) {
        fprintf(stderr, "Failed to get source IP or interface name.\n");
        exit(1);
    }

    // MAC adresini almak için ioctl kullan
    strncpy(ifr.ifr_name, data.ifName, IFNAMSIZ-1);
    if (ioctl(sockFd, SIOCGIFHWADDR, &ifr) == -1) {
        perror("Interface MAC Address alınamadı");
        close(sockFd);
        exit(1);
    }

    // Paket boyutunu hesapla
    icmp_packet_size = sizeof(struct icmphdr) + payload_size;
    *packet_len = sizeof(struct ethernet_frame) + sizeof(struct iphdr) + icmp_packet_size;
    packet = malloc(*packet_len);
    if (!packet) {
        printf("Memory Allocation Error!\n");
        exit(1);
    }

    // Paketi sıfırla
    memset(packet, 0, *packet_len);
    ethFrame = (struct ethernet_frame *)packet;
    ipHdr = (struct iphdr *)(packet + sizeof(struct ethernet_frame));
    icmpHdr = (struct icmphdr *)(packet + sizeof(struct iphdr) + sizeof(struct ethernet_frame));

    // Ethernet çerçevesini ayarla
    memset(ethFrame->dest_mac, 0xff, 6); // Broadcast MAC
    memcpy(ethFrame->src_mac, &ifr.ifr_hwaddr.sa_data, 6);
    ethFrame->eth_type = htons(ETH_P_IP);

    // IP başlığını ayarla
    ipHdr->saddr = inet_addr(data.ip);
    ipHdr->daddr = inet_addr(destIp);
    ipHdr->ihl = 5;
    ipHdr->version = 4;
    ipHdr->tos = 0;
    ipHdr->tot_len = htons(sizeof(struct iphdr) + icmp_packet_size);
    ipHdr->id = htons(getpid() & 0xFFFF); // Process ID'yi kullanarak benzersizleştir
    ipHdr->frag_off = htons(0);
    ipHdr->ttl = ttl;
    ipHdr->protocol = IPPROTO_ICMP;
    ipHdr->check = calculate_checksum((unsigned short *)ipHdr, sizeof(struct iphdr));

    // ICMP başlığını ayarla
    icmpHdr->type = ICMP_ECHO;
    icmpHdr->code = 0;
    icmpHdr->un.echo.id = htons(getpid() + rand()); // Benzersiz ID için PID ve rastgele sayı kombinasyonu
    icmpHdr->un.echo.sequence = htons(seq_num); // Sıra numarası dinamik olarak ayarlanabilir
    icmpHdr->checksum = 0;

    // Veri yükü ekle (payload)
    if (payload_size > 0 && payload != NULL) {
        memcpy((unsigned char *)(icmpHdr + 1), payload, payload_size);
    }

    // ICMP başlığının checksum'unu hesapla
    icmpHdr->checksum = calculate_checksum((unsigned short *)icmpHdr, icmp_packet_size);

    // Belleği temizle
    free(data.ip);
    free(data.ifName);

    return packet;
}

int main(int ac, char **av)
{
    int sockFd, seq_num = 1;
    unsigned char *packet;
    struct sockaddr_ll target_addr;
    int packet_len;
    unsigned char recvBuff[1024];
    struct icmphdr *icmp_hdr;
    struct iphdr *ip_hdr;
    struct timeval timeout;
    fd_set read_fds;
    struct timeval start, end;
    double rtt;
    struct ifreq ifr;

    if (ac != 2)
    {
        printf("Usage: %s <destination IP>\n", av[0]);
        exit(1);
    }

    sockFd = createSocket();

    struct ipAndIf data = getIpAddr();
    strncpy(ifr.ifr_name, data.ifName, IFNAMSIZ-1);
    if (ioctl(sockFd, SIOCGIFINDEX, &ifr) == -1) {
        perror("Interface index alınamadı");
        close(sockFd);
        exit(1);
    }
    free(data.ifName);
    free(data.ip);

    const char *payload = "Hello, this is a custom payload!";
    int payload_size = strlen(payload);
    int ttl = 64;

    while (seq_num <= 4) { // 4 deneme için döngü
        packet = craftPacket(av[1], sockFd, &packet_len, ttl, seq_num, payload, payload_size);

        memset(&target_addr, 0, sizeof(target_addr));
        target_addr.sll_ifindex = ifr.ifr_ifindex;
        target_addr.sll_halen = ETH_ALEN;
        memset(target_addr.sll_addr, 0xff, 6); // Broadcast MAC

        if (sendto(sockFd, packet, packet_len, 0, (struct sockaddr *)&target_addr, sizeof(target_addr)) <= 0) {
            perror("sendto");
            exit(1);
        }

        gettimeofday(&start, NULL);

        FD_ZERO(&read_fds);
        FD_SET(sockFd, &read_fds);
        timeout.tv_sec = 1;  // 1 second timeout
        timeout.tv_usec = 0;

        int ret = select(sockFd + 1, &read_fds, NULL, NULL, &timeout);
        if (ret > 0) {
            socklen_t addr_len = sizeof(target_addr);
            if (recvfrom(sockFd, recvBuff, sizeof(recvBuff), 0, NULL, NULL) <= 0) {
                perror("Recvfrom error");
                exit(1);
            }

            gettimeofday(&end, NULL);

            ip_hdr = (struct iphdr *)(recvBuff + sizeof(struct ethernet_frame));
            icmp_hdr = (struct icmphdr *)(recvBuff + sizeof(struct ethernet_frame) + (ip_hdr->ihl * 4));

            rtt = (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_usec - start.tv_usec) / 1000.0;

            printf("64 bytes from %s: icmp_seq=%d ttl=%d time=%.2f ms\n",
                   av[1],
                   ntohs(icmp_hdr->un.echo.sequence),
                   ip_hdr->ttl,
                   rtt);
        } else if (ret == 0) {
            printf("Request timed out for icmp_seq=%d\n", seq_num);
        } else {
            perror("select error");
            exit(1);
        }

        free(packet);
        seq_num++;
        sleep(1); // 1 saniye bekle
    }

    close(sockFd);

    return 0;
}
