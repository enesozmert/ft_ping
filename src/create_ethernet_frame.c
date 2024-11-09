#include "header.h"

int create_ethernet_frame(t_ping *ping){
    ping->ethernet_frame = (t_ethernet_frame *)ping->packet->packet;

    // Setup Ethernet Frame header
    memcpy(ping->ethernet_frame->src_mac, ping->src_mac, 6);
    // printf("ping->gateway_mac %s\n", ping->gateway_mac);
        printf("Gateway MAC Adresi: %02x:%02x:%02x:%02x:%02x:%02x\n",
           ping->gateway_mac[0], ping->gateway_mac[1], ping->gateway_mac[2], ping->gateway_mac[3], ping->gateway_mac[4], ping->gateway_mac[5]);
    memcpy(ping->ethernet_frame->dest_mac, ping->gateway_mac, 6);
    // memcpy(ping->ethernet_frame->dest_mac, "\xff\xff\xff\xff\xff\xff", 6);
    ping->ethernet_frame->eth_type = htons(ETH_P_IP);
    return 1;
}