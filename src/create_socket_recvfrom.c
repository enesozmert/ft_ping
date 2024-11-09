#include "header.h"

int create_socket_recvfrom(t_ping *ping)
{
    printf("Waiting for ping reply...\n");
    unsigned char recvBuff[1024];
    socklen_t addr_len = sizeof(*(ping->target_addr));
    ssize_t len = recvfrom(ping->sock_fd, &recvBuff, sizeof(recvBuff), 0, (struct sockaddr *)ping->target_addr, &addr_len);
    printf("len %ld", len);
    if (len <= 0)
    {
        perror("Recvfrom error");
        return -1;
    }
    else
    {
        struct iphdr *recv_ip_hdr = (struct iphdr *)(recvBuff);
        struct icmphdr *recv_icmp_hdr = (struct icmphdr *)(recvBuff + (recv_ip_hdr->ihl * 4));

        printf("recv_icmp_hdr->type %d .\n", recv_icmp_hdr->type);
        printf("ICMP_ECHOREPLY %d .\n", ICMP_ECHOREPLY);
        printf("recv_icmp_hdr->type %d, recv_icmp_hdr->code %d\n", recv_icmp_hdr->type, recv_icmp_hdr->code);

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