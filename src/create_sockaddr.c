#include "header.h"

int create_sockaddr(t_ping *ping){
    memset(&ping->target_addr, 0, sizeof(ping->target_addr));
    ping->target_addr.sll_ifindex = ping->ifreq->ifr_ifindex;
    ping->target_addr.sll_halen = ETH_ALEN;
    memcpy(ping->target_addr.sll_addr, ping->ethernet_frame->dest_mac, ETH_ALEN);
    return 1;
}