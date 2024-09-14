#include "header.h"

int create_ip_header(t_ping *ping){
    ping->ip_header = (struct iphdr *)(ping->packet->packet + sizeof(t_ethernet_frame));

    // Setup IP header
    ping->ip_header->version = 4;
    ping->ip_header->ihl = 5;
    ping->ip_header->tos = 0;
    ping->ip_header->tot_len = htons(sizeof(struct iphdr) + sizeof(struct icmphdr) + ping->payload->payload_size);
    ping->ip_header->id = htons(getpid() & 0xFFFF);
    ping->ip_header->frag_off = htons(0);
    ping->ip_header->ttl = 64;
    ping->ip_header->protocol = IPPROTO_ICMP;
    ping->ip_header->check = 0;
    ping->ip_header->saddr = ping->src_ip_addr;
    ping->ip_header->daddr = inet_addr(ping->dest_ip_addr);
    ping->ip_header->check = checksum((unsigned short *)ping->ip_header, sizeof(struct iphdr));
    return 1;
}