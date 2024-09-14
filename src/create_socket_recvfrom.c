#include "header.h"

int create_socket_recvfrom(t_ping *ping)
{
    printf("Waiting for ping reply...\n");
    unsigned char recvBuff[1024];
    socklen_t addr_len = sizeof(*(ping->target_addr));
    ssize_t len = recvfrom(ping->sock_fd, &recvBuff, sizeof(recvBuff), 0, (struct sockaddr *)ping->target_addr, &addr_len);
    if (len <= 0)
    {
        perror("Recvfrom error");
        return -1;
    }
    else
    {
        struct iphdr *recv_ip_hdr = (struct iphdr *)(recvBuff + sizeof(t_ethernet_frame));
        struct icmphdr *recv_icmp_hdr = (struct icmphdr *)(recvBuff + sizeof(t_ethernet_frame) + (recv_ip_hdr->ihl * 4));

        if (recv_icmp_hdr->type == ICMP_ECHOREPLY)
        {
            ping->result->received_packets++;
            gettimeofday(&ping->time.end_time, NULL);
            ping->time.elapsed_time = (ping->time.end_time.tv_sec - ping->time.start_time.tv_sec) * 1000.0;
            ping->time.elapsed_time += (ping->time.end_time.tv_usec - ping->time.start_time.tv_usec) / 1000.0;
            ping->result->rtt += ping->time.elapsed_time;
            printf("%zd bytes from %s: icmp_seq=%d ttl=%d time=%.2f ms\n", len, ping->dest_ip_addr, ntohs(recv_icmp_hdr->un.echo.sequence), recv_ip_hdr->ttl, ping->time.elapsed_time);
        }
    }
    return 1;
}