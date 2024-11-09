#include "header.h"

int get_network_gateway_mac_address(t_ping *ping)
{
    int sock;
    struct sockaddr_in addr;
    struct arpreq arpreq;

    // ARP isteğini düzgün şekilde yapılandırmak için arpreq'i sıfırlayın
    memset(&arpreq, 0, sizeof(struct arpreq));

    // Socket oluştur
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0)
    {
        perror("Socket oluşturulamadı");
        return -1;
    }

    printf("get_network_gateway_mac_address ping->gateway_ip %s\n ", ping->gateway_ip);

    // Gateway IP adresini yapılandır
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ping->gateway_ip);
    memcpy(&arpreq.arp_pa, &addr, sizeof(addr));

    strncpy(arpreq.arp_dev, ping->ifreq->ifr_name, IFNAMSIZ - 1);
    // ARP isteği yap
    if (ioctl(sock, SIOCGARP, &arpreq) == -1)
    {
        perror("ARP isteği başarısız");
        close(sock);
        return -2;
    }

    close(sock);

    // MAC adresini al
    ping->gateway_mac = (unsigned char *)malloc(6 * sizeof(unsigned char));
    memcpy(ping->gateway_mac, arpreq.arp_ha.sa_data, 6);

    printf("Gateway MAC Adresi: %02x:%02x:%02x:%02x:%02x:%02x\n",
           ping->gateway_mac[0], ping->gateway_mac[1], ping->gateway_mac[2], ping->gateway_mac[3], ping->gateway_mac[4], ping->gateway_mac[5]);

    return 1;
}