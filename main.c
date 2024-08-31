#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/ip_icmp.h>
#include <netinet/in.h>
#include <errno.h>
#include <stdint.h>
#include <sys/time.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <net/ethernet.h>
#include <linux/if_packet.h>
#include <net/if.h>
#include <sys/ioctl.h>

#define MAX_IP_OPTIONS_LENGTH 40

// Ethernet Frame Struct
struct ethernet_frame
{
    unsigned char dest_mac[6];
    unsigned char src_mac[6];
    unsigned short eth_type;
};

// IP Header Struct
struct ip_header
{
    uint8_t ihl : 4;                        // Header length
    uint8_t version : 4;                    // Version
    uint8_t tos;                            // Type of service
    uint16_t total_length;                  // Total length
    uint16_t identification;                // Identification
    uint16_t fragment_offset;               // Fragment offset
    uint8_t ttl;                            // Time to live
    uint8_t protocol;                       // Protocol
    uint16_t checksum;                      // Header checksum
    uint32_t source_address;                // Source address
    uint32_t dest_address;                  // Destination address
    uint8_t options[MAX_IP_OPTIONS_LENGTH]; // Opsiyonlar için alan
};

// ICMP Header Struct
struct icmp_header
{
    uint8_t type;      // ICMP mesaj tipi
    uint8_t code;      // ICMP mesaj kodu
    uint16_t checksum; // Hata kontrolü (checksum)
    union
    {
        struct
        {
            uint16_t id;       // Tanımlayıcı
            uint16_t sequence; // Sıra numarası
        } echo;                // Echo mesajları için
        uint32_t gateway;      // Redirect mesajları için
        struct
        {
            uint16_t unused; // Kullanılmayan alan
            uint16_t mtu;    // MTU değeri
        } frag;              // Fragmentation için
    } un;
};

struct icmp_reply
{
    struct ip_header ip_hdr;     // IP header
    struct icmp_header icmp_hdr; // ICMP header
    unsigned char payload[64];   // ICMP payload (64 byte olarak sabit)
};

// Ping Sonucu için Struct
struct ping_result
{
    int sent_packets;
    int received_packets;
    double rtt; // Round-trip time
};

// Prototipler
void parse_args(int argc, char *argv[], int *verbose_flag);
int create_socket(const char *ip_addr, struct ping_result *result, int verbose_flag);
unsigned short calculate_checksum(void *buffer, int length);
uint32_t get_source_ip_address();
unsigned char *find_src_mac_addr(int sockfd);
void print_usage(const char *program_name);

