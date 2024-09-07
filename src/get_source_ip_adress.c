#include "header.h"

uint32_t get_source_ip_address()
{
    struct ifaddrs *ifaddr, *ifa;
    int family, s;
    char host[NI_MAXHOST];
    uint32_t ip_address = 0;

    if (getifaddrs(&ifaddr) == -1)
    {
        perror("getifaddrs");
        exit(EXIT_FAILURE);
    }

    // Iterate through all network interfaces
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr == NULL)
            continue;

        family = ifa->ifa_addr->sa_family;

        // Find an IPv4 address
        if (family == AF_INET)
        {
            s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in),
                            host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
            if (s != 0)
            {
                printf("getnameinfo() failed: %s\n", gai_strerror(s));
                exit(EXIT_FAILURE);
            }

            // Get the first non-loopback IPv4 address
            if (strcmp(ifa->ifa_name, "lo") != 0)
            {
                ip_address = ((struct sockaddr_in *)ifa->ifa_addr)->sin_addr.s_addr;
                break;
            }
        }
    }

    freeifaddrs(ifaddr);

    if (ip_address == 0)
    {
        fprintf(stderr, "Could not determine local IP address\n");
        exit(EXIT_FAILURE);
    }

    return ip_address;
}