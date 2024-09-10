#include "header.h"

int create_send_request(t_ping *ping){
    ping->result->sent_packets++;
    gettimeofday(&ping->time.start_time, NULL);
    printf("Sending ping request...\n");
    if (sendto(ping->sock_fd, ping->packet.packet, ping->packet.packet_len, 0, (struct sockaddr *)ping->target_addr, sizeof(*ping->target_addr)) < 0)
    {
        perror("Send failed");
        close(ping->sock_fd);
        return -1; 
    }
    printf("Ping request sent.\n");

    FD_ZERO(&ping->read_fds);
    FD_SET(ping->sock_fd, &ping->read_fds);
    return 1;
}