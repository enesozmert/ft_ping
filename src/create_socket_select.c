#include "header.h"

int create_socket_select(t_ping *ping){
    ping->time.timeout.tv_sec = 1;
    ping->time.timeout.tv_usec = 0;
    int ret = select(ping->sock_fd + 1, &ping->read_fds, NULL, NULL, &ping->time.timeout);

    if (ret <= 0){
        return -1;
    }
    return 1;
}