#include "header.h"

int create_payload(t_ping *ping){
    if (ping->payload.payload_size > 0 && ping->payload.payload != NULL)
    {
        memcpy((unsigned char *)(ping->icmp_header + 1), ping->payload.payload, ping->payload.payload_size);
        ping->icmp_header->checksum = checksum((unsigned short *)ping->icmp_header, sizeof(struct icmphdr) + ping->payload.payload_size);
    }
    return 1;
}