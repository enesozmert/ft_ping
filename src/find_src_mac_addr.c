#include "header.h"

unsigned char *find_src_mac_addr(int sockfd)
{
    struct ifreq ifr;
    struct ifconf ifc;
    char buffer[1024];

    ifc.ifc_len = sizeof(buffer);
    ifc.ifc_buf = buffer;
    if (ioctl(sockfd, SIOCGIFCONF, &ifc) == -1)
    {
        perror("ioctl");
        return NULL;
    }

    struct ifreq *it = ifc.ifc_req;
    const struct ifreq *const end = it + (ifc.ifc_len / sizeof(struct ifreq));

    for (; it != end; ++it)
    {
        strcpy(ifr.ifr_name, it->ifr_name);
        if (ioctl(sockfd, SIOCGIFFLAGS, &ifr) == 0)
        {
            if (!(ifr.ifr_flags & IFF_LOOPBACK))
            {
                if (ioctl(sockfd, SIOCGIFHWADDR, &ifr) == 0)
                {
                    unsigned char *mac = (unsigned char *)ifr.ifr_hwaddr.sa_data;
                    return mac;
                }
            }
        }
        else
        {
            perror("ioctl");
        }
    }

    return NULL;
}