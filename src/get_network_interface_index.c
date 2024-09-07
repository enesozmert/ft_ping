#include "header.h"

int get_network_interface_index(int sockfd)
{
    struct ifreq ifr;
    struct ifconf ifc;
    char buf[1024];
    struct ifreq *it;
    struct ifreq *end;

    ifc.ifc_len = sizeof(buf);
    ifc.ifc_buf = buf;
    if (ioctl(sockfd, SIOCGIFCONF, &ifc) == -1)
    {
        perror("ioctl SIOCGIFCONF");
        return -1;
    }

    it = ifc.ifc_req;
    end = it + (ifc.ifc_len / sizeof(struct ifreq));

    for (; it != end; ++it)
    {
        strcpy(ifr.ifr_name, it->ifr_name);
        if (ioctl(sockfd, SIOCGIFFLAGS, &ifr) == 0)
        {
            if (!(ifr.ifr_flags & IFF_LOOPBACK))
            {
                return if_nametoindex(it->ifr_name);
            }
        }
        else
        {
            perror("ioctl SIOCGIFFLAGS");
        }
    }

    fprintf(stderr, "No suitable network interface found.\n");
    return -1;
}