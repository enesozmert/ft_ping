#include "header.h"

int get_network_interface_name(t_ping *ping)
{
    struct ifaddrs *ifaddr;
    struct ifaddrs *ifa;
    char host[NI_MAXHOST];
    char *network_interface_name;

    if (getifaddrs(&ifaddr) == -1)
    {
        perror("getifaddrs");
        exit(1);
    }

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr == NULL)
        {
            continue;
        }

        if (ifa->ifa_addr->sa_family == AF_INET)
        {
            int s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in),
                                host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
            if (s != 0)
            {
                printf("getnameinfo() failed: %s\n", gai_strerror(s));
                exit(1);
            }
            if (strcmp(ifa->ifa_name, "lo") != 0)
            {
                network_interface_name = strdup(ifa->ifa_name);
                break;
            }
        }
    }

    freeifaddrs(ifaddr);
    ping->network_interface_name = network_interface_name;
    return 1;
}