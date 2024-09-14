#include "header.h"

int create_ethernet_frame(t_ping *ping){
    ping->ethernet_frame = (t_ethernet_frame *)ping->packet->packet;

    // Setup Ethernet Frame header
    memcpy(ping->ethernet_frame->src_mac, ping->src_mac, 6);
    printf("ping->gateway_mac %s\n", ping->gateway_mac);
    memcpy(ping->ethernet_frame->dest_mac, ping->gateway_mac, 6);
    // memcpy(ping->ethernet_frame->dest_mac, "\xff\xff\xff\xff\xff\xff", 6);
    ping->ethernet_frame->eth_type = htons(ETH_P_IP);
    return 1;
}