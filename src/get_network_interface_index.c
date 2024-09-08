#include "header.h"

int get_network_interface_index(t_ping *ping)
{
    strncpy(ping->ifreq->ifr_name, ping->network_interface_name, IFNAMSIZ - 1);
    if (ioctl(ping->sock_fd, SIOCGIFINDEX, ping->ifreq) == -1)
    {
        perror("Interface index alınamadı");
        // ping->close(ping);
        return -1;
    }
    return 1;
}