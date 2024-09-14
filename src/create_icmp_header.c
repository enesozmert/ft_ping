#include "header.h"

int create_icmp_header(t_ping *ping){
    ping->icmp_header = (struct icmphdr *)(ping->packet.packet + sizeof(t_ethernet_frame) + sizeof(struct iphdr));

    // Setup ICMP header
    ping->icmp_header->type = ICMP_ECHO;
    ping->icmp_header->code = 0;
    ping->icmp_header->checksum = 0;
    ping->icmp_header->un.echo.id = htons(getpid() + rand());
    ping->icmp_header->un.echo.sequence = htons(0);

        // ICMP header'ın belleğe tahsis edilip edilmediğini kontrol et
    if (ping->icmp_header == NULL) {
        free(ping->payload->payload); // Bellek sızıntısını önlemek için serbest bırak
        return -1; // Bellek tahsisi başarısızsa hata döndür
    }


    // Veriyi ICMP header'ın sonrasına kopyalama
    memcpy((unsigned char *)(ping->icmp_header + 1), ping->payload->payload, ping->payload->payload_size);
        // Checksum hesaplama
    ping->icmp_header->checksum = checksum((unsigned short *)ping->icmp_header, sizeof(struct icmphdr) + ping->payload->payload_size);
    return 1;
}