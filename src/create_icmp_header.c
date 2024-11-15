#include "header.h"

int create_icmp_header(t_ping *ping){
    ping->icmp_header = (struct icmphdr *)(ping->packet->packet + sizeof(struct iphdr) + sizeof(t_ethernet_frame));

    if (ping->icmp_header == NULL) {
        free(ping->payload->payload);
        return -1;
    }

    ping->icmp_header->type = ICMP_ECHO;
    ping->icmp_header->code = 0;
    ping->icmp_header->checksum = 0;
    ping->icmp_header->un.echo.id = htons(getpid() + rand());
    ping->icmp_header->un.echo.sequence = htons(0);

    ping->icmp_header->checksum = checksum((unsigned short *)ping->icmp_header, sizeof(struct icmphdr) + ping->payload->payload_size);
    return 1;
}