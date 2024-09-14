#include "header.h"

int create_packet(t_ping *ping){
    // Allocate memory for packet
    ping->packet->icmp_packet_size = sizeof(struct icmphdr) + ping->payload->payload_size;
    ping->packet->packet_len = sizeof(t_ethernet_frame) + sizeof(struct iphdr) + ping->packet->icmp_packet_size;
    ping->packet->packet = malloc(ping->packet->packet_len);
    if (!ping->packet->packet)
    {
        printf("Memory Allocation Error!\n");
        exit(1);
    }
    memset(ping->packet->packet, 0, ping->packet->packet_len);
    return 1;
}