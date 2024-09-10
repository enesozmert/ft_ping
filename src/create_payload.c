#include "header.h"

int create_payload(t_ping *ping){

    const char* data = "enes ozmert 1121092387432784732894783279";
    uint32_t length = strlen(data);
    ping->payload->payload = malloc(length);
    ping->payload->payload_size = strlen(ping->payload->payload);
    if (ping->payload->payload_size > 0 && ping->payload->payload != NULL)
    {
        memcpy((unsigned char *)(ping->icmp_header + 1), ping->payload->payload, ping->payload->payload_size);
        ping->icmp_header->checksum = checksum((unsigned short *)ping->icmp_header, sizeof(struct icmphdr) + ping->payload->payload_size);
    }
    return 1;
}