#include "header.h"

int create_raw_socket(t_ping *ping){
    int sock_fd;

    sock_fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sock_fd < 0)
    {
        perror("Socket creation failed");
        return -1;
    }
    ping->sock_fd = sock_fd;
    return 1;
}
