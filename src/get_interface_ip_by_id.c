#include "header.h"

int get_interface_ip_by_id(unsigned int interface_id, char *ip_address, size_t size) {
    struct ifaddrs *ifaddr, *ifa;
    char host[NI_MAXHOST];
    char interface_name[IF_NAMESIZE];

    // Arayüz ID'sini arayüz ismine çevir
    if (if_indextoname(interface_id, interface_name) == NULL) {
        perror("if_indextoname");
        return -1; // Arayüz ismi bulunamadı
    }

    // Ağ arayüzlerini al
    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        return -1;
    }

    // Tüm ağ arayüzlerini dolaş
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL)
            continue;

        // Sadece IPv4 adreslerini kontrol et
        if (ifa->ifa_addr->sa_family == AF_INET) {
            // Belirtilen arayüz adı ile eşleşen arayüzü kontrol et
            if (strcmp(ifa->ifa_name, interface_name) == 0) {
                // IP adresini al
                if (getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in), host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST) == 0) {
                    strncpy(ip_address, host, size);
                    freeifaddrs(ifaddr);
                    return 0; // Başarı
                }
            }
        }
    }

    freeifaddrs(ifaddr);
    return -1; // Başarısızlık, arayüz bulunamadı
}
