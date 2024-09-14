#include "header.h"

int get_network_src_mac_addr(t_ping *ping)
{
    struct ifreq ifr;
    struct ifconf ifc;
    char buffer[1024];

    ifc.ifc_len = sizeof(buffer);
    ifc.ifc_buf = buffer;
    if (ioctl(ping->sock_fd, SIOCGIFCONF, &ifc) == -1)
    {
        perror("ioctl");
        return -1;
    }

    struct ifreq *it = ifc.ifc_req;
    const struct ifreq *const end = it + (ifc.ifc_len / sizeof(struct ifreq));

    for (; it != end; ++it)
    {
        strcpy(ifr.ifr_name, it->ifr_name);
        if (ioctl(ping->sock_fd, SIOCGIFFLAGS, &ifr) == 0)
        {
            if (!(ifr.ifr_flags & IFF_LOOPBACK))
            {
                if (ioctl(ping->sock_fd, SIOCGIFHWADDR, &ifr) == 0)
                {
                    ping->src_mac = malloc(strlen(ifr.ifr_hwaddr.sa_data));
                    memcpy(ping->src_mac, (unsigned char *)ifr.ifr_hwaddr.sa_data, 6);
                    return 1;
                }
            }
        }
        else
        {
            perror("ioctl");
        }
    }

    return -1;
}