int main(int argc, char *argv[])
{
    int verbose_flag = 0;
    struct ping_result result = {0, 0, 0.0};

    // Komut satırı argümanlarını işle
    parse_args(argc, argv, &verbose_flag);

    // Hedef IP adresini al
    if (optind >= argc)
    {
        fprintf(stderr, "Usage: %s [-v] [-?] <hostname>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    const char *ip_addr = argv[optind];

    // Ping işlemi başlat
    if (create_socket(ip_addr, &result, verbose_flag) != 0)
    {
        fprintf(stderr, "Ping request failed.\n");
        return 1;
    }

    // Sonuçları göster
    printf("\n--- %s ping statistics ---\n", ip_addr);
    printf("%d packets transmitted, %d received, %.2f%% packet loss, time %.2fms\n",
           result.sent_packets, result.received_packets,
           ((result.sent_packets - result.received_packets) / (double)result.sent_packets) * 100,
           result.rtt);

    return 0;
}

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

    // Tüm ağ arayüzlerini dolaş
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr == NULL)
            continue;

        family = ifa->ifa_addr->sa_family;

        // IPv4 ailesine ait bir adres bul
        if (family == AF_INET)
        {
            s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in),
                            host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
            if (s != 0)
            {
                printf("getnameinfo() failed: %s\n", gai_strerror(s));
                exit(EXIT_FAILURE);
            }

            // loopback olmayan (127.0.0.1 değil) ilk IPv4 adresini al
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
            { // Loopback olmayan bir arayüz bulduk
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

void parse_args(int argc, char *argv[], int *verbose_flag)
{
    int opt;
    while ((opt = getopt(argc, argv, "v?")) != -1)
    {
        switch (opt)
        {
        case 'v':
            *verbose_flag = 1;
            break;
        case '?':
            print_usage(argv[0]);
            exit(EXIT_SUCCESS);
        default:
            fprintf(stderr, "Usage: %s [-v] [-?] <hostname>\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }
}

void print_usage(const char *program_name)
{
    printf("Usage: %s [-v] [-?] <hostname>\n", program_name);
    printf("Options:\n");
    printf("  -v   Enable verbose output.\n");
    printf("  -?   Display this help message and exit.\n");
}

int create_socket(const char *ip_addr, struct ping_result *result, int verbose_flag)
{
    int sockFd;
    char packet[1000] = {0};
    struct ethernet_frame *ethHdr;
    struct ip_header *ipHdr;
    struct icmp_header *icmpHeader;
    struct sockaddr_ll target_addr;
    struct timeval start_time, end_time;
    struct icmp_reply reply;
    double elapsed_time;

    // Raw socket oluştur
    sockFd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sockFd < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    // MAC adresini bul
    unsigned char *src_mac = find_src_mac_addr(sockFd);
    if (src_mac == NULL)
    {
        fprintf(stderr, "Source MAC address not found.\n");
        close(sockFd);
        return 1;
    }

    memset(packet, 0, sizeof(packet));
    ethHdr = (struct ethernet_frame *)packet;
    ipHdr = (struct ip_header *)(packet + sizeof(struct ethernet_frame));
    icmpHeader = (struct icmp_header *)(packet + sizeof(struct ethernet_frame) + sizeof(struct ip_header));

    // Ethernet header setup
    memcpy(ethHdr->src_mac, src_mac, 6);
    memcpy(ethHdr->dest_mac, "\xff\xff\xff\xff\xff\xff", 6); // Broadcast
    ethHdr->eth_type = htons(ETH_P_IP);

    // IP header setup
    ipHdr->version = 4; // IPv4
    ipHdr->ihl = 5;     // IP header length (5 * 32 bits = 20 bytes)
    ipHdr->tos = 0;
    ipHdr->total_length = htons(sizeof(struct ip_header) + sizeof(struct icmp_header));
    ipHdr->identification = htons(54321);
    ipHdr->fragment_offset = htons(IP_DF);
    ipHdr->ttl = 64;
    ipHdr->protocol = IPPROTO_ICMP;
    ipHdr->checksum = 0;
    ipHdr->source_address = get_source_ip_address();
    ipHdr->dest_address = inet_addr(ip_addr);

    // IP header checksum hesapla
    ipHdr->checksum = calculate_checksum((unsigned short *)ipHdr, sizeof(struct ip_header));

    // ICMP header setup
    icmpHeader->type = ICMP_ECHO;
    icmpHeader->code = 0;
    icmpHeader->checksum = 0;
    icmpHeader->un.echo.id = htons(1234);    // Echo mesajı için id
    icmpHeader->un.echo.sequence = htons(1); // Echo mesajı için sıra numarası

    // ICMP checksum hesapla
    icmpHeader->checksum = calculate_checksum((unsigned short *)icmpHeader, sizeof(struct icmp_header));

    // Set up destination address
    memset(&target_addr, 0, sizeof(target_addr));
    target_addr.sll_ifindex = get_network_interface_index(sockFd);
    target_addr.sll_halen = ETH_ALEN;
    memcpy(target_addr.sll_addr, ethHdr->dest_mac, ETH_ALEN);

    // Ping isteği gönder
    result->sent_packets++;
    gettimeofday(&start_time, NULL);
    printf("Ping isteği gönderiliyor.\n");
    if (sendto(sockFd, packet, sizeof(struct ethernet_frame) + ntohs(ipHdr->total_length), 0, (struct sockaddr *)&target_addr, sizeof(target_addr)) < 0)
    {
        perror("Send failed");
        close(sockFd);
        return 1;
    }
    printf("Ping isteği gönderildi.\n");

    // Cevap al
    printf("Ping cevabı bekleniyor.\n");
    ssize_t len = recvfrom(sockFd, &reply, sizeof(reply), 0, NULL, NULL);
    if (len <= 0)
    {
        perror("Recvfrom error");
    }
    else
    {
        // IP ve ICMP header'ı ayrıştır
        struct ip_header *recv_ip_hdr = &reply.ip_hdr;
        struct icmp_header *recv_icmp_hdr = &reply.icmp_hdr;

        if (verbose_flag)
        {
            printf("Received packet type: %d\n", recv_icmp_hdr->type);
            printf("Expected ICMP type: %d\n", ICMP_ECHOREPLY);
            printf("Received packet ID: %d\n", ntohs(recv_icmp_hdr->un.echo.id));
            printf("Expected packet ID: %d\n", ntohs(icmpHeader->un.echo.id));
            printf("Received packet Code: %d\n", ntohs(recv_icmp_hdr->code));
        }

        // ICMP türünü kontrol et
        if (recv_icmp_hdr->type == ICMP_ECHOREPLY && recv_icmp_hdr->un.echo.id == icmpHeader->un.echo.id)
        {
            result->received_packets++;
            gettimeofday(&end_time, NULL);
            elapsed_time = (end_time.tv_sec - start_time.tv_sec) * 1000.0;
            elapsed_time += (end_time.tv_usec - start_time.tv_usec) / 1000.0;
            result->rtt += elapsed_time;
            printf("%zd bytes from %s: icmp_seq=%d ttl=%d time=%.2f ms\n",
                   len, ip_addr, ntohs(recv_icmp_hdr->un.echo.sequence), recv_ip_hdr->ttl, elapsed_time);
        }
        else
        {
            printf("Received packet is not an ICMP Echo Reply.\n");
        }
    }

    close(sockFd);
    return 0;
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
