#include "header.h"

int create_icmp_header(t_ping *ping){
    ping->icmp_header = (struct icmphdr *)(ping->packet.packet + sizeof(t_ethernet_frame) + sizeof(struct iphdr));

    // Setup ICMP header
    ping->icmp_header->type = ICMP_ECHO;
    ping->icmp_header->code = 0;
    ping->icmp_header->checksum = 0;
    ping->icmp_header->un.echo.id = htons(getpid() + rand());
    ping->icmp_header->un.echo.sequence = htons(0);
    return 1;
}